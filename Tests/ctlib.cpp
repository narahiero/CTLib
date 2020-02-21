#include <gtest/gtest.h>

#include <CTLib/ctlib.hpp>

#include <stdio.h>

/* Test the following functions:
 *      getVersion(int*, int*, int*)
 *      binariesVersionMatches(int, int, int)
 */
TEST(VersionTests, HeaderMatchesBinaries) {
    int major = 0, minor = 0, patch = 0;
    ctlib::getVersion(&major, &minor, &patch);

    EXPECT_EQ(CT_LIB_VERSION_MAJOR, major);
    EXPECT_EQ(CT_LIB_VERSION_MINOR, minor);
    EXPECT_EQ(CT_LIB_VERSION_PATCH, patch);

    EXPECT_TRUE(ctlib::binariesVersionMatches(CT_LIB_VERSION_MAJOR,
                                            CT_LIB_VERSION_MINOR,
                                            CT_LIB_VERSION_PATCH));
}

/* Test the following functions:
 *      getVersionString()
 *      getVersionCString()
 */
TEST(VersionTests, VersionStrings) {
    char expect_ver[6];
    snprintf(expect_ver, 6, "%d.%d.%d", CT_LIB_VERSION_MAJOR,
                                    CT_LIB_VERSION_MINOR,
                                    CT_LIB_VERSION_PATCH);

    std::string std_string_ver = ctlib::getVersionString();
    EXPECT_EQ(expect_ver, std_string_ver);

    const char* c_string_ver = ctlib::getVersionCString();
    EXPECT_STREQ(expect_ver, c_string_ver);
}
