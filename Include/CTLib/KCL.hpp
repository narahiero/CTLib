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

    /*! @brief Constructs a copy of the specified KCL instance. */
    KCL(const KCL&);

    /*! @brief Moves the specified KCL instance to a newly created one.
     *  
     *  The source KCL will then be empty, but may still be safely used.
     */
    KCL(KCL&&);

    ~KCL();

private:

    // structure of each entry in 'triangles'
    struct Triangle
    {

        // length of this triangle
        float length;

        // index in 'vertices'
        uint16_t position;

        // index in 'normals'
        uint16_t direction;

        // indices in 'normals'
        uint16_t normA, normB, normC;

        // kcl flag
        uint16_t flag;
    };

    // constructs an empty KCL
    KCL();

    // generate the spacial index of this kcl
    void generateSpacialIndex();

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