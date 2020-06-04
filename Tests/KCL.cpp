//////////////////////////////////////////////////
//  Copyright (c) 2020 Nara Hiero
//
// This file is licensed under GPLv3+
// Refer to the `License.txt` file included.
//////////////////////////////////////////////////

#include <gtest/gtest.h>

#include <CTLib/KCL.hpp>

TEST(KCLTests, MinPosAndMasks)
{
    CTLib::KCL::Settings settings;
    settings.blowFactor = 0.f;
    CTLib::KCL::setSettings(settings);

    float verts[] = {
        3492.f, 432.f, 10002.f,   1231.f, 839.f, -234.f,   -881.f, -210.f, 430.f,
        432.f, -2.f, 7832.f,   5428.f, -432.f, -4013.f,   542.f, 574.f, -324.f,
        94.f, 832.f, 0.f,   2830.f, 1.f, -3243.f,   4523.f, -354.f, 9384.f
    };
    uint32_t vertCount = static_cast<uint32_t>(sizeof(verts) / sizeof(float));

    CTLib::Buffer vertices(vertCount * 4);
    for (uint32_t i = 0; i < vertCount; ++i)
    {
        vertices.putFloat(verts[i]);
    }
    vertices.flip();

    CTLib::Buffer flags(vertCount / 9 * 2);
    for (uint32_t i = 0; i < vertCount / 9; ++i)
    {
        flags.putShort(0);
    }
    flags.flip();

    CTLib::KCL kcl = CTLib::KCL::fromModel(vertices, flags);
    CTLib::KCL::Octree* octree = kcl.getOctree();

    EXPECT_EQ(CTLib::Vector3f(-881.f, -432.f, -4013.f), octree->getMinPos());
    EXPECT_EQ(0xFFFFE000, octree->getMaskX());
    EXPECT_EQ(0xFFFFF800, octree->getMaskY());
    EXPECT_EQ(0xFFFFC000, octree->getMaskZ());

    //////////////////////////

    settings.blowFactor = 400.f;
    CTLib::KCL::setSettings(settings);

    vertices.clear();
    flags.clear();
    CTLib::KCL kcl2 = CTLib::KCL::fromModel(vertices, flags);
    CTLib::KCL::Octree* octree2 = kcl2.getOctree();

    EXPECT_EQ(CTLib::Vector3f(-1281.f, -832.f, -4413.f), octree2->getMinPos());
    EXPECT_EQ(0xFFFFE000, octree2->getMaskX());
    EXPECT_EQ(0xFFFFF000, octree2->getMaskY());
    EXPECT_EQ(0xFFFFC000, octree2->getMaskZ());
}

TEST(KCLTests, ShiftsAndSize)
{
    CTLib::KCL::Settings settings;
    settings.blowFactor = 0.f;
    CTLib::KCL::setSettings(settings);

    float verts[] = {
        17432.f, -2134.f, 975.f,   -3242.f, 439.f, 423.f,   8943.f, 2489.f, -347.f,
        -42.f, -348.f, -2342.f,   9128.f, -1432.f, 8235.f,   185.f, -843.f, -9823.f,
        2341.f, 142.f, 432.f,   2.f, 923.f, 4323.f,   -43.f, -12.f, -6234.f
    };
    uint32_t vertCount = static_cast<uint32_t>(sizeof(verts) / sizeof(float));

    CTLib::Buffer vertices(vertCount * 4);
    for (uint32_t i = 0; i < vertCount; ++i)
    {
        vertices.putFloat(verts[i]);
    }
    vertices.flip();

    CTLib::Buffer flags(vertCount / 9 * 2);
    for (uint32_t i = 0; i < vertCount / 9; ++i)
    {
        flags.putShort(0);
    }
    flags.flip();

    CTLib::KCL kcl = CTLib::KCL::fromModel(vertices, flags);
    CTLib::KCL::Octree* octree = kcl.getOctree();

    EXPECT_EQ(0xD, octree->getShift());
    EXPECT_EQ(0x2, octree->getShiftY());
    EXPECT_EQ(0x2, octree->getShiftZ());
    EXPECT_EQ((CTLib::Vector<uint32_t, 3>{4, 1, 4}), octree->getSize());
    EXPECT_EQ(CTLib::Vector3f(8192, 8192, 8192), octree->getBlockSize());

    //////////////////////////

    vertices.putFloat(0x24, -15392.f); // 2nd triangle; 1st point; X-axis
    vertices.putFloat(0x58, 7239.f); // 3rd triangle; 2nd point; Y-axis
    vertices.putFloat(0x20, 23710.f); // 1st triangle; 3rd point; Z-axis
    vertices.clear();
    flags.clear();

    CTLib::KCL kcl2 = CTLib::KCL::fromModel(vertices, flags);
    CTLib::KCL::Octree* octree2 = kcl2.getOctree();

    EXPECT_EQ(0xD, octree2->getShift());
    EXPECT_EQ(0x3, octree2->getShiftY());
    EXPECT_EQ(0x4, octree2->getShiftZ());
    EXPECT_EQ((CTLib::Vector<uint32_t, 3>{8, 2, 8}), octree2->getSize());
    EXPECT_EQ(CTLib::Vector3f(8192, 8192, 8192), octree2->getBlockSize());
}

TEST(KCLTests, OctreeErrors)
{
    float verts[] = {
        2843.f, -721.f, -23.f,   -1042.f, 284.f, 732.f,   8123.f, -5.f, -8923.f,
        -138.f, 67.f, -545.f,   -63.f, 510.f, 9103.f,   55.f, -92.f, -1233.f,
        -792.f, 152.f, 2742.f,   254.f, 13.f, -243.f,   -642.f, 334.f, -6752.f
    };
    uint32_t vertCount = static_cast<uint32_t>(sizeof(verts) / sizeof(float));

    CTLib::Buffer vertices(vertCount * 4);
    for (uint32_t i = 0; i < vertCount; ++i)
    {
        vertices.putFloat(verts[i]);
    }
    vertices.flip();

    CTLib::Buffer flags(vertCount / 9 * 2);
    for (uint32_t i = 0; i < vertCount / 9; ++i)
    {
        flags.putShort(0);
    }
    flags.flip();

    CTLib::KCL kcl = CTLib::KCL::fromModel(vertices, flags);
    CTLib::KCL::Octree* octree = kcl.getOctree();

    ASSERT_EQ((CTLib::Vector<uint32_t, 3>{2, 1, 4}), octree->getSize());
    EXPECT_THROW(octree->getNode({2, 0, 3}), CTLib::KCLError);
    EXPECT_THROW(octree->getNode({1, 1, 3}), CTLib::KCLError);
    EXPECT_THROW(octree->getNode({1, 0, 4}), CTLib::KCLError);
    EXPECT_NO_THROW(octree->getNode({1, 0, 3}));

    ASSERT_EQ(8, octree->getRootNodeCount());
    EXPECT_THROW(octree->getNode(8), CTLib::KCLError);
    EXPECT_NO_THROW(octree->getNode(0));
}

TEST(KCLTests, FromModelErrors)
{
    float verts[] = {
        98.f, 32.f, -243.f,   -162.f, -234.f, 342.f,   2.f, -5523.f, -3.f,
        -42.f, 232.f, 2342.f,   924.f, 192.f, -32.f,   721.f, -832.f, -712.f,
        -3.f, 3028.f, 12.f,   -324.f, 432.f, -3.f,   2342.f, 5.f, -324.f
    };
    uint32_t vertCount = static_cast<uint32_t>(sizeof(verts) / sizeof(float));

    CTLib::Buffer vertices(vertCount * 4);
    for (uint32_t i = 0; i < vertCount; ++i)
    {
        vertices.putFloat(verts[i]);
    }
    vertices.flip();

    CTLib::Buffer flags(vertCount / 9 * 2);
    for (uint32_t i = 0; i < vertCount / 9; ++i)
    {
        flags.putShort(0);
    }
    flags.flip();

    EXPECT_THROW(CTLib::KCL::fromModel(vertices, flags, 4), CTLib::KCLError);

    flags.position(2);
    EXPECT_THROW(CTLib::KCL::fromModel(vertices, flags), CTLib::KCLError);

    vertices.position(0xC);
    flags.position(0);
    EXPECT_THROW(CTLib::KCL::fromModel(vertices, flags, 3), CTLib::KCLError);
}
