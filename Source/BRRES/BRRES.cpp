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

uint16_t BRRES::getSubfileCount() const
{
    return static_cast<uint16_t>(
        mdl0s.size()
        + tex0s.size()
    );
}

#define CT_LIB_DEFINE_BRRES_ADD(Type, container) \
template <> \
Type* BRRES::add<Type>(const std::string& name) \
{ \
    if (container.count(name) > 0) \
    { \
        throw BRRESError(Strings::format( \
            "BRRES: There is already another " #Type " with the name: %s!", name.c_str() \
        )); \
    } \
    Type* instance = new Type(this, name); \
    container.insert(std::map<std::string, Type*>::value_type(name, instance)); \
    return instance; \
}

#define CT_LIB_DEFINE_BRRES_GET(Type, container) \
template <> \
Type* BRRES::get<Type>(const std::string& name) const \
{ \
    if (container.count(name) == 0) \
    { \
        throw BRRESError(Strings::format( \
            "BRRES: There is no " #Type " with the name: %s!", name.c_str() \
        )); \
    } \
    return container.at(name); \
}

#define CT_LIB_DEFINE_BRRES_HAS(Type, container) \
template <> \
bool BRRES::has<Type>(const std::string& name) const \
{ \
    return container.count(name) > 0; \
}

#define CT_LIB_DEFINE_BRRES_REMOVE(Type, container) \
template <> \
void BRRES::remove<Type>(const std::string& name) \
{ \
    if (container.count(name) == 0) \
    { \
        throw BRRESError(Strings::format( \
            "BRRES: There is no " #Type " with the name: %s!", name.c_str() \
        )); \
    } \
    delete container.at(name); \
    container.erase(name); \
}

#define CT_LIB_DEFINE_BRRES_GET_ALL(Type, container) \
template <> \
std::vector<Type*> BRRES::getAll<Type>() const \
{ \
    std::vector<Type*> vec; \
    vec.reserve(container.size()); \
    for (const auto& pair : container) \
    { \
        vec.push_back(pair.second); \
    } \
    return vec; \
}

#define CT_LIB_DEFINE_BRRES_COUNT(Type, container) \
template <> \
uint16_t BRRES::count<Type>() const \
{ \
    return static_cast<uint16_t>(container.size()); \
}

#define CT_LIB_DEFINE_ALL_BRRES(Type, container) \
CT_LIB_DEFINE_BRRES_ADD(Type, container) \
CT_LIB_DEFINE_BRRES_GET(Type, container) \
CT_LIB_DEFINE_BRRES_HAS(Type, container) \
CT_LIB_DEFINE_BRRES_REMOVE(Type, container) \
CT_LIB_DEFINE_BRRES_GET_ALL(Type, container) \
CT_LIB_DEFINE_BRRES_COUNT(Type, container) \

CT_LIB_DEFINE_ALL_BRRES(MDL0, mdl0s)
CT_LIB_DEFINE_ALL_BRRES(TEX0, tex0s)

#undef CT_LIB_DEFINE_ALL_BRRES
#undef CT_LIB_DEFINE_BRRES_ADD
#undef CT_LIB_DEFINE_BRRES_GET
#undef CT_LIB_DEFINE_BRRES_HAS
#undef CT_LIB_DEFINE_BRRES_REMOVE
#undef CT_LIB_DEFINE_BRRES_GET_ALL
#undef CT_LIB_DEFINE_BRRES_COUNT


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
