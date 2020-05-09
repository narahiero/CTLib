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
    callbacks{}
{

}

#define CT_LIB_KMP_MOVE_CONTAINER(container) \
    for (auto entry : container) \
        entry->kmp = this

KMP::KMP(KMP&& src) :
    ktpts{std::move(src.ktpts)},
    enpts{std::move(src.enpts)},
    enphs{std::move(src.enphs)}
{
    CT_LIB_KMP_MOVE_CONTAINER(ktpts);
    CT_LIB_KMP_MOVE_CONTAINER(enpts);
    CT_LIB_KMP_MOVE_CONTAINER(enphs);
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
    if (kmp->count<ENPT>() >= MAX_ENPT_ENTRY_COUNT)
    {
        throw KMPError(Strings::format(
            "KMP: The maximum amount of ENPT entries was reached for this KMP! (%d)",
            MAX_ENPT_ENTRY_COUNT
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
    Section(kmp),
    first{nullptr},
    last{nullptr},
    prevs{},
    nexts{}
{
    kmp->registerCallback(this);
}

KMP::ENPH::~ENPH()
{
    kmp->unregisterCallback(this);
}

KMP::SectionType KMP::ENPH::getType() const
{
    return SectionType::ENPH;
}

void KMP::ENPH::setFirst(ENPT* first)
{
    if (first != nullptr)
    {
        assertSameKMP(first);
    }
    this->first = first;
}

void KMP::ENPH::setLast(ENPT* last)
{
    if (last != nullptr)
    {
        assertSameKMP(last);
    }
    this->last = last;
}

void KMP::ENPH::addPrevious(ENPH* prev)
{
    assertNotNull(prev);
    assertNotThis(prev);
    assertSameKMP(prev);
    assertCanAddLink(prevs);
    prevs.push_back(prev);
}

void KMP::ENPH::removePrevious(ENPH* prev)
{
    assertNotNull(prev);
    assertNotThis(prev);

    if (Collections::removeAll(prevs, prev) == 0)
    {
        throw CTLib::KMPError(
            "KMP: The specified ENPH group was not found in the next groups list of this ENPH!"
        );
    }
}

void KMP::ENPH::addNext(ENPH* next)
{
    assertNotNull(next);
    assertNotThis(next);
    assertSameKMP(next);
    assertCanAddLink(nexts);
    nexts.push_back(next);
}

void KMP::ENPH::removeNext(ENPH* next)
{
    assertNotNull(next);
    assertNotThis(next);

    if (Collections::removeAll(nexts, next) == 0)
    {
        throw CTLib::KMPError(
            "KMP: The specified ENPH group was not found in the next groups list of this ENPH!"
        );
    }
}

KMP::ENPT* KMP::ENPH::getFirst() const
{
    return first;
}

KMP::ENPT* KMP::ENPH::getLast() const
{
    return last;
}

uint8_t KMP::ENPH::getPreviousCount() const
{
    return static_cast<uint8_t>(prevs.size());
}

std::vector<KMP::ENPH*> KMP::ENPH::getPrevious() const
{
    return prevs;
}

uint8_t KMP::ENPH::getNextCount() const
{
    return static_cast<uint8_t>(nexts.size());
}

std::vector<KMP::ENPH*> KMP::ENPH::getNext() const
{
    return nexts;
}

void KMP::ENPH::sectionAdded(Section* section)
{

}

void KMP::ENPH::sectionRemoved(Section* section)
{
    if (section == first)
    {
        first = nullptr;
    }
    if (section == last)
    {
        last = nullptr;
    }
    if (section->getType() == SectionType::ENPH)
    {
        Collections::removeAll(prevs, (ENPH*)section);
        Collections::removeAll(nexts, (ENPH*)section);
    }
}

void KMP::ENPH::assertCanAddLink(const std::vector<ENPH*>& links) const
{
    if (links.size() >= MAX_ENPH_LINKS)
    {
        throw KMPError(Strings::format(
            "KMP: The maximum number of links was reached by this ENPH! (%d)",
            links.size()
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
