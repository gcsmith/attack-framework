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

#include "attack_manager.h"
#include "aes.h"

class crypto_aes_hw_r10: public crypto_instance
{
    std::vector<uint8_t> m_msg;
    std::vector<uint8_t> m_key;

public:
    virtual void set_message(const std::vector<uint8_t> &msg);
    virtual void set_key(const std::vector<uint8_t> &key);
    virtual int extract_estimate(int k);
    virtual int compute(int n, int k);
    virtual int key_bits()      { return 128; }
    virtual int estimate_bits() { return 8; }
    virtual int target_bits()   { return 8; }
};

// -----------------------------------------------------------------------------
void crypto_aes_hw_r10::set_message(const std::vector<uint8_t> &msg)
{
    m_msg = msg;
}

// -----------------------------------------------------------------------------
void crypto_aes_hw_r10::set_key(const std::vector<uint8_t> &key)
{
    m_key = key;
}

// -----------------------------------------------------------------------------
int crypto_aes_hw_r10::extract_estimate(int k)
{
    return m_key[k];
}

// -----------------------------------------------------------------------------
int crypto_aes_hw_r10::compute(int n, int k)
{
    return aes::sbox_inv[m_msg[aes::shift_inv[n]] ^ k];
}

register_crypto(aes_hw_r10, crypto_aes_hw_r10());

