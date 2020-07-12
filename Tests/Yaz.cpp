//////////////////////////////////////////////////
//  Copyright (c) 2020 Nara Hiero
//
// This file is licensed under GPLv3+
// Refer to the `License.txt` file included.
//////////////////////////////////////////////////

#include <gtest/gtest.h>

#include <CTLib/Utilities.hpp>
#include <CTLib/Yaz.hpp>

using namespace CTLib;

TEST(DecompressTests, Simple)
{
    {
        uint8_t* data = (uint8_t*)"Yaz0\0\0\0\x22" "\0\0\0\0\0\0\0\0"
            "\xFBThis \x10\x02so\xFFme text,\xF7 for\x60\x0Est!\x80\0\0";
        
        Buffer buffer(48);
        buffer.putArray(data, 48).flip();

        Buffer decompressed = Yaz::decompress(buffer);
        EXPECT_TRUE(Bytes::matchesString(
            "This is some text, for some test!", *decompressed, 34
        ));
    }

    {
        uint8_t* data = (uint8_t*)"Yaz1\0\0\0\x3A" "\0\0\0\0\0\0\0\0"
            "\xFF""Data com\xFFpressed \xFFusing Ya\xFFz1 and d\xBF""e\xF0\x1B"
            "CT Lib\xC0.\0\0\0\0";

        Buffer buffer(68);
        buffer.putArray(data, 68).flip();

        Buffer decompressed = Yaz::decompress(buffer);
        EXPECT_TRUE(Bytes::matchesString(
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

        Buffer buffer(32);
        buffer.putArray(data, 32).flip();

        Buffer decompressed = Yaz::decompress(buffer);
        EXPECT_TRUE(Bytes::matches(expect, *decompressed, 30));
    }
}
