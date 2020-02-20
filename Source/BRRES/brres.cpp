#include <CTLib/brres.hpp>

namespace ctlib {

BRRESIndexGroup::BRRESIndexGroup()
        : _count{0}, _entries{BRRESIndexGroupEntry{this}} {
}

BRRESIndexGroup::BRRESIndexGroup(const BRRESIndexGroup& src)
        : _count{src._count}, _entries{src._count + 1} {
    for (uint16_t i = 0; i < _count + 1; i++) {
        _entries[i] = src._entries[i];
        _entries[i]._group = this; // set the owner to this instead of src
    }
}

BRRESIndexGroup::BRRESIndexGroup(BRRESIndexGroup&& src)
        : _count{src._count}, _entries{std::move(src._entries)} {
    for (auto entry : _entries)
        entry._group = this; // set the owner to this instead of src
}

BRRESIndexGroupEntry::BRRESIndexGroupEntry(BRRESIndexGroup* owner)
        : _group{owner}, _id{0xFFFF}, _idx{0}, _l_idx{0}, _r_idx{0} {
}

}
