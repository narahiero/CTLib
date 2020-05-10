//////////////////////////////////////////////////
//  Copyright (c) 2020 Nara Hiero
//
// This file is licensed under GPLv3+
// Refer to the `License.txt` file included.
//////////////////////////////////////////////////

#include <CTLib/KMP.hpp>

#include <CTLib/Utilities.hpp>

namespace CTLib
{

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////
////   KMP class
////

KMP::KMP() :
    ktpts{},
    enpts{},
    enphs{},
    itpts{},
    itphs{},
    ckpts{},
    ckphs{},
    gobjs{},
    potis{},
    areas{},
    cames{},
    jgpts{},
    cnpts{},
    mspts{},
    stgis{},
    callbacks{}
{

}

#define CT_LIB_KMP_MOVE_CONTAINER(container) \
    for (auto entry : container) \
        entry->kmp = this

KMP::KMP(KMP&& src) :
    ktpts{std::move(src.ktpts)},
    enpts{std::move(src.enpts)},
    enphs{std::move(src.enphs)},
    itpts{std::move(src.itpts)},
    itphs{std::move(src.itphs)},
    ckpts{std::move(src.ckpts)},
    ckphs{std::move(src.ckphs)},
    gobjs{std::move(src.gobjs)},
    potis{std::move(src.potis)},
    areas{std::move(src.areas)},
    cames{std::move(src.cames)},
    jgpts{std::move(src.jgpts)},
    cnpts{std::move(src.cnpts)},
    mspts{std::move(src.mspts)},
    stgis{std::move(src.stgis)},
    callbacks{std::move(src.callbacks)}
{
    CT_LIB_KMP_MOVE_CONTAINER(ktpts);
    CT_LIB_KMP_MOVE_CONTAINER(enpts);
    CT_LIB_KMP_MOVE_CONTAINER(enphs);
    CT_LIB_KMP_MOVE_CONTAINER(itpts);
    CT_LIB_KMP_MOVE_CONTAINER(itphs);
    CT_LIB_KMP_MOVE_CONTAINER(ckpts);
    CT_LIB_KMP_MOVE_CONTAINER(ckphs);
    CT_LIB_KMP_MOVE_CONTAINER(gobjs);
    CT_LIB_KMP_MOVE_CONTAINER(potis);
    CT_LIB_KMP_MOVE_CONTAINER(areas);
    CT_LIB_KMP_MOVE_CONTAINER(cames);
    CT_LIB_KMP_MOVE_CONTAINER(jgpts);
    CT_LIB_KMP_MOVE_CONTAINER(cnpts);
    CT_LIB_KMP_MOVE_CONTAINER(mspts);
    CT_LIB_KMP_MOVE_CONTAINER(stgis);
}

#undef CT_LIB_KMP_MOVE_CONTAINER

#define CT_LIB_KMP_DELETE_CONTAINER(container) \
    for (auto entry : container) \
        delete entry

KMP::~KMP()
{
    CT_LIB_KMP_DELETE_CONTAINER(ktpts);
    CT_LIB_KMP_DELETE_CONTAINER(enpts);
    CT_LIB_KMP_DELETE_CONTAINER(enphs);
    CT_LIB_KMP_DELETE_CONTAINER(itpts);
    CT_LIB_KMP_DELETE_CONTAINER(itphs);
    CT_LIB_KMP_DELETE_CONTAINER(ckpts);
    CT_LIB_KMP_DELETE_CONTAINER(ckphs);
    CT_LIB_KMP_DELETE_CONTAINER(gobjs);
    CT_LIB_KMP_DELETE_CONTAINER(potis);
    CT_LIB_KMP_DELETE_CONTAINER(areas);
    CT_LIB_KMP_DELETE_CONTAINER(cames);
    CT_LIB_KMP_DELETE_CONTAINER(jgpts);
    CT_LIB_KMP_DELETE_CONTAINER(cnpts);
    CT_LIB_KMP_DELETE_CONTAINER(mspts);
    CT_LIB_KMP_DELETE_CONTAINER(stgis);
}

#undef CT_LIB_KMP_DELETE_CONTAINER

#define CT_LIB_DEFINE_KMP_ADD(Type, container) \
template <> \
KMP::Type* KMP::add<KMP::Type>() \
{ \
    Type::assertCanAdd(this); \
    Type* instance = new Type(this); \
    container.push_back(instance); \
    invokeSectionCallbacksAdd(instance); \
    return instance; \
}

#define CT_LIB_DEFINE_KMP_GET(Type, container) \
template <> \
KMP::Type* KMP::get<KMP::Type>(uint16_t index) const \
{ \
    if (index >= container.size()) \
    { \
        throw KMPError(Strings::format( \
            "KMP: Invalid index for type " #Type "! (%d >= %d)", \
            index, container.size() \
        )); \
    } \
    return container.at(index); \
}

#define CT_LIB_DEFINE_KMP_REMOVE(Type, container) \
template <> \
void KMP::remove<KMP::Type>(uint16_t index) \
{ \
    if (index >= container.size()) \
    { \
        throw KMPError(Strings::format( \
            "KMP: Invalid index for type " #Type "! (%d >= %d)", \
            index, container.size() \
        )); \
    } \
    Type* instance = container.at(index); \
    invokeSectionCallbacksRemove(instance); \
    delete instance; \
    container.erase(container.begin() + index); \
}

#define CT_LIB_DEFINE_KMP_INDEX_OF(Type, container) \
template <> \
int16_t KMP::indexOf<KMP::Type>(Type* entry) const \
{ \
    for (uint16_t i = 0; i < container.size(); ++i) \
    { \
        if (entry == container.at(i)) \
        { \
            return i; \
        } \
    } \
    return -1i16; \
}

#define CT_LIB_DEFINE_KMP_GET_ALL(Type, container) \
template <> \
std::vector<KMP::Type*> KMP::getAll<KMP::Type>() const \
{ \
    return container; \
}

#define CT_LIB_DEFINE_KMP_COUNT(Type, container) \
template <> \
uint16_t KMP::count<KMP::Type>() const \
{ \
    return static_cast<uint16_t>(container.size()); \
}

#define CT_LIB_DEFINE_ALL_KMP(Type, container) \
CT_LIB_DEFINE_KMP_ADD(Type, container) \
CT_LIB_DEFINE_KMP_GET(Type, container) \
CT_LIB_DEFINE_KMP_REMOVE(Type, container) \
CT_LIB_DEFINE_KMP_INDEX_OF(Type, container) \
CT_LIB_DEFINE_KMP_GET_ALL(Type, container) \
CT_LIB_DEFINE_KMP_COUNT(Type, container)

CT_LIB_DEFINE_ALL_KMP(KTPT, ktpts)
CT_LIB_DEFINE_ALL_KMP(ENPT, enpts)
CT_LIB_DEFINE_ALL_KMP(ENPH, enphs)
CT_LIB_DEFINE_ALL_KMP(ITPT, itpts)
CT_LIB_DEFINE_ALL_KMP(ITPH, itphs)
CT_LIB_DEFINE_ALL_KMP(CKPT, ckpts)
CT_LIB_DEFINE_ALL_KMP(CKPH, ckphs)
CT_LIB_DEFINE_ALL_KMP(GOBJ, gobjs)
CT_LIB_DEFINE_ALL_KMP(POTI, potis)
CT_LIB_DEFINE_ALL_KMP(AREA, areas)
CT_LIB_DEFINE_ALL_KMP(CAME, cames)
CT_LIB_DEFINE_ALL_KMP(JGPT, jgpts)
CT_LIB_DEFINE_ALL_KMP(CNPT, cnpts)
CT_LIB_DEFINE_ALL_KMP(MSPT, mspts)
CT_LIB_DEFINE_ALL_KMP(STGI, stgis)

#undef CT_LIB_DEFINE_ALL_KMP
#undef CT_LIB_DEFINE_KMP_ADD
#undef CT_LIB_DEFINE_KMP_GET
#undef CT_LIB_DEFINE_KMP_REMOVE
#undef CT_LIB_DEFINE_KMP_INDEX_OF
#undef CT_LIB_DEFINE_KMP_GET_ALL
#undef CT_LIB_DEFINE_KMP_COUNT

void KMP::registerCallback(SectionCallback* cb)
{
    callbacks.push_back(cb);
}

void KMP::unregisterCallback(SectionCallback* cb)
{
    Collections::removeAll(callbacks, cb);
}

void KMP::invokeSectionCallbacksAdd(Section* section) const
{
    for (SectionCallback* cb : callbacks)
    {
        cb->sectionAdded(section);
    }
}

void KMP::invokeSectionCallbacksRemove(Section* section) const
{
    for (SectionCallback* cb : callbacks)
    {
        cb->sectionRemoved(section);
    }
}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////
////   Base KMP section
////

const char* KMP::Section::nameForType(SectionType type)
{
    static const char* NAMES =
    "KTPT\0ENPT\0ENPH\0ITPT\0ITPH\0CKPT\0CKPH\0GOBJ\0POTI\0AREA\0CAME\0JGPT\0CNPT\0MSPT\0STGI\0";
    return NAMES + (static_cast<uint32_t>(type) * 0x5);
}

KMP::Section::Section(KMP* kmp) :
    kmp{kmp}
{

}

KMP::Section::~Section() = default;

void KMP::Section::assertNotNull(Section* section) const
{
    if (section == nullptr)
    {
        throw KMPError("KMP: The specified section is `nullptr`!");
    }
}

void KMP::Section::assertNotThis(Section* section) const
{
    if (section == this)
    {
        throw KMPError("KMP: The specified section is the same as this!");
    }
}

void KMP::Section::assertSameKMP(Section* section) const
{
    if (kmp != section->kmp)
    {
        throw KMPError("KMP: The specified section is not owned by the same KMP!");
    }
}

////// GroupSection class //////////////

template <class PH, class PT>
KMP::GroupSection<PH, PT>::GroupSection(KMP* kmp) :
    Section(kmp),
    first{nullptr},
    last{nullptr},
    prevs{},
    nexts{}
{
    kmp->registerCallback(this);
}

template <class PH, class PT>
KMP::GroupSection<PH, PT>::~GroupSection()
{
    kmp->unregisterCallback(this);
}

template <class PH, class PT>
void KMP::GroupSection<PH, PT>::setFirst(PT* first)
{
    if (first != nullptr)
    {
        assertSameKMP(first);
    }
    this->first = first;
}

template <class PH, class PT>
void KMP::GroupSection<PH, PT>::setLast(PT* last)
{
    if (last != nullptr)
    {
        assertSameKMP(last);
    }
    this->last = last;
}

template <class PH, class PT>
void KMP::GroupSection<PH, PT>::addPrevious(PH* prev)
{
    assertNotNull(prev);
    assertNotThis(prev);
    assertSameKMP(prev);
    assertCanAddLink(prevs);
    prevs.push_back(prev);
}

template <class PH, class PT>
void KMP::GroupSection<PH, PT>::removePrevious(PH* prev)
{
    assertNotNull(prev);
    assertNotThis(prev);

    if (Collections::removeAll(prevs, prev) == 0)
    {
        throw CTLib::KMPError(
            "KMP: The specified group was not found in the next groups list of this group!"
        );
    }
}

template <class PH, class PT>
void KMP::GroupSection<PH, PT>::addNext(PH* next)
{
    assertNotNull(next);
    assertNotThis(next);
    assertSameKMP(next);
    assertCanAddLink(nexts);
    nexts.push_back(next);
}

template <class PH, class PT>
void KMP::GroupSection<PH, PT>::removeNext(PH* next)
{
    assertNotNull(next);
    assertNotThis(next);

    if (Collections::removeAll(nexts, next) == 0)
    {
        throw CTLib::KMPError(
            "KMP: The specified group was not found in the next groups list of this group!"
        );
    }
}

template <class PH, class PT>
PT* KMP::GroupSection<PH, PT>::getFirst() const
{
    return first;
}

template <class PH, class PT>
PT* KMP::GroupSection<PH, PT>::getLast() const
{
    return last;
}

template <class PH, class PT>
uint8_t KMP::GroupSection<PH, PT>::getPreviousCount() const
{
    return static_cast<uint8_t>(prevs.size());
}

template <class PH, class PT>
std::vector<PH*> KMP::GroupSection<PH, PT>::getPrevious() const
{
    return prevs;
}

template <class PH, class PT>
uint8_t KMP::GroupSection<PH, PT>::getNextCount() const
{
    return static_cast<uint8_t>(nexts.size());
}

template <class PH, class PT>
std::vector<PH*> KMP::GroupSection<PH, PT>::getNext() const
{
    return nexts;
}

template <class PH, class PT>
void KMP::GroupSection<PH, PT>::sectionAdded(Section* section)
{

}

template <class PH, class PT>
void KMP::GroupSection<PH, PT>::sectionRemoved(Section* section)
{
    if (section == first)
    {
        first = nullptr;
    }
    if (section == last)
    {
        last = nullptr;
    }
    if (section->getType() == PH::TYPE)
    {
        Collections::removeAll(prevs, (PH*)section);
        Collections::removeAll(nexts, (PH*)section);
    }
}

template <class PH, class PT>
void KMP::GroupSection<PH, PT>::assertCanAddLink(const std::vector<PH*>& links) const
{
    if (links.size() >= MAX_LINKS)
    {
        throw KMPError(Strings::format(
            "KMP: The maximum number of links was reached by this group! (%d)",
            MAX_LINKS
        ));
    }
}

#define CT_LIB_DECLARE_GROUP_SECTION_METHODS(PH, PT) \
template KMP::GroupSection<KMP::PH, KMP::PT>::GroupSection(KMP* kmp); \
template KMP::GroupSection<KMP::PH, KMP::PT>::~GroupSection(); \
template void KMP::GroupSection<KMP::PH, KMP::PT>::setFirst(PT*); \
template void KMP::GroupSection<KMP::PH, KMP::PT>::setLast(PT*); \
template void KMP::GroupSection<KMP::PH, KMP::PT>::addPrevious(PH*); \
template void KMP::GroupSection<KMP::PH, KMP::PT>::removePrevious(PH*); \
template void KMP::GroupSection<KMP::PH, KMP::PT>::addNext(PH*); \
template void KMP::GroupSection<KMP::PH, KMP::PT>::removeNext(PH*); \
template KMP::PT* KMP::GroupSection<KMP::PH, KMP::PT>::getFirst() const; \
template KMP::PT* KMP::GroupSection<KMP::PH, KMP::PT>::getLast() const; \
template uint8_t KMP::GroupSection<KMP::PH, KMP::PT>::getPreviousCount() const; \
template std::vector<KMP::PH*> KMP::GroupSection<KMP::PH, KMP::PT>::getPrevious() const; \
template uint8_t KMP::GroupSection<KMP::PH, KMP::PT>::getNextCount() const; \
template std::vector<KMP::PH*> KMP::GroupSection<KMP::PH, KMP::PT>::getNext() const; \
template void KMP::GroupSection<KMP::PH, KMP::PT>::sectionAdded(Section*); \
template void KMP::GroupSection<KMP::PH, KMP::PT>::sectionRemoved(Section*); \
template void KMP::GroupSection<KMP::PH, KMP::PT>::assertCanAddLink(const std::vector<PH*>&) const;

CT_LIB_DECLARE_GROUP_SECTION_METHODS(ENPH, ENPT)
CT_LIB_DECLARE_GROUP_SECTION_METHODS(ITPH, ITPT)
CT_LIB_DECLARE_GROUP_SECTION_METHODS(CKPH, CKPT)

#undef CT_LIB_DECLARE_GROUP_SECTION_METHODS


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////
////   KTPT section

void KMP::KTPT::assertCanAdd(KMP* kmp)
{

}

KMP::KTPT::KTPT(KMP* kmp) :
    Section(kmp),
    pos{},
    rot{},
    index{-1}
{

}

KMP::KTPT::~KTPT() = default;

KMP::SectionType KMP::KTPT::getType() const
{
    return SectionType::KTPT;
}

void KMP::KTPT::setPosition(Vector3f position)
{
    pos = position;
}

void KMP::KTPT::setRotation(Vector3f rotation)
{
    rot = rotation;
}

void KMP::KTPT::setPlayerIndex(int16_t index)
{
    assertValidPlayerIndex(index);
    this->index = index;
}

Vector3f KMP::KTPT::getPosition() const
{
    return pos;
}

Vector3f KMP::KTPT::getRotation() const
{
    return rot;
}

int16_t KMP::KTPT::getPlayerIndex() const
{
    return index;
}

void KMP::KTPT::assertValidPlayerIndex(int16_t index) const
{
    if (index < -1 || index >= MAX_PLAYER_INDEX)
    {
        throw KMPError(Strings::format(
            "KMP: Invalid KTPT player index! (%d)",
            index
        ));
    }
}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////
////   ENPT section
////

void KMP::ENPT::assertCanAdd(KMP* kmp)
{
    if (kmp->count<ENPT>() >= MAX_ENTRY_COUNT)
    {
        throw KMPError(Strings::format(
            "KMP: The maximum amount of ENPT entries was reached for this KMP! (%d)",
            MAX_ENTRY_COUNT
        ));
    }
}

KMP::ENPT::ENPT(KMP* kmp) :
    Section(kmp),
    pos{},
    radius{20.f},
    routeCtrl{RouteControl::Default},
    driftCtrl{DriftControl::Default}
{

}

KMP::ENPT::~ENPT() = default;

KMP::SectionType KMP::ENPT::getType() const
{
    return SectionType::ENPT;
}

void KMP::ENPT::setPosition(Vector3f position)
{
    pos = position;
}

void KMP::ENPT::setRadius(float radius)
{
    this->radius = radius;
}

void KMP::ENPT::setRouteControl(RouteControl control)
{
    routeCtrl = control;
}

void KMP::ENPT::setDriftControl(DriftControl control)
{
    driftCtrl = control;
}

Vector3f KMP::ENPT::getPosition() const
{
    return pos;
}

float KMP::ENPT::getRadius() const
{
    return radius;
}

KMP::ENPT::RouteControl KMP::ENPT::getRouteControl() const
{
    return routeCtrl;
}

KMP::ENPT::DriftControl KMP::ENPT::getDriftControl() const
{
    return driftCtrl;
}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////
////   ENPH section
////

void KMP::ENPH::assertCanAdd(KMP* kmp)
{

}

KMP::ENPH::ENPH(KMP* kmp) :
    GroupSection(kmp)
{

}

KMP::ENPH::~ENPH() = default;

KMP::SectionType KMP::ENPH::getType() const
{
    return SectionType::ENPH;
}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////
////   ITPT section
////

void KMP::ITPT::assertCanAdd(KMP* kmp)
{
    if (kmp->count<ITPT>() >= MAX_ENTRY_COUNT)
    {
        throw KMPError(Strings::format(
            "KMP: The maximum amount of ITPT entries was reached for this KMP! (%d)",
            MAX_ENTRY_COUNT
        ));
    }
}

KMP::ITPT::ITPT(KMP* kmp) :
    Section(kmp)
{

}

KMP::ITPT::~ITPT() = default;

KMP::SectionType KMP::ITPT::getType() const
{
    return SectionType::ITPT;
}

void KMP::ITPT::setPosition(Vector3f position)
{
    pos = position;
}

void KMP::ITPT::setBulletRange(float range)
{
    bulletRange = range;
}

void KMP::ITPT::setBulletControl(BulletControl control)
{
    bulletCtrl = control;
}

void KMP::ITPT::setForceBullet(bool force)
{
    forceBullet = force;
}

void KMP::ITPT::setShellIgnore(bool ignore)
{
    shellIgnore = ignore;
}

Vector3f KMP::ITPT::getPosition() const
{
    return pos;
}

float KMP::ITPT::getBulletRange() const
{
    return bulletRange;
}

KMP::ITPT::BulletControl KMP::ITPT::getBulletControl() const
{
    return bulletCtrl;
}

bool KMP::ITPT::isForceBullet() const
{
    return forceBullet;
}

bool KMP::ITPT::isShellIgnore() const
{
    return shellIgnore;
}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////
////   ITPH section
////

void KMP::ITPH::assertCanAdd(KMP* kmp)
{

}

KMP::ITPH::ITPH(KMP* kmp) :
    GroupSection(kmp)
{

}

KMP::ITPH::~ITPH() = default;

KMP::SectionType KMP::ITPH::getType() const
{
    return SectionType::ITPH;
}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////
////   CKPT section
////

void KMP::CKPT::assertCanAdd(KMP* kmp)
{
    if (kmp->count<CKPT>() >= MAX_ENTRY_COUNT)
    {
        throw KMPError(Strings::format(
            "KMP: The maximum amount of CKPT entries was reached for this KMP! (%d)",
            MAX_ENTRY_COUNT
        ));
    }
}

KMP::CKPT::CKPT(KMP* kmp) :
    Section(kmp),
    left{},
    right{},
    respawn{nullptr},
    key{false}
{
    kmp->registerCallback(this);
}

KMP::CKPT::~CKPT()
{
    kmp->unregisterCallback(this);
}

KMP::SectionType KMP::CKPT::getType() const
{
    return SectionType::CKPT;
}

void KMP::CKPT::setLeft(Vector2f position)
{
    left = position;
}

void KMP::CKPT::setRight(Vector2f position)
{
    right = position;
}

void KMP::CKPT::setRespawn(JGPT* respawn)
{
    if (respawn != nullptr)
    {
        assertSameKMP(respawn);
    }
    this->respawn = respawn;
}

void KMP::CKPT::setIsKey(bool set)
{
    key = set;
}

Vector2f KMP::CKPT::getLeft() const
{
    return left;
}

Vector2f KMP::CKPT::getRight() const
{
    return right;
}

KMP::JGPT* KMP::CKPT::getRespawn() const
{
    return respawn;
}

bool KMP::CKPT::isKey() const
{
    return key;
}

void KMP::CKPT::sectionAdded(Section* section)
{

}

void KMP::CKPT::sectionRemoved(Section* section)
{
    if (section == respawn)
    {
        respawn = nullptr;
    }
}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////
////   CKPH section
////

void KMP::CKPH::assertCanAdd(KMP* kmp)
{

}

KMP::CKPH::CKPH(KMP* kmp) :
    GroupSection(kmp)
{

}

KMP::CKPH::~CKPH() = default;

KMP::SectionType KMP::CKPH::getType() const
{
    return SectionType::CKPH;
}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////
////   GOBJ section

void KMP::GOBJ::assertCanAdd(KMP* kmp)
{

}

KMP::GOBJ::GOBJ(KMP* kmp) :
    Section(kmp)
{

}

KMP::GOBJ::~GOBJ() = default;

KMP::SectionType KMP::GOBJ::getType() const
{
    return SectionType::GOBJ;
}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////
////   POTI section

void KMP::POTI::assertCanAdd(KMP* kmp)
{

}

KMP::POTI::POTI(KMP* kmp) :
    Section(kmp)
{

}

KMP::POTI::~POTI() = default;

KMP::SectionType KMP::POTI::getType() const
{
    return SectionType::POTI;
}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////
////   AREA section

void KMP::AREA::assertCanAdd(KMP* kmp)
{

}

KMP::AREA::AREA(KMP* kmp) :
    Section(kmp)
{

}

KMP::AREA::~AREA() = default;

KMP::SectionType KMP::AREA::getType() const
{
    return SectionType::AREA;
}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////
////   CAME section

void KMP::CAME::assertCanAdd(KMP* kmp)
{

}

KMP::CAME::CAME(KMP* kmp) :
    Section(kmp)
{

}

KMP::CAME::~CAME() = default;

KMP::SectionType KMP::CAME::getType() const
{
    return SectionType::CAME;
}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////
////   JGPT section

void KMP::JGPT::assertCanAdd(KMP* kmp)
{

}

KMP::JGPT::JGPT(KMP* kmp) :
    Section(kmp)
{

}

KMP::JGPT::~JGPT() = default;

KMP::SectionType KMP::JGPT::getType() const
{
    return SectionType::JGPT;
}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////
////   CNPT section

void KMP::CNPT::assertCanAdd(KMP* kmp)
{

}

KMP::CNPT::CNPT(KMP* kmp) :
    Section(kmp)
{

}

KMP::CNPT::~CNPT() = default;

KMP::SectionType KMP::CNPT::getType() const
{
    return SectionType::CNPT;
}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////
////   MSPT section

void KMP::MSPT::assertCanAdd(KMP* kmp)
{

}

KMP::MSPT::MSPT(KMP* kmp) :
    Section(kmp)
{

}

KMP::MSPT::~MSPT() = default;

KMP::SectionType KMP::MSPT::getType() const
{
    return SectionType::MSPT;
}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////
////   STGI section

void KMP::STGI::assertCanAdd(KMP* kmp)
{

}

KMP::STGI::STGI(KMP* kmp) :
    Section(kmp)
{

}

KMP::STGI::~STGI() = default;

KMP::SectionType KMP::STGI::getType() const
{
    return SectionType::STGI;
}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////
////   KMP error class
////

KMPError::KMPError(const char* msg) : std::runtime_error(msg)
{

}

KMPError::KMPError(const std::string& msg) : std::runtime_error(msg)
{

}
}
