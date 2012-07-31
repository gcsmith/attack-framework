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
#include <algorithm>
#include <cstdio>
#include "cmdline.h"
#include "utility.h"
#include "aes.h"

using namespace std;

// -----------------------------------------------------------------------------
void print_masked_sbox(uint8_t mask, bool decimal)
{
    int masked_sbox[256];
    aes::mask_sbox(aes::sbox, masked_sbox, mask, mask);

    printf("const logic [7:0] sbox_%02X[0:255] = '{", mask);
    for (int i = 0; i < 255; ++i) {
        if (0 == i % 16) printf("\n  ");
        printf("'h%02X, ", masked_sbox[i]);
    }
    printf("'h%02X\n};\n", masked_sbox[255]);
}

// -----------------------------------------------------------------------------
int main(int argc, char *argv[])
{
    // build and parse the table of command line arguments
    static const string usage_message = string(argv[0]) + " [options]";
    static const cmdline_option cmdline_args[] = {
        { CL_STRV, "mask,m",       "display sbox with the specified mask" },
        { CL_LONG, "rand-masks,r", "number of random masked sboxes to print" },
        { CL_FLAG, "sbox",         "display unmodified rijndael sbox" },
        { CL_FLAG, "dec",          "display data in base 10" },
        { CL_FLAG, "help,h",       "display this usage message" },
        { CL_TERM, 0, 0 }
    };

    cmdline cl(cmdline_args, usage_message);
    if (!cl.parse(argc, argv) || cl.count("help")) {
        cl.print_usage();
        return 1;
    }

    srand(time(0));

    // print out the sbox for a given mask
    for (const string &mask_str : cl.get_strv("mask")) {
        uint8_t mask = 0;
        if (!util::atob(mask_str, &mask, 1)) {
            fprintf(stderr, "mask mask: %s\n", mask_str.c_str());
            continue;
        }
        print_masked_sbox(mask, cl.get_flag("dec"));
    }

    // print out the specified number of randomly masked sboxes
    vector<uint8_t> unique_random;
    for (int i = 0; i < 256; i++) unique_random.push_back(i);
    random_shuffle(unique_random.begin(), unique_random.end());

    const int num_random_masks = min(256L, cl.get_long("rand-masks"));
    for (int i = 0; i < num_random_masks; i++)
        print_masked_sbox(unique_random[i], cl.get_flag("dec"));

    return 0;
}

