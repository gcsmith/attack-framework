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
#include <cassert>
#include <cstdio>
#include "trace_format.h"
#include "utility.h"

using namespace std;

// -----------------------------------------------------------------------------
class trace_reader_packed: public trace_reader {
public:
    bool summary(const string &path) const;
    bool open(const string &path, const options &opt);
    void close(void);
    bool read(trace &pt);
    size_t trace_count(void) const             { return m_ntraces; }
    const trace::event_set &events(void) const { return m_events; }

protected:
    trace::event_set m_events;
    vector<uint32_t> m_times;
    ifstream         m_input;
    uint32_t         m_textlen;
    uint32_t         m_ntraces;
    unsigned long    m_tmin;
    unsigned long    m_tmax;
    size_t           m_current;
};

// -----------------------------------------------------------------------------
class trace_writer_packed: public trace_writer {
public:
    bool open(const string &path, const string &key, const trace::event_set &e);
    void close(void);
    bool write(const trace &pt);

protected:
    ofstream m_output;
    uint32_t m_textlen;
    uint32_t m_ntraces;
    uint32_t m_samples;
};

// -----------------------------------------------------------------------------
// virtual
bool trace_reader_packed::summary(const string &path) const
{
    // TODO: implement me
    fprintf(stderr, "trace_reader_packed::summary is not implemented\n");
    return true;
}

// -----------------------------------------------------------------------------
// virtual
bool trace_reader_packed::open(const string &path, const options &opt)
{
    close();
    m_tmin = opt.min_time;
    m_tmax = opt.max_time;

    // attempt to open the trace archive for reading in binary mode
    m_input.open(path.c_str(), ios::binary);
    if (!m_input.is_open()) {
        fprintf(stderr, "unable to open bin file '%s'\n", path.c_str());
        return false;
    }

    // read in the header and version number, and verify that they are correct
    string header(8, 0);
    m_input.read(&header[0], 8);

    if (header != "TRACE.30") {
        fprintf(stderr, "read invalid header %s\n", header.c_str());
        return false;
    }

    // read the text length, trace count, and sample/event count
    uint32_t num_samples = 0;
    m_input.read((char *)&m_textlen, sizeof(uint32_t));
    m_input.read((char *)&m_ntraces, sizeof(uint32_t));
    m_input.read((char *)&num_samples, sizeof(uint32_t));

    // read in the complete set of event times
    m_times.resize(num_samples);
    m_input.read((char *)&m_times[0], sizeof(uint32_t) * num_samples);

    foreach (uint32_t event_time, m_times) {
        if (m_tmin && event_time < m_tmin) continue;
        if (m_tmax && event_time > m_tmax) break;
        m_events.insert(event_time);
    }

    if (opt.num_traces > 0) {
        // if specified, limit the maximum number of traces to process
        m_ntraces = min(m_ntraces, (uint32_t)opt.num_traces);
    }

    m_current = 0;
    return true;
}

// -----------------------------------------------------------------------------
// virtual
void trace_reader_packed::close(void)
{
    m_events.clear();
    m_times.clear();

    if (m_input.is_open())
        m_input.close();
}

// -----------------------------------------------------------------------------
// virtual
bool trace_reader_packed::read(trace &pt)
{
    if (m_current >= m_ntraces)
        return false;

    // read in the message text
    vector<uint8_t> text(m_textlen, 0);
    m_input.read((char *)&text[0], sizeof(uint8_t) * m_textlen);

    pt.clear();
    pt.set_text(text);

    // read in the sample data
    trace::sample data;
    foreach (uint32_t event_time, m_times) {
        m_input.read((char *)&data.power, sizeof(trace::real));

        if (m_tmin && data.time < m_tmin) continue;
        if (m_tmax && data.time > m_tmax) continue; // NOTE: can NOT break!

        data.time = event_time;
        pt.push_back(data);
    }

    return true;
}

// -----------------------------------------------------------------------------
// virtual
bool trace_writer_packed::open(const string &path, const string &key,
                               const trace::event_set &events)
{
    m_output.open(path.c_str(), ios::binary);
    if (!m_output.is_open()) {
        fprintf(stderr, "failed to open bin file '%s'\n", path.c_str());
        return false;
    }

    // write the header and set the text length and trace count to zero for now
    m_textlen = 0;
    m_ntraces = 0;
    m_samples = events.size();

    m_output.write("TRACE.30", 8);
    m_output.write((const char *)&m_textlen, sizeof(uint32_t));
    m_output.write((const char *)&m_ntraces, sizeof(uint32_t));
    m_output.write((const char *)&m_samples, sizeof(uint32_t));

    // write the complete set of event times immediately before the samples
    foreach (uint32_t event_time, events)
        m_output.write((const char *)&event_time, sizeof(uint32_t));

    return true;
}

// -----------------------------------------------------------------------------
// virtual
void trace_writer_packed::close(void)
{
    // seek back to the beginning of the file and write the correct counts
    m_output.seekp(8, ios::beg);
    m_output.write((const char *)&m_textlen, sizeof(uint32_t));
    m_output.write((const char *)&m_ntraces, sizeof(uint32_t));
    m_output.close();
}

// -----------------------------------------------------------------------------
// virtual
bool trace_writer_packed::write(const trace &pt)
{
    if (!m_textlen) {
        // keep track for the next iteration... it should never change
        m_textlen = (uint32_t)pt.text().size();
    }
    else if (m_textlen != pt.text().size()) {
        fprintf(stderr, "invalid text length: got %zu, expected %zu\n",
                pt.text().size(), (size_t)m_textlen);
        return false;
    }

    if (m_samples != pt.samples().size()) {
        fprintf(stderr, "invalid sample count: got %zu, expected %zu\n",
                pt.size(), (size_t)m_samples);
        return false;
    }

    // write the message text followed by the power waveform
    m_output.write((const char *)&pt.text()[0], sizeof(uint8_t) * m_textlen);

    foreach (const trace::sample &sample, pt.samples())
        m_output.write((const char *)&sample.power, sizeof(trace::real));

    ++m_ntraces;
    return true;
}

register_trace_reader(packed, trace_reader_packed);
register_trace_writer(packed, trace_writer_packed);

