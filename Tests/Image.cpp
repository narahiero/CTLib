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

TEST(ImageTests, Test)
{
    uint8_t expect[4 * 4 * 4] = {
        0xFF, 0x00, 0x00, 0xFF,  0x00, 0xFF, 0x00, 0xFF,  0x00, 0x00, 0xFF, 0xFF,  0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0x00, 0xFF,  0xFF, 0xFF, 0x00, 0xFF,  0xFF, 0xFF, 0x00, 0xFF,  0xFF, 0xFF, 0x00, 0xFF,
        0xFF, 0xFF, 0x00, 0xFF,  0xFF, 0xFF, 0x00, 0xFF,  0xFF, 0xFF, 0x00, 0xFF,  0xFF, 0xFF, 0x00, 0xFF,
        0xFF, 0xFF, 0x00, 0xFF,  0xFF, 0xFF, 0x00, 0xFF,  0xFF, 0xFF, 0x00, 0xFF,  0xFF, 0xFF, 0x00, 0xFF,
    };

    CTLib::Buffer data(0x10);
    data.putInt(0xFF0000FF).putInt(0x00FF00FF).putInt(0x0000FFFF).putInt(0xFFFFFFFF).flip();
    CTLib::Image image(4, 4, data, {0xFF, 0xFF, 0x00, 0xFF});

    EXPECT_EQ(4, image.getWidth());
    EXPECT_EQ(4, image.getHeight());
    EXPECT_EQ(4 * 4 * 4, image.getData().capacity());
    EXPECT_TRUE(CTLib::Bytes::matches(expect, *image, 4 * 4 * 4));
}

TEST(ImageCoderTests, I4)
{
    CTLib::Buffer encoded = CTLib::IO::readFile(CT_LIB_TESTS_DATA_DIR"/Images/Coder/I4.bin");
    CTLib::Image decoded = CTLib::ImageCoder::decode(encoded, 6, 3, CTLib::ImageFormat::I4);

    CTLib::Buffer expect = CTLib::IO::readFile(CT_LIB_TESTS_DATA_DIR"/Images/Coder/I4D.bin");
    CTLib::Image expectImg(6, 3, expect);

    EXPECT_EQ(expectImg, decoded);
}

TEST(ImageCoderTests, I8)
{
    CTLib::Buffer encoded = CTLib::IO::readFile(CT_LIB_TESTS_DATA_DIR"/Images/Coder/I8.bin");
    CTLib::Image decoded = CTLib::ImageCoder::decode(encoded, 6, 3, CTLib::ImageFormat::I8);

    CTLib::Buffer expect = CTLib::IO::readFile(CT_LIB_TESTS_DATA_DIR"/Images/Coder/I8D.bin");
    CTLib::Image expectImg(6, 3, expect);

    EXPECT_EQ(expectImg, decoded);
}

TEST(ImageCoderTests, IA4)
{
    CTLib::Buffer encoded = CTLib::IO::readFile(CT_LIB_TESTS_DATA_DIR"/Images/Coder/IA4.bin");
    CTLib::Image decoded = CTLib::ImageCoder::decode(encoded, 6, 3, CTLib::ImageFormat::IA4);

    CTLib::Buffer expect = CTLib::IO::readFile(CT_LIB_TESTS_DATA_DIR"/Images/Coder/IA4D.bin");
    CTLib::Image expectImg(6, 3, expect);

    EXPECT_EQ(expectImg, decoded);
}

TEST(ImageCoderTests, IA8)
{
    CTLib::Buffer encoded = CTLib::IO::readFile(CT_LIB_TESTS_DATA_DIR"/Images/Coder/IA8.bin");
    CTLib::Image decoded = CTLib::ImageCoder::decode(encoded, 6, 3, CTLib::ImageFormat::IA8);

    CTLib::Buffer expect = CTLib::IO::readFile(CT_LIB_TESTS_DATA_DIR"/Images/Coder/IA8D.bin");
    CTLib::Image expectImg(6, 3, expect);

    EXPECT_EQ(expectImg, decoded);
}

TEST(ImageCoderTests, RGB565)
{
    CTLib::Buffer encoded = CTLib::IO::readFile(CT_LIB_TESTS_DATA_DIR"/Images/Coder/RGB565.bin");
    CTLib::Image decoded = CTLib::ImageCoder::decode(encoded, 6, 3, CTLib::ImageFormat::RGB565);

    CTLib::Buffer expect = CTLib::IO::readFile(CT_LIB_TESTS_DATA_DIR"/Images/Coder/RGB565D.bin");
    CTLib::Image expectImg(6, 3, expect);

    EXPECT_EQ(expectImg, decoded);
}

TEST(ImageCoderTests, RGB5A3)
{
    CTLib::Buffer encoded = CTLib::IO::readFile(CT_LIB_TESTS_DATA_DIR"/Images/Coder/RGB5A3.bin");
    CTLib::Image decoded = CTLib::ImageCoder::decode(encoded, 6, 3, CTLib::ImageFormat::RGB5A3);

    CTLib::Buffer expect = CTLib::IO::readFile(CT_LIB_TESTS_DATA_DIR"/Images/Coder/RGB5A3D.bin");
    CTLib::Image expectImg(6, 3, expect);

    EXPECT_EQ(expectImg, decoded);
}

TEST(ImageCoderTests, RGBA8)
{
    CTLib::Buffer encoded = CTLib::IO::readFile(CT_LIB_TESTS_DATA_DIR"/Images/Coder/RGBA8.bin");
    CTLib::Image decoded = CTLib::ImageCoder::decode(encoded, 6, 3, CTLib::ImageFormat::RGBA8);

    CTLib::Buffer expect = CTLib::IO::readFile(CT_LIB_TESTS_DATA_DIR"/Images/Coder/RGBA8D.bin");
    CTLib::Image expectImg(6, 3, expect);

    EXPECT_EQ(expectImg, decoded);
}

TEST(ImageIOTests, Read)
{
    uint8_t expectPNG[4 * 4 * 4] = {
        0xFF, 0x00, 0x00, 0xFF,  0x00, 0xFF, 0x00, 0xFF,  0x00, 0x00, 0xFF, 0xFF,  0x00, 0x00, 0x00, 0xFF,
        0xFF, 0xFF, 0x00, 0xFF,  0x00, 0xFF, 0xFF, 0xFF,  0xFF, 0x00, 0xFF, 0xFF,  0x55, 0x55, 0x55, 0xFF,
        0xFF, 0x7F, 0x00, 0xFF,  0x00, 0xFF, 0x7F, 0xFF,  0x7F, 0x00, 0xFF, 0xFF,  0xAA, 0xAA, 0xAA, 0xFF,
        0xFF, 0x00, 0x7F, 0xFF,  0x7F, 0xFF, 0x00, 0xFF,  0x00, 0x7F, 0xFF, 0xFF,  0xFF, 0xFF, 0xFF, 0xFF
    };
    CTLib::Image png = CTLib::ImageIO::read(CT_LIB_TESTS_DATA_DIR"/Images/4x4.png");
    EXPECT_EQ(4, png.getWidth());
    EXPECT_EQ(4, png.getHeight());
    EXPECT_TRUE(CTLib::Bytes::matches(expectPNG, *png, 4 * 4 * 4));

    uint8_t expectGIF[4 * 4 * 4] = {
        0xFF, 0x00, 0x00, 0xFF,  0xAA, 0x00, 0x00, 0xFF,  0x55, 0x00, 0x00, 0xFF,  0x00, 0x00, 0x00, 0xFF,
        0x00, 0xFF, 0x00, 0xFF,  0x00, 0xAA, 0x00, 0xFF,  0x00, 0x55, 0x00, 0xFF,  0x00, 0x00, 0x00, 0xFF,
        0x00, 0x00, 0xFF, 0xFF,  0x00, 0x00, 0xAA, 0xFF,  0x00, 0x00, 0x55, 0xFF,  0x00, 0x00, 0x00, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF,  0xAA, 0xAA, 0xAA, 0xFF,  0x55, 0x55, 0x55, 0xFF,  0x00, 0x00, 0x00, 0xFF
    };
    
    CTLib::Image tga = CTLib::ImageIO::read(CT_LIB_TESTS_DATA_DIR"/Images/4x4.tga");
    EXPECT_EQ(4, tga.getWidth());
    EXPECT_EQ(4, tga.getHeight());
    EXPECT_TRUE(CTLib::Bytes::matches(expectGIF, *tga, 4 * 4 * 4));
}
