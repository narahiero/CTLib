//////////////////////////////////////////////////
//  Copyright (c) 2020 Nara Hiero
//
// This file is licensed under GPLv3+
// Refer to the `License.txt` file included.
//////////////////////////////////////////////////

#include <CTLib/Ext/WGCode.hpp>

#include <CTLib/Utilities.hpp>

namespace CTLib::Ext
{

constexpr uint64_t BP_MASK_ALL = 0xFFFFFF;

void initBP(uint64_t* bp)
{
    for (uint32_t i = 0; i < 0x100; ++i)
    {
        bp[i] = 0;
    }

    bp[WGCode::BP_MASK] = BP_MASK_ALL;
}

void WGCode::readBP(Buffer& gcode, uint64_t* bp)
{
    initBP(bp);

    while (gcode.hasRemaining())
    {
        uint8_t cmd = gcode.get();

        if (cmd == CMD_NOOP)
        {
            continue;
        }
        else if (cmd != CMD_LOAD_BP)
        {
            throw BRRESError(Strings::format(
                "WGCode: Non-BP command found while reading BP command buffer! (0x%02X)",
                cmd
            ));
        }

        if (gcode.remaining() < 4)
        {
            throw BRRESError(
                "WGCode: Invalid graphics code! Not enough bytes left in buffer for BP command!"
            );
        }

        uint32_t pack = gcode.getInt();
        uint8_t addr = static_cast<uint8_t>(pack >> 24);
        uint64_t value = pack & 0x00FFFFFF;

        uint64_t mask = bp[BP_MASK];
        bp[BP_MASK] = BP_MASK_ALL;

        if (addr >= BP_MATERIAL_COLOUR && addr < BP_MATERIAL_COLOUR + 8)
        {
            bool isconst = (value >> 23) & 1;
            value &= ~(1Ui64 << 23);
            bp[addr] = ((0xFFFFFFUi64 << (isconst ? 0 : 32)) & bp[addr]) | (value << (isconst ? 32 : 0));
        }
        else
        {
            bp[addr] = (value & mask) | (bp[addr] & ~mask);
        }
    }
}
}
