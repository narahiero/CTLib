//////////////////////////////////////////////////
//  Copyright (c) 2020 Nara Hiero
//
// This file is licensed under GPLv3+
// Refer to the `License.txt` file included.
//////////////////////////////////////////////////

#include <stdio.h>

#include <gtest/gtest.h>

#include <CTLib/CTLib.hpp>

/* Test the following functions:
 *      getVersion(int*, int*, int*)
 *      binariesVersionMatches(int, int, int)
 */
TEST(VersionTests, HeaderMatchesBinaries)
{
    int major = 0, minor = 0, patch = 0;
    CTLib::getVersion(&major, &minor, &patch);

    EXPECT_EQ(CT_LIB_VERSION_MAJOR, major);
    EXPECT_EQ(CT_LIB_VERSION_MINOR, minor);
    EXPECT_EQ(CT_LIB_VERSION_PATCH, patch);

    EXPECT_TRUE(
        CTLib::binariesVersionMatches(
            CT_LIB_VERSION_MAJOR, CT_LIB_VERSION_MINOR, CT_LIB_VERSION_PATCH
        )
    );
}

/* Test the following functions:
 *      getVersionString()
 *      getVersionCString()
 */
TEST(VersionTests, VersionStrings)
{
    char expectedVersion[6];
    snprintf(
        expectedVersion, 6,
        "%d.%d.%d",
        CT_LIB_VERSION_MAJOR, CT_LIB_VERSION_MINOR, CT_LIB_VERSION_PATCH
    );

    std::string stdStringVersion = CTLib::getVersionString();
    EXPECT_EQ(expectedVersion, stdStringVersion);

    const char* cStringVersion = CTLib::getVersionCString();
    EXPECT_STREQ(expectedVersion, cStringVersion);
}
