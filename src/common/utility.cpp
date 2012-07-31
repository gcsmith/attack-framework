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

#include <algorithm>
#include <cstdio>
#include <ctime>
#include <boost/regex.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>
#include "utility.h"

using namespace std;
namespace fs = boost::filesystem;

namespace util {

// -----------------------------------------------------------------------------
string timestamp(void)
{
    time_t t = time(NULL);
    char buffer[128];
    strftime(buffer, 128, "%Y_%b_%d_%H_%M_%S", localtime(&t));
    return buffer;
}

// -----------------------------------------------------------------------------
bool valid_input_directory(const string &path)
{
    // verify that the specified input path exists and is a directory
    if (!fs::exists(path) || !fs::is_directory(path)) {
        fprintf(stderr, "invalid input directory %s\n", path.c_str());
        return false;
    }
    return true;
}

// -----------------------------------------------------------------------------
bool valid_output_directory(const string &path)
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
size_t glob(const string &path, const string &pattern, vector<string> &out)
{
    if (!fs::is_directory(path)) return 0;

    // scan through 'path' and glob the files matching 'pattern'
    const boost::regex regex_pattern(pattern);
    fs::directory_iterator iter_begin(path), iter_end;

    BOOST_FOREACH (const fs::path &curr_path, make_pair(iter_begin, iter_end)) {
        if (regex_search(curr_path.string(), regex_pattern))
            out.push_back(curr_path.string());
    }

    // return a sorted list of matching paths
    sort(out.begin(), out.end());
    return out.size();
}

// -----------------------------------------------------------------------------
size_t glob_recursive(const string &path, const string &pattern,
                      vector<string> &out)
{
    if (!fs::is_directory(path)) return 0;

    // recursively scan through 'path' and glob the files matching 'pattern'
    const boost::regex regex_pattern(pattern);
    fs::recursive_directory_iterator iter_begin(path), iter_end;

    BOOST_FOREACH (const fs::path &curr_path, make_pair(iter_begin, iter_end)) {
        if (regex_search(curr_path.string(), regex_pattern))
            out.push_back(curr_path.string());
    }

    // return a sorted list of matching paths
    sort(out.begin(), out.end());
    return out.size();
}

// -----------------------------------------------------------------------------
bool directory_search(const string &path, const string &pattern)
{
    if (!fs::is_directory(path)) return false;

    const boost::regex regex_pattern(pattern);
    fs::recursive_directory_iterator iter_begin(path), iter_end;

    BOOST_FOREACH (const fs::path &curr_path, make_pair(iter_begin, iter_end))
        if (regex_search(curr_path.string(), regex_pattern)) return true;

    return false;
}

// -----------------------------------------------------------------------------
bool atob(const string &str, uint8_t *bytes, size_t count)
{
    if ((count * 2) != str.length()) {
        fprintf(stderr, "incorrect hex format %s (expect %d digits, got %d)\n",
                str.c_str(), (int)count * 2, (int)str.length());
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
string path_stem(const string &path)
{
    return fs::path(path).stem().string();
}

// -----------------------------------------------------------------------------
string path_extension(const string &path)
{
    return fs::path(path).extension().string();
}

// -----------------------------------------------------------------------------
bool path_exists(const string &path)
{
    return fs::exists(path);
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
vector<string> split(const string &str)
{
    vector<string> tok;
    const string tstr = boost::trim_copy(str);
    boost::split(tok, tstr, boost::is_any_of(" \t"), boost::token_compress_on);
    return tok;
}

// -----------------------------------------------------------------------------
vector<string> split(const string &str, const string &delim)
{
    vector<string> tok;
    boost::split(tok, str, boost::is_any_of(delim), boost::token_compress_on);
    return tok;
}

// -----------------------------------------------------------------------------
string trim_copy(const string &str)
{
    return boost::trim_copy(str);
}

// -----------------------------------------------------------------------------
string &trim(string &str)
{
    boost::trim(str);
    return str;
}

// -----------------------------------------------------------------------------
bool is_directory(const string &path)
{
    return fs::is_directory(path);
}

}; // namespace util

