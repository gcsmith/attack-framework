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

#ifndef CMDLINE__H
#define CMDLINE__H

#include <string>
#include <vector>

//! TODO: description
enum cmdline_argtype {
    CL_TERM,
    CL_FLAG,
    CL_STR,
    CL_STRV,
    CL_LONG,
};

//! TODO: description
struct cmdline_option {
    cmdline_argtype type;
    const char *name;
    const char *desc;
};

//! TODO: description
class cmdline {
public:
    //! TODO: description
    cmdline(const cmdline_option *args, const std::string &usage);

    //! TODO: description
    ~cmdline();

    //! TODO: description
    bool parse(int argc, char *argv[]);

    //! TODO: description
    size_t count(const std::string &name);

    //! TODO: description
    void print_usage(void);

    //! TODO: description
    std::string get_str(const std::string &name, const std::string &def = "");

    //! TODO: description
    std::vector<std::string> get_strv(const std::string &name);

    //! TODO: description
    long get_long(const std::string &name, long def = 0);

    //! TODO: description
    bool get_flag(const std::string &name);

protected:
    class cmdline_impl;
    cmdline_impl *m_pimpl;
};

#endif // CMDLINE__H

