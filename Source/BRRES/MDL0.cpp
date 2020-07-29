//////////////////////////////////////////////////
//  Copyright (c) 2020 Nara Hiero
//
// This file is licensed under GPLv3+
// Refer to the `License.txt` file included.
//////////////////////////////////////////////////

#include <CTLib/BRRES.hpp>

#include <limits>

#include <CTLib/Utilities.hpp>

namespace CTLib
{

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////
////   MDL0 class
////

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
CT_LIB_DEFINE_ALL_MDL0(MDL0::ColourArray, coloursSections)
CT_LIB_DEFINE_ALL_MDL0(MDL0::TexCoordArray, texCoordsSections)
CT_LIB_DEFINE_ALL_MDL0(MDL0::Material, materialSections)
CT_LIB_DEFINE_ALL_MDL0(MDL0::Object, objectSections)
CT_LIB_DEFINE_ALL_MDL0(MDL0::Shader, shaderSections)
CT_LIB_DEFINE_ALL_MDL0(MDL0::TextureLink, textureLinkSections)

#undef CT_LIB_DEFINE_ALL_MDL0
#undef CT_LIB_DEFINE_MDL0_ADD
#undef CT_LIB_DEFINE_MDL0_GET
#undef CT_LIB_DEFINE_MDL0_HAS
#undef CT_LIB_DEFINE_MDL0_REMOVE
#undef CT_LIB_DEFINE_MDL0_GET_ALL
#undef CT_LIB_DEFINE_MDL0_COUNT

MDL0::MDL0(BRRES* brres, const std::string& name) :
    BRRESSubFile(brres, name),
    linksSections{this},
    boneSections{this},
    verticesSections{this},
    normalsSections{this},
    coloursSections{this},
    texCoordsSections{this},
    materialSections{this},
    objectSections{this},
    shaderSections{this},
    textureLinkSections{this},
    entryCallbacks{},
    rootBone{nullptr}
{
    linksSections.directAdd(new Links(this, Links::Type::DrawOpa));
}

MDL0::~MDL0() = default;

void MDL0::addCallback(SectionCallback* cb)
{
    entryCallbacks.push_back(cb);
}

void MDL0::removeCallback(SectionCallback* cb)
{
    for (size_t i = 0; i < entryCallbacks.size(); ++i)
    {
        if (entryCallbacks.at(i) == cb)
        {
            entryCallbacks.erase(entryCallbacks.begin() + i);
            --i;
        }
    }
}

void MDL0::sectionAdded(Section* instance)
{
    if (instance->getType() == SectionType::Bone)
    {
        if (boneSections.sections.size() == 1)
        {
            rootBone = (Bone*)instance;
            linksSections.directAdd(new Links(this, Links::Type::NodeTree));
        }
    }

    for (SectionCallback* cb : entryCallbacks)
    {
        cb->entryCallback(instance, true);
    }
}

void MDL0::sectionRemoved(Section* instance)
{
    for (SectionCallback* cb : entryCallbacks)
    {
        cb->entryCallback(instance, false);
    }

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

/// Model info methods /////////////////

void MDL0::setBoxMin(const Vector3f& boxMin)
{
    this->boxMin = boxMin;
}

void MDL0::setBoxMax(const Vector3f& boxMax)
{
    this->boxMax = boxMax;
}

Vector3f MDL0::getBoxMin() const
{
    return boxMin;
}

Vector3f MDL0::getBoxMax() const
{
    return boxMax;
}

/// Section-specific methods ///////////

MDL0::Links* MDL0::getDrawOpaSection() const
{
    return linksSections.get(Links::nameForType(Links::Type::DrawOpa));
}

MDL0::Bone* MDL0::getRootBone() const
{
    return rootBone;
}

MDL0::TextureLink* MDL0::linkTEX0(TEX0* tex0)
{
    assertSameBRRES(tex0);

    TextureLink* link = new TextureLink(this, tex0);
    textureLinkSections.directAdd(link);
    return link;
}

void MDL0::assertSameBRRES(BRRESSubFile* subfile) const
{
    if (subfile == nullptr)
    {
        throw BRRESError("MDL0: The specified subfile is `nullptr`!");
    }

    if (subfile->getBRRES() != brres)
    {
        throw BRRESError(
            "MDL0: The specified subfile is not owned by the same BRRES as this MDL0!"
        );
    }
}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////
////   Base section classes
////

const char* MDL0::Section::nameForType(SectionType type)
{
    switch (type)
    {
    case SectionType::Links:
        return "Links";

    case SectionType::Bone:
        return "Bone";

    case SectionType::VertexArray:
        return "VertexArray";

    case SectionType::NormalArray:
        return "NormalArray";

    case SectionType::ColourArray:
        return "ColourArray";

    case SectionType::TexCoordArray:
        return "TexCoordArray";

    case SectionType::Material:
        return "Material";

    case SectionType::Shader:
        return "Shader";

    case SectionType::Object:
        return "Object";

    case SectionType::TextureLink:
        return "TextureLink";

    case SectionType::NONE:
        return "NONE";

    default:
        return "???";
    }
}

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

MDL0* MDL0::Section::getMDL0() const
{
    return mdl0;
}

std::string MDL0::Section::getName() const
{
    return name;
}

MDL0::ArraySection::ArraySection(MDL0* mdl0, const std::string& name) :
    Section(mdl0, name),
    count{0},
    data{}
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

MDL0::SectionCallback::~SectionCallback() = default;


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

    case Type::DrawOpa:
        return "DrawOpa";

    default:
        return "???";
    }
}

uint32_t MDL0::Links::sizeFor(Type type)
{
    switch (type)
    {
    case Type::NodeTree:
        return 0x5;

    case Type::DrawOpa:
        return 0x8;

    default:
        return 0x0;
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

uint32_t MDL0::Links::getCount() const
{
    switch (linksType)
    {
    case Type::NodeTree:
        return mdl0->count<Bone>();

    case Type::DrawOpa:
        return static_cast<uint32_t>(drawOpaLinks.size());
    
    default:
        return 0;
    }
}

void MDL0::Links::link(Object* obj, Material* mat, Bone* bone)
{
    assertDrawOpaSection();
    drawOpaLinks.push_back({obj, mat, bone});
}

std::vector<MDL0::Links::DrawOpaLink> MDL0::Links::getLinks() const
{
    assertDrawOpaSection();
    return drawOpaLinks;
}

void MDL0::Links::entryCallback(Section* instance, bool add)
{
    if (add) // ignore if not removing
    {
        return;
    }

    if (linksType == Type::DrawOpa)
    {
        for (size_t i = 0; i < drawOpaLinks.size(); ++i)
        {
            if (drawOpaLinks.at(i).obj == instance
                || drawOpaLinks.at(i).mat == instance 
                || drawOpaLinks.at(i).bone == instance
            )
            {
                drawOpaLinks.erase(drawOpaLinks.begin() + i);
                break;
            }
        }
    }
}

void MDL0::Links::assertDrawOpaSection() const
{
    if (linksType != Type::DrawOpa)
    {
        throw BRRESError("MDL0: This Links section must be of type DrawOpa!");
    }
}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////
////   Bone section
////

MDL0::Bone* MDL0::Bone::flatNext(Bone* bone)
{
    if (bone == nullptr)
    {
        return nullptr;
    }

    if (bone->child != nullptr)
    {
        return bone->child;
    }
    else if (bone->next != nullptr)
    {
        return bone->next;
    }

    Bone* parent = bone->parent;
    while (parent != nullptr)
    {
        if (parent->next != nullptr)
        {
            return parent->next;
        }
        parent = parent->parent;
    }

    return nullptr;
}

MDL0::Bone::Bone(MDL0* mdl0, const std::string& name) :
    Section(mdl0, name),
    parent{nullptr},
    child{nullptr},
    next{nullptr},
    prev{nullptr},
    pos{0, 0, 0},
    rot{0, 0, 0},
    scale{1, 1, 1},
    visible{true}
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
    if (bone != nullptr)
    {
        assertSameMDL0(bone);
        assertNotThis(bone);
        assertNotChild(bone);
    }

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
        if (insertPrivate(bone->child, this) == nullptr)
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
    assertNotNull(bone);

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

void MDL0::Bone::setPosition(Vector3f position)
{
    pos = position;
}

void MDL0::Bone::setRotation(Vector3f rotation)
{
    rot = rotation;
}

void MDL0::Bone::setScale(Vector3f scale)
{
    this->scale = scale;
}

void MDL0::Bone::setVisible(bool visible)
{
    visible = true;
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

bool MDL0::Bone::isVisible() const
{
    return visible;
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
    bone->prev = nullptr;

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
            "MDL0: The specified bone is the same as this (%s)!",
            name.c_str()
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

void MDL0::Bone::assertNotNull(Bone* bone) const
{
    if (bone == nullptr)
    {
        throw BRRESError("MDL0: The specified bone is `nullptr`!");
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

uint8_t MDL0::VertexArray::byteCount(Format format)
{
    switch (format)
    {
    case Format::UInt8:
    case Format::Int8:
        return 1;

    case Format::UInt16:
    case Format::Int16:
        return 2;

    case Format::Float:
        return 4;

    default:
        return 0;
    }
}

MDL0::VertexArray::VertexArray(MDL0* mdl0, const std::string& name) :
    ArraySection(mdl0, name),
    comps{Components::XYZ},
    format{Format::Float},
    divisor{0}
{

}

MDL0::VertexArray::~VertexArray() = default;

MDL0::SectionType MDL0::VertexArray::getType() const
{
    return SectionType::VertexArray;
}

float MDL0::VertexArray::getComponent(Buffer& buffer)
{
    uint8_t b; uint16_t s; float f;
    switch (format)
    {
    case Format::UInt8:
    case Format::Int8:
        b = buffer.get();
        data.put(b);
        return (format == Format::UInt8 ? (float)b : (float)((int8_t)b)) / (1 << divisor);

    case Format::UInt16:
    case Format::Int16:
        s = buffer.getShort();
        data.putShort(s);
        return (format == Format::UInt16 ? (float)s : (float)((int16_t)s)) / (1 << divisor);

    case Format::Float:
        f = buffer.getFloat();
        data.putFloat(f);
        return f;

    default:
        return 0.f;
    }
}

void MDL0::VertexArray::setData(Buffer& buffer, Components comps, Format format)
{
    const uint8_t compCount = componentCount(comps);
    const uint8_t compSize = byteCount(format);
    const uint8_t elemSize = compCount * compSize;
    count = static_cast<uint16_t>(buffer.remaining() / elemSize);

    this->comps = comps;
    this->format = format;

    const float min = count == 0 ? 0 : std::numeric_limits<float>::lowest();
    const float max = count == 0 ? 0 : std::numeric_limits<float>::max();

    boxMin = Vector3f(max, max, max);
    boxMax = Vector3f(min, min, min);

    data = Buffer(count * elemSize);
    while (buffer.remaining() >= elemSize)
    {
        for (uint32_t i = 0; i < compCount; ++i)
        {
            float val = getComponent(buffer);

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
}

void MDL0::VertexArray::setDivisor(uint8_t divisor)
{
    this->divisor = divisor;
}

void MDL0::VertexArray::setBoxMin(const Vector3f& boxMin)
{
    this->boxMin = boxMin;
}

void MDL0::VertexArray::setBoxMax(const Vector3f& boxMax)
{
    this->boxMax = boxMax;
}

MDL0::VertexArray::Components MDL0::VertexArray::getComponentsType() const
{
    return comps;
}

MDL0::VertexArray::Format MDL0::VertexArray::getFormat() const
{
    return format;
}

uint8_t MDL0::VertexArray::getDivisor() const
{
    return divisor;
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

uint8_t MDL0::NormalArray::byteCount(Format format)
{
    switch (format)
    {
    case Format::UInt8:
    case Format::Int8:
        return 1;

    case Format::UInt16:
    case Format::Int16:
        return 2;

    case Format::Float:
        return 4;

    default:
        return 0;
    }
}

MDL0::NormalArray::NormalArray(MDL0* mdl0, const std::string& name) :
    ArraySection(mdl0, name),
    comps{Components::Normal},
    format{Format::Float},
    divisor{0}
{

}

MDL0::NormalArray::~NormalArray() = default;

MDL0::SectionType MDL0::NormalArray::getType() const
{
    return SectionType::NormalArray;
}

void MDL0::NormalArray::setData(Buffer& buffer, Components comps, Format format)
{
    const uint8_t elemSize = componentCount(comps) * byteCount(format);
    count = static_cast<uint16_t>(buffer.remaining() / elemSize);

    data = Buffer(count * elemSize);
    data.putArray(*buffer + buffer.position(), data.capacity());
    buffer.position(data.capacity());
    data.flip();

    this->comps = comps;
    this->format = format;
}

void MDL0::NormalArray::setDivisor(uint8_t divisor)
{
    this->divisor = divisor;
}

MDL0::NormalArray::Components MDL0::NormalArray::getComponentsType() const
{
    return comps;
}

MDL0::NormalArray::Format MDL0::NormalArray::getFormat() const
{
    return format;
}

uint8_t MDL0::NormalArray::getDivisor() const
{
    return divisor;
}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////
////  Colour array section

uint8_t MDL0::ColourArray::byteCount(Format format)
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

uint8_t MDL0::ColourArray::componentCount(Format format)
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

MDL0::ColourArray::ColourArray(MDL0* mdl0, const std::string& name) :
    ArraySection(mdl0, name),
    format{Format::RGBA8}
{

}

MDL0::ColourArray::~ColourArray() = default;

MDL0::SectionType MDL0::ColourArray::getType() const
{
    return SectionType::ColourArray;
}

void MDL0::ColourArray::setData(Buffer& buffer, Format format)
{
    const uint8_t elemSize = byteCount(format);
    count = static_cast<uint32_t>(buffer.remaining() / elemSize);

    data = Buffer(count * elemSize);
    data.putArray(*buffer + buffer.position(), data.capacity());
    buffer.position(data.capacity());
    data.flip();

    this->format = format;
}

MDL0::ColourArray::Format MDL0::ColourArray::getFormat() const
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

uint8_t MDL0::TexCoordArray::byteCount(Format format)
{
    switch (format)
    {
    case Format::UInt8:
    case Format::Int8:
        return 1;

    case Format::UInt16:
    case Format::Int16:
        return 2;

    case Format::Float:
        return 4;

    default:
        return 0;
    }
}

MDL0::TexCoordArray::TexCoordArray(MDL0* mdl0, const std::string& name) :
    ArraySection(mdl0, name),
    comps{Components::ST},
    format{Format::Float},
    divisor{0}
{

}

MDL0::TexCoordArray::~TexCoordArray() = default;

MDL0::SectionType MDL0::TexCoordArray::getType() const
{
    return SectionType::TexCoordArray;
}

float MDL0::TexCoordArray::getComponent(Buffer& buffer)
{
    uint8_t b; uint16_t s; float f;
    switch (format)
    {
    case Format::UInt8:
    case Format::Int8:
        b = buffer.get();
        data.put(b);
        return (format == Format::UInt8 ? (float)b : (float)((int8_t)b)) / (1 << divisor);

    case Format::UInt16:
    case Format::Int16:
        s = buffer.getShort();
        data.putShort(s);
        return (format == Format::UInt16 ? (float)s : (float)((int16_t)s)) / (1 << divisor);

    case Format::Float:
        f = buffer.getFloat();
        data.putFloat(f);
        return f;

    default:
        return 0.f;
    }
}

void MDL0::TexCoordArray::setData(Buffer& buffer, Components comps, Format format)
{
    const uint8_t compCount = componentCount(comps);
    const uint8_t compSize = byteCount(format);
    const uint8_t elemSize = compCount * compSize;
    count = static_cast<uint16_t>(buffer.remaining() / elemSize);

    this->comps = comps;
    this->format = format;

    const float min = count == 0 ? 0 : std::numeric_limits<float>::lowest();
    const float max = count == 0 ? 0 : std::numeric_limits<float>::max();

    boxMin = Vector2f(max, max);
    boxMax = Vector2f(min, min);

    data = Buffer(count * elemSize);
    while (buffer.remaining() >= elemSize)
    {
        for (uint32_t i = 0; i < compCount; ++i)
        {
            float val = getComponent(buffer);

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
}

void MDL0::TexCoordArray::setDivisor(uint8_t divisor)
{
    this->divisor = divisor;
}

void MDL0::TexCoordArray::setBoxMin(const Vector2f& boxMin)
{
    this->boxMin = boxMin;
}

void MDL0::TexCoordArray::setBoxMax(const Vector2f& boxMax)
{
    this->boxMax = boxMax;
}

MDL0::TexCoordArray::Components MDL0::TexCoordArray::getComponentsType() const
{
    return comps;
}

MDL0::TexCoordArray::Format MDL0::TexCoordArray::getFormat() const
{
    return format;
}

uint8_t MDL0::TexCoordArray::getDivisor() const
{
    return divisor;
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
////   Material section
////

MDL0::Material::Material(MDL0* mdl0, const std::string& name) :
    Section(mdl0, name),
    xlu{false},
    cullMode{CullMode::Inside},
    alphaMode{false, AlphaMode::Source::SourceAlpha, AlphaMode::Dest::InverseSourceAlpha},
    layers{},
    shader{nullptr}
{

}

MDL0::Material::~Material()
{
    for (Layer* layer : layers)
    {
        delete layer;
    }
}

MDL0::SectionType MDL0::Material::getType() const
{
    return SectionType::Material;
}

MDL0::Material::Layer* MDL0::Material::addLayer(TextureLink* link)
{
    Layer* layer = new Layer(this, link);
    layers.push_back(layer);
    link->addReference(layer);
    return layer;
}

std::vector<MDL0::Material::Layer*> MDL0::Material::getLayers() const
{
    return layers;
}

uint32_t MDL0::Material::getLayerCount() const
{
    return static_cast<uint32_t>(layers.size());
}

void MDL0::Material::setIsXLU(bool xlu)
{
    this->xlu = xlu;
}

void MDL0::Material::setCullMode(CullMode mode)
{
    cullMode = mode;
}

void MDL0::Material::setAlphaMode(AlphaMode mode)
{
    alphaMode = mode;
}

void MDL0::Material::setShader(Shader* shader)
{
    assertSameMDL0(shader);
    this->shader = shader;
}

bool MDL0::Material::isXLU() const
{
    return xlu;
}

MDL0::Material::CullMode MDL0::Material::getCullMode() const
{
    return cullMode;
}

MDL0::Material::AlphaMode MDL0::Material::getAlphaMode() const
{
    return alphaMode;
}

MDL0::Shader* MDL0::Material::getShader() const
{
    return shader;
}

void MDL0::Material::setGraphicsCode(Buffer& data)
{
    gcode = Buffer(data.remaining());
    gcode.put(data).flip();
}

Buffer MDL0::Material::getGraphicsCode() const
{
    return gcode.duplicate();
}

void MDL0::Material::entryCallback(Section* instance, bool add)
{
    if (add) // ignore if not removing
    {
        return;
    }

    if (instance == shader)
    {
        shader = nullptr;
    }
}

void MDL0::Material::assertSameMDL0(Section* instance) const
{
    if (instance != nullptr && instance->getMDL0() != mdl0)
    {
        throw BRRESError("MDL0: The specified section is not owned by the same MDL0!");
    }
}

void MDL0::Material::assertHasRemainingLayers() const
{
    if (layers.size() >= MAX_LAYER_COUNT)
    {
        throw BRRESError(Strings::format(
            "MDL0: You cannot have more than %d layers per material!",
            MAX_LAYER_COUNT
        ));
    }
}

////// Layer class /////////////////////

MDL0::Material::Layer::Layer(Material* material, TextureLink* link) :
    mat{material},
    link{link},
    wrapMode{TextureWrap::Repeat},
    minFilter{MinFilter::Linear},
    magFilter{MagFilter::Linear},
    lodBias{0.f},
    anisotropyFiltering{AnisotropyFiltering::One},
    clampBias{false},
    texelInterpolate{false}
{

}

MDL0::Material::Layer::~Layer() = default;

MDL0::Material* MDL0::Material::Layer::getMaterial() const
{
    return mat;
}

MDL0::TextureLink* MDL0::Material::Layer::getTextureLink() const
{
    return link;
}

void MDL0::Material::Layer::setTextureWrapMode(TextureWrap mode)
{
    wrapMode = mode;
}

void MDL0::Material::Layer::setMinFilter(MinFilter filter)
{
    minFilter = filter;
}

void MDL0::Material::Layer::setMagFilter(MagFilter filter)
{
    magFilter = filter;
}

void MDL0::Material::Layer::setLODBias(float bias)
{
    lodBias = bias;
}

void MDL0::Material::Layer::setMaxAnisotropyFiltering(AnisotropyFiltering val)
{
    anisotropyFiltering = val;
}

void MDL0::Material::Layer::setClampBiasEnabled(bool enable)
{
    clampBias = enable;
}

void MDL0::Material::Layer::setUsesTexelInterpolate(bool enable)
{
    texelInterpolate = enable;
}

MDL0::Material::Layer::TextureWrap MDL0::Material::Layer::getTextureWrapMode() const
{
    return wrapMode;
}

MDL0::Material::Layer::MinFilter MDL0::Material::Layer::getMinFilter() const
{
    return minFilter;
}

MDL0::Material::Layer::MagFilter MDL0::Material::Layer::getMagFilter() const
{
    return magFilter;
}

float MDL0::Material::Layer::getLODBias() const
{
    return lodBias;
}

MDL0::Material::Layer::AnisotropyFiltering MDL0::Material::Layer::getMaxAnisotropyFiltering() const
{
    return anisotropyFiltering;
}

bool MDL0::Material::Layer::isClampBiasEnabled() const
{
    return clampBias;
}

bool MDL0::Material::Layer::usesTexelInterpolate() const
{
    return texelInterpolate;
}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////
////   Shader section
////

MDL0::Shader::Shader(MDL0* mdl0, const std::string& name) :
    Section(mdl0, name),
    gcode{}
{
    for (uint8_t i = 0; i < MAX_TEX_REF; ++i)
    {
        texRefs[i] = UNUSED;
    }
}

MDL0::Shader::~Shader() = default;

MDL0::SectionType MDL0::Shader::getType() const
{
    return SectionType::Shader;
}

void MDL0::Shader::setStageCount(uint8_t count)
{
    assertValidStageCount(count);
    stageCount = count;
}

void MDL0::Shader::setTexRef(uint8_t index, uint8_t layer)
{
    assertValidTexRefIndex(index);
    assertValidLayerRef(layer);

    texRefs[index] = layer;
}

uint8_t MDL0::Shader::getStageCount() const
{
    return stageCount;
}

uint8_t MDL0::Shader::getTexRef(uint8_t index) const
{
    assertValidTexRefIndex(index);
    return texRefs[index];
}

void MDL0::Shader::setGraphicsCode(Buffer& data)
{
    gcode = Buffer(data.remaining());
    gcode.put(data).flip();
}

Buffer MDL0::Shader::getGraphicsCode() const
{
    return gcode.duplicate();
}

void MDL0::Shader::assertValidStageCount(uint8_t count) const
{
    if (count > MAX_STAGE_COUNT)
    {
        throw BRRESError(Strings::format(
            "MDL0: The specified stage count is invalid! (%d > %d)",
            count, MAX_STAGE_COUNT
        ));
    }
}

void MDL0::Shader::assertValidTexRefIndex(uint8_t index) const
{
    if (index >= MAX_TEX_REF)
    {
        throw BRRESError(Strings::format(
            "MDL0: The specified texture ref index is invalid! (%d >= %d)",
            index, MAX_TEX_REF
        ));
    }
}

void MDL0::Shader::assertValidLayerRef(uint8_t layer) const
{
    if (layer >= Material::MAX_LAYER_COUNT && layer != UNUSED)
    {
        throw BRRESError(Strings::format(
            "MDL0: The specified material layer index is invalid! (%d)",
            layer
        ));
    }
}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////
////   Object section
////

uint8_t MDL0::Object::indexSizeFor(ArraySection* instance)
{
    return instance->getCount() > 0x100 ? 2 : 1;
}

MDL0::Object::Object(MDL0* mdl0, const std::string& name) :
    Section(mdl0, name),
    bone{nullptr},
    vertexArray{nullptr},
    vertexIndexSize{0},
    normalArray{nullptr},
    normalIndexSize{0},
    data{},
    vertexCount{0},
    faceCount{0}
{
    mdl0->addCallback(this);

    for (uint32_t i = 0; i < COLOUR_ARRAY_COUNT; ++i)
    {
        colourArrays[i] = nullptr;
        colourIndexSizes[i] = 0;
    }

    for (uint32_t i = 0; i < TEX_COORD_ARRAY_COUNT; ++i)
    {
        texCoordArrays[i] = nullptr;
        texCoordIndexSizes[i] = 0;
    }
}

MDL0::Object::~Object()
{
    mdl0->removeCallback(this);
}

MDL0::SectionType MDL0::Object::getType() const
{
    return SectionType::Object;
}

void MDL0::Object::setBone(Bone* bone)
{
    this->bone = bone;
}

void MDL0::Object::setVertexArray(VertexArray* array)
{
    vertexArray = array;
}

void MDL0::Object::setVertexArrayIndexSize(uint8_t size)
{
    assertValidArrayIndexSize(size);
    vertexIndexSize = size;
}

void MDL0::Object::setNormalArray(NormalArray* array)
{
    normalArray = array;
}

void MDL0::Object::setNormalArrayIndexSize(uint8_t size)
{
    assertValidArrayIndexSize(size);
    normalIndexSize = size;
}

void MDL0::Object::setColourArray(ColourArray* array, uint32_t index)
{
    assertValidColourArrayIndex(index);
    colourArrays[index] = array;
}

void MDL0::Object::setColourArrayIndexSize(uint32_t index, uint8_t size)
{
    assertValidColourArrayIndex(index);
    assertValidArrayIndexSize(size);
    colourIndexSizes[index] = size;
}

void MDL0::Object::setTexCoordArray(TexCoordArray* array, uint32_t index)
{
    assertValidTexCoordArrayIndex(index);
    texCoordArrays[index] = array;
}

void MDL0::Object::setTexCoordArrayIndexSize(uint32_t index, uint8_t size)
{
    assertValidTexCoordArrayIndex(index);
    assertValidArrayIndexSize(size);
    texCoordIndexSizes[index] = size;
}

void MDL0::Object::setGeometryData(const Buffer& buffer)
{
    assertVertexArraySet();

    data = buffer; data = data.slice();
    vertexCount = 0;
    faceCount = 0;

    const uint32_t vertexSize = getVertexSize();

    try
    {
        // TODO: bounds checking

        while (data.hasRemaining())
        {
            uint8_t primVal = data.get();
            PrimitiveType prim = static_cast<PrimitiveType>(primVal);

            uint16_t count = data.getShort();
            if (count == 0)
            {
                throw BRRESError("MDL0: Vertex count of 0 is invalid!");
            }

            switch (prim)
            {
            case PrimitiveType::QUADS:
                if (count % 4 != 0)
                {
                    throw BRRESError(Strings::format(
                        "MDL0: Non-multiple of 4 vertex count for quads primitive type! (%d)",
                        count
                    ));
                }
                faceCount += count / 4;
                break;

            case PrimitiveType::TRIANGLES:
                if (count % 3 != 0)
                {
                    throw BRRESError(Strings::format(
                        "MDL0: Non-multiple of 3 vertex count for triangles primitive type! (%d)",
                        count
                    ));
                }
                faceCount += count / 3;
                break;

            case PrimitiveType::TRIANGLE_STRIP:
                if (count < 3)
                {
                    throw BRRESError(Strings::format(
                        "MDL0: Less than 3 vertices for triangle strip primitive type! (%d)",
                        count
                    ));
                }
                faceCount += count - 2;
                break;

            case PrimitiveType::TRIANGLE_FAN:
                if (count < 3)
                {
                    throw BRRESError(Strings::format(
                        "MDL0: Less than 3 vertices for triangle fan primitive type! (%d)",
                        count
                    ));
                }
                faceCount += count - 2;
                break;

            case PrimitiveType::LINES:
                if (count % 2 != 0)
                {
                    throw BRRESError(Strings::format(
                        "MDL0: Non-multiple of 2 vertex count for lines primitive type! (%d)",
                        count
                    ));
                }
                // faceCount += count / 2; // not a face?
                break;

            case PrimitiveType::LINE_STRIP:
                if (count < 2)
                {
                    throw BRRESError(Strings::format(
                        "MDL0: Less than 2 vertices for line strip primitive type! (%d)",
                        count
                    ));
                }
                // faceCount += count - 1; // not a face?
                break;

            case PrimitiveType::POINTS:
                // faceCount += count; // not a face?
                break;

            default:
                throw BRRESError(Strings::format(
                    "MDL0: Invalid primitive type value! (0x%02X) %d",
                    primVal, vertexSize
                ));
            }

            vertexCount += count;

            data.position(data.position() + (count * vertexSize));
        }
    }
    catch (const BufferError&)
    {
        throw BRRESError("MDL0: Invalid Object geometry data!");
    }

    data.flip();
}

MDL0::Bone* MDL0::Object::getBone() const
{
    return bone;
}

MDL0::VertexArray* MDL0::Object::getVertexArray() const
{
    return vertexArray;
}

uint8_t MDL0::Object::getVertexArrayIndexSize() const
{
    assertVertexArraySet();
    return vertexIndexSize == 0 ? indexSizeFor(vertexArray) : vertexIndexSize;
}

MDL0::NormalArray* MDL0::Object::getNormalArray() const
{
    return normalArray;
}

uint8_t MDL0::Object::getNormalArrayIndexSize() const
{
    assertNormalArraySet();
    return normalIndexSize == 0 ? indexSizeFor(normalArray) : normalIndexSize;
}

MDL0::ColourArray* MDL0::Object::getColourArray(uint32_t index) const
{
    assertValidColourArrayIndex(index);
    return colourArrays[index];
}

uint8_t MDL0::Object::getColourArrayIndexSize(uint32_t index) const
{
    assertValidColourArrayIndex(index);
    assertColourArraySet(index);
    return colourIndexSizes[index] == 0
        ? indexSizeFor(colourArrays[index]) : colourIndexSizes[index];
}

MDL0::TexCoordArray* MDL0::Object::getTexCoordArray(uint32_t index) const
{
    assertValidTexCoordArrayIndex(index);
    return texCoordArrays[index];
}

uint8_t MDL0::Object::getTexCoordArrayIndexSize(uint32_t index) const
{
    assertValidTexCoordArrayIndex(index);
    assertTexCoordArraySet(index);
    return texCoordIndexSizes[index] == 0
        ? indexSizeFor(texCoordArrays[index]) : texCoordIndexSizes[index];
}

Buffer MDL0::Object::getGeometryData() const
{
    return data.duplicate();
}

uint32_t MDL0::Object::getGeometryDataSize() const
{
    return static_cast<uint32_t>(data.remaining());
}

uint32_t MDL0::Object::getVertexCount() const
{
    return vertexCount;
}

uint32_t MDL0::Object::getFaceCount() const
{
    return faceCount;
}

uint32_t MDL0::Object::getVertexSize() const
{
    uint32_t size = 0;

    if (vertexArray != nullptr)
    {
        size += getVertexArrayIndexSize();
    }

    if (normalArray != nullptr)
    {
        size += getNormalArrayIndexSize();
    }

    for (uint32_t i = 0; i < COLOUR_ARRAY_COUNT; ++i)
    {
        if (colourArrays[i] != nullptr)
        {
            size += getColourArrayIndexSize(i);
        }
    }

    for (uint32_t i = 0; i < TEX_COORD_ARRAY_COUNT; ++i)
    {
        if (texCoordArrays[i] != nullptr)
        {
            size += getTexCoordArrayIndexSize(i);
        }
    }

    return size;
}

void MDL0::Object::entryCallback(Section* instance, bool add)
{
    if (add) // not removing, so ignore
    {
        return;
    }

    if (instance == vertexArray)
    {
        vertexArray = nullptr;
        return;
    }
    if (instance == normalArray)
    {
        normalArray = nullptr;
        return;
    }
}

void MDL0::Object::assertNotNull(Section* instance) const
{
    if (instance == nullptr)
    {
        throw BRRESError("MDL0: The specified section pointer is nullptr!");
    }
}

void MDL0::Object::assertValidColourArrayIndex(uint32_t index) const
{
    if (index >= COLOUR_ARRAY_COUNT)
    {
        throw BRRESError(Strings::format(
            "MDL0: The specified colour array index is invalid! (%d >= %d)",
            index, COLOUR_ARRAY_COUNT
        ));
    }
}

void MDL0::Object::assertValidTexCoordArrayIndex(uint32_t index) const
{
    if (index >= TEX_COORD_ARRAY_COUNT)
    {
        throw BRRESError(Strings::format(
            "MDL0: The specified texture coord array index is invalid! (%d >= %d)",
            index, TEX_COORD_ARRAY_COUNT
        ));
    }
}

void MDL0::Object::assertVertexArraySet() const
{
    if (vertexArray == nullptr)
    {
        throw BRRESError("MDL0: The vertex array of this Object is not set (`nullptr`)!");
    }
}

void MDL0::Object::assertNormalArraySet() const
{
    if (normalArray == nullptr)
    {
        throw BRRESError("MDL0: The normal array of this Object is not set (`nullptr`)!");
    }
}

void MDL0::Object::assertColourArraySet(uint32_t index) const
{
    if (colourArrays[index] == nullptr)
    {
        throw BRRESError(Strings::format(
            "MDL0: The colour array at the specified index is not set (`nullptr`)! (%d)",
            index
        ));
    }
}

void MDL0::Object::assertTexCoordArraySet(uint32_t index) const
{
    if (texCoordArrays[index] == nullptr)
    {
        throw BRRESError(Strings::format(
            "MDL0: The texture coord array at the specified index is not set (`nullptr`)! (%d)",
            index
        ));
    }
}

void MDL0::Object::assertValidArrayIndexSize(uint8_t size) const
{
    if (size > 2)
    {
        throw BRRESError(Strings::format(
            "MDL0: Array index size must not be more than 2! (%d)",
            (int)size
        ));
    }
}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////
////   Texture links section
////

MDL0::TextureLink::TextureLink(MDL0* mdl0, const std::string& name) :
    Section(mdl0, name)
{
    throw BRRESError("MDL0: Use MDL0::linkTEX0() to link a texture.");
}

MDL0::TextureLink::TextureLink(MDL0* mdl0, TEX0* tex0) :
    Section(mdl0, tex0->getName()),
    tex0{tex0},
    references{}
{
    mdl0->brres->registerCallback(this);
    mdl0->addCallback(this);
}

MDL0::TextureLink::~TextureLink()
{
    mdl0->removeCallback(this);
    mdl0->brres->unregisterCallback(this);
}

TEX0* MDL0::TextureLink::getTEX0() const
{
    return tex0;
}

uint32_t MDL0::TextureLink::getCount() const
{
    return static_cast<uint32_t>(references.size());
}

std::vector<MDL0::Material::Layer*> MDL0::TextureLink::getReferences() const
{
    return references;
}

void MDL0::TextureLink::subfileAdded(BRRESSubFile* subfile)
{

}

void MDL0::TextureLink::subfileRemoved(BRRESSubFile* subfile)
{
    if (subfile == tex0)
    {
        mdl0->remove<MDL0::TextureLink>(name);
    }
}

void MDL0::TextureLink::entryCallback(Section* instance, bool add)
{
    if (add) // ignore if not removing
    {
        return;
    }

    for (size_t i = 0; i < references.size(); ++i)
    {
        if (references.at(i)->getMaterial() == instance)
        {
            references.erase(references.begin() + i);
            --i;
        }
    }
}

void MDL0::TextureLink::addReference(Material::Layer* layer)
{
    references.push_back(layer);
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
CT_LIB_DECLARE_SECTION_CONTAINER_METHODS(MDL0::ColourArray)
CT_LIB_DECLARE_SECTION_CONTAINER_METHODS(MDL0::TexCoordArray)
CT_LIB_DECLARE_SECTION_CONTAINER_METHODS(MDL0::Material)
CT_LIB_DECLARE_SECTION_CONTAINER_METHODS(MDL0::Shader)
CT_LIB_DECLARE_SECTION_CONTAINER_METHODS(MDL0::Object)
CT_LIB_DECLARE_SECTION_CONTAINER_METHODS(MDL0::TextureLink)

#undef CT_LIB_DECLARE_SECTION_CONTAINER_METHODS
}
