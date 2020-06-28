//////////////////////////////////////////////////
//  Copyright (c) 2020 Nara Hiero
//
// This file is licensed under GPLv3+
// Refer to the `License.txt` file included.
//////////////////////////////////////////////////

#include "BRRES/RW/BRRESRWCommon.hpp"

#include <CTLib/Utilities.hpp>

namespace CTLib
{

const MDL0::SectionType SECTION_TYPES[] = {
    MDL0::SectionType::Links, MDL0::SectionType::Bone, MDL0::SectionType::VertexArray,
    MDL0::SectionType::NormalArray, MDL0::SectionType::ColourArray, MDL0::SectionType::TexCoordArray,
    MDL0::SectionType::NONE, MDL0::SectionType::NONE, MDL0::SectionType::Material,
    MDL0::SectionType::Shader, MDL0::SectionType::Object, MDL0::SectionType::TextureLink,
    MDL0::SectionType::NONE, MDL0::SectionType::NONE
};

struct MDL0Header
{

    // size of file in bytes
    uint32_t size;

    // offsets to sections
    std::map<MDL0::SectionType, uint32_t> sectionOffs;

    // offset to bone table (add 0x4C for absolute offset)
    uint32_t boneTableOff;
};

void readMDL0Header(Buffer& data, MDL0* mdl0, MDL0Header* header)
{
    const uint32_t dataSize = static_cast<uint32_t>(data.remaining());
    if (dataSize < 0x10)
    {
        throw BRRESError("MDL0: Invalid header! Not enough bytes in buffer!");
    }

    if (!Bytes::matchesString("MDL0", *data + 0x00, 0x4))
    {
        throw BRRESError(Strings::format(
            "MDL0: Invalid header! Invalid magic! (Expected 'MDL0', Got '%s')",
            Strings::stringify(*data + 0x00, 0x4).c_str()
        ));
    }
    data.getInt(); // skip magic

    header->size = data.getInt();
    if (dataSize < header->size)
    {
        throw BRRESError(Strings::format(
            "MDL0: Invalid data! Not enough bytes in buffer! (%d < %d)",
            dataSize < header->size
        ));
    }

    uint32_t version = data.getInt();
    if (version != 11)
    {
        throw BRRESError(Strings::format(
            "MDL0: Unsupported version (%d)! (Only version 11 is supported)",
            version
        ));
    }

    data.getInt(); // ignore offset to BRRES

    if (dataSize < 0x4C)
    {
        throw BRRESError("MDL0: Invalid header! Not enough bytes in buffer!");
    }

    for (MDL0::SectionType type : SECTION_TYPES)
    {
        if (type == MDL0::SectionType::NONE)
        {
            data.getInt();
        }
        else
        {
            uint32_t off = data.getInt();
            if (off != 0)
            {
                if (off > header->size)
                {
                    throw BRRESError(Strings::format(
                        "MDL0: Section offset (%s) out of range! (%d > %d)",
                        MDL0::Section::nameForType(type), off, header->size
                    ));
                }

                header->sectionOffs.insert(
                    std::map<MDL0::SectionType, uint32_t>::value_type(type, off)
                );
            }
        }
    }

    std::string name = readBRRESString(data, data.getInt());
    if (name != mdl0->getName())
    {
        throw BRRESError(Strings::format(
            "MDL0: Section name does not match index group entry name! (%s != %s)",
            name.c_str(), mdl0->getName().c_str()
        ));
    }
}

void readMDL0ModelInfo(Buffer& data, MDL0* mdl0, MDL0Header* header)
{
    const uint32_t dataSize = static_cast<uint32_t>(header->size - data.position());
    if (dataSize < 0x40)
    {
        throw BRRESError(Strings::format(
            "MDL0: Invalid model info header! Not enough bytes in buffer! (%d < 64)",
            dataSize
        ));
    }

    uint32_t size = data.getInt();
    if (size != 0x40)
    {
        throw BRRESError(Strings::format(
            "MDL0: Invalid model info header! Invalid block size! (%d != 64)",
            size
        ));
    }

    data.getInt(); // ignore offset to header

    data.getInt(); // unknown/unused
    data.getInt(); // unknown/unused

    data.getInt(); // ignore vertex count
    data.getInt(); // ignore face count

    data.getInt(); // unknown/unused

    data.getInt(); // ignore bone count
    data.getInt(); // ignore unknown value

    header->boneTableOff = data.getInt();
    if (header->boneTableOff > dataSize)
    {
        throw BRRESError(Strings::format(
            "MDL0: Bone table offset out of range! (%d > %d)",
            header->boneTableOff, dataSize
        ));
    }

    Vector3f boxMin, boxMax;
    boxMin.get(data);
    boxMax.get(data);

    mdl0->setBoxMin(boxMin);
    mdl0->setBoxMax(boxMax);
}

template <class Type>
void readMDL0Section(Buffer& data, Type* instance, uint32_t size)
{} // temporarily define method to prevent linker errors

template <>
void readMDL0Section<MDL0::VertexArray>(Buffer& data, MDL0::VertexArray* va, uint32_t size)
{
    if (size < 0x38)
    {
        throw BRRESError(Strings::format(
            "MDL0: Not enough bytes in VertexArray section (%s) header! (%d < 56)",
            va->getName().c_str(), size
        ));
    }

    uint32_t len = data.getInt();
    if (len > size)
    {
        throw BRRESError(Strings::format(
            "MDL0: VertexArray section (%s) overflows out of MDL0! (%d > %d)",
            va->getName().c_str(), len, size
        ));
    }

    data.getInt(); // ignore offset to MDL0

    uint32_t dataOff = data.getInt();
    if (dataOff > len)
    {
        throw BRRESError(Strings::format(
            "MDL0: VertexArray section (%d) data out of range! (%d > %d)",
            dataOff, len
        ));
    }

    std::string name = readBRRESString(data, data.getInt());
    if (name != va->getName())
    {
        throw BRRESError(Strings::format(
            "MDL0: VertexArray section name does not match index group entry name! (%s != %s)",
            va->getName().c_str(), name.c_str()
        ));
    }

    data.getInt(); // ignore section index

    uint32_t compsE = data.getInt();
    if (compsE > 0x1)
    {
        throw BRRESError(Strings::format(
            "MDL0: Invalid VertexArray (%s) component count enum! (%d)",
            va->getName().c_str(), compsE
        ));
    }

    uint32_t formatE = data.getInt();
    if (formatE > 0x4)
    {
        throw BRRESError(Strings::format(
            "MDL0: Invalid VertexArray (%s) component format enum! (%d)",
            va->getName().c_str(), formatE
        ));
    }

    va->setDivisor(data.get());

    MDL0::VertexArray::Components comps = static_cast<MDL0::VertexArray::Components>(compsE);
    MDL0::VertexArray::Format format = static_cast<MDL0::VertexArray::Format>(formatE);
    uint8_t byteCount = MDL0::VertexArray::componentCount(comps) * MDL0::VertexArray::byteCount(format);

    uint8_t stride = data.get();
    if (stride != byteCount)
    {
        throw BRRESError(Strings::format(
            "MDL0: VertexArray (%s) stride does not match element size! (%d != %d)",
            va->getName().c_str(), stride, byteCount
        ));
    }

    uint16_t count = data.getShort();
    if (count == 0)
    {
        throw BRRESError(Strings::format(
            "MDL0: VertexArray (%s) vertex count is 0!",
            va->getName().c_str()
        ));
    }

    Vector3f boxMin, boxMax;
    boxMin.get(data);
    boxMax.get(data);

    uint32_t dataSize = count * byteCount;
    if (dataSize + dataOff > len)
    {
        throw BRRESError(Strings::format(
            "MDL0: VertexArray (%s) data overflows out of section! (%d > %d)",
            dataSize + dataOff, len
        ));
    }

    Buffer vertexData = data.position(dataOff).slice();
    vertexData.limit(dataSize);

    va->setData(vertexData, comps, format);

    va->setBoxMin(boxMin);
    va->setBoxMax(boxMax);
}

template <>
void readMDL0Section<MDL0::NormalArray>(Buffer& data, MDL0::NormalArray* na, uint32_t size)
{
    if (size < 0x20)
    {
        throw BRRESError(Strings::format(
            "MDL0: Not enough bytes in NormalArray section (%s) header! (%d < 32)",
            na->getName().c_str(), size
        ));
    }

    uint32_t len = data.getInt();
    if (len > size)
    {
        throw BRRESError(Strings::format(
            "MDL0: NormalArray section (%s) overflows out of MDL0! (%d > %d)",
            len, size
        ));
    }

    data.getInt(); // ignore offset to MDL0

    uint32_t dataOff = data.getInt();
    if (dataOff > len)
    {
        throw BRRESError(Strings::format(
            "MDL0: NormalArray section (%s) data offset out of range! (%d > %d)",
            dataOff, len
        ));
    }

    std::string name = readBRRESString(data, data.getInt());
    if (name != na->getName())
    {
        throw BRRESError(Strings::format(
            "MDL0: NormalArray section name does not match index group entry name! (%s != %s)",
            na->getName().c_str(), name.c_str()
        ));
    }

    data.getInt(); // ignore section index

    uint32_t compsE = data.getInt();
    if (compsE > 0x2)
    {
        throw BRRESError(Strings::format(
            "MDL0: Invalid NormalArray (%s) components type enum! (%d)",
            na->getName().c_str(), compsE
        ));
    }

    uint32_t formatE = data.getInt();
    if (formatE > 0x4)
    {
        throw BRRESError(Strings::format(
            "MDL0: Invalid NormalArray (%s) format enum! (%d)",
            na->getName().c_str(), formatE
        ));
    }

    na->setDivisor(data.get());

    MDL0::NormalArray::Components comps = static_cast<MDL0::NormalArray::Components>(compsE);
    MDL0::NormalArray::Format format = static_cast<MDL0::NormalArray::Format>(formatE);
    uint8_t byteCount = MDL0::NormalArray::componentCount(comps) * MDL0::NormalArray::byteCount(format);

    uint8_t stride = data.get();
    if (stride != byteCount)
    {
        throw BRRESError(Strings::format(
            "MDL0: NormalArray (%s) stride does not match element size! (%d != %d)",
            na->getName().c_str(), stride, byteCount
        ));
    }

    uint16_t count = data.getShort();
    if (count == 0)
    {
        throw BRRESError(Strings::format(
            "MDL0: NormalArray (%s) normal count is 0!",
            na->getName().c_str()
        ));
    }

    uint32_t dataSize = count * byteCount;
    if (dataSize + dataOff > len)
    {
        throw BRRESError(Strings::format(
            "MDL0: NormalArray (%s) data overflows out of section! (%d > %d)",
            na->getName().c_str(), dataSize + dataOff, len
        ));
    }

    Buffer normalData = data.position(dataOff).slice();
    normalData.limit(dataSize);

    na->setData(normalData, comps, format);
}

template <>
void readMDL0Section<MDL0::ColourArray>(Buffer& data, MDL0::ColourArray* ca, uint32_t size)
{
    if (size < 0x20)
    {
        throw BRRESError(Strings::format(
            "MDL0: Not enough bytes in ColourArray section (%s) header! (%d < 32)",
            ca->getName().c_str(), size
        ));
    }

    uint32_t len = data.getInt();
    if (len > size)
    {
        throw BRRESError(Strings::format(
            "MDL0: ColourArray section (%s) overflows out of MDL0! (%d > %d)",
            ca->getName().c_str(), len, size
        ));
    }

    data.getInt(); // ignore offset to MDL0

    uint32_t dataOff = data.getInt();
    if (dataOff > len)
    {
        throw BRRESError(Strings::format(
            "MDL0: ColourArray section (%s) data offset out of range! (%d > %d)",
            ca->getName().c_str(), dataOff, len
        ));
    }

    std::string name = readBRRESString(data, data.getInt());
    if (name != ca->getName())
    {
        throw BRRESError(Strings::format(
            "MDL0: ColourArray section name does not match index group entry name! (%s != %s)",
            ca->getName().c_str(), name.c_str()
        ));
    }

    data.getInt(); // ignore section index

    uint32_t compCount = data.getInt();
    if (compCount > 0x1)
    {
        throw BRRESError(Strings::format(
            "MDL0: Invalid ColourArray (%s) component count enum! (%d)",
            ca->getName().c_str(), compCount
        ));
    }

    uint32_t formatE = data.getInt();
    if (formatE > 0x5)
    {
        throw BRRESError(Strings::format(
            "MDL0: Invalid ColourArray (%s) format enum! (%d)",
            ca->getName().c_str(), formatE
        ));
    }
    
    if ((compCount == 0x0 && formatE > 0x2) || (compCount == 0x1 && formatE < 0x3))
    {
        throw BRRESError(Strings::format(
            "MDL0: ColourArray (%s) component count enum (%d) does not match format enum (%d)!",
            ca->getName().c_str(), compCount, formatE
        ));
    }

    MDL0::ColourArray::Format format = static_cast<MDL0::ColourArray::Format>(formatE);
    uint8_t byteCount = MDL0::ColourArray::byteCount(format);

    uint8_t stride = data.get();
    if (stride != byteCount)
    {
        throw BRRESError(Strings::format(
            "MDL0: ColourArray (%s) stride does not match format size! (%d != %d)",
            ca->getName().c_str(), stride, byteCount
        ));
    }

    data.get(); // unknown/unused

    uint16_t count = data.getShort();
    if (count == 0)
    {
        throw BRRESError(Strings::format(
            "MDL0: ColourArray (%s) colour count is 0!",
            ca->getName().c_str()
        ));
    }

    uint32_t dataSize = count * byteCount;
    if (dataSize + dataOff > len)
    {
        throw BRRESError(Strings::format(
            "MDL0: ColourArray (%s) data overflows out of section! (%d > %d)",
            dataSize + dataOff, len
        ));
    }

    Buffer colourData = data.position(dataOff).slice();
    colourData.limit(dataSize);

    ca->setData(colourData, format);
}

template <>
void readMDL0Section<MDL0::TexCoordArray>(Buffer& data, MDL0::TexCoordArray* tca, uint32_t size)
{
    if (size < 0x30)
    {
        throw BRRESError(Strings::format(
            "MDL0: Not enough bytes in TextureCoordArray section (%s) header! (%d < 48)",
            tca->getName().c_str(), size
        ));
    }

    uint32_t len = data.getInt();
    if (len > size)
    {
        throw BRRESError(Strings::format(
            "MDL0: TexCoordArray section (%s) overflows out of MDL0! (%d > %d)",
            tca->getName().c_str(), len, size
        ));
    }

    data.getInt(); // ignore offset to MDL0

    uint32_t dataOff = data.getInt();
    if (dataOff > len)
    {
        throw BRRESError(Strings::format(
            "MDL0: TexCoordArray section (%s) data offset out of range! (%d > %d)",
            tca->getName().c_str(), dataOff, len
        ));
    }

    std::string name = readBRRESString(data, data.getInt());
    if (name != tca->getName())
    {
        throw BRRESError(Strings::format(
            "MDL0: TexCoordArray section name does not match index group entry name! (%s != %s)",
            tca->getName().c_str(), name.c_str()
        ));
    }

    data.getInt(); // ignore section index

    uint32_t compsE = data.getInt();
    if (compsE > 0x2)
    {
        throw BRRESError(Strings::format(
            "MDL0: Invalid TexCoordArray (%s) components type enum! (%d)",
            tca->getName().c_str(), compsE
        ));
    }

    uint32_t formatE = data.getInt();
    if (formatE > 0x4)
    {
        throw BRRESError(Strings::format(
            "MDL0: Invalid TexCoordArray (%s) format enum! (%d)",
            tca->getName().c_str(), formatE
        ));
    }

    tca->setDivisor(data.get());

    MDL0::TexCoordArray::Components comps = static_cast<MDL0::TexCoordArray::Components>(compsE);
    MDL0::TexCoordArray::Format format = static_cast<MDL0::TexCoordArray::Format>(formatE);
    uint8_t byteCount = MDL0::TexCoordArray::componentCount(comps) * MDL0::TexCoordArray::byteCount(format);

    uint8_t stride = data.get();
    if (stride != byteCount)
    {
        throw BRRESError(Strings::format(
            "MDL0: TexCoordArray (%s) stride does not match element size! (%d != %d)",
            tca->getName().c_str(), stride, byteCount
        ));
    }

    uint16_t count = data.getShort();
    if (count == 0)
    {
        throw BRRESError(Strings::format(
            "MDL0: TexCoordArray (%s) texture coord count is 0!",
            tca->getName().c_str()
        ));
    }

    uint32_t dataSize = count * byteCount;
    if (dataSize + dataOff > len)
    {
        throw BRRESError(Strings::format(
            "MDL0: TexCoordArray (%s) data overflows out of section! (%d > %d)",
            tca->getName().c_str(), dataSize + dataOff, len
        ));
    }

    Buffer texCoordData = data.position(dataOff).slice();
    texCoordData.limit(dataSize);

    tca->setData(texCoordData, comps, format);
}

template <class Type>
void readMDL0SectionIndexGroup(Buffer& data, MDL0* mdl0, MDL0Header* header)
{
    if (header->sectionOffs.count(Type::TYPE) == 0)
    {
        return;
    }

    uint32_t off = header->sectionOffs.at(Type::TYPE);
    data.position(off);

    BRRESIndexGroup group;
    group.read(data);

    for (BRRESIndexGroupEntry* entry : group.getEntries())
    {
        if (entry->isRoot())
        {
            continue;
        }

        Buffer entryData = data.position(off + entry->getDataOff()).slice();

        Type* obj = mdl0->add<Type>(entry->getName());
        readMDL0Section<Type>(entryData, obj, header->size - static_cast<uint32_t>(data.position()));
    }
}

void readMDL0Sections(Buffer& data, MDL0* mdl0, MDL0Header* header)
{
    //readMDL0SectionIndexGroup<MDL0::Links>(data, mdl0, header);
    readMDL0SectionIndexGroup<MDL0::Bone>(data, mdl0, header);
    readMDL0SectionIndexGroup<MDL0::VertexArray>(data, mdl0, header);
    readMDL0SectionIndexGroup<MDL0::NormalArray>(data, mdl0, header);
    readMDL0SectionIndexGroup<MDL0::ColourArray>(data, mdl0, header);
    readMDL0SectionIndexGroup<MDL0::TexCoordArray>(data, mdl0, header);
    readMDL0SectionIndexGroup<MDL0::Material>(data, mdl0, header);
    readMDL0SectionIndexGroup<MDL0::Shader>(data, mdl0, header);
    readMDL0SectionIndexGroup<MDL0::Object>(data, mdl0, header);
    //readMDL0SectionIndexGroup<MDL0::TextureLink>(data, mdl0, header);
}

void readMDL0(Buffer& buffer, MDL0* mdl0)
{
    Buffer data = buffer.slice();

    ////// Read header /////////////////
    MDL0Header header;
    readMDL0Header(data, mdl0, &header);
    readMDL0ModelInfo(data, mdl0, &header);

    ////// Read sections ///////////////
    readMDL0Sections(data, mdl0, &header);
}
}
