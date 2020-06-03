//////////////////////////////////////////////////
//  Copyright (c) 2020 Nara Hiero
//
// This file is licensed under GPLv3+
// Refer to the `License.txt` file included.
//////////////////////////////////////////////////

#include <CTLib/KCL.hpp>

#include <map>
#include <vector>

namespace CTLib
{

constexpr uint8_t VERTICES = 0;
constexpr uint8_t NORMALS = 1;
constexpr uint8_t TRIANGLES = 2;
constexpr uint8_t OCTREE = 3;

struct KCLInfo
{

    // size in bytes of file
    uint32_t size;
};

struct KCLOffsets
{

    // offsets to sections
    std::vector<uint32_t> sectionOffs;

    // offsets to octree nodes relative to octree section start
    std::map<KCL::OctreeNode*, uint32_t> nodeOffs;

    // offset of octree triangle lists
    uint32_t triListOff;
};

uint32_t calcNodeSizeAndOffsets(KCL::OctreeNode* node, uint32_t& pos, KCLOffsets* offsets)
{
    if (node->isSuperNode())
    {
        offsets->nodeOffs.insert(std::map<KCL::OctreeNode*, uint32_t>::value_type(node, pos));
        pos += 0x20;

        uint32_t size = 0;
        for (uint32_t i = 0; i < 8; ++i)
        {
            size += calcNodeSizeAndOffsets(node->getChild(i), pos, offsets);
        }
        return 4 + size;
    }
    else
    {
        return 4;
    }
}

void createKCLInfoAndOffsets(const KCL& kcl, KCLInfo* info, KCLOffsets* offsets)
{
    info->size = 0x3C; // header

    offsets->sectionOffs.push_back(info->size); // VERTICES
    info->size += static_cast<uint32_t>(kcl.getVertices().size()) * 0xC;

    offsets->sectionOffs.push_back(info->size); // NORMALS
    info->size += static_cast<uint32_t>(kcl.getNormals().size()) * 0xC;

    offsets->sectionOffs.push_back(info->size); // TRIANGLES
    info->size += static_cast<uint32_t>(kcl.getTriangles().size()) * 0x10;

    offsets->sectionOffs.push_back(info->size); // OCTREE

    KCL::Octree* octree = kcl.getOctree();

    uint32_t count = octree->getRootNodeCount();
    uint32_t pos = count * 4;
    for (uint32_t i = 0; i < count; ++i)
    {
        info->size += calcNodeSizeAndOffsets(octree->getNode(i), pos, offsets);
    }

    offsets->triListOff = info->size;   
    info->size += 2; // first empty list
    for (KCL::OctreeNode* node : octree->getAllNodes())
    {
        if (!node->isSuperNode())
        {
            uint32_t size = static_cast<uint32_t>(node->getIndices().size());
            info->size += size == 0 ? 0 : (size + 1) * 2;
        }
    }
}

void writeKCLHeader(Buffer& out, const KCL& kcl, KCLOffsets* offsets)
{
    // section offsets
    out.putInt(offsets->sectionOffs.at(VERTICES));
    out.putInt(offsets->sectionOffs.at(NORMALS));
    out.putInt(offsets->sectionOffs.at(TRIANGLES) - 0x10);
    out.putInt(offsets->sectionOffs.at(OCTREE));

    out.putFloat(300.f); // unknown

    // octree settings
    KCL::Octree* octree = kcl.getOctree();

    // area bounds
    octree->getMinPos().put(out);
    out.putInt(octree->getMaskX());
    out.putInt(octree->getMaskY());
    out.putInt(octree->getMaskZ());

    // coord shifts
    out.putInt(octree->getShift());
    out.putInt(octree->getShiftY());
    out.putInt(octree->getShiftZ());

    out.putFloat(250.f); // unknown
}

void writeKCLVertices(Buffer& out, const KCL& kcl, KCLOffsets* offsets)
{
    out.position(offsets->sectionOffs.at(VERTICES));

    for (Vector3f v : kcl.getVertices())
    {
        v.put(out);
    }
}

void writeKCLNormals(Buffer& out, const KCL& kcl, KCLOffsets* offsets)
{
    out.position(offsets->sectionOffs.at(NORMALS));

    for (Vector3f v : kcl.getNormals())
    {
        v.put(out);
    }
}

void writeKCLTriangles(Buffer& out, const KCL& kcl, KCLOffsets* offsets)
{
    out.position(offsets->sectionOffs.at(TRIANGLES));

    for (KCL::Triangle t : kcl.getTriangles())
    {
        out.putFloat(t.length);
        out.putShort(t.position);
        out.putShort(t.direction);
        out.putShort(t.normA);
        out.putShort(t.normB);
        out.putShort(t.normC);
        out.putShort(t.flag);
    }
}

void writeKCLOctreeNode(
    Buffer& out, Buffer& triOut, KCL::OctreeNode* node, KCLOffsets* offsets, uint32_t pos
)
{
    if (node->isSuperNode())
    {
        uint32_t off = offsets->nodeOffs.at(node);
        out.putInt(off - pos);

        Buffer tmp = out.duplicate(); // to prevent from changing the original buffer's position
        tmp.position(offsets->sectionOffs.at(OCTREE) + off);
        for (uint32_t i = 0; i < 8; ++i)
        {
            writeKCLOctreeNode(tmp, triOut, node->getChild(i), offsets, off);
        }
    }
    else
    {
        uint32_t baseOff = offsets->triListOff - offsets->sectionOffs.at(OCTREE) - pos;
        std::vector<uint16_t> tris = node->getIndices();
        if (tris.empty())
        {
            out.putInt(0x80000000 | baseOff);
        }
        else
        {
            out.putInt(0x80000000 | (baseOff + static_cast<uint32_t>(triOut.position())));
            for (uint16_t tri : tris)
            {
                triOut.putShort(tri + 1);
            }
            triOut.putShort(0x0000);
        }
    }
}

void writeKCLOctree(Buffer& out, const KCL& kcl, KCLOffsets* offsets)
{
    uint32_t baseOff = offsets->sectionOffs.at(OCTREE);
    out.position(baseOff);

    Buffer triOut = out.duplicate();
    triOut = triOut.position(offsets->triListOff).slice();
    triOut.putShort(0x0000); // empty list

    KCL::Octree* octree = kcl.getOctree();
    for (uint32_t i = 0; i < octree->getRootNodeCount(); ++i)
    {
        writeKCLOctreeNode(out, triOut, octree->getNode(i), offsets, 0);
    }
}

Buffer KCL::write(const KCL& kcl)
{
    ////////////////////////////////////
    /// Setup required information

    KCLInfo info;
    KCLOffsets offsets;
    createKCLInfoAndOffsets(kcl, &info, &offsets);

    ////////////////////////////////////
    /// Write MDL0 file

    Buffer out(info.size);

    writeKCLHeader(out, kcl, &offsets);
    writeKCLVertices(out, kcl, &offsets);
    writeKCLNormals(out, kcl, &offsets);
    writeKCLTriangles(out, kcl, &offsets);
    writeKCLOctree(out, kcl, &offsets);

    return out.clear();
}
}
