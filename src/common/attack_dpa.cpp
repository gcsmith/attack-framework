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

#include <cstdlib>
#include <cmath>
#include "attack_engine.h"
#include "attack_manager.h"
#include "crypto.h"

using namespace std;

// -----------------------------------------------------------------------------
template <typename real>
class attack_dpa: public attack_instance {
public:
    attack_dpa();
    ~attack_dpa();

    virtual bool setup(crypto_instance *crypto, const util::parameters &params);
    virtual void process(const time_map &tmap, const trace &pt);
    virtual void record_interval(size_t n);
    virtual void coalesce(attack_instance *inst);
    virtual void write_results(const string &path);
    virtual bool cleanup();

    virtual void get_diffs(vector<double> &diffs);
    virtual void get_maxes(vector<double> &maxes);

protected:
    void compute_diffs(real *d);

    size_t m_nevents;
    size_t m_nreports;
    unsigned int m_mask, m_byte, m_offset, m_bits;
    unsigned int m_min, m_max;
    vector<size_t> m_binsz; // current size of each bin
    vector<size_t> m_group; // bin sizes for each report interval
    vector<real> m_dtemp;
    vector<real> m_diffs;   // positive and negative differentials
    vector<real> m_maxes;
    crypto_instance *m_crypto;
    int m_guesses;
};

// -----------------------------------------------------------------------------
template <typename real>
attack_dpa<real>::attack_dpa()
{
}

// -----------------------------------------------------------------------------
template <typename real>
attack_dpa<real>::~attack_dpa()
{
}

// -----------------------------------------------------------------------------
template <typename real>
void attack_dpa<real>::compute_diffs(real *d)
{
    for (size_t i = 0; i < m_nevents; ++i) {
        for (int k = 0; k < m_guesses; ++k) {
            const size_t i_off = k * m_nevents * 2 + i;
            const size_t o_off = k * m_nevents + i;
            const real a = m_diffs[i_off];
            const real b = m_diffs[i_off + m_nevents];
            d[o_off] = (b / m_binsz[k * 3 + 1]) - (a / m_binsz[k * 3]);
        }
    }
}

// -----------------------------------------------------------------------------
template <typename real>
bool attack_dpa<real>::setup(crypto_instance *crypto,
                             const util::parameters &params)
{
    unsigned int thresh;
    if (!params.get("num_events", m_nevents) ||
        !params.get("num_reports", m_nreports) ||
        !params.get("byte", m_byte) ||
        !params.get("offset", m_offset) ||
        !params.get("bits", m_bits) ||
        !params.get("thresh", thresh)) {
        fprintf(stderr, "missing parameters in attack_dpa\n");
        return false;
    }

    if (m_bits > 1) {
        // compute the minimum and maximum range for segmenting the traces
        m_min = m_bits - thresh - 1;
        m_max = thresh;
    }
    else {
        // for a single-bit attack we'll only compare against 0 and 1
        m_min = 0;
        m_max = 1;
    }

    // check for an erroneous combination of #bits and thresh
    if (thresh >= m_bits || m_min >= m_max) {
        fprintf(stderr, "invalid threshold specified\n");
        return false;
    }

    // compute a mask to extract the portion of the weight we're interested in
    m_mask = 0;
    for (unsigned int i = m_offset; i < (m_offset + m_bits); ++i)
        m_mask |= 1 << i;

    m_crypto = crypto;
    m_guesses = 1 << m_crypto->estimate_bits();

    m_dtemp.resize(m_guesses * m_nevents, 0);
    m_diffs.resize(m_guesses * 2 * m_nevents, 0);
    m_binsz.resize(m_guesses * 3, 0);
    m_group.resize(m_guesses * 3 * m_nreports, 0);
    m_maxes.resize(m_guesses * m_nreports, 0);

    return true;
}

// -----------------------------------------------------------------------------
template <typename real>
void attack_dpa<real>::process(const time_map &tmap, const trace &pt)
{
    for (int k = 0; k < m_guesses; ++k) {
        const unsigned int target = m_crypto->compute(m_byte, k);
        const unsigned int weight = crypto::popcnt[target & m_mask];

        int select = 2;
        if (weight <= m_min)
            select = 0;
        else if (weight >= m_max)
            select = 1;

        ++m_binsz[k * 3 + select];
        if (select >= 2) continue;

        real *d = &m_diffs[(k * 2 + select) * m_nevents];
        for (size_t s = 0; s < pt.size(); ++s)
            d[tmap[s]] += pt[s].power;
    }
}

// -----------------------------------------------------------------------------
template <typename real>
void attack_dpa<real>::record_interval(size_t n)
{
    // store bin sizes for this interval
    size_t *c = &m_group[n * m_guesses * 3];
    for (int i = 0; i < m_guesses * 3; ++i) c[i] = m_binsz[i];

    // compute differentials and interval maxes
    compute_diffs(&m_dtemp[0]);
    real *m = &m_maxes[n * m_guesses];
    for (size_t i = 0; i < m_nevents; ++i)
        for (int k = 0; k < m_guesses; ++k)
            m[k] = max(m[k], m_dtemp[k * m_nevents + i]);
}

// -----------------------------------------------------------------------------
template <typename real>
void attack_dpa<real>::coalesce(attack_instance *inst)
{
    attack_dpa *other = (attack_dpa *)inst;
    assert(m_guesses == other->m_guesses);
    assert(m_nevents == other->m_nevents);

    for (int k = 0; k < m_guesses; ++k) {
        // accumulate bin sizes
        for (int g = 0; g < 3; ++g)
            m_binsz[k * 3 + g] += other->m_binsz[k * 3 + g];

        // accumulate differentials
        for (int g = 0; g < 2; ++g) {
            const size_t noff = (k * 2 + g) * m_nevents;
            for (size_t s = 0; s < m_nevents; ++s)
                m_diffs[noff + s] += other->m_diffs[noff + s];
        }
    }
}

// -----------------------------------------------------------------------------
template <typename real>
void attack_dpa<real>::get_diffs(vector<double> &diffs)
{
    compute_diffs(&m_dtemp[0]);

    diffs.resize(m_guesses * m_nevents);
    for (size_t i = 0; i < m_guesses * m_nevents; ++i)
        diffs[i] = m_dtemp[i];
}

// -----------------------------------------------------------------------------
template <typename real>
void attack_dpa<real>::get_maxes(vector<double> &maxes)
{
    maxes.resize(m_guesses * m_nreports);
    for (size_t i = 0; i < m_guesses * m_nreports; ++i)
        maxes[i] = m_maxes[i];
}

// -----------------------------------------------------------------------------
template <typename real>
void attack_dpa<real>::write_results(const string &path)
{
//     ngroups = 3;
//     group.resize(m_guesses * 3 * m_nreports);
//     for (size_t i = 0; i < m_guesses * 3 * m_nreports; ++i)
//         group[i] = m_group[i];
// 
//     if (!ngroups || group.size() != 256 * ngroups * m_numintervals) {
//         fprintf(stderr, "invalid # of groups in write_group_report\n");
//         return;
//     }
// 
//     for (size_t i = 0; i < m_numintervals; ++i) {
//         const size_t *c = &group[i * 256 * ngroups];
//         m_ogrp << m_interval * (i + 1);
// 
//         for (int g = 0; g < ngroups; ++g) {
//             m_ogrp << ',' << g << ',';
//             for (int k = 0; k < 256; ++k)
//                 m_ogrp << c[k * ngroups + g] << ',';
//             m_ogrp << endl;
//         }
//     }
}

// -----------------------------------------------------------------------------
template <typename real>
bool attack_dpa<real>::cleanup()
{
    return true;
}

register_attack(dpa, attack_dpa<float>);
register_attack(dpa_dp, attack_dpa<double>);

