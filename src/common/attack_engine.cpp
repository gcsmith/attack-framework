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
#include "attack_thread.h"

using namespace std;
using namespace util;

// -----------------------------------------------------------------------------
attack_engine::attack_engine(void)
: m_tracemax(0), m_numtraces(0), m_numintervals(0), m_interval(0),
  m_numthreads(3), m_prefix("attack"), m_attack("cpa"), m_crypto("aes_hw_r0")
{
}

// -----------------------------------------------------------------------------
attack_engine::~attack_engine(void)
{
}

// -----------------------------------------------------------------------------
bool attack_engine::open_out(const string &path, ofstream &fout)
{
    fout.open(path.c_str());
    if (!fout.is_open()) {
        fprintf(stderr, "failed to open output file '%s'\n", path.c_str());
        return false;
    }
    return true;
}

// -----------------------------------------------------------------------------
void attack_engine::set_attack(const string &name)
{
    m_attack = name;
}

// -----------------------------------------------------------------------------
void attack_engine::set_crypto(const string &name)
{
    m_crypto = name;
}

// -----------------------------------------------------------------------------
void attack_engine::set_reader(trace_reader *pReader)
{
    m_reader = pReader;
}

// -----------------------------------------------------------------------------
void attack_engine::set_params(const string &params)
{
    foreach (const string &expr, util::split(params, ",")) {
        size_t pos = expr.find_first_of('=');
        if (string::npos == pos) {
            fprintf(stderr, "bad parameter declaration: %s\n", expr.c_str());
            continue;
        }
        m_params.put(expr.substr(0, pos), expr.substr(pos + 1));
    }
}

// -----------------------------------------------------------------------------
void attack_engine::set_results_prefix(const string &prefix)
{
    if (prefix.length())
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

    int best_k = -1, at_sample = -1;
    double max_diff = 0.0;
    for (size_t i = 0; i < num_events; ++i) {
        m_odif << scientific << m_times[i] << ',';
        for (int k = 0; k < nk; ++k) {
            double value = diffs[k * num_events + i];
            m_odif << value << ',';
            if (fabs(value) > max_diff) {
                max_diff = fabs(value);
                best_k = k;
                at_sample = i;
            }
        }
        m_odif << endl;
    }
    printf("best key guess = %02x @ %d (%lf)\n", best_k, at_sample, max_diff);
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
bool attack_engine::attack_setup(const string &odir)
{
    if (!util::valid_output_dir(odir))
        return false;

    // open the results file descriptors well in advance so we don't have to
    // bail out after a particularly lengthy attack
    if (!open_out(util::concat_name(odir, m_prefix + "_diffs.csv"), m_odif) ||
        !open_out(util::concat_name(odir, m_prefix + "_maxes.csv"), m_omax) || 
        !open_out(util::concat_name(odir, m_prefix + "_confs.csv"), m_conf))
        return false;

    // determine the number of traces and the order we will process them in
    m_numtraces = m_reader->trace_count();
    if (m_tracemax && m_tracemax < m_numtraces) {
        printf("only processing %zu of %zu traces\n", m_tracemax, m_numtraces);
        m_numtraces = m_tracemax;
    }

    // determine the number of interval reports, including the final report
    if (m_interval <= 0) {
        m_interval = m_numtraces;
    }
    else if (m_numthreads > 1) {
        fprintf(stderr, "cannot set report interval with multiple threads\n");
        return false;
    }

    m_numintervals = m_numtraces / m_interval;
    if (m_numtraces % m_interval)
        ++m_numintervals;
    
    m_params.put("num_events", m_times.size());
    m_params.put("num_reports", m_numintervals);

    m_trace = 0;
    m_threads.resize(m_numthreads);

    for (size_t i = 0; i < m_numthreads; ++i) {
        m_threads[i] = new attack_thread(i, this);
        if (!m_threads[i]->create(m_attack, m_crypto, m_params))
            return false;

        m_group.create_thread(boost::bind(&attack_thread::run, m_threads[i]));
    }

    return true;
}

// -----------------------------------------------------------------------------
void attack_engine::attack_shutdown(void)
{
    vector<double> diffs, maxes;
    int k = 1 << m_threads[0]->crypto()->estimate_bits();

    m_threads[0]->attack()->get_diffs(diffs);
    m_threads[0]->attack()->get_maxes(maxes);

    write_diffs_report(diffs, k);
    write_maxes_report(maxes);
    write_confs_report(maxes);

    foreach (attack_thread *thrd, m_threads) {
        delete thrd;
    }
}

// -----------------------------------------------------------------------------
bool attack_engine::run(const string &results_path)
{
    BENCHMARK_DECLARE(attack_whole);

    if (!attack_setup(results_path))
        return false;

    printf("attacking with %zu trace(s)...\n", m_numtraces);

    m_group.join_all();

    // if there are multiple worker threads, merge their results
    for (size_t i = 1; i < m_threads.size(); ++i) {
        printf("coalescing thread instance [%zu]...\n", i);
        m_threads[0]->attack()->coalesce(m_threads[i]->attack());
    }

    attack_shutdown();

    BENCHMARK_SAMPLE(attack_whole);
    return true;
}

// -----------------------------------------------------------------------------
// Read in the next available power trace and generate the trace's event map.
bool attack_engine::next_trace(int id, vector<long> &tmap, trace &pt)
{
    {
        // lock the next power trace selection to avoid race conditions
        boost::lock_guard<boost::mutex> lock(m_mutex);
        if (m_trace >= m_numtraces)
            return false;

        if (m_trace && !(m_trace % m_interval)) {
            printf("report interval %zu...\n", m_trace / m_interval);
            m_threads[0]->attack()->record_interval(m_trace / m_interval);
        }

        // request the next power trace from the trace reader
        trace::time_range range(0, 0);
        if (!m_reader->read(pt, range)) {
            fprintf(stderr, "[%d] failed to read trace %zu\n", id, m_trace + 1);
            return false;
        }

        const string trace_text(util::btoa(pt.text()));
        printf("processing trace %s [%d:%zu/%zu]\n",
               trace_text.c_str(), id, ++m_trace, m_numtraces);
    }

    if (tmap.size() && tmap.size() != pt.size()) {
        fprintf(stderr, "event count mismatch\n");
        return false;
    }

    tmap.resize(pt.size());
    for (size_t s = 0; s < pt.size(); ++s) tmap[s] = s;

    return true;
}

