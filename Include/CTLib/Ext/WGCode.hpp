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

/*! @brief Utility class used to read Wii Graphics Code. */
class WGCode final
{

public:

    /*! @brief The number of XF registers. */
    constexpr static uint32_t XF_REG_COUNT = 0x1058;

    /*! @brief The number of BP registers. */
    constexpr static uint32_t BP_REG_COUNT = 0x100;

    /*! @brief Enumeration of all known graphics code commands. */
    enum Command : uint8_t
    {
        /*! @brief No operation. */
        CMD_NOOP = 0x00,

        /*! @brief Load XF (Transform Unit). */
        CMD_LOAD_XF = 0x10,

        /*! @brief Load BP (Blitting Processor). */
        CMD_LOAD_BP = 0x61
    };

    /*! @brief Enumeration of all known BP addresses. */
    enum BPAddress : uint8_t
    {
        /*! @brief Indirect texturing stages texture references. */
        BP_IND_TEX_REF = 0x27,

        /*! @brief Shader stage sources. [`+ 4`] */
        BP_STAGE_SRC = 0x28,

        /*! @brief Material constant alpha. */
        BP_CONST_ALPHA = 0x42,

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

    /*! @brief Graphics code context. */
    struct Context
    {

        /*! @brief Transform unit registers. */
        uint32_t xf[XF_REG_COUNT];

        /*! @brief Blitting processor registers. */
        uint64_t bp[BP_REG_COUNT];
    };

    /*! @brief Graphics code read flags. */
    enum Flags : uint32_t
    {
        // flags

        /*! @brief Allow XF commands in graphics code. */
        FLAG_USE_XF = 0x00000100,

        /*! @brief Allow BP commands in graphics code. */
        FLAG_USE_BP = 0x00000200,


        // presets

        /*! @brief Allow all known commands in graphics code. */
        FLAG_USE_ALL = FLAG_USE_XF | FLAG_USE_BP,

        /*! @brief Default flags value. */
        FLAG_DEFAULT = FLAG_USE_ALL
    };

    /*! @brief Resets the specified context to its default values. */
    static void initContext(Context* context);

    /*! @brief Reads the Wii Graphics Code from the specified buffer and updates
     *  the specified context.
     *  
     *  If `reset` is `true`, initContext() will be invoked on the context
     *  before reading the graphics code.
     *  
     *  @param[in] gcode The buffer containing Wii Graphics Code
     *  @param[in] context The graphics code context
     *  @param[in] reset Whether to reset context before reading
     *  @param[in] flags Controls how the graphics code is read
     *  
     *  @throw CTLib::BRRESError If the graphics code is invalid, or one or more
     *  of the conditions set by the specified flags are not met.
     */
    static void readGraphicsCode(Buffer& gcode, Context* context, bool reset, uint32_t flags = FLAG_DEFAULT);

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
     *  
     *  @deprecated Use readGraphicsCode() with flag `FLAG_USE_BP` instead.
     */
    static void readBP(Buffer& gcode, uint64_t* bp);

private:

    // handle a XF command
    static void handleXFCommand(Context* c, uint16_t address, uint16_t transferSize, Buffer& gcode);

    // handle a BP command at the specified address and with the specified value
    static void handleBPCommand(Context* context, uint8_t address, uint64_t value);
};

/*! @} addtogroup brres-ext-wgcode */

/*! @} addtogroup brres */
}
