//////////////////////////////////////////////////
//  Copyright (c) 2020 Nara Hiero
//
// This file is licensed under GPLv3+
// Refer to the `License.txt` file included.
//////////////////////////////////////////////////

#include <CTLib/BRRES.hpp>

#include <CTLib/Utilities.hpp>

namespace CTLib
{

//////////////////////////////
///  class BRRES

BRRES::BRRES() :
    tex0s{}
{

}

BRRES::BRRES(BRRES&& src) :
    tex0s{std::move(src.tex0s)}
{
    for (auto& tex0 : tex0s)
    {
        tex0.second->brres = this;
    }
}

BRRES::~BRRES()
{
    for (auto& tex0 : tex0s)
    {
        delete tex0.second;
    }
}

TEX0* BRRES::newTEX0(const std::string& name)
{
    if (tex0s.count(name) > 0)
    {
        throw BRRESError(Strings::format(
            "BRRES: There is already another TEX0 with the name: %s!", name.c_str()
        ));
    }
    TEX0* tex0 = new TEX0(this, name);
    tex0s.insert(std::map<std::string, TEX0*>::value_type(name, tex0));
    return tex0;
}

TEX0* BRRES::getTEX0(const std::string& name) const
{
    if (tex0s.count(name) == 0)
    {
        throw BRRESError(Strings::format(
            "BRRES: There is no TEX0 with the name: %s!", name.c_str()
        ));
    }
    return tex0s.at(name);
}

void BRRES::removeTEX0(const std::string& name)
{
    if (tex0s.count(name) == 0)
    {
        throw BRRESError(Strings::format(
            "BRRES: There is no TEX0 with the name: %s!", name.c_str()
        ));
    }
    delete tex0s.at(name);
    tex0s.erase(name);
}


//////////////////////////////
///  class BRRESSubFile

BRRESSubFile::BRRESSubFile(BRRES* brres, const std::string& name) :
    brres{brres},
    name{name}
{

}

BRRESSubFile::~BRRESSubFile() = default;

std::string BRRESSubFile::getName() const
{
    return name;
}


//////////////////////////////
///  class BRRESError

BRRESError::BRRESError(const char* msg) : std::runtime_error(msg)
{

}

BRRESError::BRRESError(const std::string& msg) : std::runtime_error(msg)
{

}
}
