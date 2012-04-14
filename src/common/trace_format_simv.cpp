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
#include <cstdlib>
#include <cassert>
#include "utility.h"
#include "trace_format_simv.h"

using namespace std;

// -----------------------------------------------------------------------------
bool trace_reader_simv::summary(const string &path)
{
    return true;
}

// -----------------------------------------------------------------------------
bool trace_reader_simv::open(const string &path, const string &key, bool ct)
{
    const string sim_path = util::concat_name(path, "simulation.txt");
    const string wav_path = util::concat_name(path, "power_waveform.out");

    ifstream sim_in(sim_path.c_str());
    if (!sim_in.is_open()) {
        fprintf(stderr, "failed to open file: %s\n", sim_path.c_str());
        return false;
    }

    // read in the record for each simulated encryption
    while (getline(sim_in, m_line)) {
        const vector<string> tok(util::split(util::trim(m_line), " "));
        if (tok.size() != 3) {
            fprintf(stderr, "expected format: <index> <text_in> <text_out>\n");
            return false;
        }
        const long long event = strtoll(tok[0].c_str(), NULL, 10) * 100;
        m_records.push_back(record(event, tok[1]));
    }

    // read in the waveform data and perform a pass to collect event indices
    m_wav_in.open(wav_path.c_str());
    if (!m_wav_in.is_open()) {
        fprintf(stderr, "failed to open file: %s\n", wav_path.c_str());
        return false;
    }
    scan_events();

    m_current = 0;
    return true;
}

// -----------------------------------------------------------------------------
void trace_reader_simv::scan_events(void)
{
    printf("Building trace index...\n");
    unsigned long curr = 0;

    while (getline(m_wav_in, m_line)) {
        // skip empty lines, comments, and directives
        util::trim(m_line);
        if (!m_line.length() || m_line[0] == ';' || m_line[0] == '.' ||
            string::npos != m_line.find_first_of(' '))
            continue;

        long long event = strtoll(m_line.c_str(), NULL, 10);
        if (curr < m_records.size() - 1 && event >= m_records[curr + 1].event)
            printf("Scanned trace %lu...\n", ++curr);

        // record the relative event index
        event -= m_records[curr].event;
        if (event >= 0) {
            m_events.insert(event);
        }
    }

    printf("%zu samples...\n", m_events.size());
    m_wav_in.clear();
    m_wav_in.seekg(0, ios::beg);
}

// -----------------------------------------------------------------------------
void trace_reader_simv::close()
{
    m_wav_in.close();
    m_current = 0;
}

// -----------------------------------------------------------------------------
bool trace_reader_simv::read(trace &pt, const trace::time_range &range)
{
    if (m_current >= m_records.size())
        return false;

    const record &rec = m_records[m_current++];
    pt.clear();
    pt.set_text(util::atob(rec.text));

    if (m_current > 1)
        pt.push_back(trace::sample(m_index - rec.event, 0.0));

    trace::event_set::const_iterator evt = m_events.begin();

    while (getline(m_wav_in, m_line)) {
        // skip empty lines, comments, and directives
        util::trim(m_line);
        if (!m_line.length() || m_line[0] == ';' || m_line[0] == '.')
            continue;

        size_t split_pos = m_line.find_first_of(' ');
        if (string::npos == split_pos) {
            // check if this event time belongs to the next trace
            m_index = strtoll(m_line.c_str(), NULL, 10);
            if ((m_current < m_records.size()) &&
                (m_index >= (m_records[m_current].event)))
                break;

            // only insert a new sample if the event index is unique
            m_index -= rec.event;
            if (pt.size() && (pt.back().time >= m_index)) {
                continue;
            }

            // compute the relative index
            if (m_index < 0) {
                printf("skip\n");
                continue;
            }

            if (*evt < m_index) {
                float power = pt.size() ? pt.back().power : 0.0f;
                while ((evt != m_events.end()) && (*evt < m_index))
                    pt.push_back(trace::sample(*evt++, power));
            }

            pt.push_back(trace::sample(m_index, 0.0));
            ++evt;
        }
        else {
            // power sample
            if (pt.size() <= 0) {
                fprintf(stderr, "error: read power sample before event time\n");
                continue;
            }
            if (m_line[0] == '1')
                pt.back().power += strtof(&m_line[split_pos + 1], NULL);
        }
    }

    float power = pt.size() ? pt.back().power : 0.0f;
    while (evt != m_events.end())
        pt.push_back(trace::sample(*evt++, power));

    printf("%zu samples\n", pt.size());
    return true;
}

