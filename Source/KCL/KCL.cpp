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
    minPos{},
    vertices{},
    normals{},
    triangles{},
    maskX{0},
    maskY{0},
    maskZ{0}
{

}

KCL::KCL(const KCL& src) :
    minPos{src.minPos},
    vertices{src.vertices},
    normals{src.normals},
    triangles{src.triangles},
    maskX{src.maskX},
    maskY{src.maskY},
    maskZ{src.maskZ}
{

}

KCL::KCL(KCL&& src) :
    minPos{src.minPos},
    vertices{std::move(src.vertices)},
    normals{std::move(src.normals)},
    triangles{std::move(src.triangles)},
    maskX{src.maskX},
    maskY{src.maskY},
    maskZ{src.maskZ}
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
