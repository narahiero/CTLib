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

void initCP(uint32_t* cp)
{
    for (uint32_t i = 0; i < WGCode::CP_REG_COUNT; ++i)
    {
        cp[i] = 0;
    }
}

void initXF(uint32_t* xf)
{
    for (uint32_t i = 0; i < WGCode::XF_REG_COUNT; ++i)
    {
        xf[i] = 0;
    }
}

void initBP(uint64_t* bp)
{
    for (uint32_t i = 0; i < WGCode::BP_REG_COUNT; ++i)
    {
        bp[i] = 0;
    }

    bp[WGCode::BP_MASK] = BP_MASK_ALL;
}

void WGCode::initContext(Context* c)
{
    initCP(c->cp);
    initXF(c->xf);
    initBP(c->bp);
}

void WGCode::readGraphicsCode(Buffer& gcode, Context* c, bool reset, uint32_t flags)
{
    if (reset)
    {
        initContext(c);
    }

    while (gcode.hasRemaining())
    {
        uint8_t cmd = gcode.get();
        switch (cmd)
        {
        case CMD_NOOP:
            break;

        case CMD_LOAD_CP:
            if (!(flags & FLAG_USE_CP))
            {
                throw BRRESError("WGCode: Load CP command found, but CP is disabled!");
            }

            if (gcode.remaining() < 5)
            {
                throw BRRESError(Strings::format(
                    "WGCode: Not enough bytes remaining in buffer for CP command! (%d < 5)",
                    gcode.remaining()
                ));
            }

            {
                uint8_t address = gcode.get();
                uint32_t value = gcode.getInt();

                handleCPCommand(c, address, value);
            }
            break;

        case CMD_LOAD_XF:
            if (!(flags & FLAG_USE_XF))
            {
                throw BRRESError("WGCode: Load XF command found, but XF is disabled!");
            }

            if (gcode.remaining() < 4)
            {
                throw BRRESError(Strings::format(
                    "WGCode: Not enough bytes remaining in buffer for XF command head! (%d < 4)",
                    gcode.remaining()
                ));
            }

            {
                uint16_t transferSize = gcode.getShort();
                uint16_t address = gcode.getShort();

                if (gcode.remaining() < (transferSize + 1) * 4)
                {
                    throw BRRESError(Strings::format(
                        "WGCode: Not enough bytes remaining in buffer for XF command body! (%d < %d)",
                        gcode.remaining(), (transferSize + 1) * 4
                    ));
                }

                // move the buffer's position to the end of this command
                Buffer data = gcode.duplicate();
                for (uint16_t i = 0; i < transferSize + 1; ++i)
                {
                    gcode.getInt();
                }

                if (address + transferSize > XF_REG_COUNT)
                {
                    if (address > XF_REG_COUNT) // nothing to write
                    {
                        break;
                    }
                    transferSize = XF_REG_COUNT - address;
                }

                handleXFCommand(c, address, transferSize + 1, data);
            }
            break;

        case CMD_LOAD_BP:
            if (!(flags & FLAG_USE_BP))
            {
                throw BRRESError("WGCode: Load BP command found, but BP is disabled!");
            }

            if (gcode.remaining() < 4)
            {
                throw BRRESError(Strings::format(
                    "WGCode: Not enough bytes remaining in buffer for BP command! (%d < 4)",
                    gcode.remaining()
                ));
            }

            {
                uint32_t pack = gcode.getInt();
                uint8_t address = static_cast<uint8_t>(pack >> 24);
                uint64_t value = pack & 0xFFFFFF;

                handleBPCommand(c, address, value);
            }
            break;

        default:
            throw BRRESError(Strings::format(
                "WGCode: Unknown command found in buffer! (0x%02X)",
                cmd
            ));
        }
    }
}

void WGCode::handleCPCommand(Context* c, uint8_t addr, uint32_t value)
{
    c->cp[addr] = value;
}

void WGCode::handleXFCommand(Context* c, uint16_t addr, uint16_t transferSize, Buffer& gcode)
{
    for (uint16_t i = 0; i < transferSize; ++i)
    {
        c->xf[addr + i] = gcode.getInt();
    }
}

void WGCode::handleBPCommand(Context* c, uint8_t addr, uint64_t value)
{
    uint64_t mask = c->bp[BP_MASK];
    c->bp[BP_MASK] = BP_MASK_ALL;

    if (addr >= BP_MATERIAL_COLOUR && addr < BP_MATERIAL_COLOUR + 8)
    {
        bool isconst = (value >> 23) & 1;
        value &= ~(1Ui64 << 23);
        c->bp[addr] = ((0xFFFFFFUi64 << (isconst ? 0 : 32)) & c->bp[addr]) | (value << (isconst ? 32 : 0));
    }
    else
    {
        c->bp[addr] = (value & mask) | (c->bp[addr] & ~mask);
    }
}
}
