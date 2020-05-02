//////////////////////////////////////////////////
//  Copyright (c) 2020 Nara Hiero
//
// This file is licensed under GPLv3+
// Refer to the `License.txt` file included.
//////////////////////////////////////////////////


/**************************************************************************
 * This header contains functionalities used both for reading and writing.
 * 
 * P.S., `RW` stands for 'R'ead/'W'rite
 **************************************************************************/


#include <map>
#include <string>
#include <vector>

#include <CTLib/BRRES.hpp>


namespace CTLib
{

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////
////   General utilities
////

uint32_t padNumber(uint32_t num, uint8_t pad);

uint32_t flipBits(uint32_t num, uint8_t lowI, uint8_t highI);

std::string readString(Buffer& in, uint32_t off);

struct BRRESStringTable
{

    // map containing string offsets in table
    std::map<std::string, uint32_t> offsets;

    // the binary data of the string table
    Buffer data;
};

void addToStringTable(BRRESStringTable* table, const std::string& str);


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////
////   BRRES index group
////

class BRRESIndexGroupEntry;

class BRRESIndexGroup final
{

    friend class BRRESIndexGroupEntry;

public:

    // returns the size in bytes of a group with the specified number of
    // entries, the root excluded
    static uint32_t getSizeInBytesForCount(uint16_t count);

    BRRESIndexGroup();
    BRRESIndexGroup(const BRRESIndexGroup&) = delete;
    BRRESIndexGroup(BRRESIndexGroup&&) = delete;
    ~BRRESIndexGroup();

    // creates and adds an entry with the specified name and returns it
    BRRESIndexGroupEntry* addEntry(const std::string& name);

    // returns the entries in this group
    std::vector<BRRESIndexGroupEntry*> getEntries() const;

    // returns the size in bytes of this group
    uint32_t getSizeInBytes() const;

    // returns the number of entries in this group minus the root
    uint32_t getEntryCount() const;

    // removes all existing entries and read the index group from the specified buffer
    void read(Buffer& in);

    // writes this index group to the specified buffer
    void write(Buffer& out, BRRESStringTable* table, uint32_t tableOff) const;

private:

    void deleteAll();

    // vector containing all entries in this index group
    std::vector<BRRESIndexGroupEntry*> entries;
};

class BRRESIndexGroupEntry
{
    
    friend class BRRESIndexGroup;

public:

    BRRESIndexGroupEntry(const BRRESIndexGroupEntry&) = delete;
    BRRESIndexGroupEntry(BRRESIndexGroupEntry&&) = delete;
    ~BRRESIndexGroupEntry();

    // returns the id of this entry
    uint16_t getId() const;

    // returns the index of this entry
    uint16_t getIndex() const;

    // returns the right entry of this entry
    BRRESIndexGroupEntry* getRight() const;

    // returns the left entry of this entry
    BRRESIndexGroupEntry* getLeft() const;

    // returns the name of this entry
    std::string getName() const;

    void setDataOffset(uint32_t off);

    uint32_t getDataOff() const;

private:

    BRRESIndexGroupEntry(BRRESIndexGroup* group, const std::string& name);

    BRRESIndexGroupEntry(BRRESIndexGroup* group);

    // calculate the original id of this entry
    void calculateId();

    // recalculates the id based on the specified object entry
    void recalculateId(BRRESIndexGroupEntry* entry);

    // returns whether this entry is right based on the specified entry
    bool isRightOf(BRRESIndexGroupEntry* entry);

    // inserts the specified entry
    void insert(BRRESIndexGroupEntry* entry);

    // pointer to the group this entry is part of
    BRRESIndexGroup* group;

    // the id of this entry
    uint16_t id;

    // the index of entry
    uint16_t idx;

    // the right link of this entry
    BRRESIndexGroupEntry* right;

    // the left link of this entry
    BRRESIndexGroupEntry* left;

    // the name of this entry
    std::string name;

    // the offset to data of this entry
    uint32_t dataOff;
};


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////
////   Function prototypes
////

/// MDL0 ///////////////////////////////

void addMDL0StringsToTable(BRRESStringTable* table, MDL0* mdl0);

uint32_t calculateMDL0Size(MDL0* mdl0);

void writeMDL0(
    Buffer& out, MDL0* mdl0, int32_t offToBRRES, BRRESStringTable* table, uint32_t tableOff
);

/// TEX0 ///////////////////////////////

void addTEX0StringsToTable(BRRESStringTable* table, TEX0* mdl0);

uint32_t calculateTEX0Size(TEX0* tex0);

void writeTEX0(
    Buffer& out, TEX0* tex0, int32_t offToBRRES, BRRESStringTable* table, uint32_t tableOff
);
}
