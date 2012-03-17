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

#ifndef TRACE_FORMAT_SQLITE__H
#define TRACE_FORMAT_SQLITE__H

#include <sqlite3.h>
#include "trace_format.h"

class trace_reader_sqlite: public trace_reader {
public:
    bool summary(const std::string &path);
    bool open(const std::string &path, const std::string &key, bool ct);
    void close(void);
    bool read(trace &pt, const trace::time_range &range);
    size_t trace_count(void) { return m_count; }
    const trace::event_set &events(void) { return m_events; }

protected:
    trace::event_set  m_events;
    sqlite3          *m_db;
    sqlite3_stmt     *m_stmt;
    size_t            m_count;
};

class trace_writer_sqlite: public trace_writer {
public:
    bool open(const std::string &path, const std::string &key);
    void close(void);
    bool write(const trace &pt);

protected:
    sqlite3      *m_db;
    sqlite3_stmt *m_stmt;
    std::string   m_key;
};

#endif // TRACE_FORMAT_SQLITE__H

