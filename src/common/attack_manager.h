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

#ifndef ATTACK_MANAGER__H
#define ATTACK_MANAGER__H

#include <cstdio>
#include "trace.h"
#include "utility.h"

typedef std::vector<long> time_map;

struct crypto_instance {
    /// Set the current message block (plaintext or ciphertext).
    virtual bool set_message(const std::vector<uint8_t> &msg) = 0;

    /// Set the known encryption key (for use in power profiling).
    virtual bool set_key(const std::vector<uint8_t> &key) = 0;

    /// Given a known encryption key, extract estimate partition index k.
    virtual int extract_estimate(int k) = 0;

    /// Compute the sensitive value for the given key guess.
    virtual int compute(int n, int k) = 0;

    /// Return the total size of the encryption key in bits.
    virtual int key_bits() = 0;

    /// Return the size of the secret value estimate in bits.
    virtual int estimate_bits() = 0;

    /// Return the size of the attack target (sensitive value) in bits.
    virtual int target_bits() = 0;

    // Explicit virtual destructor, as crypto_instance will be subclassed
    virtual ~crypto_instance() {}
};

struct attack_instance {
    /// Process attack parameters and perform pre-attack initialization.
    virtual bool setup(crypto_instance *crypto, const util::parameters &params) = 0;

    /// Process a single power trace / message pair.
    virtual void process(const time_map &tmap, const trace &pt) = 0;

    /// Record the intermediate attack state for the specified interval.
    virtual void record_interval(size_t n) = 0;

    /// Merge the state of two attack_instance objects together.
    virtual void coalesce(attack_instance *inst) = 0;

    // Write attack results to the specified directory
    virtual void write_results(const std::string &path) = 0;

    /// Perform post-attack shutdown (if any).
    virtual bool cleanup() = 0;

    /// TODO: description
    virtual void get_diffs(std::vector<double> &diffs) = 0;

    /// TODO: description
    virtual void get_maxes(std::vector<double> &maxes) = 0;

    // Explicit virtual destructor, as attack_instance will be subclassed
    virtual ~attack_instance() {}
};

namespace attack_manager {
    typedef attack_instance *(*create_attack_fn)(void);
    typedef crypto_instance *(*create_crypto_fn)(void);

    /// Add a new attack instance to the manager.
    bool add_attack(const std::string &name, create_attack_fn fn);

    /// Add a new crypto instance to the manager.
    bool add_crypto(const std::string &name, create_crypto_fn fn);

    /// Create an attack instance of the specified name
    attack_instance *create_attack(const std::string &name);

    /// Create a crypto instance of the specified name
    crypto_instance *create_crypto(const std::string &name);

    // TODO: description
    void list(FILE *fp);
};

#define register_attack(n, o)                                           \
    namespace {                                                         \
        static struct register_##n {                                    \
            register_##n() { attack_manager::add_attack(#n, create); }  \
            static attack_instance *create() { return new o ; }         \
        } _##n;                                                         \
    };

#define register_crypto(n, o)                                           \
    namespace {                                                         \
        static struct register_##n {                                    \
            register_##n() { attack_manager::add_crypto(#n, create); }  \
            static crypto_instance *create() { return new o ; }         \
        } _##n;                                                         \
    };

#endif // ATTACK_MANAGER__H

