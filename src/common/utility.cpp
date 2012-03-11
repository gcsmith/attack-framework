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
bool valid_input_dir(const string &path)
{
    // verify that the specified input path exists and is a directory
    if (!fs::exists(path) || !fs::is_directory(path)) {
        fprintf(stderr, "invalid input directory %s\n", path.c_str());
        return false;
    }
    return true;
}

// -----------------------------------------------------------------------------
bool valid_output_dir(const string &path)
{
    // create output directory if it doesn't exist
    if (!fs::exists(path) && !fs::create_directory(path)) {
        fprintf(stderr, "error creating output directory %s\n", path.c_str());
        return false;
    }

    // verify that the specified output path is a directory
    if (!fs::is_directory(path)) {
        fprintf(stderr, "output path must specifiy a directory\n");
        return false;
    }
    return true;
}

// -----------------------------------------------------------------------------
bool scan_directory(const string &in_path, const string &ext, pathlist &paths)
{
    if (!fs::is_directory(in_path))
        return false;

    fs::directory_iterator begin(in_path), end;
    foreach (fs::path path, make_pair(begin, end)) {
        const string name(path.string());
        if (boost::ends_with(name, ext))
            paths.push_back(name);
    }

    sort(paths.begin(), paths.end());
    return true;
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
vector<uint8_t> atob(const string &str)
{
    vector<uint8_t> result;
    for (size_t i = 0; i < (str.length() / 2); ++i)
        result.push_back(strtol(str.substr(i * 2, 2).c_str(), NULL, 16));
    return result;
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
string btoa(const vector<uint8_t> &bytes)
{
    return btoa(&bytes[0], bytes.size());
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
vector<string> split(const string &str, const string &delim)
{
    vector<string> tokens;
    boost::split(tokens, str, boost::is_any_of(delim));
    return tokens;
}

// -----------------------------------------------------------------------------
bool is_dir(const string &path)
{
    return fs::is_directory(path);
}

}; // namespace util

