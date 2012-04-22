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
#include <vector>

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

