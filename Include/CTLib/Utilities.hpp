//////////////////////////////////////////////////
//  Copyright (c) 2020 Nara Hiero
//
// This file is licensed under GPLv3+
// Refer to the `License.txt` file included.
//////////////////////////////////////////////////

#pragma once

/*! @file Utilities.hpp
 *
 *  @brief The header containing miscellaneous utilities for CT Lib.
 */


#include <cstdint>
#include <stdexcept>
#include <string>


namespace CTLib
{

/*! @brief Utility class containing various methods to work with strings. */
class Strings
{

public:

/*! @brief Returns a string representation of the specified binary data.
 *
 *  All bytes with their value `< 0x20 || >= 0x7F` will be replaced by `\xNN`,
 *  with _N_ being the hexadecimal representation of the value. Otherwise the
 *  byte is simply casted to `char`.
 *  
 *  @param[in] bytes The bytes to be 'stringified'
 *  @param[in] size The amount of bytes in the array
 * 
 *  @return The 'stringified' array of bytes
 */
static std::string stringify(uint8_t* bytes, size_t size);

/*! @brief Formats a string `sprintf`-style.
 *  
 *  @param[in] fmt The format string
 *  @param[in] args The format arguments
 * 
 *  @throw std::runtime_error If an error occurs while formatting.
 * 
 *  @return The formatted string
 */
template<typename ... Args>
static std::string format(const std::string& fmt, Args ... args)
{
    size_t size = snprintf(nullptr, 0, fmt.c_str(), args ...) + 1;
    if (size <= 0)
    {
        throw std::runtime_error("An error occured while formatting...");
    }
    std::string str(size, '\0');
    snprintf(str.data(), size, fmt.c_str(), args ...);
    str.erase(str.end() - 1);
    return str;
}
};

/*! @brief Utility class containing various methods to analyse and modify
 *  sequences of bytes.
 */
class Bytes
{

public:

    /*! @brief Checks if the first `count` bytes of `a` and `b` matches.
     * 
     *  @param[in] a Left hand side of the comparision
     *  @param[in] b Right hand side of the comparision
     *  @param[in] count The amount of bytes to be compared
     * 
     *  @return The result of the comparision
     */
    static bool matches(uint8_t* a, uint8_t* b, size_t count);

    /*! @brief Checks if the first `count` bytes of `str` and `bytes` matches.
     * 
     *  @param[in] str The string to be compared
     *  @param[in] bytes The bytes to be compared
     *  @param[in] count The amount of bytes to be compared
     * 
     *  @return The result of the comparision
     */
    static bool matchesString(const char* str, uint8_t* bytes, size_t count);
};
}
