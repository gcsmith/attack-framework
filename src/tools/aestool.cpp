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
#include "aes.h"

using namespace std;

// -----------------------------------------------------------------------------
void print_key(uint8_t *key, bool dec)
{
    const char *fmt = dec ? "%d " : "%02X";
    for (int i = 0; i < 11; ++i) {
        printf("    round %02d  ", i);
        for (int j = 0; j < 16; ++j) printf(fmt, key[i * 16 + j]);
        printf("\n");
    }
}

// -----------------------------------------------------------------------------
void print_key_cpp(uint8_t *key, bool dec)
{
    const char *fmt = dec ? "%03d, " : "%02X, ";
    for (int i = 0; i < 11; ++i) {
        printf("    ");
        for (int j = 0; j < 16; ++j) printf(fmt, key[i * 16 + j]);
        printf("\n");
    }
}

// -----------------------------------------------------------------------------
void print_key_vhdl(uint8_t *key, bool dec)
{
}

// -----------------------------------------------------------------------------
void print_key_verilog(uint8_t *key, bool dec)
{
}

// -----------------------------------------------------------------------------
int main(int argc, char *argv[])
{
    // build and parse the table of command line arguments
    static const string usage_message = string(argv[0]) + " [options]";
    static const cmdline_option cmdline_args[] = {
        { CL_STR,  "key,k",        "specify the 16 byte AES128 key to expand" },
        { CL_STRV, "plaintext,p",  "specify a 16 byte plaintext block" },
        { CL_STRV, "ciphertext,c", "specify a 16 byte ciphertext block" },
        { CL_STR,  "mask,m",       "specify an 8-bit mask" },
        { CL_FLAG, "format-cpp",   "display data as C/C++ array" },
        { CL_FLAG, "format-vhdl",  "display data as VHDL array" },
        { CL_FLAG, "format-v",     "display data as Verilog array" },
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

    uint8_t sk[16 * 11], msg_in[16], msg_out[16];
    bool print_dec = cl.get_flag("dec");
    string ek = cl.get_str("key");

    // parse the 16 byte key and expand to 176 byte key schedule
    if (!util::atob(ek, sk, 16)) {
        fprintf(stderr, "invalid key specified. exiting\n");
        return 1;
    }

    aes::key_schedule(sk, 10);

    if (cl.get_flag("format-cpp"))
        print_key_cpp(sk, print_dec);
    else if (cl.get_flag("format-vhdl"))
        print_key_vhdl(sk, print_dec);
    else if (cl.get_flag("format-v"))
        print_key_verilog(sk, print_dec);
    else
        print_key(sk, print_dec);

    vector<uint8_t> mask(2);
    bool enable_mask = !!cl.count("mask");
    if (enable_mask && !util::atob(cl.get_str("mask"), &mask[0], 2)) {
        fprintf(stderr, "invalid mask specified. exiting\n");
        return 1;
    }

    foreach (const string &pt, cl.get_strv("plaintext")) {
        // parse the plaintext and perform encryption
        if (!util::atob(pt, msg_in, 16)) {
            fprintf(stderr, "invalid plaintext specified. exiting\n");
            return 1;
        }

        if (enable_mask)
            aes::encrypt_mask(msg_in, sk, msg_out, mask[0], mask[1]);
        else
            aes::encrypt(msg_in, sk, msg_out);

        // retreive and display the encrypted ciphertext and inputs
        string ct = util::btoa(msg_out, 16);
        printf("enc: pt=%s ek=%s ct=%s\n", pt.c_str(), ek.c_str(), ct.c_str());
    }

    foreach (const string &ct, cl.get_strv("ciphertext")) {
        // parse the ciphertext and perform decryption
        if (!util::atob(ct, msg_in, 16)) {
            fprintf(stderr, "invalid ciphertext specified. exiting\n");
            return 1;
        }

        if (enable_mask)
            aes::decrypt_mask(msg_in, sk, msg_out, mask[0], mask[1]);
        else
            aes::decrypt(msg_in, sk, msg_out);

        // retreive and display the decrypted plaintext and inputs
        string pt = util::btoa(msg_out, 16);
        printf("dec: ct=%s ek=%s pt=%s\n", ct.c_str(), ek.c_str(), pt.c_str());
    }

    return 0;
}

