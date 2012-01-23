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

typedef std::set<long> event_set;
typedef std::pair<long, long> time_range;

struct sample
{
    sample() { }
    sample(uint32_t _time, float _power) : time(_time), power(_power) { }

    uint32_t time;
    float power;
};

enum TraceBinFmt
{
    FMT_PWR_S16,            // no time index, int16 sample
    FMT_PWR_F32,            // no time index, float32 sample
    FMT_IDX_U32_PWR_S16,    // uint32 time index, int16 sample
    FMT_IDX_U32_PWR_F32,    // uint32 time index, float32 sample
};

class trace
{
public:
    // TODO: description
    trace() { }

    // TODO: description
    trace(size_t n) : m_samples(n) { }

    // TODO: description
    const sample &operator[](size_t i) const { return m_samples[i]; }

    // TODO: description
    sample &operator[](size_t i) { return m_samples[i]; }

    // TODO: description
    void resize(size_t n) { m_samples.resize(n); }

    // TODO: description
    size_t size() const { return m_samples.size(); }

    // TODO: description
    void set_text(const std::vector<uint8_t> &txt);

    // TODO: description
    void set_text(const uint8_t *txt);

    // TODO: description
    const std::vector<uint8_t> &get_text() const { return m_text; }

    // TODO: description
    bool read_out(const std::string &i_path, const time_range &tr, event_set &ts);

    // TODO: description
    bool write_out(const std::string &o_path) const;

    // TODO: description
    bool read_agilent(const std::string &i_path, const time_range &tr);

    // TODO: description
    bool write_agilent(const std::string &o_path);

    // TODO: description
    bool read_csv(const std::string &i_path, const time_range &tr);

    // TODO: description
    bool write_csv(const std::string &o_path) const;

    // TODO: description
    bool read_bin(const std::string &i_path);

    // TODO: description
    bool write_bin(const std::string &o_path, TraceBinFmt fmt) const;

    // TODO: description
    void merge_events(event_set &events) const;

    // TODO: description
    void sample_and_hold(const trace &pt, const std::vector<long> &times);

    // TODO: description
    static bool read_profile(const std::string &i_path, std::vector<long> &ts);

    // TODO: description
    static bool write_profile(const std::string &o_path, const event_set &ts);

    // TODO: description
    static bool write_profile(const std::string &o_path, int start, int end);

protected:
    std::vector<sample> m_samples;
    std::vector<uint8_t> m_text;
};

#endif // TRACE__H

