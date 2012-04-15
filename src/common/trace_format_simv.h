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

#ifndef TRACE_FORMAT_SIMV__H
#define TRACE_FORMAT_SIMV__H

#include <fstream>
#include "trace_format.h"

class trace_reader_simv: public trace_reader {
public:
    bool summary(const std::string &path);
    bool open(const std::string &path, const std::string &key, bool ct);
    void close();
    bool read(trace &pt, const trace::time_range &range);
    size_t trace_count(void) { return m_records.size() - 1; }
    const trace::event_set &events(void) { return m_events; }

protected:
    struct record {
        record(long long _event, const std::string &_text)
        : event(_event), text(_text) { }
        long long   event; // sample event index
        std::string text;  // plaintext or ciphertext
    };

    bool read_sim_timestamps(const std::string &path, bool ct);
    bool read_sim_waveforms(const std::string &path);
    void read_waveform_header(std::istream &fin);
    void read_waveform_samples(std::istream &fin, std::ostream &fout);

    trace::event_set    m_events;
    std::vector<record> m_records;
    std::string         m_line;
    std::ifstream       m_wav_in;
    unsigned long       m_current;
    long long           m_index;
};

#endif // TRACE_FORMAT_SIMV__H

