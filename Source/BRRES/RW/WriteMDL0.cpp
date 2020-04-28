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
    addMDL0SectionsToStringTable<MDL0::ColorArray>(table, mdl0);
    addMDL0SectionsToStringTable<MDL0::TexCoordArray>(table, mdl0);
}

template <class Type>
uint32_t calculateMDL0SectionSize(Type* instance);

template <>
uint32_t calculateMDL0SectionSize<MDL0::Links>(MDL0::Links* instance)
{
    switch (instance->getLinksType())
    {
    case MDL0::Links::Type::NodeTree:
        return padNumber((instance->getCount() * 0x5) + 0x1, 0x10);
    
    default:
        return 0;
    }
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
uint32_t calculateMDL0SectionSize<MDL0::ColorArray>(MDL0::ColorArray* instance)
{
    return 0x20 + padNumber(static_cast<uint32_t>(instance->getData().remaining()), 0x10);
}

template <>
uint32_t calculateMDL0SectionSize<MDL0::TexCoordArray>(MDL0::TexCoordArray* instance)
{
    return 0x30 + padNumber(static_cast<uint32_t>(instance->getData().remaining()), 0x10);
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
    size += BRRESIndexGroup::getSizeInBytesForCount(mdl0->count<MDL0::ColorArray>());
    size += BRRESIndexGroup::getSizeInBytesForCount(mdl0->count<MDL0::TexCoordArray>());

    /// Sections size //////////////////
    size = padNumber(size, 0x10);

    size += calculateMDL0SectionsSize<MDL0::Links>(mdl0);
    size += calculateMDL0SectionsSize<MDL0::Bone>(mdl0);
    size += calculateMDL0SectionsSize<MDL0::VertexArray>(mdl0);
    size += calculateMDL0SectionsSize<MDL0::NormalArray>(mdl0);
    size += calculateMDL0SectionsSize<MDL0::ColorArray>(mdl0);
    size += calculateMDL0SectionsSize<MDL0::TexCoordArray>(mdl0);

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
};

struct MDL0SectionIndices
{

    // indices of all sections
    std::map<MDL0::Section*, uint32_t> indices;
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
    addIfNotEmpty<MDL0::ColorArray>(mdl0, info);
    addIfNotEmpty<MDL0::TexCoordArray>(mdl0, info);
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
    createIndexGroup<MDL0::ColorArray>(mdl0, groups, info);
    createIndexGroup<MDL0::TexCoordArray>(mdl0, groups, info);
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
    createMDL0SectionOffsets<MDL0::ColorArray>(mdl0, offsets, pos);
    createMDL0SectionOffsets<MDL0::TexCoordArray>(mdl0, offsets, pos);
}

void createMDL0BoneSectionIndices(MDL0* mdl0, MDL0SectionIndices* indices)
{
    uint32_t index = 0;
    MDL0::Bone* bone = mdl0->getRootBone();
    while (bone != nullptr)
    {
        indices->indices.insert(std::map<MDL0::Section*, uint32_t>::value_type(bone, index));
        ++index;

        bone = MDL0::Bone::flatNext(bone);
    }
}

void createMDL0SectionsIndices(MDL0* mdl0, MDL0SectionIndices* indices)
{
    createMDL0BoneSectionIndices(mdl0, indices);
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
        out.putShort(static_cast<uint16_t>(indices->indices.at(bone)));
        out.putShort(bone->getParent() == nullptr ? 0 :
            static_cast<uint16_t>(indices->indices.at(bone->getParent()))
        );

        bone = MDL0::Bone::flatNext(bone);
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
    Buffer& out, MDL0* mdl0, MDL0Offsets* offsets, BRRESStringTable* table, uint32_t tableOff
)
{
    uint32_t idx = 0;
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
        out.putInt(idx++); // section index
        out.putInt(static_cast<uint32_t>(instance->getComponentsType()));
        out.putInt(0x4); // floats
        out.put(0); // divisor; unused for floats
        out.put(0); // stride
        out.putShort(instance->getCount()); // vertex count
        boxMin.put(out);
        boxMax.put(out);

        out.position(pos + 0x40);
        out.put(instance->getData());
    }
}

void writeMDL0NormalsSections(
    Buffer& out, MDL0* mdl0, MDL0Offsets* offsets, BRRESStringTable* table, uint32_t tableOff
)
{
    uint32_t idx = 0;
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
        out.putInt(idx++); // section index
        out.putInt(static_cast<uint32_t>(instance->getComponentsType()));
        out.putInt(0x4); // floats
        out.put(0); // divisor; unused for floats
        out.put(0); // stride
        out.putShort(instance->getCount()); // normal count

        out.position(pos + 0x20);
        out.put(instance->getData());
    }
}

void writeMDL0ColorsSections(
    Buffer& out, MDL0* mdl0, MDL0Offsets* offsets, BRRESStringTable* table, uint32_t tableOff
)
{
    uint32_t idx = 0;
    for (MDL0::ColorArray* instance : mdl0->getAll<MDL0::ColorArray>())
    {
        uint32_t pos = offsets->sectionOffs.at(instance);
        int32_t offToMDL0 = -static_cast<int32_t>(pos);
        uint32_t nameOff = (table->offsets.at(instance->getName()) + tableOff) - pos;

        uint32_t compType = MDL0::ColorArray::componentCount(instance->getFormat()) == 3
            ? 0x0 : 0x1;

        out.position(pos);
        out.putInt(calculateMDL0SectionSize<MDL0::ColorArray>(instance));
        out.putInt(static_cast<uint32_t>(offToMDL0));
        out.putInt(0x20); // data offset
        out.putInt(nameOff);
        out.putInt(idx++); // section index
        out.putInt(compType);
        out.putInt(static_cast<uint32_t>(instance->getFormat()));
        out.put(0); // stride
        out.put(0); // unknown/unused
        out.putShort(instance->getCount()); // color count

        out.position(pos + 0x20);
        out.put(instance->getData());
    }
}

void writeMDL0TextureCoordsSections(
    Buffer& out, MDL0* mdl0, MDL0Offsets* offsets, BRRESStringTable* table, uint32_t tableOff
)
{
    uint32_t idx = 0;
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
        out.putInt(idx++); // section index
        out.putInt(static_cast<uint32_t>(instance->getComponentsType()));
        out.putInt(0x4); // floats
        out.put(0); // divisor; unused for floats
        out.put(0); // stride
        out.putShort(instance->getCount()); // texture coord count
        boxMin.put(out);
        boxMax.put(out);

        out.position(pos + 0x30);
        out.put(instance->getData());
    }
}

void writeMDL0Data(
    Buffer& out, MDL0* mdl0, MDL0Offsets* offsets, MDL0SectionIndices* indices,
    BRRESStringTable* table, uint32_t tableOff
)
{
    writeMDL0LinksSections(out, mdl0, offsets, indices);
    writeMDL0BoneSections(out, mdl0, offsets, indices, table, tableOff);
    writeMDL0VerticesSections(out, mdl0, offsets, table, tableOff);
    writeMDL0NormalsSections(out, mdl0, offsets, table, tableOff);
    writeMDL0ColorsSections(out, mdl0, offsets, table, tableOff);
    writeMDL0TextureCoordsSections(out, mdl0, offsets, table, tableOff);
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
