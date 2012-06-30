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

#ifndef TRACE_FORMAT__H
#define TRACE_FORMAT__H

#include <string>
#include "trace.h"

//! Abstract interface for trace reader objects.
struct trace_reader {
    //! Trace reader options.
    struct options {
        size_t min_time;
        size_t max_time;
        size_t num_traces;
        bool ciphertext;
        std::string key;
    };

    //! Attempt to determine the trace format from the input path.
    static std::string guess_format(const std::string &path);

    //! Create a trace_reader instance for the given trace format.
    static trace_reader *create(const std::string &format);

    //! Copy the input trace to the output trace, expanding if necessary.
    static bool copy_trace(const trace &pt_in, trace &pt_out,
                           const trace::event_set &events);

    //! Provide a summary of the specified trace directory.
    virtual bool summary(const std::string &path) const = 0;

    //! Open the reader for a specified key and plaintext/ciphertext selection.
    virtual bool open(const std::string &path, const options &opt) = 0;

    //! Close the reader and perform cleanup.
    virtual void close(void) = 0;

    //! Read the next trace into pt, limited to the specified time range.
    virtual bool read(trace &pt) = 0;

    //! Returns the number traces available for reading.
    virtual size_t trace_count(void) const = 0;

    //! Returns a set of time events across the traces read so far.
    virtual const trace::event_set &events(void) const = 0;

    //! Explicit virtual destructor, as trace_reader will be sub-classed.
    virtual ~trace_reader(void) { }
};

//! Abstract interface for trace writer objects.
struct trace_writer {
    //! Create a trace_writer instance for the given trace format.
    static trace_writer *create(const std::string &format);

    //! Open the writer for a specified key.
    virtual bool open(const std::string &path, const std::string &key,
                      const trace::event_set &events) = 0;

    //! Close the writer and perform cleanup.
    virtual void close(void) = 0;

    //! Write the next trace object.
    virtual bool write(const trace &pt) = 0;

    //! Explicit virtual destructor, as trace_writer will be sub-classed.
    virtual ~trace_writer(void) { }
};

#define register_trace_reader(n, o)                                     \
    trace_reader *create_trace_reader_##n(void) {                       \
        return new o;                                                   \
    }

#define register_trace_writer(n, o)                                     \
    trace_writer *create_trace_writer_##n(void) {                       \
        return new o;                                                   \
    }

#endif // TRACE_FORMAT__H

