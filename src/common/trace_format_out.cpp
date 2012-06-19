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
#include <cstdlib>
#include <cassert>
#include "utility.h"
#include "trace_format.h"

using namespace std;

// -----------------------------------------------------------------------------
class trace_reader_out: public trace_reader {
public:
    bool summary(const string &path) const;
    bool open(const string &path, const options &opt);
    void close();
    bool read(trace &pt);
    size_t trace_count(void) const             { return m_traces.size(); }
    const trace::event_set &events(void) const { return m_events; }

protected:
    bool read_waveform_data(const string &path, unsigned long min_time,
                            unsigned long max_time);

    trace::event_set m_events;  // set of unique power event timestamps
    vector<trace>    m_traces;  // power waveforms
    unsigned int     m_current; // current trace index for ::read()
};

// -----------------------------------------------------------------------------
// virtual
bool trace_reader_out::summary(const string &path) const
{
    // TODO: implement trace_reader_out::summary
    fprintf(stderr, "trace_reader_out::summary is not implemented\n");
    return true;
}

// -----------------------------------------------------------------------------
// virtual
bool trace_reader_out::open(const string &path, const options &opt)
{
    vector<string> paths;

    // get the full list of .out files located in the input directory
    if (!util::glob(path, ".*\\.out$", paths) || !paths.size()) {
        fprintf(stderr, "no trace found in directory: %s\n", path.c_str());
        return false;
    }

    // read in the waveform data for each scanned power trace
    foreach (const string &trace_path, paths) {
        if (!read_waveform_data(trace_path, opt.min_time, opt.max_time)) {
            fprintf(stderr, "error parsing trace '%s'\n", trace_path.c_str());
            return false;
        }
    }

    return true;
}

// -----------------------------------------------------------------------------
bool trace_reader_out::read_waveform_data(const string &path,
                                          unsigned long min_time,
                                          unsigned long max_time)
{
    // attempt to open and parse the trace file
    ifstream fin(path.c_str());
    if (!fin.is_open()) {
        fprintf(stderr, "unable to open '%s' for reading\n", path.c_str());
        return false;
    }

    // parse the plain/ciphertext from the trace filename
    const string name(util::path_stem(path));
    vector<uint8_t> text;

    foreach (const string &token, util::split(name, "_."))
        text.push_back(strtol(token.c_str(), NULL, 16));

    // initialize the trace object with the extracted message text
    m_traces.push_back(trace(text));
    trace &curr_trace = m_traces.back();

    string curr_line;
    while (getline(fin, curr_line)) {
        if (curr_line[0] == 'd') {
            // "done" indicates the end of the trace file
            break;
        }
        else if (curr_line[0] == '1' && curr_line[1] == ' ') {
            // power value, update the current trace entry
            if (curr_trace.size() <= 0) {
                fprintf(stderr, "error: read power sample before event time\n");
                return false;
            }
            curr_trace.back().power += strtof(&curr_line[2], NULL);
        }
        else {
            // time index -- add a new trace entry, break if max time reached
            const unsigned long sample_time = strtol(&curr_line[0], NULL, 10);
            if (min_time && sample_time < min_time) continue;
            if (max_time && sample_time > max_time) break;

            curr_trace.push_back(trace::sample(sample_time, 0.0));
            m_events.insert(sample_time);
        }
    }
    return true;
}

// -----------------------------------------------------------------------------
// virtual
void trace_reader_out::close()
{
    m_events.clear();
    m_traces.clear();
    m_current = 0;
}

// -----------------------------------------------------------------------------
// virtual
bool trace_reader_out::read(trace &pt)
{
    if (m_current >= m_traces.size())
        return false;

    // initialize the trace object with the previously extracted message text
    pt.clear();
    pt.set_text(m_traces[m_current].text());

    return trace_reader::copy_trace(m_traces[m_current++], pt, m_events);
}

register_trace_reader(out, trace_reader_out);

