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
    vertices{},
    normals{},
    triangles{},
    octree{new Octree(this)}
{

}

KCL::KCL(KCL&& src) :
    vertices{std::move(src.vertices)},
    normals{std::move(src.normals)},
    triangles{std::move(src.triangles)},
    octree{src.octree}
{
    octree->kcl = this;
    src.octree = new Octree(&src);
}

KCL::~KCL()
{
    delete octree;
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

KCL::Octree* KCL::getOctree() const
{
    return octree;
}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////
////   KCL Octree class

KCL::Octree::Octree(KCL* kcl) :
    kcl{kcl},
    minPos{},
    maskX{0xFFFFFFFF},
    maskY{0xFFFFFFFF},
    maskZ{0xFFFFFFFF},
    shift{0},
    shiftY{0},
    shiftZ{0},
    nodes{}
{

}

KCL::Octree::~Octree()
{
    for (OctreeNode* node : nodes)
    {
        delete node;
    }
}

Vector3f KCL::Octree::getMinPos() const
{
    return minPos;
}

uint32_t KCL::Octree::getMaskX() const
{
    return maskX;
}

uint32_t KCL::Octree::getMaskY() const
{
    return maskY;
}

uint32_t KCL::Octree::getMaskZ() const
{
    return maskZ;
}

Vector<uint32_t, 3> KCL::Octree::getMasks() const
{
    return {maskX, maskY, maskZ};
}

uint32_t KCL::Octree::getShift() const
{
    return shift;
}

uint32_t KCL::Octree::getShiftY() const
{
    return shiftY;
}

uint32_t KCL::Octree::getShiftZ() const
{
    return shiftZ;
}

Vector<uint32_t, 3> KCL::Octree::getSize() const
{
    return
    {
        (~maskX >> shift) + 1,
        (~maskY >> shift) + 1,
        (~maskZ >> shift) + 1
    };
}

uint8_t getHighestBit(uint32_t val)
{
    for (uint8_t idx = 31; idx >= 0; --idx)
    {
        if (val >> idx)
        {
            return idx;
        }
    }
    return 0;
}

Vector3f KCL::Octree::getBlockSize() const
{
    float size = static_cast<float>(1 << shift);
    return {size, size, size};
}

uint32_t KCL::Octree::getRootNodeCount() const
{
    Vector<uint32_t, 3> size = getSize();
    return size[0] * size[1] * size[2];
}

KCL::OctreeNode* KCL::Octree::getNode(uint32_t index) const
{
    assertValidIndex(index);
    return nodes.at(index);
}

KCL::OctreeNode* KCL::Octree::getNode(const Vector<uint32_t, 3>& index) const
{
    assertValidIndex(index);
    return nodes.at(index[0] | (index[1] << shiftY) | (index[2] << shiftZ));
}

std::vector<KCL::OctreeNode*> KCL::Octree::getAllNodes() const
{
    return nodes;
}

// calculates the kcl coord mask for the specified value
uint32_t toMask(float f)
{
    uint32_t m = 0;
    while (m < f)
    {
        m = (m << 1) | 1;
    }
    return ~m;
}

constexpr float BLOW_FACTOR = 400.f;
const Vector3f BLOW_V = {BLOW_FACTOR, BLOW_FACTOR, BLOW_FACTOR};

void KCL::Octree::setBounds(const Vector3f& min, const Vector3f& max)
{
    minPos = min - BLOW_V;

    Vector3f size = max - minPos + BLOW_V;
    maskX = toMask(size[0]);
    maskY = toMask(size[1]);
    maskZ = toMask(size[2]);
}

void KCL::Octree::calculateShifts()
{
    shift = 0xD;

    Vector<uint32_t, 3> size = getSize();
    shiftY = getHighestBit(size[0]);
    shiftZ = getHighestBit(size[1]) + shiftY;
}

void KCL::Octree::genRootNodes()
{
    Vector<uint32_t, 3> size = getSize();
    nodes.reserve(size[0] * size[1] * size[2]);
    for (uint32_t z = 0; z < size[2]; ++z)
    {
        for (uint32_t y = 0; y < size[1]; ++y)
        {
            for (uint32_t x = 0; x < size[0]; ++x)
            {
                nodes.push_back(new OctreeNode(this, {x, y, z}));
            }
        }
    }
}

void KCL::Octree::insert(uint16_t idx, const Vector3f& t0, const Vector3f& t1, const Vector3f& t2)
{
    Elem tri = {idx, t0 - minPos, t1 - minPos, t2 - minPos};

    for (uint32_t i = 0; i < getRootNodeCount(); ++i)
    {
        OctreeNode* node = nodes.at(i);
        if (Math::isPartlyInsideAABB(node->bounds, tri.t0, tri.t1, tri.t2))
        {
            node->insert(tri);
        }
    }
}

void KCL::Octree::assertValidIndex(uint32_t index) const
{
    if (index >= getRootNodeCount())
    {
        throw KCLError(Strings::format(
            "KCL: The specified index is out of range! (%d >= %d)",
            index, getRootNodeCount()
        ));
    }
}

void KCL::Octree::assertValidIndex(const Vector<uint32_t, 3>& index) const
{
    static const char* AXES = "X\0Y\0Z\0";

    Vector<uint32_t, 3> size = getSize();
    for (uint32_t i = 0; i < 3; ++i)
    {
        if (index[i] >= size[i])
        {
            throw KCLError(Strings::format(
                "KCL: Index for %s-axis is out of range! (%d >= %d)",
                AXES + (i << 1), index[i], size[i]
            ));
        }
    }
}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////
////   KCL OctreeNode class
////

KCL::OctreeNode::OctreeNode(Octree* octree, const Vector<uint32_t, 3>& index) :
    octree{octree},
    bounds{calcAABBRoot(index)},
    superNode{false},
    elems{}
{
    for (uint32_t i = 0; i < 8; ++i)
    {
        childs[i] = nullptr;
    }
}

KCL::OctreeNode::OctreeNode(Octree* octree, OctreeNode* node, const Vector<uint32_t, 3>& index) :
    octree{octree},
    bounds{calcAABBChild(node, index)},
    superNode{false},
    elems{}
{
    for (uint32_t i = 0; i < 8; ++i)
    {
        childs[i] = nullptr;
    }
}

KCL::OctreeNode::~OctreeNode() = default;

AABB KCL::OctreeNode::getBounds() const
{
    return bounds;
}

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

KCL::OctreeNode* KCL::OctreeNode::getChild(const Vector<uint32_t, 3>& index) const
{
    assertSuperNode();
    assertValidChildIndex(index);
    return childs[index[0] | (index[1] << 1) | (index[2] << 2)];
}

AABB KCL::OctreeNode::calcAABBRoot(const Vector<uint32_t, 3>& index)
{
    Vector3f bSize = octree->getBlockSize();
    Vector3f pos = {bSize[0] * index[0], bSize[1] * index[1], bSize[2] * index[2]};
    return {pos - BLOW_V, pos + bSize + BLOW_V};
}

AABB KCL::OctreeNode::calcAABBChild(OctreeNode* node, const Vector<uint32_t, 3>& index)
{
    Vector3f bPos = node->bounds.getMin() + BLOW_V;
    Vector3f size = (node->bounds.getSize() - (BLOW_V * 2.f)) * .5f;
    Vector3f pos = {
        bPos[0] + (size[0] * index[0]),
        bPos[1] + (size[1] * index[1]),
        bPos[2] + (size[2] * index[2])
    };
    return {pos - BLOW_V, pos + size + BLOW_V};
}

// maximum number of triangles per node
constexpr uint32_t MAX_NODE_CAPACITY = 32;

void KCL::OctreeNode::insert(const Octree::Elem& tri)
{
    if (superNode)
    {
        for (uint32_t i = 0; i < 8; ++i)
        {
            if (Math::isPartlyInsideAABB(childs[i]->bounds, tri.t0, tri.t1, tri.t2))
            {
                childs[i]->insert(tri);
            }
        }
    }
    else
    {
        elems.push_back(tri);
        if (elems.size() > MAX_NODE_CAPACITY)
        {
            split();
        }
    }
}

void KCL::OctreeNode::split()
{
    for (uint32_t i = 0; i < 8; ++i)
    {
        Vector<uint32_t, 3> index = {i & 1, (i >> 1) & 1, i >> 2};
        OctreeNode* node = new OctreeNode(octree, this, index);
        octree->nodes.push_back(node);
        childs[i] = node;
    }

    superNode = true;

    for (const Octree::Elem& tri : elems)
    {
        insert(tri); // insert logic different when 'superNode' is true
    }

    elems.clear();
}

void KCL::OctreeNode::assertSuperNode() const
{
    if (!superNode)
    {
        throw KCLError("KCL: The OctreeNode is not a super node!");
    }
}

void KCL::OctreeNode::assertNotSuperNode() const
{
    if (superNode)
    {
        throw KCLError("KCL: The OctreeNode is a super node!");
    }
}

void KCL::OctreeNode::assertValidChildIndex(uint8_t index) const
{
    if (index >= 8)
    {
        throw KCLError(Strings::format(
            "KC: The specified child index is out of range! (%d >= 8)",
            index
        ));
    }
}

void KCL::OctreeNode::assertValidChildIndex(const Vector<uint32_t, 3>& index) const
{
    static const char* AXES = "X\0Y\0Z\0";

    for (uint32_t i = 0; i < 3; ++i)
    {
        if (index[i] >= 1)
        {
            throw KCLError(Strings::format(
                "KCL: Index for %s-axis is out of range! (%d >= 1)",
                AXES + (i << 2), index[i]
            ));
        }
    }
}

std::vector<uint16_t> KCL::OctreeNode::getIndices() const
{
    assertNotSuperNode();

    std::vector<uint16_t> indices;
    indices.reserve(elems.size());
    for (const auto& tri : elems)
    {
        indices.push_back(tri.idx);
    }
    return indices;
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
