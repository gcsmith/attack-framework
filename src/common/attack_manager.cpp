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

#include <map>
#include "attack_manager.h"

using namespace std;

namespace attack_manager
{
    typedef map<string, create_attack_fn> attack_map;
    attack_map *n_attackMap = NULL;

    typedef map<string, create_crypto_fn> crypto_map;
    crypto_map *n_cryptoMap = NULL;
};

// -----------------------------------------------------------------------------
bool attack_manager::add_attack(const string &name, create_attack_fn fn)
{
    if (!n_attackMap)
        n_attackMap = new attack_map;

    return n_attackMap->insert(make_pair(name, fn)).second;
}

// -----------------------------------------------------------------------------
attack_instance *attack_manager::create_attack(const string &name)
{
    if (!n_attackMap) {
        fprintf(stderr, "error: no registered attack_instance objects!\n");
        return NULL;
    }

    attack_map::const_iterator iter = n_attackMap->find(name);
    if (n_attackMap->end() == iter) {
        list_attack(stderr);
        return NULL;
    }
    return iter->second();
}

// -----------------------------------------------------------------------------
bool attack_manager::add_crypto(const string &name, create_crypto_fn fn)
{
    if (!n_cryptoMap)
        n_cryptoMap = new crypto_map;

    return n_cryptoMap->insert(make_pair(name, fn)).second;
}

// -----------------------------------------------------------------------------
crypto_instance *attack_manager::create_crypto(const string &name)
{
    if (!n_cryptoMap) {
        fprintf(stderr, "error: no registered crypto_instance objects!\n");
        return NULL;
    }

    crypto_map::const_iterator iter = n_cryptoMap->find(name);
    if (n_cryptoMap->end() == iter) {
        list_crypto(stderr);
        return NULL;
    }
    return iter->second();
}

// -----------------------------------------------------------------------------
void attack_manager::list_attack(FILE *fp)
{
    fprintf(fp, "Supported attack algorithms:\n");
    if (n_attackMap) {
        int count = 0;
        attack_map::const_iterator i;
        for (i = n_attackMap->begin(); i != n_attackMap->end(); ++i)
            fprintf(fp, "    [%d] %s\n", count++, i->first.c_str());
    }
}

// -----------------------------------------------------------------------------
void attack_manager::list_crypto(FILE *fp)
{
    fprintf(fp, "Supported crypto functions:\n");
    if (n_cryptoMap) {
        int count = 0;
        crypto_map::const_iterator i;
        for (i = n_cryptoMap->begin(); i != n_cryptoMap->end(); ++i)
            fprintf(fp, "    [%d] %s\n", count++, i->first.c_str());
    }
}

