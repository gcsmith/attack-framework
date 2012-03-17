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
#include "trace_format_v2.h"

using namespace std;

// -----------------------------------------------------------------------------
bool trace_reader_v2::summary(const string &path)
{
    printf("describing: %s\n", path.c_str());

    util::pathlist paths;
    if (!util::scan_directory(path, ".csv", paths)) {
        fprintf(stderr, "not a valid trace directory\n");
        return false;
    }

    set<string> keys;
    for (util::pathlist::iterator i = paths.begin(); i != paths.end(); ++i) {
        size_t beg = i->find("k=");
        size_t end = i->find("_", beg);
        if (beg != string::npos && end != string::npos)
            keys.insert(i->substr(beg + 2, end - beg - 2));
    }

    printf("trace count: %zu\n", paths.size());
    printf("key count: %zu\n", keys.size());

    foreach (const string &key, keys)
        printf("    %s\n", key.c_str());

    return true;
}

// -----------------------------------------------------------------------------
bool trace_reader_v2::open(const string &path, const string &key, bool ct)
{
    // get the full list of .csv files located in the input directory
    util::pathlist paths;
    if (!util::scan_directory(path, ".csv", paths) || !paths.size()) {
        fprintf(stderr, "no trace found in directory: %s\n", path.c_str());
        return false;
    }

    // cull the list down to traces matching the specified key
    const string search("k=" + key);
    for (util::pathlist::iterator i = paths.begin(); i != paths.end(); ++i) {
        if (string::npos != i->find(search))
            m_paths.push_back(*i);
    }

    if (!m_paths.size()) {
        fprintf(stderr, "no traces found matching key: %s\n", key.c_str());
        return false;
    }

    // select search string based on whether we want plaintext or ciphertext
    m_search = ct ? "c=" : "m=";
    m_current = 0;
    return true;
}

// -----------------------------------------------------------------------------
void trace_reader_v2::close()
{
    m_paths.clear();
    m_current = 0;
}

// -----------------------------------------------------------------------------
bool trace_reader_v2::read(trace &pt, const trace::time_range &range)
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

    string line;
    long sample_time = 0;
    while (getline(fin, line)) {
        // skip comments
        if (line[0] == '#') continue;

        // only record samples within the specified time range
        sample_time++;
        if (range.first && sample_time < range.first)
            continue;
        if (range.second && sample_time >= range.second)
            break;

        float value = (float)strtol(line.c_str(), NULL, 10);
        pt.push_back(trace::sample(sample_time, value));
        m_events.insert(sample_time);
    }

    return true;
}

// -----------------------------------------------------------------------------
bool trace_writer_v2::open(const string &path, const string &key)
{
    assert(!"not implemented");
    return false;
}

// -----------------------------------------------------------------------------
void trace_writer_v2::close()
{
    assert(!"not implemented");
}

// -----------------------------------------------------------------------------
bool trace_writer_v2::write(const trace &pt)
{
    assert(!"not implemented");
    return false;
}

