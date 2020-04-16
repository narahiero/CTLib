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
 * 
 *  The utilities header contains mostly methods used to help with development.
 * 
 *  **Note**: You may have noticed that some methods are unused. They are not
 *  to be removed at the moment, as they are used when making a rough first
 *  draft.
 */


#include <cstdint>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>

#include <CTLib/Memory.hpp>


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

/*! @brief Returns the number of occurrences of the specified character in the
 *  specified string.
 *  
 *  @param[in] str The string to be searched
 *  @param[in] c The character to be counted
 * 
 *  @return The number of occurrences
 */
static size_t count(const std::string& str, const char c);

/*! @brief Splits the specified string by the specified character.
 *  
 *  @param[in] str The string to be split
 *  @param[in] c The character to split by
 * 
 *  @return The split string
 */
static std::vector<std::string> split(const std::string& str, const char c);

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

    /*! @brief Find the longest match of the array `find` in the array `bytes`.
     *  
     *  @param[in] bytes The array to be scanned
     *  @param[in] bytesSize The size of the array `bytes`
     *  @param[in] find The array containing the bytes to be matched
     *  @param[in,out] findSize The size of the array `find`, then the size of
     *  the returned array when this function returns
     * 
     *  @return The location of the longest match in the array `bytes`
     */
    static uint8_t* findLongestMatch(
        uint8_t* bytes, size_t bytesSize, uint8_t* find, size_t& findSize
    );
};

/*! @brief Utility class containing various methods to perform I/O operations. 
 */
class IO
{

public:

    /*! @brief Reads the file with the specified filename. 
     *
     *  @param[in] filename The input file name
     *  @param[out] err The error code, 0 if none
     * 
     *  @return A buffer containing the data in the file, or of size 0 if an
     *  error occurred
     */
    static Buffer readFile(const char* filename, uint32_t* err = nullptr);

    /*! @brief Reads the file with the specified filename. 
     *
     *  @param[in] filename The input file name
     *  @param[out] err The error code, 0 if none
     * 
     *  @return A buffer containing the data in the file, or of size 0 if an
     *  error occurred
     */
    static Buffer readFile(const std::string& filename, uint32_t* err = nullptr);

    /*! @brief Writes the data in the specified buffer to the file with the
     *  specified filename.
     * 
     *  @param[in] filename The output file name
     *  @param[in] data The input data
     * 
     *  @return Whether the write was successful
     */
    static bool writeFile(const char* filename, Buffer& data);

    /*! @brief Writes the data in the specified buffer to the file with the
     *  specified filename.
     * 
     *  @param[in] filename The output file name
     *  @param[in] data The input data
     * 
     *  @return Whether the write was successful
     */
    static bool writeFile(const std::string& filename, Buffer& data);
};

/*! @brief Utility class used to iterate over the values of a map. */
template<class K, class V>
class MapValueIterator final
{

public:

    /*! Alias for std::map::iterator. */
    using MapIterator = typename std::map<K, V>::iterator;

    /*! @brief Constructs a MapValueIterator wrapped around the specified
     *  map iterator.
     * 
     *  @param[in] it The map iterator to be wrapped
     */
    MapValueIterator(MapIterator& it) :
        it{it}
    {

    }

    /*! @brief Returns whether this iterator equals `other`. */
    bool operator==(const MapValueIterator<K, V>& other) const
    {
        return it == other.it;
    }

    /*! @brief Returns whether this iterator does not equal `other`. */
    bool operator!=(const MapValueIterator<K, V>& other) const
    {
        return it != other.it;
    }

    /*! @brief Returns the element currently pointed by this iterator. */
    V& operator*() const
    {
        return (*it).second;
    }

    /*! @brief Increments the position of this iterator. */
    MapValueIterator& operator++()
    {
        ++it;
        return *this;
    }

private:

    MapIterator it;
};
}
