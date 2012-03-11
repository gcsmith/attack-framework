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
class attack_relpow: public attack_instance {
public:
    attack_relpow();
    ~attack_relpow();

    virtual bool setup(crypto_instance *crypto, const parameters &params);
    virtual void process(const time_map &tmap, const trace &pt);
    virtual void record_interval(size_t n);
    virtual void coalesce(attack_instance *inst);
    virtual bool cleanup();

    virtual void get_diffs(vector<double> &diffs);
    virtual void get_maxes(vector<double> &maxes);
    virtual void get_group(vector<size_t> &group, int &ngroups);

protected:
    size_t m_nevents;       // number of traces processed
    size_t m_nreports;      // number of reports to compute
    int m_bytes;            // number of bytes to correlate
    int m_offset;           // offset of first byte to correlate
    vector<float> m_pow;    // power consumption at each sensitive value
    vector<float> m_num;    // number of samples for each sensitive value
    crypto_instance *m_crypto;
};

// -----------------------------------------------------------------------------
attack_relpow::attack_relpow()
{
}

// -----------------------------------------------------------------------------
attack_relpow::~attack_relpow()
{
}

// -----------------------------------------------------------------------------
bool attack_relpow::setup(crypto_instance *crypto, const parameters &params)
{
    if (!params.get("num_events", m_nevents) ||
        !params.get("num_reports", m_nreports) ||
        !params.get("bytes", m_bytes) ||
        !params.get("offset", m_offset))
        return false;

    const int key_bytes = crypto->key_bits() >> 3;
    vector<uint8_t> key(key_bytes);

    if (!util::atob(params["key"], &key[0], key_bytes))
        return false;

    m_crypto = crypto;
    m_crypto->set_key(key);

    m_pow.resize(256, 0);
    m_num.resize(256, 0);

    return true;
}

// -----------------------------------------------------------------------------
void attack_relpow::process(const time_map &tmap, const trace &pt)
{
    // compute the sensitive value of the entire 128-bit key
    int sensitive_value = 0;
    for (int i = m_offset; i < (m_offset + m_bytes); ++i) {
        const int k = m_crypto->extract_estimate(i);
        sensitive_value += crypto::popcnt[m_crypto->compute(i, k)];
    }

    // compute the maximum/peak power consumption across the samples
    float power = 0.0;
    for (size_t s = 0; s < pt.size(); ++s)
        power = max(power, pt[s].power);

    m_pow[sensitive_value] += power;
    m_num[sensitive_value] += 1;
}

// -----------------------------------------------------------------------------
void attack_relpow::record_interval(size_t n)
{
}

// -----------------------------------------------------------------------------
void attack_relpow::coalesce(attack_instance *inst)
{
    attack_relpow *other = (attack_relpow *)inst;

    for (int i = 0; i < 256; ++i) {
        m_pow[i] += other->m_pow[i];
        m_num[i] += other->m_num[i];
    }
}

// -----------------------------------------------------------------------------
void attack_relpow::get_diffs(vector<double> &diffs)
{
    ofstream fp_v("svalue_vs_voltage.csv");
    ofstream fp_t("svalue_vs_traces.csv");
    for (int i = 0; i < 256; ++i) {
        float avg_pow = m_num[i] ? (m_pow[i] / m_num[i]) : 0;
        fp_v << i << "," << scientific << avg_pow << endl;
        fp_t << i << "," << m_num[i] << endl;
    }
}

// -----------------------------------------------------------------------------
void attack_relpow::get_maxes(vector<double> &maxes)
{
}

// -----------------------------------------------------------------------------
void attack_relpow::get_group(vector<size_t> &group, int &ngroups)
{
    ngroups = 0;
}

// -----------------------------------------------------------------------------
bool attack_relpow::cleanup()
{
    return true;
}

register_attack(relpow, attack_relpow());

