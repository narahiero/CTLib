//////////////////////////////////////////////////
//  Copyright (c) 2020 Nara Hiero
//
// This file is licensed under GPLv3+
// Refer to the `License.txt` file included.
//////////////////////////////////////////////////

#include <CTLib/KCL.hpp>

#include <limits>
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

uint32_t toMask(float v)
{
    uint32_t inv = 0;
    while (inv < v)
    {
        inv = (inv << 1) + 1;
    }
    return ~inv;
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

    KCL kcl;

    constexpr float max = std::numeric_limits<float>::max();
    kcl.minPos = {max, max, max};

    constexpr float min = std::numeric_limits<float>::lowest();
    Vector3f maxPos = {min, min, min};

    std::vector<Tri> tris;
    for (int32_t i = 0; i < count; ++i)
    {
        Vector3f v0, v1, v2;
        v0.get(vertices);
        v1.get(vertices);
        v2.get(vertices);

        kcl.minPos = minVec3(kcl.minPos, minVec3(v0, minVec3(v1, v2)));
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
    }

    Vector3f diff = maxPos - kcl.minPos;
    kcl.maskX = toMask(diff[0]);
    kcl.maskY = toMask(diff[1]);
    kcl.maskZ = toMask(diff[2]);

    auto cmp = [](const Vector3f& lhs, const Vector3f& rhs)
    {
        float lenL = Vector3f::lengthSquared(lhs), lenR = Vector3f::lengthSquared(rhs);
        if (lenL < lenR)
        {
            return true;
        }
        else if (lenL == lenR)
        {
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
        }
        return false;
    };
    std::set<Vector3f, decltype(cmp)> vSet(cmp);
    std::set<Vector3f, decltype(cmp)> nSet(cmp);

    for (Tri& t : tris)
    {
        auto posIdx = vSet.insert(t.pos);
        auto dirIdx = nSet.insert(t.dir);
        auto normAIdx = nSet.insert(t.normA);
        auto normBIdx = nSet.insert(t.normB);
        auto normCIdx = nSet.insert(t.normC);

        if (posIdx.second)
        {
            kcl.vertices.push_back(t.pos);
        }
        if (dirIdx.second)
        {
            kcl.normals.push_back(t.dir);
        }
        if (normAIdx.second)
        {
            kcl.normals.push_back(t.normA);
        }
        if (normBIdx.second)
        {
            kcl.normals.push_back(t.normB);
        }
        if (normCIdx.second)
        {
            kcl.normals.push_back(t.normC);
        }

        auto vBeg = vSet.begin();
        auto nBeg = nSet.begin();

        kcl.triangles.push_back({
            t.len,
            static_cast<uint16_t>(std::distance(vBeg, posIdx.first)),
            static_cast<uint16_t>(std::distance(nBeg, dirIdx.first)),
            static_cast<uint16_t>(std::distance(nBeg, normAIdx.first)),
            static_cast<uint16_t>(std::distance(nBeg, normBIdx.first)),
            static_cast<uint16_t>(std::distance(nBeg, normCIdx.first)),
            t.flag
        });
    }
}
}
