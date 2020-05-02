//////////////////////////////////////////////////
//  Copyright (c) 2020 Nara Hiero
//
// This file is licensed under GPLv3+
// Refer to the `License.txt` file included.
//////////////////////////////////////////////////

#include <gtest/gtest.h>

#include <CTLib/BRRES.hpp>

#include "Tests.hpp"

TEST(BRRESTests, AddHasAndRemove)
{
    CTLib::BRRES brres;
    EXPECT_FALSE(brres.has<CTLib::TEX0>("texture"));

    brres.add<CTLib::TEX0>("texture");
    EXPECT_TRUE(brres.has<CTLib::TEX0>("texture"));
    EXPECT_FALSE(brres.has<CTLib::MDL0>("texture"));

    brres.remove<CTLib::TEX0>("texture");
    EXPECT_FALSE(brres.has<CTLib::TEX0>("texture"));

    brres.add<CTLib::MDL0>("model");
    EXPECT_TRUE(brres.has<CTLib::MDL0>("model"));
    EXPECT_FALSE(brres.has<CTLib::TEX0>("model"));

    brres.remove<CTLib::MDL0>("model");
    EXPECT_FALSE(brres.has<CTLib::MDL0>("model"));

    brres.add<CTLib::TEX0>("tex1");
    brres.add<CTLib::TEX0>("tex2");
    brres.add<CTLib::TEX0>("tex3");
    brres.add<CTLib::TEX0>("tex4");
    EXPECT_TRUE(brres.has<CTLib::TEX0>("tex1"));
    EXPECT_TRUE(brres.has<CTLib::TEX0>("tex3"));
    EXPECT_FALSE(brres.has<CTLib::TEX0>("tex6"));
    
    brres.remove<CTLib::TEX0>("tex2");
    EXPECT_FALSE(brres.has<CTLib::TEX0>("tex2"));
    EXPECT_TRUE(brres.has<CTLib::TEX0>("tex4"));

    brres.add<CTLib::MDL0>("tex3");
    EXPECT_TRUE(brres.has<CTLib::TEX0>("tex3"));
    EXPECT_TRUE(brres.has<CTLib::MDL0>("tex3"));

    brres.remove<CTLib::TEX0>("tex3");
    EXPECT_FALSE(brres.has<CTLib::TEX0>("tex3"));
    EXPECT_TRUE(brres.has<CTLib::MDL0>("tex3"));
}

TEST(BRRESTests, GetGetAllAndCount)
{
    CTLib::BRRES brres;
    
    EXPECT_EQ(0, brres.count<CTLib::MDL0>());
    EXPECT_EQ(0, brres.count<CTLib::TEX0>());

    CTLib::MDL0* model = brres.add<CTLib::MDL0>("model");
    EXPECT_EQ(1, brres.count<CTLib::MDL0>());
    EXPECT_EQ(0, brres.count<CTLib::TEX0>());
    EXPECT_EQ(model, brres.get<CTLib::MDL0>("model"));

    CTLib::TEX0* texture = brres.add<CTLib::TEX0>("texture");
    EXPECT_EQ(1, brres.count<CTLib::MDL0>());
    EXPECT_EQ(1, brres.count<CTLib::TEX0>());
    EXPECT_EQ(texture, brres.get<CTLib::TEX0>("texture"));

    brres.add<CTLib::MDL0>("model2");
    brres.add<CTLib::MDL0>("model3");
    brres.add<CTLib::TEX0>("model");
    EXPECT_EQ(3, brres.count<CTLib::MDL0>());
    EXPECT_EQ(2, brres.count<CTLib::TEX0>());
    EXPECT_NE((void*)model, (void*)brres.get<CTLib::TEX0>("model"));

    brres.add<CTLib::TEX0>("tex1");
    brres.add<CTLib::TEX0>("tex2");
    brres.add<CTLib::TEX0>("tex3");
    brres.add<CTLib::TEX0>("tex4");
    EXPECT_EQ(6, brres.count<CTLib::TEX0>());

    std::vector<CTLib::TEX0*> tex0s = brres.getAll<CTLib::TEX0>();
    EXPECT_EQ(6, tex0s.size());

    brres.add<CTLib::TEX0>("new");
    EXPECT_EQ(7, brres.count<CTLib::TEX0>());
    EXPECT_EQ(6, tex0s.size());
}

TEST(BRRESTests, SubfileCount)
{
    CTLib::BRRES brres;

    EXPECT_EQ(0, brres.getSubfileCount());

    brres.add<CTLib::MDL0>("model");
    brres.add<CTLib::TEX0>("texture");
    EXPECT_EQ(2, brres.getSubfileCount());

    brres.add<CTLib::MDL0>("mdl1");
    brres.add<CTLib::MDL0>("mdl2");
    brres.add<CTLib::MDL0>("mdl3");
    brres.add<CTLib::MDL0>("mdl4");
    EXPECT_EQ(6, brres.getSubfileCount());

    brres.remove<CTLib::MDL0>("mdl3");
    brres.remove<CTLib::TEX0>("texture");
    EXPECT_EQ(4, brres.getSubfileCount());
}

TEST(BRRESTests, Errors)
{
    CTLib::BRRES brres;

    EXPECT_NO_THROW(brres.add<CTLib::MDL0>("entry"));
    EXPECT_THROW(brres.add<CTLib::MDL0>("entry"), CTLib::BRRESError);
    EXPECT_NO_THROW(brres.add<CTLib::TEX0>("entry"));
    EXPECT_THROW(brres.add<CTLib::TEX0>("entry"), CTLib::BRRESError);

    EXPECT_NO_THROW(brres.get<CTLib::TEX0>("entry"));
    EXPECT_THROW(brres.get<CTLib::TEX0>("texture"), CTLib::BRRESError);

    brres.add<CTLib::TEX0>("texture");
    EXPECT_NO_THROW(brres.get<CTLib::TEX0>("texture"));
    EXPECT_THROW(brres.get<CTLib::MDL0>("texture"), CTLib::BRRESError);

    EXPECT_THROW(brres.remove<CTLib::MDL0>("model"), CTLib::BRRESError);
    EXPECT_THROW(brres.remove<CTLib::MDL0>("texture"), CTLib::BRRESError);
    EXPECT_NO_THROW(brres.remove<CTLib::TEX0>("texture"));
    EXPECT_THROW(brres.remove<CTLib::TEX0>("texture"), CTLib::BRRESError);
    EXPECT_THROW(brres.get<CTLib::TEX0>("texture"), CTLib::BRRESError);
}

TEST(MDL0Tests, AddHasAndRemove)
{
    CTLib::BRRES brres;
    CTLib::MDL0* mdl0 = brres.add<CTLib::MDL0>("model");
    EXPECT_FALSE(mdl0->has<CTLib::MDL0::VertexArray>("vertices"));

    mdl0->add<CTLib::MDL0::VertexArray>("vertices");
    EXPECT_TRUE(mdl0->has<CTLib::MDL0::VertexArray>("vertices"));
    EXPECT_FALSE(mdl0->has<CTLib::MDL0::ColourArray>("vertices"));

    mdl0->add<CTLib::MDL0::TexCoordArray>("#0");
    EXPECT_TRUE(mdl0->has<CTLib::MDL0::TexCoordArray>("#0"));
    EXPECT_FALSE(mdl0->has<CTLib::MDL0::TexCoordArray>("#1"));
    EXPECT_FALSE(mdl0->has<CTLib::MDL0::NormalArray>("#0"));
    EXPECT_TRUE(mdl0->has<CTLib::MDL0::VertexArray>("vertices"));

    mdl0->add<CTLib::MDL0::ColourArray>("#0");
    mdl0->add<CTLib::MDL0::ColourArray>("#1");
    mdl0->add<CTLib::MDL0::ColourArray>("#2");
    mdl0->add<CTLib::MDL0::ColourArray>("#3");
    EXPECT_TRUE(mdl0->has<CTLib::MDL0::ColourArray>("#1"));
    EXPECT_TRUE(mdl0->has<CTLib::MDL0::ColourArray>("#2"));
    EXPECT_FALSE(mdl0->has<CTLib::MDL0::ColourArray>("#4"));

    mdl0->remove<CTLib::MDL0::ColourArray>("#0");
    mdl0->remove<CTLib::MDL0::ColourArray>("#2");
    EXPECT_FALSE(mdl0->has<CTLib::MDL0::ColourArray>("#0"));
    EXPECT_TRUE(mdl0->has<CTLib::MDL0::ColourArray>("#1"));
    EXPECT_FALSE(mdl0->has<CTLib::MDL0::ColourArray>("#2"));
    EXPECT_TRUE(mdl0->has<CTLib::MDL0::ColourArray>("#3"));
    EXPECT_TRUE(mdl0->has<CTLib::MDL0::TexCoordArray>("#0"));
}

TEST(MDL0Tests, GetGetAllAndCount)
{
    CTLib::BRRES brres;
    CTLib::MDL0* mdl0 = brres.add<CTLib::MDL0>("model");
    EXPECT_EQ(0, mdl0->count<CTLib::MDL0::VertexArray>());
    EXPECT_EQ(0, mdl0->count<CTLib::MDL0::ColourArray>());

    CTLib::MDL0::ColourArray* colours = mdl0->add<CTLib::MDL0::ColourArray>("colours");
    EXPECT_EQ(0, mdl0->count<CTLib::MDL0::VertexArray>());
    EXPECT_EQ(1, mdl0->count<CTLib::MDL0::ColourArray>());
    EXPECT_EQ(colours, mdl0->get<CTLib::MDL0::ColourArray>("colours"));

    CTLib::MDL0::VertexArray* vertices = mdl0->add<CTLib::MDL0::VertexArray>("vertices");
    EXPECT_EQ(1, mdl0->count<CTLib::MDL0::VertexArray>());
    EXPECT_EQ(1, mdl0->count<CTLib::MDL0::ColourArray>());
    EXPECT_EQ(vertices, mdl0->get<CTLib::MDL0::VertexArray>("vertices"));

    EXPECT_EQ(0, mdl0->count<CTLib::MDL0::TexCoordArray>());
    mdl0->add<CTLib::MDL0::TexCoordArray>("#0");
    mdl0->add<CTLib::MDL0::TexCoordArray>("#1");
    EXPECT_EQ(2, mdl0->count<CTLib::MDL0::TexCoordArray>());
    mdl0->add<CTLib::MDL0::TexCoordArray>("#2");
    mdl0->add<CTLib::MDL0::TexCoordArray>("#3");
    mdl0->add<CTLib::MDL0::TexCoordArray>("#4");
    EXPECT_EQ(5, mdl0->count<CTLib::MDL0::TexCoordArray>());
    mdl0->add<CTLib::MDL0::TexCoordArray>("#5");
    EXPECT_EQ(6, mdl0->count<CTLib::MDL0::TexCoordArray>());
    mdl0->remove<CTLib::MDL0::TexCoordArray>("#3");
    mdl0->remove<CTLib::MDL0::TexCoordArray>("#1");
    EXPECT_EQ(4, mdl0->count<CTLib::MDL0::TexCoordArray>());
    mdl0->remove<CTLib::MDL0::TexCoordArray>("#4");
    mdl0->add<CTLib::MDL0::TexCoordArray>("#6");
    mdl0->add<CTLib::MDL0::TexCoordArray>("#7");
    EXPECT_EQ(5, mdl0->count<CTLib::MDL0::TexCoordArray>());

    std::vector<CTLib::MDL0::TexCoordArray*> vector = mdl0->getAll<CTLib::MDL0::TexCoordArray>();
    EXPECT_EQ(5, vector.size());

    mdl0->remove<CTLib::MDL0::TexCoordArray>("#0");
    EXPECT_EQ(5, vector.size());
    EXPECT_EQ(4, mdl0->count<CTLib::MDL0::TexCoordArray>());
}

TEST(MDL0Tests, Errors)
{
    CTLib::BRRES brres;
    CTLib::MDL0* mdl0 = brres.add<CTLib::MDL0>("model");

    EXPECT_THROW(mdl0->add<CTLib::MDL0::Links>("links"), CTLib::BRRESError);
    EXPECT_THROW(mdl0->add<CTLib::MDL0::TextureLink>("link"), CTLib::BRRESError);
}

TEST(TEX0Tests, SetData)
{
    CTLib::BRRES brres;
    CTLib::TEX0* tex0 = brres.add<CTLib::TEX0>("texture");

    CTLib::Image image = CTLib::ImageIO::read(CT_LIB_TESTS_DATA_DIR"/Images/TEX0/Sand.png");
    tex0->setTextureData(image, CTLib::ImageFormat::RGB565);

    EXPECT_EQ(image.getWidth(), tex0->getWidth());
    EXPECT_EQ(image.getHeight(), tex0->getHeight());
    EXPECT_EQ(CTLib::ImageFormat::RGB565, tex0->getFormat());
    EXPECT_EQ(image.getWidth() * image.getHeight() * 2, tex0->getTextureData().capacity());
}

TEST(TEX0Tests, MipmapSizes)
{
    CTLib::BRRES brres;
    CTLib::TEX0* tex0 = brres.add<CTLib::TEX0>("texture");
    
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
    CTLib::TEX0* tex0 = brres.add<CTLib::TEX0>("texture");
    
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
