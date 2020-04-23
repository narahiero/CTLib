//////////////////////////////////////////////////
//  Copyright (c) 2020 Nara Hiero
//
// This file is licensed under GPLv3+
// Refer to the `License.txt` file included.
//////////////////////////////////////////////////

#include "BRRES/RW/BRRESRWCommon.hpp"

#include <CTLib/Utilities.hpp>
#include <iostream>
namespace CTLib
{

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////
////   General utilities
////

uint32_t padNumber(uint32_t num, uint8_t pad)
{
    return (num & ~(pad - 1)) + ((num & (pad - 1)) > 0 ? pad : 0);
}

std::string readBRRESString(Buffer& in, uint32_t off)
{
    if (off > in.capacity())
    {
        throw BRRESError(Strings::format(
            "String offset out of bounds! (%d)", off
        ));
    }

    if (off < 4)
    {
        throw BRRESError(Strings::format(
            "Invalid string offset: missing size! (%d)", off
        ));
    }

    uint32_t size = in.getInt(off - 4);
    if (off + size > in.capacity())
    {
        throw BRRESError(Strings::format(
            "String overflows bounds! (%d)", off + size
        ));
    }

    std::string str((const char*)*in + off);
    if (str.size() != size)
    {
        throw BRRESError(Strings::format(
            "String size is not matching! (%d != %d)", size, str.size()
        ));
    }

    return str;
}

void addToStringTable(BRRESStringTable* table, const std::string& str)
{
    uint32_t strSize = padNumber(static_cast<uint32_t>(str.size()) + 1, 0x4);

    if (table->data.remaining() < strSize + 4)
    {
        Buffer old = table->data.duplicate();
        uint32_t oldC = static_cast<uint32_t>(old.capacity());
        uint32_t newSize = (oldC < strSize + 4) ? (oldC + strSize + 4) : (oldC << 1);

        table->data = Buffer(newSize);
        table->data.put(old.flip());
    }

    // put size first for correct offset
    table->data.putInt(static_cast<uint32_t>(str.size()));

    table->offsets.insert(std::map<std::string, uint32_t>::value_type(
        str, static_cast<uint32_t>(table->data.position())
    ));
    table->data.putArray((uint8_t*)str.data(), str.size());

    uint32_t padding = strSize - static_cast<uint32_t>(str.size());
    while (padding-- > 0)
    {
        table->data.put(0);
    }
}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////
////   BRRES index group
////

BRRESIndexGroup::BRRESIndexGroup() :
    entries{}
{
    // create root entry
    entries.push_back(new BRRESIndexGroupEntry(this, ""));
}

BRRESIndexGroup::~BRRESIndexGroup()
{
    deleteAll();
}

BRRESIndexGroupEntry* BRRESIndexGroup::addEntry(const std::string& name)
{
    BRRESIndexGroupEntry* entry = new BRRESIndexGroupEntry(this, name);
    entries.push_back(entry);

    BRRESIndexGroupEntry* prev = entries[0];
    BRRESIndexGroupEntry* curr = prev->left;

    while (entry->id <= curr->id && curr->id < prev->id)
    {
        if (entry->id == curr->id)
        {
            entry->recalculateId(curr);
        }
        prev = curr;
        curr = entry->isRightOf(curr) ? curr->right : curr->left;
    }

    prev->insert(entry);

    return entry;
}

std::vector<BRRESIndexGroupEntry*> BRRESIndexGroup::getEntries() const
{
    return entries;
}

uint32_t BRRESIndexGroup::getSizeInBytes() const
{
    return static_cast<uint32_t>(entries.size() * 0x10) + 0x8;
}

uint32_t BRRESIndexGroup::getEntryCount() const
{
    return static_cast<uint32_t>(entries.size()) - 1;
}

void BRRESIndexGroup::read(Buffer& in)
{
    deleteAll();

    in.getInt(); // ignore size in bytes
    uint32_t count = in.getInt();

    // left and right indices
    std::vector<std::pair<uint16_t, uint16_t>> indices;

    for (uint32_t i = 0; i < count + 1; ++i)
    {
        BRRESIndexGroupEntry* entry = new BRRESIndexGroupEntry(this);
        entries.push_back(entry);

        entry->id = in.getShort();
        in.getShort(); // ignore unknown/unused value

        indices.push_back(std::make_pair(in.getShort(), in.getShort()));

        readBRRESString(in, in.getInt());

        entry->dataOff = in.getInt();
    }

    for (size_t i = 0; i < entries.size(); ++i)
    {
        BRRESIndexGroupEntry* entry = entries[i];
        entry->left = entries[indices[i].first];
        entry->right = entries[indices[i].second];
    }
}

void BRRESIndexGroup::write(Buffer& out, BRRESStringTable* table, uint32_t tableOff) const
{
    out.putInt(getSizeInBytes()); // size in bytes
    out.putInt(static_cast<uint32_t>(entries.size()) - 1); // number in group

    for (BRRESIndexGroupEntry* entry : entries)
    {
        out.putShort(entry->id); // entry id
        out.putShort(0x0000); // unknown; probably unused
        out.putShort(entry->left->idx); // left index
        out.putShort(entry->right->idx); // right index
        out.putInt(entry->idx == 0 ? 0 : table->offsets.at(entry->name) + tableOff); // name offset
        out.putInt(entry->dataOff); // data offset
    }
}

void BRRESIndexGroup::deleteAll()
{
    for (BRRESIndexGroupEntry* entry : entries)
    {
        delete entry;
    }
    entries.clear();
}

BRRESIndexGroupEntry::BRRESIndexGroupEntry(BRRESIndexGroup* group, const std::string& name) :
    group{group},
    idx{static_cast<uint16_t>(group->entries.size())},
    right{this},
    left{this},
    name{name},
    dataOff{0}
{
    calculateId();
}

BRRESIndexGroupEntry::BRRESIndexGroupEntry(BRRESIndexGroup* group) :
    group{group},
    idx{static_cast<uint16_t>(group->entries.size())}
{

}

BRRESIndexGroupEntry::~BRRESIndexGroupEntry()
{

}

uint16_t BRRESIndexGroupEntry::getId() const
{
    return id;
}

uint16_t BRRESIndexGroupEntry::getIndex() const
{
    return idx;
}

BRRESIndexGroupEntry* BRRESIndexGroupEntry::getRight() const
{
    return right;
}

BRRESIndexGroupEntry* BRRESIndexGroupEntry::getLeft() const
{
    return left;
}

std::string BRRESIndexGroupEntry::getName() const
{
    return name;
}

void BRRESIndexGroupEntry::setDataOffset(uint32_t off)
{
    dataOff = off;
}

uint32_t BRRESIndexGroupEntry::getDataOff() const
{
    return dataOff;
}

uint8_t highestBit(uint8_t val)
{
    for (uint8_t i = 7; i >= 0; --i)
    {
        if (val >> i)
        {
            return i;
        }
    }
    return ~0; // null character was passed; should NEVER execute
}

void BRRESIndexGroupEntry::calculateId()
{
    const int32_t size = static_cast<uint16_t>(name.size()) - 1;
    id = size < 0 ? ~0Ui16 : (size << 3) | highestBit(name[size]);
}

void BRRESIndexGroupEntry::recalculateId(BRRESIndexGroupEntry* object)
{
    for (int32_t i = static_cast<int32_t>(name.size()) - 1; i >= 0; ++i)
    {
        uint8_t cmp = name[i] ^ object->name[i];
        if (cmp)
        {
            id = (i << 3) | highestBit(cmp);
        }
    }

    if (object->isRightOf(this))
    {
        left = this;
        right = object;
    }
    else
    {
        left = this;
        right = object;
    }
}

bool BRRESIndexGroupEntry::isRightOf(BRRESIndexGroupEntry* entry)
{
    uint16_t cidx = entry->id >> 3;
    return cidx < name.size() && (name[cidx] >> (entry->id & 0x7)) & 1;
}

void BRRESIndexGroupEntry::insert(BRRESIndexGroupEntry* entry)
{
    if (entry->isRightOf(this))
    {
        (right->isRightOf(entry) ? entry->right : entry->left) = right;
        right = entry;
    }
    else
    {
        (left->isRightOf(entry) ? entry->right : entry->left) = left;
        left = entry;
    }
}
}
