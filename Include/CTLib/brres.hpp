#pragma once

/*! @file brres.hpp
 * 
 * @brief The header for the BRRES module.
 */

#include <cstdint>
#include <vector>

namespace ctlib {

/*! @defgroup brres BRRES
 * @addtogroup brres
 * @{
 */

/*! @brief A BRRES Index Group entry is a data structure that is part of a
 * BRRES Index Group.
 * 
 *  The ID of each entry is not a unique number, but is calculated from the
 * name comparing it to another name (see below) and used to search for a
 * given entry. The entries form a binary search tree, with the first entry
 * being the root. The left and right indicies describe this tree. [...]
 * 
 * Source: http://wiki.tockdom.com/wiki/BRRES_Index_Group_(File_Format)
 */
class BRRESIndexGroupEntry {

friend class BRRESIndexGroup;

public:

    /*! @brief Default constructor with optional owner parameter.
     * 
     *  This constructor will default all indices to 0, the ID to 0xFFFF, and
     * set the index group owner to the specified ctlib::BRRESIndexGroup.
     */
    BRRESIndexGroupEntry(BRRESIndexGroup* owner = nullptr);

private:

    BRRESIndexGroup* _group; // brres_index_group owning this entry

    uint16_t _id; // id of this entry
    uint16_t _idx; // index of this entry
    uint16_t _l_idx; // index of the left entry of this entry
    uint16_t _r_idx; // index of the right entry of this entry
};

/*! @brief The BRRES Index Group is a data structure that occurs very often
 * within BRRES files and sections of BRRES.
 * 
 *  It can be used to point to more BRRES Index Groups in order to allow folder
 * structure, as in the root section. [...]
 * 
 * Source: http://wiki.tockdom.com/wiki/BRRES_Index_Group_(File_Format)
 */
class BRRESIndexGroup {

public:

    /*! @brief Default constructor.
     * 
     *  This constructor will add the root to the entries, and have the count
     * set to 0.
     */
    BRRESIndexGroup();

    /*! @brief Copy constructor.
     * 
     *  This constructor will copy all entries from the source index group,
     * and set the owner of the copied entries to `this`.
     */
    BRRESIndexGroup(const BRRESIndexGroup&);

    /*! @brief Move constructor.
     * 
     *  This constructor will move all entries from the source index group,
     * and set the owner of the entries to `this`.
     */
    BRRESIndexGroup(BRRESIndexGroup&&);

private:

    uint32_t _count; // number of entries excluding the root
    std::vector<BRRESIndexGroupEntry> _entries; // the entries of this group
};

/*! @brief BRRES Files are used to make objects. They contain models, textures
 * and animations.
 * 
 * Source: http://wiki.tockdom.com/wiki/BRRES
 */
class BRRES {

private:
    BRRESIndexGroup _index_group; // the index group of the root section
};

/*! @} addtogroup brres */
}
