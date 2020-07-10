//////////////////////////////////////////////////
//  Copyright (c) 2020 Nara Hiero
//
// This file is licensed under GPLv3+
// Refer to the `License.txt` file included.
//////////////////////////////////////////////////

#include <CTLib/Ext/MDL0.hpp>

#include <CTLib/Utilities.hpp>

namespace CTLib
{

namespace Ext
{

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
    return (static_cast<uint32_t>(stages[idx].getColourOpConstantSource()) <<  4)
        |  (static_cast<uint32_t>(stages[idx].getAlphaOpConstantSource() ) <<  9)
        | (!two ? 0 :
              (static_cast<uint32_t>(stages[idx + 1].getColourOpConstantSource()) << 14)
            | (static_cast<uint32_t>(stages[idx + 1].getAlphaOpConstantSource() ) << 19)
        );
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
    CMD_NOOP(0x26) /* temp space filling */

Buffer ShaderCode::toStandardLayout() const
{
    Buffer gcode(0x1D0);

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
    colourCSrc{ColourConstant::MaterialConstColour0_RGB},
    alphaCSrc{AlphaConstant::MaterialConstColour0_Alpha}
{

}

void ShaderCode::Stage::setColourOpConstantSource(ColourConstant source)
{
    colourCSrc = source;
}

void ShaderCode::Stage::setAlphaOpConstantSource(AlphaConstant source)
{
    alphaCSrc = source;
}

ShaderCode::Stage::ColourConstant ShaderCode::Stage::getColourOpConstantSource() const
{
    return colourCSrc;
}

ShaderCode::Stage::AlphaConstant ShaderCode::Stage::getAlphaOpConstantSource() const
{
    return alphaCSrc;
}
}
}
