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

    public:

        /*! @brief Enumeration of the colour operation constant arg values. */
        enum class ColourConstant
        {
            /*! @brief 1/1 (`1.f`). */
            Constant_1_1 = 0x00,

            /*! @brief 7/8 (`0.875f`). */
            Constant_7_8 = 0x01,

            /*! @brief 3/4 (`0.75f`). */
            Constant_3_4 = 0x02,

            /*! @brief 5/8 (`0.625f`). */
            Constant_5_8 = 0x03,

            /*! @brief 1/2 (`0.5f`). */
            Constant_1_2 = 0x04,

            /*! @brief 3/8 (`0.375f`). */
            Constant_3_8 = 0x05,

            /*! @brief 1/4 (`0.25f`). */
            Constant_1_4 = 0x06,

            /*! @brief 1/8 (`0.125f`). */
            Constant_1_8 = 0x07,

            /*! @brief Material constant colour 0 (`RGB`). */
            MaterialConstColour0_RGB = 0x0C,

            /*! @brief Material constant colour 1 (`RGB`). */
            MaterialConstColour1_RGB = 0x0D,

            /*! @brief Material constant colour 2 (`RGB`). */
            MaterialConstColour2_RGB = 0x0E,

            /*! @brief Material constant colour 3 (`RGB`). */
            MaterialConstColour3_RGB = 0x0F,

            /*! @brief Material constant colour 0 (`RRR`). */
            MaterialConstColour0_RRR = 0x10,

            /*! @brief Material constant colour 1 (`RRR`). */
            MaterialConstColour1_RRR = 0x11,

            /*! @brief Material constant colour 2 (`RRR`). */
            MaterialConstColour2_RRR = 0x12,

            /*! @brief Material constant colour 3 (`RRR`). */
            MaterialConstColour3_RRR = 0x13,

            /*! @brief Material constant colour 0 (`GGG`). */
            MaterialConstColour0_GGG = 0x14,

            /*! @brief Material constant colour 1 (`GGG`). */
            MaterialConstColour1_GGG = 0x15,

            /*! @brief Material constant colour 2 (`GGG`). */
            MaterialConstColour2_GGG = 0x16,

            /*! @brief Material constant colour 3 (`GGG`). */
            MaterialConstColour3_GGG = 0x17,

            /*! @brief Material constant colour 0 (`BBB`). */
            MaterialConstColour0_BBB = 0x18,

            /*! @brief Material constant colour 1 (`BBB`). */
            MaterialConstColour1_BBB = 0x19,

            /*! @brief Material constant colour 2 (`BBB`). */
            MaterialConstColour2_BBB = 0x1A,

            /*! @brief Material constant colour 3 (`BBB`). */
            MaterialConstColour3_BBB = 0x1B,

            /*! @brief Material constant colour 0 (`AAA`). */
            MaterialConstColour0_AAA = 0x1C,

            /*! @brief Material constant colour 1 (`AAA`). */
            MaterialConstColour1_AAA = 0x1D,

            /*! @brief Material constant colour 2 (`AAA`). */
            MaterialConstColour2_AAA = 0x1E,

            /*! @brief Material constant colour 3 (`AAA`). */
            MaterialConstColour3_AAA = 0x1F,
        };

        /*! @brief Enumeration of the alpha operation constant arg values. */
        enum class AlphaConstant
        {
            /*! @brief 1/1 (`1.f`). */
            Constant_1_1 = 0x00,

            /*! @brief 7/8 (`0.875f`). */
            Constant_7_8 = 0x01,

            /*! @brief 3/4 (`0.75f`). */
            Constant_3_4 = 0x02,

            /*! @brief 5/8 (`0.625f`). */
            Constant_5_8 = 0x03,

            /*! @brief 1/2 (`0.5f`). */
            Constant_1_2 = 0x04,

            /*! @brief 3/8 (`0.375f`). */
            Constant_3_8 = 0x05,

            /*! @brief 1/4 (`0.25f`). */
            Constant_1_4 = 0x06,

            /*! @brief 1/8 (`0.125f`). */
            Constant_1_8 = 0x07,

            /*! @brief Material constant colour 0 (`Red`). */
            MaterialConstColour0_Red = 0x10,

            /*! @brief Material constant colour 1 (`Red`). */
            MaterialConstColour1_Red = 0x11,

            /*! @brief Material constant colour 2 (`Red`). */
            MaterialConstColour2_Red = 0x12,

            /*! @brief Material constant colour 3 (`Red`). */
            MaterialConstColour3_Red = 0x13,

            /*! @brief Material constant colour 0 (`Green`). */
            MaterialConstColour0_Green = 0x14,

            /*! @brief Material constant colour 1 (`Green`). */
            MaterialConstColour1_Green = 0x15,

            /*! @brief Material constant colour 2 (`Green`). */
            MaterialConstColour2_Green = 0x16,

            /*! @brief Material constant colour 3 (`Green`). */
            MaterialConstColour3_Green = 0x17,

            /*! @brief Material constant colour 0 (`Blue`). */
            MaterialConstColour0_Blue = 0x18,

            /*! @brief Material constant colour 1 (`Blue`). */
            MaterialConstColour1_Blue = 0x19,

            /*! @brief Material constant colour 2 (`Blue`). */
            MaterialConstColour2_Blue = 0x1A,

            /*! @brief Material constant colour 3 (`Blue`). */
            MaterialConstColour3_Blue = 0x1B,

            /*! @brief Material constant colour 0 (`Alpha`). */
            MaterialConstColour0_Alpha = 0x1C,

            /*! @brief Material constant colour 1 (`Alpha`). */
            MaterialConstColour1_Alpha = 0x1D,

            /*! @brief Material constant colour 2 (`Alpha`). */
            MaterialConstColour2_Alpha = 0x1E,

            /*! @brief Material constant colour 3 (`Alpha`). */
            MaterialConstColour3_Alpha = 0x1F,
        };

        /*! @brief Constructs a new Stage instance with default values. */
        Stage();

        /*! @brief Sets the colour operation constant argument source of this
         *  shader stage.
         *  
         *  @param[in] source The constant argument source
         */
        void setColourOpConstantSource(ColourConstant source);

        /*! @brief Sets the alpha operation constant argument source of this
         *  shader stage.
         *  
         *  @param[in] source The constant argument source
         */
        void setAlphaOpConstantSource(AlphaConstant source);

        /*! @brief Returns the colour operation constant argument source of this
         *  shader stage.
         */
        ColourConstant getColourOpConstantSource() const;

        /*! @brief Returns the alpha operation constant argument source of this
         *  shader stage.
         */
        AlphaConstant getAlphaOpConstantSource() const;

    private:

        // colour operation constant arg source
        ColourConstant colourCSrc;

        // alpha operation constant arg source
        AlphaConstant alphaCSrc;
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

    /*! @brief Constructs a new ShaderCode instance with default values. */
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

    /*! @brief Adds a stage to this shader and returns it.
     *  
     *  @throw CTLib::BRRESError If this shader already has `MAX_STAGE_COUNT`
     *  stages.
     */
    Stage& addStage();

    /*! @brief Returns the shader stage at the specified index.
     *  
     *  @param[in] index The shader stage index
     *  
     *  @throw CTLib::BRRESError If the specified index is more than or equal to
     *  the stage count of this shader.
     */
    Stage& getStage(uint32_t index);

    /*! @brief Returns the stage count of this shader. */
    uint32_t getStageCount() const;

private:

    // throws if 'index' >= 'SWAP_TABLE_COUNT'
    void assertValidSwapTableIndex(uint32_t index) const;

    // throws if 'stage' >= 'INDIRECT_TEX_STAGE_COUNT'
    void assertValidIndirectStageIndex(uint32_t stage) const;

    // throws if 'id' >= 'MDL0::Material::MAX_LAYER_COUNT'
    void assertValidTexMapID(uint32_t id) const;

    // throws if 'index' >= 'MDL0::Object::TEX_COORD_ARRAY_COUNT'
    void assertValidTexCoordIndex(uint32_t index) const;

    // throws if 'stages.size()' >= 'MAX_STAGE_COUNT'
    void assertCanAddStage() const;

    // throws if 'index' >= 'stages.size()'
    void assertValidStageIndex(uint32_t index) const;

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
