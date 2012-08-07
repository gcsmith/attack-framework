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
#include "attack_engine.h"
#include "attack_manager.h"
#include "attack_thread.h"
#include "utility.h"

#define THREADED_REPORTING

using namespace std;
using namespace util;

class report_thread {
public:
    report_thread(const string &attack)
    : m_index(0), m_running(true), m_attack(NULL) {
        m_attack.reset(attack_manager::create_attack(attack));
        assert(NULL != m_attack.get());
    }

    void run(void) {
        boost::unique_lock<boost::mutex> lock(m_mutex);

        while (true) {
            m_condition.wait(lock);
            if (!m_running) break;
            m_attack->record_interval(m_index);
        }
    }

    void compute(const attack_engine::thread_list &threads, size_t index) {
        {
            boost::lock_guard<boost::mutex> lock(m_mutex);

            m_attack->clone(threads.front()->attack());
            for (size_t i = 1; i < threads.size(); ++i)
                m_attack->coalesce(threads[i]->attack());

            m_index = index;
        }
        m_condition.notify_one();
    }

    void terminate(void) {
        {
            boost::lock_guard<boost::mutex> lock(m_mutex);
            m_running = false;
        }
        m_condition.notify_one();
    }

    attack_instance *attack(void) const { return m_attack.get(); }   

protected:
    size_t                         m_index;
    bool                           m_running;
    boost::mutex                   m_mutex;
    boost::condition_variable      m_condition;
    std::auto_ptr<attack_instance> m_attack;
};

// -----------------------------------------------------------------------------
attack_engine::attack_engine(void)
: m_reports(0), m_interval(0), m_index(0)
{
}

// -----------------------------------------------------------------------------
attack_engine::~attack_engine(void)
{
}

// -----------------------------------------------------------------------------
bool attack_engine::run(const options &opt, trace_reader *pReader)
{
    BENCHMARK_DECLARE(attack_whole);

    printf("attacking with %zu trace(s)...\n", pReader->trace_count());

    // perform pre-attack initialization
    if (!attack_setup(opt, pReader))
        return false;

    m_group.join_all();

    // perform post-attack shutdown
    attack_shutdown();

    BENCHMARK_SAMPLE(attack_whole);
    return true;
}

// -----------------------------------------------------------------------------
bool attack_engine::attack_setup(const options &opt, trace_reader *pReader)
{
    // if no results directory is specified, create one using the timestamp
    m_results = opt.result_path;
    if (m_results.length() == 0) {
        ostringstream oss;
        oss << util::timestamp()
            << "_" << opt.attack_name
            << "_" << opt.crypto_name
            << "_" << pReader->trace_count();
        m_results = oss.str();
    }

    if (!util::valid_output_directory(m_results))
        return false;

    const size_t num_traces = pReader->trace_count();

    // process the user-specified attack options
    m_reader   = pReader;
    m_nthreads = max(opt.num_threads, 1U);
    m_interval = opt.report_tick ? opt.report_tick : num_traces;
    m_reports  = 1 + ((num_traces - 1) / m_interval);
    m_index    = 0;

    // disable multithreading if the report interval is too low (no benefit)
    if (m_interval < m_nthreads) {
        fprintf(stderr, "warning: report interval less than thread count\n");
        m_nthreads = 1;
    }

    // write the parameter map used to configure the attack instance
    util::parameters param_map;
    param_map.put("num_events", pReader->events().size());
    param_map.put("num_reports", m_reports);

    foreach (const string &expr, util::split(opt.parameters, ",")) {
        const size_t pos = expr.find_first_of('=');
        if (string::npos == pos) {
            fprintf(stderr, "bad parameter declaration: %s\n", expr.c_str());
            return false;
        }
        param_map.put(expr.substr(0, pos), expr.substr(pos + 1));
    }

#ifdef THREADED_REPORTING
    // spawn the report thread
    m_rt = new report_thread(opt.attack_name);
    m_thrd = boost::thread(&report_thread::run, m_rt);
#endif

    // spawn the worker threads based on the specified thread count
    for (size_t i = 0; i < m_nthreads; ++i) {
        attack_thread *thread = new attack_thread(i, this);
        if (!thread->create(opt.attack_name, opt.crypto_name, param_map)) {
            fprintf(stderr, "failed to launch thread %zu\n", i);
            return false;
        }

        // add the current worker to the thread group for joining
        m_group.create_thread(boost::bind(&attack_thread::run, thread));
        m_threads.push_back(thread);
    }

    return true;
}

// -----------------------------------------------------------------------------
void attack_engine::attack_shutdown(void)
{
    // if there are multiple worker threads, merge their results
    printf("\n");

    // compute the final differential trace and write the attack results
    vector<double> diffs, maxes;

#ifdef THREADED_REPORTING
    m_rt->terminate();
    m_thrd.join();
    attack_instance *attack = m_rt->attack();
#else
    attack_instance *attack = m_threads.front()->attack();
    for (size_t i = 1; i < m_threads.size(); ++i) {
        printf("coalescing thread instance [%zu]...\n", i);
        attack->coalesce(m_threads[i]->attack());
    }
#endif

    attack->get_diffs(diffs);
    attack->get_maxes(maxes);

    const int num_guesses = 1 << m_threads.front()->crypto()->estimate_bits();
    write_maxes_report(maxes, num_guesses);
    write_confs_report(maxes, num_guesses);
    write_diffs_report(diffs, num_guesses);

    // allow the attack to write out additional reports if necessary
    attack->write_results(m_results);

    // finally, destroy the threads themselves
    foreach (attack_thread *thread, m_threads) delete thread;
    m_threads.clear();
}

// -----------------------------------------------------------------------------
// Read in the next available power trace and generate the trace's event map.
bool attack_engine::next_trace(int id, vector<long> &tmap, trace &pt)
{
    {
        // lock the next power trace selection to avoid race conditions
        boost::lock_guard<boost::mutex> lock(m_mutex);
        const size_t num_traces = m_reader->trace_count();

        if (m_index && (!(m_index % m_interval) || m_index == num_traces)) {
            const size_t interval_index = (m_index - 1) / m_interval;
#ifdef THREADED_REPORTING
            m_rt->compute(m_threads, interval_index);
#else
            m_threads[0]->attack()->record_interval(interval_index);
#endif
        }

        // if there are no more traces left, signal threads to terminate
        if (m_index >= num_traces)
            return false;

        // request the next power trace from the trace reader
        if (!m_reader->read(pt)) {
            fprintf(stderr, "[%d] failed to read trace %zu\n", id, m_index + 1);
            return false;
        }

        printf("processing trace %s [%d:%zu/%zu]\r",
               util::btoa(pt.text()).c_str(), id, ++m_index, num_traces);
    }

    if (tmap.size() && tmap.size() != pt.size()) {
        fprintf(stderr, "event count mismatch\n");
        return false;
    }

    tmap.resize(pt.size());
    for (size_t s = 0; s < pt.size(); ++s) tmap[s] = s;

    return true;
}

// -----------------------------------------------------------------------------
void attack_engine::write_diffs_report(const vector<double> &diffs, int nk)
{
    const size_t num_events = m_reader->events().size();
    if (diffs.size() != nk * num_events) {
        fprintf(stderr, "invalid # of differentials in write_diffs_report\n");
        return;
    }

    // attempt to open the differential report file for writing
    const string path = util::concat_name(m_results, "differentials.csv");
    ofstream report(path.c_str());

    if (!report.is_open()) {
        fprintf(stderr, "failed to open '%s' for writing\n", path.c_str());
        return;
    }
    printf("writing %s ...\n", path.c_str());

    int best_k = -1, at_sample = -1;
    double max_diff = 0.0;
    size_t sample_number = 0;

    foreach (uint32_t sample_time, m_reader->events()) {
        report << scientific << sample_time;
        for (int k = 0; k < nk; ++k) {
            double value = diffs[k * num_events + sample_number];
            report << ',' << value;
            if (fabs(value) > max_diff) {
                max_diff = fabs(value);
                best_k = k;
                at_sample = sample_number;
            }
        }
        report << endl;
        ++sample_number;
    }

    printf("best key guess = %02x @ %d (%lf)\n", best_k, at_sample, max_diff);
}

// -----------------------------------------------------------------------------
void attack_engine::write_maxes_report(const vector<double> &maxes, int nk)
{
    if (m_reports == 1 || maxes.size() != nk * m_reports)
        return;

    // attempt to open the confidence interval report file for writing
    const string path = util::concat_name(m_results, "interval_maxes.csv");
    ofstream report(path.c_str());

    if (!report.is_open()) {
        fprintf(stderr, "failed to open '%s' for writing\n", path.c_str());
        return;
    }
    printf("writing %s ...\n", path.c_str());

    for (size_t i = 0; i < m_reports; ++i) {
        const double *m = &maxes[i * nk];
        report << scientific << m_interval * (i + 1);
        for (int k = 0; k < nk; ++k) report << ',' << m[k];
        report << endl;
    }
}

// -----------------------------------------------------------------------------
void attack_engine::write_confs_report(const vector<double> &maxes, int nk)
{
    if (m_reports == 1 || maxes.size() != nk * m_reports)
        return;

    // attempt to open the confidence interval report file for writing
    const string path = util::concat_name(m_results, "confidence_interval.csv");
    ofstream report(path.c_str());

    if (!report.is_open()) {
        fprintf(stderr, "failed to open '%s' for writing\n", path.c_str());
        return;
    }
    printf("writing %s ...\n", path.c_str());

    // always write a row of zeroes for the first report
    report << "0";
    for (int k = 0; k < nk; ++k) {
        report << ",0";
    }
    report << endl;

    for (size_t i = 0; i < m_reports; ++i) {
        // compute the confidence ratio for each key guess in this interval
        report << scientific << m_interval * (i + 1);
        for (int k = 0; k < nk; ++k) {
            double correct = maxes[i * nk + k];
            double incorrect = 0.0;
            for (int j = 0; j < nk; ++j)
                if (j != k) incorrect = max(incorrect, maxes[i * nk + j]);
            report << ',' << (incorrect > 0.0 ? correct / incorrect : 0);
        }
        report << endl;
    }
}

