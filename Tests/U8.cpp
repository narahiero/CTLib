//////////////////////////////////////////////////
//  Copyright (c) 2020 Nara Hiero
//
// This file is licensed under GPLv3+
// Refer to the `License.txt` file included.
//////////////////////////////////////////////////

#include <gtest/gtest.h>

#include <CTLib/U8.hpp>

using namespace CTLib;

TEST(U8ArcTests, Count)
{
    U8Arc arc;
    EXPECT_EQ(0, arc.totalCount());
    EXPECT_EQ(0, arc.count());

    U8Dir* root = arc.addDirectory(".");
    EXPECT_EQ(1, arc.totalCount());
    EXPECT_EQ(1, arc.count());
    EXPECT_EQ(0, root->count());

    root->addFile("course_model.brres");
    root->addFile("course.kcl");
    root->addFile("course.kmp");
    EXPECT_EQ(4, arc.totalCount());
    EXPECT_EQ(1, arc.count());
    EXPECT_EQ(3, root->count());

    U8Dir* posteffect = root->addDirectory("posteffect");
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
    U8Arc arc;
    EXPECT_THROW(arc.addFile(""), U8Error);
    EXPECT_THROW(arc.addDirectory("contains/slash"), U8Error);
}

TEST(U8ArcTests, GetParent)
{
    U8Arc arc;
    U8Dir* root = arc.addDirectory(".");
    U8File* file = root->addFile("course_model.brres");
    EXPECT_EQ(root, file->getParent());

    U8Dir* posteffect = root->addDirectory("posteffect");
    EXPECT_EQ(root, posteffect->getParent());

    U8File* blight = posteffect->addFile("posteffect.blight");
    EXPECT_EQ(posteffect, blight->getParent());
}

TEST(U8ArcTests, GetEntry)
{
    U8Arc arc;
    U8Dir* root = arc.addDirectory(".");
    root->addFile("course_model.brres");
    root->addFile("course.kcl");
    root->addFile("course.kmp");
    root->addDirectory("posteffect");

    U8Entry* entry = root->getEntry("course.kcl");
    EXPECT_EQ(U8EntryType::File, entry->getType());
    EXPECT_EQ("course.kcl", entry->getName());
}

TEST(U8ArcTests, RangeBasedLoop)
{
    U8Arc arc;
    U8Dir* root = arc.addDirectory(".");
    root->addFile("course.kcl");
    root->addFile("course.kmp");
    root->addFile("course_model.brres");
    root->addDirectory("posteffect");

    const std::string expect[] = {
        ".", "course.kcl", "course.kmp", "course_model.brres", "posteffect"
    };
    const U8EntryType types[] = {
        U8EntryType::Directory, U8EntryType::File, U8EntryType::File,
        U8EntryType::File, U8EntryType::Directory
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
    U8Arc arc;
    U8Dir* root = arc.addDirectory(".");

    U8File* file = root->addFile("course_model.brres");
    EXPECT_TRUE(root->hasEntry("course_model.brres"));

    file->rename("course_d_model.brres");
    EXPECT_FALSE(root->hasEntry("course_model.brres"));
    EXPECT_TRUE(root->hasEntry("course_d_model.brres"));

    root->addFile("vrcorn_model.brres");
    EXPECT_THROW(file->rename("vrcorn_model.brres"), U8Error);

    EXPECT_THROW(file->rename(""), U8Error);
}

TEST(U8ArcTests, AbsoluteGet)
{
    U8Arc arc;
    U8Dir* root = arc.addDirectory(".");
    root->addDirectory("posteffect")->addFile("posteffect.blight");

    U8Entry* entry = arc.getEntryAbsolute("./posteffect/posteffect.blight");
    ASSERT_TRUE(entry != nullptr);
    EXPECT_EQ("posteffect.blight", entry->getName());
    EXPECT_EQ("posteffect", entry->getParent()->getName());
    EXPECT_EQ(U8EntryType::File, entry->getType());

    U8Entry* nothing = arc.getEntryAbsolute("./some/file.txt");
    EXPECT_EQ(nullptr, nothing);

    U8Dir* dossun = root->addDirectory("effect")->addDirectory("dossun");
    dossun->addFile("rk_dossun.breff");
    dossun->addFile("rk_dossun.breft");
    
    U8Entry* dir = arc.getEntryAbsolute("./effect/dossun");
    ASSERT_TRUE(dir != nullptr);
    EXPECT_EQ(dossun, dir);
}

TEST(U8ArcTests, AbsoluteAdd)
{
    U8Arc arc;
    U8Dir* posteffect = arc.addDirectoryAbsolute("./posteffect");
    EXPECT_EQ(2, arc.totalCount());
    EXPECT_EQ(1, arc.count());
    EXPECT_EQ("posteffect", posteffect->getName());
    EXPECT_EQ(".", posteffect->getParent()->getName());
    EXPECT_EQ(U8EntryType::Directory, posteffect->getType());

    U8Dir* root = arc.getEntry(".")->asDirectory();
    U8File* model = arc.addFileAbsolute("./course_model.brres");
    EXPECT_EQ(3, arc.totalCount());
    EXPECT_EQ(1, arc.count());
    EXPECT_EQ(2, root->count());
    EXPECT_EQ("course_model.brres", model->getName());
    EXPECT_EQ(root, model->getParent());
    EXPECT_EQ(U8EntryType::File, model->getType());

    U8File* file = arc.addFileAbsolute("./effect/dossun/rk_dossun.breft");
    EXPECT_EQ(6, arc.totalCount());
    EXPECT_EQ(1, arc.count());
    EXPECT_EQ(3, root->count());
    EXPECT_EQ("rk_dossun.breft", file->getName());
    EXPECT_EQ("dossun", file->getParent()->getName());
    EXPECT_EQ(U8EntryType::File, file->getType());

    // entry already exists
    EXPECT_THROW(arc.addFileAbsolute("./effect/dossun/rk_dossun.breft"), U8Error);

    // parent directory with invalid name
    EXPECT_THROW(arc.addDirectoryAbsolute("./posteffect//posteffect.blight"), U8Error);

    // parent directory is an already existing file
    EXPECT_THROW(arc.addFileAbsolute("./course_model.brres/model.mdl0"), U8Error);
}
