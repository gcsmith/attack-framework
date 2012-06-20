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

using namespace std;
using namespace util;

// -----------------------------------------------------------------------------
template <typename real>
class attack_cpa: public attack_instance {
public:
    attack_cpa();
    virtual ~attack_cpa();

    virtual bool setup(crypto_instance *crypto, const parameters &params);
    virtual void process(const time_map &tmap, const trace &pt);
    virtual void record_interval(size_t n);
    virtual void coalesce(attack_instance *inst);
    virtual void write_results(const string &path);
    virtual bool cleanup();

    virtual void get_diffs(vector<double> &diffs);
    virtual void get_maxes(vector<double> &maxes);

protected:
    void compute_diffs(real *d);

    crypto_instance *m_crypto;
    size_t m_traces;
    size_t m_nevents;
    size_t m_nreports;
    unsigned int m_mask, m_byte, m_offset, m_bits;
    vector<real> m_t1; // sum of traces
    vector<real> m_t2; // sum of squared traces
    vector<real> m_w1; // sum of weights
    vector<real> m_w2; // sum of squared weights
    vector<real> m_tw; // sum of weighted traces
    vector<real> m_dtemp;
    vector<real> m_maxes;
    int m_guesses;
    int m_center;
};

// -----------------------------------------------------------------------------
template <typename real>
attack_cpa<real>::attack_cpa()
{
}

// -----------------------------------------------------------------------------
template <typename real>
attack_cpa<real>::~attack_cpa()
{
}

// -----------------------------------------------------------------------------
template <typename real>
void attack_cpa<real>::compute_diffs(real *d)
{
    const real ni = 1.0 / m_traces;

    for (int k = 0; k < m_guesses; ++k) {
        const real hv = (m_w2[k] - m_w1[k] * m_w1[k] * ni) * ni;
        const real *tw = &m_tw[k * m_nevents];

        for (size_t s = 0; s < m_nevents; ++s) {
            const size_t off = k * m_nevents + s;
            const real tv = (m_t2[s] - m_t1[s] * m_t1[s] * ni) * ni;
            d[off] = (tw[s] - m_w1[k] * m_t1[s] * ni) * ni;
            d[off] = (tv != 0.f) ? (d[off] / sqrt(tv)) : 0.f;
            d[off] = (hv != 0.f) ? (d[off] / sqrt(hv)) : 0.f;
        }
    }
}

// -----------------------------------------------------------------------------
template <typename real>
bool attack_cpa<real>::setup(crypto_instance *crypto, const parameters &params)
{
    if (!params.get("num_events", m_nevents) ||
        !params.get("num_reports", m_nreports) ||
        !params.get("byte", m_byte) ||
        !params.get("offset", m_offset) ||
        !params.get("bits", m_bits)) {
        fprintf(stderr, "required parameters: byte, offset, bits\n");
        return false;
    }

    m_mask = 0;
    for (unsigned int i = m_offset; i < (m_offset + m_bits); ++i)
        m_mask |= 1 << i;

    m_crypto = crypto;
    m_guesses = 1 << m_crypto->estimate_bits();
    m_center = m_bits >> 1;
    m_traces = 0;

    // allocate storage for intermediate results in advance
    m_t1.resize(m_nevents, 0);
    m_t2.resize(m_nevents, 0);
    m_w1.resize(m_guesses, 0);
    m_w2.resize(m_guesses, 0);
    m_tw.resize(m_guesses * m_nevents, 0);
    m_dtemp.resize(m_guesses * m_nevents, 0);
    m_maxes.resize(m_guesses * m_nreports, 0);

    return true;
}

// -----------------------------------------------------------------------------
template <typename real>
void attack_cpa<real>::process(const time_map &tmap, const trace &pt)
{
    // accumulate power and power^2 for each sample
    for (size_t s = 0; s < pt.size(); ++s) {
        m_t1[tmap[s]] += pt[s].power;
        m_t2[tmap[s]] += pt[s].power * pt[s].power;
    }

    for (int k = 0; k < m_guesses; ++k) {
        const int target = m_crypto->compute(m_byte, k);
        const int weight = util::popcnt[target & m_mask] - m_center;
        real *tw = &m_tw[k * m_nevents], fw = (real)weight;

        if (weight != 0) {
            m_w1[k] += fw;
            m_w2[k] += fw * fw;

            for (size_t s = 0; s < pt.size(); ++s)
                tw[tmap[s]] += pt[s].power * fw;
        }
    }

    ++m_traces;
}

// -----------------------------------------------------------------------------
template <typename real>
void attack_cpa<real>::record_interval(size_t n)
{
    // compute differentials and interval maxes
    compute_diffs(&m_dtemp[0]);

    real *m = &m_maxes[n * m_guesses];
    for (int k = 0; k < m_guesses; ++k) {
        const size_t off = k * m_nevents;
        for (size_t i = 0; i < m_nevents; ++i)
            m[k] = max(m[k], m_dtemp[off + i]);
    }
}

// -----------------------------------------------------------------------------
template <typename real>
void attack_cpa<real>::coalesce(attack_instance *inst)
{
    attack_cpa *other = (attack_cpa *)inst;
    m_traces += other->m_traces;

    for (size_t s = 0; s < m_nevents; ++s) {
        m_t1[s] += other->m_t1[s];
        m_t2[s] += other->m_t2[s];
    }

    for (int k = 0; k < m_guesses; ++k) {
        m_w1[k] += other->m_w1[k];
        m_w2[k] += other->m_w2[k];

        const size_t offset = k * m_nevents;
        for (size_t s = 0; s < m_nevents; ++s)
            m_tw[offset + s] += other->m_tw[offset + s];
    }
}

// -----------------------------------------------------------------------------
template <typename real>
void attack_cpa<real>::get_diffs(vector<double> &diffs)
{
    compute_diffs(&m_dtemp[0]);

    diffs.resize(m_guesses * m_nevents);
    for (size_t i = 0; i < m_guesses * m_nevents; ++i)
        diffs[i] = m_dtemp[i];
}

// -----------------------------------------------------------------------------
template <typename real>
void attack_cpa<real>::get_maxes(vector<double> &maxes)
{
    maxes.resize(m_guesses * m_nreports);
    for (size_t i = 0; i < m_guesses * m_nreports; ++i)
        maxes[i] = m_maxes[i];
}

// -----------------------------------------------------------------------------
template <typename real>
void attack_cpa<real>::write_results(const string &path)
{
}

// -----------------------------------------------------------------------------
template <typename real>
bool attack_cpa<real>::cleanup()
{
    return true;
}

register_attack(cpa, attack_cpa<float>);
register_attack(cpa_dp, attack_cpa<double>);

