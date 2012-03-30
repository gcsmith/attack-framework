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

#include "trace_format_out.h"
#include "trace_format_packed.h"
#include "trace_format_simv.h"
#include "trace_format_v1.h"
#include "trace_format_v2.h"
#include "trace_format_v3.h"

#ifdef HAVE_SQLITE3_H
#include "trace_format_sqlite.h"
#endif

// -----------------------------------------------------------------------------
trace_reader *trace_reader::create(const std::string &format)
{
    if (format == "out")
        return new trace_reader_out();
    else if (format == "packed")
        return new trace_reader_packed();
    else if (format == "simv")
        return new trace_reader_simv();
#ifdef HAVE_SQLITE3_H
    else if (format == "sqlite")
        return new trace_reader_sqlite();
#endif
    else if (format == "v1")
        return new trace_reader_v1();
    else if (format == "v2")
        return new trace_reader_v2();
    else if (format == "v3")
        return new trace_reader_v3();
    else {
        fprintf(stderr, "unknown trace_reader format: %s\n", format.c_str());
        return NULL;
    }
}

// -----------------------------------------------------------------------------
trace_writer *trace_writer::create(const std::string &format)
{
    if (format == "packed")
        return new trace_writer_packed();
#ifdef HAVE_SQLITE3_H
    else if (format == "sqlite")
        return new trace_writer_sqlite();
#endif
    else if (format == "v1")
        return new trace_writer_v1();
    else if (format == "v2")
        return new trace_writer_v2();
    else if (format == "v3")
        return new trace_writer_v3();
    else {
        fprintf(stderr, "unknown trace_writer format: %s\n", format.c_str());
        return NULL;
    }
}

