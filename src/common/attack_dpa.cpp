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
#include "crypto.h"

using namespace std;
using namespace util;

// -----------------------------------------------------------------------------
template <typename real>
class attack_dpa: public attack_instance
{
public:
    attack_dpa();
    ~attack_dpa();

    void compute_diffs(real *d);

    virtual bool setup(crypto_instance *crypto, const parameters &params);
    virtual void process(const time_map &tmap, const trace &pt);
    virtual void record_interval(size_t n);
    virtual void coalesce(attack_instance *inst);
    virtual bool cleanup();

    virtual void get_diffs(vector<double> &diffs);
    virtual void get_maxes(vector<double> &maxes);
    virtual void get_group(vector<size_t> &group, int &ngroups);

protected:
    size_t m_nevents;
    size_t m_nreports;
    unsigned int m_mask, m_byte, m_offset, m_bits, m_thresh;
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
bool attack_dpa<real>::setup(crypto_instance *crypto, const parameters &params)
{
    if (!params.get("num_events", m_nevents) ||
        !params.get("num_reports", m_nreports) ||
        !params.get("byte", m_byte) ||
        !params.get("offset", m_offset) ||
        !params.get("bits", m_bits) ||
        !params.get("thresh", m_thresh)) {
        fprintf(stderr, "missing parameters in attack_dpa\n");
        return false;
    }

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
    const size_t num_samples = pt.size();
    m_crypto->set_message(pt.text());

    for (int k = 0; k < m_guesses; ++k) {
        const unsigned int target = m_crypto->compute(m_byte, k);
        const unsigned int weight = crypto::popcnt[target & m_mask];

        int select = 2;
        if (weight <= (m_bits - m_thresh))
            select = 0;
        else if (weight >= m_thresh)
            select = 1;

        ++m_binsz[k * 3 + select];
        if (select >= 2) continue;

        real *d = &m_diffs[(k * 2 + select) * m_nevents];
        for (size_t s = 0; s < num_samples; ++s)
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

    if (m_guesses != other->m_guesses) {
        fprintf(stderr, "attack_dpa::coalesce - guesses mismatch\n");
        return;
    }
    if (m_nevents != other->m_nevents) {
        fprintf(stderr, "attack_dpa::coalesce - nevents mismatch\n");
        return;
    }

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
void attack_dpa<real>::get_group(vector<size_t> &group, int &ngroups)
{
    ngroups = 3;
    group.resize(m_guesses * 3 * m_nreports);
    for (size_t i = 0; i < m_guesses * 3 * m_nreports; ++i)
        group[i] = m_group[i];
}

// -----------------------------------------------------------------------------
template <typename real>
bool attack_dpa<real>::cleanup()
{
    return true;
}

register_attack(dpa, attack_dpa<float>);
register_attack(dpa_dp, attack_dpa<double>);
