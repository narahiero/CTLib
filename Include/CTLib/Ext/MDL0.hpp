//////////////////////////////////////////////////
//  Copyright (c) 2020 Nara Hiero
//
// This file is licensed under GPLv3+
// Refer to the `License.txt` file included.
//////////////////////////////////////////////////

#pragma once


/*! @file Ext/MDL0.hpp
 *  
 *  @brief The header containing utilities for the MDL0 format.
 *  
 *  **Note**: This file is part of the BRRES module.
 */


#include <CTLib/BRRES.hpp>


namespace CTLib::Ext
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

        friend class ShaderCode;

    public:

        /*! @brief Enumeration of the possible stage raster colour values. */
        enum class RasterColour
        {
            /*! @brief Load colour from the material's light channel 0. */
            LightChannel0 = 0x0,

            /*! @brief Load colour from the material's light channel 1. */
            LightChannel1 = 0x1,

            /*! @brief Unknown... */
            BumpAlpha = 0x5,

            /*! @brief Same as `BumpAlpha`, clamped between `0.f` and `1.f`. */
            NormalizedBumpAlpha = 0x6,

            /*! @brief Disabled (transparent black). */
            None = 0x7
        };

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

        /*! @brief Enumeration of the possible bias values. */
        enum class Bias
        {
            /*! @brief Zero. */
            Zero = 0x0,

            /*! @brief Add `0.5f`. */
            AddHalf = 0x1,

            /*! @brief Subtract `0.5f`. */
            SubHalf = 0x2,

            /*! @brief Special case. */
            SpecialCase = 0x3
        };

        /*! @brief Enumeration of the possible operation values. */
        enum class Op
        {
            /*! @brief Add. */
            Add = 0x0,

            /*! @brief Subtract. */
            Sub = 0x1
        };

        /*! @brief Enumeration of the possible shift values. */
        enum class Shift
        {
            /*! @brief None (shift by `0`). */
            Shift0 = 0x0,

            /*! @brief Shift left by `1`. */
            LShift1 = 0x1,

            /*! @brief Shift left by `2`. */
            LShift2 = 0x2,

            /*! @brief Shift right by `1`. */
            RShift1 = 0x3
        };

        /*! @brief Enumeration of the possible destination values. */
        enum class Dest
        {
            /*! @brief Output to fragment. */
            PixelOutput = 0x0,

            /*! @brief Output to material's colour block 0. */
            Colour0 = 0x1,

            /*! @brief Output to material's colour block 1. */
            Colour1 = 0x2,

            /*! @brief Output to material's colour block 2. */
            Colour2 = 0x3
        };

        /*! @brief The colour operation of a shader stage. */
        struct ColourOp final
        {

            /*! @brief Enumeration of the possible colour operation args. */
            enum class Arg
            {
                /*! @brief The colour of last value outputted to `PixelOutput`,
                 *  initially black.
                 */
                PixelOutput = 0x0,

                /*! @brief The alpha of last value outputted to `PixelOutput`,
                 *  initially zero.
                 */
                PixelOutputAlpha = 0x1,

                /*! @brief The material's colour block 0. */
                Colour0 = 0x2,

                /*! @brief The material's colour block 0 alpha. */
                Colour0Alpha = 0x3,

                /*! @brief The material's colour block 1. */
                Colour1 = 0x4,

                /*! @brief The material's colour block 1 alpha. */
                Colour1Alpha = 0x5,

                /*! @brief The material's colour block 2. */
                Colour2 = 0x6,

                /*! @brief The material's colour block 2 alpha. */
                Colour2Alpha = 0x7,

                /*! @brief The texture colour. */
                Texture = 0x8,

                /*! @brief The texture alpha. */
                TextureAlpha = 0x9,

                /*! @brief The raster colour. */
                Raster = 0xA,

                /*! @brief The raster alpha. */
                RasterAlpha = 0xB,

                /*! @brief White (`1.f`). */
                One = 0xC,

                /*! @brief Gray (`0.5f`). */
                Half = 0xD,

                /*! @brief The constant selection colour. */
                Constant = 0xE,

                /*! @brief Black (`0.f`). */
                Zero = 0xF
            };

            /*! @brief Argument `A` of the colour operation. */
            Arg argA;

            /*! @brief Argument `B` of the colour operation. */
            Arg argB;

            /*! @brief Argument `C` of the colour operation. */
            Arg argC;

            /*! @brief Argument `D` of the colour operation. */
            Arg argD;

            /*! @brief Bias of the colour operation. */
            Bias bias;

            /*! @brief Operator of the colour operation. */
            Op op;

            /*! @brief Whether to clamp the output between `0.f` and `1.f`. */
            bool clamp;

            /*! @brief Value shift of the colour operation. */
            Shift shift;

            /*! @brief Destination of the colour operation. */
            Dest dest;
        };

        /*! @brief The alpha operation of a shader stage. */
        struct AlphaOp final
        {

            /*! @brief Enumeration of the possible alpha operation args. */
            enum class Arg
            {
                /*! @brief The alpha of last value outputted to `PixelOutput`,
                 *  initially zero.
                 */
                PixelOutput = 0x0,

                /*! @brief The material's colour block 0 alpha. */
                Colour0 = 0x1,

                /*! @brief The material's colour block 1 alpha. */
                Colour1 = 0x2,

                /*! @brief The material's colour block 2 alpha. */
                Colour2 = 0x3,

                /*! @brief The texture alpha. */
                Texture = 0x4,

                /*! @brief The raster alpha. */
                Raster = 0x5,

                /*! @brief The constant selection alpha. */
                Constant = 0x6,

                /*! @brief Transparent (`0.f`). */
                Zero = 0x7
            };

            /*! @brief Argument `A` of the alpha operation. */
            Arg argA;

            /*! @brief Argument `B` of the alpha operation. */
            Arg argB;

            /*! @brief Argument `C` of the alpha operation. */
            Arg argC;

            /*! @brief Argument `D` of the alpha operation. */
            Arg argD;

            /*! @brief Bias of the alpha operation. */
            Bias bias;

            /*! @brief Operator of the alpha operation. */
            Op op;

            /*! @brief Whether to clamp the output between `0.f` and `1.f`. */
            bool clamp;

            /*! @brief Value shift of the alpha operation. */
            Shift shift;

            /*! @brief Destination of the alpha operation. */
            Dest dest;
        };

        /*! @brief Constructs a new Stage instance with default values. */
        Stage();

        /*! @brief Sets whether this stage uses a texture. */
        void setUsesTexture(bool use);

        /*! @brief Returns whether this stage uses a texture. */
        bool usesTexture() const;

        /*! @brief Sets the texture map ID of this shader stage.
         *  
         *  @throw CTLib::BRRESError If the specified texture map ID is more
         *  than or equal to `MDL0::Material::MAX_LAYER_COUNT`.
         */
        void setTexMapID(uint32_t id);

        /*! @brief Sets the texture coord index of this shader stage.
         *  
         *  @throw CTLib::BRRESError If the specified texture coord index is
         *  more than or equal to `MDL0::Object::TEX_COORD_ARRAY_COUNT`.
         */
        void setTexCoordIndex(uint32_t index);

        /*! @brief Returns the texture map ID of this shader stage. */
        uint32_t getTexMapID() const;

        /*! @brief Returns the texture coord index of this shader stage. */
        uint32_t getTexCoordIndex() const;

        /*! @brief Sets the raster colour input of this shader stage. */
        void setRasterColour(RasterColour colour);

        /*! @brief Returns the raster colour input of this shader stage. */
        RasterColour getRasterColour() const;

        /*! @brief Sets the colour constant argument source of this shader
         *  stage.
         *  
         *  @param[in] source The constant argument source
         */
        void setColourConstantSource(ColourConstant source);

        /*! @brief Sets the alpha constant argument source of this shader stage.
         *  
         *  @param[in] source The constant argument source
         */
        void setAlphaConstantSource(AlphaConstant source);

        /*! @brief Returns the colour constant argument source of this shader
         *  stage.
         */
        ColourConstant getColourConstantSource() const;

        /*! @brief Returns the alpha constant argument source of this shader
         *  stage.
         */
        AlphaConstant getAlphaConstantSource() const;

        /*! @brief Sets the colour operation of this shader stage. */
        void setColourOp(ColourOp op);

        /*! @brief Sets the alpha operation of this shader stage. */
        void setAlphaOp(AlphaOp op);

        /*! @brief Returns the colour operation of this shader stage. */
        ColourOp getColourOp() const;

        /*! @brief Returns the alpha operation of this shader stage. */
        AlphaOp getAlphaOp() const;

    private:

        // throws if 'id' >= 'MDL0::Material::MAX_LAYER_COUNT'
        void assertValidTexMapID(uint32_t id) const;

        // throws if 'index' >= 'MDL0::Object::TEX_COORD_ARRAY_COUNT'
        void assertValidTexCoordIndex(uint32_t index) const;

        // whether to use a texture
        bool useTexture;

        // material layer id
        uint32_t texMap;

        // object tex coord index
        uint32_t texCoord;

        // raster colour input
        RasterColour rasterColour;

        // colour operation constant arg source
        ColourConstant colourCSrc;

        // alpha operation constant arg source
        AlphaConstant alphaCSrc;

        // colour operation
        ColourOp colourOp;

        // alpha operation
        AlphaOp alphaOp;
    };

    /*! @brief The number of swap tables per shader. */
    constexpr static uint32_t SWAP_TABLE_COUNT = 4;

    /*! @brief The number of indirect texturing stages per shader. */
    constexpr static uint32_t INDIRECT_TEX_STAGE_COUNT = 4;

    /*! @brief The maximum number of stages per shader. */
    constexpr static uint32_t MAX_STAGE_COUNT = 8;

    /*! @brief Parses the graphics code from the specified buffer.
     *  
     *  **Note**: Unused BP addresses will be silently ignored by this function.
     *  
     *  @param[in] gcode The buffer containing the Wii Graphics Code.
     *  @param[in] stageCount The number of shader stages
     *  
     *  @throw CTLib::BRRESError If the graphics code is invalid or contains
     *  illegal (non-BP) commands.
     */
    static ShaderCode fromGraphicsCode(Buffer& gcode, uint32_t stageCount);

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
