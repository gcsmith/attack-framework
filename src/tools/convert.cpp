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
#include <memory>
#include <string>
#include <cstdio>
#include "cmdline.h"
#include "trace_format.h"
#include "utility.h"

using namespace std;

// -----------------------------------------------------------------------------
int convert_traces(trace_reader *pReader, trace_writer *pWriter,
                   const trace::time_range &range, size_t max_trace,
                   const string profile_path)
{
    BENCHMARK_DECLARE(trace_convert);

    // determine the total number of traces to convert
    size_t trace_count = pReader->trace_count();
    if (max_trace && max_trace < trace_count) {
        printf("only processing %zu of %zu traces\n", max_trace, trace_count);
        trace_count = max_trace;
    }

    // read in each trace in source format and output in destination format
    trace pt;
    for (size_t i = 0; i < trace_count; ++i) {
        if (!pReader->read(pt, range)) {
            fprintf(stderr, "failed to read trace %zu\n", i);
            return 1;
        }
        if (!pWriter->write(pt)) {
            fprintf(stderr, "failed to write trace %zu\n", i);
            return 1;
        }

        const string text(util::btoa(pt.text()));
        printf("converted %s [%zu/%zu]\n", text.c_str(), i + 1, trace_count);
        BENCHMARK_SAMPLE_WHEN(trace_convert, i && !(i % 100));
    }

    // write out the timing profile and close the reader/writer objects
    trace::write_profile(profile_path, pReader->events());
    pReader->close();
    pWriter->close();
    return 0;
}

// -----------------------------------------------------------------------------
int main(int argc, char *argv[])
{
    // build and parse the table of command line arguments
    static const string usage_message = string(argv[0]) + " [options]";
    static const cmdline_option cmdline_args[] = {
        { CL_STR,  "src-fmt,s",     "convert from source format" },
        { CL_STR,  "dest-fmt,d",    "convert to destination format" },
        { CL_STR,  "input-path,i",  "specify the input trace path" },
        { CL_STR,  "output-path,o", "specify the output trace path" },
        { CL_STR,  "profile,P",     "specify the timing profile path" },
        { CL_STR,  "key,k",         "convert traces with the specified key" },
        { CL_LONG, "min-time,m",    "earliest sample event time to convert" },
        { CL_LONG, "max-time,M",    "latest sample event time to convert" },
        { CL_LONG, "num-traces,n",  "maximum number of traces to process" },
        { CL_FLAG, "ciphertext",    "use ciphertext rather than plaintext" },
        { CL_FLAG, "help,h",        "display this usage message" },
        { CL_TERM, 0, 0 }
    };

    cmdline cl(cmdline_args, usage_message);
    if (!cl.parse(argc, argv) || cl.count("help")) {
        cl.print_usage();
        return 1;
    }

    // parse each command line argument from the variable map
    string src_fmt  = cl.get_str("src-fmt", "packed");
    string dst_fmt  = cl.get_str("dest-fmt", "sqlite");
    string in_path  = cl.get_str("input-path", "trace_input_path");
    string out_path = cl.get_str("output-path", "trace_output_path");
    string profile  = cl.get_str("profile", "timing_profile.txt");
    string key      = cl.get_str("key");
    long time_min   = cl.get_long("time-min");
    long time_max   = cl.get_long("time-max");
    long num_traces = cl.get_long("num-traces");
    bool ciphertext = cl.get_flag("ciphertext");

    // create reader and writer for specified source/destination trace formats
    auto_ptr<trace_reader> rd(trace_reader::create(src_fmt));
    if (!rd.get() || !rd->open(in_path, key, ciphertext))
        return 1;

    auto_ptr<trace_writer> wr(trace_writer::create(dst_fmt));
    if (!wr.get() || !wr->open(out_path, key))
        return 1;

    const trace::time_range range(time_min, time_max);
    return convert_traces(rd.get(), wr.get(), range, num_traces, profile);
}

