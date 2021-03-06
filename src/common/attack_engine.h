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

#include <vector>
#include <boost/thread/thread.hpp>
#include <boost/thread/locks.hpp>
#include "trace_format.h"

class attack_thread;
class report_thread;

//! front-end for performing power analysis attacks
class attack_engine {
public:
    typedef std::vector<attack_thread *> thread_list;

    //! user-specified options to configure the attack
    struct options {
        std::string attack_name;
        std::string crypto_name;
        std::string parameters;
        std::string result_path;
        unsigned int num_threads;
        unsigned int report_tick;
    };

    //! attack_engine constructor -- set default parameters
    attack_engine(void);

    //! attack_engine destructor -- perform any last minute cleanup
    ~attack_engine(void);

    //! execute the attack and write the results to results_path
    bool run(const options &opt, trace_reader *pReader);

    //! fetch the next power trace for processing
    bool next_trace(int id, std::vector<long> &tm, trace &pt);

protected:
    //! write the differential trace report
    void write_diffs_report(const std::vector<double> &diffs, int guesses);

    //! write the maximum trace report
    void write_maxes_report(const std::vector<double> &maxes, int guesses);

    //! write the confidence interval report
    void write_confs_report(const std::vector<double> &maxes, int guesses);

    //! perform pre-attack initialization
    bool attack_setup(const options &opt, trace_reader *pReader);

    //! perform post-attack shutdown
    void attack_shutdown(void);

protected:
    size_t              m_reports;  //! total number of reports to generate
    size_t              m_interval; //! user specified reporting interval
    size_t              m_index;    //! current available trace index
    size_t              m_nthreads; //! number of threads to launch
    std::string         m_results;  //! output results directory
    trace_reader       *m_reader;   //! generic trace reader
    boost::mutex        m_mutex;    //! critical section for trace_reader
    boost::thread_group m_group;    //! collection of worker threads
    thread_list         m_threads;  //! collection of worker instances
    report_thread      *m_rt;
    boost::thread       m_thrd;
};

#endif // ATTACK_ENGINE__H

