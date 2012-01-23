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

#include "attack_engine.h"
#include "cmdline.h"

using namespace std;

// -----------------------------------------------------------------------------
int main(int argc, char *argv[])
{
    // build and parse the table of command line arguments
    static const string usage_message = "attack [options]";
    static const cmdline_option cmdline_args[] = {
        { CL_STR,  "input-dir,i",  "specify the trace input directory" },
        { CL_STR,  "output-dir,o", "specify the results output directory" },
        { CL_STR,  "attack,a",     "attack algorithm name" },
        { CL_STR,  "crypto,c",     "cryptographic function name" },
        { CL_LONG, "num-traces,t", "maximum number of traces to process" },
        { CL_STR,  "params,p",     "specify attack specific parameters" },
        { CL_STR,  "order,O",      "specify a trace order file" },
        { CL_STR,  "profile,P",    "specify the timing profile" },
        { CL_LONG, "report,r",     "generate report every N traces" },
        { CL_LONG, "threads",      "number of worker threads to run" },
        { CL_STR,  "prefix",       "specify prefix for result files" },
        { CL_FLAG, "list",         "print a list of attack algorithms" },
        { CL_FLAG, "help,h",       "display this usage message" },
        { CL_FLAG, "version,V",    "display the program version" },
        { CL_TERM, 0, 0 }
    };

    cmdline cl(cmdline_args, usage_message);
    if (!cl.parse(argc, argv) || cl.count("help")) {
        cl.print_usage();
        return 1;
    }

    attack_engine at;

    // parse each command line argument from the variable map
    if (cl.count("list")) {
        attack_manager::list(stdout);
        return 0;
    }

    if (!at.set_paths(cl.get_str("input-dir", "trace_bin"),
                      cl.get_str("output-dir", "attack_results")))
        return 1;

    if (cl.count("attack"))
        at.set_attack(cl.get_str("attack"));
    if (cl.count("crypto"))
        at.set_crypto(cl.get_str("crypto"));
    if (cl.count("params"))
        at.set_params(cl.get_str("params"));
    if (cl.count("prefix"))
        at.set_results_prefix(cl.get_str("prefix"));
    if (cl.count("threads"))
        at.set_thread_count(cl.get_long("threads"));

    at.set_num_traces(cl.get_long("num-traces", 0));
    at.set_report_interval(cl.get_long("report", 1000));
    at.load_trace_order(cl.get_str("order", "trace_order.txt"));
    at.load_trace_profile(cl.get_str("profile", "trace_profile.txt"));

    return at.run();
}

