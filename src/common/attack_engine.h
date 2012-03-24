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

#ifndef ATTACK_ENGINE__H
#define ATTACK_ENGINE__H

#include <fstream>
#include <vector>
#include <memory>
#include <map>
#include <tr1/unordered_map>
#include <boost/thread/thread.hpp>
#include <boost/thread/locks.hpp>
#include "utility.h"
#include "trace_format.h"
#include "attack_manager.h"

class attack_thread;

class attack_engine {
    typedef std::tr1::unordered_map<long, long> xlat_map;

public:
    // attack_engine constructor
    attack_engine(void);

    // attack_engine destructor
    ~attack_engine(void);

    // set the attack algorithm (ie. cpa, dpa)
    void set_attack(const std::string &name);

    // set the crypto algorithm to attack (ie. aes, des)
    void set_crypto(const std::string &name);

    // set the trace reader object
    void set_reader(trace_reader *pReader);

    // set parameter list to control the attack algorithm (attack dependent)
    void set_params(const std::string &params);

    // set a prefix for each results file
    void set_results_prefix(const std::string &prefix);

    // set the maximum number of traces to process
    void set_num_traces(size_t count);

    // generate reports at the specified interval (in terms of traces)
    void set_report_interval(size_t count);

    // set the number of attack threads to spawn
    void set_thread_count(size_t count);

    // load the trace timing profile
    bool load_trace_profile(const std::string &path);

    // execute the attack and write the results to results_path
    bool run(const std::string &results_path);

    // fetch the next trace to process
    bool next_trace(int id, std::vector<long> &tm, trace &pt);

protected:
    // TODO: description
    bool open_out(const std::string &path, std::ofstream &fout);

    // TODO: description
    void write_diffs_report(const std::vector<double> &diffs, int k);

    // TODO: description
    void write_maxes_report(const std::vector<double> &maxes);

    // TODO: description
    void write_confs_report(const std::vector<double> &maxes);

    // TODO: description
    bool attack_setup(const std::string &results_path);

    // TODO: description
    void attack_shutdown(void);

protected:
    util::parameters m_params;
    std::vector<long> m_times;
    size_t m_tracemax;
    size_t m_numtraces;
    size_t m_numintervals;
    size_t m_interval;
    size_t m_numthreads;
    size_t m_trace;
    std::string m_odir, m_prefix;
    std::ofstream m_odif;
    std::ofstream m_omax;
    std::ofstream m_conf;
    std::string m_attack;
    std::string m_crypto;
    trace_reader *m_reader;
    xlat_map m_xlat;

    boost::mutex m_mutex;
    boost::thread_group m_group;
    std::vector<attack_thread *> m_threads;
};

#endif // ATTACK_ENGINE__H

