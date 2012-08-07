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
#include <cstdio>
#include "cmdline.h"
#include "utility.h"
#include "grostl.h"

using namespace std;

// -----------------------------------------------------------------------------
int main(int argc, char *argv[])
{
    // build and parse the table of command line arguments
    static const string usage_message = string(argv[0]) + " [options]";
    static const cmdline_option cmdline_args[] = {
        { CL_STRV, "ascii-msg,a", "specify the message to hash as ASCII" },
        { CL_STRV, "hex-msg,h",   "specify the message to hash as hex number" },
        { CL_STRV, "compress,c",  "specify the message block to compress" },
        { CL_STRV, "p-permute,p", "specify the message block to p-permute" },
        { CL_STRV, "q-permute,q", "specify the message block to q-permute" },
        { CL_FLAG, "dec",         "display data in base 10" },
        { CL_FLAG, "help,h",      "display this usage message" },
        { CL_TERM, 0, 0 }
    };

    cmdline cl(cmdline_args, usage_message);
    if (!cl.parse(argc, argv) || cl.count("help")) {
        cl.print_usage();
        return 1;
    }

    foreach (const string &msg, cl.get_strv("ascii-msg")) {
        // convert the ASCII string to a byte sequence and perform the hash
        vector<uint8_t> out, in(msg.begin(), msg.end());
        grostl::hash(in, out);

        // display both the input message and the output digest
        printf("message: %s\n", util::btoa(in).c_str());
        printf("digest:  %s\n", util::btoa(out).c_str());
    }

    foreach (const string &msg, cl.get_strv("hex-msg")) {
        // convert the hex string to a byte sequence and perform the hash
        vector<uint8_t> out, in = util::atob(msg);
        grostl::hash(in, out);

        // display both the input message and the output digest
        printf("message: %s\n", util::btoa(in).c_str());
        printf("digest:  %s\n", util::btoa(out).c_str());
    }

    foreach (const string &msg, cl.get_strv("compress")) {
        // convert the hex string to a byte sequence and perform the permutation
        vector<uint8_t> out(64), in = util::atob(msg);
        if (128 != in.size()) {
            fprintf(stderr, "incorrect state size (%zu)\n", in.size());
            continue;
        }
        grostl::compress(&in[0], &in[64], &out[0]);

        // display both the input state and the permuted output state
        printf("m/h_in     %s\n", util::btoa(in).c_str());
        printf("F(state):  %s\n", util::btoa(out).c_str());
    }

    foreach (const string &msg, cl.get_strv("p-permute")) {
        // convert the hex string to a byte sequence and perform the permutation
        vector<uint8_t> out(64), in = util::atob(msg);
        if (64 != in.size()) {
            fprintf(stderr, "incorrect state size (%zu)\n", in.size());
            continue;
        }
        grostl::permute_p(&in[0], &out[0]);

        // display both the input state and the permuted output state
        printf("state:     %s\n", util::btoa(in).c_str());
        printf("P(state):  %s\n", util::btoa(out).c_str());
    }

    foreach (const string &msg, cl.get_strv("q-permute")) {
        // convert the hex string to a byte sequence and perform the permutation
        vector<uint8_t> out(64), in = util::atob(msg);
        if (64 != in.size()) {
            fprintf(stderr, "incorrect state size (%zu)\n", in.size());
            continue;
        }
        grostl::permute_q(&in[0], &out[0]);

        // display both the input state and the permuted output state
        printf("state:     %s\n", util::btoa(in).c_str());
        printf("Q(state):  %s\n", util::btoa(out).c_str());
    }

    return 0;
}

