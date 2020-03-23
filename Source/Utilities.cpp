//////////////////////////////////////////////////
//  Copyright (c) 2020 Nara Hiero
//
// This file is licensed under GPLv3+
// Refer to the `License.txt` file included.
//////////////////////////////////////////////////

#include <CTLib/Utilities.hpp>

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
}
