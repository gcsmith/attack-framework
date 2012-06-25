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
class trace_writer_csv: public trace_writer {
public:
    bool open(const string &path, const string &key, const trace::event_set &e);
    void close(void);
    bool write(const trace &pt);

protected:
    ofstream m_output;
    string m_path;
    size_t m_count;
};

// -----------------------------------------------------------------------------
// virtual
bool trace_writer_csv::open(const string &path, const string &key,
                            const trace::event_set &events)
{
    if (!util::valid_output_directory(path))
        return false;

    m_path = path;
    m_count = 0;
    return true;
}

// -----------------------------------------------------------------------------
// virtual
void trace_writer_csv::close(void)
{
}

// -----------------------------------------------------------------------------
// virtual
bool trace_writer_csv::write(const trace &pt)
{
    const string filename = util::btoa(pt.text()) + ".csv";
    const string filepath = util::concat_name(m_path, filename);

    ofstream fout(filepath.c_str());
    if (!fout.is_open()) {
        fprintf(stderr, "failed to open %s for writing\n", filename.c_str());
        return false;
    }

    foreach (const trace::sample &sample, pt.samples())
        fout << sample.time << "," << sample.power << ",\n";

    return true;
}

register_trace_writer(csv, trace_writer_csv);

