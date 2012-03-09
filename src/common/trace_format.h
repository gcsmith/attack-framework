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

struct trace_reader {
    // create a trace_reader instance for the given trace format
    static trace_reader *create(const std::string &format);

    // open the reader for a specified key and plaintex/ciphertext selection
    virtual bool open(const std::string &path, const std::string &key, bool ct) = 0;

    // close the reader and perform cleanup
    virtual void close(void) = 0;

    // read the next trace into pt, limited to the specified time range
    virtual bool read(trace &pt, const trace::time_range &range) = 0;

    // returns the number traces available for reading
    virtual size_t trace_count(void) = 0;

    // returns a set of time events across the traces read so far
    virtual const trace::event_set &events(void) = 0;

    // explicit virtual destructor, as trace_reader will be subclassed
    virtual ~trace_reader(void) { }
};

struct trace_writer {
    // create a trace_writer instance for the given trace format
    static trace_writer *create(const std::string &format);

    // open the writer for a specified key
    virtual bool open(const std::string &path, const std::string &key) = 0;

    // close the writer and perform cleanup
    virtual void close(void) = 0;

    // write the next trace object
    virtual bool write(const trace &pt) = 0;

    // explicit virtual destructor, as trace_writer will be subclassed
    virtual ~trace_writer(void) { }
};

#endif // TRACE_FORMAT__H

