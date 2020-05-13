//////////////////////////////////////////////////
//  Copyright (c) 2020 Nara Hiero
//
// This file is licensed under GPLv3+
// Refer to the `License.txt` file included.
//////////////////////////////////////////////////

#include <gtest/gtest.h>

#include <CTLib/KMP.hpp>

TEST(KMPTests, AddRemoveAndCount)
{
    CTLib::KMP kmp;
    EXPECT_EQ(0, kmp.count<CTLib::KMP::KTPT>());

    kmp.add<CTLib::KMP::KTPT>();
    EXPECT_EQ(1, kmp.count<CTLib::KMP::KTPT>());

    kmp.add<CTLib::KMP::KTPT>();
    kmp.add<CTLib::KMP::KTPT>();
    EXPECT_EQ(3, kmp.count<CTLib::KMP::KTPT>());

    kmp.remove<CTLib::KMP::KTPT>(1);
    EXPECT_EQ(2, kmp.count<CTLib::KMP::KTPT>());

    kmp.remove<CTLib::KMP::KTPT>(1);
    EXPECT_EQ(1, kmp.count<CTLib::KMP::KTPT>());
}

TEST(KMPTests, GetIndexOfAndGetAll)
{
    CTLib::KMP kmp;

    CTLib::KMP::KTPT* k0 = kmp.add<CTLib::KMP::KTPT>();
    EXPECT_EQ(k0, kmp.get<CTLib::KMP::KTPT>(0));
    EXPECT_EQ(0, kmp.indexOf<CTLib::KMP::KTPT>(k0));

    CTLib::KMP::KTPT* k1 = kmp.add<CTLib::KMP::KTPT>();
    EXPECT_EQ(k0, kmp.get<CTLib::KMP::KTPT>(0));
    EXPECT_EQ(k1, kmp.get<CTLib::KMP::KTPT>(1));
    EXPECT_EQ(0, kmp.indexOf<CTLib::KMP::KTPT>(k0));
    EXPECT_EQ(1, kmp.indexOf<CTLib::KMP::KTPT>(k1));

    kmp.remove<CTLib::KMP::KTPT>(0);
    EXPECT_EQ(k1, kmp.get<CTLib::KMP::KTPT>(0));
    EXPECT_EQ(0, kmp.indexOf<CTLib::KMP::KTPT>(k1));

    kmp.add<CTLib::KMP::KTPT>();
    kmp.add<CTLib::KMP::KTPT>();
    kmp.add<CTLib::KMP::KTPT>();
    kmp.add<CTLib::KMP::KTPT>();
    std::vector<CTLib::KMP::KTPT*> ktpts = kmp.getAll<CTLib::KMP::KTPT>();
    EXPECT_EQ(5, ktpts.size());

    kmp.remove<CTLib::KMP::KTPT>(0);
    EXPECT_EQ(4, kmp.count<CTLib::KMP::KTPT>());
    EXPECT_EQ(5, ktpts.size());
}

TEST(KMPTests, Errors)
{
    CTLib::KMP kmp;
    EXPECT_THROW(kmp.get<CTLib::KMP::KTPT>(0), CTLib::KMPError);
    EXPECT_THROW(kmp.remove<CTLib::KMP::KTPT>(0), CTLib::KMPError);

    kmp.add<CTLib::KMP::KTPT>();
    kmp.add<CTLib::KMP::KTPT>();
    kmp.add<CTLib::KMP::KTPT>();
    kmp.add<CTLib::KMP::KTPT>();
    kmp.add<CTLib::KMP::KTPT>();
    EXPECT_THROW(kmp.get<CTLib::KMP::KTPT>(5), CTLib::KMPError);
    EXPECT_NO_THROW(kmp.get<CTLib::KMP::KTPT>(4));
    EXPECT_THROW(kmp.remove<CTLib::KMP::KTPT>(5), CTLib::KMPError);
    EXPECT_NO_THROW(kmp.remove<CTLib::KMP::KTPT>(4));
    EXPECT_THROW(kmp.get<CTLib::KMP::KTPT>(4), CTLib::KMPError);
    EXPECT_THROW(kmp.remove<CTLib::KMP::KTPT>(4), CTLib::KMPError);
}

TEST(KMPSectionTests, NameForType)
{
    EXPECT_STREQ("KTPT", CTLib::KMP::Section::nameForType(CTLib::KMP::SectionType::KTPT));
    EXPECT_STREQ("ENPT", CTLib::KMP::Section::nameForType(CTLib::KMP::SectionType::ENPT));
    EXPECT_STREQ("ENPH", CTLib::KMP::Section::nameForType(CTLib::KMP::SectionType::ENPH));
    EXPECT_STREQ("ITPT", CTLib::KMP::Section::nameForType(CTLib::KMP::SectionType::ITPT));
    EXPECT_STREQ("ITPH", CTLib::KMP::Section::nameForType(CTLib::KMP::SectionType::ITPH));
    EXPECT_STREQ("CKPT", CTLib::KMP::Section::nameForType(CTLib::KMP::SectionType::CKPT));
    EXPECT_STREQ("CKPH", CTLib::KMP::Section::nameForType(CTLib::KMP::SectionType::CKPH));
    EXPECT_STREQ("GOBJ", CTLib::KMP::Section::nameForType(CTLib::KMP::SectionType::GOBJ));
    EXPECT_STREQ("POTI", CTLib::KMP::Section::nameForType(CTLib::KMP::SectionType::POTI));
    EXPECT_STREQ("AREA", CTLib::KMP::Section::nameForType(CTLib::KMP::SectionType::AREA));
    EXPECT_STREQ("CAME", CTLib::KMP::Section::nameForType(CTLib::KMP::SectionType::CAME));
    EXPECT_STREQ("JGPT", CTLib::KMP::Section::nameForType(CTLib::KMP::SectionType::JGPT));
    EXPECT_STREQ("CNPT", CTLib::KMP::Section::nameForType(CTLib::KMP::SectionType::CNPT));
    EXPECT_STREQ("MSPT", CTLib::KMP::Section::nameForType(CTLib::KMP::SectionType::MSPT));
    EXPECT_STREQ("STGI", CTLib::KMP::Section::nameForType(CTLib::KMP::SectionType::STGI));
}

// For the following 'KMPGroupAndPointSectionsTests', only the EN* classes are
// used, because the IT* and CK* classes works the same way

TEST(KMPGroupAndPointSectionsTests, FirstLastAndParent)
{
    CTLib::KMP kmp;
    CTLib::KMP::ENPH* group = kmp.add<CTLib::KMP::ENPH>();
    EXPECT_EQ(nullptr, group->getFirst());
    EXPECT_EQ(nullptr, group->getLast());

    CTLib::KMP::ENPT* points[100];
    for (uint16_t i = 0; i < 100; ++i)
    {
        points[i] = kmp.add<CTLib::KMP::ENPT>();
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
    CTLib::KMP kmp;
    CTLib::KMP::ENPH* group = kmp.add<CTLib::KMP::ENPH>();
    
    CTLib::KMP::ENPT* points[100];
    for (uint16_t i = 0; i < 100; ++i)
    {
        points[i] = kmp.add<CTLib::KMP::ENPT>();
    }

    group->setFirst(points[37]);
    group->setLast(points[79]);

    for (uint16_t i = 0; i < 100; ++i)
    {
        EXPECT_EQ(i >= 37 && i <= 79 ? group : nullptr, points[i]->getParent());
    }

    kmp.remove<CTLib::KMP::ENPH>(kmp.indexOf(group));

    for (uint16_t i = 0; i < 100; ++i)
    {
        EXPECT_EQ(nullptr, points[i]->getParent());
    }
}

TEST(KMPGroupAndPointSectionsTests, RemovePointAndFirstLast)
{
    CTLib::KMP kmp;
    CTLib::KMP::ENPH* group = kmp.add<CTLib::KMP::ENPH>();

    std::vector<CTLib::KMP::ENPT*> points;
    for (uint16_t i = 0; i < 100; ++i)
    {
        points.push_back(kmp.add<CTLib::KMP::ENPT>());
    }

    group->setFirst(points[26]);
    group->setLast(points[81]);

    for (uint16_t i = 0; i < points.size(); ++i)
    {
        EXPECT_EQ(i >= 26 && i <= 81 ? group : nullptr, points[i]->getParent());
    }

    kmp.remove<CTLib::KMP::ENPT>(63);
    points.erase(points.begin() + 63);

    EXPECT_EQ(points[26], group->getFirst());
    EXPECT_EQ(points[80], group->getLast());
    for (uint16_t i = 0; i < points.size(); ++i)
    {
        EXPECT_EQ(i >= 26 && i <= 80 ? group : nullptr, points[i]->getParent());
    }

    kmp.remove<CTLib::KMP::ENPT>(80);
    points.erase(points.begin() + 80);

    EXPECT_EQ(points[26], group->getFirst());
    EXPECT_EQ(points[79], group->getLast());
    for (uint16_t i = 0; i < points.size(); ++i)
    {
        EXPECT_EQ(i >= 26 && i <= 79 ? group : nullptr, points[i]->getParent());
    }

    kmp.remove<CTLib::KMP::ENPT>(26);
    points.erase(points.begin() + 26);

    EXPECT_EQ(points[26], group->getFirst());
    EXPECT_EQ(points[78], group->getLast());
    for (uint16_t i = 0; i < points.size(); ++i)
    {
        EXPECT_EQ(i >= 26 && i <= 78 ? group : nullptr, points[i]->getParent());
    }

    group->setFirst(points[4]);
    group->setLast(points[4]);

    kmp.remove<CTLib::KMP::ENPT>(4);
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

    kmp.remove<CTLib::KMP::ENPT>(72);
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
    CTLib::KMP kmp;

    std::vector<CTLib::KMP::ENPH*> groups;
    for (uint16_t i = 0; i < 5; ++i)
    {
        groups.push_back(kmp.add<CTLib::KMP::ENPH>());
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
    CTLib::KMP kmp;
    CTLib::KMP::ENPH* group = kmp.add<CTLib::KMP::ENPH>();

    CTLib::KMP::ENPT* points[100];
    for (uint16_t i = 0; i < 100; ++i)
    {
        points[i] = kmp.add<CTLib::KMP::ENPT>();
    }

    EXPECT_THROW(group->setLast(points[0]), CTLib::KMPError);
    EXPECT_THROW(group->setLast(points[32]), CTLib::KMPError);
    EXPECT_THROW(group->setLast(points[57]), CTLib::KMPError);
    EXPECT_THROW(group->setLast(points[83]), CTLib::KMPError);

    group->setFirst(points[37]);
    EXPECT_THROW(group->setLast(points[12]), CTLib::KMPError);
    EXPECT_THROW(group->setLast(points[26]), CTLib::KMPError);
    EXPECT_THROW(group->setLast(points[36]), CTLib::KMPError);
    EXPECT_NO_THROW(group->setLast(points[37]));
    EXPECT_NO_THROW(group->setLast(points[59]));
    EXPECT_NO_THROW(group->setLast(points[72]));
    EXPECT_NO_THROW(group->setLast(points[83]));

    group->setLast(points[74]);
    EXPECT_THROW(group->setFirst(points[98]), CTLib::KMPError);
    EXPECT_THROW(group->setFirst(points[81]), CTLib::KMPError);
    EXPECT_THROW(group->setFirst(points[75]), CTLib::KMPError);
    EXPECT_NO_THROW(group->setFirst(points[74]));
    EXPECT_NO_THROW(group->setFirst(points[52]));
    EXPECT_NO_THROW(group->setFirst(points[38]));
    EXPECT_NO_THROW(group->setFirst(points[15]));

    CTLib::KMP::ENPH* group2 = kmp.add<CTLib::KMP::ENPH>();
    
    group->setFirst(points[26]);
    group->setLast(points[72]);
    EXPECT_NO_THROW(group2->setFirst(points[3]));
    EXPECT_NO_THROW(group2->setFirst(points[17]));
    EXPECT_NO_THROW(group2->setFirst(points[25]));
    EXPECT_THROW(group2->setFirst(points[26]), CTLib::KMPError);
    EXPECT_THROW(group2->setFirst(points[38]), CTLib::KMPError);
    EXPECT_THROW(group2->setFirst(points[53]), CTLib::KMPError);
    EXPECT_THROW(group2->setFirst(points[69]), CTLib::KMPError);
    EXPECT_THROW(group2->setFirst(points[72]), CTLib::KMPError);
    EXPECT_NO_THROW(group2->setFirst(points[73]));
    EXPECT_NO_THROW(group2->setFirst(points[86]));
    EXPECT_NO_THROW(group2->setFirst(points[95]));

    group2->setFirst(points[0]);
    EXPECT_NO_THROW(group2->setLast(points[3]));
    EXPECT_NO_THROW(group2->setLast(points[17]));
    EXPECT_NO_THROW(group2->setLast(points[25]));
    EXPECT_THROW(group2->setLast(points[26]), CTLib::KMPError);
    EXPECT_THROW(group2->setLast(points[38]), CTLib::KMPError);
    EXPECT_THROW(group2->setLast(points[53]), CTLib::KMPError);
    EXPECT_THROW(group2->setLast(points[69]), CTLib::KMPError);
    EXPECT_THROW(group2->setLast(points[72]), CTLib::KMPError);
    EXPECT_NO_THROW(group2->setLast(points[73]));
    EXPECT_NO_THROW(group2->setLast(points[86]));
    EXPECT_NO_THROW(group2->setLast(points[95]));
}

TEST(KMPGroupAndPointSectionTests, NextAndPreviousErrors)
{
    CTLib::KMP kmp;

    CTLib::KMP::ENPH* group0 = kmp.add<CTLib::KMP::ENPH>();
    CTLib::KMP::ENPH* group1 = kmp.add<CTLib::KMP::ENPH>();

    EXPECT_THROW(group0->addPrevious(nullptr), CTLib::KMPError);
    EXPECT_THROW(group0->removePrevious(group1), CTLib::KMPError);

    EXPECT_THROW(group0->addNext(nullptr), CTLib::KMPError);
    EXPECT_THROW(group0->removeNext(group1), CTLib::KMPError);

    for (uint8_t i = 0; i < CTLib::KMP::ENPH::MAX_LINKS; ++i)
    {
        group0->addPrevious(group1);
        group0->addNext(group1);
    }
    EXPECT_THROW(group0->addPrevious(group1), CTLib::KMPError);
    EXPECT_THROW(group0->addNext(group1), CTLib::KMPError);
}

TEST(KMPGroupAndPointSectionsTests, OtherKMPErrors)
{
    CTLib::KMP kmp;
    CTLib::KMP kmp2;

    CTLib::KMP::ENPH* group = kmp.add<CTLib::KMP::ENPH>();
    CTLib::KMP::ENPT* pointFromKMP2 = kmp2.add<CTLib::KMP::ENPT>();
    EXPECT_THROW(group->setFirst(pointFromKMP2), CTLib::KMPError);
    EXPECT_THROW(group->setLast(pointFromKMP2), CTLib::KMPError);

    CTLib::KMP::ENPH* groupFromKMP2 = kmp2.add<CTLib::KMP::ENPH>();
    EXPECT_THROW(group->addNext(groupFromKMP2), CTLib::KMPError);
    EXPECT_THROW(group->addPrevious(groupFromKMP2), CTLib::KMPError);
    EXPECT_THROW(groupFromKMP2->addNext(group), CTLib::KMPError);
    EXPECT_THROW(groupFromKMP2->addPrevious(group), CTLib::KMPError);
}

TEST(KMPKTPTTests, Errors)
{
    CTLib::KMP kmp;
    CTLib::KMP::KTPT* ktpt = kmp.add<CTLib::KMP::KTPT>();

    EXPECT_THROW(ktpt->setPlayerIndex(-32), CTLib::KMPError);
    EXPECT_THROW(ktpt->setPlayerIndex(-2), CTLib::KMPError);
    EXPECT_NO_THROW(ktpt->setPlayerIndex(-1));
    EXPECT_NO_THROW(ktpt->setPlayerIndex(0));
    EXPECT_NO_THROW(ktpt->setPlayerIndex(3));
    EXPECT_NO_THROW(ktpt->setPlayerIndex(8));
    EXPECT_NO_THROW(ktpt->setPlayerIndex(10));
    EXPECT_NO_THROW(ktpt->setPlayerIndex(11));
    EXPECT_THROW(ktpt->setPlayerIndex(12), CTLib::KMPError);
    EXPECT_THROW(ktpt->setPlayerIndex(234), CTLib::KMPError);
}

TEST(KMPENPTTests, Errors)
{
    CTLib::KMP kmp;

    for (uint16_t i = 0; i < CTLib::KMP::ENPT::MAX_ENTRY_COUNT; ++i)
    {
        kmp.add<CTLib::KMP::ENPT>();
    }
    EXPECT_THROW(kmp.add<CTLib::KMP::ENPT>(), CTLib::KMPError);
}

TEST(KMPITPTTests, Errors)
{
    CTLib::KMP kmp;

    for (uint16_t i = 0; i < CTLib::KMP::ITPT::MAX_ENTRY_COUNT; ++i)
    {
        kmp.add<CTLib::KMP::ITPT>();
    }
    EXPECT_THROW(kmp.add<CTLib::KMP::ITPT>(), CTLib::KMPError);
}

TEST(KMPCKPTTests, RemoveJGPT)
{
    CTLib::KMP kmp;
    CTLib::KMP::CKPT* ckpt = kmp.add<CTLib::KMP::CKPT>();
    EXPECT_EQ(nullptr, ckpt->getRespawn());

    CTLib::KMP::JGPT* jgpt = kmp.add<CTLib::KMP::JGPT>();
    ckpt->setRespawn(jgpt);
    EXPECT_EQ(jgpt, ckpt->getRespawn());

    kmp.remove<CTLib::KMP::JGPT>(kmp.indexOf(jgpt));
    EXPECT_EQ(nullptr, ckpt->getRespawn());
}

TEST(KMPCKPTTests, Errors)
{
    CTLib::KMP kmp;

    for (uint16_t i = 0; i < CTLib::KMP::CKPT::MAX_ENTRY_COUNT; ++i)
    {
        kmp.add<CTLib::KMP::CKPT>();
    }
    EXPECT_THROW(kmp.add<CTLib::KMP::CKPT>(), CTLib::KMPError);
}

TEST(KMPCKPTTests, OtherKMPErrors)
{
    CTLib::KMP kmp;
    CTLib::KMP::CKPT* ckpt = kmp.add<CTLib::KMP::CKPT>();

    CTLib::KMP kmp2;
    CTLib::KMP::JGPT* jgpt = kmp2.add<CTLib::KMP::JGPT>();

    EXPECT_THROW(ckpt->setRespawn(jgpt), CTLib::KMPError);
}

TEST(KMPGOBJTests, RemovePOTI)
{
    CTLib::KMP kmp;
    CTLib::KMP::GOBJ* gobj = kmp.add<CTLib::KMP::GOBJ>();
    EXPECT_EQ(nullptr, gobj->getRoute());

    CTLib::KMP::POTI* poti = kmp.add<CTLib::KMP::POTI>();
    gobj->setRoute(poti);
    EXPECT_EQ(poti, gobj->getRoute());

    kmp.remove<CTLib::KMP::POTI>(kmp.indexOf(poti));
    EXPECT_EQ(nullptr, gobj->getRoute());
}

TEST(KMPGOBJTests, Errors)
{
    CTLib::KMP kmp;
    CTLib::KMP::GOBJ* gobj = kmp.add<CTLib::KMP::GOBJ>();

    for (uint8_t i = 0; i < CTLib::KMP::GOBJ::SETTINGS_COUNT; ++i)
    {
        EXPECT_NO_THROW(gobj->setSetting(i, 0));
    }
    for (uint8_t i = CTLib::KMP::GOBJ::SETTINGS_COUNT; i != 0; ++i)
    {
        EXPECT_THROW(gobj->setSetting(i, 0), CTLib::KMPError);
    }
}

TEST(KMPGOBJTests, OtherKMPErrors)
{
    CTLib::KMP kmp;
    CTLib::KMP kmp2;

    CTLib::KMP::GOBJ* gobj = kmp.add<CTLib::KMP::GOBJ>();
    CTLib::KMP::POTI* potiFromKMP2 = kmp2.add<CTLib::KMP::POTI>();
    EXPECT_THROW(gobj->setRoute(potiFromKMP2), CTLib::KMPError);
}

TEST(KMPPOTITests, Errors)
{
    CTLib::KMP kmp;
    CTLib::KMP::POTI* poti = kmp.add<CTLib::KMP::POTI>();

    EXPECT_THROW(poti->getPoint(0), CTLib::KMPError);
    EXPECT_THROW(poti->removePoint(0), CTLib::KMPError);

    poti->addPoint(CTLib::KMP::POTI::Point());
    EXPECT_THROW(poti->getPoint(1), CTLib::KMPError);
    EXPECT_THROW(poti->removePoint(1), CTLib::KMPError);
    EXPECT_NO_THROW(poti->getPoint(0));
    EXPECT_NO_THROW(poti->removePoint(0)); // point removed here
    EXPECT_THROW(poti->getPoint(0), CTLib::KMPError);
    EXPECT_THROW(poti->removePoint(0), CTLib::KMPError);

    for (uint16_t i = 0; i < 100; ++i)
    {
        poti->addPoint(CTLib::KMP::POTI::Point());
        EXPECT_NO_THROW(poti->getPoint(i));
        EXPECT_THROW(poti->getPoint(i + 1), CTLib::KMPError);
    }

    for (uint16_t i = 0; i < 100; ++i)
    {
        EXPECT_NO_THROW(poti->removePoint(0));
    }
    EXPECT_THROW(poti->removePoint(0), CTLib::KMPError);
}

TEST(KMPAREATests, RemoveCAMEAndPOTIAndENPT)
{
    CTLib::KMP kmp;
    CTLib::KMP::AREA* area = kmp.add<CTLib::KMP::AREA>();
    EXPECT_EQ(nullptr, area->getCamera());
    EXPECT_EQ(nullptr, area->getRoute());
    EXPECT_EQ(nullptr, area->getDestinationPoint());

    CTLib::KMP::CAME* came = kmp.add<CTLib::KMP::CAME>();
    area->setCamera(came);
    EXPECT_EQ(came, area->getCamera());
    EXPECT_EQ(nullptr, area->getRoute());
    EXPECT_EQ(nullptr, area->getDestinationPoint());

    CTLib::KMP::POTI* poti = kmp.add<CTLib::KMP::POTI>();
    CTLib::KMP::ENPT* enpt = kmp.add<CTLib::KMP::ENPT>();
    area->setRoute(poti);
    area->setDestinationPoint(enpt);
    EXPECT_EQ(came, area->getCamera());
    EXPECT_EQ(poti, area->getRoute());
    EXPECT_EQ(enpt, area->getDestinationPoint());

    kmp.remove<CTLib::KMP::POTI>(kmp.indexOf(poti));
    EXPECT_EQ(came, area->getCamera());
    EXPECT_EQ(nullptr, area->getRoute());
    EXPECT_EQ(enpt, area->getDestinationPoint());

    kmp.remove<CTLib::KMP::CAME>(kmp.indexOf(came));
    kmp.remove<CTLib::KMP::ENPT>(kmp.indexOf(enpt));
    EXPECT_EQ(nullptr, area->getCamera());
    EXPECT_EQ(nullptr, area->getRoute());
    EXPECT_EQ(nullptr, area->getDestinationPoint());
}

TEST(KMPAREATests, OtherKMPErrors)
{
    CTLib::KMP kmp;
    CTLib::KMP kmp2;

    CTLib::KMP::AREA* area = kmp.add<CTLib::KMP::AREA>();

    CTLib::KMP::CAME* cameFromKMP2 = kmp2.add<CTLib::KMP::CAME>();
    EXPECT_THROW(area->setCamera(cameFromKMP2), CTLib::KMPError);

    CTLib::KMP::POTI* potiFromKMP2 = kmp2.add<CTLib::KMP::POTI>();
    EXPECT_THROW(area->setRoute(potiFromKMP2), CTLib::KMPError);

    CTLib::KMP::ENPT* enptFromKMP2 = kmp2.add<CTLib::KMP::ENPT>();
    EXPECT_THROW(area->setDestinationPoint(enptFromKMP2), CTLib::KMPError);
}

TEST(KMPCAMETests, RemoveCAMEAndPOTI)
{
    CTLib::KMP kmp;
    CTLib::KMP::CAME* came = kmp.add<CTLib::KMP::CAME>();
    EXPECT_EQ(nullptr, came->getNext());
    EXPECT_EQ(nullptr, came->getRoute());

    CTLib::KMP::CAME* came2 = kmp.add<CTLib::KMP::CAME>();
    came->setNext(came2);
    EXPECT_EQ(came2, came->getNext());
    EXPECT_EQ(nullptr, came->getRoute());

    CTLib::KMP::POTI* poti = kmp.add<CTLib::KMP::POTI>();
    came->setRoute(poti);
    EXPECT_EQ(came2, came->getNext());
    EXPECT_EQ(poti, came->getRoute());

    kmp.remove<CTLib::KMP::CAME>(kmp.indexOf(came2));
    EXPECT_EQ(nullptr, came->getNext());
    EXPECT_EQ(poti, came->getRoute());

    kmp.remove<CTLib::KMP::POTI>(kmp.indexOf(poti));
    EXPECT_EQ(nullptr, came->getNext());
    EXPECT_EQ(nullptr, came->getRoute());
}

TEST(KMPCAMETests, OtherKMPErrors)
{
    CTLib::KMP kmp;
    CTLib::KMP kmp2;

    CTLib::KMP::CAME* came = kmp.add<CTLib::KMP::CAME>();

    CTLib::KMP::CAME* cameFromKMP2 = kmp2.add<CTLib::KMP::CAME>();
    EXPECT_THROW(came->setNext(cameFromKMP2), CTLib::KMPError);

    CTLib::KMP::POTI* potiFromKMP2 = kmp2.add<CTLib::KMP::POTI>();
    EXPECT_THROW(came->setRoute(potiFromKMP2), CTLib::KMPError);
}

TEST(KMPJGPTTests, Errors)
{
    CTLib::KMP kmp;

    for (uint16_t i = 0; i < CTLib::KMP::JGPT::MAX_ENTRY_COUNT; ++i)
    {
        kmp.add<CTLib::KMP::JGPT>();
    }
    EXPECT_THROW(kmp.add<CTLib::KMP::JGPT>(), CTLib::KMPError);
}

TEST(KMPCNPTTests, CannonType)
{
    CTLib::KMP kmp;
    CTLib::KMP::CNPT* cnpt = kmp.add<CTLib::KMP::CNPT>();
    EXPECT_EQ(CTLib::KMP::CNPT::CannonType::Default, cnpt->getCannonType());
    EXPECT_EQ(0, cnpt->getTypeID());

    cnpt->setCannonType(CTLib::KMP::CNPT::CannonType::Slow);
    EXPECT_EQ(CTLib::KMP::CNPT::CannonType::Slow, cnpt->getCannonType());
    EXPECT_EQ(2, cnpt->getTypeID());

    cnpt->setTypeID(1);
    EXPECT_EQ(CTLib::KMP::CNPT::CannonType::Curved, cnpt->getCannonType());
    EXPECT_EQ(1, cnpt->getTypeID());

    cnpt->setTypeID(4);
    EXPECT_EQ(CTLib::KMP::CNPT::CannonType::Custom, cnpt->getCannonType());
    EXPECT_EQ(4, cnpt->getTypeID());

    cnpt->setCannonType(CTLib::KMP::CNPT::CannonType::Custom);
    EXPECT_EQ(CTLib::KMP::CNPT::CannonType::Custom, cnpt->getCannonType());
    EXPECT_EQ(-1, cnpt->getTypeID());
}

TEST(KMPSTGITests, Errors)
{
    CTLib::KMP kmp;
    CTLib::KMP::STGI* stgi = kmp.add<CTLib::KMP::STGI>();
    EXPECT_THROW(kmp.add<CTLib::KMP::STGI>(), CTLib::KMPError);

    EXPECT_THROW(stgi->setLapCount(0), CTLib::KMPError);
    EXPECT_THROW(stgi->setLapCount(10), CTLib::KMPError);
    EXPECT_THROW(stgi->setLapCount(23), CTLib::KMPError);
    EXPECT_THROW(stgi->setLapCount(94), CTLib::KMPError);
    EXPECT_THROW(stgi->setLapCount(210), CTLib::KMPError);

    EXPECT_THROW(stgi->setSpeedFactor(0.f), CTLib::KMPError);
    EXPECT_THROW(stgi->setSpeedFactor(-0.f), CTLib::KMPError);
    EXPECT_THROW(stgi->setSpeedFactor(-1.f), CTLib::KMPError);
    EXPECT_THROW(stgi->setSpeedFactor(-35.f), CTLib::KMPError);
}
