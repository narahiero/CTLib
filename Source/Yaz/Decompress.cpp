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

struct YazHeader
{
    // format of compressed data
    YazFormat format;

    // length of uncompressed data
    uint32_t dataSize;
};

void readHeader(Buffer& data, YazHeader* header)
{
    try
    {
        uint8_t magic[4];
        data.getArray(magic, 4);

        if (Bytes::matchesString("Yaz0", magic, 4))
        {
            header->format = YazFormat::Yaz0;
        }
        else if (Bytes::matchesString("Yaz1", magic, 4))
        {
            header->format = YazFormat::Yaz1;
        }
        else
        {
            throw YazError(Strings::format(
                "Data is not Yaz0/Yaz1 compressed: Invalid magic! (Got \"%s\")",
                Strings::stringify(magic, 4).c_str()
            ));
        }

        header->dataSize = data.getInt();

        data.getInt(); // two unused integers
        data.getInt(); // ^~~
    }
    catch(const CTLib::BufferError&)
    {
        throw YazError("Invalid Yaz0/Yaz1 header!");
    }
}

void decompressData(Buffer& data, Buffer& out)
{
    try
    {
        uint8_t head;
        uint8_t chunkIdx = 1; // 1 so that `--chunkIdx` evaluates to 0
        while (out.hasRemaining())
        {
            if (--chunkIdx == 0) // move to next data group
            {
                head = data.get();
                chunkIdx = 8;
            }

            if (head & 0x80) // set bit so direct byte copy
            {
                out.put(data.get());
            }
            else // clear bit so copy already decompressed data
            {
                uint16_t b = data.getShort(); // back reference first two bytes

                uint16_t rewind = (b & 0x0FFF) + 1;

                uint16_t count = b >> 12;
                if (count == 0) // read thrid byte for actual count
                {
                    count = data.get() + 0x12;
                }
                else
                {
                    count += 2;
                }

                while (count-- > 0) // copy some previous bytes from out
                {
                    out.put(out.get(out.position() - rewind));
                }
            }

            head <<= 1; // shift to next control bit
        }
    }
    catch (const BufferError&)
    {
        throw YazError(Strings::format(
            "Invalid or corrupted data: %s!",
            data.hasRemaining() ? "The uncompressed data was larger than expected"
                : "An incomplete data group was found at the end of compressed data"
        ));
    }
}

Buffer decompressBase(Buffer& data, YazFormat* format)
{
    YazHeader header;
    readHeader(data, &header);

    if (format != nullptr && *format != header.format)
    {
        throw YazError("Data is not compressed in the requested format.");
    }

    if (header.dataSize == 0)
    {
        throw YazError("Uncompressed data length is 0!");
    }

    Buffer out(header.dataSize);
    decompressData(data, out);

    return out;
}

Buffer Yaz::decompress(Buffer& data)
{
    return decompressBase(data, nullptr);
}

Buffer Yaz::decompress(Buffer& data, YazFormat format)
{
    return decompressBase(data, &format);
}
}
