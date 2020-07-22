//////////////////////////////////////////////////
//  Copyright (c) 2020 Nara Hiero
//
// This file is licensed under GPLv3+
// Refer to the `License.txt` file included.
//////////////////////////////////////////////////

#include <gtest/gtest.h>

#include <CTLib/Ext/WGCode.hpp>

using namespace CTLib;

TEST(WGCodeTests, ReadXF)
{
    uint8_t raw[] = {
        0x00, 0x00, // <- ignore noops
        0x10, 0x00, 0x01, 0x10, 0x50, 0xFD, 0x12, 0x4D, 0x3A, 0xD4, 0x4A, 0x49, 0x61,
        0x00, 0x00, 0x00,
        0x61, 0x23, 0x43, 0x12, 0x54, // <- ignore BP
        0x10, 0x00, 0x00, 0x0F, 0xD3, 0x24, 0xC0, 0x4A, 0xE3,
        0x00, 0x00, 0x00, 0x00,
        0x10, 0x00, 0x00, 0x10, 0x60, 0x12, 0x34, 0x56, 0x78, // <- ignore out of range address
    };
    Buffer gcode(sizeof(raw) / sizeof(raw[0]));
    gcode.putArray(raw, gcode.capacity()).flip();

    Ext::WGCode::Context c;
    Ext::WGCode::readGraphicsCode(gcode, &c, true);

    for (uint32_t i = 0; i < Ext::WGCode::XF_REG_COUNT; ++i)
    {
        if (i == 0x0FD3)
        {
            EXPECT_EQ(0x24C04AE3, c.xf[i]);
        }
        else if (i == 0x1050)
        {
            EXPECT_EQ(0xFD124D3A, c.xf[i]);
        }
        else if (i == 0x1051)
        {
            EXPECT_EQ(0xD44A4961, c.xf[i]);
        }
        else
        {
            EXPECT_EQ(0x00000000, c.xf[i]);
        }
    }
}

TEST(WGCodeTests, ReadBP)
{
    uint8_t raw[] = {
        0x10, 0x00, 0x00, 0x10, 0x20, 0x34, 0xD3, 0x1D, 0xF5, // ignore XF
        0x00, 0x00, 0x00, // ignore noops
        0x61, 0x9A, 0x24, 0x1D, 0xED,
        0x00,
        0x61, 0x34, 0xAA, 0x5C, 0x10,
        0x61, 0x22, 0x53, 0x09, 0x51
    };
    Buffer gcode(sizeof(raw) / sizeof(raw[0]));
    gcode.putArray(raw, gcode.capacity()).flip();

    Ext::WGCode::Context c;
    Ext::WGCode::readGraphicsCode(gcode, &c, true);

    for (uint32_t i = 0; i < Ext::WGCode::BP_REG_COUNT; ++i)
    {
        if (i == 0x22)
        {
            EXPECT_EQ(0x530951, c.bp[i]);
        }
        else if (i == 0x34)
        {
            EXPECT_EQ(0xAA5C10, c.bp[i]);
        }
        else if (i == 0x9A)
        {
            EXPECT_EQ(0x241DED, c.bp[i]);
        }
        else if (i == 0xFE) // BP mask
        {
            EXPECT_EQ(0xFFFFFF, c.bp[i]);
        }
        else
        {
            EXPECT_EQ(0x000000, c.bp[i]);
        }
    }
}

TEST(WGCodeTests, BPMask)
{
    uint8_t raw[] = {
        0x61, 0x49, 0xA2, 0x38, 0x02, // no mask by default
        0x61, 0xFE, 0xFF, 0x0F, 0xFE, // <- mask set here
        0x00, 0x00, // noops does not affect it
        0x10, 0x00, 0x00, 0x10, 0x40, 0x34, 0xA3, 0x92, 0x28, // nor does any non-BP commands
        0x61, 0x23, 0xAC, 0x42, 0x98, // will be masked
        0x61, 0xA2, 0x28, 0xCE, 0xE3, // will not be masked
        0x61, 0xFE, 0xF0, 0x0F, 0xFF, // mask will be set in context since no BP command follows
        0x00, 0x00, 0x00
    };
    Buffer gcode(sizeof(raw) / sizeof(raw[0]));
    gcode.putArray(raw, gcode.capacity()).flip();

    Ext::WGCode::Context c;
    Ext::WGCode::readGraphicsCode(gcode, &c, true);

    for (uint32_t i = 0; i < Ext::WGCode::BP_REG_COUNT; ++i)
    {
        if (i == 0x23)
        {
            EXPECT_EQ(0xAC0298, c.bp[i]);
        }
        else if (i == 0x49)
        {
            EXPECT_EQ(0xA23802, c.bp[i]);
        }
        else if (i == 0xA2)
        {
            EXPECT_EQ(0x28CEE3, c.bp[i]);
        }
        else if (i == 0xFE) // mask
        {
            EXPECT_EQ(0xF00FFF, c.bp[i]);
        }
        else
        {
            EXPECT_EQ(0x000000, c.bp[i]);
        }
    }
}

TEST(WGCodeTests, FlagErrors)
{
    uint8_t raw[] = {
        0x61, 0x23, 0x00, 0x00, 0x00, // allowed because BP command
        0x00, 0x00, // noops still allowed
        0x10, 0x00, 0x00, 0x10, 0x58, 0x00, 0x00, 0x00, 0x00, // will cause error since disabled
        0x00, 0x00, 0x00
    };
    Buffer gcode(sizeof(raw) / sizeof(raw[0]));
    gcode.putArray(raw, gcode.capacity()).flip();

    Ext::WGCode::Context c;
    EXPECT_THROW(Ext::WGCode::readGraphicsCode(gcode, &c, true, Ext::WGCode::FLAG_USE_BP), BRRESError);

    // will still throw because gcode contains a BP command
    gcode.clear();
    EXPECT_THROW(Ext::WGCode::readGraphicsCode(gcode, &c, true, Ext::WGCode::FLAG_USE_XF), BRRESError);
}

TEST(WGCodeTests, InvalidCommand)
{
    uint8_t raw[] = {
        0x61, 0x12, 0xAC, 0x3E, 0x99, // fine; 0x61 is Load BP
        0x00, 0x00, // fine; 0x00 is NOOP
        0x10, 0x00, 0x00, 0x10, 0x30, 0x00, 0x00, 0x00, 0x00, // fine; 0x10 is Load XF
        0x07 // unknown command
    };
    Buffer gcode(sizeof(raw) / sizeof(raw[0]));
    gcode.putArray(raw, gcode.capacity()).flip();

    Ext::WGCode::Context c;
    EXPECT_THROW(Ext::WGCode::readGraphicsCode(gcode, &c, true), BRRESError);
}

TEST(WGCodeTests, InvalidXFCommand)
{
    uint8_t raw[] = {
        0x10, 0x00, 0x00, 0x10, 0x40, 0x00, 0x30, 0x4A, 0xDF, // fine
        0x00,
        0x10, 0x00, 0x01, 0x10, 0x41, 0x23, 0x12, 0xAC, 0xC4, 0x67, 0x2D, 0xB3, 0xC0, // fine
        0x10, 0x00, 0x02, 0x10, 0x50, 0x32, 0x2D, 0x3C, 0xF0, 0x3A // missing 7 data bytes
    };
    Buffer gcode(sizeof(raw) / sizeof(raw[0]));
    gcode.putArray(raw, gcode.capacity()).flip();

    Ext::WGCode::Context c;
    EXPECT_THROW(Ext::WGCode::readGraphicsCode(gcode, &c, true), BRRESError);
}

TEST(WGCodeTests, InvalidBPCommand)
{
    uint8_t raw[] = {
        0x61, 0x23, 0xE3, 0x00, 0x08, // fine; all 4 data bytes present
        0x00, 0x00, 0x00,
        0x61, 0x01, 0x32, 0xAD // missing one data byte
    };
    Buffer gcode(sizeof(raw) / sizeof(raw[0]));
    gcode.putArray(raw, gcode.capacity()).flip();

    Ext::WGCode::Context c;
    EXPECT_THROW(Ext::WGCode::readGraphicsCode(gcode, &c, true), BRRESError);
}
