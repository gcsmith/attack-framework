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
#include "des.h"
#include "utility.h"

class crypto_des_hd_r0: public crypto_instance {
public:
    virtual void set_message(const std::vector<uint8_t> &msg) {
        assert(msg.size() == 8);
        m_bits = util::convert_bytes(&msg[0]);
    }

    virtual void set_key(const std::vector<uint8_t> &key) {
        m_key.resize(8);
        for (int i = 0; i < 8; ++i) {
            int byte = (util::convert_bytes(&key[0]) >> (i * 6)) & 0x3F;
            m_key[i] = util::revb(byte, 6);
        }
    }

    virtual int extract_estimate(int n) {
        return m_key[n];
    }

    virtual int compute(int n, int k) {
        uint64_t ip = des::permute(des::ip, m_bits, 64);
        uint32_t l0 = ip & 0xFFFFFFFF, r0 = ip >> 32;
        uint32_t e0 = ((des::permute(des::e, r0, 48) >> (n * 6)) & 0x3F);
        uint32_t ep = des::permute(des::ps, e0 ^ util::revb(k, 6), 6);
        uint32_t s0 = (des::permute_inv(des::p, l0 ^ r0, 32) >> (n * 4)) & 0x0F;
        uint32_t sb = util::revb(des::sbox[n][ep], 4);
        return sb ^ s0;
    }

    virtual int key_bits()      { return 48; }
    virtual int estimate_bits() { return 6; }
    virtual int target_bits()   { return 4; }

protected:
    uint64_t m_bits;
    std::vector<int> m_key;
};

register_crypto(des_hd_r0, crypto_des_hd_r0);

