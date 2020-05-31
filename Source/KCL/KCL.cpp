//////////////////////////////////////////////////
//  Copyright (c) 2020 Nara Hiero
//
// This file is licensed under GPLv3+
// Refer to the `License.txt` file included.
//////////////////////////////////////////////////

#include <CTLib/KCL.hpp>

#include <CTLib/Utilities.hpp>

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

Vector3f KCL::getMinPos() const
{
    return minPos;
}

uint32_t KCL::getMaskX() const
{
    return maskX;
}

uint32_t KCL::getMaskY() const
{
    return maskY;
}

uint32_t KCL::getMaskZ() const
{
    return maskZ;
}

std::vector<Vector3f> KCL::getVertices() const
{
    return vertices;
}

std::vector<Vector3f> KCL::getNormals() const
{
    return normals;
}

std::vector<KCL::Triangle> KCL::getTriangles() const
{
    return triangles;
}

KCL::OctreeNode* KCL::getRootNode() const
{
    return nodes.empty() ? nullptr : nodes[0];
}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////
////   KCL OctreeNode class
////

bool KCL::OctreeNode::isSuperNode() const
{
    return superNode;
}

KCL::OctreeNode* KCL::OctreeNode::getChild(uint8_t index) const
{
    assertSuperNode();
    assertValidChildIndex(index);
    return childs[index];
}

std::vector<KCL::Triangle> KCL::OctreeNode::getTriangles() const
{
    assertNotSuperNode();
    return tris;
}

std::vector<uint16_t> KCL::OctreeNode::getIndices() const
{
    assertNotSuperNode();
    return tIndices;
}

void KCL::OctreeNode::assertSuperNode() const
{
    if (!superNode)
    {
        throw KCLError("The OctreeNode is not a super node!");
    }
}

void KCL::OctreeNode::assertNotSuperNode() const
{
    if (superNode)
    {
        throw KCLError("The OctreeNode is a super node!");
    }
}

void KCL::OctreeNode::assertValidChildIndex(uint8_t index) const
{
    if (index >= 8)
    {
        throw KCLError(Strings::format(
            "The specified child index is out of range! (%d >= 8)",
            index
        ));
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
