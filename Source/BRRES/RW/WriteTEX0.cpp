//////////////////////////////////////////////////
//  Copyright (c) 2020 Nara Hiero
//
// This file is licensed under GPLv3+
// Refer to the `License.txt` file included.
//////////////////////////////////////////////////

#include "BRRES/RW/BRRESRWCommon.hpp"

namespace CTLib
{

void addTEX0StringsToTable(BRRESStringTable* table, TEX0* tex0)
{
    // there are no internal strings used by TEX0
}

uint32_t calculateTEX0Size(TEX0* tex0)
{
    uint32_t size = 0x40; // header size
    
    size += static_cast<uint32_t>(tex0->getTextureData().remaining());
    for (uint32_t i = 0; i < tex0->getMipmapCount(); ++i)
    {
        size += static_cast<uint32_t>(tex0->getMipmapTextureData(i).remaining());
    }

    return padNumber(size, 0x10);
}

struct TEX0Info
{

    // size of file in bytes
    uint32_t size;

    // offset to outer BRRES file
    int32_t offToBRRES;

    // offset to data
    uint32_t dataOff;
    
    // offset to name
    uint32_t nameOff;
};

void createInfo(
    TEX0* tex0, TEX0Info* info, int32_t offToBRRES, BRRESStringTable* table, uint32_t tableOff
)
{
    info->size = calculateTEX0Size(tex0);
    info->offToBRRES = offToBRRES;
    info->dataOff = 0x40;
    info->nameOff = table->offsets.at(tex0->getName()) + tableOff;
}

void writeHeader(Buffer& out, TEX0Info* info)
{
    out.putArray((uint8_t*)"TEX0", 4);
    out.putInt(info->size);
    out.putInt(1); // TEX0 version
    out.putInt(static_cast<uint32_t>(info->offToBRRES));
    out.putInt(info->dataOff); // offset to data
    out.putInt(info->nameOff);
}

void writeTEX0Header(Buffer& out, TEX0* tex0)
{
    out.putInt(0); // unknown/unused
    out.putShort(tex0->getWidth());
    out.putShort(tex0->getHeight());
    out.putInt(static_cast<uint32_t>(tex0->getFormat()));
    out.putInt(tex0->getMipmapCount() + 1); // image count
    out.putInt(0); // unknown/unused
    out.putFloat(static_cast<float>(tex0->getMipmapCount()));
    out.putInt(0); // unknown/unused
}

void writeTextureData(Buffer& out, TEX0Info* info, TEX0* tex0)
{
    out.position(info->dataOff);

    out.put(tex0->getTextureData());
    for (uint32_t i = 0; i < tex0->getMipmapCount(); ++i)
    {
        out.put(tex0->getMipmapTextureData(i));
    }
}

void writeTEX0(
    Buffer& out, TEX0* tex0, int32_t offToBRRES, BRRESStringTable* table, uint32_t tableOff
)
{
    TEX0Info info;
    createInfo(tex0, &info, offToBRRES, table, tableOff);

    writeHeader(out, &info);
    writeTEX0Header(out, tex0);
    writeTextureData(out, &info, tex0);
}
}
