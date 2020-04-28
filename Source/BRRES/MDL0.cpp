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
    linksSections{this},
    boneSections{this},
    verticesSections{this},
    normalsSections{this},
    colorsSections{this},
    texCoordsSections{this},
    rootBone{nullptr}
{
    
}

MDL0::~MDL0() = default;

#define CT_LIB_DEFINE_MDL0_ADD(Type, container) \
template <> \
Type* MDL0::add<Type>(const std::string& name) \
{ \
    return container.add(name); \
}

#define CT_LIB_DEFINE_MDL0_GET(Type, container) \
template <> \
Type* MDL0::get<Type>(const std::string& name) const \
{ \
    return container.get(name); \
}

#define CT_LIB_DEFINE_MDL0_HAS(Type, container) \
template <> \
bool MDL0::has<Type>(const std::string& name) const \
{ \
    return container.has(name); \
}

#define CT_LIB_DEFINE_MDL0_REMOVE(Type, container) \
template <> \
void MDL0::remove<Type>(const std::string& name) \
{ \
    container.remove(name); \
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
CT_LIB_DEFINE_MDL0_ADD(Type, container) \
CT_LIB_DEFINE_MDL0_GET(Type, container) \
CT_LIB_DEFINE_MDL0_HAS(Type, container) \
CT_LIB_DEFINE_MDL0_REMOVE(Type, container) \
CT_LIB_DEFINE_MDL0_GET_ALL(Type, container) \
CT_LIB_DEFINE_MDL0_COUNT(Type, container)

CT_LIB_DEFINE_ALL_MDL0(MDL0::Links, linksSections)
CT_LIB_DEFINE_ALL_MDL0(MDL0::Bone, boneSections)
CT_LIB_DEFINE_ALL_MDL0(MDL0::VertexArray, verticesSections)
CT_LIB_DEFINE_ALL_MDL0(MDL0::NormalArray, normalsSections)
CT_LIB_DEFINE_ALL_MDL0(MDL0::ColorArray, colorsSections)
CT_LIB_DEFINE_ALL_MDL0(MDL0::TexCoordArray, texCoordsSections)

#undef CT_LIB_DEFINE_ALL_MDL0
#undef CT_LIB_DEFINE_MDL0_ADD
#undef CT_LIB_DEFINE_MDL0_GET
#undef CT_LIB_DEFINE_MDL0_HAS
#undef CT_LIB_DEFINE_MDL0_REMOVE
#undef CT_LIB_DEFINE_MDL0_GET_ALL
#undef CT_LIB_DEFINE_MDL0_COUNT

void MDL0::sectionAdded(Section* instance)
{
    if (instance->getType() == SectionType::Bone)
    {
        if (boneSections.sections.size() == 1)
        {
            rootBone = (Bone*)instance;
            Links* nodeTree = new Links(this, Links::Type::NodeTree);
            linksSections.directAdd(nodeTree);
        }
    }
}

void MDL0::sectionRemoved(Section* instance)
{
    if (instance->getType() == SectionType::Bone)
    {
        if (boneSections.sections.size() == 0) // no bones left
        {
            linksSections.remove(Links::nameForType(Links::Type::NodeTree));
            rootBone = nullptr;
        }
        else
        {
            Bone::removePrivate(this, (Bone*)instance);
        }
    }
}

/// Section-specific methods ///////////

MDL0::Bone* MDL0::getRootBone() const
{
    return rootBone;
}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////
////   Base section classes
////

MDL0::Section::Section(MDL0* mdl0, const std::string& name) :
    mdl0{mdl0},
    name{name}
{

}

MDL0::Section::~Section() = default;

MDL0::SectionType MDL0::Section::getType() const
{
    return SectionType::NONE;
}

std::string MDL0::Section::getName() const
{
    return name;
}

MDL0::ArraySection::ArraySection(MDL0* mdl0, const std::string& name) :
    Section(mdl0, name)
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
////   Links section
////

std::string MDL0::Links::nameForType(Type type)
{
    switch (type)
    {
    case Type::NodeTree:
        return "NodeTree";
    
    default:
        return "???";
    }
}

MDL0::Links::Links(MDL0* mdl0, const std::string& name) :
    Section(mdl0, name),
    linksType{}
{
    throw BRRESError("MDL0: Manually adding a Links section is not allowed.");
}

MDL0::Links::Links(MDL0* mdl0, Type type) :
    Section(mdl0, nameForType(type)),
    linksType{type}
{

}

MDL0::Links::~Links() = default;

MDL0::SectionType MDL0::Links::getType() const
{
    return SectionType::Links;
}

MDL0::Links::Type MDL0::Links::getLinksType() const
{
    return linksType;
}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////
////   Bone section
////

MDL0::Bone::Bone(MDL0* mdl0, const std::string& name) :
    Section(mdl0, name),
    parent{nullptr},
    child{nullptr},
    next{nullptr},
    prev{nullptr},
    pos{0, 0, 0},
    rot{0, 0, 0},
    scale{1, 1, 1}
{
    if (insertPrivate(mdl0->rootBone, this) == nullptr)
    {
        mdl0->rootBone = this;
    }
}

MDL0::Bone::~Bone() = default;

MDL0::SectionType MDL0::Bone::getType() const
{
    return SectionType::Bone;
}

MDL0::Bone* MDL0::Bone::insert(const std::string& name)
{
    Bone* bone = mdl0->add<Bone>(name);

    if (removePrivate(mdl0->rootBone, bone) == nullptr)
    {
        mdl0->rootBone = mdl0->rootBone->next;
    }

    if (insertPrivate(child, bone) == nullptr)
    {
        child = bone;
    }
    bone->parent = this;

    return bone;
}

void MDL0::Bone::moveTo(Bone* bone)
{
    assertSameMDL0(bone);
    assertNotThis(bone);
    assertNotChild(bone);

    if (parent == bone)
    {
        return;
    }

    if (parent != nullptr)
    {
        if (removePrivate(parent->child, this) == nullptr)
        {
            parent->child = next;
        }
    }
    else
    {
        if (removePrivate(mdl0->rootBone, this) == nullptr)
        {
            mdl0->rootBone = next;
        }
    }

    parent = bone;

    if (bone != nullptr)
    {
        if (bone->child == nullptr || insertPrivate(bone->child, this) == nullptr)
        {
            bone->child = this;
        }
    }
    else // move at root level
    {
        if (insertPrivate(mdl0->rootBone, this) == nullptr)
        {
            mdl0->rootBone = this;
        }
    }
}

MDL0::Bone* MDL0::Bone::getParent() const
{
    return parent;
}

bool MDL0::Bone::isChild(Bone* bone) const
{
    if (isDirectChild(bone))
    {
        return true;
    }

    Bone* curr = child;
    while (curr != nullptr)
    {
        if (curr->isChild(bone))
        {
            return true;
        }
        curr = curr->next;
    }
    return false;
}

bool MDL0::Bone::isDirectChild(Bone* bone) const
{
    Bone* curr = child;
    while (curr != nullptr)
    {
        if (curr == bone)
        {
            return true;
        }
        curr = curr->next;
    }
    return false;
}

bool MDL0::Bone::hasChild(const std::string& name) const
{
    if (this->name == name)
    {
        return false;
    }

    if (mdl0->has<Bone>(name))
    {
        return isChild(mdl0->get<Bone>(name));
    }
    return false;
}

bool MDL0::Bone::hasDirectChild(const std::string& name) const
{
    if (this->name == name)
    {
        return false;
    }

    if (mdl0->has<Bone>(name))
    {
        return isDirectChild(mdl0->get<Bone>(name));
    }
    return false;
}

MDL0::Bone* MDL0::Bone::getChild(const std::string& name) const
{
    assertHasDirectChild(name);
    return mdl0->get<Bone>(name);
}

MDL0::Bone* MDL0::Bone::getFirstChild() const
{
    return child;
}

MDL0::Bone* MDL0::Bone::getNext() const
{
    return next;
}

MDL0::Bone* MDL0::Bone::getPrevious() const
{
    return prev;
}

Vector3f MDL0::Bone::getPosition() const
{
    return pos;
}

Vector3f MDL0::Bone::getRotation() const
{
    return rot;
}

Vector3f MDL0::Bone::getScale() const
{
    return scale;
}

Vector3f MDL0::Bone::getBoxMin() const
{
    return boxMin;
}

Vector3f MDL0::Bone::getBoxMax() const
{
    return boxMax;
}

MDL0::Bone* MDL0::Bone::insertPrivate(Bone* first, Bone* bone)
{
    if (first == nullptr) // 'bone' is the first one to be inserted
    {
        bone->prev = nullptr;
        bone->next = nullptr;
        return nullptr;
    }

    Bone* pos = nullptr; // the entry before 'bone'
    Bone* curr = first; // the entry after 'bone'
    while (curr != nullptr && bone->name > curr->name)
    {
        pos = curr;
        curr = curr->next;
    }

    // set previous and next for all affected bones
    bone->prev = pos;
    if (pos != nullptr)
    {
        pos->next = bone;
    }

    bone->next = curr;
    if (curr != nullptr)
    {
        curr->prev = bone;
    }

    return pos;
}

void MDL0::Bone::removePrivate(MDL0* mdl0, Bone* bone)
{
    Bone*& first = bone->parent == nullptr ? mdl0->rootBone : bone->parent->child;

    if (removePrivate(first, bone) == nullptr) // remove 'bone' from its chain
    {
        first = bone->next;
    }

    Bone* curr = bone->child;
    while (curr != nullptr) // add all childs to the old chain of 'bone'
    {
        curr->parent = bone->parent;

        Bone* oldNext = curr->next;
        if (insertPrivate(first, curr) == nullptr)
        {
            first = curr;
        }

        curr = oldNext;
    }
}

MDL0::Bone* MDL0::Bone::removePrivate(Bone* first, Bone* bone)
{
    if (first == bone) // 'bone' is first in chain, so execute simplified code
    {
        if (bone->next != nullptr)
        {
            bone->next->prev = nullptr;
        }
        return nullptr;
    }

    Bone* curr = first;
    while (curr->next != bone) // find 'bone' in chain
    {
        curr = curr->next;
    }

    // remove 'bone' from its previous and next siblings
    curr->next = bone->next;
    if (curr->next != nullptr)
    {
        curr->next->prev = curr;
    }

    return curr;
}

void MDL0::Bone::assertSameMDL0(Bone* bone) const
{
    if (mdl0 != bone->mdl0)
    {
        throw BRRESError(Strings::format(
            "MDL0: The specified bone (%s) is not owned by the same MDL0 as this (%s)! (%s != %s)",
            bone->name.c_str(), name.c_str(), bone->mdl0->getName().c_str(), mdl0->name.c_str()
        ));
    }
}

void MDL0::Bone::assertNotThis(Bone* bone) const
{
    if (bone == this)
    {
        throw BRRESError(Strings::format(
            "MDL0: "
        ));
    }
}

void MDL0::Bone::assertNotChild(Bone* bone) const
{
    if (isChild(bone))
    {
        throw BRRESError(Strings::format(
            "MDL0: The specified bone (%s) is already a child of this (%s)!",
            bone->name.c_str(), name.c_str()
        ));
    }
}

void MDL0::Bone::assertHasDirectChild(const std::string& name) const
{
    if (!hasDirectChild(name))
    {
        throw BRRESError(Strings::format(
            "MDL0: This bone (%s) has no direct child bone named: %s!",
            this->name.c_str(), name.c_str()
        ));
    }
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
    ArraySection(mdl0, name),
    comps{Components::XYZ}
{

}

MDL0::VertexArray::~VertexArray() = default;

MDL0::SectionType MDL0::VertexArray::getType() const
{
    return SectionType::VertexArray;
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
    ArraySection(mdl0, name),
    comps{Components::Normal}
{

}

MDL0::NormalArray::~NormalArray() = default;

MDL0::SectionType MDL0::NormalArray::getType() const
{
    return SectionType::NormalArray;
}

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
////  Color array section

uint8_t MDL0::ColorArray::byteCount(Format format)
{
    switch (format)
    {
    case Format::RGB565:
    case Format::RGBA4:
        return 2;

    case Format::RGB8:
    case Format::RGBA6:
        return 3;

    case Format::RGBX8:
    case Format::RGBA8:
        return 4;

    default:
        return 0;
    }
}

uint8_t MDL0::ColorArray::componentCount(Format format)
{
    switch (format)
    {
    case Format::RGB565:
    case Format::RGB8:
    case Format::RGBX8:
        return 3;

    case Format::RGBA4:
    case Format::RGBA6:
    case Format::RGBA8:
        return 4;

    default:
        return 0;
    }
}

MDL0::ColorArray::ColorArray(MDL0* mdl0, const std::string& name) :
    ArraySection(mdl0, name),
    format{Format::RGBA8}
{

}

MDL0::ColorArray::~ColorArray() = default;

MDL0::SectionType MDL0::ColorArray::getType() const
{
    return SectionType::ColorArray;
}

void MDL0::ColorArray::setData(Buffer& buffer, Format format)
{
    const uint8_t elemSize = byteCount(format);
    count = static_cast<uint32_t>(buffer.remaining() / elemSize);

    data = Buffer(count * elemSize);
    data.putArray(*buffer + buffer.position(), data.capacity());
    buffer.position(data.capacity());
    data.flip();

    this->format = format;
}

MDL0::ColorArray::Format MDL0::ColorArray::getFormat() const
{
    return format;
}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////
////   Tex coord array section
////

uint8_t MDL0::TexCoordArray::componentCount(Components comps)
{
    switch (comps)
    {
    case Components::S:
        return 1;

    case Components::ST:
        return 2;
    
    default:
        return 0;
    }
}

MDL0::TexCoordArray::TexCoordArray(MDL0* mdl0, const std::string& name) :
    ArraySection(mdl0, name),
    comps{Components::ST}
{

}

MDL0::TexCoordArray::~TexCoordArray() = default;

MDL0::SectionType MDL0::TexCoordArray::getType() const
{
    return SectionType::TexCoordArray;
}

void MDL0::TexCoordArray::setData(Buffer& buffer, Components comps)
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

    for (uint32_t i = compCount; i < boxMin.size(); ++i) // fill in T if comps is S
    {
        boxMin[i] = 0.f;
        boxMax[i] = 0.f;
    }

    this->comps = comps;
}

MDL0::TexCoordArray::Components MDL0::TexCoordArray::getComponentsType() const
{
    return comps;
}

Vector2f MDL0::TexCoordArray::getBoxMin() const
{
    return boxMin;
}

Vector2f MDL0::TexCoordArray::getBoxMax() const
{
    return boxMax;
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

    Type* instance = new Type(mdl0, name);
    directAdd(instance);
    return instance;
}

template <class Type>
Type* MDL0::SectionContainer<Type>::get(const std::string& name) const
{
    assertExists(name);

    return sections.at(nameLinks.at(name));
}

template <class Type>
bool MDL0::SectionContainer<Type>::has(const std::string& name) const
{
    return nameLinks.count(name) > 0;
}

template <class Type>
void MDL0::SectionContainer<Type>::remove(const std::string& name)
{
    assertExists(name);
    
    size_t idx = nameLinks.at(name);
    nameLinks.erase(name);

    Type* section = sections.at(idx);
    sections.erase(sections.begin() + idx);

    mdl0->sectionRemoved(section);
    delete section;

    // update name links
    for (auto& pair : nameLinks)
    {
        if (pair.second > idx)
        {
            --pair.second;
        }
    }
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
void MDL0::SectionContainer<Type>::directAdd(Type* instance)
{
    assertUniqueName(instance->name);

    nameLinks.insert(std::map<std::string, uint32_t>::value_type(
        instance->name, static_cast<uint32_t>(sections.size())
    ));
    sections.push_back(instance);
    
    mdl0->sectionAdded(instance);
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
template bool MDL0::SectionContainer<Type>::has(const std::string&) const; \
template void MDL0::SectionContainer<Type>::remove(const std::string&); \
template void MDL0::SectionContainer<Type>::deleteAll(); \
template void MDL0::SectionContainer<Type>::directAdd(Type*); \
template void MDL0::SectionContainer<Type>::assertUniqueName(const std::string&) const; \
template void MDL0::SectionContainer<Type>::assertExists(const std::string&) const;

CT_LIB_DECLARE_SECTION_CONTAINER_METHODS(MDL0::Links)
CT_LIB_DECLARE_SECTION_CONTAINER_METHODS(MDL0::Bone)
CT_LIB_DECLARE_SECTION_CONTAINER_METHODS(MDL0::VertexArray)
CT_LIB_DECLARE_SECTION_CONTAINER_METHODS(MDL0::NormalArray)
CT_LIB_DECLARE_SECTION_CONTAINER_METHODS(MDL0::ColorArray)
CT_LIB_DECLARE_SECTION_CONTAINER_METHODS(MDL0::TexCoordArray)

#undef CT_LIB_DECLARE_SECTION_CONTAINER_METHODS
}
