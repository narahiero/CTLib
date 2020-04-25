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

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////
////   MDL0 class
////

MDL0::MDL0(BRRES* brres, const std::string& name) :
    BRRESSubFile(brres, name),
    verticesSections{this},
    normalsSections{this}
{

}

MDL0::~MDL0() = default;

#define CT_LIB_DEFINE_MDL0_ADD_NAMED(Type, container) \
template <> \
Type* MDL0::add<Type>(const std::string& name) \
{ \
    return container.add(name); \
}

#define CT_LIB_DEFINE_MDL0_GET_NAMED(Type, container) \
template <> \
Type* MDL0::get<Type>(const std::string& name) const \
{ \
    return container.get(name); \
}

#define CT_LIB_DEFINE_MDL0_GET_ALL(Type, container) \
template <> \
std::vector<Type*> MDL0::getAll<Type>() const \
{ \
    return container.sections; \
}

#define CT_LIB_DEFINE_MDL0_COUNT(Type, container) \
template <> \
uint16_t MDL0::count<Type>() const \
{ \
    return static_cast<uint16_t>(container.sections.size()); \
}

#define CT_LIB_DEFINE_ALL_MDL0(Type, container) \
CT_LIB_DEFINE_MDL0_ADD_NAMED(Type, container) \
CT_LIB_DEFINE_MDL0_GET_NAMED(Type, container) \
CT_LIB_DEFINE_MDL0_GET_ALL(Type, container) \
CT_LIB_DEFINE_MDL0_COUNT(Type, container)

CT_LIB_DEFINE_ALL_MDL0(MDL0::VertexArray, verticesSections)
CT_LIB_DEFINE_ALL_MDL0(MDL0::NormalArray, normalsSections)

#undef CT_LIB_DEFINE_ALL_MDL0
#undef CT_LIB_DEFINE_MDL0_ADD_NAMED
#undef CT_LIB_DEFINE_MDL0_GET_NAMED
#undef CT_LIB_DEFINE_MDL0_GET_ALL
#undef CT_LIB_DEFINE_MDL0_COUNT


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////
////   Base section classes
////

MDL0::Section::Section(MDL0* mdl0) :
    mdl0{mdl0}
{

}

MDL0::Section::~Section() = default;

MDL0::NamedSection::NamedSection(MDL0* mdl0, const std::string& name) :
    Section(mdl0),
    name{name}
{

}

MDL0::NamedSection::~NamedSection() = default;

std::string MDL0::NamedSection::getName() const
{
    return name;
}

MDL0::ArraySection::ArraySection(MDL0* mdl0, const std::string& name) :
    NamedSection(mdl0, name)
{

}

MDL0::ArraySection::~ArraySection() = default;

uint16_t MDL0::ArraySection::getCount() const
{
    return count;
}

Buffer MDL0::ArraySection::getData() const
{
    return data.duplicate();
}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////
////   Vertex array section
////

uint8_t MDL0::VertexArray::componentCount(Components comps)
{
    switch (comps)
    {
    case Components::XY:
        return 2;
    
    case Components::XYZ:
        return 3;
    
    default:
        return 0;
    }
}

MDL0::VertexArray::VertexArray(MDL0* mdl0, const std::string& name) :
    ArraySection(mdl0, name)
{

}

void MDL0::VertexArray::setData(Buffer& buffer, Components comps)
{
    const uint8_t compCount = componentCount(comps);
    const uint8_t elemSize = compCount * 4;
    count = static_cast<uint16_t>(buffer.remaining() / elemSize);

    data = Buffer(count * elemSize);
    while (buffer.remaining() >= elemSize)
    {
        for (uint32_t i = 0; i < compCount; ++i)
        {
            float val = buffer.getFloat();
            data.putFloat(val);

            boxMin[i] = boxMin[i] > val ? val : boxMin[i];
            boxMax[i] = boxMax[i] < val ? val : boxMax[i];
        }
    }
    data.flip();

    for (uint32_t i = compCount; i < boxMin.size(); ++i) // fill in Z if comps is XY
    {
        boxMin[i] = 0.f;
        boxMax[i] = 0.f;
    }

    this->comps = comps;
}

MDL0::VertexArray::Components MDL0::VertexArray::getComponentsType() const
{
    return comps;
}

Vector3f MDL0::VertexArray::getBoxMin() const
{
    return boxMin;
}

Vector3f MDL0::VertexArray::getBoxMax() const
{
    return boxMax;
}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////
////   Normal array section
////

uint8_t MDL0::NormalArray::componentCount(Components comps)
{
    switch (comps)
    {
    case Components::Normal:
        return 3;

    case Components::Normal_BiNormal_Tangent:
        return 9;

    case Components::Normal_OR_BiNormal_OR_Tangent:
        return 3;
    
    default:
        return 0;
    }
}

MDL0::NormalArray::NormalArray(MDL0* mdl0, const std::string& name) :
    ArraySection(mdl0, name)
{

}

MDL0::NormalArray::~NormalArray() = default;

void MDL0::NormalArray::setData(Buffer& buffer, Components comps)
{
    const uint8_t elemSize = componentCount(comps) * 4;
    count = static_cast<uint16_t>(buffer.remaining() / elemSize);

    data = Buffer(count * elemSize);
    data.putArray(*buffer + buffer.position(), data.capacity());
    buffer.position(data.capacity());
    data.flip();

    this->comps = comps;
}

MDL0::NormalArray::Components MDL0::NormalArray::getComponentsType() const
{
    return comps;
}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////
////   Section container
////

template <class Type>
MDL0::SectionContainer<Type>::SectionContainer(MDL0* mdl0) :
    sections{},
    nameLinks{},
    mdl0{mdl0}
{

}

template <class Type>
MDL0::SectionContainer<Type>::~SectionContainer()
{
    deleteAll();
}

template <class Type>
Type* MDL0::SectionContainer<Type>::add(const std::string& name)
{
    assertUniqueName(name);

    nameLinks.insert(std::map<std::string, uint32_t>::value_type(
        name, static_cast<uint32_t>(sections.size())
    ));

    Type* instance = new Type(mdl0, name);
    sections.push_back(instance);
    return instance;
}

template <class Type>
Type* MDL0::SectionContainer<Type>::get(const std::string& name) const
{
    assertExists(name);

    return sections.at(nameLinks.at(name));
}

template <class Type>
void MDL0::SectionContainer<Type>::deleteAll()
{
    for (Type* instance : sections)
    {
        delete instance;
    }
    sections.clear();
    nameLinks.clear();
}

template <class Type>
void MDL0::SectionContainer<Type>::assertUniqueName(const std::string& name) const
{
    if (nameLinks.count(name) > 0)
    {
        throw BRRESError(Strings::format(
            "MDL0: Another section entry of the same type has the same name! (%s)",
            name.c_str()
        ));
    }
}

template <class Type>
void MDL0::SectionContainer<Type>::assertExists(const std::string& name) const
{
    if (nameLinks.count(name) == 0)
    {
        throw BRRESError(Strings::format(
            "MDL0: No section entry has the specified name! (%s)",
            name.c_str()
        ));
    }
}

#define CT_LIB_DECLARE_SECTION_CONTAINER_METHODS(Type) \
template MDL0::SectionContainer<Type>::SectionContainer(MDL0*); \
template MDL0::SectionContainer<Type>::~SectionContainer(); \
template Type* MDL0::SectionContainer<Type>::add(const std::string&); \
template Type* MDL0::SectionContainer<Type>::get(const std::string&) const; \
template void MDL0::SectionContainer<Type>::deleteAll(); \
template void MDL0::SectionContainer<Type>::assertUniqueName(const std::string&) const; \
template void MDL0::SectionContainer<Type>::assertExists(const std::string&) const;

CT_LIB_DECLARE_SECTION_CONTAINER_METHODS(MDL0::VertexArray)
CT_LIB_DECLARE_SECTION_CONTAINER_METHODS(MDL0::NormalArray)

#undef CT_LIB_DECLARE_SECTION_CONTAINER_METHODS
}
