//////////////////////////////////////////////////
//  Copyright (c) 2020 Nara Hiero
//
// This file is licensed under GPLv3+
// Refer to the `License.txt` file included.
//////////////////////////////////////////////////

#include <CTLib/Yaz.hpp>

namespace CTLib
{

YazError::YazError(const char* msg) : std::runtime_error(msg)
{
    
}

YazError::YazError(const std::string& msg) : std::runtime_error(msg)
{
    
}
}
