//////////////////////////////////////////////////
//  Copyright (c) 2020 Nara Hiero
//
// This file is licensed under GPLv3+
// Refer to the `License.txt` file included.
//////////////////////////////////////////////////

#include <gtest/gtest.h>

#include <CTLib/Ext/WGCode.hpp>

using namespace CTLib;

TEST(WGCodeTests, BPRead)
{
    uint8_t gcodeRaw[] = {
        0x61, 0x23, 0x43, 0x54, 0xF4,
        0x61, 0x54, 0x77, 0xD0, 0x3F,
        0x61, 0x03, 0x23, 0xF2, 0x5D,
        0x00, 0x00,
        0x61, 0xEE, 0xDC, 0x4F, 0x61,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x61, 0x7C, 0x43, 0xCC, 0xAB,
        0x00,
        0x61, 0x3D, 0x00, 0xFF, 0xFF
    };
    Buffer gcode(sizeof(gcodeRaw) / sizeof(gcodeRaw[0]));
    gcode.putArray(gcodeRaw, gcode.capacity()).flip();

    uint64_t bp[0x100];
    Ext::WGCode::readBP(gcode, bp);

    for (uint32_t addr = 0; addr < 0x100; ++addr)
    {
        if (addr == 0x03)
        {
            EXPECT_EQ(bp[addr], 0x23F25D);
        }
        else if (addr == 0x23)
        {
            EXPECT_EQ(bp[addr], 0x4354F4);
        }
        else if (addr == 0x3D)
        {
            EXPECT_EQ(bp[addr], 0x00FFFF);
        }
        else if (addr == 0x54)
        {
            EXPECT_EQ(bp[addr], 0x77D03F);
        }
        else if (addr == 0x7C)
        {
            EXPECT_EQ(bp[addr], 0x43CCAB);
        }
        else if (addr == 0xEE)
        {
            EXPECT_EQ(bp[addr], 0xDC4F61);
        }
        else if (addr == 0xFE) // notice here that address 0xFE was not set
        {
            EXPECT_EQ(bp[addr], 0xFFFFFF); // mask is 0xFFFFFF by default
        }
        else
        {
            EXPECT_EQ(bp[addr], 0x000000);
        }
    }
}

TEST(WGCodeTests, BPMask)
{
    uint8_t gcodeRaw[] = {
        0x00,
        0x61, 0x73, 0xF8, 0x01, 0xC2,
        0x61, 0xFE, 0xFF, 0xF0, 0x0F, // <- mask set here
        0x00, 0x00,
        0x61, 0x23, 0x54, 0x18, 0xD1,
        0x61, 0xF4, 0x0C, 0x44, 0x7A,
        0x61, 0xFE, 0x0F, 0x3F, 0xFC, // <- mask set here
        0x61, 0x2F, 0x45, 0x28, 0x12,
        0x00, 0x00, 0x00, 0x00,
        0x61, 0xC8, 0x5D, 0x18, 0xDC,
        0x61, 0xFE, 0xFE, 0x01, 0xF8, // <- mask set here
        0x61, 0xC8, 0x20, 0xF1, 0x22
    };
    Buffer gcode(sizeof(gcodeRaw) / sizeof(gcodeRaw[0]));
    gcode.putArray(gcodeRaw, gcode.capacity()).flip();

    uint64_t bp[0x100];
    Ext::WGCode::readBP(gcode, bp);

    for (uint32_t addr = 0; addr < 0x100; ++addr)
    {
        if (addr == 0x23)
        {
            EXPECT_EQ(bp[addr], 0x541001); // 0x5418D1 without mask
        }
        else if (addr == 0x2F)
        {
            EXPECT_EQ(bp[addr], 0x052810); // 0x452812 without mask
        }
        else if (addr == 0x73)
        {
            EXPECT_EQ(bp[addr], 0xF801C2);
        }
        else if (addr == 0xC8)
        {
            EXPECT_EQ(bp[addr], 0x211924); // write with mask over exisiting value
        }
        else if (addr == 0xF4)
        {
            EXPECT_EQ(bp[addr], 0x0C447A);
        }
        else if (addr == 0xFE)
        {
            EXPECT_EQ(bp[addr], 0xFFFFFF); // mask is set only for next write
        }
        else
        {
            EXPECT_EQ(bp[addr], 0x000000);
        }
    }
}

TEST(WGCodeTests, BPMaskLast)
{
    uint8_t gcodeRaw[] = {
        0x61, 0xFE, 0x05, 0x32, 0x12,
        0x00, 0x00, 0x00
    };
    Buffer gcode(sizeof(gcodeRaw) / sizeof(gcodeRaw[0]));
    gcode.putArray(gcodeRaw, gcode.capacity()).flip();

    uint64_t bp[0x100];
    Ext::WGCode::readBP(gcode, bp);

    EXPECT_EQ(bp[0xFE], 0x053212); // mask not reset since no write followed
}

TEST(WGCodeTests, ColourRegisters)
{
    uint8_t gcodeRaw[] = {
        0x00,
        0x61, 0xE2, 0x80, 0x34, 0x12, // <- const colour set
        0x00, 0x00, 0x00,
        0x61, 0xE4, 0x52, 0x12, 0xF4, // <- colour set
        0x61, 0xE1, 0x9D, 0xE4, 0x00, // <- const colour set
        0x00, 0x00,
        0x61, 0xE2, 0x73, 0x0D, 0xAA, // <- colour set
        0x00
    };
    Buffer gcode(sizeof(gcodeRaw) / sizeof(gcodeRaw[0]));
    gcode.putArray(gcodeRaw, gcode.capacity()).flip();

    uint64_t bp[0x100];
    Ext::WGCode::readBP(gcode, bp);

    for (uint32_t addr = 0xE0; addr < 0xE8; ++addr)
    {
        if (addr == 0xE1)
        {
            EXPECT_EQ(bp[addr], 0x1DE40000000000);
        }
        else if (addr == 0xE2)
        {
            EXPECT_EQ(bp[addr], 0x00341200730DAA);
        }
        else if (addr == 0xE4)
        {
            EXPECT_EQ(bp[addr], 0x5212F4);
        }
    }
}

TEST(WGCodeTests, BPReadErrors)
{
    uint8_t invalidCmd[] = {
        0x00, 0x00,
        0x61, 0x23, 0x02, 0x23, 0x4D,
        0x00, 0x00, 0x00,
        0x61, 0xFF, 0xD2, 0x74, 0xC1,
        0x08, // <- non-BP command
        0x61, 0x03, 0x93, 0xF2, 0x1A,
        0x00, 0x00
    };
    Buffer gcode(sizeof(invalidCmd) / sizeof(invalidCmd[0]));
    gcode.putArray(invalidCmd, gcode.capacity()).flip();

    uint64_t bp[0x100];
    EXPECT_THROW(Ext::WGCode::readBP(gcode, bp), BRRESError);

    ////////////////////////////////////

    uint8_t incompleteBPCmd[] = {
        0x61, 0x72, 0xF3, 0xD3, 0x89,
        0x00, 0x00,
        0x61, 0x1D, 0x3F, 0x12, 0xAA,
        0x61, 0x42, 0xD3, 0x73 // <- missing 1 byte
    };
    gcode = Buffer(sizeof(incompleteBPCmd) / sizeof(incompleteBPCmd[0]));
    gcode.putArray(incompleteBPCmd, gcode.capacity()).flip();

    // bp array already defined before
    EXPECT_THROW(Ext::WGCode::readBP(gcode, bp), BRRESError);
}
