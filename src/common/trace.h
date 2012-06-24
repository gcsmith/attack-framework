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
#include <set>

class trace {
public:
    typedef std::set<uint32_t> event_set;
    typedef float real;

    struct sample {
        sample(void) { }
        sample(uint32_t _time, real _power) : time(_time), power(_power) { }

        uint32_t time;
        real power;
    };

    //! create an empty trace
    trace(void) { }

    //! create an empty trace with the specified message text
    trace(const std::vector<uint8_t> &text) : m_text(text) { }

    //! initialize a trace with the specified message text and sample data
    trace(const std::vector<uint8_t> &text, const std::vector<sample> &samples)
    : m_text(text), m_samples(samples) { }

    //! set the plaintext or ciphertext value
    void set_text(const std::vector<uint8_t> &data) { m_text = data; } 

    //! get the plaintext or ciphertext value
    const std::vector<uint8_t> &text(void) const { return m_text; }

    //! set the sample data from an existing vector
    void set_samples(const std::vector<sample> &data) { m_samples = data; }

    //! return constant reference to this trace's samples
    const std::vector<sample> &samples(void) const { return m_samples; }

    //! access a specific sample index (read-only)
    const sample &operator[](size_t i) const { return m_samples[i]; }

    //! access a specific sample index
    sample &operator[](size_t i) { return m_samples[i]; }

    //! add a new sample to the trace
    void push_back(const sample &data) { m_samples.push_back(data); }

    //! access the last sample
    sample &back(void) { return m_samples.back(); }

    //! return the number of samples in this trace
    size_t size(void) const { return m_samples.size(); }

    //! remove all samples from the trace
    void clear(void) { m_samples.clear(); }

    //! set the maximum number of samples in this trace
    void resize(size_t n) { m_samples.resize(n); }

protected:
    std::vector<uint8_t> m_text;
    std::vector<sample>  m_samples;
};

#endif // TRACE__H

