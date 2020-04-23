//////////////////////////////////////////////////
//  Copyright (c) 2020 Nara Hiero
//
// This file is licensed under GPLv3+
// Refer to the `License.txt` file included.
//////////////////////////////////////////////////

#include "BRRES/RW/BRRESRWCommon.hpp"
#include <iostream>
namespace CTLib
{

constexpr const char* TEX0_GROUP = "Textures(NW4R)";

struct BRRESInfo
{

    // size of file in bytes
    uint32_t size;

    // section count including root
    uint16_t sectionCount;
};

struct BRRESGroupsInfo
{

    // number of index groups in the BRRES, excluding the root group
    uint32_t count;

    // map of <name, index>
    std::map<std::string, uint32_t> indices;

    // map of <entry, subfile>
    std::map<BRRESIndexGroupEntry*, BRRESSubFile*> subfiles;
};

struct BRRESOffsets
{

    // offsets to all index groups
    std::map<BRRESIndexGroup*, uint32_t> groupOffs;

    // offsets to all subfiles
    std::map<BRRESSubFile*, uint32_t> subfileOffs;

    // offset to string table
    uint32_t tableOff;
};

void createStringTable(const BRRES& brres, BRRESStringTable* table)
{
    if (brres.count<TEX0>() > 0)
    {
        addToStringTable(table, TEX0_GROUP);
        for (TEX0* tex0 : brres.getAll<TEX0>())
        {
            addToStringTable(table, tex0->getName());
        }
    }
}

void createRootIndexGroup(const BRRES& brres, BRRESIndexGroup* root, BRRESGroupsInfo* info)
{
    info->count = 0;

    if (brres.count<TEX0>() > 0)
    {
        root->addEntry(TEX0_GROUP);

        info->indices.insert(
            std::map<std::string, uint32_t>::value_type(TEX0_GROUP, info->count)
        );
        ++info->count;
    }
}

void createIndexGroups(const BRRES& brres, BRRESIndexGroup* groups, BRRESGroupsInfo* info)
{
    if (info->indices.count(TEX0_GROUP) > 0)
    {
        BRRESIndexGroup* group = groups + info->indices.at(TEX0_GROUP);
        for (TEX0* tex0 : brres.getAll<TEX0>())
        {
            BRRESIndexGroupEntry* entry = group->addEntry(tex0->getName());
            info->subfiles.insert(
                std::map<BRRESIndexGroupEntry*, BRRESSubFile*>::value_type(entry, tex0)
            );
        }
    }
}

uint32_t calculateSizeAndOffsets(
    const BRRES& brres, BRRESStringTable* table, BRRESIndexGroup* root,
    BRRESIndexGroup* groups, uint32_t groupCount, BRRESOffsets* offsets
)
{
    uint32_t size = 0x10; // header size
    size += 0x8 + root->getSizeInBytes(); // root section size

    ////////////////////////////////////
    /// index groups
    
    for (uint32_t i = 0; i < groupCount; ++i)
    {
        BRRESIndexGroup* group = groups + i;
        offsets->groupOffs.insert(std::map<BRRESIndexGroup*, uint32_t>::value_type(group, size));
        size += group->getSizeInBytes();
    }

    ////////////////////////////////////
    /// subfiles

    size = padNumber(size, 0x10);

    for (TEX0* tex0 : brres.getAll<TEX0>())
    {
        offsets->subfileOffs.insert(std::map<BRRESSubFile*, uint32_t>::value_type(tex0, size));
        size += calculateTEX0Size(tex0);
    }

    size = padNumber(size, 0x10);

    ////////////////////////////////////
    /// string table

    offsets->tableOff = size;
    size += padNumber(static_cast<uint32_t>(table->data.flip().remaining()), 0x10);

    return padNumber(size, 0x10);
}

void createInfoAndOffsets(
    const BRRES& brres, BRRESInfo* info, BRRESStringTable* table, BRRESIndexGroup* root,
    BRRESIndexGroup* groups, uint32_t groupCount, BRRESOffsets* offsets
)
{
    info->size = calculateSizeAndOffsets(brres, table, root, groups, groupCount, offsets);
    info->sectionCount = brres.getSubfileCount() + 1;
}

void resolveRootGroupOffsets(
    BRRESIndexGroup* root, BRRESGroupsInfo* info, BRRESIndexGroup* groups, BRRESOffsets* offsets
)
{
    std::vector<BRRESIndexGroupEntry*> entries = root->getEntries();
    for (uint32_t i = 0; i < root->getEntryCount(); ++i)
    {
        BRRESIndexGroupEntry* entry = entries[i + 1];
        BRRESIndexGroup* group = groups + info->indices.at(entry->getName());
        entry->setDataOffset(offsets->groupOffs.at(group) - 0x18);
    }
}

void resolveGroupsOffsets(BRRESIndexGroup* groups, BRRESGroupsInfo* info, BRRESOffsets* offsets)
{
    for (uint32_t g = 0; g < info->count; ++g)
    {
        BRRESIndexGroup* group = groups + g;
        uint32_t off = offsets->groupOffs.at(group);

        std::vector<BRRESIndexGroupEntry*> entries = group->getEntries();
        for (uint32_t i = 0; i < group->getEntryCount(); ++i)
        {
            BRRESIndexGroupEntry* entry = entries[i + 1];
            BRRESSubFile* subfile = info->subfiles.at(entry);
            entry->setDataOffset(offsets->subfileOffs.at(subfile) - off);
        }
    }
}

void writeHeader(Buffer& out, BRRESInfo* info)
{
    out.putArray((uint8_t*)"bres", 4);
    out.putShort(0xFEFF); // byte-order mark
    out.putShort(0); // unknown/unused
    out.putInt(info->size);
    out.putShort(0x10); // offset to root section
    out.putShort(info->sectionCount);
}

void writeRootSection(
    Buffer& out, BRRESIndexGroup* root, BRRESStringTable* table, uint32_t tableOff
)
{
    out.putArray((uint8_t*)"root", 4);
    out.putInt(root->getSizeInBytes() + 0x8);

    root->write(out, table, tableOff - static_cast<uint32_t>(out.position()));
}

void writeGroups(
    Buffer& out, BRRESIndexGroup* groups, uint32_t count, BRRESStringTable* table,
    uint32_t tableOff
)
{
    for (uint32_t i = 0; i < count; ++i)
    {
        (groups + i)->write(out, table, tableOff - static_cast<uint32_t>(out.position()));
    }
}

void writeData(Buffer& out, const BRRES& brres, BRRESStringTable* table, BRRESOffsets* offsets)
{
    for (TEX0* tex0 : brres.getAll<TEX0>())
    {
        uint32_t pos = offsets->subfileOffs.at(tex0);
        out.position(pos);

        writeTEX0(
            out.slice(), tex0, -static_cast<int32_t>(pos),
            table->offsets.at(tex0->getName()) + offsets->tableOff - pos
        );
    }
}

void writeStringTable(Buffer& out, BRRESStringTable* table, BRRESOffsets* offsets)
{
    out.position(offsets->tableOff);
    out.put(table->data);
}

void zeroBuffer(Buffer& out)
{
    while (out.hasRemaining())
    {
        out.put(0);
    }
    out.clear();
}

Buffer BRRES::write(const BRRES& brres)
{
    ////////////////////////////////////
    /// Setup required information

    BRRESStringTable table;
    createStringTable(brres, &table);

    BRRESIndexGroup root;
    BRRESGroupsInfo groupsInfo;
    createRootIndexGroup(brres, &root, &groupsInfo);

    auto groups = std::make_unique<BRRESIndexGroup[]>(root.getEntryCount());
    createIndexGroups(brres, groups.get(), &groupsInfo);

    BRRESInfo info;
    BRRESOffsets offsets;
    createInfoAndOffsets(brres, &info, &table, &root, groups.get(), groupsInfo.count, &offsets);

    ////////////////////////////////////
    /// Resolve offsets

    resolveRootGroupOffsets(&root, &groupsInfo, groups.get(), &offsets);
    resolveGroupsOffsets(groups.get(), &groupsInfo, &offsets);

    ////////////////////////////////////
    /// Write BRRES file

    Buffer out(info.size);
    zeroBuffer(out);

    writeHeader(out, &info);
    writeRootSection(out, &root, &table, offsets.tableOff);
    writeGroups(out, groups.get(), groupsInfo.count, &table, offsets.tableOff);
    writeData(out, brres, &table, &offsets);
    writeStringTable(out, &table, &offsets);

    return out.clear();
}
}
