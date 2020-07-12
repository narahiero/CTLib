//////////////////////////////////////////////////
//  Copyright (c) 2020 Nara Hiero
//
// This file is licensed under GPLv3+
// Refer to the `License.txt` file included.
//////////////////////////////////////////////////

#include <gtest/gtest.h>

#include <CTLib/Utilities.hpp>

using namespace CTLib;

TEST(StringsTests, Stringify)
{
    uint8_t bytes[] = {0x61, 0x83, 0x3D, 0x20, 0x7E, 0x7F, 0x64, 0x19, 0x00, 0x6C};
    EXPECT_EQ("a\\x83= ~\\x7Fd\\x19\\x00l", Strings::stringify(bytes, 10));

    uint8_t bytes1[] = {0x6E, 0x75, 0x6C, 0x6C, 0x20, 0x00};
    EXPECT_EQ("null \\x00", Strings::stringify(bytes1, 6));
}

TEST(StringsTests, Count)
{
    std::string str = "this is some string";
    EXPECT_EQ(4, Strings::count(str, 's'));
    EXPECT_EQ(3, Strings::count(str, 'i'));
    EXPECT_EQ(1, Strings::count(str, 'r'));
    EXPECT_EQ(0, Strings::count(str, 'z'));

    str = "some/path/to/some/file";
    EXPECT_EQ(4, Strings::count(str, '/'));
}

TEST(StringsTests, Split)
{
    std::string str = "this is some string";
    auto vec = Strings::split(str, ' ');
    EXPECT_EQ(4, vec.size());
    EXPECT_EQ((std::vector<std::string>{"this", "is", "some", "string"}), vec);

    str = "some/path/to/some/file";
    vec = Strings::split(str, '/');
    EXPECT_EQ(5, vec.size());
    EXPECT_EQ((std::vector<std::string>{"some", "path", "to", "some", "file"}), vec);

    str = "/some/funky//string/";
    vec = Strings::split(str, '/');
    EXPECT_EQ(6, vec.size());
    EXPECT_EQ((std::vector<std::string>{"", "some", "funky", "", "string", ""}), vec);
}

TEST(StringsTests, Format)
{
    const char* str = "string";
    EXPECT_EQ(
        "A formatted string using Strings::format()!",
        Strings::format("A formatted %s using Strings::format()!", str)
    );

    EXPECT_EQ(
        "name: Test, age: 32, x: 7.5, y: 1.25",
        Strings::format(
            "name: %s, age: %d, x: %.1f, y: %.2f",
            "Test", 32, 7.5f, 1.25f
        )
    );
}

TEST(CollectionsTests, Remove)
{
    std::vector<int> vec{0, 32, 54, 0, 32, 0, 3, 2, 9, 32, 0, 0};
    EXPECT_EQ(1, Collections::removeFirst(vec, 32));
    EXPECT_EQ(8, Collections::removeLast(vec, 32));
    EXPECT_EQ(3, Collections::removeFirst(vec, 32));
    EXPECT_EQ(Collections::NPOS, Collections::removeFirst(vec, 32));
    EXPECT_EQ(Collections::NPOS, Collections::removeLast(vec, 32));

    EXPECT_EQ(5, Collections::removeAll(vec, 0));
    EXPECT_EQ(0, Collections::removeAll(vec, 0));
}

TEST(BytesTests, Matches)
{
    uint8_t bytes[] = {
        0xFF, 0xAA, 0x00, 0x45, 0x64, 0x73, 0x20, 0x50
    };

    uint8_t match[] = {
        0x43, 0x12, 0xFF, 0xAA, 0x00, 0x45, 0x64, 0x73, 0x20, 0x50, 0x90
    };

    EXPECT_TRUE(Bytes::matches(match + 2, bytes, 8));
    EXPECT_FALSE(Bytes::matches(match, bytes, 8));
}

TEST(BytesTests, MatchesString)
{
    uint8_t* bytes = (uint8_t*)"apples";
    EXPECT_TRUE(Bytes::matchesString("apples", bytes, 6));
    EXPECT_FALSE(Bytes::matchesString("grapes", bytes, 6));
    EXPECT_TRUE(Bytes::matchesString("apple", bytes, 5));

    bytes = (uint8_t*)"blueberries";
    EXPECT_TRUE(Bytes::matchesString("blueberries", bytes, 10));
    EXPECT_TRUE(Bytes::matchesString("berries", bytes + 4, 6));
}
