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
class attack_pscc: public attack_instance {
public:
    typedef float real;

    attack_pscc();
    ~attack_pscc();

    virtual bool setup(crypto_instance *crypto, const parameters &params);
    virtual void process(const time_map &tmap, const trace &pt);
    virtual void record_interval(size_t n);
    virtual void coalesce(attack_instance *inst);
    virtual void write_results(const string &path);
    virtual bool cleanup();

    virtual void get_diffs(vector<double> &diffs);
    virtual void get_maxes(vector<double> &maxes);

protected:
    void compute_differentials(vector<double> &diffs);

    size_t           m_traces;   //!< number of traces processed
    size_t           m_nevents;  //!< number of unique trace events
    size_t           m_nreports; //!< number of reports to compute
    int              m_bytes;    //!< number of bytes to correlate
    int              m_offset;   //!< offset of first byte to correlate
    vector<real>     m_p1;       //!< sum of traces at each event
    vector<real>     m_p2;       //!< sum of squared traces at each event
    vector<real>     m_pw;       //!< sum of weighted traces at each event
    real             m_w1;       //!< sum of weights
    real             m_w2;       //!< sum of squared weights
    crypto_instance *m_crypto;   //!< crypto instance
};

// -----------------------------------------------------------------------------
attack_pscc::attack_pscc()
{
}

// -----------------------------------------------------------------------------
attack_pscc::~attack_pscc()
{
}

// -----------------------------------------------------------------------------
bool attack_pscc::setup(crypto_instance *crypto, const parameters &params)
{
    const int key_length = crypto->key_bits() >> 3;
    string key_string;

    if (!params.get("num_events", m_nevents) ||
        !params.get("num_reports", m_nreports) ||
        !params.get("bytes", m_bytes) ||
        !params.get("offset", m_offset) ||
        !params.get("key", key_string)) {
        fprintf(stderr, "required parameters: key, bytes, offset\n");
        return false;
    }


    vector<uint8_t> key(key_length);
    if (!util::atob(key_string, &key[0], key_length))
        return false;

    m_crypto = crypto;
    m_crypto->set_key(key);

    m_traces = 0;
    m_p1.resize(m_nevents, 0);
    m_p2.resize(m_nevents, 0);
    m_pw.resize(m_nevents, 0);
    m_w1 = 0;
    m_w2 = 0;

    return true;
}

// -----------------------------------------------------------------------------
void attack_pscc::process(const time_map &tmap, const trace &pt)
{
    // accumulate weight and weight^2 of the sensitive value
    real weight = 0;
    for (int i = m_offset; i < (m_offset + m_bytes); ++i) {
        const int k = m_crypto->extract_estimate(i);
        weight += util::popcnt[m_crypto->compute(i, k)];
    }

    m_w1 += weight;
    m_w2 += weight * weight;

    // accumulate power, power^2, and power*weight for each sample
    for (size_t s = 0; s < pt.size(); ++s) {
        m_p1[tmap[s]] += pt[s].power;
        m_p2[tmap[s]] += pt[s].power * pt[s].power;
        m_pw[tmap[s]] += pt[s].power * weight;
    }

    ++m_traces;
}

// -----------------------------------------------------------------------------
void attack_pscc::record_interval(size_t n)
{
}

// -----------------------------------------------------------------------------
void attack_pscc::coalesce(attack_instance *inst)
{
    attack_pscc *other = (attack_pscc *)inst;
    
    m_w1 += other->m_w1;
    m_w2 += other->m_w2;
    m_traces += other->m_traces;

    for (size_t s = 0; s < m_nevents; ++s) {
        m_p1[s] += other->m_p1[s];
        m_p2[s] += other->m_p2[s];
        m_pw[s] += other->m_pw[s];
    }
}

// -----------------------------------------------------------------------------
void attack_pscc::compute_differentials(vector<double> &diffs)
{
    const real n = (real)m_traces;
    const real dw = sqrt(n * m_w2 - m_w1 * m_w1);

    diffs.resize(m_nevents, 0);
    for (size_t s = 0; s < m_nevents; ++s) {
        real dn = n * m_pw[s] - m_p1[s] * m_w1;
        real dp = n * m_p2[s] - m_p1[s] * m_p1[s];
        real dd = dw * sqrt(dp);
        diffs[s] = (fabs(dd) > EPSILON) ? (dn / dd) : 0;
    }
}

// -----------------------------------------------------------------------------
void attack_pscc::get_diffs(vector<double> &diffs)
{
    compute_differentials(diffs);
}

// -----------------------------------------------------------------------------
void attack_pscc::get_maxes(vector<double> &maxes)
{
}

// -----------------------------------------------------------------------------
void attack_pscc::write_results(const string &path)
{
    const string s_path = util::concat_name(path, "pscc_sanity.txt");
    const string r_path = util::concat_name(path, "pscc_results.txt");

    // dump the contents of attack_pscc
    ofstream fp_s(s_path.c_str());
    if (!fp_s.is_open()) {
        fprintf(stderr, "failed to open '%s' for writing\n", s_path.c_str());
        return;
    }
    printf("writing %s ...\n", s_path.c_str());

    for (size_t s = 0; s < m_nevents; ++s) {
        fp_s << m_p1[s] << " " << m_p2[s] << " " << m_pw[s] << " " << m_w1
             << " " << m_w2 << " " << m_traces << " " << m_nevents << endl;
    }

    // write correlation coefficients for each event
    ofstream fp_r(r_path.c_str());
    if (!fp_r.is_open()) {
        fprintf(stderr, "failed to open '%s' for writing\n", r_path.c_str());
        return;
    }
    printf("writing %s ...\n", r_path.c_str());

    vector<double> diffs;
    compute_differentials(diffs);

    for (size_t s = 0; s < m_nevents; ++s)
        fp_r << s << "," << scientific << diffs[s] << endl;
}

// -----------------------------------------------------------------------------
bool attack_pscc::cleanup()
{
    return true;
}

register_attack(pscc, attack_pscc);

