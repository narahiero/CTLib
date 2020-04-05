//////////////////////////////////////////////////
//  Copyright (c) 2020 Nara Hiero
//
// This file is licensed under GPLv3+
// Refer to the `License.txt` file included.
//////////////////////////////////////////////////

#include <CTLib/Yaz.hpp>

#include <memory>

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

size_t findBestMatch(Buffer& data, uint16_t* table, uint16_t tableOff, size_t& best)
{
    uint8_t* curr = *data + data.position();
    size_t size = data.remaining() > 0x111 ? 0x111 : data.remaining();

    best = 0;
    uint8_t* bestLoc = curr;
    for (size_t i = 1, j = 0; i <= table[*curr << 13]; ++i, j = 0)
    {
        int16_t back = table[(*curr << 13) + i] - tableOff;
        if (back <= 0)
        {
            break; // all remaining offsets are after the current location
        }
        else if (back > 0x1000)
        {
            continue;
        }
        uint8_t* search = curr - back;
        while ((curr[j] == search[j]) && (++j < size))
        {
            // loop until difference is found or max size is reached
        }
        bestLoc = best < j ? search : bestLoc;
        best = best < j ? j : best;
    }

    return curr - bestLoc;
}

void fillInTable(Buffer& data, uint16_t* table)
{
    uint8_t* curr = *data + data.position();
    size_t size = data.remaining() > 0x1000 ? 0x1000 : data.remaining();
    for (uint16_t i = 0; i < size; ++i)
    {
        uint16_t count = ++table[curr[i] << 13];
        table[(curr[i] << 13) + count] = 0xFFF - i;
    }
}

void updateTable(Buffer& data, uint16_t* table, uint16_t off)
{
    for (size_t i = 0; i < 0x100; ++i)
    {
        uint16_t* curr = table + (i << 13);
        uint16_t move = 0;
        for (uint16_t j = 1; j <= *curr; ++j)
        {
            curr[j] += 0xFFF + off;
            if (curr[j] > 0x1FFF)
            {
                move = j;
            }
            else if (move > 0)
            {
                curr[j - move] = curr[j];
            }
        }
        *curr -= move; // subtract from count
    }
    fillInTable(data, table);
}

void compressData(Buffer& data, Buffer& out)
{
    auto offsetsTableHandler = std::make_unique<uint16_t[]>(1 << 21);
    uint16_t* offsetsTable = offsetsTableHandler.get();
    int16_t tableOff = 0xFFF;

    for (size_t i = 0; i < 0x100; ++i)
    {
        offsetsTable[i << 13] = 0; // set offset counts to 0
    }
    fillInTable(data, offsetsTable);
    
    Buffer dataGroup(0x18);
    uint8_t groupHead = 0;
    uint8_t groupIdx = 8;
    while (data.hasRemaining())
    {
        size_t size = 0;
        size_t pos = findBestMatch(data, offsetsTable, tableOff, size);

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
            tableOff -= static_cast<int16_t>(size);
        }
        else // direct single byte copy
        {
            dataGroup.put(data.get());
            groupHead |= 0x1; // set bit of current chunk

            --tableOff;
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

        if (tableOff < 0)
        {
            updateTable(data, offsetsTable, -tableOff);
            tableOff = 0xFFF;
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
