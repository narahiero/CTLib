//////////////////////////////////////////////////
//  Copyright (c) 2020 Nara Hiero
//
// This file is licensed under GPLv3+
// Refer to the `License.txt` file included.
//////////////////////////////////////////////////

#include <CTLib/KCL.hpp>

#include <limits>
#include <map>
#include <set>

#include <CTLib/Utilities.hpp>

namespace CTLib
{

// non-indexed version of KCL::Triangle
struct Tri
{
    Vector3f pos;
    Vector3f dir;
    Vector3f normA, normB, normC;
    float len;
    uint16_t flag;
};

Vector3f minVec3(Vector3f v0, Vector3f v1)
{
    return
    {
        v0[0] < v1[0] ? v0[0] : v1[0],
        v0[1] < v1[1] ? v0[1] : v1[1],
        v0[2] < v1[2] ? v0[2] : v1[2]
    };
}

Vector3f maxVec3(Vector3f v0, Vector3f v1)
{
    return
    {
        v0[0] > v1[0] ? v0[0] : v1[0],
        v0[1] > v1[1] ? v0[1] : v1[1],
        v0[2] > v1[2] ? v0[2] : v1[2]
    };
}

bool cmpVec3s(const Vector3f& lhs, const Vector3f& rhs)
{
    if (lhs.lengthSquared() < rhs.lengthSquared())
    {
        return true;
    }
    else if (lhs.lengthSquared() > rhs.lengthSquared())
    {
        return false;
    }
    for (uint32_t i = 0; i < 3; ++i)
    {
        if (lhs[i] < rhs[i])
        {
            return true;
        }
        else if (lhs[i] > rhs[i])
        {
            return false;
        }
    }
    return false;
}

using CmpVec3s = decltype(&cmpVec3s);

std::map<Vector3f, uint32_t, CmpVec3s> removeDuplicates(const std::vector<Vector3f>& vs)
{
    std::map<Vector3f, uint32_t, CmpVec3s> m(&cmpVec3s);
    for (Vector3f v : vs)
    {
        m.insert(std::map<Vector3f, uint32_t>::value_type(v, static_cast<uint32_t>(m.size())));
    }
    return m;
}

std::vector<KCL::Triangle> createTriangles(
    const std::map<Vector3f, uint32_t, CmpVec3s>& vMap,
    const std::map<Vector3f, uint32_t, CmpVec3s>& nMap,
    const std::vector<Tri>& tris
)
{
    std::vector<KCL::Triangle> triangles;
    for (Tri t : tris)
    {
        KCL::Triangle triangle;

        triangle.length = t.len;
        triangle.position = vMap.at(t.pos);
        triangle.direction = nMap.at(t.dir);
        triangle.normA = nMap.at(t.normA);
        triangle.normB = nMap.at(t.normB);
        triangle.normC = nMap.at(t.normC);
        triangle.flag = t.flag;

        triangles.push_back(triangle);
    }
    return triangles;
}

std::vector<Vector3f> toStdVector(const std::map<Vector3f, uint32_t, CmpVec3s>& map)
{
    std::vector<Vector3f> vs(map.size());
    for (auto& pair : map)
    {
        vs[pair.second] = pair.first;
    }
    return vs;
}

KCL KCL::fromRawModel(Buffer& vertices, Buffer& kclFlags, int32_t count)
{
    if (count == -1)
    {
        count = static_cast<int32_t>(vertices.remaining() / 36);
    }

    if (vertices.remaining() < count * 36)
    {
        throw KCLError(Strings::format(
            "KCL: Not enough bytes in vertices buffer! (%d < %d)",
            vertices.remaining(), count * 36
        ));
    }
    if (kclFlags.remaining() < count * 2)
    {
        throw KCLError(Strings::format(
            "KCL: Not enough bytes in flags buffer! (%d < %d)",
            kclFlags.remaining(), count * 2
        ));
    }

    if (count == 0)
    {
        return KCL();
    }

    constexpr float MIN = std::numeric_limits<float>::lowest();
    constexpr float MAX = std::numeric_limits<float>::max();
    Vector3f minPos{MAX, MAX, MAX}, maxPos{MIN, MIN, MIN};

    std::vector<Tri> tris;
    std::vector<Vector3f> verts, norms;
    for (int32_t i = 0; i < count; ++i)
    {
        Vector3f v0, v1, v2;
        v0.get(vertices);
        v1.get(vertices);
        v2.get(vertices);

        minPos = minVec3(minPos, minVec3(v0, minVec3(v1, v2)));
        maxPos = maxVec3(maxPos, maxVec3(v0, maxVec3(v1, v2)));

        // code taken from Tockdom Wiki (link below)
        // http://wiki.tockdom.com/wiki/KCL_(File_Format)#Section_3_-_Triangles

        Tri t;
        t.pos = v0;
        t.dir = Vector3f::unit(Vector3f::cross(v1 - v0, v2 - v0));
        t.normA = Vector3f::unit(Vector3f::cross(t.dir, v2 - v0));
        t.normB = Vector3f::unit(-Vector3f::cross(t.dir, v1 - v0));
        t.normC = Vector3f::unit(Vector3f::cross(t.dir, v1 - v2));
        t.len = Vector3f::dot(v1 - v0, t.normC);
        t.flag = kclFlags.getShort();

        tris.push_back(t);

        verts.push_back(t.pos);
        norms.push_back(t.dir);
        norms.push_back(t.normA);
        norms.push_back(t.normB);
        norms.push_back(t.normC);
    }

    KCL kcl;
    kcl.setBounds(minPos, maxPos);

    auto vMap = removeDuplicates(verts);
    auto nMap = removeDuplicates(norms);
    kcl.triangles = createTriangles(vMap, nMap, tris);
    kcl.vertices = toStdVector(vMap);
    kcl.normals = toStdVector(nMap);

    kcl.generateOctree();

    return kcl;
}

uint32_t toMask(float v)
{
    uint32_t inv = 0;
    while (inv < v)
    {
        inv = (inv << 1) + 1;
    }
    return ~inv;
}

void KCL::setBounds(Vector3f min, Vector3f max)
{
    minPos = min;

    max = max - min;
    maskX = toMask(max[0]);
    maskY = toMask(max[1]);
    maskZ = toMask(max[2]);
}

KCL::OctreeNode::OctreeNode(KCL* kcl, Vector3f pos, Vector3f size, bool root) :
    kcl{kcl},
    pos{pos},
    size{size},
    root{root},
    superNode{root},
    tris{},
    tIndices{}
{
    for (uint32_t i = 0; i < 8; ++i)
    {
        childs[i] = nullptr;
    }

    kcl->nodes.push_back(this);

    if (root)
    {
        split();
    }
}

// max number of triangle in octree node
constexpr uint32_t BLOCK_SIZE = 512;

// node block size expand
constexpr float BLOW_FACTOR = 400.f;

void KCL::OctreeNode::whichChilds(Vector3f pos, bool flags[8]) const
{
    bool axes[6] {false, false, false, false, false, false};
    for (uint32_t i = 0; i < 3; ++i)
    {
        if (pos[i] < this->pos[i] + (size[i] / 2.f) + BLOW_FACTOR)
        {
            axes[i << 1] = true;
        }
        if (pos[i] > this->pos[i] + (size[i] / 2.f) - BLOW_FACTOR)
        {
            axes[(i << 1) + 1] = true;
        }
    }
    
    flags[0] = axes[0] && axes[2] && axes[4]; // X0, Y0, Z0
    flags[1] = axes[0] && axes[2] && axes[5]; // X0, Y0, Z1
    flags[2] = axes[0] && axes[3] && axes[4]; // X0, Y1, Z0
    flags[3] = axes[0] && axes[3] && axes[5]; // X0, Y1, Z1
    flags[4] = axes[1] && axes[2] && axes[4]; // X1, Y0, Z0
    flags[5] = axes[1] && axes[2] && axes[5]; // X1, Y0, Z1
    flags[6] = axes[1] && axes[3] && axes[4]; // X1, Y1, Z0
    flags[7] = axes[1] && axes[3] && axes[5]; // X1, Y1, Z1
}

void KCL::OctreeNode::addTriangle(Triangle tri, uint16_t index)
{
    if (superNode)
    {
        Vector3f pos = kcl->vertices.at(tri.position) - kcl->getMinPos();
        bool flags[8];
        whichChilds(pos, flags);
        
        for (uint8_t i = 0; i < 8; ++i)
        {
            if (flags[i])
            {
                childs[i]->addTriangle(tri, index);
            }
        }
    }
    else
    {
        tris.push_back(tri);
        tIndices.push_back(index);

        if (tris.size() > BLOCK_SIZE)
        {
            split();
        }
    }
}

void KCL::OctreeNode::split()
{
    superNode = true;

    for (uint8_t i = 0; i < 8; ++i)
    {
        Vector3f ns = size * .5f;
        Vector3f np(ns[0] * (i >> 2), ns[1] * ((i >> 1) & 1), ns[2] * (i & 1));
        OctreeNode* node = new OctreeNode(kcl, np + pos, ns);
        childs[i] = node;
    }

    for (uint32_t i = 0; i < tris.size(); ++i)
    {
        addTriangle(tris[i], tIndices[i]); // logic is different if 'superNode' is set to true
    }

    tris.clear();
    tIndices.clear();
}

Vector3f KCL::calcRootNodeSize() const
{
    return
    {
        static_cast<float>(~maskX),
        static_cast<float>(~maskY),
        static_cast<float>(~maskZ)
    };
}

void KCL::generateOctree()
{
    OctreeNode* node = new OctreeNode(this, Vector3f(), calcRootNodeSize(), true);
    for (uint16_t i = 0; i < triangles.size(); ++i)
    {
        node->addTriangle(triangles[i], i);
    }
}
}
