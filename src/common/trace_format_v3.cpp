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
#include <sstream>
#include <cstdlib>
#include <cassert>
#include "utility.h"
#include "trace_format_v3.h"

using namespace std;

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
bool trace_reader_v3::summary(const string &path)
{
    return true;
}

// -----------------------------------------------------------------------------
bool trace_reader_v3::open(const string &path, const string &key, bool ct)
{
    const string text_file = ct ? "text_out.txt" : "text_in.txt";
    const string text_path = util::concat_name(path, text_file);
    const string wave_path = util::concat_name(path, "wave.txt");

    ifstream text_in(text_path.c_str());
    if (!text_in.is_open()) {
        fprintf(stderr, "failed to open text file '%s'\n", text_path.c_str());
        return false;
    }

    text_t text;
    while (getline(text_in, m_line) && parse_data(m_line, text, 16))
        m_texts.push_back(text);
    text_in.close();

    m_wave_in.open(wave_path.c_str());
    if (!m_wave_in.is_open()) {
        fprintf(stderr, "failed to open wave file '%s'\n", wave_path.c_str());
        return false;
    }

    m_current = 0;
    return true;
}

// -----------------------------------------------------------------------------
void trace_reader_v3::close()
{
    m_wave_in.close();
}

// -----------------------------------------------------------------------------
bool trace_reader_v3::read(trace &pt, const trace::time_range &range)
{
    if (m_current >= m_texts.size() ||
        !getline(m_wave_in, m_line) || !parse_data(m_line, m_wave, 10))
        return false;

    pt.clear();
    pt.set_text(m_texts[m_current++]);

    for (size_t i = 0; i < m_wave.size(); ++i) {
        pt.push_back(trace::sample(i, (float)m_wave[i]));
        m_events.insert(i);
    }

    return true;
}

// -----------------------------------------------------------------------------
bool trace_writer_v3::open(const string &path, const string &key)
{
    if (!util::valid_output_dir(path))
        return false;

    const string text_path = util::concat_name(path, "text_out.txt");
    const string wave_path = util::concat_name(path, "wave.txt");
    const string key_path  = util::concat_name(path, "key.txt");

    // open the plaintext or ciphertext file
    m_text = fopen(text_path.c_str(), "w");
    if (!m_text) {
        fprintf(stderr, "failed to open text file '%s'\n", text_path.c_str());
        return false;
    }

    // open the waveform file
    m_wave = fopen(wave_path.c_str(), "w");
    if (!m_wave) {
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
void trace_writer_v3::close()
{
    fclose(m_text);
    fclose(m_wave);
}

// -----------------------------------------------------------------------------
bool trace_writer_v3::write(const trace &pt)
{
    // write the plaintext or ciphertext for the current trace
    const vector<uint8_t> text(pt.text());
    for (size_t i = 0; i < text.size(); ++i)
        fprintf(m_text, "%02X ", text[i]);
    fprintf(m_text, "\n");

    // write each sample for this trace (one line per trace)
    for (size_t i = 0; i < pt.size(); ++i)
        fprintf(m_wave, "%d ", (int)pt[i].power);
    fprintf(m_wave, "\n");

    return true;
}

