//////////////////////////////////////////////////
//  Copyright (c) 2020 Nara Hiero
//
// This file is licensed under GPLv3+
// Refer to the `License.txt` file included.
//////////////////////////////////////////////////

#include <gtest/gtest.h>

#include <CTLib/Image.hpp>

#include <CTLib/Utilities.hpp>

#include "Tests.hpp"

using namespace CTLib;

TEST(ImageTests, Test)
{
    uint8_t expect[4 * 4 * 4] = {
        0xFF, 0x00, 0x00, 0xFF,  0x00, 0xFF, 0x00, 0xFF,  0x00, 0x00, 0xFF, 0xFF,  0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0x00, 0xFF,  0xFF, 0xFF, 0x00, 0xFF,  0xFF, 0xFF, 0x00, 0xFF,  0xFF, 0xFF, 0x00, 0xFF,
        0xFF, 0xFF, 0x00, 0xFF,  0xFF, 0xFF, 0x00, 0xFF,  0xFF, 0xFF, 0x00, 0xFF,  0xFF, 0xFF, 0x00, 0xFF,
        0xFF, 0xFF, 0x00, 0xFF,  0xFF, 0xFF, 0x00, 0xFF,  0xFF, 0xFF, 0x00, 0xFF,  0xFF, 0xFF, 0x00, 0xFF,
    };

    Buffer data(0x10);
    data.putInt(0xFF0000FF).putInt(0x00FF00FF).putInt(0x0000FFFF).putInt(0xFFFFFFFF).flip();
    Image image(4, 4, data, {0xFF, 0xFF, 0x00, 0xFF});

    EXPECT_EQ(4, image.getWidth());
    EXPECT_EQ(4, image.getHeight());
    EXPECT_EQ(4 * 4 * 4, image.getData().capacity());
    EXPECT_TRUE(Bytes::matches(expect, *image, 4 * 4 * 4));
}

TEST(ImageCoderTests, I4)
{
    Buffer encoded = IO::readFile(CT_LIB_TESTS_DATA_DIR"/Images/Coder/I4.bin");
    Image decoded = ImageCoder::decode(encoded, 6, 3, ImageFormat::I4);

    Buffer expect = IO::readFile(CT_LIB_TESTS_DATA_DIR"/Images/Coder/I4D.bin");
    Image expectImg(6, 3, expect);

    EXPECT_EQ(expectImg, decoded);
}

TEST(ImageCoderTests, I8)
{
    Buffer encoded = IO::readFile(CT_LIB_TESTS_DATA_DIR"/Images/Coder/I8.bin");
    Image decoded = ImageCoder::decode(encoded, 6, 3, ImageFormat::I8);

    Buffer expect = IO::readFile(CT_LIB_TESTS_DATA_DIR"/Images/Coder/I8D.bin");
    Image expectImg(6, 3, expect);

    EXPECT_EQ(expectImg, decoded);
}

TEST(ImageCoderTests, IA4)
{
    Buffer encoded = IO::readFile(CT_LIB_TESTS_DATA_DIR"/Images/Coder/IA4.bin");
    Image decoded = ImageCoder::decode(encoded, 6, 3, ImageFormat::IA4);

    Buffer expect = IO::readFile(CT_LIB_TESTS_DATA_DIR"/Images/Coder/IA4D.bin");
    Image expectImg(6, 3, expect);

    EXPECT_EQ(expectImg, decoded);
}

TEST(ImageCoderTests, IA8)
{
    Buffer encoded = IO::readFile(CT_LIB_TESTS_DATA_DIR"/Images/Coder/IA8.bin");
    Image decoded = ImageCoder::decode(encoded, 6, 3, ImageFormat::IA8);

    Buffer expect = IO::readFile(CT_LIB_TESTS_DATA_DIR"/Images/Coder/IA8D.bin");
    Image expectImg(6, 3, expect);

    EXPECT_EQ(expectImg, decoded);
}

TEST(ImageCoderTests, RGB565)
{
    Buffer encoded = IO::readFile(CT_LIB_TESTS_DATA_DIR"/Images/Coder/RGB565.bin");
    Image decoded = ImageCoder::decode(encoded, 6, 3, ImageFormat::RGB565);

    Buffer expect = IO::readFile(CT_LIB_TESTS_DATA_DIR"/Images/Coder/RGB565D.bin");
    Image expectImg(6, 3, expect);

    EXPECT_EQ(expectImg, decoded);
}

TEST(ImageCoderTests, RGB5A3)
{
    Buffer encoded = IO::readFile(CT_LIB_TESTS_DATA_DIR"/Images/Coder/RGB5A3.bin");
    Image decoded = ImageCoder::decode(encoded, 6, 3, ImageFormat::RGB5A3);

    Buffer expect = IO::readFile(CT_LIB_TESTS_DATA_DIR"/Images/Coder/RGB5A3D.bin");
    Image expectImg(6, 3, expect);

    EXPECT_EQ(expectImg, decoded);
}

TEST(ImageCoderTests, RGBA8)
{
    Buffer encoded = IO::readFile(CT_LIB_TESTS_DATA_DIR"/Images/Coder/RGBA8.bin");
    Image decoded = ImageCoder::decode(encoded, 6, 3, ImageFormat::RGBA8);

    Buffer expect = IO::readFile(CT_LIB_TESTS_DATA_DIR"/Images/Coder/RGBA8D.bin");
    Image expectImg(6, 3, expect);

    EXPECT_EQ(expectImg, decoded);
}

TEST(ImageCoderTests, SizeFor)
{
    EXPECT_EQ(672, ImageCoder::sizeFor(19, 53, ImageFormat::I4));
    EXPECT_EQ(1056, ImageCoder::sizeFor(81, 12, ImageFormat::I8));
    EXPECT_EQ(1440, ImageCoder::sizeFor(39, 34, ImageFormat::IA4));
    EXPECT_EQ(2688, ImageCoder::sizeFor(27, 45, ImageFormat::IA8));
    EXPECT_EQ(1632, ImageCoder::sizeFor(66, 10, ImageFormat::RGB565));
    EXPECT_EQ(3520, ImageCoder::sizeFor(41, 38, ImageFormat::RGB5A3));
    EXPECT_EQ(4224, ImageCoder::sizeFor(9, 87, ImageFormat::RGBA8));
    EXPECT_EQ(1120, ImageCoder::sizeFor(55, 39, ImageFormat::CMPR));
}

TEST(ImageIOTests, Read)
{
    uint8_t expectPNG[4 * 4 * 4] = {
        0xFF, 0x00, 0x00, 0xFF,  0x00, 0xFF, 0x00, 0xFF,  0x00, 0x00, 0xFF, 0xFF,  0x00, 0x00, 0x00, 0xFF,
        0xFF, 0xFF, 0x00, 0xFF,  0x00, 0xFF, 0xFF, 0xFF,  0xFF, 0x00, 0xFF, 0xFF,  0x55, 0x55, 0x55, 0xFF,
        0xFF, 0x7F, 0x00, 0xFF,  0x00, 0xFF, 0x7F, 0xFF,  0x7F, 0x00, 0xFF, 0xFF,  0xAA, 0xAA, 0xAA, 0xFF,
        0xFF, 0x00, 0x7F, 0xFF,  0x7F, 0xFF, 0x00, 0xFF,  0x00, 0x7F, 0xFF, 0xFF,  0xFF, 0xFF, 0xFF, 0xFF
    };
    Image png = ImageIO::read(CT_LIB_TESTS_DATA_DIR"/Images/4x4.png");
    EXPECT_EQ(4, png.getWidth());
    EXPECT_EQ(4, png.getHeight());
    EXPECT_TRUE(Bytes::matches(expectPNG, *png, 4 * 4 * 4));

    uint8_t expectTGA[4 * 4 * 4] = {
        0xFF, 0x00, 0x00, 0xFF,  0xAA, 0x00, 0x00, 0xFF,  0x55, 0x00, 0x00, 0xFF,  0x00, 0x00, 0x00, 0xFF,
        0x00, 0xFF, 0x00, 0xFF,  0x00, 0xAA, 0x00, 0xFF,  0x00, 0x55, 0x00, 0xFF,  0x00, 0x00, 0x00, 0xFF,
        0x00, 0x00, 0xFF, 0xFF,  0x00, 0x00, 0xAA, 0xFF,  0x00, 0x00, 0x55, 0xFF,  0x00, 0x00, 0x00, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF,  0xAA, 0xAA, 0xAA, 0xFF,  0x55, 0x55, 0x55, 0xFF,  0x00, 0x00, 0x00, 0xFF
    };
    
    Image tga = ImageIO::read(CT_LIB_TESTS_DATA_DIR"/Images/4x4.tga");
    EXPECT_EQ(4, tga.getWidth());
    EXPECT_EQ(4, tga.getHeight());
    EXPECT_TRUE(Bytes::matches(expectTGA, *tga, 4 * 4 * 4));
}
