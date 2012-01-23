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
#include <stdio.h>
#include "cmdline.h"
#include "trace.h"
#include "utility.h"

using namespace std;

// -----------------------------------------------------------------------------
int main(int argc, char *argv[])
{
    // build and parse the table of command line arguments
    static const string usage_message = string(argv[0]) + " [options]";
    static const cmdline_option cmdline_args[] = {
        { CL_STR,  "input-dir,i", "specify the input trace directory path" },
        { CL_STR,  "output,o",    "specify the output timing profile path" },
        { CL_FLAG, "help,h",      "display this usage message" },
        { CL_TERM, 0, 0 }
    };

    cmdline cl(cmdline_args, usage_message);
    if (!cl.parse(argc, argv) || cl.count("help")) {
        cl.print_usage();
        return 1;
    }

    // parse each command line argument from the variable map
    string in_dir = cl.get_str("input-dir", "trace_out");
    string output = cl.get_str("output", "timing_profile.txt");

    util::pathlist paths;
    util::scan_directory(in_dir, ".bin", paths);

    event_set sample_times;
    trace pt(4096);

    for (size_t i = 0; i < paths.size(); ++i) {
        pt.read_bin(paths[i]);
        pt.merge_events(sample_times);
        printf("processed %s [%ld/%ld]\n", paths[i].c_str(), i+1, paths.size());
    }

    trace::write_profile(output, sample_times);
    return 0;
}

