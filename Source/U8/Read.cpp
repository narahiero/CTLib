//////////////////////////////////////////////////
//  Copyright (c) 2020 Nara Hiero
//
// This file is licensed under GPLv3+
// Refer to the `License.txt` file included.
//////////////////////////////////////////////////

#include <CTLib/U8.hpp>

namespace CTLib
{

// u8 archive header
struct U8Header
{
    // offset to the first node in the filesystem section (usually 0x20)
    uint32_t entriesOff;

    // size of the filesystem section including the string table
    uint32_t entriesSize;

    // offset to the beginning of file data
    uint32_t dataOff;
};

// a node in the filesystem section
struct U8Node
{
    // node index
    uint32_t index;

    // node type (0 for file, 1 for directory)
    uint8_t type;

    // offset to name (24 bits; 0x00FFFFFF mask)
    uint32_t nameOff;

    /* file: offset to the data
     * directory: index of the parent node */
    uint32_t offIdx;

    /* file: size of the data
     * directory: index of first entry not part of this node */
    uint32_t size;
};

void readHeader(Buffer& data, U8Header* header)
{
    if (data.remaining() < 0x20)
    {
        throw U8Error("Invalid U8 archive header!");
    }

    uint8_t magic[4];
    data.getArray(magic, 4);
    if (!Bytes::matchesString("U\xAA""8-", magic, 4))
    {
        throw U8Error(Strings::format(
            "Data is not a valid U8 archive: Invalid magic! (Got \"%s\")",
            Strings::stringify(magic, 4).c_str()
        ));
    }

    header->entriesOff = data.getInt();
    if (header->entriesOff < 0x20) // would overlap with header
    {
        throw U8Error(Strings::format(
            "Invalid U8 archive: Illegal entries section offset! (Got 0x%02X)",
            header->entriesOff
        ));
    }

    header->entriesSize = data.getInt();
    if (header->entriesSize < 0xC) // less than one node
    {
        throw U8Error(Strings::format(
            "Invalid U8 archive: Illegal entries section size! (Got 0x%02X)",
            header->entriesSize
        ));
    }

    header->dataOff = data.getInt();
    if (header->dataOff < (header->entriesOff + header->entriesSize))
    {
        throw U8Error(
            "Invalid U8 archive: Data section overlaps with entries section!"
        );
    }

    // 4 unused integers follows
    data.getInt();
    data.getInt();
    data.getInt();
    data.getInt();
}

U8Node readNode(Buffer& filesystem, std::vector<U8Node>& out, uint32_t max)
{
    U8Node node;

    node.index = static_cast<uint32_t>(out.size());

    // u32 containing: u8 for type, u24 for name off
    uint32_t tn = filesystem.getInt();

    node.type = tn >> 24;
    if (node.type != 0 && node.type != 1)
    {
        throw U8Error(Strings::format(
            "Invalid U8 filesystem section: Node with invalid type! (Got 0x%02X)",
            node.type
        ));
    }

    // apply mask since first byte is type
    node.nameOff = tn & 0x00FFFFFF;

    node.offIdx = filesystem.getInt();
    if (node.type == 1 && node.offIdx >= max)
    {
        throw U8Error(Strings::format(
            "Invalid U8 filesystem section: Directory node with invalid parent index! "
            "(Index 0x%02X >= Max 0x%02X)",
            node.offIdx, max
        ));
    }

    node.size = filesystem.getInt();
    if (node.type == 1 && node.size > max)
    {
        throw U8Error(Strings::format(
            "Invalid U8 filesystem section: Directory node with invalid first node out index! "
            "(Index 0x%02X > Max 0x%02X)",
            node.size, max
        ));
    }

    out.push_back(node);
    return node;
}

// simply returns the index of the last null character '\0' in table
uint32_t getMaxStringTableOffset(Buffer& stringTable)
{
    if (stringTable.remaining() <= 0)
    {
        throw U8Error("Invalid U8 filesystem section: String table is empty!");
    }

    uint32_t max = 0;
    for (uint32_t i = 0; i < stringTable.remaining(); ++i)
    {
        if (stringTable[i] == '\0')
        {
            max = i;
        }
    }
    return max;
}

std::string readNodeName(Buffer& stringTable, uint32_t off, uint32_t max)
{
    if (off > max) // the string will not be properly null terminated or is out of bounds
    {
        throw U8Error("Invalid U8 filesystem section: Invalid string table!");
    }
    return std::string((const char*)(*stringTable + off));
}

void readFileData(Buffer& base, U8Node node, U8File* file)
{
    Buffer data = base.position(node.offIdx).slice();
    base.rewind();

    if (data.capacity() < node.size)
    {
        throw U8Error("Invalid U8 data section: Not enough data remaining!");
    }
    data.limit(node.size);

    Buffer filedata(node.size);
    filedata.put(data).flip();
    file->setData(std::move(filedata));
}

// 'filesystem' points to filesystem section
// 'data' points to file data section
U8Arc readData(Buffer& filesystem, Buffer& data, U8Header* header)
{
    U8Arc arc;

    std::vector<U8Node> nodes;
    U8Node root = readNode(filesystem, nodes, ~0Ui32); // root node
    if (root.type != 1)
    {
        throw U8Error("Invalid U8 filesystem section: Root entry is not a directory!");
    }

    if (filesystem.remaining() < ((root.size - 1) * 0xC))
    {
        throw U8Error("Invalid U8 filesystem section: Not enough data for nodes!");
    }

    nodes.reserve(root.size);
    while (nodes.size() < root.size) // get remaining nodes
    {
        readNode(filesystem, nodes, root.size);
    }

    // now that the nodes are read, the remaining data is the string table
    Buffer stringTable = filesystem.slice();
    uint32_t stringTableMax = getMaxStringTableOffset(stringTable);

    U8Node dir = root;
    U8Dir* parent = arc.asDirectory();
    for (uint32_t i = 1; i < root.size; ++i)
    {
        while (dir.index != dir.offIdx && i >= dir.size)
        {
            dir = nodes[dir.offIdx];
            parent = parent->getParent();
        }

        U8Node node = nodes[i];
        std::string name = readNodeName(stringTable, node.nameOff, stringTableMax);

        if (node.type == 0) // file
        {
            U8File* file = parent->addFile(name);
            readFileData(data, node, file);
        }
        else if (node.type == 1) // directory
        {
            parent = parent->addDirectory(name);
            dir = node;
        }
    }

    return arc;
}

U8Arc U8::read(Buffer& data)
{
    Buffer buffer = data.slice();

    // order must be big endian
    buffer.order(Buffer::BIG_ENDIAN);

    // parse header with some error checking
    U8Header header;
    readHeader(buffer, &header);

    // move to the filesystem section
    buffer.position(buffer.position() + (header.entriesOff - 0x20));
    Buffer filesystem = buffer.slice();
    filesystem.limit(header.entriesSize);

    // move back to the beginning for data
    buffer.rewind();

    // parse the actual U8 archive
    U8Arc arc = readData(filesystem, buffer, &header);

    // pretend the data was read in a normal way :-)
    data.position(data.limit());

    return arc;
}
}
