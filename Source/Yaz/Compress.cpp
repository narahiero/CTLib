//////////////////////////////////////////////////
//  Copyright (c) 2020 Nara Hiero
//
// This file is licensed under GPLv3+
// Refer to the `License.txt` file included.
//////////////////////////////////////////////////

#include <CTLib/Yaz.hpp>

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

// modified version of CTLib::Bytes::findLongestMatch from <Utilities.hpp>
// to allow self referencing chunks
size_t findBestMatch(Buffer& data, size_t& best)
{
    uint8_t* curr = *data + data.position();
    size_t size = data.limit() > 0x111 ? 0x111 : data.limit();

    uint8_t* search = data.position() > 0x1000 ? (curr - 0x1000) : *data;
    size_t searchSize = curr - search;

    best = 0;
    uint8_t* bestLoc = search;
    for (size_t i = 0, j = 0; i < searchSize; ++i, j = 0)
    {
        while ((curr[j] == search[i + j]) && (++j < size))
        {
            // loop until difference is found or max size is reached
        }
        bestLoc = best < j ? (search + i) : bestLoc;
        best = best < j ? j : best;
    }

    return curr - bestLoc;
}

void compressData(Buffer& data, Buffer& out)
{
    Buffer dataGroup(0x18);
    uint8_t groupHead = 0;
    uint8_t groupIdx = 8;
    while (data.hasRemaining())
    {
        size_t size = 0;
        size_t pos = findBestMatch(data, size);

        if (size > 2) // only use back reference if worth it
        {
            uint16_t chunk = (pos - 1) & 0xFFF; // relative position
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

Buffer compressBase(Buffer& data, YazFormat format)
{
    Buffer out(calculateMaxSize(data.remaining()));

    writeHeader(out, format, data.remaining());
    compressData(data, out);

    return out.flip();
}

Buffer Yaz::compress(Buffer& data, YazFormat format)
{
    return compressBase(data, format);
}
}
