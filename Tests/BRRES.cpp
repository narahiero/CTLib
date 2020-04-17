//////////////////////////////////////////////////
//  Copyright (c) 2020 Nara Hiero
//
// This file is licensed under GPLv3+
// Refer to the `License.txt` file included.
//////////////////////////////////////////////////

#include <gtest/gtest.h>

#include <CTLib/BRRES.hpp>

#include "Tests.hpp"

TEST(TEX0Tests, MipmapSizes)
{
    CTLib::BRRES brres;
    CTLib::TEX0* tex0 = brres.newTEX0("texture");
    
    CTLib::Image image = CTLib::ImageIO::read(CT_LIB_TESTS_DATA_DIR"/Images/TEX0/Sand.png");
    tex0->setTextureData(image, CTLib::ImageFormat::RGB565);

    EXPECT_EQ(320, tex0->getMipmapWidth(0));
    EXPECT_EQ(240, tex0->getMipmapHeight(0));

    EXPECT_EQ(160, tex0->getMipmapWidth(1));
    EXPECT_EQ(120, tex0->getMipmapHeight(1));

    EXPECT_EQ(80, tex0->getMipmapWidth(2));
    EXPECT_EQ(60, tex0->getMipmapHeight(2));

    EXPECT_EQ(40, tex0->getMipmapWidth(3));
    EXPECT_EQ(30, tex0->getMipmapHeight(3));

    EXPECT_EQ(20, tex0->getMipmapWidth(4));
    EXPECT_EQ(15, tex0->getMipmapHeight(4));

    EXPECT_EQ(10, tex0->getMipmapWidth(5));
    EXPECT_EQ(7, tex0->getMipmapHeight(5));

    EXPECT_EQ(5, tex0->getMipmapWidth(6));
    EXPECT_EQ(3, tex0->getMipmapHeight(6));

    EXPECT_EQ(2, tex0->getMipmapWidth(7));
    EXPECT_EQ(1, tex0->getMipmapHeight(7));

    EXPECT_EQ(1, tex0->getMipmapWidth(8));
    EXPECT_EQ(0, tex0->getMipmapHeight(8));
}

TEST(TEX0Tests, GenMipmaps)
{
    CTLib::BRRES brres;
    CTLib::TEX0* tex0 = brres.newTEX0("texture");
    
    CTLib::Image image = CTLib::ImageIO::read(CT_LIB_TESTS_DATA_DIR"/Images/TEX0/Sand.png");
    tex0->setTextureData(image, CTLib::ImageFormat::RGB565);
    tex0->generateMipmaps(6, image);
    
    // some values are not what they should be because the RGB565 encoder uses 4x4 blocks
    EXPECT_EQ(320 * 240 * 2, tex0->getMipmapTextureData(0).capacity());
    EXPECT_EQ(160 * 120 * 2, tex0->getMipmapTextureData(1).capacity());
    EXPECT_EQ( 80 *  60 * 2, tex0->getMipmapTextureData(2).capacity());
    EXPECT_EQ( 40 *  32 * 2, tex0->getMipmapTextureData(3).capacity());
    EXPECT_EQ( 20 *  16 * 2, tex0->getMipmapTextureData(4).capacity());
    EXPECT_EQ( 12 *   8 * 2, tex0->getMipmapTextureData(5).capacity());

    EXPECT_THROW(tex0->generateMipmaps(9), CTLib::BRRESError);
}
