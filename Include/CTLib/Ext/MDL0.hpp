//////////////////////////////////////////////////
//  Copyright (c) 2020 Nara Hiero
//
// This file is licensed under GPLv3+
// Refer to the `License.txt` file included.
//////////////////////////////////////////////////

#pragma


/*! @file Ext/MDL0.hpp
 *  
 *  @brief The header containing utilities for the MDL0 format.
 *  
 *  **Note**: This file is part of the BRRES module.
 */


#include <CTLib/BRRES.hpp>


namespace CTLib
{

namespace Ext
{

/*! @addtogroup brres
 *  @{
 */

/*! @defgroup brres-ext-mdl0 MDL0 Extension
 *  
 *  @addtogroup brres-ext-mdl0
 * 
 *  @brief The MDL0 extension of BRRES module contains additional utilities to
 *  handle the MDL0 subfile format.
 *  @{
 */

/*! @brief The ShaderCode class is used to parse and create Wii Graphics Code
 *  for a MDL0 Shader.
 */
class ShaderCode final
{

public:

    /*! @brief Enumeration of the swap table channels. */
    enum class Channel
    {
        Red = 0x0,
        Green = 0x1,
        Blue = 0x2,
        Alpha = 0x3
    };

    /*! @brief A swap table within a shader. */
    struct SwapTable final
    {

        /*! @brief Swap the red channel with this channel. */
        Channel red;

        /*! @brief Swap the green channel with this channel. */
        Channel green;

        /*! @brief Swap the blue channel with this channel. */
        Channel blue;

        /*! @brief Swap the alpha channel with this channel. */
        Channel alpha;
    };

    /*! @brief A stage within a shader. */
    class Stage final
    {


    };

    /*! @brief The number of swap tables per shader. */
    constexpr static uint32_t SWAP_TABLE_COUNT = 4;

    /*! @brief The number of indirect texturing stages per shader. */
    constexpr static uint32_t INDIRECT_TEX_STAGE_COUNT = 4;

    /*! @brief The maximum number of stages per shader. */
    constexpr static uint32_t MAX_STAGE_COUNT = 8;

    /*! @brief Parses the graphics code from the specified buffer.
     *  
     *  **Note**: Non-BP commands and unused BP addresses will be silently
     *  ignored by this function.
     *  
     *  @param[in] gcode The buffer containing the Wii Graphics Code.
     *  
     *  @throw CTLib::BRRESError If the graphics code is invalid.
     */
    static ShaderCode fromGraphicsCode(Buffer& gcode);

    /*! @brief Parses the graphics code from the specified buffer using the
     *  standard fixed-position code layout.
     *  
     *  **Note**: This function is very strict and not very user-friendly. The
     *  `fromGraphicsCode()` function should be used _whenever possible_.
     *  
     *  @param[in] gcode The buffer containing the Wii Graphics Code
     *  
     *  @throw CTLib::BRRESError If the graphics code is invalid, contains
     *  illegal commands, or is not formatted in the standard layout.
     *  
     *  @return The ShaderCode representation of the graphics code
     */
    static ShaderCode fromStandardLayout(Buffer& gcode);

    /*! @brief Constructs a new ShaderCode instance. */
    ShaderCode();

    /*! @brief Generates graphics code from this ShaderCode object using the
     *  standard fixed-position code layout.
     *  
     *  @return A buffer containing the generated Wii Graphics Code
     */
    Buffer toStandardLayout() const;

    /*! @brief Sets the swap table at the specified index.
     *  
     *  @param[in] index The index of the swap table to be modified
     *  @param[in] table The new value of the swap table
     *  
     *  @throw CTLib::BRRESError If the specified index is more than or equal to
     *  `SWAP_TABLE_COUNT`.
     */
    void setSwapTable(uint32_t index, SwapTable table);

    /*! @brief Returns the swap table at the specified index.
     *  
     *  @param[in] index The index of the swap table
     *  
     *  @throw CTLib::BRRESError If the specified index is more than or equal to
     *  `SWAP_TABLE_COUNT`.
     */
    SwapTable getSwapTable(uint32_t index) const;

    /*! @brief Sets the texture map ID for the specified indirect texturing
     *  stage.
     *  
     *  @param[in] stage The indirect texturing stage index
     *  @param[in] id The texture map ID
     *  
     *  @throw CTLib::BRRESError If the specified stage is more than or equal to
     *  `INDIRECT_TEX_STAGE_COUNT` or the specified tex map ID is more than or
     *  equal to `MDL0::Material::MAX_LAYER_COUNT`.
     */
    void setTexMapID(uint32_t stage, uint32_t id);

    /*! @brief Sets the texture coord index for the specified indirect texturing
     *  stage.
     *  
     *  @param[in] stage The indirect texturing stage index
     *  @param[in] index The texture coord index
     *  
     *  @throw CTLib::BRRESError If the specified stage is more than or equal to
     *  `INDIRECT_TEX_STAGE_COUNT` or the specified tex coord index is more than
     *  or equal to `MDL0::Object::TEX_COORD_ARRAY_COUNT`.
     */
    void setTexCoordIndex(uint32_t stage, uint32_t index);

    /*! @brief Returns the texture map ID for the specified indirect texturing
     *  stage.
     *  
     *  @param[in] stage The indirect texturing stage index
     *  
     *  @throw CTLib::BRRESError If the specified stage is more than or equal to
     *  `INDIRECT_TEX_STAGE_COUNT`.
     */
    uint32_t getTexMapID(uint32_t stage) const;

    /*! @brief Returns the texture coord index for the specified indirect
     *  texturing stage.
     *  
     *  @param[in] stage The indirect texturing stage index
     *  
     *  @throw CTLib::BRRESError If the specified stage is more than or equal to
     *  `INDIRECT_TEX_STAGE_COUNT`.
     */
    uint32_t getTexCoordIndex(uint32_t stage) const;

private:

    // throws if 'index' >= 'SWAP_TABLE_COUNT'
    void assertValidSwapTableIndex(uint32_t index) const;

    // throws if 'stage' >= 'INDIRECT_TEX_STAGE_COUNT'
    void assertValidIndirectStageIndex(uint32_t stage) const;

    // throws if 'id' >= 'MDL0::Material::MAX_LAYER_COUNT'
    void assertValidTexMapID(uint32_t id) const;

    // throws if 'index' >= 'MDL0::Object::TEX_COORD_ARRAY_COUNT'
    void assertValidTexCoordIndex(uint32_t index) const;

    // swap tables
    SwapTable tables[SWAP_TABLE_COUNT];

    // indirect texture map IDs
    uint32_t texMaps[INDIRECT_TEX_STAGE_COUNT];

    // indirect texture coord indices
    uint32_t texCoords[INDIRECT_TEX_STAGE_COUNT];

    // stages
    std::vector<Stage> stages;
};

/*! @} addtogroup brres-ext-mdl0 */

/*! @} addtogroup brres */
}
}
