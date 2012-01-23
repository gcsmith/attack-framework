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

#ifndef DES__H
#define DES__H

#include <stddef.h>
#include <stdint.h>

namespace des {

extern int ip[64];      /// initial permutation table
extern int e[48];       /// expansion table
extern int p[32];       /// function F permutation table
extern int pc1[56];     /// permuted choice 1 table
extern int pc2[48];     /// permuted choice 2 table
extern int rot[16];     /// bit rotation table
extern int sbox[8][64]; /// sbox 1-8 tables
extern int ps[6];

/// Perform a forward bit permutation.
inline uint64_t permute(const int *lut, uint64_t in, size_t bits)
{
    uint64_t x = 0;
    for (size_t i = 0; i < bits; ++i) x |= ((in >> (lut[i] - 1)) & 1) << i;
    return x;
}

/// Perform a backward bit permutation.
inline uint64_t permute_inv(const int *lut, uint64_t in, size_t bits)
{
    uint64_t x = 0;
    for (size_t i = 0; i < bits; ++i) x |= ((in >> i) & 1) << (lut[i] - 1);
    return x;
}

/// Compute the round function F for the given value R and subkey K.
uint32_t f(uint32_t r, uint64_t k);

/// Compute each of the 16 subkeys for the specified encryption key.
void key_schedule(uint64_t key, uint64_t *sk);

/// Perform a DES encryption operation on the specified plaintext.
uint64_t encrypt(uint64_t pt, const uint64_t *sk);

/// Perform a DES decryption operation on the specified ciphertext.
uint64_t decrypt(uint64_t pt, const uint64_t *sk);

}; // namespace des

#endif // DES__H

