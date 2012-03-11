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
#include "trace_format_packed.h"

using namespace std;

// -----------------------------------------------------------------------------
bool trace_reader_packed::open(const string &path, const string &key, bool ct)
{
    close();

    // attempt to open the trace archive for reading in binary mode
    m_input.open(path.c_str(), ios::binary);
    if (!m_input.is_open()) {
        fprintf(stderr, "unable to open bin file '%s'\n", path.c_str());
        return false;
    }

    // read in the header and version number, and verify that they are correct
    string header(8, 0);
    m_input.read(&header[0], 8);
    m_input.read((char *)&m_count, sizeof(uint32_t));

    if (header != "TRACE.20") {
        fprintf(stderr, "read invalid header %s\n", header.c_str());
        return false;
    }

    return true;
}

// -----------------------------------------------------------------------------
void trace_reader_packed::close(void)
{
    m_events.clear();
    if (m_input.is_open())
        m_input.close();
}

// -----------------------------------------------------------------------------
bool trace_reader_packed::read(trace &pt, const trace::time_range &range)
{
    trace::sample data;
    uint32_t num_samples = 0;
    vector<uint8_t> text(16, 0);

    // read in the number of samples and the plaintext/ciphertext
    m_input.read((char *)&num_samples, sizeof(uint32_t));
    m_input.read((char *)&text[0], 16);

    pt.clear();
    pt.set_text(text);

    // set the trace size and load in the sample data
    for (uint32_t i = 0; i < num_samples; ++i) {
        m_input.read((char *)&data.time, sizeof(uint32_t));
        m_input.read((char *)&data.power, sizeof(float));
        if (range.first && (long)data.time < range.first)
            continue;
        else if (range.second && (long)data.time >= range.second)
            break;

        pt.push_back(data);
        m_events.insert(data.time);
    }

    return true;
}

// -----------------------------------------------------------------------------
bool trace_writer_packed::open(const string &path, const string &key)
{
    m_output.open(path.c_str(), ios::binary);
    if (!m_output.is_open()) {
        fprintf(stderr, "failed to open bin file '%s'\n", path.c_str());
        return false;
    }

    // write the header and set the trace count to zero for now
    m_count = 0;
    m_output.write("TRACE.20", 8);
    m_output.write((const char *)&m_count, sizeof(uint32_t));

    return true;
}

// -----------------------------------------------------------------------------
void trace_writer_packed::close(void)
{
    // seek back to the beginning of the file and write the correct trace count
    m_output.seekp(8, ios::beg);
    m_output.write((const char *)&m_count, sizeof(uint32_t));
    m_output.close();
}

// -----------------------------------------------------------------------------
bool trace_writer_packed::write(const trace &pt)
{
    const vector<uint8_t> &text(pt.text());
    if (16 != text.size()) {
        fprintf(stderr, "expected 16 byte text, got %d\n", (int)text.size());
        return false;
    }

    uint32_t num_samples = pt.size();
    m_output.write((const char *)&num_samples, sizeof(uint32_t));
    m_output.write((const char *)&text[0], 16);

    for (size_t i = 0; i < pt.size(); ++i) {
        m_output.write((const char *)&pt[i].time, sizeof(uint32_t));
        m_output.write((const char *)&pt[i].power, sizeof(float));
    }

    ++m_count;
    return true;
}

