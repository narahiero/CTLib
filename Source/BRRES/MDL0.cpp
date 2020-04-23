//////////////////////////////////////////////////
//  Copyright (c) 2020 Nara Hiero
//
// This file is licensed under GPLv3+
// Refer to the `License.txt` file included.
//////////////////////////////////////////////////

#include <CTLib/BRRES.hpp>

namespace CTLib
{

MDL0::MDL0(BRRES* brres, const std::string& name) :
    BRRESSubFile(brres, name)
{

}

MDL0::~MDL0() = default;
}
