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

#ifndef TRACE__H
#define TRACE__H

#include <stdint.h>
#include <vector>
#include <string>
#include <set>

enum TraceBinFormat
{
    FMT_PWR_S16,            // no time index, int16 sample
    FMT_PWR_F32,            // no time index, float32 sample
    FMT_IDX_U32_PWR_S16,    // uint32 time index, int16 sample
    FMT_IDX_U32_PWR_F32,    // uint32 time index, float32 sample
};

class trace {
public:
    typedef std::set<long> event_set;
    typedef std::vector<long> event_list;
    typedef std::pair<long, long> time_range;

    struct sample {
        sample(void) { }
        sample(uint32_t _time, float _power) : time(_time), power(_power) { }

        uint32_t time;
        float power;
    };

    /// default constructor - create width default size of 4096 samples
    trace(void) : m_samples(4096) { }

    /// create trace with specified number of samples
    trace(size_t n) : m_samples(n) { }

    // return constant reference to this trace's samples
    const std::vector<sample> &samples(void) const { return m_samples; }

    // access a specific sample index
    const sample &operator[](size_t i) const { return m_samples[i]; }

    // access a specific sample index
    sample &operator[](size_t i) { return m_samples[i]; }

    // return the number of samples in this trace
    size_t size(void) const { return m_samples.size(); }

    // remove all samples from the trace
    void clear(void) { m_samples.clear(); }

    // set the maximum number of samples in this trace
    void resize(size_t n) { m_samples.resize(n); }

    // add a new sample to the trace
    void push_back(const sample &data) { m_samples.push_back(data); }

    // set the plaintext or ciphertext value
    void set_text(const std::vector<uint8_t> &data) { m_text = data; } 

    // get the plaintext or ciphertext value
    const std::vector<uint8_t> &text(void) const { return m_text; }

    // set the sample binary format
    void set_format(TraceBinFormat fmt) { m_format = fmt; }

    // get the sample binary format
    TraceBinFormat format(void) const { return m_format; }

    // Expand trace to cover the specified list of sample events.
    void sample_and_hold(const trace &pt, const std::vector<long> &times);

    // Read in the trace timing profile (ordered list of event indices).
    static bool read_profile(const std::string &i_path, event_list &evt);

    // Write out the trace timing profile (ordered list of event indices).
    static bool write_profile(const std::string &o_path, const event_set &evt);

protected:
    std::vector<sample>  m_samples;
    std::vector<uint8_t> m_text;
    TraceBinFormat       m_format;
};

#endif // TRACE__H

