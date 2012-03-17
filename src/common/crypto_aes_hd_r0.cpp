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
    std::vector<uint8_t> m_msg;
    std::vector<uint8_t> m_key;

public:
    virtual bool set_message(const std::vector<uint8_t> &msg);
    virtual bool set_key(const std::vector<uint8_t> &key);
    virtual int extract_estimate(int n);
    virtual int compute(int n, int k);
    virtual int key_bits()      { return 128; }
    virtual int estimate_bits() { return 8; }
    virtual int target_bits()   { return 8; }
};

// -----------------------------------------------------------------------------
bool crypto_aes_hd_r0::set_message(const std::vector<uint8_t> &msg)
{
    if (msg.size() != 16) {
        fprintf(stderr, "expected 16-byte text, got %d\n", (int)msg.size());
        return false;
    }
    m_msg = msg;
    return true;
}

// -----------------------------------------------------------------------------
bool crypto_aes_hd_r0::set_key(const std::vector<uint8_t> &key)
{
    if (key.size() != 16) {
        fprintf(stderr, "expected 16-byte key, got %d\n", (int)key.size());
        return false;
    }
    m_key = key;
    return true;
}

// -----------------------------------------------------------------------------
int crypto_aes_hd_r0::extract_estimate(int n)
{
    assert(n < 16);
    return m_key[n];
}

// -----------------------------------------------------------------------------
int crypto_aes_hd_r0::compute(int n, int k)
{
    assert(n < 16 && k < 256);
    const int istate = m_msg[n] ^ k;
    const int ostate = aes::sbox[m_msg[n] ^ k];
    return istate ^ ostate;
}

register_crypto(aes_hd_r0, crypto_aes_hd_r0());

