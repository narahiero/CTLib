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

constexpr uint32_t BP_MASK_ALL = 0xFFFFFF;

void initBP(uint32_t* bp)
{
    for (uint32_t i = 0; i < 0x100; ++i)
    {
        bp[i] = 0;
    }

    bp[WGCode::BP_MASK] = BP_MASK_ALL;
}

void WGCode::readBP(Buffer& gcode, uint32_t* bp)
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
        uint32_t addr = pack >> 24;
        uint32_t value = pack & 0x00FFFFFF;

        uint32_t mask = bp[BP_MASK];
        bp[BP_MASK] = BP_MASK_ALL;

        bp[addr] = (value & mask) | (bp[addr] & ~mask);
    }
}
}
