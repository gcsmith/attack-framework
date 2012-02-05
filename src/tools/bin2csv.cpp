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

#include <string>
#include <cstdio>
#include "cmdline.h"
#include "trace.h"
#include "utility.h"

using namespace std;

// -----------------------------------------------------------------------------
bool convert_traces(const string &o_dir, const util::pathlist &paths,
                    const time_range &range, size_t max_trace)
{
    BENCHMARK_DECLARE(trace_convert);

    size_t trace_count = paths.size();
    if (max_trace && max_trace < trace_count) {
        printf("only processing %zu of %zu traces\n", max_trace, trace_count);
        trace_count = max_trace;
    }

    // create a set to record the sample time indices across all traces
    event_set sample_times;
    trace pt(4096);

    for (size_t i = 0; i < trace_count; ++i) {
        // construct the output file path
        string base = util::base_name(paths[i]);
        string name = util::concat_name(o_dir, base, ".csv");

        printf("converting %s... [%zu/%zu]\n", base.c_str(), i+1, trace_count);

        if (!pt.read_bin(paths[i])) break;
        if (!pt.write_csv(name)) break;

        BENCHMARK_SAMPLE_WHEN(trace_convert, i && !(i % 100));
    }

    // write the timing information to a text file
    //return trace::write_profile("timing_profile.txt", sample_times);
    return true;
}

// -----------------------------------------------------------------------------
int main(int argc, char *argv[])
{
    // build and parse the table of command line arguments
    static const string usage_message = string(argv[0]) + " [options]";
    static const cmdline_option cmdline_args[] = {
        { CL_STR,  "input,i",      "input trace file or directory path" },
        { CL_STR,  "output-dir,o", "specify the output trace directory path" },
        { CL_LONG, "time-min,m",   "earliest sample event time to convert" },
        { CL_LONG, "time-max,M",   "latest sample event time to convert" },
        { CL_LONG, "num-traces,n", "maximum number of traces to process" },
        { CL_FLAG, "help,h",       "display this usage message" },
        { CL_TERM, 0, 0 }
    };

    cmdline cl(cmdline_args, usage_message);
    if (!cl.parse(argc, argv) || cl.count("help")) {
        cl.print_usage();
        return 1;
    }

    // parse each command line argument from the variable map
    string input   = cl.get_str("input", "trace_out");
    string out_dir  = cl.get_str("output-dir", "trace_bin");
    long time_min   = cl.get_long("time-min", 0);
    long time_max   = cl.get_long("time-max", 402000);
    long num_traces = cl.get_long("num-traces", 0);
    time_range tr   = make_pair(time_min, time_max);

    // make sure that the input and output directories exist and are valid
//  if (!util::check_inout_directories(input, out_dir))
//      return 1;

    util::pathlist paths;
    if (util::is_dir(input))
        util::scan_directory(input, ".bin", paths);
    else
        paths.push_back(input);

    return convert_traces(out_dir, paths, tr, num_traces);
}

