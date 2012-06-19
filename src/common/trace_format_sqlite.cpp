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

#include <cstdio>
#include <cassert>
#include <sqlite3.h>
#include "trace_format.h"
#include "utility.h"

using namespace std;

// -----------------------------------------------------------------------------
class trace_reader_sqlite: public trace_reader {
public:
    bool summary(const string &path) const;
    bool open(const string &path, const options &opt);
    void close(void);
    bool read(trace &pt);
    size_t trace_count(void) const             { return m_count; }
    const trace::event_set &events(void) const { return m_events; }

protected:
    trace::event_set  m_events;
    sqlite3          *m_db;
    sqlite3_stmt     *m_stmt;
    size_t            m_count;
    unsigned long     m_tmin;
    unsigned long     m_tmax;
};

// -----------------------------------------------------------------------------
class trace_writer_sqlite: public trace_writer {
public:
    bool open(const string &path, const string &key, const trace::event_set &e);
    void close(void);
    bool write(const trace &pt);

protected:
    sqlite3      *m_db;
    sqlite3_stmt *m_stmt;
    string        m_key;
};

// -----------------------------------------------------------------------------
// virtual
bool trace_reader_sqlite::summary(const string &path) const
{
    // TODO: implement me
    return true;
}

// -----------------------------------------------------------------------------
// virtual
bool trace_reader_sqlite::open(const string &path, const options &opt)
{
    static const char *sql_length = "SELECT count(*) FROM data;";
    static const char *sql_select = "SELECT key,text,wave FROM data;";

    m_tmin = opt.min_time;
    m_tmax = opt.max_time;

    // attempt to open the database for read-only access
    int rc = sqlite3_open_v2(path.c_str(), &m_db, SQLITE_OPEN_READONLY, NULL);
    if (SQLITE_OK != rc) {
        fprintf(stderr, "failed to open database for reading (rc=%d)\n", rc);
        return false;
    }

    // determine the number of rows in the database befere we start reading
    if (SQLITE_OK != sqlite3_prepare_v2(m_db, sql_length, -1, &m_stmt, NULL)) {
        fprintf(stderr, "failed to prepare statement\n");
        return false;
    }

    sqlite3_step(m_stmt);
    m_count = (size_t)sqlite3_column_int(m_stmt, 0);
    sqlite3_finalize(m_stmt);

    // compile the select statement in advance for better performance
    if (SQLITE_OK != sqlite3_prepare_v2(m_db, sql_select, -1, &m_stmt, NULL)) {
        fprintf(stderr, "failed to prepare statement\n");
        return false;
    }

    return true;
}

// -----------------------------------------------------------------------------
// virtual
void trace_reader_sqlite::close(void)
{
    // close the database
    sqlite3_finalize(m_stmt);
    sqlite3_close(m_db);
}

// -----------------------------------------------------------------------------
// virtual
bool trace_reader_sqlite::read(trace &pt)
{
    if (SQLITE_ROW != sqlite3_step(m_stmt)) {
        fprintf(stderr, "error evaluating sqlite3 statement\n");
        return false;
    }

    // const char *key = (const char *)sqlite3_column_text(m_stmt, 0);
    const char *msg = (const char *)sqlite3_column_text(m_stmt, 1);
    const float *wav = (const float *)sqlite3_column_blob(m_stmt, 2);

    pt.clear();
    pt.set_text(util::atob(msg));

    // construct the power trace for the sample blob
    const int num_samples = sqlite3_column_bytes(m_stmt, 2);
    if (num_samples % 4) {
        fprintf(stderr, "sample blob should be 32-bit aligned!\n");
        return false;
    }

    for (unsigned long i = 0; i < (unsigned long)(num_samples >> 2); ++i) {
        if (m_tmin && i < m_tmin) continue;
        if (m_tmax && i > m_tmax) break;

        pt.push_back(trace::sample(i, wav[i]));
        m_events.insert(i);
    }

    return true;
}

// -----------------------------------------------------------------------------
// virtual
bool trace_writer_sqlite::open(const string &path, const string &key,
                               const trace::event_set &events)
{
    static const char *sql_create =
        "CREATE TABLE IF NOT EXISTS data (key TEXT, text TEXT, wave BLOB)";
    static const char *sql_insert = 
        "INSERT INTO data (key,text,wave) VALUES(?,?,?)";

    // attempt to open the database for writing (or creation)
    const int flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE;
    int rc = sqlite3_open_v2(path.c_str(), &m_db, flags, NULL);
    if (SQLITE_OK != rc) {
        fprintf(stderr, "failed to open database for writing (rc=%d)\n", rc);
        return false;
    }

    // create the table and start a transaction for bulk insertion
    sqlite3_exec(m_db, sql_create, NULL, NULL, NULL);
    sqlite3_exec(m_db, "PRAGMA synchronous = OFF;", NULL, NULL, NULL);
    sqlite3_exec(m_db, "PRAGMA journal_mode = MEMORY;", NULL, NULL, NULL);
    sqlite3_exec(m_db, "BEGIN TRANSACTION;", NULL, NULL, NULL);

    // compile the insertion statement in advance for better performance
    if (SQLITE_OK != sqlite3_prepare_v2(m_db, sql_insert, -1, &m_stmt, NULL)) {
        fprintf(stderr, "failed to prepare statement\n");
        return false;
    }

    m_key = key;
    return true;
}

// -----------------------------------------------------------------------------
// virtual
void trace_writer_sqlite::close()
{
    // complete the transaction and close the database
    sqlite3_finalize(m_stmt);
    sqlite3_exec(m_db, "COMMIT TRANSACTION;", NULL, NULL, NULL);
    sqlite3_close(m_db);
}

// -----------------------------------------------------------------------------
// virtual
bool trace_writer_sqlite::write(const trace &pt)
{
    const string text(util::btoa(pt.text()));

    vector<float> samples;
    for (size_t i = 0; i < pt.size(); ++i)
        samples.push_back(pt[i].power);
    const char *data = (const char *)&samples[0];

    sqlite3_bind_text(m_stmt, 1, m_key.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(m_stmt, 2, text.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_blob(m_stmt, 3, data, pt.size() * 4, SQLITE_TRANSIENT);

    sqlite3_step(m_stmt);
    sqlite3_clear_bindings(m_stmt);
    sqlite3_reset(m_stmt);
    return true;
}

register_trace_reader(sqlite, trace_reader_sqlite);
register_trace_writer(sqlite, trace_writer_sqlite);

