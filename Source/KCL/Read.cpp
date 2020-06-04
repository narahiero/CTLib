//////////////////////////////////////////////////
//  Copyright (c) 2020 Nara Hiero
//
// This file is licensed under GPLv3+
// Refer to the `License.txt` file included.
//////////////////////////////////////////////////

#include <CTLib/KCL.hpp>

#include <CTLib/Utilities.hpp>

namespace CTLib
{

struct KCLHeader
{

    // offset to kcl sections
    uint32_t vertsOff, normsOff, trisOff, octreeOff;

    // octree minimum position
    Vector3f minPos;

    // octree coord masks
    uint32_t maskX, maskY, maskZ;

    // octree coord shifts
    uint32_t shift, shiftY, shiftZ;
};

void assertValidMask(uint32_t mask, const char axis)
{
    uint8_t index = 31;
    while (index > 0 && ((mask >> index) & 1) == 1)
    {
        --index;
    }
    while (index > 0 && ((mask >> index) & 1) == 0)
    {
        --index;
    }

    if (index > 0)
    {
        throw KCLError(Strings::format(
            "KCL: Invalid mask for %c-axis! (0x%08X)",
            axis, mask
        ));
    }
}

void readKCLHeader(Buffer& data, KCLHeader* header)
{
    uint32_t dataSize = static_cast<uint32_t>(data.remaining());

    if (dataSize < 0x3C)
    {
        throw KCLError(Strings::format(
            "KCL: Invalid KCL data! Not enough bytes for header! (%d < 60)",
            dataSize
        ));
    }

    header->vertsOff = data.getInt();
    if (header->vertsOff >= dataSize)
    {
        throw KCLError(Strings::format(
            "KCL: Invalid KCL data! Vertices section offset out of bounds! (%d >= %d)",
            header->vertsOff, dataSize
        ));
    }

    header->normsOff = data.getInt();
    if (header->normsOff >= dataSize)
    {
        throw KCLError(Strings::format(
            "KCL: Invalid KCL data! Normals section offset out of bounds! (%d >= %d)",
            header->normsOff, dataSize
        ));
    }
    else if (header->normsOff < header->vertsOff)
    {
        throw KCLError(Strings::format(
            "KCL: The normals section must be located after the vertices section! (%d < %d)",
            header->normsOff, header->vertsOff
        ));
    }

    header->trisOff = data.getInt();
    if (header->trisOff >= dataSize)
    {
        throw KCLError(Strings::format(
            "KCL: Invalid KCL data! Triangles section offset out of bounds! (%d >= %d)",
            header->trisOff, dataSize
        ));
    }
    else if (header->trisOff < header->normsOff)
    {
        throw KCLError(Strings::format(
            "KCL: The triangles section must be located after the normals section! (%d < %d)",
            header->trisOff, header->normsOff
        ));
    }

    header->octreeOff = data.getInt();
    if (header->octreeOff >= dataSize)
    {
        throw KCLError(Strings::format(
            "KCL: Invalid KCL data! Octree section offset out of bounds! (%d >= %d)",
            header->octreeOff, dataSize
        ));
    }
    else if (header->octreeOff < header->trisOff)
    {
        throw KCLError(Strings::format(
            "KCL: The octree section must be located after the triangles section! (%d < %d)",
            header->octreeOff, header->trisOff
        ));
    }

    data.getFloat(); // ignore unknown value

    header->minPos.get(data);

    header->maskX = data.getInt();
    assertValidMask(header->maskX, 'X');

    header->maskY = data.getInt();
    assertValidMask(header->maskY, 'Y');

    header->maskZ = data.getInt();
    assertValidMask(header->maskZ, 'Z');

    header->shift = data.getInt();
    header->shiftY = data.getInt();
    header->shiftZ = data.getInt();

    data.getFloat(); // ignore unknown value
}

std::vector<Vector3f> readKCLVertices(Buffer& data, KCLHeader* header)
{
    uint32_t count = (header->normsOff - header->vertsOff) / 0xC;

    std::vector<Vector3f> vertices;
    vertices.reserve(count);
    for (uint32_t i = 0; i < count; ++i)
    {
        Vector3f v;
        v.get(data);
        vertices.push_back(v);
    }

    return vertices;
}

std::vector<Vector3f> readKCLNormals(Buffer& data, KCLHeader* header)
{
    uint32_t count = (header->trisOff + 0x10 - header->normsOff) / 0xC;

    std::vector<Vector3f> normals;
    normals.reserve(count);
    for (uint32_t i = 0; i < count; ++i)
    {
        Vector3f n;
        n.get(data);
        normals.push_back(n);
    }

    return normals;
}

std::vector<KCL::Triangle> readKCLTriangles(Buffer& data, KCLHeader* header)
{
    uint32_t count = (header->octreeOff - header->trisOff - 0x10) / 0x10;

    std::vector<KCL::Triangle> triangles;
    triangles.reserve(count);
    for (uint32_t i = 0; i < count; ++i)
    {
        KCL::Triangle t;
        t.length = data.getFloat();
        t.position = data.getShort();
        t.direction = data.getShort();
        t.normA = data.getShort();
        t.normB = data.getShort();
        t.normC = data.getShort();
        t.flag = data.getShort();
        triangles.push_back(t);
    }

    return triangles;
}

void KCL::readOctree(Buffer& buffer, KCL::Octree* octree)
{
    Buffer data = buffer.slice();

    octree->genRootNodes();
    for (uint32_t i = 0; i < octree->getRootNodeCount(); ++i)
    {
        readOctreeNode(data, octree->getNode(i), 0);
    }
}

void KCL::readOctreeNode(Buffer& data, KCL::OctreeNode* node, uint32_t pos)
{
    uint32_t nv = data.getInt();
    if (nv >> 31) // triangle list node
    {
        Buffer triData = data.duplicate();
        triData.position(pos + (nv & 0x7FFFFFFF) + 2);

        uint16_t idx;
        while ((idx = triData.getShort()) != 0)
        {
            node->elems.push_back(node->octree->toElem(idx - 1));
        }
    }
    else // super node
    {
        Buffer nodeData = data.duplicate();
        nodeData.position(pos + nv);

        node->split();
        for (uint8_t i = 0; i < 8; ++i)
        {
            readOctreeNode(nodeData, node->getChild(i), pos + nv);
        }
    }
}

KCL KCL::read(Buffer& buffer)
{
    Buffer data = buffer.slice();

    // Read header
    KCLHeader header;
    readKCLHeader(data, &header);

    KCL kcl;

    // Set octree values
    KCL::Octree* octree = kcl.getOctree();
    octree->minPos = header.minPos;
    octree->maskX = header.maskX;
    octree->maskY = header.maskY;
    octree->maskZ = header.maskZ;
    octree->shift = header.shift;
    octree->shiftY = header.shiftY;
    octree->shiftZ = header.shiftZ;

    // Read data arrays
    kcl.vertices = readKCLVertices(data, &header);
    kcl.normals = readKCLNormals(data, &header);
    kcl.triangles = readKCLTriangles(data, &header);

    // Read octree
    readOctree(data, octree);

    return kcl;
}
}
