//////////////////////////////////////////////////
//  Copyright (c) 2020 Nara Hiero
//
// This file is licensed under GPLv3+
// Refer to the `License.txt` file included.
//////////////////////////////////////////////////

#include <CTLib/Ext/MDL0.hpp>

#include <CTLib/Ext/WGCode.hpp>
#include <CTLib/Utilities.hpp>

namespace CTLib::Ext
{

/// Graphics code utilities

inline void fillNOOPs(Buffer& gcode, uint32_t count)
{
    for (uint32_t i = 0; i < count; ++i)
    {
        gcode.put(0x00);
    }
}

inline void putBPCommand(Buffer& gcode, uint8_t address, uint32_t value)
{
    gcode.put(0x61).putInt((address << 24) | (value & 0x00FFFFFF));
}

inline uint32_t toBPAlphaConstValue(bool enable, uint8_t value)
{
    return ((enable ? 1 : 0) << 8) | value;
}

inline uint32_t toBPConstColourARValue(MaterialCode::Colour colour)
{
    return 0x800000 | ((colour.alpha & 0x7FF) << 12) | (colour.red & 0x7FF);
}

inline uint32_t toBPConstColourGBValue(MaterialCode::Colour colour)
{
    return 0x800000 | ((colour.green & 0x7FF) << 12) | (colour.blue & 0x7FF);
}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////
////   MaterialCode class
////

MaterialCode MaterialCode::fromGraphicsCode(Buffer& gcode)
{
    MaterialCode mat;



    return mat;
}

Buffer MaterialCode::toStandardLayout() const
{
    Buffer gcode(0x180);

    fillNOOPs(gcode, 0x14); // temporary space fill
    putBPCommand(gcode, 0x42, toBPAlphaConstValue(useConstAlpha, constAlpha));
    fillNOOPs(gcode, 0x7); // padding

    // non-const material colour registers
    for (uint32_t i = 0; i < 3; ++i)
    {
        putBPCommand(gcode, 0xE2 + (i << 1), 0x000000);
        putBPCommand(gcode, 0xE3 + (i << 1), 0x000000);
        putBPCommand(gcode, 0xE3 + (i << 1), 0x000000);
        putBPCommand(gcode, 0xE3 + (i << 1), 0x000000);
    }
    fillNOOPs(gcode, 0x4); // padding

    // const material colour registers
    for (uint32_t i = 0; i < CONST_COLOUR_COUNT; ++i)
    {
        putBPCommand(gcode, 0xE0 + (i << 1), toBPConstColourARValue(constColours[i]));
        putBPCommand(gcode, 0xE1 + (i << 1), toBPConstColourGBValue(constColours[i]));
    }
    fillNOOPs(gcode, 0x18); // padding

    fillNOOPs(gcode, 0xE0); // temporary space fill

    return gcode.clear();
}

MaterialCode::MaterialCode() :
    useConstAlpha{false},
    constAlpha{0xFF}
{
    for (uint32_t i = 0; i < CONST_COLOUR_COUNT; ++i)
    {
        constColours[i] = {0x000, 0x000, 0x000, 0x000};
    }
}

void MaterialCode::setUseConstAlpha(bool enable)
{
    useConstAlpha = enable;
}

void MaterialCode::setConstAlpha(uint8_t alpha)
{
    constAlpha = alpha;
}

bool MaterialCode::usesConstAlpha() const
{
    return useConstAlpha;
}

uint8_t MaterialCode::getConstAlpha() const
{
    return constAlpha;
}

void MaterialCode::setConstColour(uint32_t index, Colour colour)
{
    assertValidConstColourIndex(index);
    constColours[index] = colour;
}

MaterialCode::Colour MaterialCode::getConstColour(uint32_t index) const
{
    assertValidConstColourIndex(index);
    return constColours[index];
}

void MaterialCode::assertValidConstColourIndex(uint32_t index) const
{
    if (index >= CONST_COLOUR_COUNT)
    {
        throw BRRESError(Strings::format(
            "MaterialCode: Constant colour block index out of range! (%d >= %d)",
            index, CONST_COLOUR_COUNT
        ));
    }
}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////
////   ShaderCode class
////

void fromIndirectSources(ShaderCode& shader, uint32_t val)
{
    for (uint32_t i = 0; i < ShaderCode::INDIRECT_TEX_STAGE_COUNT; ++i)
    {
        shader.setTexMapID(i, (val >> (i * 6)) & 0x7);
        shader.setTexCoordIndex(i, (val >> (3 + (i * 6))) & 0x7);
    }
}

void fromSwapTable(ShaderCode& shader, uint32_t idx, bool rg, uint32_t val)
{
    ShaderCode::SwapTable table = shader.getSwapTable(idx);
    (rg ? table.red : table.blue) = static_cast<ShaderCode::Channel>(val & 0x3);
    (rg ? table.green : table.alpha) = static_cast<ShaderCode::Channel>((val >> 2) & 0x3);
    shader.setSwapTable(idx, table);
}

void fromStageConstants(ShaderCode::Stage& stage, bool n2, uint32_t val)
{
    stage.setColourConstantSource(
        static_cast<ShaderCode::Stage::ColourConstant>((val >> (n2 ? 14 : 4)) & 0x1F));

    stage.setAlphaConstantSource(
        static_cast<ShaderCode::Stage::AlphaConstant>((val >> (n2 ? 19 : 9)) & 0x1F));
}

void fromStageSources(ShaderCode::Stage& stage, bool n2, uint32_t val)
{
    stage.setTexMapID((val >> (n2 ? 12 : 0)) & 0x7);
    stage.setTexCoordIndex((val >> (n2 ? 15 : 3)) & 0x7);
    stage.setUsesTexture((val >> (n2 ? 18 : 6)) & 0x1);
    stage.setRasterColour(
        static_cast<ShaderCode::Stage::RasterColour>((val >> (n2 ? 19 : 7)) & 0x7));
}

void fromColourOp(ShaderCode::Stage& stage, uint32_t val)
{
    ShaderCode::Stage::ColourOp cop;
    cop.argD  = static_cast<ShaderCode::Stage::ColourOp::Arg>((val      ) & 0xF);
    cop.argC  = static_cast<ShaderCode::Stage::ColourOp::Arg>((val >>  4) & 0xF);
    cop.argB  = static_cast<ShaderCode::Stage::ColourOp::Arg>((val >>  8) & 0xF);
    cop.argA  = static_cast<ShaderCode::Stage::ColourOp::Arg>((val >> 12) & 0xF);
    cop.bias  = static_cast<ShaderCode::Stage::Bias         >((val >> 16) & 0x3);
    cop.op    = static_cast<ShaderCode::Stage::Op           >((val >> 18) & 0x1);
    cop.clamp = static_cast<bool                            >((val >> 19) & 0x1);
    cop.shift = static_cast<ShaderCode::Stage::Shift        >((val >> 20) & 0x3);
    cop.dest  = static_cast<ShaderCode::Stage::Dest         >((val >> 22) & 0x3);
    stage.setColourOp(cop);
}

void fromAlphaOp(ShaderCode::Stage& stage, uint32_t val)
{
    stage.setRasterSwapTable(val & 0x3);
    stage.setTextureSwapTable((val >> 2) & 0x3);

    ShaderCode::Stage::AlphaOp aop;
    aop.argD  = static_cast<ShaderCode::Stage::AlphaOp::Arg>((val >>  4) & 0x7);
    aop.argC  = static_cast<ShaderCode::Stage::AlphaOp::Arg>((val >>  7) & 0x7);
    aop.argB  = static_cast<ShaderCode::Stage::AlphaOp::Arg>((val >> 10) & 0x7);
    aop.argA  = static_cast<ShaderCode::Stage::AlphaOp::Arg>((val >> 13) & 0x7);
    aop.bias  = static_cast<ShaderCode::Stage::Bias        >((val >> 16) & 0x3);
    aop.op    = static_cast<ShaderCode::Stage::Op          >((val >> 18) & 0x1);
    aop.clamp = static_cast<bool                           >((val >> 19) & 0x1);
    aop.shift = static_cast<ShaderCode::Stage::Shift       >((val >> 20) & 0x3);
    aop.dest  = static_cast<ShaderCode::Stage::Dest        >((val >> 22) & 0x3);
    stage.setAlphaOp(aop);
}

ShaderCode ShaderCode::fromGraphicsCode(Buffer& gcode, uint32_t stageCount)
{
    if (stageCount > MAX_STAGE_COUNT)
    {
        throw BRRESError(Strings::format(
            "ShaderCode: Stage count out of range! (%d > %d)",
            stageCount, MAX_STAGE_COUNT
        ));
    }

    ShaderCode shader;
    for (uint32_t i = 0; i < stageCount; ++i)
    {
        shader.addStage();
    }

    WGCode::Context c;
    WGCode::readGraphicsCode(gcode, &c, true, WGCode::FLAG_USE_BP);

    for (uint32_t i = 0; i < SWAP_TABLE_COUNT; ++i)
    {
        fromSwapTable(shader, i, true, (uint32_t)c.bp[WGCode::BP_SWAP_TABLE + (i << 1)]);
        fromSwapTable(shader, i, false, (uint32_t)c.bp[WGCode::BP_SWAP_TABLE + 1 + (i << 1)]);
    }

    fromIndirectSources(shader, (uint32_t)c.bp[WGCode::BP_IND_TEX_REF]);

    for (uint32_t s = 0; s < stageCount; ++s)
    {
        Stage& stage = shader.getStage(s);

        fromStageConstants(stage, s & 1, (uint32_t)c.bp[WGCode::BP_STAGE_CONST_SRC + (s >> 1)]);
        fromStageSources(stage, s & 1, (uint32_t)c.bp[WGCode::BP_STAGE_SRC + (s >> 1)]);

        fromColourOp(stage, (uint32_t)c.bp[WGCode::BP_STAGE_COMBINER + (s << 1)]);
        fromAlphaOp(stage, (uint32_t)c.bp[WGCode::BP_STAGE_COMBINER + 1 + (s << 1)]);
    }

    return shader;
}

ShaderCode ShaderCode::fromStandardLayout(Buffer& gcode)
{
    return ShaderCode();
}

inline uint32_t toBPSwapTableValue(ShaderCode::Channel a, ShaderCode::Channel b)
{
    return ((uint32_t)a & 0x3) | (((uint32_t)b & 0x3) << 2);
}

inline uint32_t toBPIndirectIDsValue(const uint32_t* ms, const uint32_t* cs)
{
    return ((cs[0] & 0x7) << 21) | ((ms[0] & 0x7) << 18)
        |  ((cs[1] & 0x7) << 15) | ((ms[1] & 0x7) << 12)
        |  ((cs[2] & 0x7) <<  9) | ((ms[2] & 0x7) <<  6)
        |  ((cs[3] & 0x7) <<  3) | ( ms[3] & 0x7);
}

inline uint32_t toBPConstSelectionsValue(
    const std::vector<ShaderCode::Stage>& stages, uint32_t idx, bool two
)
{
    return (static_cast<uint32_t>(stages[idx].getColourConstantSource()) <<  4)
        |  (static_cast<uint32_t>(stages[idx].getAlphaConstantSource() ) <<  9)
        | (!two ? 0 :
              (static_cast<uint32_t>(stages[idx + 1].getColourConstantSource()) << 14)
            | (static_cast<uint32_t>(stages[idx + 1].getAlphaConstantSource() ) << 19)
        );
}

inline uint32_t toBPFragmentSourcesValueOne(const ShaderCode::Stage& stage)
{
    return (stage.getTexMapID() & 0x7) | ((stage.getTexCoordIndex() & 0x7) << 3)
        | (stage.usesTexture() ? 1 << 6 : 0)
        | ((static_cast<uint32_t>(stage.getRasterColour()) & 0x7) << 7);
}

inline uint32_t toBPFragmentSourcesValue(
    const std::vector<ShaderCode::Stage>& stages, uint32_t idx, bool two
)
{
    return toBPFragmentSourcesValueOne(stages[idx])
        | (!two ? 0 : toBPFragmentSourcesValueOne(stages[idx + 1]) << 12);
}

inline uint32_t toBPColourOpValue(const ShaderCode::Stage::ColourOp& op)
{
    return ((static_cast<uint32_t>(op.argD ) & 0xF)      )
        |  ((static_cast<uint32_t>(op.argC ) & 0xF) <<  4)
        |  ((static_cast<uint32_t>(op.argB ) & 0xF) <<  8)
        |  ((static_cast<uint32_t>(op.argA ) & 0xF) << 12)
        |  ((static_cast<uint32_t>(op.bias ) & 0x3) << 16)
        |  ((static_cast<uint32_t>(op.op   ) & 0x1) << 18)
        |  (!!op.clamp << 19)
        |  ((static_cast<uint32_t>(op.shift) & 0x3) << 20)
        |  ((static_cast<uint32_t>(op.dest ) & 0x3) << 22);
}

inline uint32_t toBPAlphaOpValue(const ShaderCode::Stage& stage, const ShaderCode::Stage::AlphaOp& op)
{
    return ((stage.getRasterSwapTable()  & 0x3))
        |  ((stage.getTextureSwapTable() & 0x3) << 2)
        |  ((static_cast<uint32_t>(op.argD ) & 0x7) <<  4)
        |  ((static_cast<uint32_t>(op.argC ) & 0x7) <<  7)
        |  ((static_cast<uint32_t>(op.argB ) & 0x7) << 10)
        |  ((static_cast<uint32_t>(op.argA ) & 0x7) << 13)
        |  ((static_cast<uint32_t>(op.bias ) & 0x3) << 16)
        |  ((static_cast<uint32_t>(op.op   ) & 0x1) << 18)
        |  (!!op.clamp << 19)
        |  ((static_cast<uint32_t>(op.shift) & 0x3) << 20)
        |  ((static_cast<uint32_t>(op.dest ) & 0x3) << 22);
}

#define CMD_NOOP(c) for (uint32_t _i = 0; _i < (c); ++_i) gcode.put(0x00)
#define BP_CMD(addr, val) gcode.put(0x61).putInt(((addr) << 24) | (val))
#define BP_MASK(mask) BP_CMD(0xFE, (mask))

#define BP_SWAP_TABLE(idx) \
    BP_MASK(0x00000F); \
    BP_CMD(0xF6 + ((idx) << 1), toBPSwapTableValue(tables[idx].red, tables[idx].green)); \
    BP_MASK(0x00000F); \
    BP_CMD(0xF7 + ((idx) << 1), toBPSwapTableValue(tables[idx].blue, tables[idx].alpha))

#define BP_SHADER_STAGE(idx, two) \
    BP_MASK(0xFFFFF0); \
    BP_CMD(0xF6 + ((idx) >> 1), toBPConstSelectionsValue(stages, idx, two)); \
    BP_CMD(0x28 + ((idx) >> 1), toBPFragmentSourcesValue(stages, idx, two)); \
    BP_CMD(0xC0 + ((idx) << 1), toBPColourOpValue(stages[idx].colourOp)); \
    if (two) BP_CMD(0xC2 + ((idx) << 1), toBPColourOpValue(stages[(idx) + 1].colourOp)); \
    else CMD_NOOP(0x5); \
    BP_CMD(0xC1 + ((idx) << 1), toBPAlphaOpValue(stages[idx], stages[idx].alphaOp)); \
    if (two) BP_CMD(0xC3 + ((idx) << 1), toBPAlphaOpValue(stages[(idx) + 1], stages[(idx) + 1].alphaOp)); \
    else CMD_NOOP(0x5); \
    BP_CMD(0x10 + (idx), 0x000000); \
    if (two) BP_CMD(0x11 + (idx), 0x000000); \
    else CMD_NOOP(0x5); \
    CMD_NOOP(0x3)

Buffer ShaderCode::toStandardLayout() const
{
    Buffer gcode(0x1E0);

    BP_SWAP_TABLE(0);
    BP_SWAP_TABLE(1);
    BP_SWAP_TABLE(2);
    BP_SWAP_TABLE(3);

    BP_CMD(0x27, toBPIndirectIDsValue(texMaps, texCoords));

    CMD_NOOP(0xB);

    uint32_t stageCount = getStageCount();
    for (uint32_t i = 0; i < (stageCount & ~1); i += 2)
    {
        BP_SHADER_STAGE(i, true);
    }
    if (stageCount & 1)
    {
        BP_SHADER_STAGE(stageCount - 1, false);
    }

    while (gcode.hasRemaining())
    {
        gcode.put(0x00);
    }

    return gcode.clear();
}

#undef BP_SWAP_TABLE

#undef BP_MASK
#undef BP_CMD
#undef CMD_NOOP

ShaderCode::ShaderCode()
{
    tables[0] = {Channel::Red,   Channel::Green, Channel::Blue,  Channel::Alpha};
    tables[1] = {Channel::Red,   Channel::Red,   Channel::Red,   Channel::Alpha};
    tables[2] = {Channel::Green, Channel::Green, Channel::Green, Channel::Alpha};
    tables[3] = {Channel::Blue,  Channel::Blue,  Channel::Blue,  Channel::Alpha};

    for (uint32_t i = 0; i < INDIRECT_TEX_STAGE_COUNT; ++i)
    {
        texMaps[i] = 7;
        texCoords[i] = 7;
    }
}

void ShaderCode::setSwapTable(uint32_t index, SwapTable table)
{
    assertValidSwapTableIndex(index);
    tables[index] = table;
}

ShaderCode::SwapTable ShaderCode::getSwapTable(uint32_t index) const
{
    assertValidSwapTableIndex(index);
    return tables[index];
}

void ShaderCode::setTexMapID(uint32_t stage, uint32_t id)
{
    assertValidIndirectStageIndex(stage);
    assertValidTexMapID(id);
    texMaps[stage] = id;
}

void ShaderCode::setTexCoordIndex(uint32_t stage, uint32_t index)
{
    assertValidIndirectStageIndex(stage);
    assertValidTexCoordIndex(index);
    texCoords[stage] = index;
}

uint32_t ShaderCode::getTexMapID(uint32_t stage) const
{
    assertValidIndirectStageIndex(stage);
    return texMaps[stage];
}

uint32_t ShaderCode::getTexCoordIndex(uint32_t stage) const
{
    assertValidIndirectStageIndex(stage);
    return texCoords[stage];
}

ShaderCode::Stage& ShaderCode::addStage()
{
    assertCanAddStage();
    return stages.emplace_back();
}

ShaderCode::Stage& ShaderCode::getStage(uint32_t index)
{
    assertValidStageIndex(index);
    return stages.at(index);
}

uint32_t ShaderCode::getStageCount() const
{
    return static_cast<uint32_t>(stages.size());
}

void ShaderCode::assertValidSwapTableIndex(uint32_t index) const
{
    if (index >= SWAP_TABLE_COUNT)
    {
        throw BRRESError(Strings::format(
            "ShaderCode: Swap table index out of range! (%d >= %d)",
            index, SWAP_TABLE_COUNT
        ));
    }
}

void ShaderCode::assertValidIndirectStageIndex(uint32_t stage) const
{
    if (stage >= INDIRECT_TEX_STAGE_COUNT)
    {
        throw BRRESError(Strings::format(
            "ShaderCode: Stage index out of range! (%d >= %d)",
            stage, INDIRECT_TEX_STAGE_COUNT
        ));
    }
}

void ShaderCode::assertValidTexMapID(uint32_t id) const
{
    if (id >= MDL0::Material::MAX_LAYER_COUNT)
    {
        throw BRRESError(Strings::format(
            "ShaderCode: Texture map ID out of range! (%d >= %d)",
            id, MDL0::Material::MAX_LAYER_COUNT
        ));
    }
}

void ShaderCode::assertValidTexCoordIndex(uint32_t index) const
{
    if (index >= MDL0::Object::TEX_COORD_ARRAY_COUNT)
    {
        throw BRRESError(Strings::format(
            "ShaderCode: Texture coord index out of range! (%d >= %d)",
            index, MDL0::Object::TEX_COORD_ARRAY_COUNT
        ));
    }
}

void ShaderCode::assertCanAddStage() const
{
    if (stages.size() >= MAX_STAGE_COUNT)
    {
        throw BRRESError(Strings::format(
            "ShaderCode: This shader already has the maximum number of stages! (%d)",
            MAX_STAGE_COUNT
        ));
    }
}

void ShaderCode::assertValidStageIndex(uint32_t index) const
{
    if (index >= stages.size())
    {
        throw BRRESError(Strings::format(
            "ShaderCode: Stage index out of range! (%d >= %d)",
            index, stages.size()
        ));
    }
}

////////// Stage class /////////////////

ShaderCode::Stage::Stage() :
    useTexture{false},
    texMap{0},
    texCoord{0},
    rasterColour{RasterColour::LightChannel0},
    texSwapTable{0},
    rasterSwapTable{0},
    colourCSrc{ColourConstant::MaterialConstColour0_RGB},
    alphaCSrc{AlphaConstant::MaterialConstColour0_Alpha},
    colourOp{
        ColourOp::Arg::Zero, ColourOp::Arg::Zero, ColourOp::Arg::Zero, ColourOp::Arg::Zero,
        Bias::Zero, Op::Add, true, Shift::Shift0, Dest::PixelOutput
    },
    alphaOp{
        AlphaOp::Arg::Zero, AlphaOp::Arg::Zero, AlphaOp::Arg::Zero, AlphaOp::Arg::Zero,
        Bias::Zero, Op::Add, true, Shift::Shift0, Dest::PixelOutput
    }
{

}

void ShaderCode::Stage::setUsesTexture(bool use)
{
    useTexture = use;
}

bool ShaderCode::Stage::usesTexture() const
{
    return useTexture;
}

void ShaderCode::Stage::setTexMapID(uint32_t id)
{
    assertValidTexMapID(id);
    texMap = id;
}

void ShaderCode::Stage::setTexCoordIndex(uint32_t index)
{
    assertValidTexCoordIndex(index);
    texCoord = index;
}

uint32_t ShaderCode::Stage::getTexMapID() const
{
    return texMap;
}

uint32_t ShaderCode::Stage::getTexCoordIndex() const
{
    return texCoord;
}

void ShaderCode::Stage::setRasterColour(RasterColour colour)
{
    rasterColour = colour;
}

ShaderCode::Stage::RasterColour ShaderCode::Stage::getRasterColour() const
{
    return rasterColour;
}

void ShaderCode::Stage::setTextureSwapTable(uint32_t table)
{
    assertValidSwapTable(table);
    texSwapTable = table;
}

void ShaderCode::Stage::setRasterSwapTable(uint32_t table)
{
    assertValidSwapTable(table);
    rasterSwapTable = table;
}

uint32_t ShaderCode::Stage::getTextureSwapTable() const
{
    return texSwapTable;
}

uint32_t ShaderCode::Stage::getRasterSwapTable() const
{
    return rasterSwapTable;
}

void ShaderCode::Stage::setColourConstantSource(ColourConstant source)
{
    colourCSrc = source;
}

void ShaderCode::Stage::setAlphaConstantSource(AlphaConstant source)
{
    alphaCSrc = source;
}

ShaderCode::Stage::ColourConstant ShaderCode::Stage::getColourConstantSource() const
{
    return colourCSrc;
}

ShaderCode::Stage::AlphaConstant ShaderCode::Stage::getAlphaConstantSource() const
{
    return alphaCSrc;
}

void ShaderCode::Stage::setColourOp(ColourOp op)
{
    colourOp = op;
}

void ShaderCode::Stage::setAlphaOp(AlphaOp op)
{
    alphaOp = op;
}

ShaderCode::Stage::ColourOp ShaderCode::Stage::getColourOp() const
{
    return colourOp;
}

ShaderCode::Stage::AlphaOp ShaderCode::Stage::getAlphaOp() const
{
    return alphaOp;
}

void ShaderCode::Stage::assertValidTexMapID(uint32_t id) const
{
    if (id >= MDL0::Material::MAX_LAYER_COUNT)
    {
        throw BRRESError(Strings::format(
            "ShaderCode: Stage texture map ID out of range! (%d >= %d)",
            id, MDL0::Material::MAX_LAYER_COUNT
        ));
    }
}

void ShaderCode::Stage::assertValidTexCoordIndex(uint32_t index) const
{
    if (index >= MDL0::Object::TEX_COORD_ARRAY_COUNT)
    {
        throw BRRESError(Strings::format(
            "ShaderCode: Stage texture coord index out of range! (%d >= %d)",
            index, MDL0::Object::TEX_COORD_ARRAY_COUNT
        ));
    }
}

void ShaderCode::Stage::assertValidSwapTable(uint32_t table) const
{
    if (table >= ShaderCode::SWAP_TABLE_COUNT)
    {
        throw BRRESError(Strings::format(
            "ShaderCode: Stage swap table out of range! (%d >= %d)",
            table, ShaderCode::SWAP_TABLE_COUNT
        ));
    }
}
}
