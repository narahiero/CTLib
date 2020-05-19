//////////////////////////////////////////////////
//  Copyright (c) 2020 Nara Hiero
//
// This file is licensed under GPLv3+
// Refer to the `License.txt` file included.
//////////////////////////////////////////////////

#include <gtest/gtest.h>

#include <CTLib/Model.hpp>

TEST(DataFormatTests, DefaultCtor)
{
    CTLib::Model::DataFormat format;

    EXPECT_EQ(CTLib::Model::DataType::Float, format.getType());
    EXPECT_EQ(3, format.getSize());
    EXPECT_EQ(3, format.getCount());
    EXPECT_EQ(false, format.isIndexed());
    EXPECT_EQ(CTLib::Model::DataType::UInt16, format.getIndexType());
}

TEST(DataFormatTests, TypeCtor)
{
    CTLib::Model::DataFormat posFormat(CTLib::Model::Type::Position0);

    EXPECT_EQ(CTLib::Model::DataType::Float, posFormat.getType());
    EXPECT_EQ(3, posFormat.getSize());
    EXPECT_EQ(3, posFormat.getCount());
    EXPECT_EQ(false, posFormat.isIndexed());
    EXPECT_EQ(CTLib::Model::DataType::UInt16, posFormat.getIndexType());

    CTLib::Model::DataFormat normFormat(CTLib::Model::Type::Position0);

    EXPECT_EQ(CTLib::Model::DataType::Float, normFormat.getType());
    EXPECT_EQ(3, normFormat.getSize());
    EXPECT_EQ(3, normFormat.getCount());
    EXPECT_EQ(false, normFormat.isIndexed());
    EXPECT_EQ(CTLib::Model::DataType::UInt16, normFormat.getIndexType());

    CTLib::Model::DataFormat texFormat(CTLib::Model::Type::TexCoord0);

    EXPECT_EQ(CTLib::Model::DataType::Float, texFormat.getType());
    EXPECT_EQ(2, texFormat.getSize());
    EXPECT_EQ(3, texFormat.getCount());
    EXPECT_EQ(false, texFormat.isIndexed());
    EXPECT_EQ(CTLib::Model::DataType::UInt16, texFormat.getIndexType());

    EXPECT_EQ(CTLib::Model::DataFormat(CTLib::Model::Type::TexCoord1), texFormat);
    EXPECT_EQ(CTLib::Model::DataFormat(CTLib::Model::Type::TexCoord2), texFormat);
    EXPECT_EQ(CTLib::Model::DataFormat(CTLib::Model::Type::TexCoord3), texFormat);
    EXPECT_EQ(CTLib::Model::DataFormat(CTLib::Model::Type::TexCoord4), texFormat);
    EXPECT_EQ(CTLib::Model::DataFormat(CTLib::Model::Type::TexCoord5), texFormat);
    EXPECT_EQ(CTLib::Model::DataFormat(CTLib::Model::Type::TexCoord6), texFormat);
    EXPECT_EQ(CTLib::Model::DataFormat(CTLib::Model::Type::TexCoord7), texFormat);

    CTLib::Model::DataFormat colFormat(CTLib::Model::Type::Colour0);

    EXPECT_EQ(CTLib::Model::DataType::UInt32, colFormat.getType());
    EXPECT_EQ(1, colFormat.getSize());
    EXPECT_EQ(3, colFormat.getCount());
    EXPECT_EQ(false, colFormat.isIndexed());
    EXPECT_EQ(CTLib::Model::DataType::UInt16, colFormat.getIndexType());

    EXPECT_EQ(CTLib::Model::DataFormat(CTLib::Model::Type::Colour1), colFormat);

    CTLib::Model::DataFormat kclFormat(CTLib::Model::Type::KCLFlag);

    EXPECT_EQ(CTLib::Model::DataType::UInt16, kclFormat.getType());
    EXPECT_EQ(1, kclFormat.getSize());
    EXPECT_EQ(1, kclFormat.getCount());
    EXPECT_EQ(false, kclFormat.isIndexed());
    EXPECT_EQ(CTLib::Model::DataType::UInt16, kclFormat.getIndexType());
}

TEST(DataFormatTests, Errors)
{
    CTLib::Model::DataFormat format;

    EXPECT_THROW(format.setSize(0), CTLib::ModelError);
    EXPECT_THROW(format.setCount(0), CTLib::ModelError);

    EXPECT_THROW(format.setIndexType(CTLib::Model::DataType::Int8), CTLib::ModelError);
    EXPECT_THROW(format.setIndexType(CTLib::Model::DataType::Int16), CTLib::ModelError);
    EXPECT_THROW(format.setIndexType(CTLib::Model::DataType::Int32), CTLib::ModelError);
    EXPECT_THROW(format.setIndexType(CTLib::Model::DataType::Float), CTLib::ModelError);
    EXPECT_THROW(format.setIndexType(CTLib::Model::DataType::Double), CTLib::ModelError);
}

TEST(ModelTests, NameOfType)
{
    EXPECT_STREQ("Position0", CTLib::Model::nameOf(CTLib::Model::Type::Position0));
    EXPECT_STREQ("Normal0", CTLib::Model::nameOf(CTLib::Model::Type::Normal0));
    EXPECT_STREQ("TexCoord0", CTLib::Model::nameOf(CTLib::Model::Type::TexCoord0));
    EXPECT_STREQ("TexCoord1", CTLib::Model::nameOf(CTLib::Model::Type::TexCoord1));
    EXPECT_STREQ("TexCoord2", CTLib::Model::nameOf(CTLib::Model::Type::TexCoord2));
    EXPECT_STREQ("TexCoord3", CTLib::Model::nameOf(CTLib::Model::Type::TexCoord3));
    EXPECT_STREQ("TexCoord4", CTLib::Model::nameOf(CTLib::Model::Type::TexCoord4));
    EXPECT_STREQ("TexCoord5", CTLib::Model::nameOf(CTLib::Model::Type::TexCoord5));
    EXPECT_STREQ("TexCoord6", CTLib::Model::nameOf(CTLib::Model::Type::TexCoord6));
    EXPECT_STREQ("TexCoord7", CTLib::Model::nameOf(CTLib::Model::Type::TexCoord7));
    EXPECT_STREQ("Colour0", CTLib::Model::nameOf(CTLib::Model::Type::Colour0));
    EXPECT_STREQ("Colour1", CTLib::Model::nameOf(CTLib::Model::Type::Colour1));
    EXPECT_STREQ("KCLFlag", CTLib::Model::nameOf(CTLib::Model::Type::KCLFlag));
}

TEST(ModelTests, NameOfDataType)
{
    EXPECT_STREQ("UInt8", CTLib::Model::nameOf(CTLib::Model::DataType::UInt8));
    EXPECT_STREQ("Int8", CTLib::Model::nameOf(CTLib::Model::DataType::Int8));
    EXPECT_STREQ("UInt16", CTLib::Model::nameOf(CTLib::Model::DataType::UInt16));
    EXPECT_STREQ("Int16", CTLib::Model::nameOf(CTLib::Model::DataType::Int16));
    EXPECT_STREQ("UInt32", CTLib::Model::nameOf(CTLib::Model::DataType::UInt32));
    EXPECT_STREQ("Int32", CTLib::Model::nameOf(CTLib::Model::DataType::Int32));
    EXPECT_STREQ("Float", CTLib::Model::nameOf(CTLib::Model::DataType::Float));
    EXPECT_STREQ("Double", CTLib::Model::nameOf(CTLib::Model::DataType::Double));
}

TEST(ModelTests, DefaultCtor)
{
    CTLib::Model model;
    
    EXPECT_EQ(false, model.isGlobalIndexModeEnabled());
    EXPECT_EQ(false, model.isForceUseIndicesEnabled());
    EXPECT_EQ(CTLib::Buffer(), model.getGlobalIndexData());
    EXPECT_EQ(CTLib::Model::DataType::UInt16, model.getGlobalIndexDataType());

    EXPECT_EQ(false, model.hasData(CTLib::Model::Type::Position0));
    EXPECT_EQ(false, model.hasData(CTLib::Model::Type::Normal0));
    EXPECT_EQ(false, model.hasData(CTLib::Model::Type::TexCoord0));
    EXPECT_EQ(false, model.hasData(CTLib::Model::Type::TexCoord6));
    EXPECT_EQ(false, model.hasData(CTLib::Model::Type::Colour0));
    EXPECT_EQ(false, model.hasData(CTLib::Model::Type::KCLFlag));

    EXPECT_EQ(false, model.hasIndexData(CTLib::Model::Type::Position0));
    EXPECT_EQ(false, model.hasIndexData(CTLib::Model::Type::Normal0));
    EXPECT_EQ(false, model.hasIndexData(CTLib::Model::Type::TexCoord0));
    EXPECT_EQ(false, model.hasIndexData(CTLib::Model::Type::TexCoord6));
    EXPECT_EQ(false, model.hasIndexData(CTLib::Model::Type::Colour0));
    EXPECT_EQ(false, model.hasIndexData(CTLib::Model::Type::KCLFlag));
}

TEST(ModelTests, SetHasAndGetData)
{
    CTLib::Model model;

    CTLib::Buffer dataPos(0x24);
    dataPos.putFloat(0.f).putFloat(.5f).putFloat(0.f)
        .putFloat(-.5f).putFloat(-.5f).putFloat(0.f)
        .putFloat(.5f).putFloat(-.5f).putFloat(0.f);
    dataPos.flip();

    model.setData(CTLib::Model::Type::Position0, dataPos);

    EXPECT_EQ(true, model.hasData(CTLib::Model::Type::Position0));
    EXPECT_EQ(false, model.hasIndexData(CTLib::Model::Type::Position0));

    EXPECT_EQ(dataPos, model.getData(CTLib::Model::Type::Position0));
    EXPECT_EQ(
        CTLib::Model::DataFormat(CTLib::Model::Type::Position0),
        model.getDataFormat(CTLib::Model::Type::Position0)
    );

    CTLib::Buffer dataTex(0x18);
    dataTex.putFloat(.5f).putFloat(1.f)
        .putFloat(0.f).putFloat(0.f)
        .putFloat(1.f).putFloat(0.f);
    dataTex.flip();

    model.setData(CTLib::Model::Type::TexCoord0, dataTex);

    EXPECT_EQ(true, model.hasData(CTLib::Model::Type::TexCoord0));

    EXPECT_EQ(dataTex, model.getData(CTLib::Model::Type::TexCoord0));
    EXPECT_EQ(
        CTLib::Model::DataFormat(CTLib::Model::Type::TexCoord0),
        model.getDataFormat(CTLib::Model::Type::TexCoord0)
    );
}

TEST(ModelTests, SetGetAndHasIndexData)
{
    CTLib::Model model;

    CTLib::Buffer dataPos(0x30);
    dataPos.putFloat(.5f).putFloat(.5f).putFloat(0.f)
        .putFloat(.5f).putFloat(-.5f).putFloat(0.f)
        .putFloat(-.5f).putFloat(-.5f).putFloat(0.f)
        .putFloat(-.5f).putFloat(.5f).putFloat(0.f);
    dataPos.flip();

    CTLib::Buffer dataTex(0x20);
    dataTex.putFloat(1.f).putFloat(1.f)
        .putFloat(1.f).putFloat(0.f)
        .putFloat(0.f).putFloat(0.f)
        .putFloat(0.f).putFloat(1.f);
    dataTex.flip();

    CTLib::Buffer indexData(0xC);
    indexData.putShort(0).putShort(2).putShort(1)
        .putShort(0).putShort(3).putShort(2);
    indexData.flip();

    CTLib::Model::DataFormat formatPos(CTLib::Model::Type::Position0);
    formatPos.setIsIndexed(true);
    model.setDataFormat(CTLib::Model::Type::Position0, formatPos);
    model.setData(CTLib::Model::Type::Position0, dataPos);
    model.setIndexData(CTLib::Model::Type::Position0, indexData);

    EXPECT_EQ(formatPos, model.getDataFormat(CTLib::Model::Type::Position0));
    EXPECT_EQ(dataPos, model.getData(CTLib::Model::Type::Position0));
    EXPECT_EQ(indexData, model.getIndexData(CTLib::Model::Type::Position0));

    indexData.flip();

    CTLib::Model::DataFormat formatTex(CTLib::Model::Type::TexCoord0);
    formatTex.setIsIndexed(true);
    model.setDataFormat(CTLib::Model::Type::TexCoord0, formatTex);
    model.setData(CTLib::Model::Type::TexCoord0, dataTex);
    model.setIndexData(CTLib::Model::Type::TexCoord0, indexData);

    EXPECT_EQ(formatTex, model.getDataFormat(CTLib::Model::Type::TexCoord0));
    EXPECT_EQ(dataTex, model.getData(CTLib::Model::Type::TexCoord0));
    EXPECT_EQ(indexData, model.getIndexData(CTLib::Model::Type::TexCoord0));
}

TEST(ModelTests, GetFaceCount)
{
    CTLib::Model model;

    CTLib::Buffer dataPos(0x6C);
    dataPos.putFloat(-1.f).putFloat(1.f).putFloat(0.f)
        .putFloat(0.f).putFloat(.5f).putFloat(0.f)
        .putFloat(1.f).putFloat(1.f).putFloat(0.f)
        .putFloat(.5f).putFloat(0.f).putFloat(0.f)
        .putFloat(1.f).putFloat(-1.f).putFloat(0.f)
        .putFloat(0.f).putFloat(-.5f).putFloat(0.f)
        .putFloat(-1.f).putFloat(-1.f).putFloat(0.f)
        .putFloat(-.5f).putFloat(0.f).putFloat(0.f)
        .putFloat(0.f).putFloat(0.f).putFloat(0.f); // unused in indexed mode
    dataPos.flip();

    model.setData(CTLib::Model::Type::Position0, dataPos);
    EXPECT_EQ(3, model.getFaceCount(CTLib::Model::Type::Position0));

    CTLib::Buffer indexData(0x24);
    indexData.putShort(0).putShort(7).putShort(1)
        .putShort(2).putShort(1).putShort(3)
        .putShort(4).putShort(3).putShort(5)
        .putShort(6).putShort(5).putShort(7)
        .putShort(1).putShort(7).putShort(5)
        .putShort(1).putShort(5).putShort(3);
    indexData.flip();

    model.setIndexData(CTLib::Model::Type::Position0, indexData);
    EXPECT_EQ(3, model.getFaceCount(CTLib::Model::Type::Position0));

    CTLib::Model::DataFormat format(model.getDataFormat(CTLib::Model::Type::Position0));
    format.setIsIndexed(true);
    model.setDataFormat(CTLib::Model::Type::Position0, format);
    EXPECT_EQ(6, model.getFaceCount(CTLib::Model::Type::Position0));

    CTLib::Buffer globalIndexData(0x18);
    globalIndexData.putShort(0).putShort(7).putShort(1)
        .putShort(2).putShort(1).putShort(3)
        .putShort(4).putShort(3).putShort(5)
        .putShort(6).putShort(5).putShort(7);
    globalIndexData.flip();

    model.setGlobalIndexData(globalIndexData, CTLib::Model::DataType::UInt16);
    EXPECT_EQ(6, model.getFaceCount(CTLib::Model::Type::Position0));

    model.setGlobalIndicesMode(true, false);
    EXPECT_EQ(4, model.getFaceCount(CTLib::Model::Type::Position0));

    format.setIsIndexed(false);
    model.setDataFormat(CTLib::Model::Type::Position0, format);
    EXPECT_EQ(3, model.getFaceCount(CTLib::Model::Type::Position0));

    model.setGlobalIndicesMode(true, true);
    EXPECT_EQ(4, model.getFaceCount(CTLib::Model::Type::Position0));

    model.setGlobalIndicesMode(false, true);
    EXPECT_EQ(3, model.getFaceCount(CTLib::Model::Type::Position0));
}
