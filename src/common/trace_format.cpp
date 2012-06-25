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
#include <cstdio>
#include "trace_format.h"
#include "utility.h"

using namespace std;

extern trace_reader *create_trace_reader_out(void);
extern trace_reader *create_trace_reader_packed(void);
extern trace_reader *create_trace_reader_simv(void);
extern trace_reader *create_trace_reader_sqlite(void);
extern trace_reader *create_trace_reader_v1(void);
extern trace_reader *create_trace_reader_v2(void);
extern trace_reader *create_trace_reader_v3(void);

extern trace_writer *create_trace_writer_csv(void);
extern trace_writer *create_trace_writer_packed(void);
extern trace_writer *create_trace_writer_sqlite(void);
extern trace_writer *create_trace_writer_v3(void);

// -----------------------------------------------------------------------------
// static
string trace_reader::guess_format(const string &path)
{
    if (!util::path_exists(path) || !util::is_directory(path)) {
        // input path is a file, or does not exist, so determine type by name
        const string ext(util::path_extension(path));

        if (ext == ".bin" || ext == ".packed")
            return "packed";
        else if (ext == ".db" || ext == ".sqlite" || ext == ".sqlite3")
            return "sqlite";
        else if (string::npos != path.find("v1"))
            return "v1";
        else if (string::npos != path.find("v2"))
            return "v2";
        else if (string::npos != path.find("v3"))
            return "v3";
        else
            return "out";
    }
    else {
        // input path is a directory, so determine type by the files within
        if (util::directory_search(path, "simulation.*\\.txt$"))
            return "simv";
        else if (util::directory_search(path, ".*\\.out$"))
            return "out";
        else if (util::directory_search(path, ".*\\.bin$"))
            return "v1";
        else if (util::directory_search(path, ".*\\.csv$"))
            return "v2";
        else if (util::directory_search(path, "text_.*\\.txt$"))
            return "v3";
    }

    return "packed"; // welp
}

// -----------------------------------------------------------------------------
// static
trace_reader *trace_reader::create(const string &format)
{
    if      (format == "out")    return create_trace_reader_out();
    else if (format == "packed") return create_trace_reader_packed();
    else if (format == "simv")   return create_trace_reader_simv();
    else if (format == "v1")     return create_trace_reader_v1();
    else if (format == "v2")     return create_trace_reader_v2();
    else if (format == "v3")     return create_trace_reader_v3();
#ifdef HAVE_SQLITE3_H
    else if (format == "sqlite") return create_trace_reader_sqlite();
#endif
    else {
        fprintf(stderr, "unknown trace_reader format: %s\n", format.c_str());
        return NULL;
    }
}

// -----------------------------------------------------------------------------
// static
bool trace_reader::copy_trace(const trace &pt_in, trace &pt_out,
                              const trace::event_set &events)
{
    trace::real last_power = 0.0f;
    trace::event_set::const_iterator curr_event = events.begin();

    foreach (const trace::sample sample, pt_in.samples()) {
        // primetime may break the power sample into multiple events
        if (pt_out.size() && pt_out.back().time == sample.time) {
#if 1
            pt_out.back().power += sample.power;
#endif
            continue;
        }

        // sample and hold by copying the previous power into each empty sample
        while ((curr_event != events.end()) && (*curr_event < sample.time))
            pt_out.push_back(trace::sample(*curr_event++, sample.power));

        assert(sample.time == *curr_event);
        pt_out.push_back(trace::sample(*curr_event++, last_power));

#if 1
        // if this is disabled, last_power will always be 0 (ie. empty samples)
        last_power = sample.power;
#endif
    }

    // pad out (with sample and hold) any trailing samples, if necessary
    while (curr_event != events.end())
        pt_out.push_back(trace::sample(*curr_event++, last_power));

    return true;
}

// -----------------------------------------------------------------------------
// static
trace_writer *trace_writer::create(const string &format)
{
    if      (format == "csv")    return create_trace_writer_csv();
    else if (format == "packed") return create_trace_writer_packed();
    else if (format == "v3")     return create_trace_writer_v3();
#ifdef HAVE_SQLITE3_H
    else if (format == "sqlite") return create_trace_writer_sqlite();
#endif
    else {
        fprintf(stderr, "unknown trace_writer format: %s\n", format.c_str());
        return NULL;
    }
}

