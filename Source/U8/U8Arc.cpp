//////////////////////////////////////////////////
//  Copyright (c) 2020 Nara Hiero
//
// This file is licensed under GPLv3+
// Refer to the `License.txt` file included.
//////////////////////////////////////////////////

#include <CTLib/U8.hpp>

#include <CTLib/Utilities.hpp>

namespace CTLib
{

//////////////////////////////
///  class U8Arc

U8Arc::U8Arc() :
    entries{},
    root{}
{
    root = new U8Dir(this);
}

U8Arc::U8Arc(U8Arc&& src) :
    entries{std::move(src.entries)},
    root{src.root}
{
    for (U8Entry* entry : entries)
    {
        entry->arc = this;
    }
    src.root = new U8Dir(this);
}

U8Arc::~U8Arc()
{
    for (U8Entry* entry : entries)
    {
        delete entry;
    }
}

U8Dir* U8Arc::addDirectory(const std::string& name)
{
    return root->addDirectory(name);
}

U8File* U8Arc::addFile(const std::string& name)
{
    return root->addFile(name);
}

size_t U8Arc::count() const
{
    return entries.size() - 1;
}

U8Arc::Iterator U8Arc::begin()
{
    return ++entries.begin(); // skip the first 'root' entry
}

U8Arc::Iterator U8Arc::end()
{
    return entries.end();
}


//////////////////////////////
///  class U8Entry

U8Entry::U8Entry(U8Arc* arc) :
    arc{arc},
    parent{nullptr},
    name{}
{
    arc->entries.push_back(this);
}

U8Entry::U8Entry(U8Arc* arc, U8Dir* parent, const std::string& name) :
    arc{arc},
    parent{parent},
    name{name}
{
    assertValidName(name);

    arc->entries.push_back(this);
    parent->entries.insert(std::map<std::string, U8Entry*>::value_type(name, this));
}

U8Entry::~U8Entry() = default;

void U8Entry::rename(const std::string& name)
{
    assertValidName(name);
    parent->assertUniqueName(name);

    parent->entries.erase(this->name);
    this->name = name;
    parent->entries.insert(std::map<std::string, U8Entry*>::value_type(name, this));
}

U8Dir* U8Entry::getParent() const
{
    return parent;
}

std::string U8Entry::getName() const
{
    return name;
}

void U8Entry::assertValidName(const std::string& name) const
{
    if (name.empty())
    {
        throw U8Error("Invalid entry name: Name is empty!");
    }
    if (name.find('/') != std::string::npos)
    {
        throw U8Error("Invalid entry name: Name contains slash '/' character!");
    }
}


//////////////////////////////
///  class U8Dir

U8Dir::U8Dir(U8Arc* arc) :
    U8Entry{arc}
{

}

U8Dir::U8Dir(U8Arc* arc, U8Dir* parent, const std::string& name) :
    U8Entry{arc, parent, name}
{

}

U8Dir::~U8Dir() = default;

U8Dir* U8Dir::addDirectory(const std::string& name)
{
    assertUniqueName(name);

    return new U8Dir(arc, this, name);
}

U8File* U8Dir::addFile(const std::string& name)
{
    assertUniqueName(name);

    return new U8File(arc, this, name);
}

size_t U8Dir::count() const
{
    return entries.size();
}

U8Entry* U8Dir::getEntry(const std::string& name) const
{
    return entries.count(name) > 0 ? entries.at(name) : nullptr;
}

bool U8Dir::hasEntry(const std::string& name) const
{
    return entries.count(name) > 0;
}

U8EntryType U8Dir::getType() const
{
    return U8EntryType::Directory;
}

U8File* U8Dir::asFile() const
{
    throw U8Error("This entry is a directory, not a file!");
}

U8Dir* U8Dir::asDirectory() const
{
    return const_cast<U8Dir*>(this);
}

U8Dir::Iterator U8Dir::begin()
{
    return Iterator(entries.begin());
}

U8Dir::Iterator U8Dir::end()
{
    return Iterator(entries.end());
}

void U8Dir::assertUniqueName(const std::string& name)
{
    if (entries.count(name) > 0)
    {
        throw U8Error(Strings::format(
            "The directory '%s' already has an entry with name '%s'!",
            this->name.c_str(), name.c_str()
        ));
    }
}


//////////////////////////////
///  class U8File

U8File::U8File(U8Arc* arc, U8Dir* parent, const std::string& name) :
    U8Entry{arc, parent, name}
{

}

U8File::~U8File() = default;

U8EntryType U8File::getType() const
{
    return U8EntryType::File;
}

U8File* U8File::asFile() const
{
    return const_cast<U8File*>(this);
}

U8Dir* U8File::asDirectory() const
{
    throw U8Error("This entry is a file, not a directory!");
}
}
