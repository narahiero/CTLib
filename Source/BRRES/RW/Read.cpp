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

// info required to read BRRES
struct BRRESHeader
{

    // size of file in bytes
    uint32_t size;

    // offset to root section in bytes
    uint16_t rootOff;
};

void readBRRESHeader(Buffer& data, BRRESHeader* header)
{
    const uint32_t dataSize = static_cast<uint32_t>(data.remaining());

    if (dataSize < 0x10)
    {
        throw BRRESError("BRRES: Invalid data! Not enough bytes in header!");
    }

    if (!Bytes::matchesString("bres", *data + 0x00, 0x4))
    {
        throw BRRESError(Strings::format(
            "BRRES: Invalid header! Invalid magic! (Expected 'bres', Got '%s')",
            Strings::stringify(*data + 0x00, 4).c_str()
        ));
    }
    data.getInt(); // skip magic

    uint16_t bom = data.getShort();
    if (bom == 0xFFFE)
    {
        throw BRRESError("BRRES: Little endian mode is not supported!");
    }
    else if (bom != 0xFEFF)
    {
        throw BRRESError(Strings::format("BRRES: Invalid BOM! (0x%04X)", bom));
    }

    data.getShort(); // unknown/unused

    header->size = data.getInt();
    if (dataSize < header->size)
    {
        throw BRRESError(Strings::format(
            "BRRES: Invalid data! Not enough bytes in buffer! (%d < %d)",
            dataSize, header->size
        ));
    }
    data.limit(header->size);

    header->rootOff = data.getShort();
    if (header->rootOff > header->size)
    {
        throw BRRESError(Strings::format(
            "BRRES: Invalid header! Root section offset out of range! (%d > %d)",
            header->rootOff, header->size
        ));
    }

    uint16_t sectionCount = data.getShort();
    if (sectionCount < 1)
    {
        throw BRRESError("BRRES: Invalid header! Section count of 0!");
    }
}

void readBRRESRootSection(Buffer& buffer, BRRESHeader* header, BRRESIndexGroup* root)
{
    Buffer data = buffer.position(header->rootOff).slice();

    if (data.remaining() < 0x8)
    {
        throw BRRESError("BRRES: Invalid root section header! Not enough bytes in buffer!");
    }

    if (!Bytes::matchesString("root", *data + 0x00, 0x4))
    {
        throw BRRESError(Strings::format(
            "BRRES: Invalid root section! Invalid magic! (Expected 'root', Got '%s')",
            Strings::stringify(*data + 0x00, 0x4).c_str()
        ));
    }
    data.getInt(); // skip magic

    uint32_t size = data.getInt();
    if (header->size - header->rootOff < size)
    {
        throw BRRESError(Strings::format(
            "BRRES: Invalid root section! Not enough bytes in buffer! (%d < %d)",
            header->size - header->rootOff, size
        ));
    }
    else if (size < 0x10)
    {
        throw BRRESError(Strings::format(
            "BRRES: Invalid root section! Section too small! (%d < 16)",
            size
        ));
    }

    root->read(data);
}

void readBRRESSectionEntry(
    Buffer& data, BRRES& brres, const std::string& name, BRRESIndexGroupEntry* entry
)
{
    if (name == "Textures(NW4R)")
    {
        TEX0* tex0 = brres.add<TEX0>(entry->getName());
        readTEX0(data, tex0);
    }
}

void readBRRESSections(Buffer& data, BRRES& brres, BRRESHeader* header, BRRESIndexGroup* root)
{
    Buffer rootData = data.position(header->rootOff + 0x8).slice();

    for (BRRESIndexGroupEntry* entry : root->getEntries())
    {
        if (entry->isRoot())
        {
            continue;
        }

        rootData.position(entry->getDataOff());
        Buffer entryData = rootData.slice();

        BRRESIndexGroup group;
        group.read(rootData);

        for (BRRESIndexGroupEntry* groupEntry : group.getEntries())
        {
            if (groupEntry->isRoot())
            {
                continue;
            }
            entryData.position(groupEntry->getDataOff());

            readBRRESSectionEntry(entryData, brres, entry->getName(), groupEntry);
        }
    }
}

BRRES BRRES::read(Buffer& buffer)
{
    Buffer data = buffer.slice();

    ////// Read header /////////////////
    BRRESHeader header;
    readBRRESHeader(data, &header);

    ////// Read root section ///////////
    BRRESIndexGroup root;
    readBRRESRootSection(data, &header, &root);

    ////// Read sections ///////////////
    BRRES brres;
    readBRRESSections(data, brres, &header, &root);

    return brres;
}
}
