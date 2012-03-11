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
#include <cassert>
#include "agilent_reader.h"
#include "trace_format_v1.h"

using namespace std;

// -----------------------------------------------------------------------------
bool trace_reader_v1::open(const string &path, const string &key, bool ct)
{
    close();

    // get the full list of binary files located in the input directory
    util::pathlist paths;
    util::scan_directory(path, ".bin", paths);

    // cull the list down to traces matching the specified key
    const string search("k=" + key);
    for (util::pathlist::iterator i = paths.begin(); i != paths.end(); ++i) {
        if (string::npos != i->find(search))
            m_paths.push_back(*i);
    }

    printf("%zu %zu\n", paths.size(), m_paths.size());

    // select search string based on whether we want plaintext or ciphertext
    m_search = ct ? "c=" : "m=";
    m_current = 0;
    return true;
}

// -----------------------------------------------------------------------------
void trace_reader_v1::close()
{
    m_events.clear();
    m_paths.clear();
}

// -----------------------------------------------------------------------------
bool trace_reader_v1::read(trace &pt, const trace::time_range &range)
{
    if (m_current >= m_paths.size())
        return false;

    const string path(m_paths[m_current++]);
    const string msg_str(path.substr(path.find(m_search) + 2, 16));

    pt.clear();
    pt.set_text(util::atob(msg_str));

    FILE *fp = fopen(path.c_str(), "rb");
    if (NULL == fp) {
        fprintf(stderr, "failed to open %s for reading\n", path.c_str());
        return false;
    }

    tPBFileHeader fh;
    if (!ReadAgilentHeader(fp, &fh) || (fh.NumberOfWaveforms != 1)) {
        fprintf(stderr, "failed to read file header: %s\n", path.c_str());
        return false;
    }

    tPBWaveformHeader wh;
    if (!ReadWaveformHeader(fp, &wh)) {
        fprintf(stderr, "failed to read waveform header: %s\n", path.c_str());
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

    for (int i = 0; i < wh.Points; ++i) {
        // only record samples within the specified time range
        if (range.first && i < range.first)
            continue;
        else if (range.second && i >= range.second)
            break;

        pt.push_back(trace::sample(i, data[i]));
        m_events.insert(i);
    }

    DestroyAnalogWaveform(data);
    fclose(fp);
    return true;
}

// -----------------------------------------------------------------------------
bool trace_writer_v1::open(const string &path, const string &key)
{
    assert(!"not implemented");
    return false;
}

// -----------------------------------------------------------------------------
void trace_writer_v1::close()
{
    assert(!"not implemented");
}

// -----------------------------------------------------------------------------
bool trace_writer_v1::write(const trace &pt)
{
    assert(!"not implemented");
    return false;
}

