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
struct U8Node
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

void makeStringTable(const U8Arc& arc, U8StringTable* table)
{
    // initialize table
    table->offsets[""] = 0; // root entry
    table->size = 1;

    for (auto it = arc.cbegin(); it != arc.cend(); ++it) // first run to order properly
    {
        table->offsets[(*it)->getName()] = 1; // 1 to differentiate from root
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

void makeInfo(const U8Arc& arc, U8StringTable* table, U8Info* info)
{
    info->entriesSize = ((arc.totalCount() + 1) * 0xC) + table->size;
    info->dataOff = (info->entriesSize & 0xFFFFFFF0) + 0x30;

    info->offsets.push_back(0); // for the root node

    uint32_t dataSize = 0;
    for (auto it = arc.cbegin(); it != arc.cend(); ++it)
    {
        U8Entry* entry = *it;
        if (entry->getType() == U8EntryType::File)
        {
            U8File* file = entry->asFile();
            
            info->offsets.push_back(file->getDataSize() == 0 ? 0 : info->dataOff + dataSize);

            uint32_t fileSize = file->getDataSize();
            dataSize += (fileSize & 0xFFFFFFF0) + ((fileSize & 0xF) > 0 ? 0x10 : 0);
        }
        else
        {
            info->offsets.push_back(0);
        }
    }
    info->size = info->dataOff + dataSize;
}

void writeHeader(const U8Arc& arc, U8Info* info, Buffer& out)
{
    out.putArray((uint8_t*)"U\xAA""8-", 4); // magic
    out.putInt(0x20); // offset to filesystem section
    out.putInt(info->entriesSize); // size of filesystem section
    out.putInt(info->dataOff); // offset to data section

    // 4 unused integers
    for (size_t i = 0; i < 16; ++i)
    {
        out.put(0);
    }
}

U8Node makeRootNode(const U8Arc& arc)
{
    U8Node node;
    node.idx = 0;
    node.tn = 0x1 << 24;
    node.offIdx = 0;
    node.size = arc.totalCount() + 1;
    return node;
}

U8Node makeNode(U8Entry* entry, U8Node* parent, U8Info* info, U8StringTable* table, uint32_t idx)
{
    U8Node node;

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

void writeNode(U8Node* node, Buffer& out)
{
    out.putInt(node->tn);
    out.putInt(node->offIdx);
    out.putInt(node->size);
}

void getOrderedNodes(
    U8Dir* dir, U8Node* parent, U8Info* info, U8StringTable* table, std::vector<U8Node>& nodes
)
{
    for (U8Entry* entry : *dir)
    {
        U8Node node = makeNode(entry, parent, info, table, static_cast<uint32_t>(nodes.size()));
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
    std::vector<U8Node> nodes;

    U8Node root = makeRootNode(arc);
    nodes.push_back(root);

    getOrderedNodes(arc.asDirectory(), &root, info, table, nodes);

    for (U8Node node : nodes)
    {
        writeNode(&node, out);
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

void writeData(const U8Arc& arc, Buffer& out)
{
    for (auto it = arc.cbegin(); it != arc.cend(); ++it)
    {
        if ((*it)->getType() == U8EntryType::File)
        {
            U8File* file = (*it)->asFile();
            out.put(file->getData());

            size_t padding = 0x10 - (out.position() & 0xF);
            while (padding != 0x10 && padding-- > 0)
            {
                out.put(0);
            }
        }
    }
}

Buffer U8::write(const U8Arc& arc)
{
    U8StringTable table;
    makeStringTable(arc, &table);

    U8Info info;
    makeInfo(arc, &table, &info);

    Buffer data(info.size);
    writeHeader(arc, &info, data);
    writeNodes(arc, &info, &table, data);
    writeStringTable(&info, &table, data);
    writeData(arc, data);

    return data.clear();
}
}
