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

void fromIndirectSources(ShaderCode& shader, uint32_t val)
{
    for (uint32_t i = 0; i < ShaderCode::INDIRECT_TEX_STAGE_COUNT; ++i)
    {
        shader.setTexMapID(i, (val >> (i * 6)) & 0x7);
        shader.setTexCoordIndex(i, (val >> (3 + (i * 6))) & 0x7);
    }
}

void fromSwapTable(ShaderCode& shader, uint32_t idx, bool rg, uint32_t val, uint32_t mask)
{
    ShaderCode::SwapTable table = shader.getSwapTable(idx);
    uint32_t old = rg ? (static_cast<uint32_t>(table.red) | (static_cast<uint32_t>(table.green) << 2))
        : (static_cast<uint32_t>(table.blue) | (static_cast<uint32_t>(table.alpha) << 2));

    uint32_t value = (val & mask) | (old & ~mask);
    (rg ? table.red : table.blue) = static_cast<ShaderCode::Channel>(value & 0x3);
    (rg ? table.green : table.alpha) = static_cast<ShaderCode::Channel>((value >> 2) & 0x3);
}

void fromStageConstants(ShaderCode::Stage& stage, bool n2, uint32_t val, uint32_t mask)
{
    uint32_t old = (static_cast<uint32_t>(stage.getColourOpConstantSource()) << (n2 ? 12 : 2))
        | (static_cast<uint32_t>(stage.getAlphaOpConstantSource()) << (n2 ? 17 : 7));

    uint32_t value = (val & mask) | (old & ~mask);

    stage.setColourOpConstantSource(
        static_cast<ShaderCode::Stage::ColourConstant>((val >> (n2 ? 12 : 2)) & 0x1F));

    stage.setAlphaOpConstantSource(
        static_cast<ShaderCode::Stage::AlphaConstant>((val >> (n2 ? 17 : 7)) & 0x1F));
}

void fromStageSources(ShaderCode::Stage& stage, uint32_t val)
{
    stage.setTexMapID(val & 0x7);
    stage.setTexCoordIndex((val >> 3) & 0x7);
    stage.setUsesTexture((val >> 6) & 0x1);
    stage.setRasterColour(static_cast<ShaderCode::Stage::RasterColour>((val >> 7) & 0x7));
}

ShaderCode::Stage::ColourOp toColourOp(uint32_t val)
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
    return cop;
}

ShaderCode::Stage::AlphaOp toAlphaOp(uint32_t val)
{
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
    return aop;
}

ShaderCode ShaderCode::fromGraphicsCode(Buffer& gcode, uint32_t stageCount)
{
    if (stageCount >= MAX_STAGE_COUNT)
    {
        throw BRRESError(Strings::format(
            "ShaderCode: Stage count out of range! (%d >= %d)",
            stageCount, MAX_STAGE_COUNT
        ));
    }

    ShaderCode shader;
    for (uint32_t i = 0; i < stageCount; ++i)
    {
        shader.addStage();
    }

    uint32_t mask;
    while (gcode.hasRemaining())
    {
        uint8_t cmd = gcode.get();

        if (cmd == 0x00)
        {
            continue;
        }
        else if (cmd != 0x61)
        {
            throw BRRESError(Strings::format(
                "ShaderCode: Invalid Wii Graphics Code! Non-BP (0x61) found! (0x%02X)",
                cmd
            ));
        }

        uint32_t pack = gcode.getInt();
        uint8_t addr = static_cast<uint8_t>(pack >> 24);
        uint32_t val = pack & mask;
        mask = 0x00FFFFFF;

        // predefined variables for switch
        uint32_t idx = 0;
        switch (addr)
        {
        case 0x27: // indirect sources

            break;

        case 0x28: // stage sources
        case 0x29:
        case 0x2A:
        case 0x2B:
            idx = (addr & 0x3) << 1;
            if (idx < stageCount) 
            {
                fromStageSources(shader.getStage(idx), val);
                if (idx + 1 < stageCount)
                {
                    fromStageSources(shader.getStage(idx + 1), val >> 12);
                }
            }
            break;

        case 0xC0: // colour operations
        case 0xC2:
        case 0xC4:
        case 0xC6:
        case 0xC8:
        case 0xCA:
        case 0xCC:
        case 0xCE:
            idx = (addr >> 1) & 0x7;
            if (idx < stageCount) 
            {
                shader.getStage(idx).setColourOp(toColourOp(val));
            }
            break;

        case 0xC1: // alpha operations
        case 0xC3:
        case 0xC5:
        case 0xC7:
        case 0xC9:
        case 0xCB:
        case 0xCD:
        case 0xCF:
            idx = (addr >> 1) & 0x7;
            if (idx < stageCount) 
            {
                shader.getStage(idx).setAlphaOp(toAlphaOp(val));
            }
            break;

        case 0xF6: // swap table & stage constant sources
        case 0xF7:
        case 0xF8:
        case 0xF9:
        case 0xFA:
        case 0xFB:
        case 0xFC:
        case 0xFD:
            if (addr <= 0xF9)
            {
                idx = ((addr - 0x06) & 0x03) << 1;
                if (idx < stageCount)
                {
                    fromStageConstants(shader.getStage(idx), false, val, mask);
                    if (idx + 1 < stageCount)
                    {
                        fromStageConstants(shader.getStage(idx + 1), true, val, mask);
                    }
                }
            }
            fromSwapTable(shader, ((addr - 0x06) & 0x0F) >> 1, addr & 1, val, mask);
            break;

        case 0xFE: // set mask
            mask = val;
            break;
        }
    }

    return shader;
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
    return (static_cast<uint32_t>(stages[idx].getColourOpConstantSource()) <<  4)
        |  (static_cast<uint32_t>(stages[idx].getAlphaOpConstantSource() ) <<  9)
        | (!two ? 0 :
              (static_cast<uint32_t>(stages[idx + 1].getColourOpConstantSource()) << 14)
            | (static_cast<uint32_t>(stages[idx + 1].getAlphaOpConstantSource() ) << 19)
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

inline uint32_t toBPAlphaOpValue(const ShaderCode::Stage::AlphaOp& op)
{
    return ((static_cast<uint32_t>(op.argD ) & 0x7) <<  4)
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
    BP_CMD(0xC1 + ((idx) << 1), toBPAlphaOpValue(stages[idx].alphaOp)); \
    if (two) BP_CMD(0xC3 + ((idx) << 1), toBPAlphaOpValue(stages[(idx) + 1].alphaOp)); \
    else CMD_NOOP(0x5); \
    BP_CMD(0x10 + (idx), 0x000000); \
    if (two) BP_CMD(0x11 + (idx), 0x000000); \
    else CMD_NOOP(0x5); \
    CMD_NOOP(0x3)

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
    useTexture{false},
    texMap{0},
    texCoord{0},
    rasterColour{RasterColour::LightChannel0},
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
    useTexture = true;
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
}
}
