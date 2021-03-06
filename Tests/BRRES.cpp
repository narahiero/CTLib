//////////////////////////////////////////////////
//  Copyright (c) 2020 Nara Hiero
//
// This file is licensed under GPLv3+
// Refer to the `License.txt` file included.
//////////////////////////////////////////////////

#include <gtest/gtest.h>

#include <CTLib/BRRES.hpp>

#include "Tests.hpp"

using namespace CTLib;

TEST(BRRESTests, AddHasAndRemove)
{
    BRRES brres;
    EXPECT_FALSE(brres.has<TEX0>("texture"));

    brres.add<TEX0>("texture");
    EXPECT_TRUE(brres.has<TEX0>("texture"));
    EXPECT_FALSE(brres.has<MDL0>("texture"));

    brres.remove<TEX0>("texture");
    EXPECT_FALSE(brres.has<TEX0>("texture"));

    brres.add<MDL0>("model");
    EXPECT_TRUE(brres.has<MDL0>("model"));
    EXPECT_FALSE(brres.has<TEX0>("model"));

    brres.remove<MDL0>("model");
    EXPECT_FALSE(brres.has<MDL0>("model"));

    brres.add<TEX0>("tex1");
    brres.add<TEX0>("tex2");
    brres.add<TEX0>("tex3");
    brres.add<TEX0>("tex4");
    EXPECT_TRUE(brres.has<TEX0>("tex1"));
    EXPECT_TRUE(brres.has<TEX0>("tex3"));
    EXPECT_FALSE(brres.has<TEX0>("tex6"));
    
    brres.remove<TEX0>("tex2");
    EXPECT_FALSE(brres.has<TEX0>("tex2"));
    EXPECT_TRUE(brres.has<TEX0>("tex4"));

    brres.add<MDL0>("tex3");
    EXPECT_TRUE(brres.has<TEX0>("tex3"));
    EXPECT_TRUE(brres.has<MDL0>("tex3"));

    brres.remove<TEX0>("tex3");
    EXPECT_FALSE(brres.has<TEX0>("tex3"));
    EXPECT_TRUE(brres.has<MDL0>("tex3"));
}

TEST(BRRESTests, GetGetAllAndCount)
{
    BRRES brres;
    
    EXPECT_EQ(0, brres.count<MDL0>());
    EXPECT_EQ(0, brres.count<TEX0>());

    MDL0* model = brres.add<MDL0>("model");
    EXPECT_EQ(1, brres.count<MDL0>());
    EXPECT_EQ(0, brres.count<TEX0>());
    EXPECT_EQ(model, brres.get<MDL0>("model"));

    TEX0* texture = brres.add<TEX0>("texture");
    EXPECT_EQ(1, brres.count<MDL0>());
    EXPECT_EQ(1, brres.count<TEX0>());
    EXPECT_EQ(texture, brres.get<TEX0>("texture"));

    brres.add<MDL0>("model2");
    brres.add<MDL0>("model3");
    brres.add<TEX0>("model");
    EXPECT_EQ(3, brres.count<MDL0>());
    EXPECT_EQ(2, brres.count<TEX0>());
    EXPECT_NE((void*)model, (void*)brres.get<TEX0>("model"));

    brres.add<TEX0>("tex1");
    brres.add<TEX0>("tex2");
    brres.add<TEX0>("tex3");
    brres.add<TEX0>("tex4");
    EXPECT_EQ(6, brres.count<TEX0>());

    std::vector<TEX0*> tex0s = brres.getAll<TEX0>();
    EXPECT_EQ(6, tex0s.size());

    brres.add<TEX0>("new");
    EXPECT_EQ(7, brres.count<TEX0>());
    EXPECT_EQ(6, tex0s.size());
}

TEST(BRRESTests, SubfileCount)
{
    BRRES brres;

    EXPECT_EQ(0, brres.getSubfileCount());

    brres.add<MDL0>("model");
    brres.add<TEX0>("texture");
    EXPECT_EQ(2, brres.getSubfileCount());

    brres.add<MDL0>("mdl1");
    brres.add<MDL0>("mdl2");
    brres.add<MDL0>("mdl3");
    brres.add<MDL0>("mdl4");
    EXPECT_EQ(6, brres.getSubfileCount());

    brres.remove<MDL0>("mdl3");
    brres.remove<TEX0>("texture");
    EXPECT_EQ(4, brres.getSubfileCount());
}

TEST(BRRESTests, Errors)
{
    BRRES brres;

    EXPECT_NO_THROW(brres.add<MDL0>("entry"));
    EXPECT_THROW(brres.add<MDL0>("entry"), BRRESError);
    EXPECT_NO_THROW(brres.add<TEX0>("entry"));
    EXPECT_THROW(brres.add<TEX0>("entry"), BRRESError);

    EXPECT_NO_THROW(brres.get<TEX0>("entry"));
    EXPECT_THROW(brres.get<TEX0>("texture"), BRRESError);

    brres.add<TEX0>("texture");
    EXPECT_NO_THROW(brres.get<TEX0>("texture"));
    EXPECT_THROW(brres.get<MDL0>("texture"), BRRESError);

    EXPECT_THROW(brres.remove<MDL0>("model"), BRRESError);
    EXPECT_THROW(brres.remove<MDL0>("texture"), BRRESError);
    EXPECT_NO_THROW(brres.remove<TEX0>("texture"));
    EXPECT_THROW(brres.remove<TEX0>("texture"), BRRESError);
    EXPECT_THROW(brres.get<TEX0>("texture"), BRRESError);
}

TEST(MDL0Tests, AddHasAndRemove)
{
    BRRES brres;
    MDL0* mdl0 = brres.add<MDL0>("model");
    EXPECT_FALSE(mdl0->has<MDL0::VertexArray>("vertices"));

    mdl0->add<MDL0::VertexArray>("vertices");
    EXPECT_TRUE(mdl0->has<MDL0::VertexArray>("vertices"));
    EXPECT_FALSE(mdl0->has<MDL0::ColourArray>("vertices"));

    mdl0->add<MDL0::TexCoordArray>("#0");
    EXPECT_TRUE(mdl0->has<MDL0::TexCoordArray>("#0"));
    EXPECT_FALSE(mdl0->has<MDL0::TexCoordArray>("#1"));
    EXPECT_FALSE(mdl0->has<MDL0::NormalArray>("#0"));
    EXPECT_TRUE(mdl0->has<MDL0::VertexArray>("vertices"));

    mdl0->add<MDL0::ColourArray>("#0");
    mdl0->add<MDL0::ColourArray>("#1");
    mdl0->add<MDL0::ColourArray>("#2");
    mdl0->add<MDL0::ColourArray>("#3");
    EXPECT_TRUE(mdl0->has<MDL0::ColourArray>("#1"));
    EXPECT_TRUE(mdl0->has<MDL0::ColourArray>("#2"));
    EXPECT_FALSE(mdl0->has<MDL0::ColourArray>("#4"));

    mdl0->remove<MDL0::ColourArray>("#0");
    mdl0->remove<MDL0::ColourArray>("#2");
    EXPECT_FALSE(mdl0->has<MDL0::ColourArray>("#0"));
    EXPECT_TRUE(mdl0->has<MDL0::ColourArray>("#1"));
    EXPECT_FALSE(mdl0->has<MDL0::ColourArray>("#2"));
    EXPECT_TRUE(mdl0->has<MDL0::ColourArray>("#3"));
    EXPECT_TRUE(mdl0->has<MDL0::TexCoordArray>("#0"));
}

TEST(MDL0Tests, GetGetAllAndCount)
{
    BRRES brres;
    MDL0* mdl0 = brres.add<MDL0>("model");
    EXPECT_EQ(0, mdl0->count<MDL0::VertexArray>());
    EXPECT_EQ(0, mdl0->count<MDL0::ColourArray>());

    MDL0::ColourArray* colours = mdl0->add<MDL0::ColourArray>("colours");
    EXPECT_EQ(0, mdl0->count<MDL0::VertexArray>());
    EXPECT_EQ(1, mdl0->count<MDL0::ColourArray>());
    EXPECT_EQ(colours, mdl0->get<MDL0::ColourArray>("colours"));

    MDL0::VertexArray* vertices = mdl0->add<MDL0::VertexArray>("vertices");
    EXPECT_EQ(1, mdl0->count<MDL0::VertexArray>());
    EXPECT_EQ(1, mdl0->count<MDL0::ColourArray>());
    EXPECT_EQ(vertices, mdl0->get<MDL0::VertexArray>("vertices"));

    EXPECT_EQ(0, mdl0->count<MDL0::TexCoordArray>());
    mdl0->add<MDL0::TexCoordArray>("#0");
    mdl0->add<MDL0::TexCoordArray>("#1");
    EXPECT_EQ(2, mdl0->count<MDL0::TexCoordArray>());
    mdl0->add<MDL0::TexCoordArray>("#2");
    mdl0->add<MDL0::TexCoordArray>("#3");
    mdl0->add<MDL0::TexCoordArray>("#4");
    EXPECT_EQ(5, mdl0->count<MDL0::TexCoordArray>());
    mdl0->add<MDL0::TexCoordArray>("#5");
    EXPECT_EQ(6, mdl0->count<MDL0::TexCoordArray>());
    mdl0->remove<MDL0::TexCoordArray>("#3");
    mdl0->remove<MDL0::TexCoordArray>("#1");
    EXPECT_EQ(4, mdl0->count<MDL0::TexCoordArray>());
    mdl0->remove<MDL0::TexCoordArray>("#4");
    mdl0->add<MDL0::TexCoordArray>("#6");
    mdl0->add<MDL0::TexCoordArray>("#7");
    EXPECT_EQ(5, mdl0->count<MDL0::TexCoordArray>());

    std::vector<MDL0::TexCoordArray*> vector = mdl0->getAll<MDL0::TexCoordArray>();
    EXPECT_EQ(5, vector.size());

    mdl0->remove<MDL0::TexCoordArray>("#0");
    EXPECT_EQ(5, vector.size());
    EXPECT_EQ(4, mdl0->count<MDL0::TexCoordArray>());
}

TEST(MDL0Tests, Errors)
{
    BRRES brres;
    MDL0* mdl0 = brres.add<MDL0>("model");

    EXPECT_THROW(mdl0->add<MDL0::Links>("links"), BRRESError);
    EXPECT_THROW(mdl0->add<MDL0::TextureLink>("link"), BRRESError);

    EXPECT_NO_THROW(mdl0->add<MDL0::VertexArray>("vertices"));
    EXPECT_THROW(mdl0->add<MDL0::VertexArray>("vertices"), BRRESError);
    EXPECT_NO_THROW(mdl0->add<MDL0::ColourArray>("vertices"));
    EXPECT_THROW(mdl0->add<MDL0::ColourArray>("vertices"), BRRESError);

    EXPECT_NO_THROW(mdl0->add<MDL0::TexCoordArray>("#0"));
    EXPECT_NO_THROW(mdl0->add<MDL0::TexCoordArray>("#1"));
    EXPECT_NO_THROW(mdl0->add<MDL0::TexCoordArray>("#2"));
    EXPECT_THROW(mdl0->add<MDL0::TexCoordArray>("#1"), BRRESError);
    EXPECT_NO_THROW(mdl0->add<MDL0::TexCoordArray>("#3"));
    EXPECT_THROW(mdl0->add<MDL0::TexCoordArray>("#3"), BRRESError);

    EXPECT_THROW(mdl0->remove<MDL0::NormalArray>("normals"), BRRESError);
    mdl0->add<MDL0::NormalArray>("normals");
    EXPECT_NO_THROW(mdl0->remove<MDL0::NormalArray>("normals"));
    EXPECT_THROW(mdl0->remove<MDL0::NormalArray>("normals"), BRRESError);

    EXPECT_THROW(mdl0->remove<MDL0::ColourArray>("array"), BRRESError);
    EXPECT_THROW(mdl0->remove<MDL0::NormalArray>("array"), BRRESError);
    mdl0->add<MDL0::ColourArray>("array");
    mdl0->add<MDL0::NormalArray>("array");
    EXPECT_NO_THROW(mdl0->remove<MDL0::ColourArray>("array"));
    EXPECT_THROW(mdl0->remove<MDL0::ColourArray>("array"), BRRESError);
    EXPECT_NO_THROW(mdl0->remove<MDL0::NormalArray>("array"));
    EXPECT_THROW(mdl0->remove<MDL0::NormalArray>("array"), BRRESError);

    EXPECT_THROW(mdl0->get<MDL0::VertexArray>("array"), BRRESError);
    mdl0->add<MDL0::VertexArray>("array");
    EXPECT_NO_THROW(mdl0->get<MDL0::VertexArray>("array"));
    EXPECT_THROW(mdl0->get<MDL0::TexCoordArray>("array"), BRRESError);
    mdl0->remove<MDL0::VertexArray>("array");
    EXPECT_THROW(mdl0->get<MDL0::VertexArray>("array"), BRRESError);
}

TEST(MDL0BoneTests, MDL0AddBone)
{
    BRRES brres;
    MDL0* mdl0 = brres.add<MDL0>("model");
    EXPECT_EQ(nullptr, mdl0->getRootBone());

    MDL0::Bone* b0 = mdl0->add<MDL0::Bone>("b0");
    EXPECT_EQ(b0, mdl0->getRootBone());
    EXPECT_EQ(nullptr, b0->getParent());
    EXPECT_EQ(nullptr, b0->getFirstChild());
    EXPECT_EQ(nullptr, b0->getNext());
    EXPECT_EQ(nullptr, b0->getPrevious());

    MDL0::Bone* b1 = mdl0->add<MDL0::Bone>("b1");
    EXPECT_EQ(b0, mdl0->getRootBone());
    EXPECT_EQ(nullptr, b0->getParent());
    EXPECT_EQ(nullptr, b1->getParent());
    EXPECT_EQ(nullptr, b0->getFirstChild());
    EXPECT_EQ(nullptr, b1->getFirstChild());
    EXPECT_EQ(b1, b0->getNext());
    EXPECT_EQ(nullptr, b1->getNext());
    EXPECT_EQ(nullptr, b0->getPrevious());
    EXPECT_EQ(b0, b1->getPrevious());

    MDL0::Bone* a0 = mdl0->add<MDL0::Bone>("a0");
    EXPECT_EQ(a0, mdl0->getRootBone());
    EXPECT_EQ(nullptr, a0->getParent());
    EXPECT_EQ(nullptr, b0->getParent());
    EXPECT_EQ(nullptr, b1->getParent());
    EXPECT_EQ(nullptr, a0->getFirstChild());
    EXPECT_EQ(nullptr, b0->getFirstChild());
    EXPECT_EQ(nullptr, b1->getFirstChild());
    EXPECT_EQ(b0, a0->getNext());
    EXPECT_EQ(b1, b0->getNext());
    EXPECT_EQ(nullptr, b1->getNext());
    EXPECT_EQ(nullptr, a0->getPrevious());
    EXPECT_EQ(a0, b0->getPrevious());
    EXPECT_EQ(b0, b1->getPrevious());
}

TEST(MDL0BoneTests, Insert)
{
    BRRES brres;
    MDL0* mdl0 = brres.add<MDL0>("model");

    MDL0::Bone* b0 = mdl0->add<MDL0::Bone>("b0");
    MDL0::Bone* c0 = b0->insert("c0");
    EXPECT_EQ(b0, mdl0->getRootBone());
    EXPECT_EQ(nullptr, b0->getParent());
    EXPECT_EQ(b0, c0->getParent());
    EXPECT_EQ(c0, b0->getFirstChild());
    EXPECT_EQ(nullptr, c0->getFirstChild());
    EXPECT_EQ(nullptr, b0->getNext());
    EXPECT_EQ(nullptr, c0->getNext());
    EXPECT_EQ(nullptr, b0->getPrevious());
    EXPECT_EQ(nullptr, c0->getPrevious());

    MDL0::Bone* a0 = b0->insert("a0");
    EXPECT_EQ(b0, mdl0->getRootBone());
    EXPECT_EQ(b0, a0->getParent());
    EXPECT_EQ(nullptr, b0->getParent());
    EXPECT_EQ(b0, c0->getParent());
    EXPECT_EQ(nullptr, a0->getFirstChild());
    EXPECT_EQ(a0, b0->getFirstChild());
    EXPECT_EQ(nullptr, c0->getFirstChild());
    EXPECT_EQ(c0, a0->getNext());
    EXPECT_EQ(nullptr, b0->getNext());
    EXPECT_EQ(nullptr, c0->getNext());
    EXPECT_EQ(nullptr, a0->getPrevious());
    EXPECT_EQ(nullptr, b0->getPrevious());
    EXPECT_EQ(a0, c0->getPrevious());

    MDL0::Bone* c1 = c0->insert("c1");
    MDL0::Bone* b1 = b0->insert("b1");
    MDL0::Bone* c2 = c0->insert("c2");
    EXPECT_EQ(b0, mdl0->getRootBone());
    EXPECT_EQ(b0, a0->getParent());
    EXPECT_EQ(nullptr, b0->getParent());
    EXPECT_EQ(b0, b1->getParent());
    EXPECT_EQ(b0, c0->getParent());
    EXPECT_EQ(c0, c1->getParent());
    EXPECT_EQ(c0, c2->getParent());
    EXPECT_EQ(nullptr, a0->getFirstChild());
    EXPECT_EQ(a0, b0->getFirstChild());
    EXPECT_EQ(nullptr, b1->getFirstChild());
    EXPECT_EQ(c1, c0->getFirstChild());
    EXPECT_EQ(nullptr, c1->getFirstChild());
    EXPECT_EQ(nullptr, c2->getFirstChild());
    EXPECT_EQ(b1, a0->getNext());
    EXPECT_EQ(nullptr, b0->getNext());
    EXPECT_EQ(c0, b1->getNext());
    EXPECT_EQ(nullptr, c0->getNext());
    EXPECT_EQ(c2, c1->getNext());
    EXPECT_EQ(nullptr, c2->getNext());
    EXPECT_EQ(nullptr, a0->getPrevious());
    EXPECT_EQ(nullptr, b0->getPrevious());
    EXPECT_EQ(a0, b1->getPrevious());
    EXPECT_EQ(b1, c0->getPrevious());
    EXPECT_EQ(nullptr, c1->getPrevious());
    EXPECT_EQ(c1, c2->getPrevious());
}

TEST(MDL0BoneTests, MoveTo)
{
    BRRES brres;
    MDL0* mdl0 = brres.add<MDL0>("model");

    MDL0::Bone* b0 = mdl0->add<MDL0::Bone>("b0");
    MDL0::Bone* b1 = mdl0->add<MDL0::Bone>("b1");

    b1->moveTo(b0);
    EXPECT_EQ(b0, mdl0->getRootBone());
    EXPECT_EQ(nullptr, b0->getParent());
    EXPECT_EQ(b0, b1->getParent());
    EXPECT_EQ(b1, b0->getFirstChild());
    EXPECT_EQ(nullptr, b1->getFirstChild());
    EXPECT_EQ(nullptr, b0->getNext());
    EXPECT_EQ(nullptr, b1->getNext());
    EXPECT_EQ(nullptr, b0->getPrevious());
    EXPECT_EQ(nullptr, b1->getPrevious());

    MDL0::Bone* a0 = mdl0->add<MDL0::Bone>("a0");
    b1->moveTo(a0);
    EXPECT_EQ(a0, mdl0->getRootBone());
    EXPECT_EQ(nullptr, a0->getParent());
    EXPECT_EQ(nullptr, b0->getParent());
    EXPECT_EQ(a0, b1->getParent());
    EXPECT_EQ(b1, a0->getFirstChild());
    EXPECT_EQ(nullptr, b0->getFirstChild());
    EXPECT_EQ(nullptr, b1->getFirstChild());
    EXPECT_EQ(b0, a0->getNext());
    EXPECT_EQ(nullptr, b0->getNext());
    EXPECT_EQ(nullptr, b1->getNext());
    EXPECT_EQ(nullptr, a0->getPrevious());
    EXPECT_EQ(a0, b0->getPrevious());
    EXPECT_EQ(nullptr, b1->getPrevious());

    MDL0::Bone* c0 = mdl0->add<MDL0::Bone>("c0");
    a0->moveTo(c0);
    EXPECT_EQ(b0, mdl0->getRootBone());
    EXPECT_EQ(c0, a0->getParent());
    EXPECT_EQ(nullptr, b0->getParent());
    EXPECT_EQ(a0, b1->getParent());
    EXPECT_EQ(nullptr, c0->getParent());
    EXPECT_EQ(b1, a0->getFirstChild());
    EXPECT_EQ(nullptr, b0->getFirstChild());
    EXPECT_EQ(nullptr, b1->getFirstChild());
    EXPECT_EQ(a0, c0->getFirstChild());
    EXPECT_EQ(nullptr, a0->getNext());
    EXPECT_EQ(c0, b0->getNext());
    EXPECT_EQ(nullptr, b1->getNext());
    EXPECT_EQ(nullptr, c0->getNext());
    EXPECT_EQ(nullptr, a0->getPrevious());
    EXPECT_EQ(nullptr, b0->getPrevious());
    EXPECT_EQ(nullptr, b1->getPrevious());
    EXPECT_EQ(b0, c0->getPrevious());

    b0->moveTo(c0);
    EXPECT_EQ(c0, mdl0->getRootBone());
    EXPECT_EQ(c0, a0->getParent());
    EXPECT_EQ(c0, b0->getParent());
    EXPECT_EQ(a0, b1->getParent());
    EXPECT_EQ(nullptr, c0->getParent());
    EXPECT_EQ(b1, a0->getFirstChild());
    EXPECT_EQ(nullptr, b0->getFirstChild());
    EXPECT_EQ(nullptr, b1->getFirstChild());
    EXPECT_EQ(a0, c0->getFirstChild());
    EXPECT_EQ(b0, a0->getNext());
    EXPECT_EQ(nullptr, b0->getNext());
    EXPECT_EQ(nullptr, b1->getNext());
    EXPECT_EQ(nullptr, c0->getNext());
    EXPECT_EQ(nullptr, a0->getPrevious());
    EXPECT_EQ(a0, b0->getPrevious());
    EXPECT_EQ(nullptr, b1->getPrevious());
    EXPECT_EQ(nullptr, c0->getPrevious());
}

TEST(MDL0BoneTests, MDL0RemoveBone)
{
    BRRES brres;
    MDL0* mdl0 = brres.add<MDL0>("model");

    MDL0::Bone* b0 = mdl0->add<MDL0::Bone>("b0");
    MDL0::Bone* b1 = b0->insert("b1");
    MDL0::Bone* b2 = b0->insert("b2");
    MDL0::Bone* b3 = b2->insert("b3");
    MDL0::Bone* a0 = b2->insert("a0");
    EXPECT_EQ(b0, mdl0->getRootBone());
    EXPECT_EQ(nullptr, b0->getParent());
    EXPECT_EQ(b0, b1->getParent());
    EXPECT_EQ(b0, b2->getParent());
    EXPECT_EQ(b2, b3->getParent());
    EXPECT_EQ(b2, a0->getParent());
    EXPECT_EQ(b1, b0->getFirstChild());
    EXPECT_EQ(nullptr, b1->getFirstChild());
    EXPECT_EQ(a0, b2->getFirstChild());
    EXPECT_EQ(nullptr, b3->getFirstChild());
    EXPECT_EQ(nullptr, a0->getFirstChild());
    EXPECT_EQ(nullptr, b0->getNext());
    EXPECT_EQ(b2, b1->getNext());
    EXPECT_EQ(nullptr, b2->getNext());
    EXPECT_EQ(nullptr, b3->getNext());
    EXPECT_EQ(b3, a0->getNext());
    EXPECT_EQ(nullptr, b0->getPrevious());
    EXPECT_EQ(nullptr, b1->getPrevious());
    EXPECT_EQ(b1, b2->getPrevious());
    EXPECT_EQ(a0, b3->getPrevious());
    EXPECT_EQ(nullptr, a0->getPrevious());

    mdl0->remove<MDL0::Bone>("b2");
    EXPECT_EQ(b0, mdl0->getRootBone());
    EXPECT_EQ(nullptr, b0->getParent());
    EXPECT_EQ(b0, b1->getParent());
    EXPECT_EQ(b0, b3->getParent());
    EXPECT_EQ(b0, a0->getParent());
    EXPECT_EQ(a0, b0->getFirstChild());
    EXPECT_EQ(nullptr, b1->getFirstChild());
    EXPECT_EQ(nullptr, b3->getFirstChild());
    EXPECT_EQ(nullptr, a0->getFirstChild());
    EXPECT_EQ(nullptr, b0->getNext());
    EXPECT_EQ(b3, b1->getNext());
    EXPECT_EQ(nullptr, b3->getNext());
    EXPECT_EQ(b1, a0->getNext());
    EXPECT_EQ(nullptr, b0->getPrevious());
    EXPECT_EQ(a0, b1->getPrevious());
    EXPECT_EQ(b1, b3->getPrevious());
    EXPECT_EQ(nullptr, a0->getPrevious());

    mdl0->remove<MDL0::Bone>("b0");
    EXPECT_EQ(a0, mdl0->getRootBone());
    EXPECT_EQ(nullptr, b1->getParent());
    EXPECT_EQ(nullptr, b3->getParent());
    EXPECT_EQ(nullptr, a0->getParent());
    EXPECT_EQ(nullptr, b1->getFirstChild());
    EXPECT_EQ(nullptr, b3->getFirstChild());
    EXPECT_EQ(nullptr, a0->getFirstChild());
    EXPECT_EQ(b3, b1->getNext());
    EXPECT_EQ(nullptr, b3->getNext());
    EXPECT_EQ(b1, a0->getNext());
    EXPECT_EQ(a0, b1->getPrevious());
    EXPECT_EQ(b1, b3->getPrevious());
    EXPECT_EQ(nullptr, a0->getPrevious());
}

TEST(MDL0BoneTests, FlatNext)
{
    BRRES brres;
    MDL0* mdl0 = brres.add<MDL0>("model");
    EXPECT_EQ(nullptr, MDL0::Bone::flatNext(nullptr));

    MDL0::Bone* b0 = mdl0->add<MDL0::Bone>("b0");
    EXPECT_EQ(b0, mdl0->getRootBone());
    EXPECT_EQ(nullptr, MDL0::Bone::flatNext(b0));

    MDL0::Bone* a0 = mdl0->add<MDL0::Bone>("a0");
    EXPECT_EQ(a0, mdl0->getRootBone());
    EXPECT_EQ(b0, MDL0::Bone::flatNext(a0));
    EXPECT_EQ(nullptr, MDL0::Bone::flatNext(b0));

    MDL0::Bone* b1 = b0->insert("b1");
    EXPECT_EQ(a0, mdl0->getRootBone());
    EXPECT_EQ(b0, MDL0::Bone::flatNext(a0));
    EXPECT_EQ(b1, MDL0::Bone::flatNext(b0));
    EXPECT_EQ(nullptr, MDL0::Bone::flatNext(b1));

    MDL0::Bone* a1 = a0->insert("a1");
    EXPECT_EQ(a0, mdl0->getRootBone());
    EXPECT_EQ(a1, MDL0::Bone::flatNext(a0));
    EXPECT_EQ(b0, MDL0::Bone::flatNext(a1));
    EXPECT_EQ(b1, MDL0::Bone::flatNext(b0));
    EXPECT_EQ(nullptr, MDL0::Bone::flatNext(b1));

    MDL0::Bone* c0 = mdl0->add<MDL0::Bone>("c0");
    MDL0::Bone* c1 = c0->insert("c1");
    MDL0::Bone* a2 = a1->insert("a2");
    MDL0::Bone* a3 = a2->insert("a3");
    MDL0::Bone* b2 = b1->insert("b2");
    MDL0::Bone* b3 = b0->insert("b3");
    EXPECT_EQ(a0, mdl0->getRootBone());
    EXPECT_EQ(a1, MDL0::Bone::flatNext(a0));
    EXPECT_EQ(a2, MDL0::Bone::flatNext(a1));
    EXPECT_EQ(a3, MDL0::Bone::flatNext(a2));
    EXPECT_EQ(b0, MDL0::Bone::flatNext(a3));
    EXPECT_EQ(b1, MDL0::Bone::flatNext(b0));
    EXPECT_EQ(b2, MDL0::Bone::flatNext(b1));
    EXPECT_EQ(b3, MDL0::Bone::flatNext(b2));
    EXPECT_EQ(c0, MDL0::Bone::flatNext(b3));
    EXPECT_EQ(c1, MDL0::Bone::flatNext(c0));
    EXPECT_EQ(nullptr, MDL0::Bone::flatNext(c1));
}

TEST(MDL0BoneTests, Errors)
{
    BRRES brres;
    MDL0* mdl0 = brres.add<MDL0>("model");

    MDL0::Bone* b0 = mdl0->add<MDL0::Bone>("b0");
    EXPECT_THROW(b0->insert("b0"), BRRESError);

    MDL0::Bone* b1 = b0->insert("b1");
    EXPECT_THROW(b1->insert("b0"), BRRESError);

    EXPECT_THROW(b0->moveTo(b1), BRRESError);
    EXPECT_NO_THROW(b1->moveTo(b0));

    b1->moveTo(nullptr);
    EXPECT_NO_THROW(b0->moveTo(b1));
    EXPECT_THROW(b1->moveTo(b0), BRRESError);

    EXPECT_THROW(b0->getChild("b1"), BRRESError);
    EXPECT_THROW(b0->getChild("bone"), BRRESError);

    EXPECT_NO_THROW(b1->getChild("b0"));
    b0->moveTo(nullptr);
    EXPECT_THROW(b1->getChild("b0"), BRRESError);
}

TEST(MDL0VertexArrayTests, SetData)
{
    BRRES brres;
    MDL0* mdl0 = brres.add<MDL0>("model");
    MDL0::VertexArray* va = mdl0->add<MDL0::VertexArray>("vertices");

    EXPECT_EQ(0, va->getCount());
    EXPECT_EQ(Buffer(), va->getData());
    EXPECT_EQ(MDL0::VertexArray::Components::XYZ, va->getComponentsType());
    EXPECT_EQ(MDL0::VertexArray::Format::Float, va->getFormat());
    EXPECT_EQ(0, va->getDivisor());
    EXPECT_EQ(Vector3f(0, 0, 0), va->getBoxMin());
    EXPECT_EQ(Vector3f(0, 0, 0), va->getBoxMax());

    Buffer buffer(7 * 4 * 3);
    buffer.putFloat(3.5f).putFloat(1.f).putFloat(0.f);
    buffer.putFloat(-.5f).putFloat(0.f).putFloat(4.f);
    buffer.putFloat(1.f).putFloat(.5f).putFloat(-3.5f);
    buffer.putFloat(2.f).putFloat(1.5f).putFloat(-1.f);
    buffer.putFloat(-2.5f).putFloat(-1.f).putFloat(.5f);
    buffer.putFloat(.5f).putFloat(-2.f).putFloat(1.5f);
    buffer.putFloat(0.f).putFloat(3.f).putFloat(-.5f);
    buffer.flip();

    va->setData(buffer);
    EXPECT_EQ(7, va->getCount());
    EXPECT_EQ(buffer, va->getData());
    EXPECT_EQ(MDL0::VertexArray::Components::XYZ, va->getComponentsType());
    EXPECT_EQ(MDL0::VertexArray::Format::Float, va->getFormat());
    EXPECT_EQ(0, va->getDivisor());
    EXPECT_EQ(Vector3f(-2.5f, -2.f, -3.5f), va->getBoxMin());
    EXPECT_EQ(Vector3f(3.5f, 3.f, 4.f), va->getBoxMax());

    va->setData(Buffer(), MDL0::VertexArray::Components::XY);
    EXPECT_EQ(0, va->getCount());
    EXPECT_EQ(Buffer(), va->getData());
    EXPECT_EQ(MDL0::VertexArray::Components::XY, va->getComponentsType());
    EXPECT_EQ(MDL0::VertexArray::Format::Float, va->getFormat());
    EXPECT_EQ(0, va->getDivisor());
    EXPECT_EQ(Vector3f(0, 0, 0), va->getBoxMin());
    EXPECT_EQ(Vector3f(0, 0, 0), va->getBoxMax());

    buffer = Buffer(3 * 3 * 2);
    buffer.putShort(100).putShort(256).putShort(64);
    buffer.putShort(512).putShort(40000).putShort(128);
    buffer.putShort(50000).putShort(32).putShort(2000);
    buffer.flip();

    va->setData(buffer, MDL0::VertexArray::Components::XYZ, MDL0::VertexArray::Format::Int16);
    EXPECT_EQ(3, va->getCount());
    EXPECT_EQ(buffer, va->getData());
    EXPECT_EQ(MDL0::VertexArray::Components::XYZ, va->getComponentsType());
    EXPECT_EQ(MDL0::VertexArray::Format::Int16, va->getFormat());
    EXPECT_EQ(0, va->getDivisor());
    EXPECT_EQ(Vector3f(-15536.f, -25536.f, 64.f), va->getBoxMin());
    EXPECT_EQ(Vector3f(512.f, 256.f, 2000.f), va->getBoxMax());

    buffer.flip();
    va->setDivisor(3);
    va->setData(buffer, MDL0::VertexArray::Components::XYZ, MDL0::VertexArray::Format::Int16);
    EXPECT_EQ(3, va->getCount());
    EXPECT_EQ(buffer, va->getData());
    EXPECT_EQ(MDL0::VertexArray::Components::XYZ, va->getComponentsType());
    EXPECT_EQ(MDL0::VertexArray::Format::Int16, va->getFormat());
    EXPECT_EQ(3, va->getDivisor());
    EXPECT_EQ(Vector3f(-1942.f, -3192.f, 8.f), va->getBoxMin());
    EXPECT_EQ(Vector3f(64.f, 32.f, 250.f), va->getBoxMax());
}

TEST(MDL0NormalArrayTests, SetData)
{
    BRRES brres;
    MDL0* mdl0 = brres.add<MDL0>("model");
    MDL0::NormalArray* na = mdl0->add<MDL0::NormalArray>("normals");

    EXPECT_EQ(0, na->getCount());
    EXPECT_EQ(Buffer(), na->getData());
    EXPECT_EQ(MDL0::NormalArray::Components::Normal, na->getComponentsType());
    EXPECT_EQ(MDL0::NormalArray::Format::Float, na->getFormat());
    EXPECT_EQ(0, na->getDivisor());

    Buffer buffer(4 * 4 * 3);
    buffer.putFloat(0.f).putFloat(1.f).putFloat(0.f);
    buffer.putFloat(1.f).putFloat(0.f).putFloat(0.f);
    buffer.putFloat(0.f).putFloat(-1.f).putFloat(0.f);
    buffer.putFloat(0.f).putFloat(0.f).putFloat(-1.f);
    buffer.flip();

    na->setData(buffer);
    EXPECT_EQ(4, na->getCount());
    EXPECT_EQ(buffer, na->getData());
    EXPECT_EQ(MDL0::NormalArray::Components::Normal, na->getComponentsType());
    EXPECT_EQ(MDL0::NormalArray::Format::Float, na->getFormat());
    EXPECT_EQ(0, na->getDivisor());

    na->setData(Buffer(), MDL0::NormalArray::Components::Normal_BiNormal_Tangent);
    EXPECT_EQ(0, na->getCount());
    EXPECT_EQ(Buffer(), na->getData());
    EXPECT_EQ(MDL0::NormalArray::Components::Normal_BiNormal_Tangent, na->getComponentsType());
    EXPECT_EQ(MDL0::NormalArray::Format::Float, na->getFormat());
    EXPECT_EQ(0, na->getDivisor());

    buffer = Buffer(5 * 3 * 2);
    buffer.putShort(0).putShort(0).putShort(32767);
    buffer.putShort(32768).putShort(0).putShort(0);
    buffer.putShort(32767).putShort(0).putShort(0);
    buffer.putShort(0).putShort(32767).putShort(0);
    buffer.putShort(0).putShort(0).putShort(32768);
    buffer.flip();

    na->setDivisor(14);
    na->setData(buffer, MDL0::NormalArray::Components::Normal, MDL0::NormalArray::Format::Int16);
    EXPECT_EQ(5, na->getCount());
    EXPECT_EQ(buffer, na->getData());
    EXPECT_EQ(MDL0::NormalArray::Components::Normal, na->getComponentsType());
    EXPECT_EQ(MDL0::NormalArray::Format::Int16, na->getFormat());
    EXPECT_EQ(14, na->getDivisor());
}

TEST(MDL0ColourArrayTests, SetData)
{
    BRRES brres;
    MDL0* mdl0 = brres.add<MDL0>("model");
    MDL0::ColourArray* ca = mdl0->add<MDL0::ColourArray>("colours");

    EXPECT_EQ(0, ca->getCount());
    EXPECT_EQ(Buffer(), ca->getData());
    EXPECT_EQ(MDL0::ColourArray::Format::RGBA8, ca->getFormat());

    Buffer buffer(11 * 3);
    buffer.put(0xFF).put(0x00).put(0x00);
    buffer.put(0x00).put(0xFF).put(0x00);
    buffer.put(0x00).put(0x00).put(0xFF);
    buffer.put(0xFF).put(0xFF).put(0x00);
    buffer.put(0xFF).put(0x7F).put(0x00);
    buffer.put(0xFF).put(0x00).put(0xFF);
    buffer.put(0x7F).put(0x00).put(0xFF);
    buffer.put(0x00).put(0xFF).put(0xFF);
    buffer.put(0x00).put(0x00).put(0x00);
    buffer.put(0x7F).put(0x7F).put(0x7F);
    buffer.put(0xFF).put(0xFF).put(0xFF);
    buffer.flip();

    ca->setData(buffer, MDL0::ColourArray::Format::RGB8);
    EXPECT_EQ(11, ca->getCount());
    EXPECT_EQ(buffer, ca->getData());
    EXPECT_EQ(MDL0::ColourArray::Format::RGB8, ca->getFormat());

    ca->setData(Buffer(), MDL0::ColourArray::Format::RGB565);
    EXPECT_EQ(0, ca->getCount());
    EXPECT_EQ(Buffer(), ca->getData());
    EXPECT_EQ(MDL0::ColourArray::Format::RGB565, ca->getFormat());
}

TEST(MDL0TexCoordArrayTests, SetData)
{
    BRRES brres;
    MDL0* mdl0 = brres.add<MDL0>("model");
    MDL0::TexCoordArray* tca = mdl0->add<MDL0::TexCoordArray>("#0");

    EXPECT_EQ(0, tca->getCount());
    EXPECT_EQ(Buffer(), tca->getData());
    EXPECT_EQ(MDL0::TexCoordArray::Components::ST, tca->getComponentsType());
    EXPECT_EQ(MDL0::TexCoordArray::Format::Float, tca->getFormat());
    EXPECT_EQ(0, tca->getDivisor());
    EXPECT_EQ(Vector2f(0.f, 0.f), tca->getBoxMin());
    EXPECT_EQ(Vector2f(0.f, 0.f), tca->getBoxMax());

    Buffer buffer(6 * 4 * 2);
    buffer.putFloat(0.f).putFloat(.85f);
    buffer.putFloat(.25f).putFloat(.35f);
    buffer.putFloat(.675f).putFloat(.4f);
    buffer.putFloat(.875f).putFloat(.5f);
    buffer.putFloat(.125f).putFloat(.25f);
    buffer.putFloat(.45f).putFloat(.6f);
    buffer.flip();

    tca->setData(buffer);
    EXPECT_EQ(6, tca->getCount());
    EXPECT_EQ(buffer, tca->getData());
    EXPECT_EQ(MDL0::TexCoordArray::Components::ST, tca->getComponentsType());
    EXPECT_EQ(MDL0::TexCoordArray::Format::Float, tca->getFormat());
    EXPECT_EQ(0, tca->getDivisor());
    EXPECT_EQ(Vector2f(0.f, .25f), tca->getBoxMin());
    EXPECT_EQ(Vector2f(.875f, .85f), tca->getBoxMax());

    tca->setData(buffer, MDL0::TexCoordArray::Components::S);
    EXPECT_EQ(0, tca->getCount());
    EXPECT_EQ(Buffer(), tca->getData());
    EXPECT_EQ(MDL0::TexCoordArray::Components::S, tca->getComponentsType());
    EXPECT_EQ(MDL0::TexCoordArray::Format::Float, tca->getFormat());
    EXPECT_EQ(0, tca->getDivisor());
    EXPECT_EQ(Vector2f(0.f, 0.f), tca->getBoxMin());
    EXPECT_EQ(Vector2f(0.f, 0.f), tca->getBoxMax());

    buffer = Buffer(4 * 2);
    buffer.put(32).put(0);
    buffer.put(248).put(128);
    buffer.put(64).put(16);
    buffer.put(128).put(32);
    buffer.flip();

    tca->setDivisor(8);
    tca->setData(buffer, MDL0::TexCoordArray::Components::ST, MDL0::TexCoordArray::Format::UInt8);
    EXPECT_EQ(4, tca->getCount());
    EXPECT_EQ(buffer, tca->getData());
    EXPECT_EQ(MDL0::TexCoordArray::Components::ST, tca->getComponentsType());
    EXPECT_EQ(MDL0::TexCoordArray::Format::UInt8, tca->getFormat());
    EXPECT_EQ(8, tca->getDivisor());
    EXPECT_EQ(Vector2f(.125f, 0.f), tca->getBoxMin());
    EXPECT_EQ(Vector2f(.96875f, .5f), tca->getBoxMax());
}

TEST(TEX0Tests, SetData)
{
    BRRES brres;
    TEX0* tex0 = brres.add<TEX0>("texture");

    Image image = ImageIO::read(CT_LIB_TESTS_DATA_DIR"/Images/TEX0/Sand.png");
    tex0->setTextureData(image, ImageFormat::RGB565);

    EXPECT_EQ(image.getWidth(), tex0->getWidth());
    EXPECT_EQ(image.getHeight(), tex0->getHeight());
    EXPECT_EQ(ImageFormat::RGB565, tex0->getFormat());
    EXPECT_EQ(image.getWidth() * image.getHeight() * 2, tex0->getTextureData().capacity());
}

TEST(TEX0Tests, MipmapSizes)
{
    BRRES brres;
    TEX0* tex0 = brres.add<TEX0>("texture");
    
    Image image = ImageIO::read(CT_LIB_TESTS_DATA_DIR"/Images/TEX0/Sand.png");
    tex0->setTextureData(image, ImageFormat::RGB565);

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
    BRRES brres;
    TEX0* tex0 = brres.add<TEX0>("texture");
    
    Image image = ImageIO::read(CT_LIB_TESTS_DATA_DIR"/Images/TEX0/Sand.png");
    tex0->setTextureData(image, ImageFormat::RGB565);
    tex0->generateMipmaps(6, image);
    
    // some values are not what they should be because the RGB565 encoder uses 4x4 blocks
    EXPECT_EQ(320 * 240 * 2, tex0->getMipmapTextureData(0).capacity());
    EXPECT_EQ(160 * 120 * 2, tex0->getMipmapTextureData(1).capacity());
    EXPECT_EQ( 80 *  60 * 2, tex0->getMipmapTextureData(2).capacity());
    EXPECT_EQ( 40 *  32 * 2, tex0->getMipmapTextureData(3).capacity());
    EXPECT_EQ( 20 *  16 * 2, tex0->getMipmapTextureData(4).capacity());
    EXPECT_EQ( 12 *   8 * 2, tex0->getMipmapTextureData(5).capacity());

    EXPECT_THROW(tex0->generateMipmaps(9), BRRESError);
}
