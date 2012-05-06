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

#ifndef GROSTL__H
#define GROSTL__H

#include <stdint.h>
#include <vector>

namespace grostl {

extern int shift_p[64];
extern int shift_q[64];
extern int shift_inv_p[64];
extern int shift_inv_q[64];

// Grostl-256 P permutation.
void permute_p(const uint8_t *in, uint8_t *out);

// Grostl-256 Q permutation.
void permute_q(const uint8_t *in, uint8_t *out);

// Grostl-256 compression function.
void compress(const uint8_t *msg, const uint8_t *chain, uint8_t *out);

// Grostl-256 compression function.
void compress(const uint8_t *msg, const uint8_t *chain, uint8_t *out,
              const uint8_t *imask, const uint8_t *omask);

/// Compute the Grostl-256 hash of the input message.
void hash(const std::vector<uint8_t> &in, std::vector<uint8_t> &out);

} // namespace grostl

#endif // GROSTL__H

