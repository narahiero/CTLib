//////////////////////////////////////////////////
//  Copyright (c) 2020 Nara Hiero
//
// This file is licensed under GPLv3+
// Refer to the `License.txt` file included.
//////////////////////////////////////////////////

#pragma once


/*! @file Ext/WGCode.hpp
 *  
 *  @brief The header containing utilities for Wii Graphics Code.
 *  
 *  **Note**: This file is part of the BRRES module.
 */


#include <CTLib/BRRES.hpp>


namespace CTLib::Ext
{

/*! @addtogroup brres 
 *  @{
 */

/*! @defgroup brres-ext-wgcode Wii Graphics Code Extension
 *  
 *  @addtogroup brres-ext-wgcode
 * 
 *  @brief The Wii Graphics Code extension contains utilities to handle Wii
 *  Graphics Code.
 *  @{
 */

class WGCode final
{

public:

    enum Command : uint8_t
    {
        /*! @brief No operation. */
        CMD_NOOP = 0x00,

        /*! @brief Load BP (Blitting Processor). */
        CMD_LOAD_BP = 0x61
    };

    enum BPAddress : uint8_t
    {
        /*! @brief Indirect texturing stages texture references. */
        BP_IND_TEX_REF = 0x27,

        /*! @brief Shader stage sources. [`+ 4`] */
        BP_STAGE_SRC = 0x28,

        /*! @brief Shader stage combiner. [`+ 16`] */
        BP_STAGE_COMBINER = 0xC0,

        /*! @brief Material colour block. [`+ 8`] */
        BP_MATERIAL_COLOUR = 0xE0,

        /*! @brief Shader swap tables. [`+ 8`] */
        BP_SWAP_TABLE = 0xF6,

        /*! @brief Shader stage constant sources. [`+ 4`] */
        BP_STAGE_CONST_SRC = 0xF6,

        /*! @brief Write mask for next BP command. */
        BP_MASK = 0xFE
    };

    /*! @brief Reads the Wii Graphics Code from the specified buffer and outputs
     *  it to the output array.
     *  
     *  **Important**: The output array's size must be at least `0x100`.
     *  
     *  @param[in] gcode The buffer containing Wii Graphics Code
     *  @param[out] bp The array containing the BP memory state
     *  
     *  @throw CTLib::BRRESError If the graphics code is invalid or contains any
     *  non-BP (`0x61`) command.
     */
    static void readBP(Buffer& gcode, uint64_t* bp);
};

/*! @} addtogroup brres */
}
