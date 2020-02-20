#include <CTLib/ctlib.hpp>

// Some clever macro tricks to make a version c-style string without generating any code
#define _CTLIB_CONCAT(m, n, p) #m "." #n "." #p
#define _CTLIB_MAKE_VER(m, n, p) _CTLIB_CONCAT(m, n, p)
#define _CTLIB_VER_STR _CTLIB_MAKE_VER(CT_LIB_VERSION_MAJOR, \
                                        CT_LIB_VERSION_MINOR, \
                                        CT_LIB_VERSION_PATCH)

namespace ctlib {

void getVersion(int* major, int* minor, int* patch) {
    *major = CT_LIB_VERSION_MAJOR;
    *minor = CT_LIB_VERSION_MINOR;
    *patch = CT_LIB_VERSION_PATCH;
}

std::string getVersionString() {
    return std::string(_CTLIB_VER_STR);
}

const char* getVersionCString() {
    return _CTLIB_VER_STR;
}

bool binariesVersionMatches(int major, int minor, int patch) {
    return major == CT_LIB_VERSION_MAJOR &&
            minor == CT_LIB_VERSION_MINOR &&
            patch == CT_LIB_VERSION_PATCH;
}
}
