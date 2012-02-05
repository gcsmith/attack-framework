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
#include <cstdio>
#include <cstdlib>
#include <cassert>
#include "trace.h"
#include "utility.h"
#include "agilent_reader.h"

using namespace std;

// -----------------------------------------------------------------------------
void trace::set_text(const std::vector<uint8_t> &txt)
{
    m_text = txt;
}

// -----------------------------------------------------------------------------
// Reads in an ASCII formatted trace file, and stores its corresponding sample
// points in 'td'. If 'tmax' is greater than zero, samples beyond this time,
// specified in ns, will be discarded.
bool trace::read_out(const string &i_path, const time_range &tr, event_set &ts)
{
    ifstream fin(i_path.c_str());
    if (!fin.is_open()) {
        fprintf(stderr, "unable to open %s for reading\n", i_path.c_str());
        return false;
    }

    string line;
    long last_time = -1;
    m_samples.clear();
    while (getline(fin, line)) {
        if (line[0] == 'd') {
            // "done" indicates the end of the trace file
            break;
        }
        else if (line[0] == '2' && line[1] == ' ') {
            // power value, update the current trace entry
            if (/*!m_samples.size()*/ last_time < 0) {
                fprintf(stderr, "warning: read power sample before time\n");
                continue;
            }
            m_samples.back().power += strtof(&line[2], NULL);
        }
        else {
            // time index -- add a new trace entry, break if max time reached
            long t = strtol(&line[0], NULL, 10);
            /*XXX*/last_time = t;
            if (tr.first && t < tr.first) continue;
            if (tr.second && t >= tr.second) break;

            m_samples.push_back(sample(t, 0.0));
            ts.insert(t);
        }
    }

    return true;
}

// -----------------------------------------------------------------------------
bool trace::write_out(const string &o_path) const
{
    return false;
}

// -----------------------------------------------------------------------------
bool trace::read_agilent(const std::string &i_path, const time_range &tr)
{
    FILE *fp = fopen(i_path.c_str(), "rb");
    if (NULL == fp) {
        fprintf(stderr, "failed to open %s for reading\n", i_path.c_str());
        return false;
    }

    tPBFileHeader fh;
    if (!ReadAgilentHeader(fp, &fh) || (fh.NumberOfWaveforms != 1)) {
        fprintf(stderr, "failed to read file header: %s\n", i_path.c_str());
        return false;
    }

    tPBWaveformHeader wh;
    if (!ReadWaveformHeader(fp, &wh)) {
        fprintf(stderr, "failed to read waveform header: %s\n", i_path.c_str());
        return false;
    }

    if ((PB_NORMAL != wh.WaveformType) && (PB_AVERAGE != wh.WaveformType)) {
        fprintf(stderr, "invalid waveform type (%d)\n", wh.WaveformType);
        return false;
    }

    float *data = ReadAnalogWaveform(fp, &wh, NULL);
    if (NULL == data) {
        fprintf(stderr, "failed to read analog waveform\n");
        return false;
    }

    long time_val = 0;
    m_samples.clear();
    for (int i = 0; i < wh.Points; ++i) {
        // only record samples within the specified time range
        int t = time_val++;
        if (tr.first && t < tr.first) continue;
        if (tr.second && t >= tr.second) break;

        m_samples.push_back(sample(time_val, data[time_val])); 
    }

    free(data);
    fclose(fp);
    return true;
}

// -----------------------------------------------------------------------------
bool trace::write_agilent(const std::string &o_path)
{
    return false;
}

// -----------------------------------------------------------------------------
bool trace::read_csv(const string &i_path, const time_range &tr)
{
    ifstream fin(i_path.c_str());
    if (!fin.is_open()) {
        fprintf(stderr, "unable to open %s for reading\n", i_path.c_str());
        return false;
    }

    // clear out the sample buffer in case this trace is re-used
    m_samples.clear();

    string line;
    long time_val = 0;
    while (getline(fin, line)) {
        // skip comments
        if (line[0] == '#') continue;

        // only record samples within the specified time range
        int t = time_val++;
        if (tr.first && t < tr.first) continue;
        if (tr.second && t >= tr.second) break;

        float value = (float)strtol(line.c_str(), NULL, 10);
        m_samples.push_back(sample(t, value));
    }

    return true;
}

// -----------------------------------------------------------------------------
bool trace::write_csv(const string &o_path) const
{
    ofstream fout(o_path.c_str());
    if (!fout.is_open()) {
        fprintf(stderr, "unable to open %s for writing\n", o_path.c_str());
        return false;
    }

    size_t num_traces = m_samples.size();
    for (size_t i = 0; i < num_traces; ++i)
        fout << m_samples[i].time << "," << m_samples[i].power << endl;

    return true;
}

// -----------------------------------------------------------------------------
// Read in samples from a binary formatted trace object file.
bool trace::read_bin(const string &i_path)
{
    string header(8, 0);
    uint32_t num_samples = 0, format = 0;

    // attempt to open the trace object for reading in binary mode
    ifstream fin(i_path.c_str());
    if (!fin.is_open()) {
        fprintf(stderr, "unable to open %s for reading\n", i_path.c_str());
        return false;
    }

    // read in the header and version number, and verify that they are correct
    fin.read(&header[0], 8);
    fin.read((char *)&format, sizeof(uint32_t));
    fin.read((char *)&num_samples, sizeof(uint32_t));

    if (header != "TRACE.00") {
        fprintf(stderr, "read invalid header %s\n", header.c_str());
        return false;
    }

    // read in the # of samples specified in the header in [time,power] format
    m_samples.resize(num_samples);

    switch (format) {
    case FMT_PWR_S16:
        for (size_t i = 0; i < num_samples; ++i) {
            int16_t power;
            fin.read((char *)&power, sizeof(int16_t));
            m_samples[i].time = i;
            m_samples[i].power = (float)power;
        }
        break;
    case FMT_PWR_F32:
        for (size_t i = 0; i < num_samples; ++i) {
            float power;
            fin.read((char *)&power, sizeof(float));
            m_samples[i].time = i;
            m_samples[i].power = power;
        }
        break;
    case FMT_IDX_U32_PWR_F32:
        for (size_t i = 0; i < num_samples; ++i) {
            fin.read((char *)&m_samples[i].time, sizeof(uint32_t));
            fin.read((char *)&m_samples[i].power, sizeof(float));
        }
        break;
    default:
        fprintf(stderr, "invalid trace file format (%02X)\n", format);
        return false;
    }

    return true;
}

// -----------------------------------------------------------------------------
// Write the samples from 'td' to a binary formatted trace object file.
bool trace::write_bin(const string &o_path, TraceBinFmt fmt) const
{
    ofstream fout(o_path.c_str(), ios::binary);
    if (!fout.is_open()) {
        fprintf(stderr, "unable to open %s for writing\n", o_path.c_str());
        return false;
    }

    // write out the header: [TRACE.][VV][fmt][#samples][data..]
    uint32_t num_traces = m_samples.size(), format = (uint32_t)fmt;
    fout.write("TRACE.00", 8);
    fout.write((char *)&format, sizeof(uint32_t));
    fout.write((char *)&num_traces, sizeof(uint32_t));

    switch (fmt) {
    case FMT_PWR_S16:
        for (size_t i = 0; i < num_traces; ++i) {
            int16_t power = (int16_t)m_samples[i].power;
            fout.write((char *)&power, sizeof(int16_t));
        }
        break;
    case FMT_PWR_F32:
        for (size_t i = 0; i < num_traces; ++i) {
            float power = (float)m_samples[i].power;
            fout.write((char *)&power, sizeof(float));
        }
        break;
    case FMT_IDX_U32_PWR_F32:
        for (size_t i = 0; i < num_traces; ++i) {
            fout.write((char *)&m_samples[i].time, sizeof(uint32_t));
            fout.write((char *)&m_samples[i].power, sizeof(float));
        }
        break;
    default:
        fprintf(stderr, "invalid trace file format (%02X)\n", format);
        return false;
    }

    return true;
}

// -----------------------------------------------------------------------------
void trace::merge_events(event_set &events) const
{
    vector<sample>::const_iterator i;
    for (i = m_samples.begin(); i != m_samples.end(); ++i)
        events.insert(i->time);
}

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
bool trace::read_profile(const string &i_path, vector<long> &times)
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
bool trace::write_profile(const string &o_path, const event_set &ts)
{
    ofstream fout(o_path.c_str());
    if (!fout.is_open()) {
        fprintf(stderr, "failed to open %s for writing\n", o_path.c_str());
        return false;
    }

    // write out each unique time from the set
    for (event_set::const_iterator i = ts.begin(); i != ts.end(); ++i)
        fout << *i << endl;

    return true;
}

// -----------------------------------------------------------------------------
bool trace::write_profile(const string &o_path, int start, int end)
{
    ofstream fout(o_path.c_str());
    if (!fout.is_open()) {
        fprintf(stderr, "failed to open %s for writing\n", o_path.c_str());
        return false;
    }

    // write out each unique time from the set
    for (int i = start; i < end; ++i)
        fout << i << endl;

    return true;
}

