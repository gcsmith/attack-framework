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
#include "des.h"

using namespace std;

inline uint64_t atob_be(const string &str)
{
    uint8_t data[8];
    if (!util::atob(str, data, 8)) return 0;
    return util::convert_bytes(data);
}

// -----------------------------------------------------------------------------
void print_keys(uint64_t *sk, bool dec)
{
    const char *fmt = dec ? "%02d " : "%02X ";

    // iterate overy each of the 16 round subkeys
    for (int i = 0; i < 16; ++i) {
        uint64_t key = util::rev64(sk[i]) >> 16;
        printf("    round %02d  %012llX  ", i, (long long )key);

        // iterate over each of the 8 six bit key partitions
        for (int j = 7; j >= 0; --j) printf(fmt, (key >> (j * 6)) & 0x3F);
        printf("\n");
    }
}

// -----------------------------------------------------------------------------
int main(int argc, char *argv[])
{
    // build and parse the table of command line arguments
    static const string usage_message = string(argv[0]) + " [options]";
    static const cmdline_option cmdline_args[] = {
        { CL_STR,  "key,k",        "specify the 16 byte DES key to expand" },
        { CL_STRV, "plaintext,p",  "specify a 16 byte plaintext block" },
        { CL_STRV, "ciphertext,c", "specify a 16 byte ciphertext block" },
        { CL_FLAG, "dec",          "display data in base 10" },
        { CL_FLAG, "help,h",       "display this usage message" },
        { CL_TERM, 0, 0 }
    };

    cmdline cl(cmdline_args, usage_message);
    if (!cl.parse(argc, argv) || cl.count("help")) {
        cl.print_usage();
        return 1;
    }

    // parse each command line argument from the variable map
    if (!cl.count("key")) {
        fprintf(stderr, "no key specified -- nothing to do. exiting\n");
        return 1;
    }

    uint64_t sk[16];
    uint64_t ek = atob_be(cl.get_str("key"));
    des::key_schedule(ek, sk);
    print_keys(sk, cl.get_flag("dec"));

    for (const string &plaintext : cl.get_strv("plaintext")) {
        // parse the plaintext and perform encryption
        uint64_t pt = atob_be(plaintext);
        uint64_t ct = des::encrypt(pt, sk);

        // retreive and display the encrypted ciphertext and inputs
        printf("enc: pt=%016llX ek=%016llX ct=%016llX\n",
               (long long)util::rev64(pt), (long long)util::rev64(ek),
               (long long)util::rev64(ct));
    }

    for (const string &ciphertext : cl.get_strv("ciphertext")) {
        // parse the ciphertext and perform decryption
        uint64_t ct = atob_be(ciphertext);
        uint64_t pt = des::decrypt(ct, sk);

        // retreive and display the decrypted plaintext and inputs
        printf("dec: ct=%016llX ek=%016llX pt=%016llX\n",
               (long long)util::rev64(ct), (long long)util::rev64(ek),
               (long long)util::rev64(pt));
    }

    return 0;
}

