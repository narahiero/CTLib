//////////////////////////////////////////////////
//  Copyright (c) 2020 Nara Hiero
//
// This file is licensed under GPLv3+
// Refer to the `License.txt` file included.
//////////////////////////////////////////////////

#include <CTLib/U8.hpp>

namespace CTLib
{

// info necessary to create the u8 archive
struct U8Info
{
    // total size of filesystem section (including string table)
    uint32_t entriesSize;

    // offset to file data
    uint32_t dataOff;

    // total file size
    uint32_t size;

    // data offset table
    std::vector<uint32_t> offsets;
};

// string table in filesystem section of u8 archive
struct U8StringTable
{
    // map of <string, offset> of strings in table
    std::map<std::string, uint32_t> offsets;

    // total size of table in bytes
    uint32_t size;
};

// node in filesystem section
struct U8PackedNode
{
    // index of node
    uint32_t idx;

    // packed node type and name offset
    uint32_t tn;

    // data offset or index to parent directory
    uint32_t offIdx;

    // size of data or index of first node out of directory
    uint32_t size;
};

uint32_t padNum(uint32_t num, uint8_t pad)
{
    return (num & ~(pad - 1)) + ((num & (pad - 1)) > 0 ? pad : 0);
}

void orderEntries(U8Dir* dir, std::vector<U8Entry*>& out)
{
    for (U8Entry* entry : *dir)
    {
        out.push_back(entry);
        if (entry->getType() == U8EntryType::Directory)
        {
            orderEntries(entry->asDirectory(), out);
        }
    }
}

void makeStringTable(std::vector<U8Entry*>& entries, U8StringTable* table)
{
    // initialize table
    table->offsets[""] = 0; // root entry
    table->size = 1;

    for (U8Entry* entry : entries) // first run to order properly
    {
        table->offsets[entry->getName()] = 1; // 1 to differentiate from root
    }

    for (auto pair : table->offsets) // actually calculate offsets
    {
        if (pair.second == 1) // not root node
        {
            table->offsets[pair.first] = table->size;
            table->size += static_cast<uint32_t>(pair.first.size()) + 1;
        }
    }
}

void makeInfo(std::vector<U8Entry*>& entries, U8StringTable* table, U8Info* info)
{
    info->entriesSize = (static_cast<uint32_t>(entries.size() + 1) * 0xC) + table->size;
    info->dataOff = padNum(info->entriesSize + 0x30, 0x40);

    info->offsets.push_back(0); // for the root node

    uint32_t dataSize = 0;
    for (U8Entry* entry : entries)
    {
        if (entry->getType() == U8EntryType::File)
        {
            uint32_t fileSize = entry->asFile()->getDataSize();
            info->offsets.push_back(fileSize == 0 ? 0 : info->dataOff + dataSize);
            dataSize += padNum(fileSize, 0x20);
        }
        else
        {
            info->offsets.push_back(0);
        }
    }
    info->size = info->dataOff + dataSize;
}

void writeHeader(U8Info* info, Buffer& out)
{
    out.putArray((uint8_t*)"U\xAA""8-", 4); // magic
    out.putInt(0x20); // offset to filesystem section
    out.putInt(info->entriesSize); // size of filesystem section
    out.putInt(info->dataOff); // offset to data section

    // 4 unused integers
    for (size_t i = 0; i < 0x10; ++i)
    {
        out.put(0);
    }
}

U8PackedNode makeRootNode(uint32_t size)
{
    U8PackedNode node;
    node.idx = 0;
    node.tn = 0x1 << 24;
    node.offIdx = 0;
    node.size = size;
    return node;
}

U8PackedNode makeNode(
    U8Entry* entry, U8PackedNode* parent, U8Info* info, U8StringTable* table, uint32_t idx
)
{
    U8PackedNode node;

    // node index
    node.idx = idx;

    // name offset in string table
    node.tn = table->offsets[entry->getName()] & 0x00FFFFFF;
    
    if (entry->getType() == U8EntryType::Directory)
    {
        node.tn |= 0x1 << 24; // entry type
        node.offIdx = parent->idx;
        node.size = 0;
    }
    else if (entry->getType() == U8EntryType::File)
    {
        node.tn |= 0x0 << 24; // entry type

        U8File* file = entry->asFile();
        node.offIdx = info->offsets[idx]; // offset to data
        node.size = file->getDataSize(); // data size
    }

    return node;
}

void getOrderedNodes(
    U8Dir* dir, U8PackedNode* parent, U8Info* info, U8StringTable* table,
    std::vector<U8PackedNode>& nodes
)
{
    for (U8Entry* entry : *dir)
    {
        U8PackedNode node = makeNode(entry, parent, info, table, static_cast<uint32_t>(nodes.size()));
        nodes.push_back(node);

        if (entry->getType() == U8EntryType::Directory)
        {
            getOrderedNodes(entry->asDirectory(), &node, info, table, nodes);
            nodes[node.idx].size = static_cast<uint32_t>(nodes.size());
        }
    }
}

void writeNodes(const U8Arc& arc, U8Info* info, U8StringTable* table, Buffer& out)
{
    std::vector<U8PackedNode> nodes;

    U8PackedNode root = makeRootNode(arc.totalCount() + 1);
    nodes.push_back(root);

    getOrderedNodes(arc.asDirectory(), &root, info, table, nodes);

    for (U8PackedNode& node : nodes)
    {
        out.putInt(node.tn);
        out.putInt(node.offIdx);
        out.putInt(node.size);
    }
}

void writeStringTable(U8Info* info, U8StringTable* table, Buffer& out)
{
    // write table
    for (auto& pair : table->offsets)
    {
        out.putArray((uint8_t*)pair.first.c_str(), pair.first.size() + 1);
    }

    // add padding
    while (out.position() < info->dataOff)
    {
        out.put(0);
    }
}

void writeData(std::vector<U8Entry*> entries, Buffer& out)
{
    for (U8Entry* entry : entries)
    {
        if (entry->getType() == U8EntryType::File)
        {
            out.put(entry->asFile()->getData());

            size_t padding = 0x20 - (out.position() & 0x1F);
            while (padding != 0x20 && padding-- > 0)
            {
                out.put(0);
            }
        }
    }
}

Buffer U8::write(const U8Arc& arc)
{
    std::vector<U8Entry*> entries;
    orderEntries(arc.asDirectory(), entries);

    U8StringTable table;
    makeStringTable(entries, &table);

    U8Info info;
    makeInfo(entries, &table, &info);

    Buffer data(info.size);
    writeHeader(&info, data);
    writeNodes(arc, &info, &table, data);
    writeStringTable(&info, &table, data);
    writeData(entries, data);

    return data.clear();
}
}
