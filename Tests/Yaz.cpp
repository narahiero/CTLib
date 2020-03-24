//////////////////////////////////////////////////
//  Copyright (c) 2020 Nara Hiero
//
// This file is licensed under GPLv3+
// Refer to the `License.txt` file included.
//////////////////////////////////////////////////

#include <gtest/gtest.h>

#include <CTLib/Utilities.hpp>
#include <CTLib/Yaz.hpp>

#include <fstream>

TEST(DecompressTests, Simple)
{
    {
        uint8_t* data = (uint8_t*)"Yaz0\0\0\0\x22" "\0\0\0\0\0\0\0\0"
            "\xFBThis \x10\x02so\xFFme text,\xF7 for\x60\x0Est!\x80\0\0";
        
        CTLib::Buffer buffer(48);
        buffer.putArray(data, 48).flip();

        CTLib::Buffer decompressed = CTLib::Yaz::decompress(buffer);
        EXPECT_TRUE(CTLib::Bytes::matchesString(
            "This is some text, for some test!", *decompressed, 34
        ));
    }

    {
        uint8_t* data = (uint8_t*)"Yaz1\0\0\0\x3A" "\0\0\0\0\0\0\0\0"
            "\xFF""Data com\xFFpressed \xFFusing Ya\xFFz1 and d\xBF""e\xF0\x1B"
            "CT Lib\xC0.\0\0\0\0";

        CTLib::Buffer buffer(68);
        buffer.putArray(data, 68).flip();

        CTLib::Buffer decompressed = CTLib::Yaz::decompress(buffer);
        EXPECT_TRUE(CTLib::Bytes::matchesString(
            "Data compressed using Yaz1 and decompressed using CT Lib.",
            *decompressed,
            58
        ));
    }

    {
        uint8_t data[] = {
            0x59, 0x61, 0x7A, 0x30, 0x00, 0x00, 0x00, 0x1E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0xF9, 0x00, 0x01, 0x02, 0x03, 0x04, 0x10, 0x03, 0x70, 0x02, 0x00, 0x00, 
            0x60, 0x00, 0x20, 0x18
        };

        uint8_t expect[] = {
            0x00, 0x01, 0x02, 0x03, 0x04, 0x01, 0x02, 0x03, 0x01, 0x02, 0x03, 0x01, 0x02, 0x03,
            0x01, 0x02, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x02,
            0x03, 0x04
        };

        CTLib::Buffer buffer(32);
        buffer.putArray(data, 32).flip();

        CTLib::Buffer decompressed = CTLib::Yaz::decompress(buffer);
        EXPECT_TRUE(CTLib::Bytes::matches(expect, *decompressed, 30));
    }
}

TEST(CompressTests, None)
{
    {
        uint8_t data[] = {
            0x10, 0x30, 0x20, 0x60, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA
        };

        uint8_t expect[] = {
            0x59, 0x61, 0x7A, 0x30, 0x00, 0x00, 0x00, 0x0B, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0xFF, 0x10, 0x30, 0x20, 0x60, 0xAA, 0xAA, 0xAA, 0xAA, 0xFF, 0xAA, 0xAA,
            0xAA, 0x00, 0x00, 0x00
        };

        CTLib::Buffer buffer(0x0B);
        buffer.putArray(data, 0x0B).flip();

        CTLib::Buffer compressed = CTLib::Yaz::compress(
            buffer, CTLib::YazFormat::Yaz0, CTLib::YazLevel::NONE
        );
        EXPECT_TRUE(CTLib::Bytes::matches(expect, *compressed, 0x20));
        EXPECT_EQ(compressed.capacity(), compressed.limit());
    }

    {
        uint8_t data[] = {
            0x01, 0x02, 0x03, 0x04, 0x05, 0xAA, 0xAA
        };

        uint8_t expect[] = {
            0x59, 0x61, 0x7A, 0x31, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0xFF, 0x01, 0x02, 0x03, 0x04, 0x05, 0xAA, 0xAA
        };

        CTLib::Buffer buffer(0x07);
        buffer.putArray(data, 0x07).flip();

        CTLib::Buffer compressed = CTLib::Yaz::compress(
            buffer, CTLib::YazFormat::Yaz1, CTLib::YazLevel::NONE
        );
        EXPECT_TRUE(CTLib::Bytes::matches(expect, *compressed, 0x18));
        EXPECT_EQ(compressed.capacity(), compressed.limit());
    }
}

TEST(CompressTests, Best)
{
    
}
