//////////////////////////////////////////////////
//  Copyright (c) 2020 Nara Hiero
//
// This file is licensed under GPLv3+
// Refer to the `License.txt` file included.
//////////////////////////////////////////////////

#include "BRRES/RW/BRRESRWCommon.hpp"

namespace CTLib
{

////////////////////////////////////////
// Constants

constexpr uint32_t NUM_SECTIONS = 14;
constexpr uint32_t BONE_SECTION_SIZE = 0xD0;

template <class Type>
void addMDL0SectionsToStringTable(BRRESStringTable* table, MDL0* mdl0)
{
    for (Type* instance : mdl0->getAll<Type>())
    {
        addToStringTable(table, instance->getName());
    }
}

void addMDL0StringsToTable(BRRESStringTable* table, MDL0* mdl0)
{
    addMDL0SectionsToStringTable<MDL0::Links>(table, mdl0);
    addMDL0SectionsToStringTable<MDL0::Bone>(table, mdl0);
    addMDL0SectionsToStringTable<MDL0::VertexArray>(table, mdl0);
    addMDL0SectionsToStringTable<MDL0::NormalArray>(table, mdl0);
    addMDL0SectionsToStringTable<MDL0::ColourArray>(table, mdl0);
    addMDL0SectionsToStringTable<MDL0::TexCoordArray>(table, mdl0);
    addMDL0SectionsToStringTable<MDL0::Material>(table, mdl0);
    addMDL0SectionsToStringTable<MDL0::Shader>(table, mdl0);
    addMDL0SectionsToStringTable<MDL0::Object>(table, mdl0);
    addMDL0SectionsToStringTable<MDL0::TextureLink>(table, mdl0);
}

template <class Type>
uint32_t calculateMDL0SectionSize(Type* instance);

template <>
uint32_t calculateMDL0SectionSize<MDL0::Links>(MDL0::Links* instance)
{
    return padNumber((instance->getCount() * 0x5) + 0x1, 0x10);
}

template <>
uint32_t calculateMDL0SectionSize<MDL0::Bone>(MDL0::Bone* instance)
{
    return BONE_SECTION_SIZE;
}

template <>
uint32_t calculateMDL0SectionSize<MDL0::VertexArray>(MDL0::VertexArray* instance)
{
    return 0x40 + padNumber(static_cast<uint32_t>(instance->getData().remaining()), 0x10);
}

template <>
uint32_t calculateMDL0SectionSize<MDL0::NormalArray>(MDL0::NormalArray* instance)
{
    return 0x20 + padNumber(static_cast<uint32_t>(instance->getData().remaining()), 0x10);
}

template <>
uint32_t calculateMDL0SectionSize<MDL0::ColourArray>(MDL0::ColourArray* instance)
{
    return 0x20 + padNumber(static_cast<uint32_t>(instance->getData().remaining()), 0x10);
}

template <>
uint32_t calculateMDL0SectionSize<MDL0::TexCoordArray>(MDL0::TexCoordArray* instance)
{
    return 0x30 + padNumber(static_cast<uint32_t>(instance->getData().remaining()), 0x10);
}

template <>
uint32_t calculateMDL0SectionSize<MDL0::Material>(MDL0::Material* instance)
{
    return padNumber(0x418 + (instance->getLayerCount() * 0x34), 0x20) + 0x180;
}

template <>
uint32_t calculateMDL0SectionSize<MDL0::Shader>(MDL0::Shader* instance)
{
    return 0x200;
}

template <>
uint32_t calculateMDL0SectionSize<MDL0::Object>(MDL0::Object* instance)
{
    return 0x160 + padNumber(instance->getGeometryDataSize(), 0x10);
}

template <>
uint32_t calculateMDL0SectionSize<MDL0::TextureLink>(MDL0::TextureLink* instance)
{
    return padNumber(0x4 + (instance->getCount() * 0x8), 0x10);
}

template <class Type>
uint32_t calculateMDL0SectionsSize(MDL0* mdl0)
{
    uint32_t size = 0;
    for (Type* instance : mdl0->getAll<Type>())
    {
        size += calculateMDL0SectionSize<Type>(instance);
    }
    return size;
}

uint32_t calculateMDL0Size(MDL0* mdl0)
{
    uint32_t size = 0x8C; // file + mdl0 header size

    // bone-link table size
    size += 0x4 + (mdl0->count<MDL0::Bone>() * 0x4);

    // size of the section index groups
    size = padNumber(size, 0x8);
    size += BRRESIndexGroup::getSizeInBytesForCount(mdl0->count<MDL0::Links>());
    size += BRRESIndexGroup::getSizeInBytesForCount(mdl0->count<MDL0::Bone>());
    size += BRRESIndexGroup::getSizeInBytesForCount(mdl0->count<MDL0::VertexArray>());
    size += BRRESIndexGroup::getSizeInBytesForCount(mdl0->count<MDL0::NormalArray>());
    size += BRRESIndexGroup::getSizeInBytesForCount(mdl0->count<MDL0::ColourArray>());
    size += BRRESIndexGroup::getSizeInBytesForCount(mdl0->count<MDL0::TexCoordArray>());
    size += BRRESIndexGroup::getSizeInBytesForCount(mdl0->count<MDL0::Material>());
    size += BRRESIndexGroup::getSizeInBytesForCount(mdl0->count<MDL0::Shader>());
    size += BRRESIndexGroup::getSizeInBytesForCount(mdl0->count<MDL0::Object>());
    size += BRRESIndexGroup::getSizeInBytesForCount(mdl0->count<MDL0::TextureLink>());

    /// Sections size //////////////////
    size = padNumber(size, 0x10);

    size += calculateMDL0SectionsSize<MDL0::Links>(mdl0);
    size += calculateMDL0SectionsSize<MDL0::Bone>(mdl0);
    size += calculateMDL0SectionsSize<MDL0::VertexArray>(mdl0);
    size += calculateMDL0SectionsSize<MDL0::NormalArray>(mdl0);
    size += calculateMDL0SectionsSize<MDL0::ColourArray>(mdl0);
    size += calculateMDL0SectionsSize<MDL0::TexCoordArray>(mdl0);

    size = padNumber(size, 0x20); // material sections needs special padding
    size += calculateMDL0SectionsSize<MDL0::Material>(mdl0);
    size += calculateMDL0SectionsSize<MDL0::Shader>(mdl0);
    size += calculateMDL0SectionsSize<MDL0::Object>(mdl0);
    size += calculateMDL0SectionsSize<MDL0::TextureLink>(mdl0);

    return padNumber(size, 0x10);
}

// info required in file header
struct MDL0Info
{

    // size of file in bytes
    uint32_t size;

    // offset to outer BRRES file
    int32_t offToBRRES;
    
    // offset to name
    uint32_t nameOff;
};

// info required to create index groups
struct MDL0GroupsInfo
{

    // number of index groups in the MDL0
    uint32_t count;

    // map of <section-num, index>
    std::map<uint32_t, uint32_t> indices;

    // map of <entry, section>
    std::map<BRRESIndexGroupEntry*, MDL0::Section*> sections;
};

// offsets required in MDL0
struct MDL0Offsets
{

    // offset to the first index group
    uint32_t groupOff;

    // offsets to all index groups
    std::map<BRRESIndexGroup*, uint32_t> groupOffs;

    // offsets to all sections
    std::map<MDL0::Section*, uint32_t> sectionOffs;

    // offsets to all material layers
    std::map<MDL0::Material::Layer*, uint32_t> layerOffs;
};

struct MDL0SectionIndices
{

    // indices of all sections
    std::map<MDL0::Section*, uint16_t> indices;
};

void createInfo(
    MDL0* mdl0, MDL0Info* info, int32_t offToBRRES, BRRESStringTable* table, uint32_t tableOff
)
{
    info->size = calculateMDL0Size(mdl0);
    info->offToBRRES = offToBRRES;
    info->nameOff = table->offsets.at(mdl0->getName()) + tableOff;
}

template <class Type>
void addIfNotEmpty(MDL0* mdl0, MDL0GroupsInfo* info)
{
    constexpr uint32_t SECTION = static_cast<uint32_t>(Type::TYPE);
    if (mdl0->count<Type>() > 0)
    {
        info->indices.insert(std::map<uint32_t, uint32_t>::value_type(SECTION, info->count));
        ++info->count;
    }
}

void createGroupsInfo(MDL0* mdl0, MDL0GroupsInfo* info)
{
    info->count = 0;

    addIfNotEmpty<MDL0::Links>(mdl0, info);
    addIfNotEmpty<MDL0::Bone>(mdl0, info);
    addIfNotEmpty<MDL0::VertexArray>(mdl0, info);
    addIfNotEmpty<MDL0::NormalArray>(mdl0, info);
    addIfNotEmpty<MDL0::ColourArray>(mdl0, info);
    addIfNotEmpty<MDL0::TexCoordArray>(mdl0, info);
    addIfNotEmpty<MDL0::Material>(mdl0, info);
    addIfNotEmpty<MDL0::Shader>(mdl0, info);
    addIfNotEmpty<MDL0::Object>(mdl0, info);
    addIfNotEmpty<MDL0::TextureLink>(mdl0, info);
}

template <class Type>
void createIndexGroup(MDL0* mdl0, BRRESIndexGroup* groups, MDL0GroupsInfo* info)
{
    constexpr uint32_t SECTION = static_cast<uint32_t>(Type::TYPE);
    if (info->indices.count(SECTION) > 0)
    {
        BRRESIndexGroup* group = groups + info->indices.at(SECTION);
        for (Type* instance : mdl0->getAll<Type>())
        {
            BRRESIndexGroupEntry* entry = group->addEntry(instance->getName());
            info->sections.insert(
                std::map<BRRESIndexGroupEntry*, MDL0::Section*>::value_type(entry, instance)
            );
        }
    }
}

template <>
void createIndexGroup<MDL0::Bone>(MDL0* mdl0, BRRESIndexGroup* groups, MDL0GroupsInfo* info)
{
    constexpr uint32_t SECTION = static_cast<uint32_t>(MDL0::SectionType::Bone);
    if (info->indices.count(SECTION))
    {
        BRRESIndexGroup* group = groups + info->indices.at(SECTION);

        MDL0::Bone* bone = mdl0->getRootBone();
        while (bone != nullptr)
        {
            BRRESIndexGroupEntry* entry = group->addEntry(bone->getName());
            info->sections.insert(
                std::map<BRRESIndexGroupEntry*, MDL0::Section*>::value_type(entry, bone)
            );

            bone = MDL0::Bone::flatNext(bone);
        }
    }
}

void createIndexGroups(MDL0* mdl0, BRRESIndexGroup* groups, MDL0GroupsInfo* info)
{
    createIndexGroup<MDL0::Links>(mdl0, groups, info);
    createIndexGroup<MDL0::Bone>(mdl0, groups, info);
    createIndexGroup<MDL0::VertexArray>(mdl0, groups, info);
    createIndexGroup<MDL0::NormalArray>(mdl0, groups, info);
    createIndexGroup<MDL0::ColourArray>(mdl0, groups, info);
    createIndexGroup<MDL0::TexCoordArray>(mdl0, groups, info);
    createIndexGroup<MDL0::Material>(mdl0, groups, info);
    createIndexGroup<MDL0::Shader>(mdl0, groups, info);
    createIndexGroup<MDL0::Object>(mdl0, groups, info);
    createIndexGroup<MDL0::TextureLink>(mdl0, groups, info);
}

template <class Type>
void createMDL0SectionOffsets(MDL0* mdl0, MDL0Offsets* offsets, uint32_t& pos)
{
    for (Type* instance : mdl0->getAll<Type>())
    {
        offsets->sectionOffs.insert(std::map<MDL0::Section*, uint32_t>::value_type(instance, pos));
        pos += calculateMDL0SectionSize<Type>(instance);
    }
}

template <>
void createMDL0SectionOffsets<MDL0::Bone>(MDL0* mdl0, MDL0Offsets* offsets, uint32_t& pos)
{
    MDL0::Bone* bone = mdl0->getRootBone();
    while (bone != nullptr)
    {
        offsets->sectionOffs.insert(std::map<MDL0::Section*, uint32_t>::value_type(bone, pos));
        pos += BONE_SECTION_SIZE;

        bone = MDL0::Bone::flatNext(bone);
    }
}

void createMDL0MaterialLayerOffsets(MDL0* mdl0, MDL0Offsets* offsets)
{
    for (MDL0::Material* instance : mdl0->getAll<MDL0::Material>())
    {
        uint32_t pos = offsets->sectionOffs.at(instance) + 0x418;

        for (MDL0::Material::Layer* layer : instance->getLayers())
        {
            offsets->layerOffs.insert(
                std::map<MDL0::Material::Layer*, uint32_t>::value_type(layer, pos)
            );
            pos += 0x34;
        }
    }
}

void createOffsets(MDL0* mdl0, MDL0Offsets* offsets, BRRESIndexGroup* groups, uint32_t count)
{
    uint32_t pos = 0x8C; // skip file and mdl0 headers

    // bone-link table
    pos += 0x4 + (mdl0->count<MDL0::Bone>() * 0x4);

    /// Index groups offsets ///////////

    pos = padNumber(pos, 0x8);

    offsets->groupOff = pos;
    for (uint32_t i = 0; i < count; ++i)
    {
        BRRESIndexGroup* group = groups + i;
        offsets->groupOffs.insert(std::map<BRRESIndexGroup*, uint32_t>::value_type(group, pos));
        pos += group->getSizeInBytes();
    }

    /// Sections offsets ///////////////

    pos = padNumber(pos, 0x10);

    createMDL0SectionOffsets<MDL0::Links>(mdl0, offsets, pos);
    createMDL0SectionOffsets<MDL0::Bone>(mdl0, offsets, pos);
    createMDL0SectionOffsets<MDL0::VertexArray>(mdl0, offsets, pos);
    createMDL0SectionOffsets<MDL0::NormalArray>(mdl0, offsets, pos);
    createMDL0SectionOffsets<MDL0::ColourArray>(mdl0, offsets, pos);
    createMDL0SectionOffsets<MDL0::TexCoordArray>(mdl0, offsets, pos);

    pos = padNumber(pos, 0x20); // material sections needs special padding
    createMDL0SectionOffsets<MDL0::Material>(mdl0, offsets, pos);
    createMDL0SectionOffsets<MDL0::Shader>(mdl0, offsets, pos);
    createMDL0SectionOffsets<MDL0::Object>(mdl0, offsets, pos);
    createMDL0SectionOffsets<MDL0::TextureLink>(mdl0, offsets, pos);

    /// Material layer offsets /////////

    createMDL0MaterialLayerOffsets(mdl0, offsets);
}

void createMDL0BoneSectionIndices(MDL0* mdl0, MDL0SectionIndices* indices)
{
    uint16_t index = 0;
    MDL0::Bone* bone = mdl0->getRootBone();
    while (bone != nullptr)
    {
        indices->indices.insert(std::map<MDL0::Section*, uint16_t>::value_type(bone, index));
        ++index;

        bone = MDL0::Bone::flatNext(bone);
    }
}

template <class Type>
void createMDL0GeneralSectionIndices(MDL0* mdl0, MDL0SectionIndices* indices)
{
    uint16_t index = 0;
    for (Type* instance : mdl0->getAll<Type>())
    {
        indices->indices.insert(std::map<MDL0::Section*, uint16_t>::value_type(instance, index));
        ++index;
    }
}

void createMDL0SectionsIndices(MDL0* mdl0, MDL0SectionIndices* indices)
{
    createMDL0BoneSectionIndices(mdl0, indices);
    createMDL0GeneralSectionIndices<MDL0::VertexArray>(mdl0, indices);
    createMDL0GeneralSectionIndices<MDL0::NormalArray>(mdl0, indices);
    createMDL0GeneralSectionIndices<MDL0::ColourArray>(mdl0, indices);
    createMDL0GeneralSectionIndices<MDL0::TexCoordArray>(mdl0, indices);
    createMDL0GeneralSectionIndices<MDL0::Material>(mdl0, indices);
    createMDL0GeneralSectionIndices<MDL0::Shader>(mdl0, indices);
    createMDL0GeneralSectionIndices<MDL0::Object>(mdl0, indices);
}

void resolveSectionGroupsOffsets(
    MDL0Offsets* offsets, BRRESIndexGroup* groups, MDL0GroupsInfo* info
)
{
    for (uint32_t g = 0; g < info->count; ++g)
    {
        BRRESIndexGroup* group = (groups + g);
        uint32_t groupOff = offsets->groupOffs.at(group);

        std::vector<BRRESIndexGroupEntry*> entries = group->getEntries();
        for (uint32_t i = 0; i < group->getEntryCount(); ++i)
        {
            BRRESIndexGroupEntry* entry = entries[i + 1];
            MDL0::Section* section = info->sections.at(entry);
            entry->setDataOffset(offsets->sectionOffs.at(section) - groupOff);
        }
    }
}

void writeMDL0FileHeader(
    Buffer& out, MDL0Info* info, MDL0GroupsInfo* groupsInfo, BRRESIndexGroup* groups,
    MDL0Offsets* offsets
)
{
    out.putArray((uint8_t*)"MDL0", 4);
    out.putInt(info->size);
    out.putInt(11); // version number
    out.putInt(static_cast<uint32_t>(info->offToBRRES));

    for (uint32_t i = 0; i < NUM_SECTIONS; ++i)
    {
        if (groupsInfo->indices.count(i) > 0)
        {
            BRRESIndexGroup* group = groups + groupsInfo->indices.at(i);
            out.putInt(offsets->groupOffs.at(group));
        }
        else // section not present
        {
            out.putInt(0);
        }
    }

    out.putInt(info->nameOff);
}

void writeMDL0Header(Buffer& out, MDL0* mdl0)
{
    out.putInt(0x40); // mdl0 header size
    out.putInt(static_cast<uint32_t>(-0x4C)); // offset to file header
    out.putInt(0); // unknown/unused
    out.putInt(0); // unknown/unused
    out.putInt(0); // vertex count
    out.putInt(0); // face count
    out.putInt(0); // unknown/unused
    out.putInt(mdl0->count<MDL0::Bone>()); // bone count
    out.putInt(0x01000000); // unknown
    out.putInt(0x40); // bone table offset
    out.putFloat(0.f).putFloat(0.f).putFloat(0.f); // box min
    out.putFloat(0.f).putFloat(0.f).putFloat(0.f); // box max
}

void writeMDL0BoneTable(Buffer& out, MDL0* mdl0)
{
    out.putInt(mdl0->count<MDL0::Bone>()); // entry count
    for (uint32_t i = 0; i < mdl0->count<MDL0::Bone>(); ++i)
    {
        out.putInt(i);
    }
}

void writeMDL0Groups(
    Buffer& out, BRRESIndexGroup* groups, uint32_t count, MDL0Offsets* offsets,
    BRRESStringTable* table, uint32_t tableOff
)
{
    out.position(offsets->groupOff);
    for (uint32_t i = 0; i < count; ++i)
    {
        (groups + i)->write(out, table, tableOff - static_cast<uint32_t>(out.position()));
    }
}

void writeMDL0NodeTreeSection(Buffer& out, MDL0* mdl0, MDL0SectionIndices* indices)
{
    MDL0::Bone* bone = mdl0->getRootBone();
    while (bone != nullptr)
    {
        out.put(static_cast<uint8_t>(MDL0::Links::Type::NodeTree)); // section command
        out.putShort(indices->indices.at(bone));
        out.putShort(bone->getParent() == nullptr ? 0 : indices->indices.at(bone->getParent()));

        bone = MDL0::Bone::flatNext(bone);
    }
}

void writeMDL0DrawOpaSection(Buffer& out, MDL0::Links* drawOpa, MDL0SectionIndices* indices)
{
    for (const MDL0::Links::DrawOpaLink& link : drawOpa->getLinks())
    {
        out.put(static_cast<uint8_t>(MDL0::Links::Type::DrawOpa)); // section command
        out.putShort(indices->indices.at(link.obj));
        out.putShort(indices->indices.at(link.mat));
        out.putShort(indices->indices.at(link.bone));
        out.put(0); // unknown/unused
    }
}

void writeMDL0LinksSections(
    Buffer& out, MDL0* mdl0, MDL0Offsets* offsets, MDL0SectionIndices* indices
)
{
    for (MDL0::Links* links : mdl0->getAll<MDL0::Links>())
    {
        out.position(offsets->sectionOffs.at(links));

        switch (links->getLinksType())
        {
        case MDL0::Links::Type::NodeTree:
            writeMDL0NodeTreeSection(out, mdl0, indices);
            break;

        case MDL0::Links::Type::DrawOpa:
            writeMDL0DrawOpaSection(out, links, indices);
            break;
        
        default:
            break;
        }

        out.put(0x1); // end section command
    }
}

uint32_t offsetForMDL0Bone(MDL0SectionIndices* indices, MDL0::Bone* base, MDL0::Bone* bone)
{
    return bone == nullptr ? 0 :
        (indices->indices.at(bone) - indices->indices.at(base)) * BONE_SECTION_SIZE;
}

void writeMDL0BoneSections(
    Buffer& out, MDL0* mdl0, MDL0Offsets* offsets, MDL0SectionIndices* indices,
    BRRESStringTable* table, uint32_t tableOff
)
{
    MDL0::Bone* bone = mdl0->getRootBone();
    while (bone != nullptr)
    {
        uint32_t pos = offsets->sectionOffs.at(bone);
        int32_t offToMDL0 = -static_cast<int32_t>(pos);
        uint32_t nameOff = (table->offsets.at(bone->getName()) + tableOff) - pos;

        Vector3f loc = bone->getPosition(), rot = bone->getRotation(),
            scale = bone->getScale();

        out.position(pos);
        out.putInt(BONE_SECTION_SIZE);
        out.putInt(static_cast<uint32_t>(offToMDL0));
        out.putInt(nameOff);
        out.putInt(indices->indices.at(bone)); // section index
        out.putInt(indices->indices.at(bone)); // section id
        out.putInt(0x31F); // unknown flags
        out.putInt(0); // billboard settings
        out.putInt(0); // unknown/unused
        scale.put(out); // scale
        rot.put(out); // rotation
        loc.put(out); // position
        out.putFloat(0).putFloat(0).putFloat(0); // box minimum
        out.putFloat(0).putFloat(0).putFloat(0); // box maximum
        out.putInt(offsetForMDL0Bone(indices, bone, bone->getParent()));
        out.putInt(offsetForMDL0Bone(indices, bone, bone->getFirstChild()));
        out.putInt(offsetForMDL0Bone(indices, bone, bone->getNext()));
        out.putInt(offsetForMDL0Bone(indices, bone, bone->getPrevious())); // previous sibling offset
        out.putInt(0); // unknown

        // 2 unused matrices 4x3
        out.putInt(0).putInt(0).putInt(0).putInt(0)
            .putInt(0).putInt(0).putInt(0).putInt(0)
            .putInt(0).putInt(0).putInt(0).putInt(0); // transformation matrix

        out.putInt(0).putInt(0).putInt(0).putInt(0)
            .putInt(0).putInt(0).putInt(0).putInt(0)
            .putInt(0).putInt(0).putInt(0).putInt(0); // inverse tranformation matrix

        bone = MDL0::Bone::flatNext(bone);
    }
}

void writeMDL0VerticesSections(
    Buffer& out, MDL0* mdl0, MDL0Offsets* offsets, MDL0SectionIndices* indices,
    BRRESStringTable* table, uint32_t tableOff
)
{
    for (MDL0::VertexArray* instance : mdl0->getAll<MDL0::VertexArray>())
    {
        uint32_t pos = offsets->sectionOffs.at(instance);
        int32_t offToMDL0 = -static_cast<int32_t>(pos);
        uint32_t nameOff = (table->offsets.at(instance->getName()) + tableOff) - pos;

        Vector3f boxMin = instance->getBoxMin(), boxMax = instance->getBoxMax();

        out.position(pos);
        out.putInt(calculateMDL0SectionSize<MDL0::VertexArray>(instance)); // section size
        out.putInt(static_cast<uint32_t>(offToMDL0));
        out.putInt(0x40); // data offset
        out.putInt(nameOff);
        out.putInt(indices->indices.at(instance)); // section index
        out.putInt(static_cast<uint32_t>(instance->getComponentsType()));
        out.putInt(0x4); // floats
        out.put(0); // divisor; unused for floats
        out.put(0x0C); // stride
        out.putShort(instance->getCount()); // vertex count
        boxMin.put(out);
        boxMax.put(out);

        out.position(pos + 0x40);
        out.put(instance->getData());
    }
}

void writeMDL0NormalsSections(
    Buffer& out, MDL0* mdl0, MDL0Offsets* offsets, MDL0SectionIndices* indices,
    BRRESStringTable* table, uint32_t tableOff
)
{
    for (MDL0::NormalArray* instance : mdl0->getAll<MDL0::NormalArray>())
    {
        uint32_t pos = offsets->sectionOffs.at(instance);
        int32_t offToMDL0 = -static_cast<int32_t>(pos);
        uint32_t nameOff = (table->offsets.at(instance->getName()) + tableOff) - pos;

        out.position(pos);
        out.putInt(calculateMDL0SectionSize<MDL0::NormalArray>(instance));
        out.putInt(static_cast<uint32_t>(offToMDL0));
        out.putInt(0x20); // data offset
        out.putInt(nameOff);
        out.putInt(indices->indices.at(instance)); // section index
        out.putInt(static_cast<uint32_t>(instance->getComponentsType()));
        out.putInt(0x4); // floats
        out.put(0); // divisor; unused for floats
        out.put(0x0C); // stride
        out.putShort(instance->getCount()); // normal count

        out.position(pos + 0x20);
        out.put(instance->getData());
    }
}

void writeMDL0ColoursSections(
    Buffer& out, MDL0* mdl0, MDL0Offsets* offsets, MDL0SectionIndices* indices,
    BRRESStringTable* table, uint32_t tableOff
)
{
    for (MDL0::ColourArray* instance : mdl0->getAll<MDL0::ColourArray>())
    {
        uint32_t pos = offsets->sectionOffs.at(instance);
        int32_t offToMDL0 = -static_cast<int32_t>(pos);
        uint32_t nameOff = (table->offsets.at(instance->getName()) + tableOff) - pos;

        uint32_t compType = MDL0::ColourArray::componentCount(instance->getFormat()) == 3
            ? 0x0 : 0x1;

        out.position(pos);
        out.putInt(calculateMDL0SectionSize<MDL0::ColourArray>(instance));
        out.putInt(static_cast<uint32_t>(offToMDL0));
        out.putInt(0x20); // data offset
        out.putInt(nameOff);
        out.putInt(indices->indices.at(instance)); // section index
        out.putInt(compType);
        out.putInt(static_cast<uint32_t>(instance->getFormat()));
        out.put(0x04); // stride
        out.put(0); // unknown/unused
        out.putShort(instance->getCount()); // colour count

        out.position(pos + 0x20);
        out.put(instance->getData());
    }
}

void writeMDL0TextureCoordsSections(
    Buffer& out, MDL0* mdl0, MDL0Offsets* offsets, MDL0SectionIndices* indices,
    BRRESStringTable* table, uint32_t tableOff
)
{
    for (MDL0::TexCoordArray* instance : mdl0->getAll<MDL0::TexCoordArray>())
    {
        uint32_t pos = offsets->sectionOffs.at(instance);
        int32_t offToMDL0 = -static_cast<int32_t>(pos);
        uint32_t nameOff = (table->offsets.at(instance->getName()) + tableOff) - pos;

        Vector2f boxMin = instance->getBoxMin(), boxMax = instance->getBoxMax();

        out.position(pos);
        out.putInt(calculateMDL0SectionSize<MDL0::TexCoordArray>(instance)); // section size
        out.putInt(static_cast<uint32_t>(offToMDL0));
        out.putInt(0x30); // data offset
        out.putInt(nameOff);
        out.putInt(indices->indices.at(instance)); // section index
        out.putInt(static_cast<uint32_t>(instance->getComponentsType()));
        out.putInt(0x4); // floats
        out.put(0); // divisor; unused for floats
        out.put(0x0C); // stride
        out.putShort(instance->getCount()); // texture coord count
        boxMin.put(out);
        boxMax.put(out);

        out.position(pos + 0x30);
        out.put(instance->getData());
    }
}

uint32_t getMDL0ShaderSectionOffset(MDL0::Material* instance, MDL0Offsets* offsets)
{
    return instance->getShader() == nullptr ? 0
        : offsets->sectionOffs.at(instance->getShader()) - offsets->sectionOffs.at(instance);
}

void writeMDL0MaterialSections(
    Buffer& out, MDL0* mdl0, MDL0Offsets* offsets, MDL0SectionIndices* indices,
    BRRESStringTable* table, uint32_t tableOff
)
{
    constexpr uint32_t MAX_LAYER_COUNT = 8;

    for (MDL0::Material* instance : mdl0->getAll<MDL0::Material>())
    {
        uint32_t pos = offsets->sectionOffs.at(instance);
        int32_t offToMDL0 = -static_cast<int32_t>(pos);
        uint32_t nameOff = (table->offsets.at(instance->getName()) + tableOff) - pos;

        uint32_t displayListOff = padNumber(0x418 + (instance->getLayerCount() * 0x34), 0x20);

        out.position(pos);
        out.putInt(calculateMDL0SectionSize<MDL0::Material>(instance));
        out.putInt(static_cast<uint32_t>(offToMDL0));
        out.putInt(nameOff);
        out.putInt(indices->indices.at(instance)); // section index
        out.putInt(instance->isXLU() ? 0x80000000 : 0x0); // flags
        out.put(1); // texgens
        out.put(1); // light channels
        out.put(1); // shader stages
        out.put(0); // indirect textures
        out.putInt(static_cast<uint32_t>(instance->getCullMode()));
        out.put(0); // alpha function
        out.put(0); // lightset
        out.put(0); // fogset
        out.put(0); // unknown/unused
        out.putInt(0); // indirect methods
        out.putInt(0xFFFFFFFF); // light normal map refs
        out.putInt(getMDL0ShaderSectionOffset(instance, offsets)); // shader offset
        out.putInt(instance->getLayerCount());
        out.putInt(instance->getLayerCount() > 0 ? 0x418 : 0x0); // layer offset
        out.putInt(0); // fur data offset
        out.putInt(0); // unused; old version
        out.putInt(displayListOff);

        for (uint32_t i = 0; i < 0x168; ++i) // unused
        {
            out.put(0); // precompiled texture and palette information
        }

        out.putInt(0xF); // layer flags
        out.putInt(0); // texture matrix mode

        for (uint32_t i = 0; i < MAX_LAYER_COUNT; ++i) // layer transformations
        {
            out.putFloat(1.f).putFloat(1.f); // scale
            out.putFloat(0.f); // rotation
            out.putFloat(0.f).putFloat(0.f); // translation
        }

        for (uint32_t i = 0; i < MAX_LAYER_COUNT; ++i) // layer texture matrices
        {
            out.put(0xFF); // SCN0 camera ref
            out.put(0xFF); // SCN0 light ref
            out.put(0); // map mode
            out.put(1); // enable identity matrix effect
            out.putFloat(1.f).putFloat(0.f).putFloat(0.f).putFloat(0.f) // 4x3 texture matrix
                .putFloat(0.f).putFloat(1.f).putFloat(0.f).putFloat(0.f)
                .putFloat(0.f).putFloat(0.f).putFloat(1.f).putFloat(0.f);
        }

        for (uint32_t i = 0; i < 2; ++i) // lighting channels
        {
            out.putInt(0x3F); // flags (only last 6 bits are used)
            out.putInt(0xFFFFFFFF); // material colour
            out.putInt(0xFFFFFFFF); // ambient colour
            out.putInt(0);
            out.putInt(0);
        }

        ////// Layers //////////////////

        for (MDL0::Material::Layer* layer : instance->getLayers())
        {
            uint32_t layerPos = offsets->layerOffs.at(layer);
            uint32_t texLinkNameOff =
                (table->offsets.at(layer->getTextureLink()->getName()) + tableOff) - layerPos;

            out.position(layerPos);
            out.putInt(texLinkNameOff); // name of texture link
            out.putInt(0); // name of palette link
            out.putInt(0); // offset to texture data; calculated at runtime
            out.putInt(0); // offset to palette data; calculated at runtime
            out.putInt(0); // texture ID
            out.putInt(0); // palette ID
            out.putInt(static_cast<uint32_t>(layer->getTextureWrapMode())); // texture wrap S
            out.putInt(static_cast<uint32_t>(layer->getTextureWrapMode())); // texture wrap T
            out.putInt(static_cast<uint32_t>(layer->getMinFilter())); // min filter
            out.putInt(static_cast<uint32_t>(layer->getMagFilter())); // mag filter
            out.putInt(0); // lod bias
            out.putInt(0); // max anisotropy filtering
            out.put(0); // clamp bias
            out.put(0); // texel interpolate
            out.putShort(0); // unknown/unused
        }

        ////// Graphics Code ///////////

        // Mode information (0x20)
        out.position(pos + displayListOff);

        out.put(0x61).put(0xF3).put(0x3F).put(0x00).put(0x00); // unknown BP instruction
        out.put(0x61).put(0x40).put(0x00).put(0x00).put(0x17); // unknown BP instruction

        out.put(0x61).put(0xFE).put(0x00).put(0xFF).put(0xE3); // set BP mask
        out.put(0x61).put(0x41).put(0x00).put(0x34).put(0xA0); // BP alpha blending settings

        out.put(0x61).put(0x42).put(0x00).put(0x00).put(0x00); // unknown BP instruction

        // Shader registers (0x80)
        out.position(pos + displayListOff + 0x20);

        out.put(0x61).put(0xE2).put(0x00).put(0x00).put(0x00); // shader color0 alpha/red
        out.put(0x61).put(0xE3).put(0x00).put(0x00).put(0x00); // shader color0 green/blue

        out.put(0x61).put(0xE4).put(0x00).put(0x00).put(0x00); // shader color1 alpha/red
        out.put(0x61).put(0xE5).put(0x00).put(0x00).put(0x00); // shader color1 green/blue

        out.put(0x61).put(0xE6).put(0x00).put(0x00).put(0x00); // shader color2 alpha/red
        out.put(0x61).put(0xE7).put(0x00).put(0x00).put(0x00); // shader color3 green/blue

        out.put(0x61).put(0xE0).put(0x80).put(0x00).put(0x00); // shader constant color0 alpha/red
        out.put(0x61).put(0xE1).put(0x80).put(0x00).put(0x00); // shader constant color0 green/blue

        out.put(0x61).put(0xE2).put(0x80).put(0x00).put(0x00); // shader constant color1 alpha/red
        out.put(0x61).put(0xE3).put(0x80).put(0x00).put(0x00); // shader constant color1 green/blue

        out.put(0x61).put(0xE4).put(0x80).put(0x00).put(0x00); // shader constant color2 alpha/red
        out.put(0x61).put(0xE5).put(0x80).put(0x00).put(0x00); // shader constant color2 green/blue

        out.put(0x61).put(0xE6).put(0x80).put(0x00).put(0x00); // shader constant color3 alpha/red
        out.put(0x61).put(0xE7).put(0x80).put(0x00).put(0x00); // shader constant color3 green/blue

        // Texture tranformations (0x40)
        out.position(pos + displayListOff + 0xA0);

        out.put(0x61).put(0x25).put(0x00).put(0x00).put(0x00); // unknown BP instruction
        out.put(0x61).put(0x26).put(0x00).put(0x00).put(0x00); // unknown BP instruction

        // Texture matrices (0xA0)
        out.position(pos + displayListOff + 0xE0);

        out.put(0x10).putShort(0).putShort(0x1040).putInt(0x00005280); // XF texture matrix0
        out.put(0x10).putShort(0).putShort(0x1050).putInt(0x00000000); // XF posteffect matrix0
    }
}

void writeMDL0ShaderSections(
    Buffer& out, MDL0* mdl0, MDL0Offsets* offsets, MDL0SectionIndices* indices
)
{
    for (MDL0::Shader* instance : mdl0->getAll<MDL0::Shader>())
    {
        uint32_t pos = offsets->sectionOffs.at(instance);
        int32_t offToMDL0 = -static_cast<int32_t>(pos);

        out.position(pos);
        out.putInt(calculateMDL0SectionSize<MDL0::Shader>(instance));
        out.putInt(static_cast<uint32_t>(offToMDL0));
        out.putInt(indices->indices.at(instance)); // section index
        out.put(0x1); // layer enable flags
        out.put(0); // unused in MKW
        out.put(0); // unused in MKW
        out.put(0); // unused in MKW
        out.put(0); // layer index [TexRef0]
        out.put(0xFF); // layer index [TexRef1]
        out.put(0xFF); // layer index [TexRef2]
        out.put(0xFF); // layer index [TexRef3]
        out.put(0xFF); // layer index [TexRef4]
        out.put(0xFF); // layer index [TexRef5]
        out.put(0xFF); // layer index [TexRef6]
        out.put(0xFF); // layer index [TexRef7]
        out.putInt(0); // unknown/unused
        out.putInt(0); // unknown/unused

        ////// Graphics Code ///////////

        // Base setup (0x60)
        out.put(0x61).put(0xFE).put(0x00).put(0x00).put(0x0F); // set BP mask
        out.put(0x61).put(0xF6).put(0x00).put(0x00).put(0x04); // BP swap mode

        out.put(0x61).put(0xFE).put(0x00).put(0x00).put(0x0F); // set BP mask
        out.put(0x61).put(0xF7).put(0x00).put(0x00).put(0x0E); // BP swap mode

        out.put(0x61).put(0xFE).put(0x00).put(0x00).put(0x0F); // set BP mask
        out.put(0x61).put(0xF8).put(0x00).put(0x00).put(0x00); // BP swap mode

        out.put(0x61).put(0xFE).put(0x00).put(0x00).put(0x0F); // set BP mask
        out.put(0x61).put(0xF9).put(0x00).put(0x00).put(0x0C); // BP swap mode

        out.put(0x61).put(0xFE).put(0x00).put(0x00).put(0x0F); // set BP mask
        out.put(0x61).put(0xFA).put(0x00).put(0x00).put(0x05); // BP swap mode

        out.put(0x61).put(0xFE).put(0x00).put(0x00).put(0x0F); // set BP mask
        out.put(0x61).put(0xFB).put(0x00).put(0x00).put(0x0D); // BP swap mode

        out.put(0x61).put(0xFE).put(0x00).put(0x00).put(0x0F); // set BP mask
        out.put(0x61).put(0xFC).put(0x00).put(0x00).put(0x0A); // BP swap mode

        out.put(0x61).put(0xFE).put(0x00).put(0x00).put(0x0F); // set BP mask
        out.put(0x61).put(0xFD).put(0x00).put(0x00).put(0x0E); // BP swap mode

        out.put(0x61).put(0x27).put(0xFF).put(0xFF).put(0xFF); // BP indirect texture sources

        // Stages
        out.position(pos + 0x80);

        out.put(0x61).put(0xFE).put(0xFF).put(0xFF).put(0xF0); // set BP mask
        out.put(0x61).put(0xF6).put(0x00).put(0x00).put(0x00); // BP swap mode

        out.put(0x61).put(0x28).put(0x3B).put(0xF0).put(0x40); // BP texture reading flags

        out.put(0x61).put(0xC0).put(0x18).put(0xFF).put(0xF8); // BP colour shader operation
        out.put(0x61).put(0xC1).put(0x08).put(0xF2).put(0xF0); // BP alpha shader operation

        out.put(0x61).put(0x10).put(0x00).put(0x00).put(0x00); // unknown BP instruction
    }
}

uint32_t calcMDL0ObjectCPPresenceFlag(MDL0::Object* instance)
{
    uint32_t flagPresenceCP = 0x0;
    if (instance->getVertexArray() != nullptr)
    {
        uint8_t mode = instance->getVertexArrayIndexSize() + 1;
        flagPresenceCP |= mode << 0x9;
    }
    if (instance->getNormalArray() != nullptr)
    {
        uint8_t mode = instance->getNormalArrayIndexSize() + 1;
        flagPresenceCP |= mode << 0xB;
    }
    for (uint32_t i = 0; i < MDL0::Object::COLOUR_ARRAY_COUNT; ++i)
    {
        if (instance->getColourArray(i) != nullptr)
        {
            uint8_t mode = instance->getColourArrayIndexSize(i) + 1;
            flagPresenceCP |= mode << ((i * 2) + 0xD);
        }
    }
    return flagPresenceCP;
}

uint32_t calcMDL0ObjectCPTexPresenceFlag(MDL0::Object* instance)
{
    uint32_t flagTexPresenceCP = 0x0;
    for (uint32_t i = 0; i < MDL0::Object::TEX_COORD_ARRAY_COUNT; ++i)
    {
        if (instance->getTexCoordArray(i) != nullptr)
        {
            uint8_t mode = instance->getTexCoordArrayIndexSize(i) + 1;
            flagTexPresenceCP |= mode << (i * 2);
        }
    }
    return flagTexPresenceCP;
}

uint32_t calcMDL0ObjectXFParamCount(MDL0::Object* instance)
{
    uint32_t paramCountXF = 0x0;

    uint32_t colourCount = 0;
    for (uint32_t i = 0; i < MDL0::Object::COLOUR_ARRAY_COUNT; ++i)
    {
        if (instance->getColourArray(i) != nullptr)
        {
            ++colourCount;
        }
    }
    paramCountXF |= colourCount;

    if (instance->getNormalArray() != nullptr)
    {
        paramCountXF |= 1 << 2;
    }

    uint32_t texCoordCount = 0;
    for (uint32_t i = 0; i < MDL0::Object::TEX_COORD_ARRAY_COUNT; ++i)
    {
        if (instance->getTexCoordArray(i) != nullptr)
        {
            ++texCoordCount;
        }
    }
    paramCountXF |= texCoordCount << 4;

    return paramCountXF;
}

uint32_t calcVertexFormat0CP(MDL0::Object* instance)
{
    uint32_t vertexFormat0CP = 0x41201009;

    for (uint32_t i = 0; i < MDL0::Object::COLOUR_ARRAY_COUNT; ++i)
    {
        if (instance->getColourArray(i) != nullptr)
        {
            vertexFormat0CP |=
                static_cast<uint32_t>(instance->getColourArray(i)->getFormat())
                    << (0xE + (i * 4));
        }
        else
        {
            vertexFormat0CP |= 0xB << (0xD + (i * 4));
        }
    }

    return vertexFormat0CP;
}

uint32_t calcVertexFormat1CP(MDL0::Object* instance)
{
    // no calculations are needed at the moment as tex coords must be formatted as floats
    uint32_t vertexFormat1CP = 0xC8241209;
    
    return vertexFormat1CP;
}

uint32_t calcVertexFormat2CP(MDL0::Object* instance)
{
    uint32_t vertexFormat2CP = 0x04824120;

    return vertexFormat2CP;
}

uint16_t indexOfMDL0Section(MDL0SectionIndices* indices, MDL0::Section* instance)
{
    return instance == nullptr ? ~0 : indices->indices.at(instance);
}

void writeMDL0ObjectSections(
    Buffer& out, MDL0* mdl0, MDL0Offsets* offsets, MDL0SectionIndices* indices,
    BRRESStringTable* table, uint32_t tableOff
)
{
    for (MDL0::Object* instance : mdl0->getAll<MDL0::Object>())
    {
        uint32_t pos = offsets->sectionOffs.at(instance);
        int32_t offToMDL0 = -static_cast<int32_t>(pos);
        uint32_t nameOff = (table->offsets.at(instance->getName()) + tableOff) - pos;

        uint32_t dataSize = padNumber(instance->getGeometryDataSize(), 0x10);

        uint32_t flagPresenceCP = calcMDL0ObjectCPPresenceFlag(instance);
        uint32_t flagTexPresenceCP = calcMDL0ObjectCPTexPresenceFlag(instance);
        uint32_t paramCountXF = calcMDL0ObjectXFParamCount(instance);

        out.position(pos);
        out.putInt(calculateMDL0SectionSize<MDL0::Object>(instance));
        out.putInt(static_cast<uint32_t>(offToMDL0));
        out.putInt(instance->getBone() == nullptr ? ~0 : indices->indices.at(instance->getBone()));
        out.putInt(flagPresenceCP); // CP vertex presence and mode
        out.putInt(flagTexPresenceCP); // CP tex coord presence and mode
        out.putInt(paramCountXF); // XF param count
        out.putInt(0xE0); // vertex declaration size
        out.putInt(0x80); // unknown flags; maybe related to vertex declaration?
        out.putInt(0x68); // vertex declaration offset
        out.putInt(dataSize); // vertex data size
        out.putInt(dataSize); // vertex data size; duplicate/unused
        out.putInt(0x13C); // vertex data offset
        out.putInt(0x00002A00); // XF texture matrix
        out.putInt(0); // unknown/unused
        out.putInt(nameOff);
        out.putInt(indices->indices.at(instance)); // section index
        out.putInt(instance->getVertexCount()); // vertex count
        out.putInt(instance->getFaceCount()); // face count
        out.putShort(indexOfMDL0Section(indices, instance->getVertexArray()));
        out.putShort(indexOfMDL0Section(indices, instance->getNormalArray()));
        out.putShort(indexOfMDL0Section(indices, instance->getColourArray(0)));
        out.putShort(indexOfMDL0Section(indices, instance->getColourArray(1)));
        out.putShort(indexOfMDL0Section(indices, instance->getTexCoordArray(0)));
        out.putShort(indexOfMDL0Section(indices, instance->getTexCoordArray(1)));
        out.putShort(indexOfMDL0Section(indices, instance->getTexCoordArray(2)));
        out.putShort(indexOfMDL0Section(indices, instance->getTexCoordArray(3)));
        out.putShort(indexOfMDL0Section(indices, instance->getTexCoordArray(4)));
        out.putShort(indexOfMDL0Section(indices, instance->getTexCoordArray(5)));
        out.putShort(indexOfMDL0Section(indices, instance->getTexCoordArray(6)));
        out.putShort(indexOfMDL0Section(indices, instance->getTexCoordArray(7)));
        out.putInt(0xFFFFFFFF); // unknown/unused
        out.putInt(0x68); // bone table offset

        uint32_t vertexFormat0CP = calcVertexFormat0CP(instance);
        uint32_t vertexFormat1CP = calcVertexFormat1CP(instance);
        uint32_t vertexFormat2CP = calcVertexFormat2CP(instance);

        // vertex declaration graphics code
        out.position(pos + 0x88);
        out.put(0).put(0); // padding
        out.put(0x08).put(0x50).putInt(flagPresenceCP); // CP vertex presence and mode
        out.put(0x08).put(0x60).putInt(flagTexPresenceCP); // CP tex presence and mode
        out.put(0x10).putShort(0).putShort(0x1008).putInt(paramCountXF); // XF param count
        out.put(0); // padding
        out.put(0x08).put(0x70).putInt(vertexFormat0CP); // CP vertex format (part 1)
        out.put(0x08).put(0x80).putInt(vertexFormat1CP); // CP vertex format (part 2)
        out.put(0x08).put(0x90).putInt(vertexFormat2CP); // CP vertex format (part 3)

        // vertex data graphics code
        out.position(pos + 0x160);
        out.put(instance->getGeometryData());
    }
}

void writeMDL0TextureLinkSections(Buffer& out, MDL0* mdl0, MDL0Offsets* offsets)
{
    for (MDL0::TextureLink* instance : mdl0->getAll<MDL0::TextureLink>())
    {
        uint32_t pos = offsets->sectionOffs.at(instance);

        out.position(pos);
        out.putInt(instance->getCount());
        for (MDL0::Material::Layer* layer : instance->getReferences())
        {
            out.putInt(offsets->sectionOffs.at(layer->getMaterial()) - pos);
            out.putInt(offsets->layerOffs.at(layer) - pos);
        }
    }
}

void writeMDL0Data(
    Buffer& out, MDL0* mdl0, MDL0Offsets* offsets, MDL0SectionIndices* indices,
    BRRESStringTable* table, uint32_t tableOff
)
{
    writeMDL0LinksSections(out, mdl0, offsets, indices);
    writeMDL0BoneSections(out, mdl0, offsets, indices, table, tableOff);
    writeMDL0VerticesSections(out, mdl0, offsets, indices, table, tableOff);
    writeMDL0NormalsSections(out, mdl0, offsets, indices, table, tableOff);
    writeMDL0ColoursSections(out, mdl0, offsets, indices, table, tableOff);
    writeMDL0TextureCoordsSections(out, mdl0, offsets, indices, table, tableOff);
    writeMDL0MaterialSections(out, mdl0, offsets, indices, table, tableOff);
    writeMDL0ShaderSections(out, mdl0, offsets, indices);
    writeMDL0ObjectSections(out, mdl0, offsets, indices, table, tableOff);
    writeMDL0TextureLinkSections(out, mdl0, offsets);
}

void writeMDL0(
    Buffer& buffer, MDL0* mdl0, int32_t offToBRRES, BRRESStringTable* table, uint32_t tableOff
)
{
    ////////////////////////////////////
    /// Setup required information

    MDL0Info info;
    createInfo(mdl0, &info, offToBRRES, table, tableOff);

    MDL0GroupsInfo groupsInfo;
    createGroupsInfo(mdl0, &groupsInfo);

    auto groups = std::make_unique<BRRESIndexGroup[]>(groupsInfo.count);
    createIndexGroups(mdl0, groups.get(), &groupsInfo);

    MDL0Offsets offsets;
    createOffsets(mdl0, &offsets, groups.get(), groupsInfo.count);

    MDL0SectionIndices indices;
    createMDL0SectionsIndices(mdl0, &indices);

    ////////////////////////////////////
    /// Resolve offsets

    resolveSectionGroupsOffsets(&offsets, groups.get(), &groupsInfo);

    ////////////////////////////////////
    /// Write MDL0 file

    Buffer out = buffer.slice();

    writeMDL0FileHeader(out, &info, &groupsInfo, groups.get(), &offsets);
    writeMDL0Header(out, mdl0);
    writeMDL0BoneTable(out, mdl0);
    writeMDL0Groups(out, groups.get(), groupsInfo.count, &offsets, table, tableOff);
    writeMDL0Data(out, mdl0, &offsets, &indices, table, tableOff);
}
}
