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
#include "attack_manager.h"
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
        { CL_STR,  "params,p",     "specify attack specific parameters" },
        { CL_LONG, "report,r",     "generate report every N traces" },
        { CL_FLAG, "ciphertext",   "use ciphertext rather than plaintext" },
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
        attack_manager::list_attack(stdout);
        attack_manager::list_crypto(stdout);
        return 0;
    }

    // determine the input trace file format, either specified or guessed
    const string input_path = cl.get_str("input-path", "trace_input_path");
    string src_fmt = cl.get_str("src-fmt", "auto");

    if (src_fmt == "auto") {
        src_fmt = trace_reader::guess_format(input_path);
        printf("guessing trace src-format: %s\n", src_fmt.c_str());
    }

    trace_reader::options reader_opt;
    reader_opt.min_time    = 0;
    reader_opt.max_time    = 0;
    reader_opt.num_traces  = cl.get_long("num-traces", 0); // 0 is maximum
    reader_opt.ciphertext  = cl.get_flag("ciphertext");
    reader_opt.key         = "";

    attack_engine::options engine_opt;
    engine_opt.attack_name = cl.get_str("attack",     "cpa");
    engine_opt.crypto_name = cl.get_str("crypto",     "aes_hd_r0");
    engine_opt.parameters  = cl.get_str("params",     "byte=0,bits=8,offset=0");
    engine_opt.result_path = cl.get_str("output-dir", "attack_results");
    engine_opt.prefix      = cl.get_str("prefix",     "output");
    engine_opt.num_threads = cl.get_long("threads",   1);
    engine_opt.report_tick = cl.get_long("report",    0);

    // allocate the reader object given the specified trace input format
    auto_ptr<trace_reader> pReader(trace_reader::create(src_fmt));
    if (!pReader.get() || !pReader->open(input_path, reader_opt))
        return 1;

    // initialize and run the attack engine
    attack_engine engine;
    return engine.run(engine_opt, pReader.get());
}

