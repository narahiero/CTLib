//////////////////////////////////////////////////
//  Copyright (c) 2020 Nara Hiero
//
// This file is licensed under GPLv3+
// Refer to the `License.txt` file included.
//////////////////////////////////////////////////

#include <CTLib/KMP.hpp>

#include <map>
#include <set>

#include <CTLib/Utilities.hpp>

namespace CTLib
{

// KMP header
struct KMPHeader
{

    // offsets to sections
    std::vector<uint32_t> sectionOffs;
};

void readKMPHeader(Buffer& data, KMPHeader* header)
{
    uint32_t dataSize = static_cast<uint32_t>(data.remaining());

    if (dataSize < 0x4C)
    {
        throw KMPError("KMP: Invalid KMP header! Not enough bytes in buffer!");
    }

    if (!Bytes::matchesString("RKMD", *data + 0x00, 4))
    {
        throw KMPError(Strings::format(
            "KMP: Invalid KMP header! Invalid magic! (Expected 'RKMD', Got '%s')",
            Strings::stringify(*data + 0x00, 4)
        ));
    }
    data.getInt(); // move 4 bytes

    uint32_t size = data.getInt();
    if (dataSize < size)
    {
        throw KMPError(Strings::format(
            "KMP: Not enough bytes in buffer! (Expected %d, Got %d)",
            size, dataSize
        ));
    }

    uint16_t sectionsCount = data.getShort();
    if (sectionsCount != 15)
    {
        throw KMPError(Strings::format(
            "KMP: Unsupported KMP format! Wrong section count! (Expected 15, Got %d)",
            sectionsCount
        ));
    }

    uint16_t headerSize = data.getShort();
    if (headerSize != 0x4C)
    {
        throw KMPError(Strings::format(
            "KMP: Unsupported KMP format! Wrong header size! (Expected 76, Got %d)",
            headerSize
        ));
    }

    uint32_t version = data.getInt();
    if (version != 0x9D8)
    {
        throw KMPError(Strings::format(
            "KMP: Unsupported KMP version! (Expected 0x9D8, Got %02X)",
            version
        ));
    }

    for (uint32_t i = 0; i < 15; ++i)
    {
        header->sectionOffs.push_back(data.getInt() + 0x4C);
    }
}

template <class PH, class PT>
void assertValidKMPPTIndex(KMP& kmp, uint8_t index, const char* which)
{
    if (index >= kmp.count<PT>())
    {
        throw KMPError(Strings::format(
            "KMP: %s %s index out of bounds in %s group! (Index %d, Size %d)",
            which, KMP::Section::nameForType(PT::TYPE), KMP::Section::nameForType(PH::TYPE),
            index, kmp.count<PT>()
        ));
    }
}

template <class PH>
void assertValidKMPPHIndex(KMP& kmp, uint8_t index, const char* which)
{
    if (index >= kmp.count<PH>())
    {
        throw KMPError(Strings::format(
            "KMP: %s group index out of bounds in %s group! (Index %d, Size %d)",
            which, KMP::Section::nameForType(PH::TYPE), index, kmp.count<PH>()
        ));
    }
}

// Currently, the *PT section MUST come before the *PH section
// TODO: Allow the *PH section to come before the *PT section

template <class PH, class PT>
void readKMPGroupSection(Buffer& data, KMP& kmp, uint16_t count)
{
    std::map<PH*, std::vector<uint8_t>> prevs, nexts;

    for (uint16_t i = 0; i < count; ++i)
    {
        PH* group = kmp.add<PH>();
        prevs.insert(std::map<PH*, std::vector<uint8_t>>::value_type(group, std::vector<uint8_t>()));
        nexts.insert(std::map<PH*, std::vector<uint8_t>>::value_type(group, std::vector<uint8_t>()));

        uint8_t firstIdx = data.get();
        assertValidKMPPTIndex<PH, PT>(kmp, firstIdx, "First");

        group->setFirst(kmp.get<PT>(firstIdx));

        uint8_t lastIdx = firstIdx + data.get() - 1;
        assertValidKMPPTIndex<PH, PT>(kmp, lastIdx, "Last");

        group->setLast(kmp.get<PT>(lastIdx));

        for (uint8_t i = 0; i < PH::MAX_LINKS; ++i)
        {
            uint8_t idx = data.get();
            if (idx != 0xFF)
            {
                prevs.at(group).push_back(idx);
            }
        }

        for (uint8_t i = 0; i < PH::MAX_LINKS; ++i)
        {
            uint8_t idx = data.get();
            if (idx != 0xFF)
            {
                nexts.at(group).push_back(idx);
            }
        }

        data.get(); // unknown/unused
        data.get(); // unknown/unused
    }

    for (auto& group : prevs)
    {
        for (uint8_t prev : prevs.at(group.first))
        {
            assertValidKMPPHIndex<PH>(kmp, prev, "Previous");
            group.first->addPrevious(kmp.get<PH>(prev));
        }
    }

    for (auto& group : nexts)
    {
        for (uint8_t next : nexts.at(group.first))
        {
            assertValidKMPPHIndex<PH>(kmp, next, "Next");
            group.first->addNext(kmp.get<PH>(next));
        }
    }
}

void readKTPTSection(Buffer& data, KMP& kmp, uint16_t count, uint16_t)
{
    for (uint16_t i = 0; i < count; ++i)
    {
        KMP::KTPT* ktpt = kmp.add<KMP::KTPT>();

        Vector3f pos, rot;
        pos.get(data);
        rot.get(data);

        ktpt->setPosition(pos);
        ktpt->setRotation(rot);
        ktpt->setPlayerIndex(data.getShort());
        data.getShort(); // unknown/unused
    }
}

void readENPTSection(Buffer& data, KMP& kmp, uint16_t count, uint16_t)
{
    for (uint16_t i = 0; i < count; ++i)
    {
        KMP::ENPT* enpt = kmp.add<KMP::ENPT>();

        Vector3f pos;
        pos.get(data);

        enpt->setPosition(pos);
        enpt->setRadius(data.getFloat());
        enpt->setRouteControl(static_cast<KMP::ENPT::RouteControl>(data.getShort()));
        enpt->setDriftControl(static_cast<KMP::ENPT::DriftControl>(data.get()));
        data.get(); // ignore unknown flags
    }
}

void readENPHSection(Buffer& data, KMP& kmp, uint16_t count, uint16_t)
{
    readKMPGroupSection<KMP::ENPH, KMP::ENPT>(data, kmp, count);
}

void readITPTSection(Buffer& data, KMP& kmp, uint16_t count, uint16_t)
{
    for (uint16_t i = 0; i < count; ++i)
    {
        KMP::ITPT* itpt = kmp.add<KMP::ITPT>();

        Vector3f pos;
        pos.get(data);

        itpt->setPosition(pos);
        itpt->setBulletRange(data.getFloat());
        itpt->setBulletControl(static_cast<KMP::ITPT::BulletControl>(data.getShort()));
        
        uint16_t flags = data.getShort();
        itpt->setForceBullet(flags & 0x1);
        itpt->setShellIgnore(flags & 0xA);
    }
}

void readITPHSection(Buffer& data, KMP& kmp, uint16_t count, uint16_t)
{
    readKMPGroupSection<KMP::ITPH, KMP::ITPT>(data, kmp, count);
}

void readCKPTSection(
    Buffer& data, KMP& kmp, uint16_t count, uint16_t, std::map<KMP::CKPT*, uint8_t>& respawns
)
{
    for (uint16_t i = 0; i < count; ++i)
    {
        KMP::CKPT* ckpt = kmp.add<KMP::CKPT>();

        Vector2f left, right;
        left.get(data);
        right.get(data);

        ckpt->setLeft(left);
        ckpt->setRight(right);
        respawns.insert(std::map<KMP::CKPT*, uint8_t>::value_type(ckpt, data.get()));
        ckpt->setTypeID(data.get());
        data.get(); // ignore prev index
        data.get(); // ignore next index
    }
}

void readCKPHSection(Buffer& data, KMP& kmp, uint16_t count, uint16_t)
{
    readKMPGroupSection<KMP::CKPH, KMP::CKPT>(data, kmp, count);
}

void readGOBJSection(
    Buffer& data, KMP& kmp, uint16_t count, uint16_t, std::map<KMP::GOBJ*, uint16_t>& routes
)
{
    for (uint16_t i = 0; i < count; ++i)
    {
        KMP::GOBJ* gobj = kmp.add<KMP::GOBJ>();

        gobj->setTypeID(data.getShort());
        data.getShort(); // unknown/unused

        Vector3f pos, rot, scale;
        pos.get(data);
        rot.get(data);
        scale.get(data);

        gobj->setPosition(pos);
        gobj->setRotation(rot);
        gobj->setScale(scale);
        routes.insert(std::map<KMP::GOBJ*, uint16_t>::value_type(gobj, data.getShort()));
        
        for (uint8_t i = 0; i < KMP::GOBJ::SETTINGS_COUNT; ++i)
        {
            gobj->setSetting(i, data.getShort());
        }

        uint16_t flags = data.getShort();
        gobj->setIsSinglePlayerEnabled(flags & 0x1);
        gobj->setIs2PlayerEnabled(flags & 0x2);
        gobj->setIs3And4PlayerEnabled(flags & 0x4);
    }
}

void readPOTISection(Buffer& data, KMP& kmp, uint16_t count, uint16_t)
{
    for (uint16_t i = 0; i < count; ++i)
    {
        KMP::POTI* poti = kmp.add<KMP::POTI>();

        uint16_t pointCount = data.getShort();
        poti->setIsSmooth(data.get());
        poti->setRouteType(static_cast<KMP::POTI::RouteType>(data.get()));

        for (uint16_t p = 0; p < pointCount; ++p)
        {
            KMP::POTI::Point point;

            point.pos.get(data);
            point.val1 = data.getShort();
            point.val2 = data.getShort();

            poti->addPoint(point);
        }
    }
}

void readAREASection(Buffer& data, KMP& kmp, uint16_t count, uint16_t)
{

}

void readCAMESection(Buffer& data, KMP& kmp, uint16_t count, uint16_t)
{

}

void readJGPTSection(Buffer& data, KMP& kmp, uint16_t count, uint16_t)
{
    for (uint16_t i = 0; i < count; ++i)
    {
        KMP::JGPT* jgpt = kmp.add<KMP::JGPT>();

        Vector3f pos, rot;
        pos.get(data);
        rot.get(data);

        jgpt->setPosition(pos);
        jgpt->setRotation(pos);
        data.getShort(); // ignore ID
        data.getShort(); // range?
    }
}

void readCNPTSection(Buffer& data, KMP& kmp, uint16_t count, uint16_t)
{
    for (uint16_t i = 0; i < count; ++i)
    {
        KMP::CNPT* cnpt = kmp.add<KMP::CNPT>();

        Vector3f dest, dir;
        dest.get(data);
        dir.get(data);

        cnpt->setDestination(dest);
        cnpt->setDirection(dest);
        data.getShort(); // ignore ID
        cnpt->setTypeID(data.getShort());
    }
}

void readMSPTSection(Buffer& data, KMP& kmp, uint16_t count, uint16_t)
{
    for (uint16_t i = 0; i < count; ++i)
    {
        KMP::MSPT* mspt = kmp.add<KMP::MSPT>();

        Vector3f pos, rot;
        pos.get(data);
        rot.get(data);

        mspt->setPosition(pos);
        mspt->setRotation(rot);
        data.getShort(); // ignore ID
        data.getShort(); // unknown/unused
    }
}

void readSTGISection(Buffer& data, KMP& kmp, uint16_t count, uint16_t)
{
    for (uint16_t i = 0; i < count; ++i)
    {
        KMP::STGI* stgi = kmp.add<KMP::STGI>();

        stgi->setLapCount(data.get());
        stgi->setStartSide(static_cast<KMP::STGI::StartSide>(data.get()));
        stgi->setNarrowMode(data.get());
        stgi->setLensFlareEnabled(data.get());
        data.get(); // unknown/unused
        stgi->setLensFlareColour(data.getInt());
        data.get(); // unknown/unused

        uint32_t intSpeedFactor = static_cast<uint32_t>(data.getShort()) << 16;
        if (intSpeedFactor == 0)
        {
            intSpeedFactor = 0x3F800000; // 1.f
        }
        stgi->setSpeedFactor(reinterpret_cast<float&>(intSpeedFactor));
    }
}

KMP::SectionType getKMPSectionType(uint8_t* data)
{
    if (Bytes::matchesString("KTPT", data, 4))
    {
        return KMP::SectionType::KTPT;
    }
    else if (Bytes::matchesString("ENPT", data, 4))
    {
        return KMP::SectionType::ENPT;
    }
    else if (Bytes::matchesString("ENPH", data, 4))
    {
        return KMP::SectionType::ENPH;
    }
    else if (Bytes::matchesString("ITPT", data, 4))
    {
        return KMP::SectionType::ITPT;
    }
    else if (Bytes::matchesString("ITPH", data, 4))
    {
        return KMP::SectionType::ITPH;
    }
    else if (Bytes::matchesString("CKPT", data, 4))
    {
        return KMP::SectionType::CKPT;
    }
    else if (Bytes::matchesString("CKPH", data, 4))
    {
        return KMP::SectionType::CKPH;
    }
    else if (Bytes::matchesString("GOBJ", data, 4))
    {
        return KMP::SectionType::GOBJ;
    }
    else if (Bytes::matchesString("POTI", data, 4))
    {
        return KMP::SectionType::POTI;
    }
    else if (Bytes::matchesString("AREA", data, 4))
    {
        return KMP::SectionType::AREA;
    }
    else if (Bytes::matchesString("CAME", data, 4))
    {
        return KMP::SectionType::CAME;
    }
    else if (Bytes::matchesString("JGPT", data, 4))
    {
        return KMP::SectionType::JGPT;
    }
    else if (Bytes::matchesString("CNPT", data, 4))
    {
        return KMP::SectionType::CNPT;
    }
    else if (Bytes::matchesString("MSPT", data, 4))
    {
        return KMP::SectionType::MSPT;
    }
    else if (Bytes::matchesString("STGI", data, 4))
    {
        return KMP::SectionType::STGI;
    }
    else
    {
        throw KMPError(Strings::format(
            "KMP: Unknown KMP section! (%s)",
            Strings::stringify(data, 4)
        ));
    }
}

void readKMPSections(Buffer& data, KMPHeader* header, KMP& kmp)
{
    std::set<KMP::SectionType> sections;

    std::map<KMP::CKPT*, uint8_t> ckptRespawns;
    std::map<KMP::GOBJ*, uint16_t> gobjRoutes;

    for (uint32_t offset : header->sectionOffs)
    {
        KMP::SectionType section = getKMPSectionType(*data + offset);
        
        if (sections.count(section) > 0)
        {
            throw KMPError(Strings::format(
                "KMP: Duplicate KMP section! (%s)",
                KMP::Section::nameForType(section)
            ));
        }
        sections.insert(section);

        data.position(offset + 4); // skip section name
        uint16_t count = data.getShort();
        uint16_t value = data.getShort();
        switch (section)
        {
        case KMP::SectionType::KTPT:
            readKTPTSection(data, kmp, count, value);
            break;

        case KMP::SectionType::ENPT:
            readENPTSection(data, kmp, count, value);
            break;

        case KMP::SectionType::ENPH:
            readENPHSection(data, kmp, count, value);
            break;

        case KMP::SectionType::ITPT:
            readITPTSection(data, kmp, count, value);
            break;

        case KMP::SectionType::ITPH:
            readITPHSection(data, kmp, count, value);
            break;

        case KMP::SectionType::CKPT:
            readCKPTSection(data, kmp, count, value, ckptRespawns);
            break;

        case KMP::SectionType::CKPH:
            readCKPHSection(data, kmp, count, value);
            break;

        case KMP::SectionType::GOBJ:
            readGOBJSection(data, kmp, count, value, gobjRoutes);
            break;

        case KMP::SectionType::POTI:
            readPOTISection(data, kmp, count, value);
            break;

        case KMP::SectionType::AREA:
            readAREASection(data, kmp, count, value);
            break;

        case KMP::SectionType::CAME:
            readCAMESection(data, kmp, count, value);
            break;

        case KMP::SectionType::JGPT:
            readJGPTSection(data, kmp, count, value);
            break;

        case KMP::SectionType::CNPT:
            readCNPTSection(data, kmp, count, value);
            break;

        case KMP::SectionType::MSPT:
            readMSPTSection(data, kmp, count, value);
            break;

        case KMP::SectionType::STGI:
            readSTGISection(data, kmp, count, value);
            break;

        default:
            break;
        }
    }

    // Resolve CKPT respawns
    for (auto& pair : ckptRespawns)
    {
        if (pair.second >= kmp.count<KMP::JGPT>())
        {
            throw KMPError(Strings::format(
                "KMP: JGPT respawn index out of bounds in CKPT entry! (Index %d, Size %d)",
                pair.second, kmp.count<KMP::JGPT>()
            ));
        }
        pair.first->setRespawn(kmp.get<KMP::JGPT>(pair.second));
    }

    // Resolve GOBJ routes
    for (auto& pair : gobjRoutes)
    {
        if (pair.second == 0xFFFF)
        {
            continue;
        }
        if (pair.second >= kmp.count<KMP::POTI>())
        {
            throw KMPError(Strings::format(
                "KMP: POTI route index out of bounds is GOBJ entry! (Index %d, Size %d)",
                pair.second, kmp.count<KMP::POTI>()
            ));
        }
        pair.first->setRoute(kmp.get<KMP::POTI>(pair.second));
    }
}

KMP KMP::read(Buffer& buffer)
{
    Buffer data = buffer.slice();

    ///// Read header ////////
    KMPHeader header;
    readKMPHeader(data, &header);

    ///// Read sections //////
    KMP kmp;
    readKMPSections(data, &header, kmp);

    return kmp;
}
}
