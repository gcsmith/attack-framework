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
    // TODO: implement me!
    return true;
}

// -----------------------------------------------------------------------------
bool trace_reader_simv::open(const string &path, const string &key, bool ct)
{
    if (!read_sim_timestamps(util::concat_name(path, "simulation.txt"), ct))
        return false;

    if (!read_sim_waveforms(util::concat_name(path, "power_waveform.out")))
        return false;

    m_wav_in.open(".mapped_trace_data");
    if (!m_wav_in.is_open()) {
        fprintf(stderr, "failed to open temporary waveform file\n");
        return false;
    }

    m_current = 0;
    return true;
}

// -----------------------------------------------------------------------------
bool trace_reader_simv::read_sim_timestamps(const string &path, bool ct)
{
    ifstream sim_in(path.c_str());
    if (!sim_in.is_open()) {
        fprintf(stderr, "failed to open sim file: %s\n", path.c_str());
        return false;
    }

    // read in the record for each simulated operation
    printf("parsing timestamps from \"%s\"...\n", path.c_str());

    while (getline(sim_in, m_line)) {
        const vector<string> tok(util::split(m_line));
        if (tok.size() < 3) {
            fprintf(stderr, "expected format: <index> <text_in> <text_out>\n");
            return false;
        }
        const string text = ct ? tok[2] : tok[1];
        m_records.push_back(record(strtoll(tok[0].c_str(), NULL, 10), text));
    }

    if (m_records.size() <= 1) {
        fprintf(stderr, "expected at least two traces in '%s'\n", path.c_str());
        return false;
    }

    return true;
}

// -----------------------------------------------------------------------------
bool trace_reader_simv::read_sim_waveforms(const std::string &path)
{
    ifstream wav_in(path.c_str());
    if (!wav_in.is_open()) {
        fprintf(stderr, "failed to open wav file: %s\n", path.c_str());
        return false;
    }

    ofstream wav_out(".mapped_trace_data");
    if (!wav_out.is_open()) {
        fprintf(stderr, "failed to create temporary waveform file\n");
        return false;
    }

    read_waveform_header(wav_in);
    read_waveform_samples(wav_in, wav_out);

    printf("%zu unique sample indices...\n", m_events.size());
    return true;
}

// -----------------------------------------------------------------------------
void trace_reader_simv::read_waveform_header(istream &wav_in)
{
    // read in the header from the waveform to determine the time resolution
    printf("Reading waveform header...\n");
    double resolution = 1.0;

    while (getline(wav_in, m_line)) {
        // skip comments and empty lines
        const size_t chars_read = m_line.size() + 1;
        util::trim(m_line);
        if (!m_line.length() || m_line[0] == ';')
            continue;

        // if this line is not a directive, assume data and break loop
        if (m_line[0] != '.') {
            for (size_t i = 0; i < chars_read; i++) wav_in.unget();
            break;
        }

        vector<string> tokens(util::split(m_line));
        if (tokens.size() == 2 && tokens[0] == ".time_resolution") {
            resolution = strtod(tokens[1].c_str(), NULL);
            printf("waveform time resolution is %f\n", resolution);
        }
    }

    // scale each timestamp by the resolution specified in the waveform file
    resolution = 1.0 / resolution;
    foreach (record &rec, m_records)
        rec.event = (uint64_t)(rec.event * resolution);
}

// -----------------------------------------------------------------------------
void trace_reader_simv::read_waveform_samples(istream &wav_in, ostream &wav_out)
{
    // read in the actual waveform data (event indices and power samples)
    printf("Reading waveform data and mapping trace events...\n");
    unsigned long curr = 0;
    bool read_timestamp = false;

    while (getline(wav_in, m_line)) {
        // skip empty lines, comments, and directives
        util::trim(m_line);
        if (!m_line.length() || m_line[0] == ';' || m_line[0] == '.')
            continue;

        size_t split_pos = m_line.find_first_of(' ');
        if (string::npos == split_pos) {
            uint64_t event_time = strtoll(m_line.c_str(), NULL, 10);
            if (event_time < m_records[curr].event) {
                printf("skipping event %llu\n", (long long unsigned)event_time);
                read_timestamp = false;
                continue;
            }
            else if (event_time >= m_records[curr + 1].event) {
                printf("scanned trace %lu\n", curr);
                wav_out << endl;
                if (++curr >= (m_records.size() - 1))
                    break;
            }
            read_timestamp = true;
            event_time -= m_records[curr].event;
            m_events.insert(event_time);
            wav_out << event_time << " ";
        }
        else if (m_line[0] == '1' && read_timestamp) {
            wav_out << util::trim_copy(m_line.substr(split_pos)) << " ";
        }
    }
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

    pt.clear();
    pt.set_text(util::atob(m_records[m_current++].text));
    trace::event_set::const_iterator event = m_events.begin();

    getline(m_wav_in, m_line);
    const vector<string> tokens(util::split(m_line));
    float last_power = 0.0f;

    for (size_t i = 0; i < tokens.size(); i += 2) {
        const long index = strtol(tokens[i].c_str(), NULL, 10);
        const float power = strtod(tokens[i + 1].c_str(), NULL);

#if 0
        if (pt.size() && index == pt.back().time) {
            printf("duplicate. summing\n");
            pt.back().power += power;
            continue;
        }
#endif

        while ((event != m_events.end()) && (*event < index))
            pt.push_back(trace::sample(*event++, last_power));

        pt.push_back(trace::sample(*event++, power));
        // XXX: last_power = power;
    }

    while (event != m_events.end())
        pt.push_back(trace::sample(*event++, last_power));

    return true;
}

