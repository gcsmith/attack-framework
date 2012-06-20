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

#ifndef AES__H
#define AES__H

#include <stdint.h>

// multiplication by constants 1-F in GF(2^8)
#define GF_1(x) (x)
#define GF_2(x) ((x & 0x80) ? ((x << 1) ^ 0x1B) : (x << 1))
#define GF_3(x) (GF_2(x) ^ GF_1(x))
#define GF_4(x) (GF_2(GF_2(x)))
#define GF_5(x) (GF_4(x) ^ GF_1(x))
#define GF_6(x) (GF_4(x) ^ GF_2(x))
#define GF_7(x) (GF_4(x) ^ GF_2(x) ^ GF_1(x))
#define GF_8(x) (GF_2(GF_2(GF_2(x))))
#define GF_9(x) (GF_8(x) ^ GF_1(x))
#define GF_A(x) (GF_8(x) ^ GF_2(x))
#define GF_B(x) (GF_8(x) ^ GF_2(x) ^ GF_1(x))
#define GF_C(x) (GF_8(x) ^ GF_4(x))
#define GF_D(x) (GF_8(x) ^ GF_4(x) ^ GF_1(x))
#define GF_E(x) (GF_8(x) ^ GF_4(x) ^ GF_2(x))
#define GF_F(x) (GF_8(x) ^ GF_4(x) ^ GF_2(x) ^ GF_1(x))

#define X_(x0, x1, x2, x3, x4, x5, x6, x7, x8, x9, xA, xB, xC, xD, xE, xF)  \
    0x##x0, 0x##x1, 0x##x2, 0x##x3, 0x##x4, 0x##x5, 0x##x6, 0x##x7,         \
    0x##x8, 0x##x9, 0x##xA, 0x##xB, 0x##xC, 0x##xD, 0x##xE, 0x##xF

namespace aes {

extern int rcon[10];
extern int shift[16];
extern int shift_inv[16];
extern int sbox[256];
extern int sbox_inv[256];

/// Compute each of the 10 subkeys for the specified encryption key.
void key_schedule(uint8_t *sk, int rounds);

/// Perform an AES128 encryption operation on the specified plaintext.
void encrypt(const uint8_t *pt, const uint8_t *sk, uint8_t *ct);

/// Perform an AES128 decryption operation on the specified ciphertext.
void decrypt(const uint8_t *ct, const uint8_t *sk, uint8_t *pt);

/// Generate masked state from the input state.
void mask_state(const uint8_t *state_in, uint8_t *state_out, uint8_t mask);

/// Generate a masked byte substitution table -- table[i + m'] = sbox[i] + m
void mask_sbox(const int *sbox_in, int *sbox_out, uint8_t m_in, uint8_t m_out);

/// Perform a masked AES128 encryption operation on the specified plaintext.
void encrypt_mask_simple(const uint8_t *pt, const uint8_t *sk, uint8_t *ct,
                         uint8_t imask, uint8_t omask);

/// Perform a masked AES128 decryption operation on the specified ciphertext.
void decrypt_mask_simple(const uint8_t *ct, const uint8_t *sk, uint8_t *pt,
                         uint8_t imask, uint8_t omask);

void encrypt_mask_full(const uint8_t *pt, const uint8_t *sk, uint8_t *ct);

}; // namespace aes

#endif // AES__H

