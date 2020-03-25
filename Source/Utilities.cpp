//////////////////////////////////////////////////
//  Copyright (c) 2020 Nara Hiero
//
// This file is licensed under GPLv3+
// Refer to the `License.txt` file included.
//////////////////////////////////////////////////

#include <CTLib/Utilities.hpp>

#include <fstream>

namespace CTLib
{

std::string Strings::stringify(uint8_t* bytes, size_t size)
{
    constexpr const char* TABLE = "0123456789ABCDEF";
    size_t strsize = 0;               // first run to calculate the size to
    for (size_t i = 0; i < size; ++i) // avoid multiple memory allocations
    {
        strsize += (bytes[i] < 0x20 || bytes[i] >= 0x7F) ? 4 : 1;
    }
    std::string str(strsize, '\0');
    for (size_t i = 0, p = 0; i < size; ++i)
    {
        uint8_t b = bytes[i];
        if (b < 0x20 || b >= 0x7F)
        {
            str[p++] = '\\';
            str[p++] = 'x';
            str[p++] = TABLE[b >> 4];
            str[p++] = TABLE[b & 0xF];
        }
        else
        {
            str[p++] = static_cast<char>(b);
        }
    }
    return str;
}

bool Bytes::matches(uint8_t* a, uint8_t* b, size_t count)
{
    for (size_t i = 0; i < count; ++i)
    {
        if (a[i] != b[i])
        {
            return false;
        }
    }
    return true;
}

bool Bytes::matchesString(const char* str, uint8_t* bytes, size_t count)
{
    return matches((uint8_t*)str, bytes, count);
}

uint8_t* Bytes::findLongestMatch(uint8_t* bytes, size_t bytesSize, uint8_t* find, size_t& findSize)
{
    size_t best = 0;
    uint8_t* bestLoc = bytes;
    for (size_t i = 0, j = 0; i < bytesSize; ++i, j = 0)
    {
        size_t maxSize = findSize > (bytesSize - i) ? (bytesSize - i) : findSize;
        while ((find[j] == bytes[i + j]) && (++j < maxSize))
        {
            // loop until difference is found or max size is reached
        }
        bestLoc = best < j ? (bytes + i) : bestLoc;
        best = best < j ? j : best;
    }
    findSize = best;
    return bestLoc;
}

Buffer IO::readFile(const char* filename)
{
    Buffer buffer;

    std::ifstream file(filename, std::ios::in | std::ios::binary | std::ios::ate);
    if (file.is_open())
    {
        size_t size = file.tellg();
        buffer = Buffer(size);

        file.seekg(std::ios::beg);
        file.read((char*)*buffer, size);

        file.close();
    }

    return buffer;
}

Buffer IO::readFile(const std::string& filename)
{
    return readFile(filename.c_str());
}

bool IO::writeFile(const char* filename, Buffer& data)
{
    std::ofstream file(filename, std::ios::out | std::ios::binary);
    if (file.is_open())
    {
        file.write((char*)*data + data.position(), data.remaining());
        file.close();

        return true;
    }
    
    return false;
}

bool IO::writeFile(const std::string& filename, Buffer& data)
{
    return writeFile(filename.c_str(), data);
}
}
