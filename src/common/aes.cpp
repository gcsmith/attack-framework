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

#include "aes.h"
#include "crypto.h"
#include "utility.h"

namespace aes {

int rcon[10] = {
    0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1B, 0x36
};

int shift[16] = {
    0, 5, 10, 15, 4, 9, 14, 3, 8, 13, 2, 7, 12, 1, 6, 11
};

int shift_inv[16] = {
    0, 13, 10, 7, 4, 1, 14, 11, 8, 5, 2, 15, 12, 9, 6, 3
};

int sbox[256] = {
    X_(63, 7C, 77, 7B, F2, 6B, 6F, C5, 30, 01, 67, 2B, FE, D7, AB, 76),
    X_(CA, 82, C9, 7D, FA, 59, 47, F0, AD, D4, A2, AF, 9C, A4, 72, C0),
    X_(B7, FD, 93, 26, 36, 3F, F7, CC, 34, A5, E5, F1, 71, D8, 31, 15),
    X_(04, C7, 23, C3, 18, 96, 05, 9A, 07, 12, 80, E2, EB, 27, B2, 75),
    X_(09, 83, 2C, 1A, 1B, 6E, 5A, A0, 52, 3B, D6, B3, 29, E3, 2F, 84),
    X_(53, D1, 00, ED, 20, FC, B1, 5B, 6A, CB, BE, 39, 4A, 4C, 58, CF),
    X_(D0, EF, AA, FB, 43, 4D, 33, 85, 45, F9, 02, 7F, 50, 3C, 9F, A8),
    X_(51, A3, 40, 8F, 92, 9D, 38, F5, BC, B6, DA, 21, 10, FF, F3, D2),
    X_(CD, 0C, 13, EC, 5F, 97, 44, 17, C4, A7, 7E, 3D, 64, 5D, 19, 73),
    X_(60, 81, 4F, DC, 22, 2A, 90, 88, 46, EE, B8, 14, DE, 5E, 0B, DB),
    X_(E0, 32, 3A, 0A, 49, 06, 24, 5C, C2, D3, AC, 62, 91, 95, E4, 79),
    X_(E7, C8, 37, 6D, 8D, D5, 4E, A9, 6C, 56, F4, EA, 65, 7A, AE, 08),
    X_(BA, 78, 25, 2E, 1C, A6, B4, C6, E8, DD, 74, 1F, 4B, BD, 8B, 8A),
    X_(70, 3E, B5, 66, 48, 03, F6, 0E, 61, 35, 57, B9, 86, C1, 1D, 9E),
    X_(E1, F8, 98, 11, 69, D9, 8E, 94, 9B, 1E, 87, E9, CE, 55, 28, DF),
    X_(8C, A1, 89, 0D, BF, E6, 42, 68, 41, 99, 2D, 0F, B0, 54, BB, 16),
};

int sbox_inv[256] = {
    X_(52, 09, 6A, D5, 30, 36, A5, 38, BF, 40, A3, 9E, 81, F3, D7, FB),
    X_(7C, E3, 39, 82, 9B, 2F, FF, 87, 34, 8E, 43, 44, C4, DE, E9, CB),
    X_(54, 7B, 94, 32, A6, C2, 23, 3D, EE, 4C, 95, 0B, 42, FA, C3, 4E),
    X_(08, 2E, A1, 66, 28, D9, 24, B2, 76, 5B, A2, 49, 6D, 8B, D1, 25),
    X_(72, F8, F6, 64, 86, 68, 98, 16, D4, A4, 5C, CC, 5D, 65, B6, 92),
    X_(6C, 70, 48, 50, FD, ED, B9, DA, 5E, 15, 46, 57, A7, 8D, 9D, 84),
    X_(90, D8, AB, 00, 8C, BC, D3, 0A, F7, E4, 58, 05, B8, B3, 45, 06),
    X_(D0, 2C, 1E, 8F, CA, 3F, 0F, 02, C1, AF, BD, 03, 01, 13, 8A, 6B),
    X_(3A, 91, 11, 41, 4F, 67, DC, EA, 97, F2, CF, CE, F0, B4, E6, 73),
    X_(96, AC, 74, 22, E7, AD, 35, 85, E2, F9, 37, E8, 1C, 75, DF, 6E),
    X_(47, F1, 1A, 71, 1D, 29, C5, 89, 6F, B7, 62, 0E, AA, 18, BE, 1B),
    X_(FC, 56, 3E, 4B, C6, D2, 79, 20, 9A, DB, C0, FE, 78, CD, 5A, F4),
    X_(1F, DD, A8, 33, 88, 07, C7, 31, B1, 12, 10, 59, 27, 80, EC, 5F),
    X_(60, 51, 7F, A9, 19, B5, 4A, 0D, 2D, E5, 7A, 9F, 93, C9, 9C, EF),
    X_(A0, E0, 3B, 4D, AE, 2A, F5, B0, C8, EB, BB, 3C, 83, 53, 99, 61),
    X_(17, 2B, 04, 7E, BA, 77, D6, 26, E1, 69, 14, 63, 55, 21, 0C, 7D),
};

// -----------------------------------------------------------------------------
void key_schedule(uint8_t *sk, int rounds)
{
    for (int r = 0; r < rounds; ++r, sk += 4) {
        sk[16] = sbox[sk[13]] ^ sk[0] ^ rcon[r];
        sk[17] = sbox[sk[14]] ^ sk[1];
        sk[18] = sbox[sk[15]] ^ sk[2];
        sk[19] = sbox[sk[12]] ^ sk[3];
        for (int c = 0; c < 3; ++c, sk += 4) {
            sk[20] = sk[16] ^ sk[4];
            sk[21] = sk[17] ^ sk[5];
            sk[22] = sk[18] ^ sk[6];
            sk[23] = sk[19] ^ sk[7];
        }
    }
}

// -----------------------------------------------------------------------------
void add_round_key(const uint8_t *in, uint8_t *out, const uint8_t *sk)
{
    for (int i = 0; i < 16; ++i)
        out[i]  = in[i] ^ sk[i];
}

// -----------------------------------------------------------------------------
void sub_bytes(const int *tab, const uint8_t *in, uint8_t *out)
{
    for (int i = 0; i < 16; ++i)
        out[i] = tab[in[i]];
}

// -----------------------------------------------------------------------------
void shift_rows(const uint8_t *in, uint8_t *out)
{
    for (int i = 0; i < 16; ++i)
        out[i] = in[shift[i]];
}

// -----------------------------------------------------------------------------
void shift_rows_inv(const uint8_t *in, uint8_t *out)
{
    for (int i = 0; i < 16; ++i)
        out[i] = in[shift_inv[i]];
}

// -----------------------------------------------------------------------------
void mix_columns(const uint8_t *in, uint8_t *out)
{
    for (int i = 0; i < 4; ++i, in += 4, out += 4) {
        out[0] = GF_2(in[0]) ^ GF_3(in[1]) ^ GF_1(in[2]) ^ GF_1(in[3]);
        out[1] = GF_1(in[0]) ^ GF_2(in[1]) ^ GF_3(in[2]) ^ GF_1(in[3]);
        out[2] = GF_1(in[0]) ^ GF_1(in[1]) ^ GF_2(in[2]) ^ GF_3(in[3]);
        out[3] = GF_3(in[0]) ^ GF_1(in[1]) ^ GF_1(in[2]) ^ GF_2(in[3]);
    }
}
// -----------------------------------------------------------------------------
void mix_columns_inv(const uint8_t *in, uint8_t *out)
{
    for (int i = 0; i < 4; ++i, in += 4, out += 4) {
        out[0] = GF_E(in[0]) ^ GF_B(in[1]) ^ GF_D(in[2]) ^ GF_9(in[3]);
        out[1] = GF_9(in[0]) ^ GF_E(in[1]) ^ GF_B(in[2]) ^ GF_D(in[3]);
        out[2] = GF_D(in[0]) ^ GF_9(in[1]) ^ GF_E(in[2]) ^ GF_B(in[3]);
        out[3] = GF_B(in[0]) ^ GF_D(in[1]) ^ GF_9(in[2]) ^ GF_E(in[3]);
    }
}

// -----------------------------------------------------------------------------
void encrypt(const uint8_t *pt, const uint8_t *sk, uint8_t *ct)
{
    uint8_t s0[16], *s1 = ct;
    add_round_key(pt, s0, sk);

    for (int round = 1; round < 10; ++round) {
        sub_bytes(sbox, s0, s1);
        shift_rows(s1, s0);
        mix_columns(s0, s1);
        add_round_key(s1, s0, &sk[round * 16]);
    }

    sub_bytes(sbox, s0, s1);
    shift_rows(s1, s0);
    add_round_key(s0, s1, &sk[10 * 16]);
}

// -----------------------------------------------------------------------------
void encrypt_mask(const uint8_t *pt, const uint8_t *sk, uint8_t *ct, uint8_t m)
{
    int mbox[256];
    uint8_t pm[16], s0[16], *s1 = ct;

    // mask the input state and generate a masked substitution table
    for (int i = 0; i < 16; ++i) pm[i] = pt[i] ^ m;
    for (int i = 0; i < 256; ++i) mbox[i ^ m] = sbox[i] ^ m;

    // perform the rest of the encryption as usual
    add_round_key(pm, s0, sk);

    for (int round = 1; round < 10; ++round) {
        sub_bytes(mbox, s0, s1);
        shift_rows(s1, s0);
        mix_columns(s0, s1);
        add_round_key(s1, s0, &sk[round * 16]);
    }

    sub_bytes(mbox, s0, s1);
    shift_rows(s1, s0);
    add_round_key(s0, s1, &sk[10 * 16]);

    // unmask the output state
    for (int i = 0; i < 16; ++i) s1[i] ^= m;
}

// -----------------------------------------------------------------------------
void decrypt(const uint8_t *pt, const uint8_t *sk, uint8_t *ct)
{
    uint8_t s0[16], *s1 = ct;
    add_round_key(pt, s0, &sk[10 * 16]);

    for (int round = 9; round > 0; --round) {
        shift_rows_inv(s0, s1);
        sub_bytes(sbox_inv, s1, s0);
        add_round_key(s0, s1, &sk[round * 16]);
        mix_columns_inv(s1, s0);
    }

    shift_rows_inv(s0, s1);
    sub_bytes(sbox_inv, s1, s0);
    add_round_key(s0, s1, sk);
}

// -----------------------------------------------------------------------------
void decrypt_mask(const uint8_t *ct, const uint8_t *sk, uint8_t *pt, uint8_t m)
{
    int mbox[256];
    uint8_t pm[16], s0[16], *s1 = pt;

    // mask the input state and generate a masked substitution table
    for (int i = 0; i < 16; ++i) pm[i] = ct[i] ^ m;
    for (int i = 0; i < 256; ++i) mbox[i ^ m] = sbox_inv[i] ^ m;

    // perform the rest of the encryption as usual
    add_round_key(pm, s0, &sk[10 * 16]);

    for (int round = 9; round > 0; --round) {
        shift_rows_inv(s0, s1);
        sub_bytes(mbox, s1, s0);
        add_round_key(s0, s1, &sk[round * 16]);
        mix_columns_inv(s1, s0);
    }

    shift_rows_inv(s0, s1);
    sub_bytes(mbox, s1, s0);
    add_round_key(s0, s1, sk);

    // unmask the output state
    for (int i = 0; i < 16; ++i) s1[i] ^= m;
}

}; // namespace aes

