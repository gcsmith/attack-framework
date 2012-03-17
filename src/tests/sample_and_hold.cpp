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
#include <vector>
#include <cstdio>
#include "cmdline.h"
#include "trace.h"

using namespace std;

// -----------------------------------------------------------------------------
int main(int argc, char *argv[])
{
    // build and parse the table of command line arguments
    static const string usage_message = string(argv[0]) + " [options]";
    static const cmdline_option cmdline_args[] = {
        { CL_STR,  "input-trace,i", "input trace path" },
        { CL_STR,  "profile,p",     "trace profile path" },
        { CL_FLAG, "help,h",        "display this usage message" },
        { CL_TERM, 0, 0 }
    };

    cmdline cl(cmdline_args, usage_message);
    if (!cl.parse(argc, argv) || cl.count("help")) {
        cl.print_usage();
        return 1;
    }

    if (!cl.count("input-trace")) {
        fprintf(stderr, "must specify a trace file\n");
        return 1;
    }

    if (!cl.count("profile")) {
        fprintf(stderr, "must specify a trace profile\n");
        return 1;
    }

#if 0
    // parse each command line argument from the variable map
    string trace_path   = cl.get_str("input-trace");
    string profile_path = cl.get_str("profile");

    trace pt;
    if (!pt.read_bin(trace_path)) {
        fprintf(stderr, "failed to load trace '%s'\n", trace_path.c_str());
        return 1;
    }

    vector<long> times;
    if (!trace::read_profile(profile_path, times)) {
        fprintf(stderr, "failed to load profile '%s'\n", profile_path.c_str());
        return 1;
    }

    trace pt_sh;
    pt_sh.sample_and_hold(pt, times);

    pt.write_csv("sh_original.csv");
    pt_sh.write_csv("sh_expanded.csv");

    printf("expanded trace from %zd to %zd samples\n", pt.size(), pt_sh.size());
#endif

    return 0;
}

