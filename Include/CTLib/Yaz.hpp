//////////////////////////////////////////////////
//  Copyright (c) 2020 Nara Hiero
//
// This file is licensed under GPLv3+
// Refer to the `License.txt` file included.
//////////////////////////////////////////////////

#pragma once

/*! @file Yaz.hpp
 * 
 *  @brief The header for the Yaz module.
 */


#include <cstdint>
#include <stdexcept>

#include <CTLib/Memory.hpp>


namespace CTLib
{

/*! @defgroup yaz Yaz
 * 
 *  @addtogroup yaz
 * 
 *  @brief The Yaz module contains all classes and methods that can be used to
 *  interact with Nintendo's Yaz compression format.
 *  @{
 */

/*! @brief Enumeration of Yaz formats. */
enum class YazFormat
{
    /*! @brief Yaz0 format. */
    Yaz0,
    
    /*! @brief Yaz1 format. */
    Yaz1
};

/*! @brief The Yaz class contains methods to compress and decompress data. */
class Yaz
{

public:
    
    /*! @brief Compresses the passed data using the specified Yaz format.
     *
     *  @param[in] data The data to be compressed
     *  @param[in] format The compression format
     * 
     *  @return The compressed data
     */
    static Buffer compress(Buffer& data, YazFormat format);

    /*! @brief Decompresses the passed data.
     *  
     *  @param[in] data The data to be decompressed
     * 
     *  @throw CTLib::YazError If `data` is invalid or corrupted.
     * 
     *  @return The decompressed data
     */
    static Buffer decompress(Buffer& data);

    /*! @brief Decompresses the passed data, forcing the format to the
     *  specified one. 
     * 
     *  For more details, see CTLib::Yaz::decompress(CTLib::Buffer&).
     * 
     *  @param[in] data The data to be decompressed
     *  @param[in] format The required Yaz format
     * 
     *  @throw CTLib::YazError If `data` is invalid or corrupted.
     * 
     *  @return The decompressed data
     */
    static Buffer decompress(Buffer& data, YazFormat format);
};

/*! @brief YazError is the error class used by the methods in this header. */
class YazError final : public std::runtime_error
{

public:

    /*! @brief Constructs a CTLib::YazError with the specified message. */
    YazError(const char* msg);

    /*! @brief Constructs a CTLib::YazError with the specified message. */
    YazError(const std::string& msg);
};

/*! @} addtogroup yaz */
}
