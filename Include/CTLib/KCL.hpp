//////////////////////////////////////////////////
//  Copyright (c) 2020 Nara Hiero
//
// This file is licensed under GPLv3+
// Refer to the `License.txt` file included.
//////////////////////////////////////////////////

#pragma once

/*! @file KCL.hpp
 * 
 *  @brief The header for KCL module.
 */


#include <vector>

#include <CTLib/Math.hpp>
#include <CTLib/Memory.hpp>


namespace CTLib
{

/*! @defgroup kcl KCL
 * 
 *  @addtogroup kcl
 * 
 *  @brief The KCL module contains all classes and methods that can be used to
 *  interact with Nintendo's KCL data format.
 *  @{
 */

/*! @brief Object representation of Nintendo's KCL data format. */
class KCL final
{

public:

    /*! @brief Reads a KCL from the specified buffer.
     *  
     *  @throw CTLib::KCLError If the specified data is invalid.
     */
    static KCL read(Buffer& data);

    /*! @brief Writes the specified KCL to a newly created Buffer. */
    static Buffer write(const KCL& kcl);

    /*! @brief The structure of a triangle in a KCL. */
    struct Triangle
    {

        /*! @brief The length of this triangle. */
        float length;

        /*! @brief The index in vertices array. */
        uint16_t position;

        /*! @brief An index in normals array. */
        uint16_t direction;

        /*! @brief Indices in normals array. */
        uint16_t normA, normB, normC;

        /*! @brief The KCL flag of this triangle. */
        uint16_t flag;
    };

    class OctreeNode;

    /*! @brief Class representing the octree in a KCL. */
    class Octree final
    {

        friend class KCL;

    public:

        ~Octree();

        /*! @brief Returns the minimum value for each axis of this Octree. */
        Vector3f getMinPos() const;

        /*! @brief Returns the mask value for the X-axis of this Octree. */
        uint32_t getMaskX() const;

        /*! @brief Returns the mask value for the Y-axis of this Octree. */
        uint32_t getMaskY() const;

        /*! @brief Returns the mask value for the Z-axis of this Octree. */
        uint32_t getMaskZ() const;

        /*! @brief Returns the mask value for each axis of this Octree. */
        Vector<uint32_t, 3> getMasks() const;

        /*! @brief Returns the coordinate shift value of this Octree. */
        uint32_t getShift() const;

        /*! @brief Returns the Y-axis shift value of this Octree. */
        uint32_t getShiftY() const;

        /*! @brief Returns the Z-axis shift value of this Octree. */
        uint32_t getShiftZ() const;

        /*! @brief Returns the number of blocks in each axis of this Octree.
         *  
         *  **Note**: The counts are always a power of 2.
         */
        Vector<uint32_t, 3> getSize() const;

        /*! @brief Returns the size of blocks in this Octree. */
        Vector3f getBlockSize() const;

        /*! @brief Returns the number of root nodes in this Octree. */
        uint32_t getRootNodeCount() const;

        /*! @brief Returns the root node at the specified index.
         *  
         *  @param[in] index The node index
         *  
         *  @throw CTLib::KCLError If the specified index is more than or
         *  equal to the root node count of this KCL.
         */
        OctreeNode* getNode(uint32_t index) const;

        /*! @brief Returns the root node at the specified 3D index.
         *  
         *  **Note**: `Y` is shifted left by `shiftY` and `Z` is shifted left
         *  by `shiftZ`.
         *  
         *  @param[in] index The 3D index
         *  
         *  @throw CTLib::KCLError If a component of the specified 3D index is
         *  more than or equal to its corresponding component of the size of
         *  this KCL.
         */
        OctreeNode* getNode(const Vector<uint32_t, 3>& index) const;

        /*! @brief Returns a std::vector containing all nodes in this Octree,
         *  including all non-root nodes.
         *  
         *  **Note**: The nodes are not ordered in any particular way.
         */
        std::vector<OctreeNode*> getAllNodes() const;

    private:

        struct Elem
        {
            uint16_t idx;
            Vector3f t0, t1, t2;
        };

        Octree(KCL* kcl);

        // sets 'minPos' and 'mask*'
        void setBounds(const Vector3f& min, const Vector3f& max);

        // sets 'shift' and 'shift*'
        void calculateShifts();

        // generate nodes
        void genRootNodes();

        // inserts the specified triangle in this octree
        void insert(uint16_t idx, const Vector3f& t0, const Vector3f& t1, const Vector3f& t2);

        // converts the KCL::Triangle at the specified index to an Elem instance
        Elem toElem(uint16_t triIdx) const;

        // throws if 'index' >= 'getRootNodeCount()'
        void assertValidIndex(uint32_t index) const;

        // throws if 'index[N]' >= 'getSize()[N]' with N in range(0, 3)
        void assertValidIndex(const Vector<uint32_t, 3>& index) const;

        // pointer to kcl owing this node
        KCL* kcl;

        // first coordinate
        Vector3f minPos;

        // coord masks
        uint32_t maskX, maskY, maskZ;

        // coord right shift
        uint32_t shift;

        // coord left shifts
        uint32_t shiftY, shiftZ;

        // vector containing all nodes in this octree
        std::vector<OctreeNode*> nodes;
    };

    /*! @brief A node in a KCL octree. */
    class OctreeNode final
    {

        friend class KCL;

    public:

        ~OctreeNode();

        /*! @brief Returns the bounds of this node. The returned AABB is
         *  relative to the Octree's minimum position.
         */
        AABB getBounds() const;

        /*! @brief Returns whether this node points to other nodes. */
        bool isSuperNode() const;

        /*! @brief Returns the child node at the specified index.
         *  
         *  @param[in] index The child index (0-7)
         *  
         *  @throw CTLib::KCLError If this node is not a super node or the
         *  specified index is more than or equal to 8.
         */
        OctreeNode* getChild(uint8_t index) const;

        /*! @brief Returns the child node at the specified 3D index.
         *  
         *  **Note**: `Y` is shifted left by 1 and `Z` is shifted left by 2.
         *  
         *  @param[in] index The 3D index
         *  
         *  @throw CTLib::KCLError If this node is not a super node or the
         *  specified 3D index has one or more of its components more than or
         *  equal to 1.
         */
        OctreeNode* getChild(const Vector<uint32_t, 3>& index) const;

        /*! @brief Returns the triangle indices in this node.
         *  
         *  @throw CTLib::KCLError If this node is a super node.
         */
        std::vector<uint16_t> getIndices() const;

    private:

        // root constructor (used by Octree)
        OctreeNode(Octree* octree, const Vector<uint32_t, 3>& index);

        // child constructor (used by split())
        OctreeNode(Octree* octree, OctreeNode* node, const Vector<uint32_t, 3>& index);

        // calculates the AABB of this node from the specified index in the octree
        AABB calcAABBRoot(const Vector<uint32_t, 3>& index);

        // calculates the AABB of this node from the specified index in specified parent node
        AABB calcAABBChild(OctreeNode* node, const Vector<uint32_t, 3>& index);

        // inserts the specified triangle element in this node
        void insert(const Octree::Elem& tri);

        // mark this node as 'superNode' and create 8 child nodes
        void split();

        // throws if 'superNode' == false
        void assertSuperNode() const;

        // throws if 'superNode' == true
        void assertNotSuperNode() const;

        // throws if 'index' >= 8
        void assertValidChildIndex(uint8_t index) const;

        // throw if 'index[N]' >= 1 with N in range(0, 3)
        void assertValidChildIndex(const Vector<uint32_t, 3>& index) const;

        // pointer to octree owning this node
        Octree* octree;

        // bounding box of this node
        AABB bounds;

        // whether this node points to other nodes
        bool superNode;

        // pointer to OctreeNode childs; unused if 'superNode' is false
        OctreeNode* childs[8];

        // triangles in this node; unused if 'superNode'
        std::vector<Octree::Elem> elems;
    };

    /*! @brief KCL creation settings. */
    struct Settings final
    {

        /*! @brief The blow factor controls by how many game units the bounding
         *  box of an octree node should be extended in each direction.
         *  
         *  The default value `400.0` is perfect for most cases, but one might
         *  need to increase it to around `600.0` for speed factors >= `1.5`.
         */
        float blowFactor = 400.f;

        /*! @brief The maximum number of triangles per octree node.
         *  
         *  If an octree node exceeds this limit, it will be 'split' into 8
         *  smaller nodes.
         */
        uint32_t maxTriangles = 32;
    };

    /*! @brief Sets the KCL creation settings. */
    static void setSettings(const Settings& settings);

    /*! @brief Returns the KCL creation settings. */
    static Settings getSettings();

    /*! @brief Creates a KCL from the specified model data.
     *  
     *  The data buffers must be formatted as follows:
     *  
     *  `vertices`: 3 vertex per face, 3 floats per vertex
     *  `flags`: 1 KCL flag per face, 1 uint16 per flag
     *  
     *  If the last parameter is negative, the number of triangles will be
     *  calculated _from the `vertices` buffer_. Else the number of triangles
     *  is that value.
     *  
     *  The normals are calculated by this function.
     *  
     *  @param[in] vertices The vertex data
     *  @param[in] flags The KCL flags
     *  @param[in] count The triangle count
     *  
     *  @throw CTLib::KCLError If there is not enough data remaining in the
     *  specified data buffers.
     */
    static KCL fromModel(Buffer& vertices, Buffer& flags, int32_t count = -1);

    /*! @brief Delete copy constructor for move-only class. */
    KCL(const KCL&) = delete;

    /*! @brief Moves the specified KCL instance to a newly created one.
     *  
     *  The source KCL will then be empty, but may still be safely used.
     */
    KCL(KCL&&);

    ~KCL();

    /*! @brief Returns the vertices in this KCL. */
    std::vector<Vector3f> getVertices() const;

    /*! @brief Returns the normals in this KCL. */
    std::vector<Vector3f> getNormals() const;

    /*! @brief Returns the triangles in this KCL. */
    std::vector<Triangle> getTriangles() const;

    /*! @brief Returns the Octree of this KCL. */
    Octree* getOctree() const;

private:

    // methods part of KCL class to access private members of Octree
    static void readOctree(Buffer& data, Octree* octree);
    static void readOctreeNode(Buffer& data, OctreeNode* octree, uint32_t pos);

    // constructs an empty KCL
    KCL();

    // throws if 'index' >= 'triangles.size()'
    void assertValidTriangleIndex(uint16_t index) const;

    // throws if one or more data index is out of range
    void assertValidTriangle(const Triangle& tri) const;

    // kcl creation settings
    static Settings settings;

    // vector containing vertices in this kcl
    std::vector<Vector3f> vertices;

    // vector containing normals in this kcl
    std::vector<Vector3f> normals;

    // vector containing triangles in this kcl
    std::vector<Triangle> triangles;

    // the octree of this kcl
    Octree* octree;
};

/*! @brief KCLError is the error class used by the methods in this header. */
class KCLError final : public std::runtime_error
{

public:

    /*! @brief Constructs a CTLib::KCLError with the specified error message. */
    KCLError(const char* msg);

    /*! @brief Constructs a CTLib::KCLError with the specified error message. */
    KCLError(const std::string& msg);
};

/*! @} addtogroup kcl */
}
