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

typedef std::vector<uint8_t> bytes;

/// Compute the hash of the input message using Grostl-256.
void hash(const bytes &in, bytes &out);

} // namespace grostl

#endif // GROSTL__H

