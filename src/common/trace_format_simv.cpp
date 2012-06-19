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

#include <cassert>
#include <cstdlib>
#include <fstream>
#include "trace_format.h"
#include "utility.h"

using namespace std;

// -----------------------------------------------------------------------------
class trace_reader_simv: public trace_reader {
public:
    bool summary(const string &path) const;
    bool open(const string &path, const options &opt);
    void close();
    bool read(trace &pt);
    size_t trace_count(void) const             { return m_traces.size(); }
    const trace::event_set &events(void) const { return m_events; }

protected:
    struct record {
        record(long long _event, const string &_text)
        : event(_event), text(_text) { }
        uint64_t event; // sample event index
        string   text;  // plaintext or ciphertext
    };

    bool read_timestamps(const string &path, bool ct);
    bool read_waveforms(const string &path, size_t base,
                        unsigned long min_time, unsigned long max_time);

    trace::event_set m_events;  // set of unique power event timestamps
    vector<record>   m_records; // simulation base event and text
    vector<trace>    m_traces;  // power waveforms
    unsigned long    m_current; // current trace index for ::read()
};

// -----------------------------------------------------------------------------
// virtual
bool trace_reader_simv::summary(const string &path) const
{
    // TODO: implement me
    return true;
}

// -----------------------------------------------------------------------------
// virtual
bool trace_reader_simv::open(const string &path, const options &opt)
{
    // process each pair of simulation(.*).txt and power_waveform(.*).out
    vector<string> globbed_paths;
    if (!util::glob(path, "simulation.*\\.txt$", globbed_paths))
        return false;

    foreach (const string &curr_path, globbed_paths) {
        // determine the waveform filename from the timestamp filename
        const string suffix = util::path_stem(curr_path).substr(10);
        const string waveform = "power_waveform" + suffix + ".out";
        const string wav_path = util::concat_name(path, waveform);
        const size_t record_base = m_records.size();

        // read in the event timestamps, then read and adjust the waveforms
        if (!read_timestamps(curr_path, opt.ciphertext) ||
            !read_waveforms(wav_path, record_base, opt.min_time, opt.max_time))
            return false;

        printf("scanned %zu traces from %s... \t%zu unique indices\n",
               m_records.size()-record_base, wav_path.c_str(), m_events.size());

        // only process the specified number of traces
        if (opt.num_traces && m_traces.size() >= opt.num_traces)
            break;
    }

    m_current = 0;
    return true;
}

// -----------------------------------------------------------------------------
bool trace_reader_simv::read_timestamps(const string &path, bool ct)
{
    ifstream sim_in(path.c_str());
    if (!sim_in.is_open()) {
        fprintf(stderr, "failed to open simulation file: %s\n", path.c_str());
        return false;
    }

    // read in the record for each simulated operation
    string curr_line;
    while (getline(sim_in, curr_line)) {
        const vector<string> tok(util::split(curr_line));
        if (tok.size() < 3) {
            fprintf(stderr, "expected format: <index> <text_in> <text_out>\n");
            return false;
        }

        // store the event time along with its corresponding plain/ciphertext
        const string &text = ct ? tok[2] : tok[1];
        m_records.push_back(record(strtoll(tok[0].c_str(), NULL, 10), text));
    }

    if (m_records.size() <= 1) {
        fprintf(stderr, "expected at least two traces in '%s'\n", path.c_str());
        return false;
    }

    return true;
}

// -----------------------------------------------------------------------------
bool trace_reader_simv::read_waveforms(const string &path, size_t base,
                                       unsigned long min_time,
                                       unsigned long max_time)
{
    ifstream wav_in(path.c_str());
    if (!wav_in.is_open()) {
        fprintf(stderr, "failed to open waveform file: %s\n", path.c_str());
        return false;
    }

    size_t record_index = base;
    bool read_timestamp = false;
    string curr_line;
    trace curr_trace;

    while (getline(wav_in, curr_line)) {
        // skip empty lines or lines containing comments
        util::trim(curr_line);
        if (!curr_line.length() || curr_line[0] == ';')
            continue;

        // any line beginning with a dot is part of the waveform header
        if (curr_line[0] == '.') {
            if (0 == curr_line.find(".time_resolution")) {
                const double resolution = 1.0 / strtod(&curr_line[16], NULL);
                for (size_t n = base; n < m_records.size(); ++n) {
                    const double event = m_records[n].event * resolution;
                    m_records[n].event = (uint64_t)event;
                }
            }
            continue;
        }

        // if the line isn't a comment or directive, assume it's data
        size_t split_pos = curr_line.find_first_of(' ');
        if (string::npos == split_pos) {
            // if the line contains a single value, it's an event timestamp
            uint64_t event_time = strtoull(curr_line.c_str(), NULL, 10);

            if (event_time < m_records[record_index].event) {
                // don't process events occurring before the current timestamp
                printf("skipping event %llu\n", (long long unsigned)event_time);
                read_timestamp = false;
                continue;
            }
            else if (event_time >= m_records[record_index + 1].event) {
                // if we've reached the next simulation, select its timestamp
                printf("scanned trace %lu\r", record_index);
                if (++record_index >= (m_records.size() - 1))
                    break;

                m_traces.push_back(curr_trace);
                curr_trace.clear();
            }

            // compute the sample time as the offset from the current timestamp
            event_time -= m_records[record_index].event;

            if ((min_time && event_time < min_time) ||
                (max_time && event_time > max_time)) { // NOTE: can NOT break!
                read_timestamp = false;
                continue;
            }

            curr_trace.push_back(trace::sample(event_time, 0.0f));

            // maintain a set of the unique event timestamps
            m_events.insert(event_time);
            read_timestamp = true;
        }
        else if (curr_line[0] == '1' && read_timestamp) {
            // only write out data for the top level events (pp_root)
            const string pow_str = util::trim_copy(curr_line.substr(split_pos));
            curr_trace.back().power += strtod(pow_str.c_str(), NULL);
        }
    }

    assert(m_records.size() == m_traces.size());
    return true;
}

// -----------------------------------------------------------------------------
// virtual
void trace_reader_simv::close()
{
    m_current = 0;
}

// -----------------------------------------------------------------------------
// virtual
bool trace_reader_simv::read(trace &pt)
{
    if (m_current >= m_traces.size())
        return false;

    // initialize the trace object with the message text from simulation.txt
    pt.clear();
    pt.set_text(util::atob(m_records[m_current].text));

    return trace_reader::copy_trace(m_traces[m_current++], pt, m_events);
}

register_trace_reader(simv, trace_reader_simv);

