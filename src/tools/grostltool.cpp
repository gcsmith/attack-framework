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
        { CL_STRV, "input,i",   "specify the message to hash" },
        { CL_FLAG, "dec",       "display data in base 10" },
        { CL_FLAG, "help,h",    "display this usage message" },
        { CL_TERM, 0, 0 }
    };

    cmdline cl(cmdline_args, usage_message);
    if (!cl.parse(argc, argv) || cl.count("help")) {
        cl.print_usage();
        return 1;
    }

    foreach (const string &msg, cl.get_strv("input")) {
        // convert the hex string to a byte sequence and perform the hash
        vector<uint8_t> out, in = util::atob(msg);
        grostl::hash(in, out);

        // display both the input message and the output digest
        printf("message: %s\n", msg.c_str());
        printf("digest:  %s\n", util::btoa(out).c_str());
    }

    return 0;
}

