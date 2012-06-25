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
    size_t trace_count(void) const             { return m_traces; }
    const trace::event_set &events(void) const { return m_events; }

protected:
    struct record {
        record(uint64_t _event, const string &_text)
        : event(_event), text(_text) { }
        uint64_t event; //!< sample event index
        string   text;  //!< plaintext or ciphertext
    };

    bool read_timestamps(const string &path, bool ct);
    bool read_events(const string &path, size_t base);
    bool read_waveforms(const string &path, size_t base);

    options          m_opt;
    size_t           m_current; //!< current trace index for ::read()
    size_t           m_traces;  //!< maximum number of traces to process
    trace::event_set m_events;  //!< set of unique power event timestamps
    fstream          m_wavfile; //!< temporary waveform input file
    vector<record>   m_records; //!< simulation base event and text
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
    // the first pass will build the set of unique event indices and write a
    // temporary file containing one trace waveform per line. the second pass
    // will consume this temporary file and generate each complete trace object
    m_wavfile.open(".trace_reader_simv_temp_waveform", ios::out);
    if (!m_wavfile.is_open()) {
        fprintf(stderr, "failed to open temporary waveform file for writing\n");
        return false;
    }

    // process each pair of simulation(.*).txt and power_waveform(.*).out
    vector<string> globbed_paths;
    if (!util::glob(path, "simulation.*\\.txt$", globbed_paths))
        return false;

    m_opt = opt;
    m_current = m_traces = 0;

    foreach (const string &curr_path, globbed_paths) {
        // determine the waveform filename from the timestamp filename
        const string suffix = util::path_stem(curr_path).substr(10);
        const string waveform = "power_waveform" + suffix + ".out";
        const string wav_path = util::concat_name(path, waveform);
        const size_t rec_base = m_records.size();

        // read in the event timestamps, then read and adjust the waveforms
        if (!read_timestamps(curr_path, opt.ciphertext) ||
            !read_waveforms(wav_path, rec_base))
            return false;

        printf("scanned %zu traces from %s... \t%zu unique indices\n",
               m_records.size() - rec_base, wav_path.c_str(), m_events.size());

        // only process the specified number of traces
        if (opt.num_traces && m_traces >= opt.num_traces)
            break;
    }

    // close the temporary trace file, then re-open it for reading
    m_wavfile.close();
    m_wavfile.open(".trace_reader_simv_temp_waveform", ios::in);
    if (!m_wavfile.is_open()) {
        fprintf(stderr, "failed to open temporary waveform file for reading\n");
        return false;
    }
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
    const int id = ct ? 2 : 1;

    while (getline(sim_in, curr_line)) {
        const vector<string> tok(util::split(curr_line));
        if (tok.size() < 3) {
            fprintf(stderr, "expected format: <index> <text_in> <text_out>\n");
            return false;
        }

        // store the event time along with its corresponding plain/ciphertext
        m_records.push_back(record(strtoll(tok[0].c_str(), NULL, 10), tok[id]));
    }

    if (m_records.size() <= 1) {
        fprintf(stderr, "expected at least two traces in '%s'\n", path.c_str());
        return false;
    }

    return true;
}

// -----------------------------------------------------------------------------
bool trace_reader_simv::read_waveforms(const string &path, size_t base)
{
    ifstream wav_in(path.c_str());
    if (!wav_in.is_open()) {
        fprintf(stderr, "failed to open waveform file: %s\n", path.c_str());
        return false;
    }

    size_t record_index = base;
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

        // if the line isn't a comment or part of the header, assume it's data
        const size_t split_pos = curr_line.find_first_of(' ');
        if (string::npos == split_pos) {
            // if the line contains a single value, it's an event timestamp
            uint64_t event_time = strtoull(curr_line.c_str(), NULL, 10);

            if (event_time < m_records[record_index].event) {
                // don't process events occurring before the current timestamp
                printf("skipping event %llu\n", (long long unsigned)event_time);
                continue;
            }
            else if (event_time >= m_records[record_index + 1].event) {
                // dump the current trace data and select the next timestamp
                foreach (const trace::sample &sample, curr_trace.samples())
                    m_wavfile << sample.time << ' ' << sample.power << ' ';
                m_wavfile << endl;
                curr_trace.clear();
                ++m_traces;

                printf("scanned trace %lu\r", record_index);
                if ((m_opt.num_traces && (m_traces >= m_opt.num_traces)) ||
                    (++record_index >= (m_records.size() - 1)))
                    break;
            }

            // compute the sample time as the offset from the current timestamp
            event_time -= m_records[record_index].event;

            if ((m_opt.min_time && event_time < m_opt.min_time) ||
                (m_opt.max_time && event_time > m_opt.max_time))
                continue;

            if (!curr_trace.size() || curr_trace.back().time != event_time) {
                curr_trace.push_back(trace::sample(event_time, 0.0f));
                m_events.insert(event_time);
            }
        }
        else if (curr_line[0] == '1' && curr_trace.size()) {
            // only write out data for the top level events (pp_root)
            const string pow_str = util::trim_copy(curr_line.substr(split_pos));
            curr_trace.back().power += strtod(pow_str.c_str(), NULL);
        }
    }

    return true;
}

// -----------------------------------------------------------------------------
// virtual
void trace_reader_simv::close()
{
    m_wavfile.close();
    m_current = 0;
}

// -----------------------------------------------------------------------------
// virtual
bool trace_reader_simv::read(trace &pt)
{
    if (m_current >= m_traces)
        return false;

    // initialize the trace object with the message text from simulation.txt
    pt.clear();
    pt.set_text(util::atob(m_records[m_current].text));

    string line;
    getline(m_wavfile, line);

    trace curr_trace;
    trace::sample sample;
    istringstream iss(line);

    while (iss >> sample.time >> sample.power)
        curr_trace.push_back(sample);

    ++m_current;
    return trace_reader::copy_trace(curr_trace, pt, m_events);
}

register_trace_reader(simv, trace_reader_simv);

