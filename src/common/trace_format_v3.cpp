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
#include <sstream>
#include <cstdlib>
#include <cassert>
#include "trace_format.h"
#include "utility.h"

using namespace std;

// -----------------------------------------------------------------------------
class trace_reader_v3: public trace_reader {
public:
    bool summary(const string &path) const;
    bool open(const string &path, const options &opt);
    void close();
    bool read(trace &pt);
    size_t trace_count(void) const             { return m_texts.size(); }
    const trace::event_set &events(void) const { return m_events; }

protected:
    typedef vector<uint8_t> text_t;
    trace::event_set m_events;
    vector<text_t>   m_texts;
    ifstream         m_wave_in;
    string           m_line;
    unsigned int     m_current;
    unsigned long    m_tmin;
    unsigned long    m_tmax;
};

// -----------------------------------------------------------------------------
class trace_writer_v3: public trace_writer {
public:
    bool open(const string &path, const string &key, const trace::event_set &e);
    void close(void);
    bool write(const trace &pt);

protected:
    FILE *m_text_out;
    FILE *m_wave_out;
};

// -----------------------------------------------------------------------------
template <typename T>
size_t parse_data(const string &input, vector<T> &data, int base)
{
    string item;
    istringstream iss(input);
    data.clear();
    while (iss >> item) data.push_back(strtol(item.c_str(), NULL, base));
    return data.size();
}

// -----------------------------------------------------------------------------
// virtual
bool trace_reader_v3::summary(const string &path) const
{
    // TODO: implement me
    return true;
}

// -----------------------------------------------------------------------------
// virtual
bool trace_reader_v3::open(const string &path, const options &opt)
{
    m_tmin = opt.min_time;
    m_tmax = opt.max_time;

    // build complete paths to the message text and waveform data files
    const string text_file = opt.ciphertext ? "text_out.txt" : "text_in.txt";
    const string text_path = util::concat_name(path, text_file);
    const string wave_path = util::concat_name(path, "wave.txt");

    ifstream text_in(text_path.c_str());
    if (!text_in.is_open()) {
        fprintf(stderr, "failed to open text file '%s'\n", text_path.c_str());
        return false;
    }

    text_t text;
    string curr_line, dummy;
    while (getline(text_in, curr_line) && parse_data(curr_line, text, 16))
        m_texts.push_back(text);
    text_in.close();

    m_wave_in.open(wave_path.c_str());
    if (!m_wave_in.is_open()) {
        fprintf(stderr, "failed to open wave file '%s'\n", wave_path.c_str());
        return false;
    }

    // consume the first line to determine the number of samples
    getline(m_wave_in, curr_line);
    istringstream iss(curr_line);
    unsigned long num_samples = 0;
    while (iss >> dummy) num_samples++;
    for (unsigned long s = 0; s < num_samples; ++s) m_events.insert(s);

    m_wave_in.seekg(0, ios::beg);
    m_current = 0;
    return true;
}

// -----------------------------------------------------------------------------
// virtual
void trace_reader_v3::close()
{
    m_wave_in.close();
}

// -----------------------------------------------------------------------------
// virtual
bool trace_reader_v3::read(trace &pt)
{
    if (m_current >= m_texts.size() || !getline(m_wave_in, m_line))
        return false;

    // read in the waveform sample data
    float item;
    vector<float> wave;
    istringstream iss(m_line);
    while (iss >> item) wave.push_back(item);

    pt.clear();
    pt.set_text(m_texts[m_current++]);

    for (size_t i = 0; i < wave.size(); ++i) {
        if (m_tmin && i < m_tmin) continue;
        if (m_tmax && i > m_tmax) break;

        pt.push_back(trace::sample(i, (float)wave[i]));
    }

    return true;
}

// -----------------------------------------------------------------------------
// virtual
bool trace_writer_v3::open(const string &path, const string &key,
                           const trace::event_set &events)
{
    if (!util::valid_output_directory(path))
        return false;

    const string text_path = util::concat_name(path, "text_in.txt");
    const string wave_path = util::concat_name(path, "wave.txt");
    const string key_path  = util::concat_name(path, "key.txt");

    // open the plaintext or ciphertext file
    m_text_out = fopen(text_path.c_str(), "w");
    if (!m_text_out) {
        fprintf(stderr, "failed to open text file '%s'\n", text_path.c_str());
        return false;
    }

    // open the waveform file
    m_wave_out = fopen(wave_path.c_str(), "w");
    if (!m_wave_out) {
        fprintf(stderr, "failed to open wave file '%s'\n", wave_path.c_str());
        return false;
    }

    // store the encryption key
    FILE *fkey = fopen(key_path.c_str(), "w");
    if (!fkey) {
        fprintf(stderr, "failed to open key file '%s'\n", key_path.c_str());
        return false;
    }

    for (size_t i = 0; i < key.length(); ++i)
        fprintf(fkey, !(i & 1) ? "%c" : "%c ", key[i]);
    fclose(fkey);

    return true;
}

// -----------------------------------------------------------------------------
// virtual
void trace_writer_v3::close()
{
    fclose(m_text_out);
    fclose(m_wave_out);
}

// -----------------------------------------------------------------------------
// virtual
bool trace_writer_v3::write(const trace &pt)
{
    // write the plaintext or ciphertext for the current trace
    const vector<uint8_t> text(pt.text());
    for (size_t i = 0; i < text.size(); ++i)
        fprintf(m_text_out, "%02X ", text[i]);
    fprintf(m_text_out, "\n");

    // write each sample for this trace (one line per trace)
    for (size_t i = 0; i < pt.size(); ++i)
        fprintf(m_wave_out, "%g ", pt[i].power);
    fprintf(m_wave_out, "\n");

    return true;
}

register_trace_reader(v3, trace_reader_v3);
register_trace_writer(v3, trace_writer_v3);

