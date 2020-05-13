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
    callbacks{},
    camera{nullptr}
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
    callbacks{std::move(src.callbacks)},
    camera{src.camera}
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

    src.camera = nullptr;
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

void KMP::setCamera(CAME* camera)
{
    this->camera = camera;
}

KMP::CAME* KMP::getCamera() const
{
    return camera;
}

void KMP::invokeSectionCallbacksAdd(Section* section)
{
    for (SectionCallback* cb : callbacks)
    {
        cb->sectionAdded(section);
    }
}

void KMP::invokeSectionCallbacksRemove(Section* section)
{
    for (SectionCallback* cb : callbacks)
    {
        cb->sectionRemoved(section);
    }

    if (section == camera)
    {
        camera = nullptr;
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
        assertNoParent(first);
        assertValidFirst(first);
    }

    setParent(this->first, this->last, nullptr);

    this->first = first;
    this->last = first == nullptr ? nullptr : this->last;

    setParent(this->first, this->last, (PH*)this);
}

template <class PH, class PT>
void KMP::GroupSection<PH, PT>::setLast(PT* last)
{
    if (last != nullptr)
    {
        assertSameKMP(last);
        assertNoParent(last);
        assertValidLast(last);
    }

    setParent(this->first, this->last, nullptr);

    this->last = last;

    setParent(this->first, this->last, (PH*)this);
}

template <class PH, class PT>
void KMP::GroupSection<PH, PT>::addPrevious(PH* prev)
{
    assertNotNull(prev);
    assertSameKMP(prev);
    assertCanAddLink(prevs);
    prevs.push_back(prev);
}

template <class PH, class PT>
void KMP::GroupSection<PH, PT>::removePrevious(PH* prev)
{
    assertNotNull(prev);

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
    assertSameKMP(next);
    assertCanAddLink(nexts);
    nexts.push_back(next);
}

template <class PH, class PT>
void KMP::GroupSection<PH, PT>::removeNext(PH* next)
{
    assertNotNull(next);

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
    if (section == last)
    {
        if (last == first)
        {
            last = nullptr;
        }
        else
        {
            last = kmp->get<PT>(kmp->indexOf(last) - 1);
        }
    }
    if (section == first)
    {
        if (last == nullptr)
        {
            first = nullptr;
        }
        else
        {
            first = kmp->get<PT>(kmp->indexOf(first) + 1);
        }
    }

    if (section->getType() == PH::TYPE)
    {
        Collections::removeAll(prevs, (PH*)section);
        Collections::removeAll(nexts, (PH*)section);
    }
}

template <class PH, class PT>
void KMP::GroupSection<PH, PT>::setParent(PT* first, PT* last, PH* value)
{
    if (first == nullptr)
    {
        return;
    }

    if (last == nullptr)
    {
        first->parent = value;
        return;
    }

    for (int16_t i = kmp->indexOf(first); i >= 0 && i <= kmp->indexOf(last); ++i)
    {
        kmp->get<PT>(i)->parent = value;
    }
}

template <class PH, class PT>
void KMP::GroupSection<PH, PT>::assertNoParent(PT* entry) const
{
    if (entry->parent != nullptr && entry->parent != this)
    {
        throw KMPError(Strings::format(
            "KMP: The specified %s entry is already owned by another %s group!",
            Section::nameForType(PT::TYPE), Section::nameForType(PH::TYPE)
        ));
    }
}

template <class PH, class PT>
void KMP::GroupSection<PH, PT>::assertValidFirst(PT* first) const
{
    if (first == nullptr || last == nullptr)
    {
        return;
    }

    if (kmp->indexOf(first) > kmp->indexOf(last))
    {
        throw KMPError(Strings::format(
            "KMP: The specified %s entry index cannot be higher than the last entry's! (%d > %d)",
            Section::nameForType(PT::TYPE), kmp->indexOf(first), kmp->indexOf(last)
        ));
    }
}

template <class PH, class PT>
void KMP::GroupSection<PH, PT>::assertValidLast(PT* last) const
{
    if (last == nullptr)
    {
        return;
    }

    if (first == nullptr)
    {
        throw KMPError(Strings::format(
            "KMP: Cannot set last %s entry if the first entry of this %s group is not set!",
            Section::nameForType(PT::TYPE), Section::nameForType(PH::TYPE)
        ));
    }
    if (kmp->indexOf(last) < kmp->indexOf(first))
    {
        throw KMPError(Strings::format(
            "KMP: The specified %s entry index cannot be lower than the first entry's! (%d < %d)",
            Section::nameForType(PT::TYPE), kmp->indexOf(last), kmp->indexOf(first)
        ));
    }
}

template <class PH, class PT>
void KMP::GroupSection<PH, PT>::assertCanAddLink(const std::vector<PH*>& links) const
{
    if (links.size() >= MAX_LINKS)
    {
        throw KMPError(Strings::format(
            "KMP: The maximum number of links was reached by this %s group! (%d)",
            Section::nameForType(PH::TYPE), MAX_LINKS
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
template void KMP::GroupSection<KMP::PH, KMP::PT>::setParent(PT*, PT*, PH*); \
template void KMP::GroupSection<KMP::PH, KMP::PT>::assertNoParent(PT*) const; \
template void KMP::GroupSection<KMP::PH, KMP::PT>::assertValidFirst(PT*) const; \
template void KMP::GroupSection<KMP::PH, KMP::PT>::assertValidLast(PT*) const; \
template void KMP::GroupSection<KMP::PH, KMP::PT>::assertCanAddLink(const std::vector<PH*>&) const;

CT_LIB_DECLARE_GROUP_SECTION_METHODS(ENPH, ENPT)
CT_LIB_DECLARE_GROUP_SECTION_METHODS(ITPH, ITPT)
CT_LIB_DECLARE_GROUP_SECTION_METHODS(CKPH, CKPT)

#undef CT_LIB_DECLARE_GROUP_SECTION_METHODS

////// PointSection class //////////////

template <class PH>
KMP::PointSection<PH>::PointSection(KMP* kmp) :
    Section(kmp),
    parent{nullptr}
{

}

template <class PH>
KMP::PointSection<PH>::~PointSection() = default;

template <class PH>
PH* KMP::PointSection<PH>::getParent() const
{
    return parent;
}

template <class PH>
void KMP::PointSection<PH>::sectionAdded(Section* section)
{

}

template <class PH>
void KMP::PointSection<PH>::sectionRemoved(Section* section)
{
    if (section == parent)
    {
        parent = nullptr;
    }
}

#define CT_LIB_DECLARE_POINT_SECTION_METHODS(PH) \
template KMP::PointSection<KMP::PH>::PointSection(KMP*); \
template KMP::PointSection<KMP::PH>::~PointSection(); \
template KMP::PH* KMP::PointSection<KMP::PH>::getParent() const; \
template void KMP::PointSection<KMP::PH>::sectionAdded(Section*); \
template void KMP::PointSection<KMP::PH>::sectionRemoved(Section*);

CT_LIB_DECLARE_POINT_SECTION_METHODS(ENPH)
CT_LIB_DECLARE_POINT_SECTION_METHODS(ITPH)
CT_LIB_DECLARE_POINT_SECTION_METHODS(CKPH)

#undef CT_LIB_DECLARE_POINT_SECTION_METHODS


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
    PointSection(kmp),
    pos{},
    radius{20.f},
    routeCtrl{RouteControl::Default},
    driftCtrl{DriftControl::Default}
{
    kmp->registerCallback(this);
}

KMP::ENPT::~ENPT()
{
    kmp->unregisterCallback(this);
}

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
    PointSection(kmp),
    pos{},
    bulletRange{1.f},
    bulletCtrl{BulletControl::Default},
    forceBullet{false},
    shellIgnore{false}
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
    PointSection(kmp),
    left{},
    right{},
    respawn{nullptr},
    type{0xFF}
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

void KMP::CKPT::setTypeID(uint8_t type)
{
    this->type = type;
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

uint8_t KMP::CKPT::getTypeID() const
{
    return type;
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
    Section(kmp),
    type{0},
    pos{},
    rot{},
    scale{},
    route{nullptr},
    flag1p{true},
    flag2p{true},
    flag34p{true}
{
    for (uint8_t i = 0; i < SETTINGS_COUNT; ++i)
    {
        settings[i] = 0x00;
    }

    kmp->registerCallback(this);
}

KMP::GOBJ::~GOBJ()
{
    kmp->unregisterCallback(this);
}

KMP::SectionType KMP::GOBJ::getType() const
{
    return SectionType::GOBJ;
}

void KMP::GOBJ::setTypeID(uint16_t type)
{
    this->type = type;
}

void KMP::GOBJ::setPosition(Vector3f position)
{
    pos = position;
}

void KMP::GOBJ::setRotation(Vector3f rotation)
{
    rot = rotation;
}

void KMP::GOBJ::setScale(Vector3f scale)
{
    this->scale = scale;
}

void KMP::GOBJ::setRoute(POTI* route)
{
    if (route != nullptr)
    {
        assertSameKMP(route);
    }
    this->route = route;
}

void KMP::GOBJ::setSetting(uint8_t index, uint16_t value)
{
    assertValidSettingsIndex(index);
    settings[index] = value;
}

void KMP::GOBJ::setIsSinglePlayerEnabled(bool enable)
{
    flag1p = enable;
}

void KMP::GOBJ::setIs2PlayerEnabled(bool enable)
{
    flag2p = enable;
}

void KMP::GOBJ::setIs3And4PlayerEnabled(bool enable)
{
    flag34p = enable;
}

uint16_t KMP::GOBJ::getTypeID() const
{
    return type;
}

Vector3f KMP::GOBJ::getPosition() const
{
    return pos;
}

Vector3f KMP::GOBJ::getRotation() const
{
    return rot;
}

Vector3f KMP::GOBJ::getScale() const
{
    return scale;
}

KMP::POTI* KMP::GOBJ::getRoute() const
{
    return route;
}

uint16_t KMP::GOBJ::getSetting(uint8_t index) const
{
    assertValidSettingsIndex(index);
    return settings[index];
}

bool KMP::GOBJ::isSinglePlayerEnabled() const
{
    return flag1p;
}

bool KMP::GOBJ::is2PlayerEnabled() const
{
    return flag2p;
}

bool KMP::GOBJ::is3And4PlayerEnabled() const
{
    return flag34p;
}

void KMP::GOBJ::sectionAdded(Section* section)
{

}

void KMP::GOBJ::sectionRemoved(Section* section)
{
    if (section == route)
    {
        route = nullptr;
    }
}

void KMP::GOBJ::assertValidSettingsIndex(uint8_t index) const
{
    if (index >= SETTINGS_COUNT)
    {
        throw KMPError(Strings::format(
            "KMP: GOBJ setting index out of range! (%d >= %d)",
            index, SETTINGS_COUNT
        ));
    }
}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////
////   POTI section

void KMP::POTI::assertCanAdd(KMP* kmp)
{

}

KMP::POTI::POTI(KMP* kmp) :
    Section(kmp),
    smooth{false},
    type{RouteType::Loop},
    points{}
{

}

KMP::POTI::~POTI() = default;

KMP::SectionType KMP::POTI::getType() const
{
    return SectionType::POTI;
}

void KMP::POTI::setIsSmooth(bool enable)
{
    smooth = enable;
}

void KMP::POTI::setRouteType(RouteType type)
{
    this->type = type;
}

bool KMP::POTI::isSmooth() const
{
    return smooth;
}

KMP::POTI::RouteType KMP::POTI::getRouteType() const
{
    return type;
}

void KMP::POTI::addPoint(Point point)
{
    points.push_back(point);
}

KMP::POTI::Point& KMP::POTI::getPoint(uint16_t index)
{
    assertValidIndex(index);
    return points.at(index);
}

void KMP::POTI::removePoint(uint16_t index)
{
    assertValidIndex(index);
    points.erase(points.begin() + index);
}

std::vector<KMP::POTI::Point> KMP::POTI::getPoints() const
{
    return points;
}

uint16_t KMP::POTI::getPointCount() const
{
    return static_cast<uint16_t>(points.size());
}

void KMP::POTI::assertValidIndex(uint16_t index) const
{
    if (index >= points.size())
    {
        throw KMPError(Strings::format(
            "KMP: Point index of out range in POTI route! (%d >= %d)",
            index, points.size()
        ));
    }
}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////
////   AREA section

void KMP::AREA::assertCanAdd(KMP* kmp)
{

}

KMP::AREA::AREA(KMP* kmp) :
    Section(kmp),
    shape{Shape::Box},
    type{Type::Camera},
    camera{nullptr},
    priority{0},
    pos{},
    rot{},
    scale{},
    val1{0},
    val2{0},
    route{nullptr},
    dest{nullptr}
{
    kmp->registerCallback(this);
}

KMP::AREA::~AREA()
{
    kmp->unregisterCallback(this);
}

KMP::SectionType KMP::AREA::getType() const
{
    return SectionType::AREA;
}

void KMP::AREA::setShape(Shape shape)
{
    this->shape = shape;
}

void KMP::AREA::setAreaType(Type type)
{
    this->type = type;
}

void KMP::AREA::setCamera(CAME* camera)
{
    if (camera != nullptr)
    {
        assertSameKMP(camera);
    }
    this->camera = camera;
}

void KMP::AREA::setPriority(uint8_t priority)
{
    this->priority = priority;
}

void KMP::AREA::setPosition(Vector3f position)
{
    pos = position;
}

void KMP::AREA::setRotation(Vector3f rotation)
{
    rot = rotation;
}

void KMP::AREA::setScale(Vector3f scale)
{
    this->scale = scale;
}

void KMP::AREA::setSetting1(uint16_t value)
{
    val1 = value;
}

void KMP::AREA::setSetting2(uint16_t value)
{
    val2 = value;
}

void KMP::AREA::setRoute(POTI* route)
{
    if (route != nullptr)
    {
        assertSameKMP(route);
    }
    this->route = route;
}

void KMP::AREA::setDestinationPoint(ENPT* point)
{
    if (point != nullptr)
    {
        assertSameKMP(point);
    }
    dest = point;
}

KMP::AREA::Shape KMP::AREA::getShape() const
{
    return shape;
}

KMP::AREA::Type KMP::AREA::getAreaType() const
{
    return type;
}

KMP::CAME* KMP::AREA::getCamera() const
{
    return camera;
}

uint8_t KMP::AREA::getPriority() const
{
    return priority;
}

Vector3f KMP::AREA::getPosition() const
{
    return pos;
}

Vector3f KMP::AREA::getRotation() const
{
    return rot;
}

Vector3f KMP::AREA::getScale() const
{
    return scale;
}

uint16_t KMP::AREA::getSetting1() const
{
    return val1;
}

uint16_t KMP::AREA::getSetting2() const
{
    return val2;
}

KMP::POTI* KMP::AREA::getRoute() const
{
    return route;
}

KMP::ENPT* KMP::AREA::getDestinationPoint() const
{
    return dest;
}

void KMP::AREA::sectionAdded(Section* section)
{

}

void KMP::AREA::sectionRemoved(Section* section)
{
    if (section == camera)
    {
        camera = nullptr;
    }
    if (section == route)
    {
        route = nullptr;
    }
    if (section == dest)
    {
        dest = nullptr;
    }
}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////
////   CAME section

void KMP::CAME::assertCanAdd(KMP* kmp)
{

}

KMP::CAME::CAME(KMP* kmp) :
    Section(kmp),
    type{Type::Goal},
    next{nullptr},
    camshake{0},
    route{nullptr},
    vPoint{1},
    vZoom{1},
    vView{1},
    fStart{0},
    fMovie{0},
    pos{},
    rot{},
    zoomStart{70.f},
    zoomEnd{70.f},
    viewStart{},
    viewEnd{},
    time{0.f}
{
    kmp->registerCallback(this);
}

KMP::CAME::~CAME()
{
    kmp->unregisterCallback(this);
}

KMP::SectionType KMP::CAME::getType() const
{
    return SectionType::CAME;
}

void KMP::CAME::setCameraType(Type type)
{
    this->type = type;
}

void KMP::CAME::setNext(CAME* next)
{
    if (next != nullptr)
    {
        assertSameKMP(next);
    }
    this->next = next;
}

void KMP::CAME::setCamshake(uint8_t value)
{
    camshake = value;
}

void KMP::CAME::setRoute(POTI* route)
{
    if (route != nullptr)
    {
        assertSameKMP(route);
    }
    this->route = route;
}

void KMP::CAME::setPointVelocity(uint16_t velocity)
{
    vPoint = velocity;
}

void KMP::CAME::setZoomVelocity(uint16_t velocity)
{
    vZoom = velocity;
}

void KMP::CAME::setViewVelocity(uint16_t velocity)
{
    vView = velocity;
}

void KMP::CAME::setStartFlags(uint8_t flags)
{
    fStart = flags;
}

void KMP::CAME::setMovieFlags(uint8_t flags)
{
    fMovie = flags;
}

void KMP::CAME::setPosition(Vector3f position)
{
    pos = position;
}

void KMP::CAME::setRotation(Vector3f rotation)
{
    rot = rotation;
}

void KMP::CAME::setZoomStart(float zoom)
{
    zoomStart = zoom;
}

void KMP::CAME::setZoomEnd(float zoom)
{
    zoomEnd = zoom;
}

void KMP::CAME::setViewStart(Vector3f view)
{
    viewStart = view;
}

void KMP::CAME::setViewEnd(Vector3f view)
{
    viewEnd = view;
}

void KMP::CAME::setTime(float time)
{
    this->time = time;
}

KMP::CAME::Type KMP::CAME::getCameraType() const
{
    return type;
}

KMP::CAME* KMP::CAME::getNext() const
{
    return next;
}

uint8_t KMP::CAME::getCamshake() const
{
    return camshake;
}

KMP::POTI* KMP::CAME::getRoute() const
{
    return route;
}

uint16_t KMP::CAME::getPointVelocity() const
{
    return vPoint;
}

uint16_t KMP::CAME::getZoomVelocity() const
{
    return vZoom;
}

uint16_t KMP::CAME::getViewVelocity() const
{
    return vView;
}

uint8_t KMP::CAME::getStartFlags() const
{
    return fStart;
}

uint8_t KMP::CAME::getMovieFlags() const
{
    return fMovie;
}

Vector3f KMP::CAME::getPosition() const
{
    return pos;
}

Vector3f KMP::CAME::getRotation() const
{
    return rot;
}

float KMP::CAME::getZoomStart() const
{
    return zoomStart;
}

float KMP::CAME::getZoomEnd() const
{
    return zoomEnd;
}

Vector3f KMP::CAME::getViewStart() const
{
    return viewStart;
}

Vector3f KMP::CAME::getViewEnd() const
{
    return viewEnd;
}

float KMP::CAME::getTime() const
{
    return time;
}

void KMP::CAME::sectionAdded(Section* section)
{

}

void KMP::CAME::sectionRemoved(Section* section)
{
    if (section == next)
    {
        next = nullptr;
    }
    if (section == route)
    {
        route = nullptr;
    }
}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////
////   JGPT section

void KMP::JGPT::assertCanAdd(KMP* kmp)
{
    if (kmp->count<JGPT>() >= MAX_ENTRY_COUNT)
    {
        throw KMPError(Strings::format(
            "KMP: The maximum amount of JGPT entries was reached for this KMP! (%d)",
            MAX_ENTRY_COUNT
        ));
    }
}

KMP::JGPT::JGPT(KMP* kmp) :
    Section(kmp),
    pos{},
    rot{}
{

}

KMP::JGPT::~JGPT() = default;

KMP::SectionType KMP::JGPT::getType() const
{
    return SectionType::JGPT;
}

void KMP::JGPT::setPosition(Vector3f position)
{
    pos = position;
}

void KMP::JGPT::setRotation(Vector3f rotation)
{
    rot = rotation;
}

Vector3f KMP::JGPT::getPosition() const
{
    return pos;
}

Vector3f KMP::JGPT::getRotation() const
{
    return rot;
}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////
////   CNPT section

void KMP::CNPT::assertCanAdd(KMP* kmp)
{

}

KMP::CNPT::CNPT(KMP* kmp) :
    Section(kmp),
    dest{},
    rot{},
    type{static_cast<int16_t>(CannonType::Default)}
{

}

KMP::CNPT::~CNPT() = default;

KMP::SectionType KMP::CNPT::getType() const
{
    return SectionType::CNPT;
}

void KMP::CNPT::setDestination(Vector3f destination)
{
    dest = destination;
}

void KMP::CNPT::setDirection(Vector3f direction)
{
    rot = direction;
}

void KMP::CNPT::setCannonType(CannonType cannonType)
{
    type = static_cast<int16_t>(cannonType);
}

void KMP::CNPT::setTypeID(int16_t type)
{
    this->type = type;
}

Vector3f KMP::CNPT::getDestination() const
{
    return dest;
}

Vector3f KMP::CNPT::getDirection() const
{
    return rot;
}

KMP::CNPT::CannonType KMP::CNPT::getCannonType() const
{
    return type >= 0 && type <= 2 ? static_cast<CannonType>(type) : CannonType::Custom;
}

int16_t KMP::CNPT::getTypeID() const
{
    return type;
}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////
////   MSPT section

void KMP::MSPT::assertCanAdd(KMP* kmp)
{

}

KMP::MSPT::MSPT(KMP* kmp) :
    Section(kmp),
    pos{},
    rot{}
{

}

KMP::MSPT::~MSPT() = default;

KMP::SectionType KMP::MSPT::getType() const
{
    return SectionType::MSPT;
}

void KMP::MSPT::setPosition(Vector3f position)
{
    pos = position;
}

void KMP::MSPT::setRotation(Vector3f rotation)
{
    rot = rotation;
}

Vector3f KMP::MSPT::getPosition() const
{
    return pos;
}

Vector3f KMP::MSPT::getRotation() const
{
    return rot;
}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////
////   STGI section

void KMP::STGI::assertCanAdd(KMP* kmp)
{
    if (kmp->count<STGI>() >= MAX_ENTRY_COUNT)
    {
        throw KMPError("KMP: More than one STGI entry per KMP is invalid!");
    }
}

KMP::STGI::STGI(KMP* kmp) :
    Section(kmp),
    lapCount{3},
    startSide{StartSide::Left},
    narrow{false},
    lensFlare{false},
    lensColour{0xFFFFFF00},
    speedFactor{1.f}
{

}

KMP::STGI::~STGI() = default;

KMP::SectionType KMP::STGI::getType() const
{
    return SectionType::STGI;
}

void KMP::STGI::setLapCount(uint8_t count)
{
    assertValidLapCount(count);
    lapCount = count;
}

void KMP::STGI::setStartSide(StartSide side)
{
    startSide = side;
}

void KMP::STGI::setNarrowMode(bool enable)
{
    narrow = enable;
}

void KMP::STGI::setLensFlareEnabled(bool enable)
{
    lensFlare = enable;
}

void KMP::STGI::setLensFlareColour(uint32_t colour)
{
    lensColour = colour;
}

void KMP::STGI::setSpeedFactor(float factor)
{
    assertValidSpeedFactor(factor);
    speedFactor = factor;
}

uint8_t KMP::STGI::getLapCount() const
{
    return lapCount;
}

KMP::STGI::StartSide KMP::STGI::getStartSide() const
{
    return startSide;
}

bool KMP::STGI::isNarrowMode() const
{
    return narrow;
}

bool KMP::STGI::isLensFlareEnabled() const
{
    return lensFlare;
}

uint32_t KMP::STGI::getLensFlareColour() const
{
    return lensColour;
}

float KMP::STGI::getSpeedFactor() const
{
    return speedFactor;
}

void KMP::STGI::assertValidLapCount(uint8_t count) const
{
    if (count < 1 || count > MAX_LAP_COUNT)
    {
        throw KMPError(Strings::format(
            "KMP: Invalid STGI lap count! (%d)",
            count
        ));
    }
}

void KMP::STGI::assertValidSpeedFactor(float factor) const
{
    if (factor <= 0.f)
    {
        throw KMPError(Strings::format(
            "KMP: Invalid STGI speed factor! (%.2f <= 0.0)",
            factor
        ));
    }
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
