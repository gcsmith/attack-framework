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
#include <fstream>
#include <cstdlib>
#include <cassert>
#include "trace_format.h"
#include "utility.h"

using namespace std;

// -----------------------------------------------------------------------------
class trace_reader_v2: public trace_reader {
public:
    bool summary(const string &path) const;
    bool open(const string &path, const options &opt);
    void close(void);
    bool read(trace &pt);
    size_t trace_count(void) const             { return m_paths.size(); }
    const trace::event_set &events(void) const { return m_events; }

protected:
    trace::event_set m_events;
    vector<string>   m_paths;
    string           m_search;
    unsigned int     m_current;
    unsigned long    m_tmin;
    unsigned long    m_tmax;
};

// -----------------------------------------------------------------------------
// virtual
bool trace_reader_v2::summary(const string &path) const
{
    printf("describing: %s\n", path.c_str());

    vector<string> paths;
    if (!util::glob(path, ".*\\.csv$", paths)) {
        fprintf(stderr, "not a valid trace directory\n");
        return false;
    }

    set<string> keys;
    foreach (const string &filename, paths) {
        size_t beg = filename.find("k=");
        size_t end = filename.find("_", beg);
        if (beg != string::npos && end != string::npos)
            keys.insert(filename.substr(beg + 2, end - beg - 2));
    }

    printf("trace count: %zu\n", paths.size());
    printf("key count: %zu\n", keys.size());

    foreach (const string &key, keys)
        printf("    %s\n", key.c_str());

    return true;
}

// -----------------------------------------------------------------------------
// virtual
bool trace_reader_v2::open(const string &path, const options &opt)
{
    m_tmin = opt.min_time;
    m_tmax = opt.max_time;

    // get the full list of .csv files located in the input directory
    vector<string> paths;
    if (!util::glob(path, ".*\\.csv", paths) || !paths.size()) {
        fprintf(stderr, "no trace found in directory: %s\n", path.c_str());
        return false;
    }

    // cull the list down to traces matching the specified key
    const string search("k=" + opt.key);
    foreach (const string &filename, paths) {
        if (string::npos != filename.find(search))
            m_paths.push_back(filename);
    }

    if (!m_paths.size()) {
        fprintf(stderr, "no traces found matching key: %s\n", opt.key.c_str());
        return false;
    }

    // select search string based on whether we want plaintext or ciphertext
    m_search = opt.ciphertext ? "c=" : "m=";
    m_current = 0;
    return true;
}

// -----------------------------------------------------------------------------
// virtual
void trace_reader_v2::close()
{
    m_paths.clear();
    m_current = 0;
}

// -----------------------------------------------------------------------------
// virtual
bool trace_reader_v2::read(trace &pt)
{
    if (m_current >= m_paths.size())
        return false;

    const string path(m_paths[m_current++]);
    const string msg_str(path.substr(path.find(m_search) + 2, 32));

    ifstream fin(path.c_str());
    if (!fin.is_open()) {
        fprintf(stderr, "unable to open %s for reading\n", path.c_str());
        return false;
    }

    pt.clear();
    pt.set_text(util::atob(msg_str));

    string curr_line;
    unsigned long sample_time = 0;
    while (getline(fin, curr_line)) {
        // skip comments
        if (curr_line[0] == '#') continue;

        // only record samples within the specified time range
        if (m_tmin && sample_time < m_tmin) { ++sample_time; continue; }
        if (m_tmax && sample_time > m_tmax) break;

        float value = (float)strtol(curr_line.c_str(), NULL, 10);
        pt.push_back(trace::sample(sample_time, value));
        m_events.insert(sample_time++); // FIXME
    }

    return true;
}

register_trace_reader(v2, trace_reader_v2);

