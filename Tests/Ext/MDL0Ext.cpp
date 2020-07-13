//////////////////////////////////////////////////
//  Copyright (c) 2020 Nara Hiero
//
// This file is licensed under GPLv3+
// Refer to the `License.txt` file included.
//////////////////////////////////////////////////

#include <gtest/gtest.h>

#include <CTLib/Ext/MDL0.hpp>

using namespace CTLib;

TEST(ShaderCodeTests, AddStage)
{
    Ext::ShaderCode shader;
    EXPECT_EQ(0, shader.getStageCount());

    shader.addStage();
    EXPECT_EQ(1, shader.getStageCount());

    shader.addStage();
    shader.addStage();
    EXPECT_EQ(3, shader.getStageCount());
}

TEST(ShaderCodeTests, Errors)
{
    Ext::ShaderCode shader;

    for (uint32_t i = 0; i < 0x100; ++i)
    {
        if (i < 4)
        {
            EXPECT_NO_THROW(shader.getSwapTable(i));
        }
        else
        {
            EXPECT_THROW(shader.getSwapTable(i), BRRESError);
        }

        if (i < 4)
        {
            EXPECT_NO_THROW(shader.getTexMapID(i));
            EXPECT_NO_THROW(shader.getTexCoordIndex(i));
        }
        else
        {
            EXPECT_THROW(shader.getTexMapID(i), BRRESError);
            EXPECT_THROW(shader.getTexCoordIndex(i), BRRESError);
        }

        if (i < 8)
        {
            EXPECT_NO_THROW(shader.setTexMapID(0, i));
            EXPECT_NO_THROW(shader.setTexCoordIndex(0, i));
        }
        else
        {
            EXPECT_THROW(shader.setTexMapID(0, i), BRRESError);
            EXPECT_THROW(shader.setTexCoordIndex(0, i), BRRESError);
        }
    }
}

TEST(ShaderCodeTests, StageErrors)
{
    Ext::ShaderCode shader;
    EXPECT_THROW(shader.getStage(0), BRRESError);

    shader.addStage();
    EXPECT_NO_THROW(shader.getStage(0));
    EXPECT_THROW(shader.getStage(1), BRRESError);

    shader.addStage();
    shader.addStage();
    EXPECT_NO_THROW(shader.getStage(0));
    EXPECT_NO_THROW(shader.getStage(1));
    EXPECT_NO_THROW(shader.getStage(2));
    EXPECT_THROW(shader.getStage(3), BRRESError);
    EXPECT_THROW(shader.getStage(4), BRRESError);
    EXPECT_THROW(shader.getStage(5), BRRESError);
    EXPECT_THROW(shader.getStage(6), BRRESError);
    EXPECT_THROW(shader.getStage(7), BRRESError);

    shader.addStage();
    shader.addStage();
    shader.addStage();
    shader.addStage();
    shader.addStage();

    EXPECT_THROW(shader.addStage(), BRRESError);
}

TEST(ShaderCodeStageTests, Errors)
{
    Ext::ShaderCode shader;
    Ext::ShaderCode::Stage& stage = shader.addStage();

    for (uint32_t i = 0; i < 0x100; ++i)
    {
        if (i < 8)
        {
            EXPECT_NO_THROW(stage.setTexMapID(i));
            EXPECT_NO_THROW(stage.setTexCoordIndex(i));
        }
        else
        {
            EXPECT_THROW(stage.setTexMapID(i), BRRESError);
            EXPECT_THROW(stage.setTexCoordIndex(i), BRRESError);
        }
    }
}
