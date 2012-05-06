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

#ifndef TRACE_FORMAT_V3__H
#define TRACE_FORMAT_V3__H

#include <fstream>
#include <cstdio>
#include "trace_format.h"

class trace_reader_v3: public trace_reader {
public:
    bool summary(const std::string &path);
    bool open(const std::string &path, const std::string &key, bool ct);
    void close();
    bool read(trace &pt, const trace::time_range &range);
    size_t trace_count(void) { return m_texts.size(); }
    const trace::event_set &events(void) { return m_events; }

protected:
    typedef std::vector<uint8_t> text_t;
    trace::event_set    m_events;
    std::vector<text_t> m_texts;
    std::ifstream       m_wave_in;
    std::string         m_line;
    unsigned int        m_current;
};

class trace_writer_v3: public trace_writer {
public:
    bool open(const std::string &path, const std::string &key);
    void close(void);
    bool write(const trace &pt);

protected:
    FILE *m_text_out;
    FILE *m_wave_out;
};

#endif // TRACE_FORMAT_V3__H

