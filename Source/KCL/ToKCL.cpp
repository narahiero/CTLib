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

void assertRemaining(Buffer& vertices, Buffer& flags, int32_t count);

struct Tri
{
    Vector3f t0, t1, t2;
};

bool less(const Vector3f& lhs, const Vector3f& rhs);
using Less = decltype(&less);

Vector3f min(const Vector3f& a, const Vector3f& b);
Vector3f max(const Vector3f& a, const Vector3f& b);

uint16_t insert(std::map<Vector3f, uint16_t, Less>& m, const Vector3f& v)
{
    return (*(m.insert({v, static_cast<uint16_t>(m.size())}).first)).second;
}

bool ignoreTriangle(const Tri& t);

std::vector<Vector3f> toStdVector(const std::map<Vector3f, uint16_t, Less>& m);

KCL KCL::fromModel(Buffer& vertices, Buffer& flags, int32_t count)
{
    count = count < 0 ? static_cast<int32_t>(vertices.remaining() / 36) : count;
    assertRemaining(vertices, flags, count);

    if (count == 0) // empty KCL
    {
        return KCL();
    }

    constexpr float MIN = std::numeric_limits<float>::lowest();
    constexpr float MAX = std::numeric_limits<float>::max();
    Vector3f minPos{MAX, MAX, MAX}, maxPos{MIN, MIN, MIN};

    std::map<Vector3f, uint16_t, Less> vMap(&less);
    std::map<Vector3f, uint16_t, Less> nMap(&less);
    std::vector<Triangle> triangles;
    std::vector<Tri> tris;
    for (int32_t i = 0; i < count; ++i)
    {
        Vector3f t0, t1, t2;
        t0.get(vertices);
        t1.get(vertices);
        t2.get(vertices);

        Tri t = {t0, t1, t2};
        if (ignoreTriangle(t))
        {
            continue;
        }

        minPos = min(minPos, min(t0, min(t1, t2)));
        maxPos = max(maxPos, max(t0, max(t1, t2)));

        Vector3f dir = Vector3f::unit(Vector3f::cross(t1 - t0, t2 - t0));
        Vector3f normA = Vector3f::unit(Vector3f::cross(dir, t2 - t0));
        Vector3f normB = Vector3f::unit(-Vector3f::cross(dir, t1 - t0));
        Vector3f normC = Vector3f::unit(Vector3f::cross(dir, t1 - t2));
        float len = Vector3f::dot(t1 - t0, normC);

        Triangle tri;
        tri.length = len;
        tri.position = insert(vMap, t0);
        tri.direction = insert(nMap, dir);
        tri.normA = insert(nMap, normA);
        tri.normB = insert(nMap, normB);
        tri.normC = insert(nMap, normC);
        tri.flag = flags.getShort();

        triangles.push_back(tri);
        tris.push_back(t);
    }

    KCL kcl;
    kcl.vertices = toStdVector(vMap);
    kcl.normals = toStdVector(nMap);
    kcl.triangles = triangles;

    kcl.octree->setBounds(minPos, maxPos);
    kcl.octree->calculateShifts();
    kcl.octree->genRootNodes();

    for (uint16_t i = 0; i < tris.size(); ++i)
    {
        Tri t = tris[i];
        kcl.octree->insert(i, t.t0, t.t1, t.t2);
    }

    return kcl;
}

void assertRemaining(Buffer& vertices, Buffer& flags, int32_t count)
{
    if (vertices.remaining() < count * 36)
    {
        throw KCLError(Strings::format(
            "KCL: Not enough bytes in vertices buffer! (%d < %d)",
            vertices.remaining(), count * 36
        ));
    }
    if (flags.remaining() < count * 2)
    {
        throw KCLError(Strings::format(
            "KCL: Not enough bytes in flags buffer! (%d < %d)",
            flags.remaining(), count * 2
        ));
    }
}

bool less(const Vector3f& lhs, const Vector3f& rhs)
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

Vector3f min(const Vector3f& a, const Vector3f& b)
{
    return
    {
        a[0] < b[0] ? a[0] : b[0],
        a[1] < b[1] ? a[1] : b[1],
        a[2] < b[2] ? a[2] : b[2]
    };
}

Vector3f max(const Vector3f& a, const Vector3f& b)
{
    return
    {
        a[0] > b[0] ? a[0] : b[0],
        a[1] > b[1] ? a[1] : b[1],
        a[2] > b[2] ? a[2] : b[2]
    };
}

bool ignoreTriangle(const Tri& t)
{
    if (t.t0 == t.t1 || t.t0 == t.t2 || t.t1 == t.t2)
    {
        return true;
    }

    Vector3f u1 = Vector3f::unit(t.t1 - t.t0), u2 = Vector3f::unit(t.t2 - t.t0);
    if (u1 == u2 || -u1 == u2)
    {
        return true;
    }

    return false;
}

std::vector<Vector3f> toStdVector(const std::map<Vector3f, uint16_t, Less>& m)
{
    std::vector<Vector3f> v;
    v.resize(m.size());
    for (const auto& p : m)
    {
        v[p.second] = p.first;
    }
    return v;
}
}
