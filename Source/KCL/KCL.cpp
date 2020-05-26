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
    maskX{0xFFFFFFFF},
    maskY{0xFFFFFFFF},
    maskZ{0xFFFFFFFF},
    nodes{}
{

}

KCL::KCL(KCL&& src) :
    minPos{src.minPos},
    vertices{std::move(src.vertices)},
    normals{std::move(src.normals)},
    triangles{std::move(src.triangles)},
    maskX{src.maskX},
    maskY{src.maskY},
    maskZ{src.maskZ},
    nodes{std::move(src.nodes)}
{
    for (OctreeNode* node : nodes)
    {
        node->kcl = this;
    }
}

KCL::~KCL()
{
    for (OctreeNode* node : nodes)
    {
        delete node;
    }
}


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
