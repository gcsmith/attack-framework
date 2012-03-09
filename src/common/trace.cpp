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
#include "utility.h"
#include "trace.h"

using namespace std;

// -----------------------------------------------------------------------------
void trace::sample_and_hold(const trace &pt, const vector<long> &times)
{
    const size_t num_events = times.size();
    const size_t num_samples = pt.size();
    float curr_power = 0.f;

    m_samples.resize(num_events);
    for (size_t i = 0, j = 0; i < num_events; ++i) {
        if ((j < num_samples) && (times[i] == (long)pt[j].time))
            curr_power = pt[j++].power;
        m_samples[i].time = times[i];
        m_samples[i].power = curr_power;
    }
}

// -----------------------------------------------------------------------------
bool trace::read_profile(const string &i_path, event_list &times)
{
    ifstream fin(i_path.c_str());
    if (!fin.is_open()) {
        fprintf(stderr, "failed to open %s for reading\n", i_path.c_str());
        return false;
    }

    times.clear();
    long event_time;
    while (fin >> event_time)
        times.push_back(event_time);

    return true;
}

// -----------------------------------------------------------------------------
bool trace::write_profile(const string &o_path, const event_set &evt)
{
    ofstream fout(o_path.c_str());
    if (!fout.is_open()) {
        fprintf(stderr, "failed to open %s for writing\n", o_path.c_str());
        return false;
    }

    // write out each unique time from the set
    foreach (event_set::value_type value, evt) fout << value << endl;
    return true;
}

