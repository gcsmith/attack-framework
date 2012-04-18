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
// Perform the ShiftBytes step for the P permutation
void shift_bytes_p(const uint8_t *in, uint8_t *out)
{
    for (int i = 0; i < 64; ++i) {
        const int row = i & 7;
        const int col = ((i >> 3) + row) & 7;
        out[i] = in[(col << 3) | row];
    }
}

// -----------------------------------------------------------------------------
// Perform the ShiftBytes step for the Q permutation
void shift_bytes_q(const uint8_t *in, uint8_t *out)
{
    for (int i = 0; i < 64; ++i) {
        const int row = i & 7;
        const int col = ((i >> 3) + (row << 1) + (row < 4)) & 7;
        out[i] = in[(col << 3) | row];
    }
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
    printf("chain: %s\n", util::btoa(chain, 64).c_str());
    printf("msg:   %s\n", util::btoa(msg, 64).c_str());

    uint8_t hm[64], p[64], q[64];
    for (int i = 0; i < 64; ++i)
        hm[i] = msg[i] ^ chain[i];

    permute_p(hm, p);
    permute_q(msg, q);

    printf("p_out:   %s\n", util::btoa(p, 64).c_str());
    printf("q_out:   %s\n", util::btoa(q, 64).c_str());

    for (int i = 0; i < 64; ++i)
        out[i] = chain[i] ^ p[i] ^ q[i];

    printf("f_out:   %s\n", util::btoa(out, 64).c_str());
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
void hash(const vector<uint8_t> &in, vector<uint8_t> &out)
{
    vector<uint8_t> state(in), chain(64, 0);
    chain[62] = 0x01; // IV

    int N = (int)in.size() << 3;
    int w = util::mod(-N - 65, 512);
    int wb = (w + 1) >> 3;
    uint64_t r = (N + w + 65) >> 9;

    // insert padding bytes
    for (int i = 0; i < wb; i++) state.push_back(i ? 0x00 : 0x80);
    for (int i = 7; i >= 0; i--) state.push_back((r >> (i << 3)) & 0xFF);

    // compute hash
    for (size_t i = 0; i < state.size(); i += 64)
        compress(&state[i], &chain[0], &chain[0]);

    out.resize(32);
    output_transform(&chain[0], &out[0]);
}

};

