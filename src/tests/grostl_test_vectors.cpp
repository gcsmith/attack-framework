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

#define BOOST_TEST_MODULE grostl

#include <vector>
#include <boost/algorithm/string.hpp>
#include <boost/test/included/unit_test.hpp>
#include "utility.h"
#include "grostl.h"

static struct {
    const char *msg, *hash; // msg is ASCII, hash is hex string
} grostl_test_vectors[] = {
    { "abc", "f3c1bb19c048801326a7efbcf16e3d7887446249829c379e1840d1a3a1e7d4d2" },
    { "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq", "22c23b160e561f80924d44f2cc5974cd5a1d36f69324211861e63b9b6cb7974c" },
};

// -----------------------------------------------------------------------------
BOOST_AUTO_TEST_CASE(grostl_test_hash)
{
    for (size_t i = 0; i < NUM_ELEMENTS(grostl_test_vectors); i++) {
        const char *msg = grostl_test_vectors[i].msg;
        std::vector<uint8_t> out, in(msg, msg + strlen(msg));

        grostl::hash(in, out);
        
        const std::string hash_str = util::btoa(out);
        const std::string hash_vec = grostl_test_vectors[i].hash;

        BOOST_CHECK( hash_str == boost::to_upper_copy(hash_vec) );
    }
}

