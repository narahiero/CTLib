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

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////
////   Graphics code utilities
////

////////////////////////////////////////
///  Read

// Command Processor

inline void fromCPVertexMode(ObjectCode& obj, uint32_t value)
{
    obj.setVertexMode(static_cast<ObjectCode::Mode>((value >>  9) & 0x3));
    obj.setNormalMode(static_cast<ObjectCode::Mode>((value >> 11) & 0x3));

    for (uint32_t i = 0; i < 2; ++i)
    {
        obj.setColourMode(i, static_cast<ObjectCode::Mode>((value >> (13 + (i << 1))) & 0x3));
    }
}

inline void fromCPTexCoordMode(ObjectCode& obj, uint32_t value)
{
    for (uint32_t i = 0; i < 8; ++i)
    {
        obj.setTexCoordMode(i, static_cast<ObjectCode::Mode>((value >> (i << 1)) & 0x3));
    }
}

inline void fromCPDataFormat0(ObjectCode& obj, uint32_t value)
{
    obj.setVertexElements(static_cast<ObjectCode::VertexElements>(value & 0x1));
    obj.setVertexType(static_cast<ObjectCode::Type>((value >> 1) & 0x7));
    obj.setVertexDivisor(static_cast<uint8_t>((value >> 4) & 0x1F));

    obj.setNormalElements(static_cast<ObjectCode::NormalElements>((value >> 9) & 0x1));
    obj.setNormalType(static_cast<ObjectCode::Type>((value >> 10) & 0x7));

    for (uint32_t i = 0; i < 2; ++i)
    {
        obj.setColourType(i, static_cast<ObjectCode::ColourType>((value >> (14 + (i << 2))) & 0x7));
    }

    obj.setTexCoordElements(0, static_cast<ObjectCode::TexCoordElements>((value >> 21) & 0x1));
    obj.setTexCoordType(0, static_cast<ObjectCode::Type>((value >> 22) & 0x7));
    obj.setTexCoordDivisor(0, static_cast<uint8_t>((value >> 25) & 0x1F));
}

inline void fromCPDataFormat1(ObjectCode& obj, uint32_t value)
{
    for (uint32_t i = 1; i < 4; ++i)
    {
        obj.setTexCoordElements(i, static_cast<ObjectCode::TexCoordElements>((value >> (i * 9)) & 0x1));
        obj.setTexCoordType(i, static_cast<ObjectCode::Type>((value >> (1 + (i * 9))) & 0x7));
        obj.setTexCoordDivisor(i, static_cast<uint8_t>((value >> (4 + (i * 9))) & 0x1F));
    }

    obj.setTexCoordElements(4, static_cast<ObjectCode::TexCoordElements>((value >> 27) & 0x1));
    obj.setTexCoordType(4, static_cast<ObjectCode::Type>((value >> 28) & 0x7));
}

inline void fromCPDataFormat2(ObjectCode& obj, uint32_t value)
{
    obj.setTexCoordDivisor(4, static_cast<uint8_t>(value & 0x1F));

    for (uint32_t i = 5; i < 8; ++i)
    {
        obj.setTexCoordElements(i, static_cast<ObjectCode::TexCoordElements>((value >> (5 + (i * 9))) & 0x1));
        obj.setTexCoordType(i, static_cast<ObjectCode::Type>((value >> (6 + (i * 9))) & 0x7));
        obj.setTexCoordDivisor(i, static_cast<uint8_t>((value >> (9 + (i * 9))) & 0x1F));
    }
}

// Blitting Processor

inline void fromBPConstAlpha(MaterialCode& mat, uint64_t value)
{
    mat.setUseConstAlpha((value >> 8) & 1);
    mat.setConstAlpha(value & 0xFF);
}

inline MaterialCode::Colour fromBPColour(uint32_t ar, uint32_t gb)
{
    return { (ar & 0x7FF), ((gb >> 12) & 0x7FF), (gb & 0x7FF), ((ar >> 12) & 0x7FF) };
}

inline void fromBPColourRegisters(MaterialCode& mat, uint32_t index, uint64_t ar, uint64_t gb)
{
    mat.setConstColour(index, fromBPColour((uint32_t)(ar >> 32), (uint32_t)(gb >> 32)));
}

////////////////////////////////////////
///  Write

// General

inline void fillNOOPs(Buffer& gcode, uint32_t count)
{
    for (uint32_t i = 0; i < count; ++i)
    {
        gcode.put(0x00);
    }
}

// Command Processor

inline void putCPCommand(Buffer& gcode, uint8_t address, uint32_t value)
{
    gcode.put(0x08).put(address).putInt(value);
}

inline uint32_t toCPVertexMode(const ObjectCode& obj)
{
    return ((static_cast<uint32_t>(obj.getVertexMode()) & 0x3) << 9)
        | ((static_cast<uint32_t>(obj.getNormalMode()) & 0x3) << 11)
        | ((static_cast<uint32_t>(obj.getColourMode(0)) & 0x3) << 13)
        | ((static_cast<uint32_t>(obj.getColourMode(1)) & 0x3) << 15);
}

inline uint32_t toCPTexCoordMode(const ObjectCode& obj)
{
    return (static_cast<uint32_t>(obj.getTexCoordMode(0)) & 0x3)
        | ((static_cast<uint32_t>(obj.getTexCoordMode(1)) & 0x3) <<  2)
        | ((static_cast<uint32_t>(obj.getTexCoordMode(2)) & 0x3) <<  4)
        | ((static_cast<uint32_t>(obj.getTexCoordMode(3)) & 0x3) <<  6)
        | ((static_cast<uint32_t>(obj.getTexCoordMode(4)) & 0x3) <<  8)
        | ((static_cast<uint32_t>(obj.getTexCoordMode(5)) & 0x3) << 10)
        | ((static_cast<uint32_t>(obj.getTexCoordMode(6)) & 0x3) << 12)
        | ((static_cast<uint32_t>(obj.getTexCoordMode(7)) & 0x3) << 14);
}

inline uint32_t toCPDataFormat0(const ObjectCode& obj)
{
    return (static_cast<uint32_t>(obj.getVertexElements()) & 0x1)
        | ((static_cast<uint32_t>(obj.getVertexType()) & 0x7) << 1)
        | ((obj.getVertexDivisor() & 0x1F) << 4)
        | ((static_cast<uint32_t>(obj.getNormalElements()) & 0x1) << 9)
        | ((static_cast<uint32_t>(obj.getNormalType()) & 0x7) << 10)
        | ((static_cast<uint32_t>(obj.getColourType(0)) >= 3) << 13)
        | ((static_cast<uint32_t>(obj.getColourType(0)) & 0x7) << 14)
        | ((static_cast<uint32_t>(obj.getColourType(1)) >= 3) << 17)
        | ((static_cast<uint32_t>(obj.getColourType(1)) & 0x7) << 18)
        | ((static_cast<uint32_t>(obj.getTexCoordElements(0)) & 0x1) << 21)
        | ((static_cast<uint32_t>(obj.getTexCoordType(0)) & 0x7) << 22)
        | ((obj.getTexCoordDivisor(0) & 0x1F) << 25)
        | 0x40000000; // Dequant = true, NormI3 = false
}

inline uint32_t toCPDataFormat1(const ObjectCode& obj)
{
    return (static_cast<uint32_t>(obj.getTexCoordElements(1)) & 0x1)
        | ((static_cast<uint32_t>(obj.getTexCoordType(1)) & 0x7) << 1)
        | ((obj.getTexCoordDivisor(1) & 0x1F) << 4)
        | ((static_cast<uint32_t>(obj.getTexCoordElements(2)) & 0x1) << 9)
        | ((static_cast<uint32_t>(obj.getTexCoordType(2)) & 0x7) << 10)
        | ((obj.getTexCoordDivisor(2) & 0x1F) << 13)
        | ((static_cast<uint32_t>(obj.getTexCoordElements(3)) & 0x1) << 18)
        | ((static_cast<uint32_t>(obj.getTexCoordType(3)) & 0x7) << 19)
        | ((obj.getTexCoordDivisor(3) & 0x1F) << 22)
        | ((static_cast<uint32_t>(obj.getTexCoordElements(4)) & 0x1) << 27)
        | ((static_cast<uint32_t>(obj.getTexCoordType(4)) & 0x7) << 28);
}

inline uint32_t toCPDataFormat2(const ObjectCode& obj)
{
    return (obj.getTexCoordDivisor(4) & 0x1F)
        | ((static_cast<uint32_t>(obj.getTexCoordElements(5)) & 0x1) << 5)
        | ((static_cast<uint32_t>(obj.getTexCoordType(5)) & 0x7) << 6)
        | ((obj.getTexCoordDivisor(5) & 0x1F) << 9)
        | ((static_cast<uint32_t>(obj.getTexCoordElements(6)) & 0x1) << 14)
        | ((static_cast<uint32_t>(obj.getTexCoordType(6)) & 0x7) << 15)
        | ((obj.getTexCoordDivisor(6) & 0x1F) << 18)
        | ((static_cast<uint32_t>(obj.getTexCoordElements(7)) & 0x1) << 23)
        | ((static_cast<uint32_t>(obj.getTexCoordType(7)) & 0x7) << 24)
        | ((obj.getTexCoordDivisor(7) & 0x1F) << 27);
}

// Transform Unit

inline void putXFCommand(Buffer& gcode, uint16_t address, uint32_t value)
{
    gcode.put(0x10).putShort(0x0000).putShort(address).putInt(value);
}

inline uint32_t toXFUnitSize(const ObjectCode& obj)
{
    uint32_t colourCount = !!static_cast<uint32_t>(obj.getColourMode(0))
        + !!static_cast<uint32_t>(obj.getColourMode(1));

    uint32_t normalCount = obj.getNormalMode() == ObjectCode::Mode::None ? 0
        : obj.getNormalElements() == ObjectCode::NormalElements::Normal ? 1 : 2;

    uint32_t texCoordCount = !!static_cast<uint32_t>(obj.getTexCoordMode(0))
        + !!static_cast<uint32_t>(obj.getTexCoordMode(1))
        + !!static_cast<uint32_t>(obj.getTexCoordMode(2))
        + !!static_cast<uint32_t>(obj.getTexCoordMode(3))
        + !!static_cast<uint32_t>(obj.getTexCoordMode(4))
        + !!static_cast<uint32_t>(obj.getTexCoordMode(5))
        + !!static_cast<uint32_t>(obj.getTexCoordMode(6))
        + !!static_cast<uint32_t>(obj.getTexCoordMode(7));

    return (colourCount & 0x3) | ((normalCount & 0x3) << 2) | ((texCoordCount & 0xF) << 4);
}

// Blitting Processor

inline void putBPCommand(Buffer& gcode, uint8_t address, uint32_t value)
{
    gcode.put(0x61).putInt((address << 24) | (value & 0x00FFFFFF));
}

inline void putBPMask(Buffer& gcode, uint32_t mask)
{
    putBPCommand(gcode, 0xFE, mask);
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

    WGCode::Context c;
    WGCode::readGraphicsCode(gcode, &c, true, WGCode::FLAG_USE_BP | WGCode::FLAG_USE_XF);

    fromBPConstAlpha(mat, c.bp[Ext::WGCode::BP_CONST_ALPHA]);

    for (uint32_t i = 0; i < CONST_COLOUR_COUNT; ++i)
    {
        fromBPColourRegisters(mat, i, c.bp[Ext::WGCode::BP_MATERIAL_COLOUR + (i << 1)],
            c.bp[Ext::WGCode::BP_MATERIAL_COLOUR + 1 + (i << 1)]);
    }

    return mat;
}

Buffer MaterialCode::toStandardLayout() const
{
    Buffer gcode(0x180);

    putBPCommand(gcode, 0xF3, 0x3F0000); // temporarily hard-code alpha func
    putBPCommand(gcode, 0x40, 0x000017); // temporarily hard-code Z-mode
    putBPMask(gcode, 0x00FFE3); // blend mode mask
    putBPCommand(gcode, 0x41, 0x0034A0); // temporarily hard-code blend mode
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

    // indirect textures scale
    putBPCommand(gcode, 0x25, 0x000000);
    putBPCommand(gcode, 0x26, 0x000000);

    fillNOOPs(gcode, 0x36); // padding

    fillNOOPs(gcode, 0xA0); // temporary space fill

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


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////
////   ObjectCode class
////

ObjectCode ObjectCode::fromGraphicsCode(Buffer& gcode)
{
    ObjectCode obj;

    WGCode::Context c;
    WGCode::readGraphicsCode(gcode, &c, true, WGCode::FLAG_USE_CP | WGCode::FLAG_USE_XF);

    fromCPVertexMode(obj, c.cp[WGCode::CP_VERTEX_MODE]);
    fromCPTexCoordMode(obj, c.cp[WGCode::CP_TEX_COORD_MODE]);

    fromCPDataFormat0(obj, c.cp[WGCode::CP_DATA_FORMAT0]);
    fromCPDataFormat1(obj, c.cp[WGCode::CP_DATA_FORMAT1]);
    fromCPDataFormat2(obj, c.cp[WGCode::CP_DATA_FORMAT2]);

    return obj;
}

Buffer ObjectCode::toStandardLayout() const
{
    Buffer gcode(0xE0);

    fillNOOPs(gcode, 0x2); // padding

    putCPCommand(gcode, 0x50, toCPVertexMode(*this));
    putCPCommand(gcode, 0x60, toCPTexCoordMode(*this));

    putXFCommand(gcode, 0x1008, toXFUnitSize(*this));

    fillNOOPs(gcode, 0x1); // padding

    putCPCommand(gcode, 0x70, toCPDataFormat0(*this));
    putCPCommand(gcode, 0x80, toCPDataFormat1(*this));
    putCPCommand(gcode, 0x90, toCPDataFormat2(*this));

    fillNOOPs(gcode, 0xB6);

    return gcode.clear();
}

ObjectCode::ObjectCode() :
    vertexMode{Mode::Indexed8},
    vertexType{Type::Float},
    vertexElements{VertexElements::XYZ},
    vertexDivisor{0},
    normalMode{Mode::Indexed8},
    normalType{Type::Float},
    normalElements{NormalElements::Normal}
{
    for (uint32_t i = 0; i < MDL0::Object::COLOUR_ARRAY_COUNT; ++i)
    {
        colourModes[i] = i == 0 ? Mode::Indexed8 : Mode::None;
        colourTypes[i] = ColourType::RGBA8;
    }

    for (uint32_t i = 0; i < MDL0::Object::TEX_COORD_ARRAY_COUNT; ++i)
    {
        texCoordModes[i] = i == 0 ? Mode::Indexed8 : Mode::None;
        texCoordTypes[i] = Type::Float;
        texCoordElements[i] = TexCoordElements::ST;
        texCoordDivisors[i] = 0;
    }
}

inline ObjectCode::Mode modeForIndexSize(uint8_t size)
{
    return size == 2 ? ObjectCode::Mode::Indexed16 : ObjectCode::Mode::Indexed8;
}

inline ObjectCode::NormalElements elemsForComponents(MDL0::NormalArray::Components comps)
{
    return comps == MDL0::NormalArray::Components::Normal_BiNormal_Tangent
        ? ObjectCode::NormalElements::NBT : ObjectCode::NormalElements::Normal;
}

void ObjectCode::configureFromMDL0Object(MDL0::Object* obj)
{
    if (obj == nullptr)
    {
        return;
    }

    MDL0::VertexArray* va = obj->getVertexArray();
    if (va != nullptr)
    {
        vertexMode = modeForIndexSize(obj->getVertexArrayIndexSize());
        vertexType = static_cast<Type>(va->getFormat());
        vertexElements = static_cast<VertexElements>(va->getComponentsType());
        vertexDivisor = va->getDivisor();
    }
    else
    {
        vertexMode = Mode::None;
    }

    MDL0::NormalArray* na = obj->getNormalArray();
    if (na != nullptr)
    {
        normalMode = modeForIndexSize(obj->getNormalArrayIndexSize());
        normalType = static_cast<Type>(na->getFormat());
        normalElements = elemsForComponents(na->getComponentsType());
    }
    else
    {
        normalMode = Mode::None;
    }

    for (uint32_t i = 0; i < MDL0::Object::COLOUR_ARRAY_COUNT; ++i)
    {
        MDL0::ColourArray* ca = obj->getColourArray(i);
        if (ca != nullptr)
        {
            colourModes[i] = modeForIndexSize(obj->getColourArrayIndexSize(i));
            colourTypes[i] = static_cast<ColourType>(ca->getFormat());
        }
        else
        {
            colourModes[i] = Mode::None;
        }
    }

    for (uint32_t i = 0; i < MDL0::Object::TEX_COORD_ARRAY_COUNT; ++i)
    {
        MDL0::TexCoordArray* tca = obj->getTexCoordArray(i);
        if (tca != nullptr)
        {
            texCoordModes[i] = modeForIndexSize(obj->getTexCoordArrayIndexSize(i));
            texCoordTypes[i] = static_cast<Type>(tca->getFormat());
            texCoordElements[i] = static_cast<TexCoordElements>(tca->getComponentsType());
            texCoordDivisors[i] = tca->getDivisor();
        }
        else
        {
            texCoordModes[i] = Mode::None;
        }
    }
}

void ObjectCode::setVertexMode(Mode mode)
{
    vertexMode = mode;
}

void ObjectCode::setVertexType(Type type)
{
    vertexType = type;
}

void ObjectCode::setVertexElements(VertexElements elems)
{
    vertexElements = elems;
}

void ObjectCode::setVertexDivisor(uint8_t divisor)
{
    vertexDivisor = divisor;
}

ObjectCode::Mode ObjectCode::getVertexMode() const
{
    return vertexMode;
}

ObjectCode::Type ObjectCode::getVertexType() const
{
    return vertexType;
}

ObjectCode::VertexElements ObjectCode::getVertexElements() const
{
    return vertexElements;
}

uint8_t ObjectCode::getVertexDivisor() const
{
    return vertexDivisor;
}

void ObjectCode::setNormalMode(Mode mode)
{
    normalMode = mode;
}

void ObjectCode::setNormalType(Type type)
{
    normalType = type;
}

void ObjectCode::setNormalElements(NormalElements elems)
{
    normalElements = elems;
}

ObjectCode::Mode ObjectCode::getNormalMode() const
{
    return normalMode;
}

ObjectCode::Type ObjectCode::getNormalType() const
{
    return normalType;
}

ObjectCode::NormalElements ObjectCode::getNormalElements() const
{
    return normalElements;
}

void ObjectCode::setColourMode(uint32_t index, Mode mode)
{
    assertValidColourIndex(index);
    colourModes[index] = mode;
}

void ObjectCode::setColourType(uint32_t index, ColourType type)
{
    assertValidColourIndex(index);
    colourTypes[index] = type;
}

ObjectCode::Mode ObjectCode::getColourMode(uint32_t index) const
{
    assertValidColourIndex(index);
    return colourModes[index];
}

ObjectCode::ColourType ObjectCode::getColourType(uint32_t index) const
{
    assertValidColourIndex(index);
    return colourTypes[index];
}

void ObjectCode::setTexCoordMode(uint32_t index, Mode mode)
{
    assertValidTexCoordIndex(index);
    texCoordModes[index] = mode;
}

void ObjectCode::setTexCoordType(uint32_t index, Type type)
{
    assertValidTexCoordIndex(index);
    texCoordTypes[index] = type;
}

void ObjectCode::setTexCoordElements(uint32_t index, TexCoordElements elems)
{
    assertValidTexCoordIndex(index);
    texCoordElements[index] = elems;
}

void ObjectCode::setTexCoordDivisor(uint32_t index, uint8_t divisor)
{
    assertValidTexCoordIndex(index);
    texCoordDivisors[index] = divisor;
}

ObjectCode::Mode ObjectCode::getTexCoordMode(uint32_t index) const
{
    assertValidTexCoordIndex(index);
    return texCoordModes[index];
}

ObjectCode::Type ObjectCode::getTexCoordType(uint32_t index) const
{
    assertValidTexCoordIndex(index);
    return texCoordTypes[index];
}

ObjectCode::TexCoordElements ObjectCode::getTexCoordElements(uint32_t index) const
{
    assertValidTexCoordIndex(index);
    return texCoordElements[index];
}

uint8_t ObjectCode::getTexCoordDivisor(uint32_t index) const
{
    assertValidTexCoordIndex(index);
    return texCoordDivisors[index];
}

void ObjectCode::assertValidColourIndex(uint32_t index) const
{
    if (index >= MDL0::Object::COLOUR_ARRAY_COUNT)
    {
        throw BRRESError(Strings::format(
            "ObjectCode: Colour index out of range! (%d >= %d)",
            index, MDL0::Object::COLOUR_ARRAY_COUNT
        ));
    }
}

void ObjectCode::assertValidTexCoordIndex(uint32_t index) const
{
    if (index >= MDL0::Object::TEX_COORD_ARRAY_COUNT)
    {
        throw BRRESError(Strings::format(
            "ObjectCode: Texture coordinate index out of range! (%d >= %d)",
            index, MDL0::Object::TEX_COORD_ARRAY_COUNT
        ));
    }
}
}
