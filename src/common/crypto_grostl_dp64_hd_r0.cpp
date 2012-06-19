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

#include <cassert>
#include "attack_manager.h"
#include "aes.h"
#include "grostl.h"

class crypto_grostl_dp64_hd_r0: public crypto_instance {
public:
    virtual void set_message(const std::vector<uint8_t> &msg) {
        assert(msg.size() == 64);
        m_msg = msg;
    }

    virtual void set_key(const std::vector<uint8_t> &key) {
        assert(key.size() == 64);
        m_key = key;
    }

    virtual int extract_estimate(int n) {
        assert(n < 64);
        return m_key[n];
    }

    virtual int compute(int n, int k) {
        assert(n < 64 && k < 256);
        const int m = grostl::shift_q[grostl::shift_inv_p[n]];
        const int p = (( n & 7) ? m_msg[n] : m_msg[n] ^ (n << 1)) ^ k;
        const int q = ((~m & 7) ? m_msg[m] : m_msg[m] ^ ((m & ~7) << 1)) ^ 0xFF;
        return aes::sbox[q] ^ aes::sbox[p];
    }

    virtual int key_bits()      { return 512; }
    virtual int block_bits()    { return 512; }
    virtual int estimate_bits() { return 8; }
    virtual int target_bits()   { return 8; }

protected:
    std::vector<uint8_t> m_msg;
    std::vector<uint8_t> m_key;
};

register_crypto(grostl_dp64_hd_r0, crypto_grostl_dp64_hd_r0);

