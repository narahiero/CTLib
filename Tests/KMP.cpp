//////////////////////////////////////////////////
//  Copyright (c) 2020 Nara Hiero
//
// This file is licensed under GPLv3+
// Refer to the `License.txt` file included.
//////////////////////////////////////////////////

#include <gtest/gtest.h>

#include <CTLib/KMP.hpp>

using namespace CTLib;

TEST(KMPTests, AddRemoveAndCount)
{
    KMP kmp;
    EXPECT_EQ(0, kmp.count<KMP::KTPT>());

    kmp.add<KMP::KTPT>();
    EXPECT_EQ(1, kmp.count<KMP::KTPT>());

    kmp.add<KMP::KTPT>();
    kmp.add<KMP::KTPT>();
    EXPECT_EQ(3, kmp.count<KMP::KTPT>());

    kmp.remove<KMP::KTPT>(1);
    EXPECT_EQ(2, kmp.count<KMP::KTPT>());

    kmp.remove<KMP::KTPT>(1);
    EXPECT_EQ(1, kmp.count<KMP::KTPT>());
}

TEST(KMPTests, GetIndexOfAndGetAll)
{
    KMP kmp;

    KMP::KTPT* k0 = kmp.add<KMP::KTPT>();
    EXPECT_EQ(k0, kmp.get<KMP::KTPT>(0));
    EXPECT_EQ(0, kmp.indexOf<KMP::KTPT>(k0));

    KMP::KTPT* k1 = kmp.add<KMP::KTPT>();
    EXPECT_EQ(k0, kmp.get<KMP::KTPT>(0));
    EXPECT_EQ(k1, kmp.get<KMP::KTPT>(1));
    EXPECT_EQ(0, kmp.indexOf<KMP::KTPT>(k0));
    EXPECT_EQ(1, kmp.indexOf<KMP::KTPT>(k1));

    kmp.remove<KMP::KTPT>(0);
    EXPECT_EQ(k1, kmp.get<KMP::KTPT>(0));
    EXPECT_EQ(0, kmp.indexOf<KMP::KTPT>(k1));

    kmp.add<KMP::KTPT>();
    kmp.add<KMP::KTPT>();
    kmp.add<KMP::KTPT>();
    kmp.add<KMP::KTPT>();
    std::vector<KMP::KTPT*> ktpts = kmp.getAll<KMP::KTPT>();
    EXPECT_EQ(5, ktpts.size());

    kmp.remove<KMP::KTPT>(0);
    EXPECT_EQ(4, kmp.count<KMP::KTPT>());
    EXPECT_EQ(5, ktpts.size());
}

TEST(KMPTests, Errors)
{
    KMP kmp;
    EXPECT_THROW(kmp.get<KMP::KTPT>(0), KMPError);
    EXPECT_THROW(kmp.remove<KMP::KTPT>(0), KMPError);

    kmp.add<KMP::KTPT>();
    kmp.add<KMP::KTPT>();
    kmp.add<KMP::KTPT>();
    kmp.add<KMP::KTPT>();
    kmp.add<KMP::KTPT>();
    EXPECT_THROW(kmp.get<KMP::KTPT>(5), KMPError);
    EXPECT_NO_THROW(kmp.get<KMP::KTPT>(4));
    EXPECT_THROW(kmp.remove<KMP::KTPT>(5), KMPError);
    EXPECT_NO_THROW(kmp.remove<KMP::KTPT>(4));
    EXPECT_THROW(kmp.get<KMP::KTPT>(4), KMPError);
    EXPECT_THROW(kmp.remove<KMP::KTPT>(4), KMPError);
}

TEST(KMPSectionTests, NameForType)
{
    EXPECT_STREQ("KTPT", KMP::Section::nameForType(KMP::SectionType::KTPT));
    EXPECT_STREQ("ENPT", KMP::Section::nameForType(KMP::SectionType::ENPT));
    EXPECT_STREQ("ENPH", KMP::Section::nameForType(KMP::SectionType::ENPH));
    EXPECT_STREQ("ITPT", KMP::Section::nameForType(KMP::SectionType::ITPT));
    EXPECT_STREQ("ITPH", KMP::Section::nameForType(KMP::SectionType::ITPH));
    EXPECT_STREQ("CKPT", KMP::Section::nameForType(KMP::SectionType::CKPT));
    EXPECT_STREQ("CKPH", KMP::Section::nameForType(KMP::SectionType::CKPH));
    EXPECT_STREQ("GOBJ", KMP::Section::nameForType(KMP::SectionType::GOBJ));
    EXPECT_STREQ("POTI", KMP::Section::nameForType(KMP::SectionType::POTI));
    EXPECT_STREQ("AREA", KMP::Section::nameForType(KMP::SectionType::AREA));
    EXPECT_STREQ("CAME", KMP::Section::nameForType(KMP::SectionType::CAME));
    EXPECT_STREQ("JGPT", KMP::Section::nameForType(KMP::SectionType::JGPT));
    EXPECT_STREQ("CNPT", KMP::Section::nameForType(KMP::SectionType::CNPT));
    EXPECT_STREQ("MSPT", KMP::Section::nameForType(KMP::SectionType::MSPT));
    EXPECT_STREQ("STGI", KMP::Section::nameForType(KMP::SectionType::STGI));
}

// For the following 'KMPGroupAndPointSectionsTests', only the EN* classes are
// used, because the IT* and CK* classes works the same way

TEST(KMPGroupAndPointSectionsTests, FirstLastAndParent)
{
    KMP kmp;
    KMP::ENPH* group = kmp.add<KMP::ENPH>();
    EXPECT_EQ(nullptr, group->getFirst());
    EXPECT_EQ(nullptr, group->getLast());

    KMP::ENPT* points[100];
    for (uint16_t i = 0; i < 100; ++i)
    {
        points[i] = kmp.add<KMP::ENPT>();
        EXPECT_EQ(nullptr, points[i]->getParent());
    }

    group->setFirst(points[34]);
    EXPECT_EQ(points[34], group->getFirst());
    EXPECT_EQ(nullptr, group->getLast());

    for (uint16_t i = 0; i < 100; ++i)
    {
        EXPECT_EQ(i == 34 ? group : nullptr, points[i]->getParent());
    }

    group->setLast(points[68]);
    EXPECT_EQ(points[34], group->getFirst());
    EXPECT_EQ(points[68], group->getLast());

    for (uint16_t i = 0; i < 100; ++i)
    {
        EXPECT_EQ(i >= 34 && i <= 68 ? group : nullptr, points[i]->getParent());
    }

    group->setFirst(points[45]);
    EXPECT_EQ(points[45], group->getFirst());
    EXPECT_EQ(points[68], group->getLast());

    for (uint16_t i = 0; i < 100; ++i)
    {
        EXPECT_EQ(i >= 45 && i <= 68 ? group : nullptr, points[i]->getParent());
    }

    group->setLast(nullptr);
    EXPECT_EQ(points[45], group->getFirst());
    EXPECT_EQ(nullptr, group->getLast());

    for (uint16_t i = 0; i < 100; ++i)
    {
        EXPECT_EQ(i == 45 ? group : nullptr, points[i]->getParent());
    }

    group->setLast(points[81]);
    EXPECT_EQ(points[45], group->getFirst());
    EXPECT_EQ(points[81], group->getLast());

    for (uint16_t i = 0; i < 100; ++i)
    {
        EXPECT_EQ(i >= 45 && i <= 81 ? group : nullptr, points[i]->getParent());
    }

    group->setFirst(nullptr);
    EXPECT_EQ(nullptr, group->getFirst());
    EXPECT_EQ(nullptr, group->getLast());

    for (uint16_t i = 0; i < 100; ++i)
    {
        EXPECT_EQ(nullptr, points[i]->getParent());
    }
}

TEST(KMPGroupAndPointSectionsTests, RemoveGroupAndParent)
{
    KMP kmp;
    KMP::ENPH* group = kmp.add<KMP::ENPH>();
    
    KMP::ENPT* points[100];
    for (uint16_t i = 0; i < 100; ++i)
    {
        points[i] = kmp.add<KMP::ENPT>();
    }

    group->setFirst(points[37]);
    group->setLast(points[79]);

    for (uint16_t i = 0; i < 100; ++i)
    {
        EXPECT_EQ(i >= 37 && i <= 79 ? group : nullptr, points[i]->getParent());
    }

    kmp.remove<KMP::ENPH>(kmp.indexOf(group));

    for (uint16_t i = 0; i < 100; ++i)
    {
        EXPECT_EQ(nullptr, points[i]->getParent());
    }
}

TEST(KMPGroupAndPointSectionsTests, RemovePointAndFirstLast)
{
    KMP kmp;
    KMP::ENPH* group = kmp.add<KMP::ENPH>();

    std::vector<KMP::ENPT*> points;
    for (uint16_t i = 0; i < 100; ++i)
    {
        points.push_back(kmp.add<KMP::ENPT>());
    }

    group->setFirst(points[26]);
    group->setLast(points[81]);

    for (uint16_t i = 0; i < points.size(); ++i)
    {
        EXPECT_EQ(i >= 26 && i <= 81 ? group : nullptr, points[i]->getParent());
    }

    kmp.remove<KMP::ENPT>(63);
    points.erase(points.begin() + 63);

    EXPECT_EQ(points[26], group->getFirst());
    EXPECT_EQ(points[80], group->getLast());
    for (uint16_t i = 0; i < points.size(); ++i)
    {
        EXPECT_EQ(i >= 26 && i <= 80 ? group : nullptr, points[i]->getParent());
    }

    kmp.remove<KMP::ENPT>(80);
    points.erase(points.begin() + 80);

    EXPECT_EQ(points[26], group->getFirst());
    EXPECT_EQ(points[79], group->getLast());
    for (uint16_t i = 0; i < points.size(); ++i)
    {
        EXPECT_EQ(i >= 26 && i <= 79 ? group : nullptr, points[i]->getParent());
    }

    kmp.remove<KMP::ENPT>(26);
    points.erase(points.begin() + 26);

    EXPECT_EQ(points[26], group->getFirst());
    EXPECT_EQ(points[78], group->getLast());
    for (uint16_t i = 0; i < points.size(); ++i)
    {
        EXPECT_EQ(i >= 26 && i <= 78 ? group : nullptr, points[i]->getParent());
    }

    group->setFirst(points[4]);
    group->setLast(points[4]);

    kmp.remove<KMP::ENPT>(4);
    points.erase(points.begin() + 4);
    
    EXPECT_EQ(nullptr, group->getFirst());
    EXPECT_EQ(nullptr, group->getLast());
    for (uint16_t i = 0; i < points.size(); ++i)
    {
        EXPECT_EQ(nullptr, points[i]->getParent());
    }

    group->setFirst(points[72]);
    group->setLast(nullptr);

    EXPECT_EQ(points[72], group->getFirst());
    EXPECT_EQ(nullptr, group->getLast());
    for (uint16_t i = 0; i < points.size(); ++i)
    {
        EXPECT_EQ(i == 72 ? group : nullptr, points[i]->getParent());
    }

    kmp.remove<KMP::ENPT>(72);
    points.erase(points.begin() + 72);

    EXPECT_EQ(nullptr, group->getFirst());
    EXPECT_EQ(nullptr, group->getLast());
    for (uint16_t i = 0; i < points.size(); ++i)
    {
        EXPECT_EQ(nullptr, points[i]->getParent());
    }
}

TEST(KMPGroupAndPointSectionsTests, PreviousAndNext)
{
    KMP kmp;

    std::vector<KMP::ENPH*> groups;
    for (uint16_t i = 0; i < 5; ++i)
    {
        groups.push_back(kmp.add<KMP::ENPH>());
        EXPECT_EQ(0, groups[i]->getPreviousCount());
        EXPECT_EQ(0, groups[i]->getPrevious().size());
        EXPECT_EQ(0, groups[i]->getNextCount());
        EXPECT_EQ(0, groups[i]->getNext().size());
    }

    groups[0]->addNext(groups[1]);
    groups[1]->addPrevious(groups[0]);
    EXPECT_EQ(0, groups[0]->getPreviousCount());
    EXPECT_EQ(1, groups[0]->getNextCount());
    EXPECT_EQ(groups[1], groups[0]->getNext()[0]);
    EXPECT_EQ(1, groups[1]->getPreviousCount());
    EXPECT_EQ(groups[0], groups[1]->getPrevious()[0]);
    EXPECT_EQ(0, groups[1]->getNextCount());

    groups[1]->addNext(groups[2]);
    groups[1]->addNext(groups[3]);
    groups[2]->addPrevious(groups[1]);
    groups[3]->addPrevious(groups[1]);
    EXPECT_EQ(2, groups[1]->getNextCount());
    EXPECT_EQ(groups[2], groups[1]->getNext()[0]);
    EXPECT_EQ(groups[3], groups[1]->getNext()[1]);
    EXPECT_EQ(1, groups[2]->getPreviousCount());
    EXPECT_EQ(groups[1], groups[2]->getPrevious()[0]);
    EXPECT_EQ(0, groups[2]->getNextCount());
    EXPECT_EQ(1, groups[3]->getPreviousCount());
    EXPECT_EQ(groups[1], groups[3]->getPrevious()[0]);
    EXPECT_EQ(0, groups[3]->getNextCount());

    groups[2]->addNext(groups[4]);
    groups[3]->addNext(groups[4]);
    groups[4]->addPrevious(groups[2]);
    groups[4]->addPrevious(groups[3]);
    EXPECT_EQ(1, groups[2]->getNextCount());
    EXPECT_EQ(groups[4], groups[2]->getNext()[0]);
    EXPECT_EQ(1, groups[3]->getNextCount());
    EXPECT_EQ(groups[4], groups[3]->getNext()[0]);
    EXPECT_EQ(2, groups[4]->getPreviousCount());
    EXPECT_EQ(groups[2], groups[4]->getPrevious()[0]);
    EXPECT_EQ(groups[3], groups[4]->getPrevious()[1]);
    EXPECT_EQ(0, groups[4]->getNextCount());

    groups[4]->addNext(groups[0]);
    groups[0]->addPrevious(groups[4]);
    EXPECT_EQ(1, groups[4]->getNextCount());
    EXPECT_EQ(groups[0], groups[4]->getNext()[0]);
    EXPECT_EQ(1, groups[0]->getPreviousCount());
    EXPECT_EQ(groups[4], groups[0]->getPrevious()[0]);
}

TEST(KMPGroupAndPointSectionsTests, FirstAndLastErrors)
{
    KMP kmp;
    KMP::ENPH* group = kmp.add<KMP::ENPH>();

    KMP::ENPT* points[100];
    for (uint16_t i = 0; i < 100; ++i)
    {
        points[i] = kmp.add<KMP::ENPT>();
    }

    EXPECT_THROW(group->setLast(points[0]), KMPError);
    EXPECT_THROW(group->setLast(points[32]), KMPError);
    EXPECT_THROW(group->setLast(points[57]), KMPError);
    EXPECT_THROW(group->setLast(points[83]), KMPError);

    group->setFirst(points[37]);
    EXPECT_THROW(group->setLast(points[12]), KMPError);
    EXPECT_THROW(group->setLast(points[26]), KMPError);
    EXPECT_THROW(group->setLast(points[36]), KMPError);
    EXPECT_NO_THROW(group->setLast(points[37]));
    EXPECT_NO_THROW(group->setLast(points[59]));
    EXPECT_NO_THROW(group->setLast(points[72]));
    EXPECT_NO_THROW(group->setLast(points[83]));

    group->setLast(points[74]);
    EXPECT_THROW(group->setFirst(points[98]), KMPError);
    EXPECT_THROW(group->setFirst(points[81]), KMPError);
    EXPECT_THROW(group->setFirst(points[75]), KMPError);
    EXPECT_NO_THROW(group->setFirst(points[74]));
    EXPECT_NO_THROW(group->setFirst(points[52]));
    EXPECT_NO_THROW(group->setFirst(points[38]));
    EXPECT_NO_THROW(group->setFirst(points[15]));

    KMP::ENPH* group2 = kmp.add<KMP::ENPH>();
    
    group->setFirst(points[26]);
    group->setLast(points[72]);
    EXPECT_NO_THROW(group2->setFirst(points[3]));
    EXPECT_NO_THROW(group2->setFirst(points[17]));
    EXPECT_NO_THROW(group2->setFirst(points[25]));
    EXPECT_THROW(group2->setFirst(points[26]), KMPError);
    EXPECT_THROW(group2->setFirst(points[38]), KMPError);
    EXPECT_THROW(group2->setFirst(points[53]), KMPError);
    EXPECT_THROW(group2->setFirst(points[69]), KMPError);
    EXPECT_THROW(group2->setFirst(points[72]), KMPError);
    EXPECT_NO_THROW(group2->setFirst(points[73]));
    EXPECT_NO_THROW(group2->setFirst(points[86]));
    EXPECT_NO_THROW(group2->setFirst(points[95]));

    group2->setFirst(points[0]);
    EXPECT_NO_THROW(group2->setLast(points[3]));
    EXPECT_NO_THROW(group2->setLast(points[17]));
    EXPECT_NO_THROW(group2->setLast(points[25]));
    EXPECT_THROW(group2->setLast(points[26]), KMPError);
    EXPECT_THROW(group2->setLast(points[38]), KMPError);
    EXPECT_THROW(group2->setLast(points[53]), KMPError);
    EXPECT_THROW(group2->setLast(points[69]), KMPError);
    EXPECT_THROW(group2->setLast(points[72]), KMPError);
    EXPECT_NO_THROW(group2->setLast(points[73]));
    EXPECT_NO_THROW(group2->setLast(points[86]));
    EXPECT_NO_THROW(group2->setLast(points[95]));
}

TEST(KMPGroupAndPointSectionTests, NextAndPreviousErrors)
{
    KMP kmp;

    KMP::ENPH* group0 = kmp.add<KMP::ENPH>();
    KMP::ENPH* group1 = kmp.add<KMP::ENPH>();

    EXPECT_THROW(group0->addPrevious(nullptr), KMPError);
    EXPECT_THROW(group0->removePrevious(group1), KMPError);

    EXPECT_THROW(group0->addNext(nullptr), KMPError);
    EXPECT_THROW(group0->removeNext(group1), KMPError);

    for (uint8_t i = 0; i < KMP::ENPH::MAX_LINKS; ++i)
    {
        group0->addPrevious(group1);
        group0->addNext(group1);
    }
    EXPECT_THROW(group0->addPrevious(group1), KMPError);
    EXPECT_THROW(group0->addNext(group1), KMPError);
}

TEST(KMPGroupAndPointSectionsTests, OtherKMPErrors)
{
    KMP kmp;
    KMP kmp2;

    KMP::ENPH* group = kmp.add<KMP::ENPH>();
    KMP::ENPT* pointFromKMP2 = kmp2.add<KMP::ENPT>();
    EXPECT_THROW(group->setFirst(pointFromKMP2), KMPError);
    EXPECT_THROW(group->setLast(pointFromKMP2), KMPError);

    KMP::ENPH* groupFromKMP2 = kmp2.add<KMP::ENPH>();
    EXPECT_THROW(group->addNext(groupFromKMP2), KMPError);
    EXPECT_THROW(group->addPrevious(groupFromKMP2), KMPError);
    EXPECT_THROW(groupFromKMP2->addNext(group), KMPError);
    EXPECT_THROW(groupFromKMP2->addPrevious(group), KMPError);
}

TEST(KMPKTPTTests, Errors)
{
    KMP kmp;
    KMP::KTPT* ktpt = kmp.add<KMP::KTPT>();

    EXPECT_THROW(ktpt->setPlayerIndex(-32), KMPError);
    EXPECT_THROW(ktpt->setPlayerIndex(-2), KMPError);
    EXPECT_NO_THROW(ktpt->setPlayerIndex(-1));
    EXPECT_NO_THROW(ktpt->setPlayerIndex(0));
    EXPECT_NO_THROW(ktpt->setPlayerIndex(3));
    EXPECT_NO_THROW(ktpt->setPlayerIndex(8));
    EXPECT_NO_THROW(ktpt->setPlayerIndex(10));
    EXPECT_NO_THROW(ktpt->setPlayerIndex(11));
    EXPECT_THROW(ktpt->setPlayerIndex(12), KMPError);
    EXPECT_THROW(ktpt->setPlayerIndex(234), KMPError);
}

TEST(KMPENPTTests, Errors)
{
    KMP kmp;

    for (uint16_t i = 0; i < KMP::ENPT::MAX_ENTRY_COUNT; ++i)
    {
        kmp.add<KMP::ENPT>();
    }
    EXPECT_THROW(kmp.add<KMP::ENPT>(), KMPError);
}

TEST(KMPITPTTests, Errors)
{
    KMP kmp;

    for (uint16_t i = 0; i < KMP::ITPT::MAX_ENTRY_COUNT; ++i)
    {
        kmp.add<KMP::ITPT>();
    }
    EXPECT_THROW(kmp.add<KMP::ITPT>(), KMPError);
}

TEST(KMPCKPTTests, RemoveJGPT)
{
    KMP kmp;
    KMP::CKPT* ckpt = kmp.add<KMP::CKPT>();
    EXPECT_EQ(nullptr, ckpt->getRespawn());

    KMP::JGPT* jgpt = kmp.add<KMP::JGPT>();
    ckpt->setRespawn(jgpt);
    EXPECT_EQ(jgpt, ckpt->getRespawn());

    kmp.remove<KMP::JGPT>(kmp.indexOf(jgpt));
    EXPECT_EQ(nullptr, ckpt->getRespawn());
}

TEST(KMPCKPTTests, Errors)
{
    KMP kmp;

    for (uint16_t i = 0; i < KMP::CKPT::MAX_ENTRY_COUNT; ++i)
    {
        kmp.add<KMP::CKPT>();
    }
    EXPECT_THROW(kmp.add<KMP::CKPT>(), KMPError);
}

TEST(KMPCKPTTests, OtherKMPErrors)
{
    KMP kmp;
    KMP::CKPT* ckpt = kmp.add<KMP::CKPT>();

    KMP kmp2;
    KMP::JGPT* jgpt = kmp2.add<KMP::JGPT>();

    EXPECT_THROW(ckpt->setRespawn(jgpt), KMPError);
}

TEST(KMPGOBJTests, RemovePOTI)
{
    KMP kmp;
    KMP::GOBJ* gobj = kmp.add<KMP::GOBJ>();
    EXPECT_EQ(nullptr, gobj->getRoute());

    KMP::POTI* poti = kmp.add<KMP::POTI>();
    gobj->setRoute(poti);
    EXPECT_EQ(poti, gobj->getRoute());

    kmp.remove<KMP::POTI>(kmp.indexOf(poti));
    EXPECT_EQ(nullptr, gobj->getRoute());
}

TEST(KMPGOBJTests, Errors)
{
    KMP kmp;
    KMP::GOBJ* gobj = kmp.add<KMP::GOBJ>();

    for (uint8_t i = 0; i < KMP::GOBJ::SETTINGS_COUNT; ++i)
    {
        EXPECT_NO_THROW(gobj->setSetting(i, 0));
    }
    for (uint8_t i = KMP::GOBJ::SETTINGS_COUNT; i != 0; ++i)
    {
        EXPECT_THROW(gobj->setSetting(i, 0), KMPError);
    }
}

TEST(KMPGOBJTests, OtherKMPErrors)
{
    KMP kmp;
    KMP kmp2;

    KMP::GOBJ* gobj = kmp.add<KMP::GOBJ>();
    KMP::POTI* potiFromKMP2 = kmp2.add<KMP::POTI>();
    EXPECT_THROW(gobj->setRoute(potiFromKMP2), KMPError);
}

TEST(KMPPOTITests, Errors)
{
    KMP kmp;
    KMP::POTI* poti = kmp.add<KMP::POTI>();

    EXPECT_THROW(poti->getPoint(0), KMPError);
    EXPECT_THROW(poti->removePoint(0), KMPError);

    poti->addPoint(KMP::POTI::Point());
    EXPECT_THROW(poti->getPoint(1), KMPError);
    EXPECT_THROW(poti->removePoint(1), KMPError);
    EXPECT_NO_THROW(poti->getPoint(0));
    EXPECT_NO_THROW(poti->removePoint(0)); // point removed here
    EXPECT_THROW(poti->getPoint(0), KMPError);
    EXPECT_THROW(poti->removePoint(0), KMPError);

    for (uint16_t i = 0; i < 100; ++i)
    {
        poti->addPoint(KMP::POTI::Point());
        EXPECT_NO_THROW(poti->getPoint(i));
        EXPECT_THROW(poti->getPoint(i + 1), KMPError);
    }

    for (uint16_t i = 0; i < 100; ++i)
    {
        EXPECT_NO_THROW(poti->removePoint(0));
    }
    EXPECT_THROW(poti->removePoint(0), KMPError);
}

TEST(KMPAREATests, RemoveCAMEAndPOTIAndENPT)
{
    KMP kmp;
    KMP::AREA* area = kmp.add<KMP::AREA>();
    EXPECT_EQ(nullptr, area->getCamera());
    EXPECT_EQ(nullptr, area->getRoute());
    EXPECT_EQ(nullptr, area->getDestinationPoint());

    KMP::CAME* came = kmp.add<KMP::CAME>();
    area->setCamera(came);
    EXPECT_EQ(came, area->getCamera());
    EXPECT_EQ(nullptr, area->getRoute());
    EXPECT_EQ(nullptr, area->getDestinationPoint());

    KMP::POTI* poti = kmp.add<KMP::POTI>();
    KMP::ENPT* enpt = kmp.add<KMP::ENPT>();
    area->setRoute(poti);
    area->setDestinationPoint(enpt);
    EXPECT_EQ(came, area->getCamera());
    EXPECT_EQ(poti, area->getRoute());
    EXPECT_EQ(enpt, area->getDestinationPoint());

    kmp.remove<KMP::POTI>(kmp.indexOf(poti));
    EXPECT_EQ(came, area->getCamera());
    EXPECT_EQ(nullptr, area->getRoute());
    EXPECT_EQ(enpt, area->getDestinationPoint());

    kmp.remove<KMP::CAME>(kmp.indexOf(came));
    kmp.remove<KMP::ENPT>(kmp.indexOf(enpt));
    EXPECT_EQ(nullptr, area->getCamera());
    EXPECT_EQ(nullptr, area->getRoute());
    EXPECT_EQ(nullptr, area->getDestinationPoint());
}

TEST(KMPAREATests, OtherKMPErrors)
{
    KMP kmp;
    KMP kmp2;

    KMP::AREA* area = kmp.add<KMP::AREA>();

    KMP::CAME* cameFromKMP2 = kmp2.add<KMP::CAME>();
    EXPECT_THROW(area->setCamera(cameFromKMP2), KMPError);

    KMP::POTI* potiFromKMP2 = kmp2.add<KMP::POTI>();
    EXPECT_THROW(area->setRoute(potiFromKMP2), KMPError);

    KMP::ENPT* enptFromKMP2 = kmp2.add<KMP::ENPT>();
    EXPECT_THROW(area->setDestinationPoint(enptFromKMP2), KMPError);
}

TEST(KMPCAMETests, RemoveCAMEAndPOTI)
{
    KMP kmp;
    KMP::CAME* came = kmp.add<KMP::CAME>();
    EXPECT_EQ(nullptr, came->getNext());
    EXPECT_EQ(nullptr, came->getRoute());

    KMP::CAME* came2 = kmp.add<KMP::CAME>();
    came->setNext(came2);
    EXPECT_EQ(came2, came->getNext());
    EXPECT_EQ(nullptr, came->getRoute());

    KMP::POTI* poti = kmp.add<KMP::POTI>();
    came->setRoute(poti);
    EXPECT_EQ(came2, came->getNext());
    EXPECT_EQ(poti, came->getRoute());

    kmp.remove<KMP::CAME>(kmp.indexOf(came2));
    EXPECT_EQ(nullptr, came->getNext());
    EXPECT_EQ(poti, came->getRoute());

    kmp.remove<KMP::POTI>(kmp.indexOf(poti));
    EXPECT_EQ(nullptr, came->getNext());
    EXPECT_EQ(nullptr, came->getRoute());
}

TEST(KMPCAMETests, OtherKMPErrors)
{
    KMP kmp;
    KMP kmp2;

    KMP::CAME* came = kmp.add<KMP::CAME>();

    KMP::CAME* cameFromKMP2 = kmp2.add<KMP::CAME>();
    EXPECT_THROW(came->setNext(cameFromKMP2), KMPError);

    KMP::POTI* potiFromKMP2 = kmp2.add<KMP::POTI>();
    EXPECT_THROW(came->setRoute(potiFromKMP2), KMPError);
}

TEST(KMPJGPTTests, Errors)
{
    KMP kmp;

    for (uint16_t i = 0; i < KMP::JGPT::MAX_ENTRY_COUNT; ++i)
    {
        kmp.add<KMP::JGPT>();
    }
    EXPECT_THROW(kmp.add<KMP::JGPT>(), KMPError);
}

TEST(KMPCNPTTests, CannonType)
{
    KMP kmp;
    KMP::CNPT* cnpt = kmp.add<KMP::CNPT>();
    EXPECT_EQ(KMP::CNPT::CannonType::Default, cnpt->getCannonType());
    EXPECT_EQ(0, cnpt->getTypeID());

    cnpt->setCannonType(KMP::CNPT::CannonType::Slow);
    EXPECT_EQ(KMP::CNPT::CannonType::Slow, cnpt->getCannonType());
    EXPECT_EQ(2, cnpt->getTypeID());

    cnpt->setTypeID(1);
    EXPECT_EQ(KMP::CNPT::CannonType::Curved, cnpt->getCannonType());
    EXPECT_EQ(1, cnpt->getTypeID());

    cnpt->setTypeID(4);
    EXPECT_EQ(KMP::CNPT::CannonType::Custom, cnpt->getCannonType());
    EXPECT_EQ(4, cnpt->getTypeID());

    cnpt->setCannonType(KMP::CNPT::CannonType::Custom);
    EXPECT_EQ(KMP::CNPT::CannonType::Custom, cnpt->getCannonType());
    EXPECT_EQ(-1, cnpt->getTypeID());
}

TEST(KMPSTGITests, Errors)
{
    KMP kmp;
    KMP::STGI* stgi = kmp.add<KMP::STGI>();
    EXPECT_THROW(kmp.add<KMP::STGI>(), KMPError);

    EXPECT_THROW(stgi->setLapCount(0), KMPError);
    EXPECT_THROW(stgi->setLapCount(10), KMPError);
    EXPECT_THROW(stgi->setLapCount(23), KMPError);
    EXPECT_THROW(stgi->setLapCount(94), KMPError);
    EXPECT_THROW(stgi->setLapCount(210), KMPError);

    EXPECT_THROW(stgi->setSpeedFactor(0.f), KMPError);
    EXPECT_THROW(stgi->setSpeedFactor(-0.f), KMPError);
    EXPECT_THROW(stgi->setSpeedFactor(-1.f), KMPError);
    EXPECT_THROW(stgi->setSpeedFactor(-35.f), KMPError);
}
