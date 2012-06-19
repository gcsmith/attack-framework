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

#include "attack_engine.h"
#include "attack_manager.h"
#include "attack_thread.h"

using namespace std;

// -----------------------------------------------------------------------------
attack_thread::attack_thread(int id, attack_engine *engine)
: m_id(id), m_engine(engine)
{
}

// -----------------------------------------------------------------------------
attack_thread::~attack_thread(void)
{
    m_attack->cleanup();
}

// -----------------------------------------------------------------------------
bool attack_thread::create(const string &attack, const string &crypto,
                           const util::parameters &params)
{
    m_attack.reset(attack_manager::create_attack(attack));
    if (!m_attack.get()) {
        fprintf(stderr, "error creating attack instance: %s\n", attack.c_str());
        return false;
    }

    m_crypto.reset(attack_manager::create_crypto(crypto));
    if (!m_crypto.get()) {
        fprintf(stderr, "error creating crypto instance: %s\n", crypto.c_str());
        return false;
    }

    return m_attack->setup(m_crypto.get(), params);
}

// -----------------------------------------------------------------------------
void attack_thread::run(void)
{
    trace pt;
    vector<long> mapper;

    // determine correct byte-length of each text and check for erroneous input
    const size_t text_len = m_crypto->block_bits() >> 3;

    while (m_engine->next_trace(m_id, mapper, pt)) {
        // provide the next plaintext/ciphertext to the crypto instance
        const vector<uint8_t> text(pt.text());
        if (text.size() != text_len) {
            fprintf(stderr, "[%d] invalid plain/ciphertext specified: %s "
                            "(expected %zu bytes, got %zu)\n",
                    m_id, util::btoa(text).c_str(), text_len, text.size());
            return;
        }
        m_crypto->set_message(text);

        // run the attack algorithm on the next set of samples
        m_attack->process(mapper, pt);
    }
}

