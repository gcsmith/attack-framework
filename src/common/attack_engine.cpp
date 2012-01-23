// dpa framework - a collection of tools for differential power analysis
// Copyright (C) 2011  Garrett C. Smith
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include <fstream>
#include <cstdio>
#include <algorithm>
#include <boost/bind.hpp>
#include "trace.h"
#include "utility.h"
#include "attack_engine.h"

using namespace std;
using namespace util;

// -----------------------------------------------------------------------------
class attack_thread
{
public:
    attack_thread(int id, attack_engine *engine);
    ~attack_thread();

    bool create(const string &an, const string &cn, const parameters &p);
    void run();
    attack_instance *attack() const { return m_attack.get(); }
    crypto_instance *crypto() const { return m_crypto.get(); }

protected:
    int m_id;
    attack_engine *m_engine;
    auto_ptr<attack_instance> m_attack;
    auto_ptr<crypto_instance> m_crypto;
};

// -----------------------------------------------------------------------------
attack_thread::attack_thread(int id, attack_engine *engine)
: m_id(id), m_engine(engine)
{
}

// -----------------------------------------------------------------------------
attack_thread::~attack_thread()
{
    m_attack->cleanup();
}

// -----------------------------------------------------------------------------
bool attack_thread::create(const string &an, const string &cn, const parameters &p)
{
    m_attack.reset(attack_manager::create_attack(an));
    if (!m_attack.get()) {
        fprintf(stderr, "failed to create attack instance %s\n", an.c_str());
        return false;
    }

    m_crypto.reset(attack_manager::create_crypto(cn));
    if (!m_crypto.get()) {
        fprintf(stderr, "failed to create crypto instance %s\n", cn.c_str());
        return false;
    }

    return m_attack->setup(m_crypto.get(), p);
}

// -----------------------------------------------------------------------------
void attack_thread::run()
{
    trace pt(4096);
    vector<long> mapper;

    while (m_engine->next_trace(m_id, mapper, pt))
        m_attack->process(mapper, pt);
}

// -----------------------------------------------------------------------------
attack_engine::attack_engine()
: m_tracemax(0), m_numtraces(0), m_numintervals(0), m_interval(0),
  m_numthreads(3), m_idir("."), m_odir("."), m_prefix("out"),
  m_attack_name("cpa"), m_crypto_name("aes_hw_r0")
{
}

// -----------------------------------------------------------------------------
attack_engine::~attack_engine()
{
}

// -----------------------------------------------------------------------------
bool attack_engine::open_out(const string &path, ofstream &fout)
{
    fout.open(path.c_str());
    if (fout.bad()) {
        fprintf(stderr, "failed to open output file '%s'\n", path.c_str());
        return false;
    }
    return true;
}

// -----------------------------------------------------------------------------
void attack_engine::set_attack(const string &name)
{
    m_attack_name = name;
}

// -----------------------------------------------------------------------------
void attack_engine::set_crypto(const string &name)
{
    m_crypto_name = name;
}

// -----------------------------------------------------------------------------
void attack_engine::set_params(const string &params)
{
    vector<string> tok;
    util::split(tok, params, ",");

    for (vector<string>::const_iterator i = tok.begin(); i != tok.end(); ++i) {
        const string &expr = *i;
        size_t pos = expr.find_first_of('=');
        if (string::npos == pos) {
            fprintf(stderr, "bad parameter declaration: %s\n", expr.c_str());
            continue;
        }
        m_params.put(expr.substr(0, pos), expr.substr(pos + 1));
    }
}

// -----------------------------------------------------------------------------
bool attack_engine::set_paths(const string &i_dir, const string &o_dir)
{
    // make sure that the input and output directories exist and are valid
    if (!util::check_inout_directories(i_dir, o_dir))
        return false;

    m_idir = i_dir;
    m_odir = o_dir;
    return true;
}

// -----------------------------------------------------------------------------
void attack_engine::set_results_prefix(const string &prefix)
{
    m_prefix = prefix;
}

// -----------------------------------------------------------------------------
void attack_engine::set_num_traces(size_t count)
{
    m_tracemax = count;
}

// -----------------------------------------------------------------------------
void attack_engine::set_report_interval(size_t count)
{
    m_interval = count;
}

// -----------------------------------------------------------------------------
void attack_engine::set_thread_count(size_t count)
{
    m_numthreads = count;
}

// -----------------------------------------------------------------------------
bool attack_engine::load_trace_order(const string &path)
{
    // read in the order in which to process the traces
    return trace::read_profile(path, m_order);
}

// -----------------------------------------------------------------------------
bool attack_engine::load_trace_profile(const string &path)
{
    // read in the time indices for these traces from the timing profile
    return trace::read_profile(path, m_times);
}

// -----------------------------------------------------------------------------
void attack_engine::write_diffs_report(const vector<double> &diffs, int nk)
{
    const size_t num_events = m_times.size();
    if (diffs.size() != nk * num_events) {
        fprintf(stderr, "invalid # of differentials in write_diffs_report\n");
        return;
    }

    for (size_t i = 0; i < num_events; ++i) {
        m_odif << scientific << m_times[i] << ',';
        for (int k = 0; k < nk; ++k)
            m_odif << diffs[k * num_events + i] << ',';
        m_odif << endl;
    }
}

// -----------------------------------------------------------------------------
void attack_engine::write_maxes_report(const vector<double> &maxes)
{
    if (maxes.size() != 256 * m_numintervals) {
        fprintf(stderr, "invalid # of maxes in write_maxes_report\n");
        return;
    }

    for (size_t i = 0; i < m_numintervals; ++i) {
        const double *m = &maxes[i * 256];
        m_omax << scientific << m_interval * (i + 1) << ',';
        for (int k = 0; k < 256; ++k) m_omax << m[k] << ',';
        m_omax << endl;
    }
}

// -----------------------------------------------------------------------------
void attack_engine::write_group_report(const vector<size_t> &group, int ngroups)
{
    if (!ngroups || group.size() != 256 * ngroups * m_numintervals) {
        fprintf(stderr, "invalid # of groups in write_group_report\n");
        return;
    }

    for (size_t i = 0; i < m_numintervals; ++i) {
        const size_t *c = &group[i * 256 * ngroups];
        m_ogrp << m_interval * (i + 1);

        for (int g = 0; g < ngroups; ++g) {
            m_ogrp << ',' << g << ',';
            for (int k = 0; k < 256; ++k)
                m_ogrp << c[k * ngroups + g] << ',';
            m_ogrp << endl;
        }
    }
}

// -----------------------------------------------------------------------------
void attack_engine::write_confs_report(const vector<double> &maxes)
{
    if (maxes.size() != 256 * m_numintervals) {
        fprintf(stderr, "invalid # of maxes in write_confs_report\n");
        return;
    }

    for (size_t i = 0; i < m_numintervals; ++i) {
        // compute the confidence ratio for each key guess in this interval
        for (int k = 0; k < 256; ++k) {
            double correct = maxes[i * 256 + k];
            double incorrect = 0.0;
            for (int j = 0; j < 256; ++j)
                if (j != k) incorrect = max(incorrect, maxes[i * 256 + j]);
            m_conf << (correct / incorrect) << ',';
        }
        m_conf << endl;
    }
}

// -----------------------------------------------------------------------------
bool attack_engine::check_order(vector<long> &order, size_t tnum, size_t tmax)
{
    // must specify order for at least as many traces as we're processing
    if (order.size() < tnum)
        return false;

    // check for duplicates and indices out of bounds
    set<long> duplicates;
    for (size_t i = 0; i < order.size(); ++i) {
        if (order[i] >= (long)tmax || duplicates.count(order[i]))
            return false;
        else duplicates.insert(order[i]);
    }

    return true;
}

// -----------------------------------------------------------------------------
void attack_engine::build_order(vector<long> &order, size_t tnum, size_t tmax)
{
    order.resize(tmax);
    for (size_t i = 0; i < tmax; ++i)
        order[i] = i;
    random_shuffle(order.begin(), order.end());
    order.resize(tnum);
}

// -----------------------------------------------------------------------------
bool attack_engine::attack_setup()
{
    // open the results file descriptors well in advance so we don't have to
    // bail out after a particularly lengthy attack
    if (!open_out(util::concat_name(m_odir, m_prefix + "_diffs.csv"), m_odif) ||
        !open_out(util::concat_name(m_odir, m_prefix + "_group.csv"), m_ogrp) ||
        !open_out(util::concat_name(m_odir, m_prefix + "_maxes.csv"), m_omax) || 
        !open_out(util::concat_name(m_odir, m_prefix + "_confs.csv"), m_conf))
        return false;

    if (!util::scan_directory(m_idir, ".bin", m_paths))
        return false;

    // determine the number of traces and the order we will process them in
    m_numtraces = m_paths.size();
    if (m_tracemax && m_tracemax < m_numtraces) {
        printf("only processing %ld of %ld traces\n", m_tracemax, m_numtraces);
        m_numtraces = m_tracemax;
    }

    if (!check_order(m_order, m_numtraces, m_paths.size())) {
        printf("trace order invalid or unspecified. using random order\n");
        build_order(m_order, m_numtraces, m_paths.size());
    }

    // determine the number of interval reports, including the final report
    if (m_interval <= 0)
        m_interval = m_numtraces;

    m_numintervals = m_numtraces / m_interval;
    if (m_numtraces % m_interval)
        ++m_numintervals;
    
    m_params.put("num_events", m_times.size());
    m_params.put("num_reports", m_numintervals);

    // map each sample time to its relative index in the list
    for (size_t index = 0; index < m_times.size(); ++index)
        m_xlat.insert(make_pair(m_times[index], (long)index));

    m_trace = 0;
    m_threads.resize(m_numthreads);

    for (size_t i = 0; i < m_numthreads; ++i) {
        m_threads[i] = new attack_thread(i, this);
        if (!m_threads[i]->create(m_attack_name, m_crypto_name, m_params))
            return false;

        m_group.create_thread(boost::bind(&attack_thread::run, m_threads[i]));
    }

    return true;
}

// -----------------------------------------------------------------------------
void attack_engine::attack_shutdown()
{
    vector<double> diffs, maxes;
    vector<size_t> groups;
    int ngroups = 0, k = 1 << m_threads[0]->crypto()->estimate_bits();

    m_threads[0]->attack()->get_diffs(diffs);
    m_threads[0]->attack()->get_maxes(maxes);
    m_threads[0]->attack()->get_group(groups, ngroups);

    write_diffs_report(diffs, k);
    write_maxes_report(maxes);
    write_confs_report(maxes);
    write_group_report(groups, ngroups);

    vector<attack_thread *>::const_iterator i;
    for (i = m_threads.begin(); i != m_threads.end(); ++i)
        delete *i;
}

// -----------------------------------------------------------------------------
bool attack_engine::run()
{
    BENCHMARK_DECLARE(attack_whole);

    if (!attack_setup())
        return false;

    printf("attacking with %ld trace(s)...\n", m_numtraces);

    m_group.join_all();

    // if there are multiple worker threads, merge their results
    if (m_threads.size() > 1) {
        printf("coalescing thread instances...\n");
        for (size_t i = 1; i < m_threads.size(); ++i)
            m_threads[0]->attack()->coalesce(m_threads[i]->attack());
    }

    attack_shutdown();

    BENCHMARK_SAMPLE(attack_whole);
    return true;
}

// -----------------------------------------------------------------------------
bool attack_engine::next_trace(int id, vector<long> &tm, trace &pt)
{
    string path;
    {
        boost::lock_guard<boost::mutex> lock(m_mutex);
        if (m_trace >= m_numtraces)
            return false;

        path = m_paths[m_order[m_trace++]];
        printf("processing trace %s [%d:%lu/%lu]\n",
               path.c_str(), id, m_trace, m_numtraces);
    }

    // extract the 16 byte plaintext from the trace's filename and load trace
    vector<uint8_t> bytes(16);
    util::parse_plaintext(path, &bytes[0], 0);
    pt.set_text(bytes);

#if 0
    if (!pt.read_bin(path))
        return false;

    // map each sample index to its corresponding event index
    const size_t num_samples = pt.size();
    tm.reserve(num_samples);

    for (size_t s = 0; s < num_samples; ++s) {
        xlat_map::const_iterator iter = m_xlat.find(pt[s].time);
        if (iter == m_xlat.end()) {
            printf("event %d not present in timing profile\n", pt[s].time);
            return false;
        }
        tm[s] = iter->second;
    }
#else
    trace temp;
    if (!temp.read_bin(path))
        return false;
    pt.sample_and_hold(temp, m_times);

    tm.resize(pt.size());
    for (size_t s = 0; s < pt.size(); ++s) tm[s] = s;
#endif

    return true;
}

