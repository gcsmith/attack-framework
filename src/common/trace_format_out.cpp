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

#include <vector>
#include <cassert>
#include "trace_format_out.h"

using namespace std;

// -----------------------------------------------------------------------------
bool trace_reader_out::open(const string &path, const string &key, bool ct)
{
    // get the full list of .out files located in the input directory
    util::scan_directory(path, ".out", m_paths);
    m_current = 0;
    return true;
}

// -----------------------------------------------------------------------------
void trace_reader_out::close()
{
    m_paths.clear();
    m_current = 0;
}

// -----------------------------------------------------------------------------
bool trace_reader_out::read(trace &pt, const trace::time_range &range)
{
    if (m_current >= m_paths.size())
        return false;

    const string path(m_paths[m_current++]);
    const string name(util::base_name(path));

    // parse and validate the plain/ciphertext from the trace filename
    vector<uint8_t> text;
    foreach (const string &token, util::split(name, "_."))
        text.push_back(strtol(token.c_str(), NULL, 16));

    if (text.size() != 16) {
        fprintf(stderr, "invalid plain/ciphertext '%s'\n", name.c_str());
        return false;
    }

    // attempt to open and parse the trace file
    ifstream fin(path.c_str());
    if (!fin.is_open()) {
        fprintf(stderr, "unable to open %s for reading\n", path.c_str());
        return false;
    }

    pt.set_text(text);
    pt.clear();

    string line;
    while (getline(fin, line)) {
        if (line[0] == 'd') {
            // "done" indicates the end of the trace file
            break;
        }
        else if (line[0] == '2' && line[1] == ' ') {
            // power value, update the current trace entry
            if (pt.size() <= 0) {
                fprintf(stderr, "warning: read power sample before time\n");
                continue;
            }
            pt[pt.size() - 1].power += strtof(&line[2], NULL);
        }
        else {
            // time index -- add a new trace entry, break if max time reached
            long sample_time = strtol(&line[0], NULL, 10);
            if (range.first && sample_time < range.first)
                continue;
            if (range.second && sample_time >= range.second)
                break;

            pt.push_back(trace::sample(sample_time, 0.0));
            m_events.insert(sample_time);
        }
    }

    return true;

}

// -----------------------------------------------------------------------------
bool trace_writer_out::open(const string &path, const string &key)
{
    assert(!"not implemented");
    return false;
}

// -----------------------------------------------------------------------------
void trace_writer_out::close()
{
    assert(!"not implemented");
}

// -----------------------------------------------------------------------------
bool trace_writer_out::write(const trace &pt)
{
    assert(!"not implemented");
    return false;
}

