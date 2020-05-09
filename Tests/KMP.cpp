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

    for (uint16_t i = 0; i < CTLib::KMP::ENPT::MAX_ENPT_ENTRY_COUNT; ++i)
    {
        kmp.add<CTLib::KMP::ENPT>();
    }
    EXPECT_THROW(kmp.add<CTLib::KMP::ENPT>(), CTLib::KMPError);
}

TEST(KMPENPHTests, FirstAndLast)
{
    CTLib::KMP kmp;
    CTLib::KMP::ENPH* enph = kmp.add<CTLib::KMP::ENPH>();
    EXPECT_EQ(nullptr, enph->getFirst());
    EXPECT_EQ(nullptr, enph->getLast());

    CTLib::KMP::ENPT* enpt0 = kmp.add<CTLib::KMP::ENPT>();
    CTLib::KMP::ENPT* enpt1 = kmp.add<CTLib::KMP::ENPT>();
    CTLib::KMP::ENPT* enpt2 = kmp.add<CTLib::KMP::ENPT>();
    CTLib::KMP::ENPT* enpt3 = kmp.add<CTLib::KMP::ENPT>();
    CTLib::KMP::ENPT* enpt4 = kmp.add<CTLib::KMP::ENPT>();
    CTLib::KMP::ENPT* enpt5 = kmp.add<CTLib::KMP::ENPT>();
    CTLib::KMP::ENPT* enpt6 = kmp.add<CTLib::KMP::ENPT>();
    
    enph->setFirst(enpt2);
    EXPECT_EQ(enpt2, enph->getFirst());
    EXPECT_EQ(nullptr, enph->getLast());

    enph->setLast(enpt5);
    EXPECT_EQ(enpt2, enph->getFirst());
    EXPECT_EQ(enpt5, enph->getLast());

    kmp.remove<CTLib::KMP::ENPT>(kmp.indexOf(enpt2));
    EXPECT_EQ(nullptr, enph->getFirst());
    EXPECT_EQ(enpt5, enph->getLast());

    enph->setFirst(enpt0);
    kmp.remove<CTLib::KMP::ENPT>(kmp.indexOf(enpt5));
    EXPECT_EQ(enpt0, enph->getFirst());
    EXPECT_EQ(nullptr, enph->getLast());

    kmp.remove<CTLib::KMP::ENPT>(kmp.indexOf(enpt0));
    EXPECT_EQ(nullptr, enph->getFirst());
    EXPECT_EQ(nullptr, enph->getLast());
}

TEST(KMPENPHTests, PreviousAndNext)
{
    CTLib::KMP kmp;
    CTLib::KMP::ENPH* enph = kmp.add<CTLib::KMP::ENPH>();
    EXPECT_EQ(0, enph->getPreviousCount());
    EXPECT_EQ(0, enph->getNextCount());

    CTLib::KMP::ENPH* enph2 = kmp.add<CTLib::KMP::ENPH>();
    enph->addNext(enph2);
    enph2->addPrevious(enph);
    EXPECT_EQ(0, enph->getPreviousCount());
    EXPECT_EQ(1, enph->getNextCount());
    EXPECT_EQ(1, enph2->getPreviousCount());
    EXPECT_EQ(0, enph2->getNextCount());

    kmp.remove<CTLib::KMP::ENPH>(kmp.indexOf(enph2));
    EXPECT_EQ(0, enph->getPreviousCount());
    EXPECT_EQ(0, enph->getNextCount());
}

TEST(KMPENPHTests, Errors)
{
    CTLib::KMP kmp;
    CTLib::KMP::ENPH* enph = kmp.add<CTLib::KMP::ENPH>();

    EXPECT_THROW(enph->addNext(nullptr), CTLib::KMPError);
    EXPECT_THROW(enph->addPrevious(nullptr), CTLib::KMPError);
    EXPECT_THROW(enph->removeNext(nullptr), CTLib::KMPError);
    EXPECT_THROW(enph->removePrevious(nullptr), CTLib::KMPError);

    EXPECT_THROW(enph->addNext(enph), CTLib::KMPError);
    EXPECT_THROW(enph->addPrevious(enph), CTLib::KMPError);
    EXPECT_THROW(enph->removeNext(enph), CTLib::KMPError);
    EXPECT_THROW(enph->removePrevious(enph), CTLib::KMPError);

    CTLib::KMP::ENPH* enph2 = kmp.add<CTLib::KMP::ENPH>();
    EXPECT_THROW(enph->removeNext(enph2), CTLib::KMPError);
    EXPECT_THROW(enph->removePrevious(enph2), CTLib::KMPError);

    enph->addPrevious(enph2);
    EXPECT_THROW(enph->removeNext(enph2), CTLib::KMPError);
    EXPECT_NO_THROW(enph->removePrevious(enph2));
    EXPECT_THROW(enph->removePrevious(enph2), CTLib::KMPError);

    for (uint8_t i = 0; i < CTLib::KMP::ENPH::MAX_ENPH_LINKS; ++i)
    {
        enph->addNext(enph2);
    }
    EXPECT_THROW(enph->addNext(enph2), CTLib::KMPError);

    for (uint8_t i = 0; i < CTLib::KMP::ENPH::MAX_ENPH_LINKS; ++i)
    {
        enph->addPrevious(enph2);
    }
    EXPECT_THROW(enph->addPrevious(enph2), CTLib::KMPError);
}

TEST(KMPENPHTests, OtherKMPErrors)
{
    CTLib::KMP kmp;
    CTLib::KMP kmp2;

    CTLib::KMP::ENPH* enph = kmp.add<CTLib::KMP::ENPH>();
    CTLib::KMP::ENPT* enptFromKMP2 = kmp2.add<CTLib::KMP::ENPT>();
    EXPECT_THROW(enph->setFirst(enptFromKMP2), CTLib::KMPError);
    EXPECT_THROW(enph->setLast(enptFromKMP2), CTLib::KMPError);

    CTLib::KMP::ENPH* enphFromKMP2 = kmp2.add<CTLib::KMP::ENPH>();
    EXPECT_THROW(enph->addNext(enphFromKMP2), CTLib::KMPError);
    EXPECT_THROW(enph->addPrevious(enphFromKMP2), CTLib::KMPError);
    EXPECT_THROW(enphFromKMP2->addNext(enph), CTLib::KMPError);
    EXPECT_THROW(enphFromKMP2->addPrevious(enph), CTLib::KMPError);
}
