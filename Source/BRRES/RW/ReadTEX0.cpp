//////////////////////////////////////////////////
//  Copyright (c) 2020 Nara Hiero
//
// This file is licensed under GPLv3+
// Refer to the `License.txt` file included.
//////////////////////////////////////////////////

#include "BRRES/RW/BRRESRWCommon.hpp"

#include <CTLib/Utilities.hpp>

namespace CTLib
{

struct TEX0Header
{

    // offset to data (section 1) in bytes
    uint32_t dataOff;
};

struct TEX0ImageInfo
{

    // dimensions of the image
    uint16_t width, height;

    // format of the image
    ImageFormat format;

    // mipmap count
    uint32_t mipmapCount;
};

void readTEX0Header(Buffer& data, TEX0* tex0, TEX0Header* header)
{
    const uint32_t dataSize = static_cast<uint32_t>(data.remaining());

    if (dataSize < 0x18)
    {
        throw BRRESError(Strings::format(
            "TEX0: Invalid header! Not enough bytes in buffer! (%d < 24)",
            dataSize
        ));
    }

    if (!Bytes::matchesString("TEX0", *data + 0x00, 0x4))
    {
        throw BRRESError(Strings::format(
            "TEX0: Invalid header! Invalid magic! (Expected 'TEX0', Got '%s')",
            Strings::stringify(*data + 0x00, 0x4).c_str()
        ));
    }
    data.getInt(); // skip magic

    uint32_t size = data.getInt();
    if (dataSize < size)
    {
        throw BRRESError(Strings::format(
            "TEX0: Invalid data! Not enough bytes in buffer! (%d < %d)",
            dataSize, size
        ));
    }

    uint32_t version = data.getInt();
    if (version != 1 && version != 3)
    {
        throw BRRESError(Strings::format(
            "TEX0: Unsupported version number (%d)! (Supported version numbers are 1 and 3)",
            version
        ));
    }

    data.getInt(); // ignore offset to BRRES

    header->dataOff = data.getInt();
    if (header->dataOff > size)
    {
        throw BRRESError(Strings::format(
            "TEX0: Data offset is out of range! (%d > %d)",
            header->dataOff, size
        ));
    }

    std::string name = readBRRESString(data, data.getInt());
    if (name != tex0->getName())
    {
        throw BRRESError(Strings::format(
            "TEX0: Section name does not match index group entry name! (%s != %s)",
            name.c_str(), tex0->getName().c_str()
        ));
    }

    data.limit(size);
}

void readTEX0ImageInfo(Buffer& data, TEX0ImageInfo* info)
{
    if (data.remaining() < 0x1C)
    {
        throw BRRESError(Strings::format(
            "TEX0: Invalid image info header! Not enough bytes in buffer! (%d < 28)",
            data.remaining()
        ));
    }

    data.getInt(); // unknown/unused

    info->width = data.getShort();
    info->height = data.getShort();
    info->format = static_cast<ImageFormat>(data.getInt());

    uint32_t imageCount = data.getInt();
    if (imageCount < 1)
    {
        throw BRRESError("TEX0: Invalid data! Image count is 0!");
    }
    info->mipmapCount = imageCount - 1;

    data.getInt(); // unknown/unused

    uint32_t mipmapCount = static_cast<uint32_t>(data.getFloat());
    if (mipmapCount != info->mipmapCount)
    {
        throw BRRESError(Strings::format(
            "TEX0: Mipmap count does not match image count! (%d != %d - 1)",
            mipmapCount, imageCount
        ));
    }

    data.getInt(); // unknown/unused
}

void readTEX0Data(Buffer& data, TEX0* tex0, TEX0Header* header, TEX0ImageInfo* info)
{
    data.position(header->dataOff);

    tex0->setTextureData(data, info->width, info->height, info->format);
    for (uint32_t i = 0; i < info->mipmapCount; ++i)
    {
        tex0->setMipmapTextureData(i, data);
    }
}

void readTEX0(Buffer& buffer, TEX0* tex0)
{
    Buffer data = buffer.slice();

    ////// Read header /////////////////
    TEX0Header header;
    readTEX0Header(data, tex0, &header);

    ////// Read image info /////////////
    TEX0ImageInfo info;
    readTEX0ImageInfo(data, &info);

    ////// Read data ///////////////////
    readTEX0Data(data, tex0, &header, &info);
}
}
