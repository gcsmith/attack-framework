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

#include <fstream>
#include <cstdlib>
#include <cmath>
#include "attack_engine.h"
#include "attack_manager.h"

using namespace std;
using namespace util;

// -----------------------------------------------------------------------------
class attack_relpow: public attack_instance {
public:
    typedef float real;

    attack_relpow();
    ~attack_relpow();

    virtual bool setup(crypto_instance *crypto, const parameters &params);
    virtual void process(const time_map &tmap, const trace &pt);
    virtual void record_interval(size_t n);
    virtual void coalesce(attack_instance *inst);
    virtual void write_results(const string &path);
    virtual bool cleanup();

    virtual void get_diffs(vector<double> &diffs);
    virtual void get_maxes(vector<double> &maxes);

protected:
    size_t           m_nevents;  //!< number of traces processed
    size_t           m_nreports; //!< number of reports to compute
    int              m_bytes;    //!< number of bytes to correlate
    int              m_offset;   //!< offset of first byte to correlate
    vector<real>     m_pow;      //!< power consumption at each sensitive value
    vector<real>     m_num;      //!< number of samples for each sensitive value
    crypto_instance *m_crypto;   //!< crypto instance
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
    string key_string;
    if (!params.get("num_events", m_nevents) ||
        !params.get("num_reports", m_nreports) ||
        !params.get("bytes", m_bytes) ||
        !params.get("offset", m_offset) ||
        !params.get("key", key_string)) {
        fprintf(stderr, "required parameters: key, bytes, offset\n");
        return false;
    }

    const int key_length = crypto->key_bits() >> 3;
    const int num_states = crypto->estimate_bits() * m_bytes;

    vector<uint8_t> key(key_length);
    if (!util::atob(key_string, &key[0], key_length))
        return false;

    m_crypto = crypto;
    m_crypto->set_key(key);

    m_pow.resize(num_states, 0);
    m_num.resize(num_states, 0);

    return true;
}

// -----------------------------------------------------------------------------
void attack_relpow::process(const time_map &tmap, const trace &pt)
{
    // compute the sensitive value of the known secret information
    int sensitive_value = 0;
    for (int i = m_offset; i < (m_offset + m_bytes); ++i) {
        const int k = m_crypto->extract_estimate(i);
        sensitive_value += util::popcnt[m_crypto->compute(i, k)];
    }

    // compute the maximum/peak power consumption across the samples
    real power = 0.0;
    for (const trace::sample &sample : pt.samples())
        power = max(power, (real)sample.power);

    m_pow[sensitive_value] += power;
    m_num[sensitive_value] += 1;
}

// -----------------------------------------------------------------------------
void attack_relpow::record_interval(size_t n)
{
    // this attack does not produce interval reports
}

// -----------------------------------------------------------------------------
void attack_relpow::coalesce(attack_instance *inst)
{
    const int num_states = m_crypto->estimate_bits() * m_bytes;
    attack_relpow *other = (attack_relpow *)inst;

    for (int i = 0; i < num_states; ++i) {
        m_pow[i] += other->m_pow[i];
        m_num[i] += other->m_num[i];
    }
}

// -----------------------------------------------------------------------------
void attack_relpow::get_diffs(vector<double> &diffs)
{
    // this attack does not produce a differential trace
    diffs.clear();
}

// -----------------------------------------------------------------------------
void attack_relpow::get_maxes(vector<double> &maxes)
{
    // this attack does not produce a interval maxes
    maxes.clear();
}

// -----------------------------------------------------------------------------
void attack_relpow::write_results(const string &path)
{
    const string p_path = util::concat_name(path, "relpow_power.csv");
    const string t_path = util::concat_name(path, "relpow_traces.csv");
    const int num_states = m_crypto->estimate_bits() * m_bytes;

    // plot of sensitive value vs. power consumption
    ofstream fp_p(p_path.c_str());
    if (!fp_p.is_open()) {
        fprintf(stderr, "failed to open '%s' for writing\n", p_path.c_str());
        return;
    }
    printf("writing %s ...\n", p_path.c_str());

    // plot of sensitive value vs. number of traces
    ofstream fp_t(t_path.c_str());
    if (!fp_t.is_open()) {
        fprintf(stderr, "failed to open '%s' for writing\n", t_path.c_str());
        return;
    }
    printf("writing %s ...\n", t_path.c_str());

    for (int i = 0; i < num_states; ++i) {
        const double avg_pow = m_num[i] ? (m_pow[i] / m_num[i]) : 0;
        fp_p << i << "," << scientific << avg_pow << endl;
        fp_t << i << "," << m_num[i] << endl;
    }
}

// -----------------------------------------------------------------------------
bool attack_relpow::cleanup()
{
    return true;
}

register_attack(relpow, attack_relpow);

