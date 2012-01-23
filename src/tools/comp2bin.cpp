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
#include <string>
#include <cstdio>
#include "cmdline.h"
#include "trace.h"
#include "utility.h"

using namespace std;

typedef vector<pair<string, string> > tracelist;

// -----------------------------------------------------------------------------
bool parse_index_file(const string &path, const string &key,
                      bool ciphertext, tracelist &traces)
{
    ifstream fin(path.c_str());
    if (!fin.is_open()) {
        fprintf(stderr, "failed to open index file %s\n", path.c_str());
        return false;
    }

    string line;
    vector<string> tokens;
    while (getline(fin, line)) {
        // make sure each line has the expected number of columns
        util::split(tokens, line, " ");
        if (4 != tokens.size()) {
            fprintf(stderr, "incorrectly formatted database index\n");
            return false;
        }

        if (!key.size() || key == tokens[0]) {
            string block = ciphertext ? tokens[2] : tokens[1];
            traces.push_back(make_pair(block, tokens[3]));
        }
    }

    printf("matched %ld trace files for key %s\n", traces.size(), key.c_str());
    return true;
}

// -----------------------------------------------------------------------------
bool convert_traces(const string &i_dir, const string &o_dir,
                    const tracelist &traces, const time_range &range,
                    size_t max_trace)
{
    BENCHMARK_DECLARE(trace_convert);

    size_t trace_count = traces.size();
    if (max_trace && max_trace < trace_count) {
        printf("only processing %ld of %ld traces\n", max_trace, trace_count);
        trace_count = max_trace;
    }

    trace pt(4096);
    for (size_t i = 0; i < trace_count; ++i) {
        // construct the output file path
        string o_name = util::hexstring_to_filename(traces[i].first, 16);
        string o_path = util::concat_name(o_dir, o_name, ".bin");
        string i_path = util::concat_name(i_dir, traces[i].second, ".csv");

        printf("converting %s... [%ld/%ld]\n", o_name.c_str(), i+1, trace_count);

        // convert trace .out file to a more compact binary representation
        if (!pt.read_csv(i_path, range)) break;
        if (!pt.write_bin(o_path, FMT_PWR_S16)) break;

        BENCHMARK_SAMPLE_WHEN(trace_convert, i && !(i % 100));
    }

    // write the timing information to a text file
    return trace::write_profile("timing_profile.txt", 0, pt.size());
}

// -----------------------------------------------------------------------------
int main(int argc, char *argv[])
{
    // build and parse the table of command line arguments
    static const string usage_message = string(argv[0]) + " [options]";
    static const cmdline_option cmdline_args[] = {
        { CL_STR,  "db-index,d",   "specify the trace database index file" },
        { CL_STR,  "input-dir,i",  "specify the input trace directory path" },
        { CL_STR,  "output-dir,o", "specify the output trace directory path" },
        { CL_STR,  "key,k",        "convert traces with the specified key" },
        { CL_LONG, "time-min,m",   "earliest sample event time to convert" },
        { CL_LONG, "time-max,M",   "latest sample event time to convert" },
        { CL_LONG, "num-traces,n", "maximum number of traces to process" },
        { CL_FLAG, "ciphertext",   "use ciphertext rather than plaintext" },
        { CL_FLAG, "help,h",       "display this usage message" },
        { CL_TERM, 0, 0 }
    };

    cmdline cl(cmdline_args, usage_message);
    if (!cl.parse(argc, argv) || cl.count("help")) {
        cl.print_usage();
        return 1;
    }

    // parse each command line argument from the variable map
    string db_index = cl.get_str("db-index", "trace_index_file");
    string in_dir   = cl.get_str("input-dir", "trace_dir");
    string out_dir  = cl.get_str("output-dir", "trace_bin");
    string key_str  = cl.get_str("key", "");
    long time_min   = cl.get_long("time-min", 0);
    long time_max   = cl.get_long("time-max", 0);
    long num_traces = cl.get_long("num-traces", 0);
    bool ciphertext = cl.get_flag("ciphertext");

    // make sure that the input and output directories exist and are valid
    if (!util::check_inout_directories(in_dir, out_dir))
        return 1;

    tracelist traces;
    if (!parse_index_file(db_index, key_str, ciphertext, traces))
        return 1;

    return convert_traces(in_dir, out_dir, traces,
                          make_pair(time_min, time_max), num_traces);
}

