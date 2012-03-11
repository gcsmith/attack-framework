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

#include <iostream>
#include <boost/program_options.hpp>
#include "cmdline.h"

using namespace std;
namespace po = boost::program_options;

class cmdline::cmdline_impl {
public:
    cmdline_impl(const cmdline_option *args, const string &usage)
    : m_usage(usage), m_desc("Options"), m_args(args) { }

    bool parse(int argc, char *argv[]);

    size_t count(const string &name) {
        return m_vm.count(name);
    }

    void print_usage() {
        cerr << "usage: " << m_usage << endl;
        cerr << m_desc;
    }

    bool get_flag(const string &name) {
        return m_vm.count(name) != 0;
    }

    string get_str(const string &name, const string &defval) {
        return m_vm.count(name) ? m_vm[name].as<string>() : defval;
    }

    void get_strv(const string &name, vector<string> &v) {
        if (m_vm.count(name))
            v = m_vm[name].as<vector<string> >();
    }

    long get_long(const string &name, long defval) {
        return m_vm.count(name) ? m_vm[name].as<long>() : defval;
    }

protected:
    string m_usage;
    po::options_description m_desc;
    po::variables_map m_vm;
    const cmdline_option *m_args;
};

// -----------------------------------------------------------------------------
bool cmdline::cmdline_impl::parse(int argc, char *argv[])
{
    const cmdline_option *opt = m_args;
    while (CL_TERM != opt->type) {
        switch (opt->type) {
        default:
        case CL_FLAG:
            m_desc.add_options()(opt->name, opt->desc);
            break;
        case CL_STR:
            m_desc.add_options()(opt->name, po::value<string>(), opt->desc);
            break;
        case CL_STRV:
            m_desc.add_options()(opt->name, po::value<vector<string> >(), opt->desc);
            break;
        case CL_LONG:
            m_desc.add_options()(opt->name, po::value<long>(), opt->desc);
            break;
        }
        ++opt;
    }

    try {
        po::store(po::parse_command_line(argc, argv, m_desc), m_vm);
        po::notify(m_vm);
    }
    catch (exception &e) {
        cerr << "error: " << e.what() << endl;
        return false;
    }

    return true;
}

// -----------------------------------------------------------------------------
cmdline::cmdline(const cmdline_option *args, const string &usage)
{
    m_pimpl = new cmdline_impl(args, usage);
}

// -----------------------------------------------------------------------------
cmdline::~cmdline()
{
    delete m_pimpl;
}

// -----------------------------------------------------------------------------
bool cmdline::parse(int argc, char *argv[])
{
    return m_pimpl->parse(argc, argv);
}

// -----------------------------------------------------------------------------
size_t cmdline::count(const string &name)
{
    return m_pimpl->count(name);
}

// -----------------------------------------------------------------------------
void cmdline::print_usage()
{
    return m_pimpl->print_usage();
}

// -----------------------------------------------------------------------------
bool cmdline::get_flag(const string &name)
{
    return m_pimpl->get_flag(name);
}

// -----------------------------------------------------------------------------
string cmdline::get_str(const string &name, const string &defval)
{
    return m_pimpl->get_str(name, defval);
}

// -----------------------------------------------------------------------------
vector<string> cmdline::get_strv(const string &name)
{
    vector<string> output;
    m_pimpl->get_strv(name, output);
    return output;
}

// -----------------------------------------------------------------------------
long cmdline::get_long(const string &name, long defval)
{
    return m_pimpl->get_long(name, defval);
}

