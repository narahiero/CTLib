//////////////////////////////////////////////////
//  Copyright (c) 2020 Nara Hiero
//
// This file is licensed under GPLv3+
// Refer to the `License.txt` file included.
//////////////////////////////////////////////////

#include <CTLib/KCL.hpp>

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

    std::vector<uint32_t> sectionOffs;
};

void createKCLInfoAndOffsets(const KCL& kcl, KCLInfo* info, KCLOffsets* offsets)
{
    info->size = 0x3C; // header

    offsets->sectionOffs.push_back(info->size);
    info->size += static_cast<uint32_t>(kcl.getVertices().size()) * 0xC;

    offsets->sectionOffs.push_back(info->size);
    info->size += static_cast<uint32_t>(kcl.getNormals().size()) * 0xC;

    offsets->sectionOffs.push_back(info->size);
    info->size += static_cast<uint32_t>(kcl.getTriangles().size()) * 0x10;

    offsets->sectionOffs.push_back(info->size);
    info->size += 0x4000;
}

void writeKCLHeader(Buffer& out, const KCL& kcl, KCLOffsets* offsets)
{
    // section offsets
    out.putInt(offsets->sectionOffs.at(VERTICES));
    out.putInt(offsets->sectionOffs.at(NORMALS));
    out.putInt(offsets->sectionOffs.at(TRIANGLES) - 0x10);
    out.putInt(offsets->sectionOffs.at(OCTREE));

    out.putFloat(300.f); // unknown

    // area bounds
    kcl.getMinPos().put(out);
    out.putInt(kcl.getMaskX());
    out.putInt(kcl.getMaskY());
    out.putInt(kcl.getMaskZ());

    out.putInt(0x0); // coord-shift
    out.putInt(0x1); // y-shift
    out.putInt(0x2); // z-shift

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

void writeKCLOctree(Buffer& out, const KCL& kcl, KCLOffsets* offsets)
{
    out.position(offsets->sectionOffs.at(OCTREE));

    KCL::OctreeNode* root = kcl.getRootNode();
    if (root == nullptr)
    {
        return;
    }

    for (uint8_t i = 0; i < 8; ++i)
    {
        KCL::OctreeNode* node = root->getChild(i);

        // write node ...
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
