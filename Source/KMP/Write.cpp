//////////////////////////////////////////////////
//  Copyright (c) 2020 Nara Hiero
//
// This file is licensed under GPLv3+
// Refer to the `License.txt` file included.
//////////////////////////////////////////////////

#include <CTLib/KMP.hpp>

#include <map>

namespace CTLib
{

const KMP::SectionType SECTION_TYPES[] =
{
    KMP::SectionType::KTPT,
    KMP::SectionType::ENPT,
    KMP::SectionType::ENPH,
    KMP::SectionType::ITPT,
    KMP::SectionType::ITPH,
    KMP::SectionType::CKPT,
    KMP::SectionType::CKPH,
    KMP::SectionType::GOBJ,
    KMP::SectionType::POTI,
    KMP::SectionType::AREA,
    KMP::SectionType::CAME,
    KMP::SectionType::JGPT,
    KMP::SectionType::CNPT,
    KMP::SectionType::MSPT,
    KMP::SectionType::STGI
};

// info required in order to write header
struct KMPInfo
{

    // file size in bytes
    uint32_t size;
};

// offsets to KMP sections
struct KMPOffsets
{

    // offsets to all sections
    std::map<KMP::SectionType, uint32_t> sectionOffs;
};

uint32_t getKMPSectionEntrySize(KMP::SectionType section)
{
    switch (section)
    {
    case KMP::SectionType::KTPT:
        return 0x1C;

    case KMP::SectionType::ENPT:
        return 0x14;

    case KMP::SectionType::ENPH:
        return 0x10;

    case KMP::SectionType::ITPT:
        return 0x14;

    case KMP::SectionType::ITPH:
        return 0x10;

    case KMP::SectionType::CKPT:
        return 0x14;

    case KMP::SectionType::CKPH:
        return 0x10;

    case KMP::SectionType::GOBJ:
        return 0x3C;

    case KMP::SectionType::AREA:
        return 0x30;

    case KMP::SectionType::CAME:
        return 0x48;

    case KMP::SectionType::JGPT:
        return 0x1C;

    case KMP::SectionType::CNPT:
        return 0x1C;

    case KMP::SectionType::MSPT:
        return 0x1C;

    case KMP::SectionType::STGI:
        return 0x0C;

    default: // KMP::SectionType::POTI is special case
        return 0xFFFF;
    }
}

template <class Type>
uint32_t calculateKMPSectionSizeAndOffset(const KMP& kmp, uint32_t pos, KMPOffsets* offsets)
{
    uint32_t size = 0;
    if (Type::TYPE == KMP::SectionType::POTI)
    {
        size = 0x8;
        for (KMP::POTI* entry : kmp.getAll<KMP::POTI>())
        {
            size += 0x4 + (entry->getPointCount() * 0x10);
        }
    }
    else
    {
        size = 0x8 + (getKMPSectionEntrySize(Type::TYPE) * kmp.count<Type>());
    }

    offsets->sectionOffs.insert(std::map<KMP::SectionType, uint32_t>::value_type(Type::TYPE, pos));
    return size;
}

void createKMPInfoAndOffsets(const KMP& kmp, KMPInfo* info, KMPOffsets* offsets)
{
    info->size = 0x4C; // header size

    info->size += calculateKMPSectionSizeAndOffset<KMP::KTPT>(kmp, info->size, offsets);
    info->size += calculateKMPSectionSizeAndOffset<KMP::ENPT>(kmp, info->size, offsets);
    info->size += calculateKMPSectionSizeAndOffset<KMP::ENPH>(kmp, info->size, offsets);
    info->size += calculateKMPSectionSizeAndOffset<KMP::ITPT>(kmp, info->size, offsets);
    info->size += calculateKMPSectionSizeAndOffset<KMP::ITPH>(kmp, info->size, offsets);
    info->size += calculateKMPSectionSizeAndOffset<KMP::CKPT>(kmp, info->size, offsets);
    info->size += calculateKMPSectionSizeAndOffset<KMP::CKPH>(kmp, info->size, offsets);
    info->size += calculateKMPSectionSizeAndOffset<KMP::GOBJ>(kmp, info->size, offsets);
    info->size += calculateKMPSectionSizeAndOffset<KMP::POTI>(kmp, info->size, offsets);
    info->size += calculateKMPSectionSizeAndOffset<KMP::AREA>(kmp, info->size, offsets);
    info->size += calculateKMPSectionSizeAndOffset<KMP::CAME>(kmp, info->size, offsets);
    info->size += calculateKMPSectionSizeAndOffset<KMP::JGPT>(kmp, info->size, offsets);
    info->size += calculateKMPSectionSizeAndOffset<KMP::CNPT>(kmp, info->size, offsets);
    info->size += calculateKMPSectionSizeAndOffset<KMP::MSPT>(kmp, info->size, offsets);
    info->size += calculateKMPSectionSizeAndOffset<KMP::STGI>(kmp, info->size, offsets);
}

void writeKMPHeader(Buffer& out, KMPInfo* info, KMPOffsets* offsets)
{
    out.putArray((uint8_t*)"RKMD", 4);
    out.putInt(info->size);
    out.putShort(15); // section count
    out.putShort(0x4C); // header size
    out.putInt(0x9D8); // version number

    // section offsets
    for (KMP::SectionType section : SECTION_TYPES)
    {
        out.putInt(offsets->sectionOffs.at(section) - 0x4C);
    }
}

template <class Type>
void writeKMPGroupSectionEntries(Buffer& out, const KMP& kmp)
{
    for (Type* entry : kmp.getAll<Type>())
    {
        uint8_t first = entry->getFirst() == nullptr ? 0x00
            : static_cast<uint8_t>(kmp.indexOf(entry->getFirst()));

        uint8_t size = entry->getLast() == nullptr ? 0x00
            : static_cast<uint8_t>(kmp.indexOf(entry->getLast())) - first + 1;

        out.put(first);
        out.put(size);

        std::vector<Type*> prevs = entry->getPrevious();
        for (uint8_t i = 0; i < Type::MAX_LINKS; ++i)
        {
            out.put(i >= entry->getPreviousCount() ? 0xFF : kmp.indexOf(prevs.at(i)));
        }

        std::vector<Type*> nexts = entry->getNext();
        for (uint8_t i = 0; i < Type::MAX_LINKS; ++i)
        {
            out.put(i >= entry->getNextCount() ? 0xFF : kmp.indexOf(nexts.at(i)));
        }

        out.putShort(0); // unknown/unused
    }
}

void writeKTPTSection(Buffer& out, const KMP& kmp, KMPOffsets* offsets)
{
    out.position(offsets->sectionOffs.at(KMP::SectionType::KTPT));

    out.putArray((uint8_t*)"KTPT", 4);
    out.putShort(kmp.count<KMP::KTPT>());
    out.putShort(0); // unused

    for (KMP::KTPT* entry : kmp.getAll<KMP::KTPT>())
    {
        entry->getPosition().put(out);
        entry->getRotation().put(out);
        out.putShort(entry->getPlayerIndex());
        out.putShort(0); // unknown/unused
    }
}

void writeENPTSection(Buffer& out, const KMP& kmp, KMPOffsets* offsets)
{
    out.position(offsets->sectionOffs.at(KMP::SectionType::ENPT));

    out.putArray((uint8_t*)"ENPT", 4);
    out.putShort(kmp.count<KMP::ENPT>());
    out.putShort(0); // unused

    for (KMP::ENPT* entry : kmp.getAll<KMP::ENPT>())
    {
        entry->getPosition().put(out);
        out.putFloat(entry->getRadius());
        out.putShort(static_cast<uint16_t>(entry->getRouteControl()));
        out.put(static_cast<uint8_t>(entry->getDriftControl()));
        out.put(entry->getFlags()); // unknown flags
    }
}

void writeENPHSection(Buffer& out, const KMP& kmp, KMPOffsets* offsets)
{
    out.position(offsets->sectionOffs.at(KMP::SectionType::ENPH));

    out.putArray((uint8_t*)"ENPH", 4);
    out.putShort(kmp.count<KMP::ENPH>());
    out.putShort(0); // unused

    writeKMPGroupSectionEntries<KMP::ENPH>(out, kmp);
}

void writeITPTSection(Buffer& out, const KMP& kmp, KMPOffsets* offsets)
{
    out.position(offsets->sectionOffs.at(KMP::SectionType::ITPT));

    out.putArray((uint8_t*)"ITPT", 4);
    out.putShort(kmp.count<KMP::ITPT>());
    out.putShort(0); // unused

    for (KMP::ITPT* entry : kmp.getAll<KMP::ITPT>())
    {
        entry->getPosition().put(out);
        out.putFloat(entry->getBulletRange());
        out.putShort(static_cast<uint16_t>(entry->getBulletControl()));
        out.putShort((entry->isForceBullet() ? 0x1 : 0x0) | (entry->isShellIgnore() ? 0xA : 0x0));
    }
}

void writeITPHSection(Buffer& out, const KMP& kmp, KMPOffsets* offsets)
{
    out.position(offsets->sectionOffs.at(KMP::SectionType::ITPH));

    out.putArray((uint8_t*)"ITPH", 4);
    out.putShort(kmp.count<KMP::ITPH>());
    out.putShort(0); // unused

    writeKMPGroupSectionEntries<KMP::ITPH>(out, kmp);
}

void writeCKPTSection(Buffer& out, const KMP& kmp, KMPOffsets* offsets)
{
    out.position(offsets->sectionOffs.at(KMP::SectionType::CKPT));

    out.putArray((uint8_t*)"CKPT", 4);
    out.putShort(kmp.count<KMP::CKPT>());
    out.putShort(0); // unused

    for (KMP::CKPT* entry : kmp.getAll<KMP::CKPT>())
    {
        int16_t index = kmp.indexOf(entry), prev = 0xFF, next = 0xFF;
        KMP::CKPH* group = entry->getParent();
        if (group != nullptr)
        {
            prev = (group->getFirst() != nullptr && entry != group->getFirst()) ? index - 1 : 0xFF;
            next = (group->getLast() != nullptr && entry != group->getLast()) ? index + 1 : 0xFF;
        }

        entry->getLeft().put(out);
        entry->getRight().put(out);
        out.put(entry->getRespawn() == nullptr ? 0x00 : kmp.indexOf(entry->getRespawn()));
        out.put(entry->getTypeID());
        out.put(static_cast<uint8_t>(prev));
        out.put(static_cast<uint8_t>(next));
    }
}

void writeCKPHSection(Buffer& out, const KMP& kmp, KMPOffsets* offsets)
{
    out.position(offsets->sectionOffs.at(KMP::SectionType::CKPH));

    out.putArray((uint8_t*)"CKPH", 4);
    out.putShort(kmp.count<KMP::CKPH>());
    out.putShort(0); // unused

    writeKMPGroupSectionEntries<KMP::CKPH>(out, kmp);
}

void writeGOBJSection(Buffer& out, const KMP& kmp, KMPOffsets* offsets)
{
    out.position(offsets->sectionOffs.at(KMP::SectionType::GOBJ));

    out.putArray((uint8_t*)"GOBJ", 4);
    out.putShort(kmp.count<KMP::GOBJ>());
    out.putShort(0); // unused

    for (KMP::GOBJ* entry : kmp.getAll<KMP::GOBJ>())
    {
        out.putShort(entry->getTypeID());
        out.putShort(0); // unknown/unused
        entry->getPosition().put(out);
        entry->getRotation().put(out);
        entry->getScale().put(out);
        out.putShort(entry->getRoute() == nullptr ? 0xFFFF : kmp.indexOf(entry->getRoute()));

        for (uint8_t i = 0; i < KMP::GOBJ::SETTINGS_COUNT; ++i)
        {
            out.putShort(entry->getSetting(i));
        }

        uint16_t flags = 0;
        flags |= entry->isSinglePlayerEnabled() ? 0x09 : 0x00;
        flags |= entry->is2PlayerEnabled() ? 0x12 : 0x00;
        flags |= entry->is3And4PlayerEnabled() ? 0x24 : 0x00;
        out.putShort(flags);
    }
}

uint16_t getTotalPOTIPointCount(const KMP& kmp)
{
    uint16_t count = 0;
    for (KMP::POTI* poti : kmp.getAll<KMP::POTI>())
    {
        count += poti->getPointCount();
    }
    return count;
}

void writePOTISection(Buffer& out, const KMP& kmp, KMPOffsets* offsets)
{
    out.position(offsets->sectionOffs.at(KMP::SectionType::POTI));

    out.putArray((uint8_t*)"POTI", 4);
    out.putShort(kmp.count<KMP::POTI>());
    out.putShort(getTotalPOTIPointCount(kmp)); // point count

    for (KMP::POTI* poti : kmp.getAll<KMP::POTI>())
    {
        out.putShort(poti->getPointCount());
        out.put(poti->isSmooth() ? 0x01 : 0x00);
        out.put(static_cast<uint8_t>(poti->getRouteType()));

        for (KMP::POTI::Point& point : poti->getPoints())
        {
            point.pos.put(out);
            out.putShort(point.val1);
            out.putShort(point.val2);
        }
    }
}

void writeAREASection(Buffer& out, const KMP& kmp, KMPOffsets* offsets)
{
    out.position(offsets->sectionOffs.at(KMP::SectionType::AREA));

    out.putArray((uint8_t*)"AREA", 4);
    out.putShort(kmp.count<KMP::AREA>());
    out.putShort(0); // unused

    for (KMP::AREA* entry : kmp.getAll<KMP::AREA>())
    {
        out.put(static_cast<uint8_t>(entry->getShape()));
        out.put(static_cast<uint8_t>(entry->getAreaType()));

        out.put(
            entry->getAreaType() == KMP::AREA::Type::Camera && entry->getCamera() != nullptr
            ? kmp.indexOf(entry->getCamera()) : 0xFF
        );

        out.put(entry->getPriority());
        entry->getPosition().put(out);
        entry->getRotation().put(out);
        entry->getScale().put(out);
        out.putShort(entry->getSetting1());
        out.putShort(entry->getSetting2());

        out.put(
            entry->getAreaType() == KMP::AREA::Type::MovingRoad && entry->getRoute() != nullptr
            ? kmp.indexOf(entry->getRoute()) : 0xFF
        );

        out.put(
            entry->getAreaType() == KMP::AREA::Type::DestinationPoint &&
            entry->getDestinationPoint() != nullptr
            ? kmp.indexOf(entry->getDestinationPoint()) : 0xFF
        );

        out.putShort(0); // unknown/unused
    }
}

void writeCAMESection(Buffer& out, const KMP& kmp, KMPOffsets* offsets)
{
    out.position(offsets->sectionOffs.at(KMP::SectionType::CAME));

    out.putArray((uint8_t*)"CAME", 4);
    out.putShort(kmp.count<KMP::CAME>());
    out.putShort(kmp.indexOf(kmp.getCamera()) << 8); // first camera index

    for (KMP::CAME* entry : kmp.getAll<KMP::CAME>())
    {
        out.put(static_cast<uint8_t>(entry->getCameraType()));
        out.put(static_cast<uint8_t>(
            entry->getNext() == nullptr ? 0xFF : kmp.indexOf(entry->getNext())
        ));
        out.put(entry->getCamshake());
        out.put(static_cast<uint8_t>(
            entry->getRoute() == nullptr ? 0xFF : kmp.indexOf(entry->getRoute())
        ));

        out.putShort(entry->getPointVelocity());
        out.putShort(entry->getZoomVelocity());
        out.putShort(entry->getViewVelocity());

        out.put(entry->getStartFlags());
        out.put(entry->getMovieFlags());

        entry->getPosition().put(out);
        entry->getRotation().put(out);

        out.putFloat(entry->getZoomStart());
        out.putFloat(entry->getZoomEnd());

        entry->getViewStart().put(out);
        entry->getViewEnd().put(out);

        out.putFloat(entry->getTime());
    }
}

void writeJGPTSection(Buffer& out, const KMP& kmp, KMPOffsets* offsets)
{
    out.position(offsets->sectionOffs.at(KMP::SectionType::JGPT));

    out.putArray((uint8_t*)"JGPT", 4);
    out.putShort(kmp.count<KMP::JGPT>());
    out.putShort(0); // unused

    for (KMP::JGPT* entry : kmp.getAll<KMP::JGPT>())
    {
        entry->getPosition().put(out);
        entry->getRotation().put(out);
        out.putShort(kmp.indexOf(entry));
        out.putShort(entry->getRange());
    }
}

void writeCNPTSection(Buffer& out, const KMP& kmp, KMPOffsets* offsets)
{
    out.position(offsets->sectionOffs.at(KMP::SectionType::CNPT));

    out.putArray((uint8_t*)"CNPT", 4);
    out.putShort(kmp.count<KMP::CNPT>());
    out.putShort(0); // unused

    for (KMP::CNPT* entry : kmp.getAll<KMP::CNPT>())
    {
        entry->getDestination().put(out);
        entry->getDirection().put(out);
        out.putShort(kmp.indexOf(entry));
        out.putShort(entry->getTypeID());
    }
}

void writeMSPTSection(Buffer& out, const KMP& kmp, KMPOffsets* offsets)
{
    out.position(offsets->sectionOffs.at(KMP::SectionType::MSPT));

    out.putArray((uint8_t*)"MSPT", 4);
    out.putShort(kmp.count<KMP::MSPT>());
    out.putShort(0); // unused

    for (KMP::MSPT* entry : kmp.getAll<KMP::MSPT>())
    {
        entry->getPosition().put(out);
        entry->getRotation().put(out);
        out.putShort(kmp.indexOf(entry));
        out.putShort(0); // unknown/unused
    }
}

void writeSTGISection(Buffer& out, const KMP& kmp, KMPOffsets* offsets)
{
    out.position(offsets->sectionOffs.at(KMP::SectionType::STGI));

    out.putArray((uint8_t*)"STGI", 4);
    out.putShort(kmp.count<KMP::STGI>());
    out.putShort(0); // unused

    for (KMP::STGI* entry : kmp.getAll<KMP::STGI>())
    {
        float speedFactor = entry->getSpeedFactor();
        uint32_t intSpeedFactor = reinterpret_cast<uint32_t&>(speedFactor);

        out.put(entry->getLapCount());
        out.put(static_cast<uint8_t>(entry->getStartSide()));
        out.put(entry->isNarrowMode() ? 0x01 : 0x00);
        out.put(entry->isLensFlareEnabled() ? 0x01 : 0x00);
        out.put(0); // unknown/unused
        out.putInt(entry->getLensFlareColour());
        out.put(0); // unknown/unused
        out.putShort(static_cast<uint16_t>(intSpeedFactor >> 16));
    }
}

void writeKMPSections(Buffer& out, const KMP& kmp, KMPOffsets* offsets)
{
    writeKTPTSection(out, kmp, offsets);
    writeENPTSection(out, kmp, offsets);
    writeENPHSection(out, kmp, offsets);
    writeITPTSection(out, kmp, offsets);
    writeITPHSection(out, kmp, offsets);
    writeCKPTSection(out, kmp, offsets);
    writeCKPHSection(out, kmp, offsets);
    writeGOBJSection(out, kmp, offsets);
    writePOTISection(out, kmp, offsets);
    writeAREASection(out, kmp, offsets);
    writeCAMESection(out, kmp, offsets);
    writeJGPTSection(out, kmp, offsets);
    writeCNPTSection(out, kmp, offsets);
    writeMSPTSection(out, kmp, offsets);
    writeSTGISection(out, kmp, offsets);
}

Buffer KMP::write(const KMP& kmp)
{
    ////////////////////////////////////
    /// Setup required information

    KMPInfo info;
    KMPOffsets offsets;
    createKMPInfoAndOffsets(kmp, &info, &offsets);

    ////////////////////////////////////
    /// Write KMP file

    Buffer buffer(info.size);

    writeKMPHeader(buffer, &info, &offsets);
    writeKMPSections(buffer, kmp, &offsets);

    return buffer.clear();
}
}
