//////////////////////////////////////////////////
//  Copyright (c) 2020 Nara Hiero
//
// This file is licensed under GPLv3+
// Refer to the `License.txt` file included.
//////////////////////////////////////////////////

#include <CTLib/Yaz.hpp>

#include <CTLib/Utilities.hpp>

namespace CTLib
{

size_t calculateMaxSize(size_t baseSize)
{
    return (baseSize + (baseSize >> 3) + 0x18) & ~0x7;
}

void writeHeader(Buffer& out, YazFormat format, size_t len)
{
    out.putArray((uint8_t*)(format == YazFormat::Yaz0 ? "Yaz0" : "Yaz1"), 4);
    out.putInt(static_cast<uint32_t>(len));
    out.putInt(0);
    out.putInt(0);
}

void noCompression(Buffer& data, Buffer& out)
{
    uint8_t counter = 1; // 1 so that `--counter` evaluates to 0
    while (data.hasRemaining())
    {
        if (--counter <= 0)
        {
            out.put(0xFF);
            counter = 8;
        }
        out.put(data.get());
    }
}

void dataCompression(Buffer& data, Buffer& out)
{
    Buffer dataGroup(0x18);
    uint8_t groupHead = 0;
    uint8_t groupIdx = 8;
    while (data.hasRemaining())
    {
        uint8_t* curr = *data + data.position();
        size_t size = data.limit() > 0x111 ? 0x111 : data.limit();

        uint8_t* search = data.position() > 0x1000 ? (curr - 0x1000) : *data;
        size_t searchSize = curr - search;

        uint8_t* find = Bytes::findLongestMatch(search, searchSize, curr, size);

        if (size > 2) // only use back reference if worth it
        {
            uint16_t chunk = (curr - find - 1) & 0xFFF; // relative position
            if (size < 0x12)
            {
                chunk |= ((size - 0x2) & 0xF) << 12; // add size info
                dataGroup.putShort(chunk);
            }
            else // use three bytes chunk
            {
                dataGroup.putShort(chunk);
                dataGroup.put((size - 0x12) & 0xFF); // put additional size info
            }

            data.position(data.position() + size);
        }
        else // direct single byte copy
        {
            dataGroup.put(data.get());
            groupHead |= 0x1; // set bit of current chunk
        }
        
        if (--groupIdx <= 0) // move to next group
        {
            out.put(groupHead).put(dataGroup.flip());
            dataGroup.clear();
            groupHead = 0;
            groupIdx = 8;
        }
        else // move to next chunk
        {
            groupHead <<= 1;
        }
    }

    if (groupIdx < 8) // flush any remaining data
    {
        groupHead <<= groupIdx - 1;
        out.put(groupHead).put(dataGroup.flip());
    }
}

void compressData(Buffer& data, Buffer& out, YazLevel level)
{
    switch (level)
    {
    case YazLevel::NONE:
        noCompression(data, out);
        break;

    default:
        dataCompression(data, out);
        break;
    }

    size_t len = out.position();

    if ((len & 0x3) > 0) // add padding
    {
        size_t padding = 4 - (len & 0x3);
        while (padding-- > 0)
        {
            out.put(0x00);
        }
    }
}

Buffer compressBase(Buffer& data, YazFormat format, YazLevel level)
{
    Buffer out(calculateMaxSize(data.remaining()));

    writeHeader(out, format, data.remaining());
    compressData(data, out, level);

    return out.flip();
}

Buffer Yaz::compress(Buffer& data, YazFormat format, YazLevel level)
{
    return compressBase(data, format, level);
}
}
