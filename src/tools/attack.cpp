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
        { CL_STR,  "src-fmt,s",    "specify the trace source format" },
        { CL_STR,  "input-path,i", "specify the trace input path" },
        { CL_STR,  "output-dir,o", "specify the results output directory" },
        { CL_STR,  "attack,a",     "attack algorithm name" },
        { CL_STR,  "crypto,c",     "cryptographic function name" },
        { CL_LONG, "num-traces,n", "maximum number of traces to process" },
        { CL_FLAG, "ciphertext",   "use ciphertext rather than plaintext" },
        { CL_STR,  "params,p",     "specify attack specific parameters" },
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

    // parse each command line argument from the variable map
    if (cl.count("list")) {
        attack_manager::list(stdout);
        return 0;
    }

    if (!cl.count("attack") || !cl.count("crypto")) {
        fprintf(stderr, "must specify an attack method and crypto type\n");
        return 1;
    }

    // parse each command line argument from the variable map
    string src_fmt  = cl.get_str("src-fmt", "packed");
    string in_path  = cl.get_str("input-path", "trace_input_path");
    string out_path = cl.get_str("output-dir", "attack_results");
    string profile  = cl.get_str("profile", "timing_profile.txt");
    int num_threads = cl.get_long("threads", 1);
    int num_traces  = cl.get_long("num-traces", 0);
    int report_int  = cl.get_long("report", 1000);
    bool ciphertext = cl.get_flag("ciphertext");

    // allocate the reader object given the specified trace input format
    auto_ptr<trace_reader> pReader(trace_reader::create(src_fmt));
    if (!pReader.get() || !pReader->open(in_path, "", ciphertext))
        return 1;

    // initialize and run the attack engine
    attack_engine engine;
    engine.set_reader(pReader.get());
    engine.set_attack(cl.get_str("attack"));
    engine.set_crypto(cl.get_str("crypto"));
    engine.set_params(cl.get_str("params"));
    engine.set_results_prefix(cl.get_str("prefix"));
    engine.set_thread_count(num_threads);
    engine.set_num_traces(num_traces);
    engine.set_report_interval(report_int);

    if (!engine.load_trace_profile(profile)) {
        fprintf(stderr, "failed to load timing profile\n");
        return 1;
    }

    return engine.run(out_path);
}

