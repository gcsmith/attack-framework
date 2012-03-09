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
#include <memory>
#include <cstdio>
#include "cmdline.h"
#include "trace_format.h"
#include "utility.h"

using namespace std;

// -----------------------------------------------------------------------------
int main(int argc, char *argv[])
{
    // build and parse the table of command line arguments
    static const string usage_message = string(argv[0]) + " [options]";
    static const cmdline_option cmdline_args[] = {
        { CL_STR,  "src-fmt,s",     "specify trace source format" },
        { CL_STR,  "input-path,i",  "specify the input trace directory path" },
        { CL_STR,  "output-path,o", "specify the output timing profile path" },
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
    string in_path  = cl.get_str("input-path", "trace_out");
    string out_path = cl.get_str("output-path", "timing_profile.txt");

    // create reader and writer for specified source/destination trace formats
    auto_ptr<trace_reader> pReader(trace_reader::create(src_fmt));
    if (!pReader.get() || !pReader->open(in_path, "", false))
        return 1;

    // process each input trace and build the complete set of sample events
    trace pt;
    trace::time_range range(0, 0);
    const size_t trace_count = pReader->trace_count();

    for (size_t i = 0; i < trace_count; ++i) {
        if (!pReader->read(pt, range)) {
            fprintf(stderr, "failed to read trace %zu\n", i);
            return 1;
        }
        const string text(util::btoa(pt.text()));
        printf("processed %s [%zu/%zu]\n", text.c_str(), i + 1, trace_count);
    }

    // write out the timing profile and close the reader/writer objects
    trace::write_profile(out_path, pReader->events());
    return 0;
}

