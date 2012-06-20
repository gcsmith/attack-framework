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

#ifndef UTILITY__H
#define UTILITY__H

#include <stdint.h>
#include <ctime>
#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <boost/foreach.hpp>

#define EPSILON         0.00001
#define NUM_ELEMENTS(x) (sizeof(x) / sizeof(x[0]))

#define foreach         BOOST_FOREACH
#define foreach_r       BOOST_REVERSE_FOREACH

#define ENABLE_BENCHMARK
#ifdef ENABLE_BENCHMARK

#define BENCHMARK_DECLARE(n)                                                   \
    static const char *BM_##n##_name = #n;                                     \
    double BM_##n##_delta;                                                     \
    struct timespec BM_##n##_t0, BM_##n##_t1;                                  \
    clock_gettime(CLOCK_REALTIME, &BM_##n##_t0)

#define BENCHMARK_SAMPLE(n)                                                    \
    clock_gettime(CLOCK_REALTIME, &BM_##n##_t1);                               \
    BM_##n##_delta = time_delta_ns(&BM_##n##_t0, &BM_##n##_t1) / 1000000000.0; \
    printf("%s: %lf seconds elapsed\n", BM_##n##_name, BM_##n##_delta);        \
    clock_gettime(CLOCK_REALTIME, &BM_##n##_t0);

#define BENCHMARK_SAMPLE_WHEN(n, cond) if (cond) { BENCHMARK_SAMPLE(n) }

#else // !ENABLE_BENCHMARK

#define BENCHMARK_DECLARE(n)
#define BENCHMARK_SAMPLE_WHEN(n, cond)

#endif

namespace util {

//! Generate a timestamp using the system clock.
std::string timestamp(void);

//! Returns true if the specified path exists and is a directory.
bool valid_input_directory(const std::string &path);

//! Returns true if the specified path exists and is a directory, or creates it.
bool valid_output_directory(const std::string &path);

//! Glob files under directory 'path' matching the specified regex.
size_t glob(const std::string &path, const std::string &pattern,
            std::vector<std::string> &out);

//! Recursively glob files under directory 'path' matching the specified regex.
size_t glob_recursive(const std::string &path, const std::string &pattern,
                      std::vector<std::string> &out);

//! Return true if there is at least one file under 'path' matching 'pattern'.
bool directory_search(const std::string &path, const std::string &pattern);

//! Convert a hexadecimal string into an array of bytes.
bool atob(const std::string &str, uint8_t *bytes, size_t count);

//! Convert a hexadecimal string into an array of bytes.
std::vector<uint8_t> atob(const std::string &str);

//! Convert an array of bytes into a hexadecimal string.
std::string btoa(const uint8_t *bytes, size_t count);

//! Convert an array of bytes into a hexadecimal string.
std::string btoa(const std::vector<uint8_t> &bytes);

//! Returns the stem portion of a complete file path.
std::string path_stem(const std::string &path);

//! Returns the extension portion of a complete file path.
std::string path_extension(const std::string &path);

//! Returns true if the specified path exists, false otherwise.
bool path_exists(const std::string &path);

//! Concatenate the specified directory and filename.
std::string concat_name(const std::string &dir, const std::string &name);

//! Concatenate the specified directory, filename, and extension.
std::string concat_name(const std::string &dir, const std::string &name,
                        const std::string &ext);

//! Split the input string using whitespace as delimiter.
std::vector<std::string> split(const std::string &str);

//! Split the input string given a string of delimiters.
std::vector<std::string> split(const std::string &str, const std::string &del);

//! Trim leading and trailing whitespace from string.
std::string trim_copy(const std::string &str);

//! Trim leading and trailing whitespace from string.
std::string &trim(std::string &str);

//! Returns true if the specified path is a directory.
bool is_directory(const std::string &path);

//! Return x % m, where x may be negative
inline int mod(int x, int m)
{
    int r = x % m;
    return (r >= 0) ? r : r + m;
}

//! Reverse the bits in a variable length integer.
inline int revb(int x, int bits)
{
    int r = 0;
    for (int i = 0; i < bits; ++i) r |= ((x >> i) & 1) << (bits - 1 - i);
    return r;
}

//! Reverse the bits in a 32-bit unsigned integer.
inline uint32_t rev32(uint32_t x)
{
    uint32_t r = 0;
    for (int i = 0; i < 32; ++i) r |= ((x >> i) & 1) << (31 - i);
    return r;
}

//! Reverse the bits in a 64-bit unsigned integer.
inline uint64_t rev64(uint64_t x)
{
    uint64_t r = 0;
    for (int i = 0; i < 64; ++i) r |= ((x >> i) & 1) << (63 - i);
    return r;
}

//! Convert an array of 8 bytes into a reversed 64-bit unsigned integer.
inline uint64_t convert_bytes(const uint8_t *data)
{
    uint64_t x = 0;
    for (int i = 0; i < 8; ++i)
        x |= (uint64_t)revb(data[i], 8) << (i << 3);
    return x;
}

static const int popcnt[256] = {
    0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4, 1, 2, 2, 3, 2, 3, 3, 4, 2,
    3, 3, 4, 3, 4, 4, 5, 1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5, 2, 3,
    3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3,
    4, 3, 4, 4, 5, 2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 2, 3, 3, 4,
    3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5,
    6, 6, 7, 1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5, 2, 3, 3, 4, 3, 4,
    4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5,
    6, 3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7, 2, 3, 3, 4, 3, 4, 4, 5,
    3, 4, 4, 5, 4, 5, 5, 6, 3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7, 3,
    4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7, 4, 5, 5, 6, 5, 6, 6, 7, 5, 6,
    6, 7, 6, 7, 7, 8, 
};

class parameters {
    typedef std::map<std::string, std::string> item_map;
    item_map m_items;

public:
    parameters() {}
    ~parameters() {}

    template <typename T> bool put(const std::string &name, const T &value)
    {
        std::ostringstream oss;
        oss << value;
        return m_items.insert(std::make_pair(name, oss.str())).second;
    }

    template <typename T> bool get(const std::string &name, T &value) const
    {
        item_map::const_iterator iter = m_items.find(name);
        if (m_items.end() == iter) return false;

        std::istringstream iss(iter->second);
        iss >> value;
        return true;
    }

    const std::string operator [](const std::string &name) const
    {
        item_map::const_iterator iter = m_items.find(name);
        return (m_items.end() == iter) ? "" : iter->second;
    }
};

}; // namespace util

// -----------------------------------------------------------------------------
static inline double time_delta_ns(struct timespec *t0, struct timespec *t1)
{
    struct timespec td;
    if ((t1->tv_nsec - t0->tv_nsec) < 0) {
        td.tv_sec = t1->tv_sec - t0->tv_sec - 1;
        td.tv_nsec = 1000000000 + t1->tv_nsec - t0->tv_nsec;
    }
    else {
        td.tv_sec = t1->tv_sec - t0->tv_sec;
        td.tv_nsec = t1->tv_nsec - t0->tv_nsec;
    }

    return td.tv_sec * 1000000000.0 + td.tv_nsec;
}

#endif // UTILITY__H

