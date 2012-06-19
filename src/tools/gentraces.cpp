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

#include <memory>
#include <string>
#include <cstdio>
#include <cstdlib>
#include "cmdline.h"
#include "trace_format.h"
#include "attack_manager.h"
#include "crypto.h"

using namespace std;

// -----------------------------------------------------------------------------
int main(int argc, char *argv[])
{
    // build and parse the table of command line arguments
    static const string usage_message = string(argv[0]) + " [options]";
    static const cmdline_option cmdline_args[] = {
        { CL_STR,  "dest-fmt,d",    "convert to destination format" },
        { CL_STR,  "output-path,o", "specify the output trace path" },
        { CL_STR,  "crypto,c",     "cryptographic function name" },
        { CL_STR,  "key,k",         "convert traces with the specified key" },
        { CL_LONG, "num-traces,n",  "maximum number of traces to process" },
        { CL_LONG, "num-samples,s", "number of samples per trace" },
        { CL_FLAG, "help,h",        "display this usage message" },
        { CL_TERM, 0, 0 }
    };

    cmdline cl(cmdline_args, usage_message);
    if (!cl.parse(argc, argv) || cl.count("help")) {
        cl.print_usage();
        return 1;
    }

    if (!cl.count("key")) {
        fprintf(stderr, "must specify an encryption key\n");
        return 1;
    }

    string dst_fmt   = cl.get_str("dest-fmt", "packed");
    string out_path  = cl.get_str("output-path", "experiment.bin");
    string crypto    = cl.get_str("crypto", "aes_hd_r0");
    string key_str   = cl.get_str("key");
    long num_traces  = cl.get_long("num-traces", 1000);
    long num_samples = cl.get_long("num-samples", 5000);

    trace::event_set events;
    for (long i = 0; i < num_samples; ++i) events.insert(i);

    // create the writer for the specified destination trace format
    auto_ptr<trace_writer> pWriter(trace_writer::create(dst_fmt));
    if (!pWriter.get() || !pWriter->open(out_path, key_str, events)) {
        fprintf(stderr, "failed to open trace writer\n");
        return 1;
    }

    auto_ptr<crypto_instance> crypt_inst(attack_manager::create_crypto(crypto));
    if (!crypt_inst.get()) {
        fprintf(stderr, "failed to create crypto instance\n");
        return 1;
    }

    const vector<uint8_t> key = util::atob(key_str);
    if (key.size() != (size_t)(crypt_inst->key_bits() >> 3)) {
        fprintf(stderr, "invalid key specified: %s\n", key_str.c_str());
        return 1;
    }
    crypt_inst->set_key(key);

    const size_t text_size = crypt_inst->key_bits() >> 3;
    const long corr_sample = rand() % num_samples;
    trace pt;

    for (long i = 0; i < num_traces; ++i) {
        // initialize the crypto instance with a randomized text block
        vector<uint8_t> text;
        for (size_t b = 0; b < text_size; ++b) 
            text.push_back(rand() % 256);
        crypt_inst->set_message(text);

        // compute the sensitive value for the known key
        int weight = 0;
        for (size_t b = 0; b < text_size; ++b) {
            const int k = crypt_inst->extract_estimate(b);
            weight += crypto::popcnt[crypt_inst->compute(b, k)];
        }

        pt.clear();
        pt.set_text(text);

        for (long s = 0; s < num_samples; ++s) {
            int power = rand() % 10;
            if (s == corr_sample)
                power += weight;
            pt.push_back(trace::sample(s, (float)power));
        }

        if (!pWriter->write(pt)) {
            fprintf(stderr, "failed to write trace %ld\n", i);
            return 1;
        }
    }

    pWriter->close();
    return 0;
}

