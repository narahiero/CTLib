//////////////////////////////////////////////////
//  Copyright (c) 2020 Nara Hiero
//
// This file is licensed under GPLv3+
// Refer to the `License.txt` file included.
//////////////////////////////////////////////////

#include <CTLib/Math.hpp>

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

Vector3f::Vector3f() :
    Vectorf()
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
}
