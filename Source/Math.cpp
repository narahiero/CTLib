//////////////////////////////////////////////////
//  Copyright (c) 2020 Nara Hiero
//
// This file is licensed under GPLv3+
// Refer to the `License.txt` file included.
//////////////////////////////////////////////////

#include <CTLib/Math.hpp>

#include <limits>

namespace CTLib
{

Vector2f::Vector2f() :
    Vectorf()
{

}

Vector2f::Vector2f(float x, float y) :
    Vectorf()
{
    array[0] = x;
    array[1] = y;
}

Vector2f::~Vector2f() = default;

////// Vector3f //////////////

Vector3f Vector3f::cross(const Vector3f& lhs, const Vector3f& rhs)
{
    return Vector3f(
        lhs[1] * rhs[2] - lhs[2] * rhs[1],
        lhs[2] * rhs[0] - lhs[0] * rhs[2],
        lhs[0] * rhs[1] - lhs[1] * rhs[0]
    );
}

Vector3f::Vector3f() :
    Vectorf()
{

}

Vector3f::Vector3f(const Vectorf<3>& src) :
    Vector3f(src[0], src[1], src[2])
{

}

Vector3f::Vector3f(float x, float y, float z) :
    Vectorf()
{
    array[0] = x;
    array[1] = y;
    array[2] = z;
}

Vector3f::~Vector3f() = default;

Vector3f Vector3f::cross(const Vector3f& rhs) const
{
    return cross(*this, rhs);
}

////// AABB //////////////////

AABB::AABB(const Vector3f& a, const Vector3f& b) :
    min{Vector3f{a[0] < b[0] ? a[0] : b[0], a[1] < b[1] ? a[1] : b[1], a[2] < b[2] ? a[2] : b[2]}},
    max{Vector3f{a[0] > b[0] ? a[0] : b[0], a[1] > b[1] ? a[1] : b[1], a[2] > b[2] ? a[2] : b[2]}}
{

}

Vector3f AABB::getMin() const
{
    return min;
}

Vector3f AABB::getMax() const
{
    return max;
}

Vector3f AABB::getSize() const
{
    return max - min;
}

////// Math //////////////////

#define FLOAT_MIN std::numeric_limits<float>::lowest()
#define FLOAT_MAX std::numeric_limits<float>::max()

#define MIN(a, b) (a < b ? a : b)
#define MAX(a, b) (a > b ? a : b)

bool Math::isInsideAABB(const AABB& aabb, const Vector3f& p)
{
    return p[0] >= aabb.min[0] && p[0] <= aabb.max[0]
        && p[1] >= aabb.min[1] && p[1] <= aabb.max[1]
        && p[2] >= aabb.min[2] && p[2] <= aabb.max[2];
}

void getPoints(const AABB& aabb, Vector3f points[8])
{
    Vector3f min = aabb.getMin(), max = aabb.getMax();
    points[0] = min;
    points[1] = {min[0], min[1], max[2]};
    points[2] = {min[0], max[1], min[2]};
    points[3] = {min[0], max[1], max[2]};
    points[4] = {max[0], min[1], min[2]};
    points[5] = {max[0], min[1], max[2]};
    points[6] = {max[0], max[1], min[2]};
    points[7] = max;
}

void project(Vector3f* points, uint32_t count, const Vector3f& axis, float& min, float& max)
{
    min = FLOAT_MAX; max = FLOAT_MIN;

    for (uint32_t i = 0; i < count; ++i)
    {
        float val = Vector3f::dot(points[i], axis);
        min = min < val ? min : val;
        max = max > val ? max : val;
    }
}

bool Math::isPartlyInsideAABB(
    const AABB& aabb, const Vector3f& t0, const Vector3f& t1, const Vector3f& t2
)
{
    if (isInsideAABB(aabb, t0) || isInsideAABB(aabb, t1) || isInsideAABB(aabb, t2))
    {
        return true; // at least one vertex is inside the AABB
    }

    for (uint32_t i = 0; i < 3; ++i)
    {
        if (MAX(t0[i], MAX(t1[i], t2[i])) < aabb.min[i] 
            || MIN(t0[i], MIN(t1[i], t2[i])) > aabb.max[i])
        {
            return false; // all vertices are outside the AABB planes
        }
    }

    // calculate triangle normal (assume counter-clockwise)
    Vector3f norm = Vector3f::unit(Vector3f::cross(t1 - t0, t2 - t0));

    // calculate AABB vertices
    Vector3f points[8];
    getPoints(aabb, points);

    float projTri = Vector3f::dot(norm, t0);
    float boxMin, boxMax;
    project(points, 8, norm, boxMin, boxMax);
    if (projTri < boxMin || projTri > boxMax)
    {
        return false; // triangle is outside of the AABB
    }

    Vector3f tris[] = {t0, t1, t2};
    Vector3f edges[] = {t0 - t1, t1 - t2, t2 - t0};
    Vector3f boxNormals[] = {{1.f, 0.f, 0.f}, {0.f, 1.f, 0.f}, {0.f, 0.f, 1.f}};
    float triMin, triMax;
    for (uint32_t edge = 0; edge < 3; ++edge)
    {
        for (uint32_t i = 0; i < 3; ++i)
        {
            Vector3f axis = Vector3f::unit(Vector3f::cross(edges[edge], boxNormals[i]));
            project(points, 8, axis, boxMin, boxMax);
            project(tris, 3, axis, triMin, triMax);
            if (triMax < boxMin || triMin > boxMax)
            {
                return false; // a separating axis was found
            }
        }
    }

    return true;
}
}
