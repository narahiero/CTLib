//////////////////////////////////////////////////
//  Copyright (c) 2020 Nara Hiero
//
// This file is licensed under GPLv3+
// Refer to the `License.txt` file included.
//////////////////////////////////////////////////

#include <gtest/gtest.h>

#include <CTLib/U8.hpp>

TEST(U8ArcTests, Count)
{
    CTLib::U8Arc arc;
    EXPECT_EQ(0, arc.totalCount());
    EXPECT_EQ(0, arc.count());

    CTLib::U8Dir* root = arc.addDirectory(".");
    EXPECT_EQ(1, arc.totalCount());
    EXPECT_EQ(1, arc.count());
    EXPECT_EQ(0, root->count());

    root->addFile("course_model.brres");
    root->addFile("course.kcl");
    root->addFile("course.kmp");
    EXPECT_EQ(4, arc.totalCount());
    EXPECT_EQ(1, arc.count());
    EXPECT_EQ(3, root->count());

    CTLib::U8Dir* posteffect = root->addDirectory("posteffect");
    EXPECT_EQ(5, arc.totalCount());
    EXPECT_EQ(1, arc.count());
    EXPECT_EQ(4, root->count());
    EXPECT_EQ(0, posteffect->count());

    posteffect->addFile("posteffect.blight");
    EXPECT_EQ(6, arc.totalCount());
    EXPECT_EQ(1, arc.count());
    EXPECT_EQ(4, root->count());
    EXPECT_EQ(1, posteffect->count());
}

TEST(U8ArcTests, InvalidNames)
{
    CTLib::U8Arc arc;
    EXPECT_THROW(arc.addFile(""), CTLib::U8Error);
    EXPECT_THROW(arc.addDirectory("contains/slash"), CTLib::U8Error);
}

TEST(U8ArcTests, GetParent)
{
    CTLib::U8Arc arc;
    CTLib::U8Dir* root = arc.addDirectory(".");
    CTLib::U8File* file = root->addFile("course_model.brres");
    EXPECT_EQ(root, file->getParent());

    CTLib::U8Dir* posteffect = root->addDirectory("posteffect");
    EXPECT_EQ(root, posteffect->getParent());

    CTLib::U8File* blight = posteffect->addFile("posteffect.blight");
    EXPECT_EQ(posteffect, blight->getParent());
}

TEST(U8ArcTests, GetEntry)
{
    CTLib::U8Arc arc;
    CTLib::U8Dir* root = arc.addDirectory(".");
    root->addFile("course_model.brres");
    root->addFile("course.kcl");
    root->addFile("course.kmp");
    root->addDirectory("posteffect");

    CTLib::U8Entry* entry = root->getEntry("course.kcl");
    EXPECT_EQ(CTLib::U8EntryType::File, entry->getType());
    EXPECT_EQ("course.kcl", entry->getName());
}

TEST(U8ArcTests, RangeBasedLoop)
{
    CTLib::U8Arc arc;
    CTLib::U8Dir* root = arc.addDirectory(".");
    root->addFile("course.kcl");
    root->addFile("course.kmp");
    root->addFile("course_model.brres");
    root->addDirectory("posteffect");

    const std::string expect[] = {
        ".", "course.kcl", "course.kmp", "course_model.brres", "posteffect"
    };
    const CTLib::U8EntryType types[] = {
        CTLib::U8EntryType::Directory, CTLib::U8EntryType::File, CTLib::U8EntryType::File,
        CTLib::U8EntryType::File, CTLib::U8EntryType::Directory
    };

    size_t i = 0;
    for (auto entry : arc)
    {
        EXPECT_EQ(expect[i], entry->getName());
        EXPECT_EQ(types[i], entry->getType());
        ++i;
    }
    EXPECT_EQ(5, i); // expect 5 entries in archive

    i = 0;
    for (auto entry : *root)
    {
        EXPECT_EQ((expect + 1)[i], entry->getName());
        EXPECT_EQ((types + 1)[i], entry->getType());
        ++i;
    }
    EXPECT_EQ(4, i); // expect 4 entries in directory
}

TEST(U8ArcTests, Rename)
{
    CTLib::U8Arc arc;
    CTLib::U8Dir* root = arc.addDirectory(".");

    CTLib::U8File* file = root->addFile("course_model.brres");
    EXPECT_TRUE(root->hasEntry("course_model.brres"));

    file->rename("course_d_model.brres");
    EXPECT_FALSE(root->hasEntry("course_model.brres"));
    EXPECT_TRUE(root->hasEntry("course_d_model.brres"));

    root->addFile("vrcorn_model.brres");
    EXPECT_THROW(file->rename("vrcorn_model.brres"), CTLib::U8Error);

    EXPECT_THROW(file->rename(""), CTLib::U8Error);
}

TEST(U8ArcTests, AbsoluteGet)
{
    CTLib::U8Arc arc;
    CTLib::U8Dir* root = arc.addDirectory(".");
    root->addDirectory("posteffect")->addFile("posteffect.blight");

    CTLib::U8Entry* entry = arc.getEntryAbsolute("./posteffect/posteffect.blight");
    ASSERT_TRUE(entry != nullptr);
    EXPECT_EQ("posteffect.blight", entry->getName());
    EXPECT_EQ("posteffect", entry->getParent()->getName());
    EXPECT_EQ(CTLib::U8EntryType::File, entry->getType());

    CTLib::U8Entry* nothing = arc.getEntryAbsolute("./some/file.txt");
    EXPECT_EQ(nullptr, nothing);

    CTLib::U8Dir* dossun = root->addDirectory("effect")->addDirectory("dossun");
    dossun->addFile("rk_dossun.breff");
    dossun->addFile("rk_dossun.breft");
    
    CTLib::U8Entry* dir = arc.getEntryAbsolute("./effect/dossun");
    ASSERT_TRUE(dir != nullptr);
    EXPECT_EQ(dossun, dir);
}

TEST(U8ArcTests, AbsoluteAdd)
{
    CTLib::U8Arc arc;
    CTLib::U8Dir* posteffect = arc.addDirectoryAbsolute("./posteffect");
    EXPECT_EQ(2, arc.totalCount());
    EXPECT_EQ(1, arc.count());
    EXPECT_EQ("posteffect", posteffect->getName());
    EXPECT_EQ(".", posteffect->getParent()->getName());
    EXPECT_EQ(CTLib::U8EntryType::Directory, posteffect->getType());

    CTLib::U8Dir* root = arc.getEntry(".")->asDirectory();
    CTLib::U8File* model = arc.addFileAbsolute("./course_model.brres");
    EXPECT_EQ(3, arc.totalCount());
    EXPECT_EQ(1, arc.count());
    EXPECT_EQ(2, root->count());
    EXPECT_EQ("course_model.brres", model->getName());
    EXPECT_EQ(root, model->getParent());
    EXPECT_EQ(CTLib::U8EntryType::File, model->getType());

    CTLib::U8File* file = arc.addFileAbsolute("./effect/dossun/rk_dossun.breft");
    EXPECT_EQ(6, arc.totalCount());
    EXPECT_EQ(1, arc.count());
    EXPECT_EQ(3, root->count());
    EXPECT_EQ("rk_dossun.breft", file->getName());
    EXPECT_EQ("dossun", file->getParent()->getName());
    EXPECT_EQ(CTLib::U8EntryType::File, file->getType());
}
