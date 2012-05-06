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

#include <vector>
#include <cstdio>
#include <cstring>
#include "aes.h"
#include "crypto.h"
#include "grostl.h"
#include "utility.h"

using namespace std;

namespace grostl {

int shift_p[64] = {
     0,  9, 18, 27, 36, 45, 54, 63,  8, 17, 26, 35, 44, 53, 62,  7,
    16, 25, 34, 43, 52, 61,  6, 15, 24, 33, 42, 51, 60,  5, 14, 23,
    32, 41, 50, 59,  4, 13, 22, 31, 40, 49, 58,  3, 12, 21, 30, 39,
    48, 57,  2, 11, 20, 29, 38, 47, 56,  1, 10, 19, 28, 37, 46, 55,
};

int shift_q[64] = {
     8, 25, 42, 59,  4, 21, 38, 55, 16, 33, 50,  3, 12, 29, 46, 63,
    24, 41, 58, 11, 20, 37, 54,  7, 32, 49,  2, 19, 28, 45, 62, 15,
    40, 57, 10, 27, 36, 53,  6, 23, 48,  1, 18, 35, 44, 61, 14, 31,
    56,  9, 26, 43, 52,  5, 22, 39,  0, 17, 34, 51, 60, 13, 30, 47,
};

int shift_inv_p[64] = {
     0, 57, 50, 43, 36, 29, 22, 15,  8,  1, 58, 51, 44, 37, 30, 23,
    16,  9,  2, 59, 52, 45, 38, 31, 24, 17, 10,  3, 60, 53, 46, 39,
    32, 25, 18, 11,  4, 61, 54, 47, 40, 33, 26, 19, 12,  5, 62, 55,
    48, 41, 34, 27, 20, 13,  6, 63, 56, 49, 42, 35, 28, 21, 14,  7,
};

int shift_inv_q[64] = {
    56, 41, 26, 11,  4, 53, 38, 23,  0, 49, 34, 19, 12, 61, 46, 31,
     8, 57, 42, 27, 20,  5, 54, 39, 16,  1, 50, 35, 28, 13, 62, 47,
    24,  9, 58, 43, 36, 21,  6, 55, 32, 17,  2, 51, 44, 29, 14, 63,
    40, 25, 10, 59, 52, 37, 22,  7, 48, 33, 18,  3, 60, 45, 30, 15,
};

// -----------------------------------------------------------------------------
// Perform the AddRoundConstant step for the P permutation
void add_round_const_p(const uint8_t *in, int round, uint8_t *out)
{
    for (int i = 0; i < 64; ++i)
        out[i] = (i & 7) ? in[i] : in[i] ^ ((i << 1) ^ round);
}

// -----------------------------------------------------------------------------
// Perform the AddRoundConstant step for the Q permutation
void add_round_const_q(const uint8_t *in, int round, uint8_t *out)
{
    for (int i = 0; i < 64; ++i)
        out[i] = ((~i & 7) ? in[i] : in[i] ^ (((i & ~7) << 1) ^ round)) ^ 0xFF;
}

// -----------------------------------------------------------------------------
// Perform the SubBytes substitution step
void sub_bytes(const uint8_t *in, uint8_t *out)
{
    for (int i = 0; i < 64; ++i)
        out[i] = aes::sbox[in[i]];
}

// -----------------------------------------------------------------------------
void sub_bytes_masked(const int tab[64][256], const uint8_t *in, uint8_t *out)
{
    for (int i = 0; i < 64; ++i)
        out[i] = tab[i][in[i]];
}

// -----------------------------------------------------------------------------
// Perform the ShiftBytes step for the P permutation
void shift_bytes_p(const uint8_t *in, uint8_t *out)
{
    for (int i = 0; i < 64; ++i)
        out[i] = in[shift_p[i]];
}

// -----------------------------------------------------------------------------
// Perform the ShiftBytes step for the Q permutation
void shift_bytes_q(const uint8_t *in, uint8_t *out)
{
    for (int i = 0; i < 64; ++i)
        out[i] = in[shift_q[i]];
}

// -----------------------------------------------------------------------------
void mix_bytes(const uint8_t *in, uint8_t *out)
{
    for (int i = 0; i < 8; ++i, in += 8, out += 8) {
        out[0] = GF_2(in[0]) ^ GF_2(in[1]) ^ GF_3(in[2]) ^ GF_4(in[3]) ^
                 GF_5(in[4]) ^ GF_3(in[5]) ^ GF_5(in[6]) ^ GF_7(in[7]);
        out[1] = GF_7(in[0]) ^ GF_2(in[1]) ^ GF_2(in[2]) ^ GF_3(in[3]) ^
                 GF_4(in[4]) ^ GF_5(in[5]) ^ GF_3(in[6]) ^ GF_5(in[7]);
        out[2] = GF_5(in[0]) ^ GF_7(in[1]) ^ GF_2(in[2]) ^ GF_2(in[3]) ^
                 GF_3(in[4]) ^ GF_4(in[5]) ^ GF_5(in[6]) ^ GF_3(in[7]);
        out[3] = GF_3(in[0]) ^ GF_5(in[1]) ^ GF_7(in[2]) ^ GF_2(in[3]) ^
                 GF_2(in[4]) ^ GF_3(in[5]) ^ GF_4(in[6]) ^ GF_5(in[7]);
        out[4] = GF_5(in[0]) ^ GF_3(in[1]) ^ GF_5(in[2]) ^ GF_7(in[3]) ^
                 GF_2(in[4]) ^ GF_2(in[5]) ^ GF_3(in[6]) ^ GF_4(in[7]);
        out[5] = GF_4(in[0]) ^ GF_5(in[1]) ^ GF_3(in[2]) ^ GF_5(in[3]) ^
                 GF_7(in[4]) ^ GF_2(in[5]) ^ GF_2(in[6]) ^ GF_3(in[7]);
        out[6] = GF_3(in[0]) ^ GF_4(in[1]) ^ GF_5(in[2]) ^ GF_3(in[3]) ^
                 GF_5(in[4]) ^ GF_7(in[5]) ^ GF_2(in[6]) ^ GF_2(in[7]);
        out[7] = GF_2(in[0]) ^ GF_3(in[1]) ^ GF_4(in[2]) ^ GF_5(in[3]) ^
                 GF_3(in[4]) ^ GF_5(in[5]) ^ GF_7(in[6]) ^ GF_2(in[7]);
    }
}

// -----------------------------------------------------------------------------
void xor_bytes(const uint8_t *in, uint8_t *out, const uint8_t *mask)
{
    for (int i = 0; i < 64; ++i)
        out[i] = in[i] ^ mask[i];
}

// -----------------------------------------------------------------------------
void permute_p(const uint8_t *in, uint8_t *out)
{
    uint8_t temp[64];
    memcpy(out, in, 64);

    for (int round = 0; round < 10; ++round) {
        add_round_const_p(out, round, temp);
        printf("[P:%x] RC: %s\n", round, util::btoa(temp, 64).c_str());
        sub_bytes(temp, out);
        printf("[P:%x] SB: %s\n", round, util::btoa(out, 64).c_str());
        shift_bytes_p(out, temp);
        printf("[P:%x] SH: %s\n", round, util::btoa(temp, 64).c_str());
        mix_bytes(temp, out);
        printf("[P:%x] MB: %s\n\n", round, util::btoa(out, 64).c_str());
    }
}

// -----------------------------------------------------------------------------
void permute_q(const uint8_t *in, uint8_t *out)
{
    uint8_t temp[64];
    memcpy(out, in, 64);

    for (int round = 0; round < 10; ++round) {
        add_round_const_q(out, round, temp);
        printf("[Q:%x] RC: %s\n", round, util::btoa(temp, 64).c_str());
        sub_bytes(temp, out);
        printf("[Q:%x] SB: %s\n", round, util::btoa(out, 64).c_str());
        shift_bytes_q(out, temp);
        printf("[Q:%x] SH: %s\n", round, util::btoa(temp, 64).c_str());
        mix_bytes(temp, out);
        printf("[Q:%x] MB: %s\n\n", round, util::btoa(out, 64).c_str());
    }
}

// -----------------------------------------------------------------------------
void compress(const uint8_t *msg, const uint8_t *chain, uint8_t *out)
{
    uint8_t hm[64], p[64], q[64];
    for (int i = 0; i < 64; ++i)
        hm[i] = msg[i] ^ chain[i];

    permute_p(hm, p);
    permute_q(msg, q);

    for (int i = 0; i < 64; ++i)
        out[i] = chain[i] ^ p[i] ^ q[i];
}

// -----------------------------------------------------------------------------
void compress(const uint8_t *msg, const uint8_t *chain, uint8_t *out,
              const uint8_t *imask, const uint8_t *omask)
{
    int mbox[64][256];
    uint8_t msg_mask[64], hm_mask[64], p[64], q[64], x2[64], x3[64], x4[64];

    // compute the masked sbox for each byte
    for (int i = 0; i < 64; i++)
        aes::mask_sbox(aes::sbox, mbox[i], imask[i], omask[i]);

    // apply input mask to msg, then compute msg ^ chain
    xor_bytes(msg, msg_mask, imask);
    xor_bytes(chain, hm_mask, msg_mask);

    // compute intermediate masks, then perform p-permutation
    shift_bytes_p(omask, x2);
    mix_bytes(x2, x3);
    xor_bytes(x3, x4, imask);

    memcpy(p, hm_mask, 64);
    for (int round = 0; round < 10; ++round) {
        add_round_const_p(p, round, out);       // -> state ^ x0
        sub_bytes_masked(mbox, out, p);         // -> state ^ x1
        shift_bytes_p(p, out);                  // -> state ^ x2
        mix_bytes(out, p);                      // -> state ^ x3
        xor_bytes(p, p, x4);                    // -> state ^ x0
    }

    // compute intermediate masks, then perform q-permutation
    shift_bytes_q(omask, x2);
    mix_bytes(x2, x3);
    xor_bytes(x3, x4, imask);

    memcpy(q, msg_mask, 64);
    for (int round = 0; round < 10; ++round) {
        add_round_const_q(q, round, out);       // -> state ^ x0
        sub_bytes_masked(mbox, out, q);         // -> state ^ x1
        shift_bytes_q(q, out);                  // -> state ^ x2
        mix_bytes(out, q);                      // -> state ^ x3
        xor_bytes(q, q, x4);                    // -> state ^ x0
    }

    for (int i = 0; i < 64; ++i)
        out[i] = chain[i] ^ p[i] ^ q[i];        // -> state
}

// -----------------------------------------------------------------------------
void output_transform(const uint8_t *msg, uint8_t *out)
{
    uint8_t p[64];
    permute_p(msg, p);
    for (int i = 0; i < 32; ++i)
        out[i] = msg[i + 32] ^ p[i + 32];
}

// -----------------------------------------------------------------------------
void pad_message(vector<uint8_t> &msg)
{
    int N = (int)msg.size() << 3;
    int w = util::mod(-N - 65, 512);
    int wb = (w + 1) >> 3;
    uint64_t r = (N + w + 65) >> 9;

    // insert padding bytes
    for (int i = 0; i < wb; i++) msg.push_back(i ? 0x00 : 0x80);
    for (int i = 7; i >= 0; i--) msg.push_back((r >> (i << 3)) & 0xFF);
}

// -----------------------------------------------------------------------------
void hash(const vector<uint8_t> &in, vector<uint8_t> &out)
{
    vector<uint8_t> state(in), chain(64, 0);
    chain[62] = 0x01; // IV

    uint8_t test_in[64], test_out[64];
    for (int i = 0; i < 64; i++)
        test_in[i] = i;

    shift_bytes_q(test_in, test_out);
    for (int i = 0; i < 64; i++)
        test_in[test_out[i]] = i;

    for (int i = 0; i < 64; i++)
        printf("%2d%s", test_in[i], ((i + 1) % 16) ? ", " : ",\n");

    // compute hash
    pad_message(state);
    for (size_t i = 0; i < state.size(); i += 64)
        compress(&state[i], &chain[0], &chain[0]);

    out.resize(32);
    output_transform(&chain[0], &out[0]);
}

};

