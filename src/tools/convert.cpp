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
int convert_traces(trace_reader *pReader, trace_writer *pWriter)
{
    // determine the total number of traces to convert
    size_t trace_count = pReader->trace_count();

    // read in each trace in source format and output in destination format
    trace pt;
    for (size_t i = 0; i < trace_count; ++i) {
        if (!pReader->read(pt)) {
            fprintf(stderr, "failed to read trace %zu\n", i + 1);
            return 1;
        }
        if (!pWriter->write(pt)) {
            fprintf(stderr, "failed to write trace %zu\n", i + 1);
            return 1;
        }

        const string text(util::btoa(pt.text()));
        printf("converted %s [%zu/%zu]\r", text.c_str(), i + 1, trace_count);
    }

    printf("\nsuccessfully converted traces...\n");

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
        { CL_STR,  "key,k",         "convert traces with the specified key" },
        { CL_LONG, "min-time,m",    "earliest sample event time to convert" },
        { CL_LONG, "max-time,M",    "latest sample event time to convert" },
        { CL_LONG, "num-traces,n",  "maximum number of traces to process" },
        { CL_FLAG, "ciphertext",    "use ciphertext rather than plaintext" },
        { CL_FLAG, "summary",       "summarize the input trace directory" },
        { CL_FLAG, "help,h",        "display this usage message" },
        { CL_TERM, 0, 0 }
    };

    cmdline cl(cmdline_args, usage_message);
    if (!cl.parse(argc, argv) || cl.count("help")) {
        cl.print_usage();
        return 1;
    }

    // determine the input trace file format, either specified or guessed
    const string input_path = cl.get_str("input-path", "trace_input_path");
    string src_fmt = cl.get_str("src-fmt", "auto");

    if (src_fmt == "auto") {
        src_fmt = trace_reader::guess_format(input_path);
        printf("guessing trace src-format: %s\n", src_fmt.c_str());
    }

    // determine the output trace file format, either specified or guessed
    const string output_path = cl.get_str("output-path", "trace_output_path");
    string dst_fmt = cl.get_str("dst-fmt", "auto");

    if (dst_fmt == "auto") {
        dst_fmt = trace_reader::guess_format(output_path);
        printf("guessing trace dst-format: %s\n", dst_fmt.c_str());
    }

    // create reader and writer for specified source/destination trace formats
    trace_reader::options opt;
    opt.min_time   = cl.get_long("min-time");
    opt.max_time   = cl.get_long("max-time");
    opt.num_traces = cl.get_long("num-traces");
    opt.ciphertext = cl.get_flag("ciphertext");
    opt.key        = cl.get_str("key");

    auto_ptr<trace_reader> rd(trace_reader::create(src_fmt));
    if (!rd.get()) return 1;

    // describe the input directory and exit when --summary is specified
    if (cl.get_flag("summary") || !rd->open(input_path, opt)) {
        rd->summary(input_path);
        return 0;
    }

    auto_ptr<trace_writer> wr(trace_writer::create(dst_fmt));
    if (!wr.get() || !wr->open(output_path, opt.key, rd->events()))
        return 1;

    return convert_traces(rd.get(), wr.get());
}

