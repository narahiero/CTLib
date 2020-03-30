//////////////////////////////////////////////////
//  Copyright (c) 2020 Nara Hiero
//
// This file is licensed under GPLv3+
// Refer to the `License.txt` file included.
//////////////////////////////////////////////////

#include <CTLib/U8.hpp>

namespace CTLib
{

U8Error::U8Error(const char* msg) : std::runtime_error{msg}
{

}

U8Error::U8Error(const std::string& msg) : std::runtime_error{msg}
{

}
}
