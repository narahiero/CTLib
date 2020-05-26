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

    /*! @brief A node in a KCL octree. */
    class OctreeNode final
    {

        friend class KCL;

    public:

        /*! @brief Adds the specified triangle index to this node. */
        void addTriangle(Triangle tri, uint16_t index);

    private:

        OctreeNode(KCL* kcl, Vector3f pos, Vector3f size, bool root = false);

        // splits this node in 8 child nodes
        void split();

        void whichChilds(Vector3f pos, bool flags[8]) const;

        // pointer to kcl owning this node
        KCL* kcl;

        // position of cube
        Vector3f pos;

        // size of cube
        Vector3f size;

        // whether this node is the root node of a kcl
        bool root;

        // whether this node points to other nodes
        bool superNode;

        // pointer to OctreeNode childs; unused if 'superNode' is false
        OctreeNode* childs[8];

        // triangles in this OctreeNode
        std::vector<Triangle> tris;

        // indices of triangles in this OctreeNode
        std::vector<uint16_t> tIndices;
    };

    /*! @brief Creates a KCL from the specified raw model data.
     *  
     *  The data buffers must be formatted as follows:
     *  
     *  `vertices`: 3 vertex per face, 3 floats per vertex
     *  `kclFlags`: 1 KCL flag per face, 1 uint16 per flag
     * 
     *  If the last (4th) parameter is `-1`, the number of triangles will be
     *  calculated _from the `vertices` vector_. Else the number of triangles
     *  is that value.
     * 
     *  The normals are calculated by this function.
     *  
     *  @param[in] vertices The vertex data
     *  @param[in] kclFlags The KCL flags
     *  @param[in] count The triangle count
     */
    static KCL fromRawModel(Buffer& vertices, Buffer& kclFlags, int32_t count = -1);

    /*! @brief Delete copy constructor for move-only class. */
    KCL(const KCL&) = delete;

    /*! @brief Moves the specified KCL instance to a newly created one.
     *  
     *  The source KCL will then be empty, but may still be safely used.
     */
    KCL(KCL&&);

    ~KCL();

private:

    // constructs an empty KCL
    KCL();

    // sets minPos and generates masks
    void setBounds(Vector3f min, Vector3f max);

    // returns the size of the root node based on the mask values
    Vector3f calcRootNodeSize() const;

    // generate the octree of this kcl
    void generateOctree();

    // first coordinate
    Vector3f minPos;

    // coord masks
    uint32_t maskX, maskY, maskZ;

    // vector containing vertices in this kcl
    std::vector<Vector3f> vertices;

    // vector containing normals in this kcl
    std::vector<Vector3f> normals;

    // vector containing triangles in this kcl
    std::vector<Triangle> triangles;

    // vector containing all OctreeNode pointers is this kcl
    std::vector<OctreeNode*> nodes;
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
