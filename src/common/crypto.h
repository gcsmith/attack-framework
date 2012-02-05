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

#ifndef CRYPTO__H
#define CRYPTO__H

// multiplication by constants 1-F in GF(2^8)
#define _m1(x) (x)
#define _m2(x) ((x & 0x80) ? ((x << 1) ^ 0x1B) : (x << 1))
#define _m3(x) (_m2(x) ^ _m1(x))
#define _m4(x) (_m2(_m2(x)))
#define _m5(x) (_m4(x) ^ _m1(x))
#define _m6(x) (_m4(x) ^ _m2(x))
#define _m7(x) (_m4(x) ^ _m2(x) ^ _m1(x))
#define _m8(x) (_m2(_m2(_m2(x))))
#define _m9(x) (_m8(x) ^ _m1(x))
#define _mA(x) (_m8(x) ^ _m2(x))
#define _mB(x) (_m8(x) ^ _m2(x) ^ _m1(x))
#define _mC(x) (_m8(x) ^ _m4(x))
#define _mD(x) (_m8(x) ^ _m4(x) ^ _m1(x))
#define _mE(x) (_m8(x) ^ _m4(x) ^ _m2(x))
#define _mF(x) (_m8(x) ^ _m4(x) ^ _m2(x) ^ _m1(x))

// helper macro to make s-box declarations more readable
#define X_(x0, x1, x2, x3, x4, x5, x6, x7, x8, x9, xA, xB, xC, xD, xE, xF)  \
    0x##x0, 0x##x1, 0x##x2, 0x##x3, 0x##x4, 0x##x5, 0x##x6, 0x##x7,         \
    0x##x8, 0x##x9, 0x##xA, 0x##xB, 0x##xC, 0x##xD, 0x##xE, 0x##xF

namespace crypto {

static const int popcnt[256] = {
    0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4, 1, 2, 2, 3, 2, 3, 3, 4, 2,
    3, 3, 4, 3, 4, 4, 5, 1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5, 2, 3,
    3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3,
    4, 3, 4, 4, 5, 2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 2, 3, 3, 4,
    3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5,
    6, 6, 7, 1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5, 2, 3, 3, 4, 3, 4,
    4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5,
    6, 3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7, 2, 3, 3, 4, 3, 4, 4, 5,
    3, 4, 4, 5, 4, 5, 5, 6, 3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7, 3,
    4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7, 4, 5, 5, 6, 5, 6, 6, 7, 5, 6,
    6, 7, 6, 7, 7, 8, 
};

}; // namespace crypto

#endif // CRYPTO__H

