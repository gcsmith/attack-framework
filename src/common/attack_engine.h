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

#ifndef ATTACK__H
#define ATTACK__H

#include <fstream>
#include <vector>
#include <memory>
#include <map>
#include <tr1/unordered_map>
#include <boost/thread/thread.hpp>
#include <boost/thread/locks.hpp>
#include "utility.h"
#include "attack_manager.h"

class attack_thread;

class attack_engine
{
    typedef std::tr1::unordered_map<long, long> xlat_map;

public:
    // TODO: description
    attack_engine();

    // TODO: description
    ~attack_engine();

    // TODO: description
    void set_attack(const std::string &name);

    // TODO: description
    void set_crypto(const std::string &name);

    // TODO: description
    void set_params(const std::string &params);

    // TODO: description
    bool set_paths(const std::string &i_dir, const std::string &o_dir);

    // TODO: description
    void set_results_prefix(const std::string &prefix);

    // TODO: description
    void set_num_traces(size_t count);

    // TODO: description
    void set_report_interval(size_t count);

    // TODO: description
    void set_thread_count(size_t count);

    // TODO: description
    bool load_trace_order(const std::string &path);

    // TODO: description
    bool load_trace_profile(const std::string &path);

    // TODO: description
    bool run();

    // TODO: description
    bool next_trace(int id, std::vector<long> &tm, trace &pt);

protected:
    // TODO: description
    bool open_out(const std::string &path, std::ofstream &fout);

    // TODO: description
    void write_diffs_report(const std::vector<double> &diffs, int k);

    // TODO: description
    void write_maxes_report(const std::vector<double> &maxes);

    // TODO: description
    void write_group_report(const std::vector<size_t> &group, int ngroups);

    // TODO: description
    void write_confs_report(const std::vector<double> &maxes);

    // TODO: description
    bool attack_setup();

    // TODO: description
    void attack_shutdown();

    // TODO: description
    bool map_events(std::vector<long> &mapper, const trace &pt);

    // TODO: description
    static bool check_order(std::vector<long> &order, size_t tnum, size_t tmax);

    // TODO: description
    static void build_order(std::vector<long> &order, size_t tnum, size_t tmax);

protected:
    util::pathlist m_paths;
    util::parameters m_params;
    std::vector<long> m_order;
    std::vector<long> m_times;
    size_t m_tracemax;
    size_t m_numtraces;
    size_t m_numintervals;
    size_t m_interval;
    size_t m_numthreads;
    size_t m_trace;
    std::string m_idir, m_odir, m_prefix;
    std::ofstream m_odif;
    std::ofstream m_ogrp;
    std::ofstream m_omax;
    std::ofstream m_conf;
    std::string m_attack_name;
    std::string m_crypto_name;
    xlat_map m_xlat;

    boost::mutex m_mutex;
    boost::thread_group m_group;
    std::vector<attack_thread *> m_threads;
};

#endif // ATTACK__H

