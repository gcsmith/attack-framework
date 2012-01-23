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

#define BOOST_FILESYSTEM_VERSION 2

#include <algorithm>
#include <cstdio>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include "utility.h"

using namespace std;
namespace fs = boost::filesystem;

namespace util {

// -----------------------------------------------------------------------------
bool check_inout_directories(const string &i_dir, const string &o_dir)
{
    // verify that the specified input path exists and is a directory
    if (!fs::exists(i_dir) || !fs::is_directory(i_dir)) {
        fprintf(stderr, "invalid input directory %s\n", i_dir.c_str());
        return false;
    }

    // create output directory if it doesn't exist
    if (!fs::exists(o_dir) && !fs::create_directory(o_dir)) {
        fprintf(stderr, "error creating output directory %s\n", o_dir.c_str());
        return false;
    }

    // verify that the specified output path is a directory
    if (!fs::is_directory(o_dir)) {
        fprintf(stderr, "output path must specifiy a directory\n");
        return false;
    }

    return true;
}

// -----------------------------------------------------------------------------
bool scan_directory(const string &in_path, const string &ext, pathlist &paths)
{
    fs::directory_iterator end;
    for (fs::directory_iterator i(in_path); i != end; ++i) {
        string name = i->string();
        if (boost::ends_with(name, ext))
            paths.push_back(name);
    }
    sort(paths.begin(), paths.end());
    return true;
}

// -----------------------------------------------------------------------------
bool parse_plaintext(const string &filename, uint8_t *bytes, size_t count)
{
    vector<string> tokens;
    string str = fs::path(filename).stem();
    boost::split(tokens, str, boost::is_any_of("_."));

    if (count && (tokens.size() != count)) {
        fprintf(stderr, "bad trace name %s: parsed %lu bytes, expected %lu\n",
                filename.c_str(), tokens.size(), count);
        return false;
    }

    for (size_t i = 0; i < tokens.size(); ++i)
        bytes[i] = strtol(tokens[i].c_str(), NULL, 16);

    return true;
}

// -----------------------------------------------------------------------------
string hexstring_to_filename(const string &str, size_t num_bytes)
{
    string o_name;
    for (size_t j = 0; j < num_bytes; ++j) {
        o_name += str.substr(j * 2, 2);
        if (j != (num_bytes - 1)) o_name += "_";
    }
    return o_name;
}

// -----------------------------------------------------------------------------
bool atob(const string &str, uint8_t *bytes, size_t count)
{
    if ((count * 2) != str.length()) {
        fprintf(stderr, "incorrect hex string format %s\n", str.c_str());
        return false;
    }

    for (size_t i = 0; i < count; ++i)
        bytes[i] = strtol(str.substr(i * 2, 2).c_str(), NULL, 16);
    return true;
}

// -----------------------------------------------------------------------------
string btoa(const uint8_t *bytes, size_t count)
{
    static const char *hex_tab = "0123456789ABCDEF";
    string str;
    for (size_t i = 0; i < count; ++i) {
        str += hex_tab[bytes[i] >> 4];
        str += hex_tab[bytes[i] & 0xF];
    }
    return str;
}

// -----------------------------------------------------------------------------
string base_name(const string &filename)
{
    return fs::path(filename).stem();
}

// -----------------------------------------------------------------------------
string concat_name(const string &dir, const string &name)
{
    return (fs::path(dir) / name).string();
}

// -----------------------------------------------------------------------------
string concat_name(const string &dir, const string &name, const string &ext)
{
    return ((fs::path(dir) / name).replace_extension(ext)).string();
}

// -----------------------------------------------------------------------------
void split(vector<string> &tokens, const string &str, const string &delim)
{
    tokens.clear();
    boost::split(tokens, str, boost::is_any_of(delim));
}

// -----------------------------------------------------------------------------
bool is_dir(const string &path)
{
    return fs::is_directory(path);
}

}; // namespace util

