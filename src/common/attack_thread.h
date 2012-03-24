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

#ifndef ATTACK_THREAD__H
#define ATTACK_THREAD__H

#include <string>
#include <memory>
#include "utility.h"

class attack_engine;
class attack_instance;
class crypto_instance;

// -----------------------------------------------------------------------------
class attack_thread {
public:
    attack_thread(int id, attack_engine *engine);
    ~attack_thread(void);

    bool create(const std::string &attack,
                const std::string &crypto,
                const util::parameters &params);
    void run(void);
    attack_instance *attack(void) const { return m_attack.get(); }
    crypto_instance *crypto(void) const { return m_crypto.get(); }

protected:
    int m_id;
    attack_engine *m_engine;
    std::auto_ptr<attack_instance> m_attack;
    std::auto_ptr<crypto_instance> m_crypto;
};

#endif // ATTACK_THREAD__H

