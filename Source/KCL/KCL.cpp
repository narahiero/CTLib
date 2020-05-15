//////////////////////////////////////////////////
//  Copyright (c) 2020 Nara Hiero
//
// This file is licensed under GPLv3+
// Refer to the `License.txt` file included.
//////////////////////////////////////////////////

#include <CTLib/KCL.hpp>

namespace CTLib
{
    
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////
////   KCL class
////

KCL::KCL() :
    vertices{},
    normals{},
    triangles{}
{

}

KCL::KCL(const KCL& src) :
    vertices{src.vertices},
    normals{src.normals},
    triangles{src.triangles}
{

}

KCL::KCL(KCL&& src) :
    vertices{std::move(src.vertices)},
    normals{std::move(src.normals)},
    triangles{std::move(src.triangles)}
{

}

KCL::~KCL() = default;


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////
////   KCL error class
////

KCLError::KCLError(const char* msg) : std::runtime_error(msg)
{

}

KCLError::KCLError(const std::string& msg) : std::runtime_error(msg)
{

}
}
