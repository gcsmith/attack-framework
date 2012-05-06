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

class crypto_aes_hd_r0: public crypto_instance {
public:
    virtual void set_message(const std::vector<uint8_t> &msg) {
        assert(msg.size() == 16);
        m_msg = msg;
    }

    virtual void set_key(const std::vector<uint8_t> &key) {
        assert(key.size() == 16);
        m_key = key;
    }

    virtual int extract_estimate(int n) {
        assert(n < 16);
        return m_key[n];
    }

    virtual int compute(int n, int k) {
        assert(n < 16 && k < 256);
        const int istate = m_msg[n] ^ k;
        const int ostate = aes::sbox[m_msg[n] ^ k];
        return istate ^ ostate;
    }

    virtual int key_bits()      { return 128; }
    virtual int estimate_bits() { return 8; }
    virtual int target_bits()   { return 8; }

protected:
    std::vector<uint8_t> m_msg;
    std::vector<uint8_t> m_key;
};

register_crypto(aes_hd_r0, crypto_aes_hd_r0);

