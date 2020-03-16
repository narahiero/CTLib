//////////////////////////////////////////////////
//  Copyright (c) 2020 Nara Hiero
//
// This file is licensed under GPLv3+
// Refer to the `License.txt` file included.
//////////////////////////////////////////////////

#include <CTLib/CTLib.hpp>

// Some clever macro tricks to make a version c-style string without generating any code
#define CT_LIB_VERSION_CONCAT(m, n, p) #m "." #n "." #p
#define CT_LIB_MAKE_VERSION(m, n, p) CT_LIB_VERSION_CONCAT(m, n, p)
#define CT_LIB_VERSION_STRING CT_LIB_MAKE_VERSION( \
    CT_LIB_VERSION_MAJOR, CT_LIB_VERSION_MINOR, CT_LIB_VERSION_PATCH \
)

namespace CTLib
{

void getVersion(int* major, int* minor, int* patch)
{
    if (major)
    {
        *major = CT_LIB_VERSION_MAJOR;
    }
    if (minor)
    {
        *minor = CT_LIB_VERSION_MINOR;
    }
    if (patch)
    {
        *patch = CT_LIB_VERSION_PATCH;
    }
}

std::string getVersionString()
{
    return std::string(CT_LIB_VERSION_STRING);
}

const char* getVersionCString()
{
    return CT_LIB_VERSION_STRING;
}

bool binariesVersionMatches(int major, int minor, int patch)
{
    return major == CT_LIB_VERSION_MAJOR
        && minor == CT_LIB_VERSION_MINOR
        && patch == CT_LIB_VERSION_PATCH;
}
}
