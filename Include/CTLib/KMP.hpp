//////////////////////////////////////////////////
//  Copyright (c) 2020 Nara Hiero
//
// This file is licensed under GPLv3+
// Refer to the `License.txt` file included.
//////////////////////////////////////////////////

#pragma once

/*! @file KMP.hpp
 * 
 *  @brief The header for KMP module.
 */


#include <stdexcept>
#include <vector>

#include <CTLib/Math.hpp>
#include <CTLib/Memory.hpp>


namespace CTLib
{

/*! @defgroup kmp KMP
 * 
 *  @addtogroup kmp
 * 
 *  @brief The KMP module contains all classes and methods that can be used to
 *  interact with Nintendo's KMP data format.
 *  @{
 */

/*! @brief Object representation of Nintendo's KMP data format. */
class KMP final
{

public:

    /*! @brief Reads a KMP from the specified Buffer. */
    static KMP read(Buffer& data);

    /*! @brief Writes the specified KMP to a new Buffer. */
    static Buffer write(const KMP& kmp);

    /*! @brief Enumeration of the possible section type values. */
    enum class SectionType
    {
        /*! @brief KTPT section. (_Kart Points_) */
        KTPT,

        /*! @brief ENPT section. (_Enemy Points_) */
        ENPT,

        /*! @brief ENPH section. (_Enemy Paths_) */
        ENPH,

        /*! @brief ITPT section. (_Item Points_) */
        ITPT,

        /*! @brief ITPH section. (_Item Paths_) */
        ITPH,

        /*! @brief CKPT section. (_Checkpoints_) */
        CKPT,

        /*! @brief CKPH section. (_Checkpoint Groups_) */
        CKPH,

        /*! @brief GOBJ section. (_Global Objects_) */
        GOBJ,

        /*! @brief POTI section. (_Routes_) */
        POTI,

        /*! @brief AREA section. (_Areas_) */
        AREA,

        /*! @brief CAME section. (_Cameras_) */
        CAME,

        /*! @brief JGPT section. (_Respawn Points_) */
        JGPT,

        /*! @brief CNPT section. (_Cannon Points_) */
        CNPT,

        /*! @brief MSPT section. (_Mission Points_) */
        MSPT,

        /*! @brief STGI section. (_Stage Information_) */
        STGI
    };

    /*! @brief Superclass of all KMP sections. */
    class Section
    {

        friend class KMP;

    public:

        /*! @brief Returns the string representation of the specified type. */
        static const char* nameForType(SectionType type);

        virtual ~Section();

        /*! @brief Returns the SectionType of this Section. */
        virtual SectionType getType() const = 0;

    protected:

        //! sets the kmp pointer of this section
        Section(KMP* kmp);

        //! throws if 'section' == `nullptr`
        void assertNotNull(Section* section) const;

        //! throws if 'section' == 'this'
        void assertNotThis(Section* section) const;

        //! throws if 'this->kmp' != 'section->kmp'
        void assertSameKMP(Section* section) const;

        //! pointer to KMP owning this section
        KMP* kmp;
    };

    /*! @brief Called when a KMP Section entry is added or removed. */
    class SectionCallback
    {

        friend class KMP;

    protected:

        /*! @brief Called when a KMP Section entry is added. */
        virtual void sectionAdded(Section* section) = 0;

        /*! @brief Called when a KMP Section entry is removed. */
        virtual void sectionRemoved(Section* section) = 0;
    };

    /*! @brief Starting positions of racers. */
    class KTPT final : public Section
    {

        friend class KMP;

    public:

        /*! @brief All player indices must be less than this constant. */
        static constexpr int16_t MAX_PLAYER_INDEX = 12;

        ~KTPT();

        /*! @brief Returns SectionType::KTPT. */
        SectionType getType() const override;

        /*! @brief Sets the starting position of this KTPT entry. */
        void setPosition(Vector3f position);

        /*! @brief Sets the starting rotation of this KTPT entry. */
        void setRotation(Vector3f rotation);

        /*! @brief Sets the player index of this KTPT entry.
         *  
         *  For race tracks, there must be only one KTPT entry and the player
         *  index must be -1.
         * 
         *  For battle tracks, there must be 12 KTPT entries (each with unique
         *  index), with indices 0 to 5 for red team and indices 6 to 11 for
         *  blue team.
         * 
         *  An additional 13th KTPT entry can be added for battle tracks with
         *  index -1 to specify start line position if using LE-CODE.
         *  
         *  @param[in] index The player index
         * 
         *  @throw CTLib::KMPError If the specified index is less than -1 or
         *  more than or equal to MAX_PLAYER_INDEX.
         */
        void setPlayerIndex(int16_t index);

        /*! @brief Returns the starting position of this KTPT entry. */
        Vector3f getPosition() const;

        /*! @brief Returns the starting rotation of this KTPT entry. */
        Vector3f getRotation() const;

        /*! @brief Returns the player index of this KTPT entry. */
        int16_t getPlayerIndex() const;

    private:

        // does nothing
        static void assertCanAdd(KMP* kmp);

        KTPT(KMP* kmp);

        // throws if 'index' < -1 or >= 'MAX_PLAYER_INDEX'
        void assertValidPlayerIndex(int16_t index) const;

        // position
        Vector3f pos;

        // rotation
        Vector3f rot;

        // player index
        int16_t index;
    };

    /*! @brief Enemy route points. */
    class ENPT final : public Section
    {

        friend class KMP;

    public:

        /*! @brief Enumeration of the possible point control values. */
        enum class RouteControl : uint16_t
        {
            /*! @brief Default (let the CPU decide what to do). */
            Default = 0x0,

            /*! @brief Only enter if CPU has offroad-cutting item. */
            OffroadPath = 0x1,

            /*! @brief Force CPU to use offroad-cutting item. */
            OffroadUseItem = 0x2,

            /*! @brief Force wheelie. */
            Wheelie = 0x3,

            /*! @brief Force drop wheelie. */
            WheelieEnd = 0x4,

            /*! @brief Unknown usage. */
            Unknown = 0x5
        };

        /*! @brief Enumeration of the possible drift control values. */
        enum class DriftControl : uint8_t
        {
            /*! @brief Default (let the CPU decide what to do). _Recommended_ */
            Default = 0x0,

            /*! @brief Force CPU to end drift. */
            EndDrift = 0x1,

            /*! @brief Prevent CPU from drifting. */
            NoDrift = 0x2,

            /*! @brief Force CPU to drift. */
            ForceDrift = 0x3,

            /*! @brief Unknown usage. */
            Unknown0 = 0xA,

            /*! @brief Unknown usage. */
            Unknown1 = 0x14
        };

        /*! @brief Maximum number of ENPT entries per KMP. */
        static constexpr uint16_t MAX_ENPT_ENTRY_COUNT = 255;

        ~ENPT();

        /*! @brief Returns SectionType::ENPT. */
        SectionType getType() const override;

        /*! @brief Sets the position of this point. */
        void setPosition(Vector3f position);

        /*! @brief Sets the radius of this point.
         * 
         *  The radius controls by how much the CPU is allowed to deviate to the
         *  left or right of the position of this point.
         * 
         *  @param[in] radius The point radius
         */
        void setRadius(float radius);

        /*! @brief Sets the RouteControl of this point. */
        void setRouteControl(RouteControl control);

        /*! @brief Sets the DriftControl of this point. */
        void setDriftControl(DriftControl control);

        /*! @brief Returns the position of this point. */
        Vector3f getPosition() const;

        /*! @brief Returns the radius of this point. */
        float getRadius() const;

        /*! @brief Returns the RouteControl of this point. */
        RouteControl getRouteControl() const;

        /*! @brief Returns the DriftControl of this point. */
        DriftControl getDriftControl() const;

    private:

        // throws if specified KMP already has 'MAX_ENPT_ENTRY_COUNT' ENPTs
        static void assertCanAdd(KMP* kmp);

        ENPT(KMP* kmp);

        // position
        Vector3f pos;

        // max enemy deviation from pos
        float radius;

        RouteControl routeCtrl;
        DriftControl driftCtrl;
    };

    /*! @brief Enemy route point groups. */
    class ENPH final : public Section, public SectionCallback
    {

        friend class KMP;

    public:

        /*! @brief The maximum number of ENPH entry links. */
        static constexpr uint8_t MAX_ENPH_LINKS = 6;

        ~ENPH();

        /*! @brief Returns SectionType::ENPH. */
        SectionType getType() const override;

        /*! @brief Sets the first ENPT point that is part of this ENPH group.
         *  
         *  If the specified ENPT entry is removed from its KMP after this
         *  function returns, the first ENPT entry of this ENPH will be set back
         *  to `nullptr`.
         * 
         *  @param[in] first The first ENPT entry
         * 
         *  @throw CTLib::KMPError If the specified ENPT entry is not owned by
         *  the same KMP as this ENPH group.
         */
        void setFirst(ENPT* first);

        /*! @brief Sets the last ENPT point that is part of this ENPH group.
         *  
         *  If the specified ENPT entry is removed from its KMP after this
         *  function returns, the last ENPT entry of this ENPH will be set back
         *  to `nullptr`.
         * 
         *  @param[in] last The last ENPT entry
         * 
         *  @throw CTLib::KMPError If the specified ENPT entry is not owned by
         *  the same KMP as this ENPH group.
         */
        void setLast(ENPT* last);

        /*! @brief Adds the specified ENPH group to the list of previous groups
         *  of this ENPH group.
         * 
         *  If the specified ENPH group is removed from its KMP after this
         *  function returns, it will be removed from the list of previous
         *  groups of this ENPH group.
         * 
         *  @param[in] prev The ENPH group to be added to the previous groups
         * 
         *  @throw CTLib::KMPError If the specified ENPH group is `nullptr` or
         *  `this`, or not owned by the same KMP as this ENPH group, or this
         *  ENPH group already has MAX_ENPH_LINKS previous groups.
         */
        void addPrevious(ENPH* prev);

        /*! @brief Removes the specified ENPH group from the list of previous
         *  groups of this ENPH group.
         * 
         *  @param[in] prev The previous ENPH group to remove
         * 
         *  @throw CTLib::KMPError If the specified ENPH group is `nullptr` or
         *  `this`, or is not a previous group of this ENPH group.
         */
        void removePrevious(ENPH* prev);

        /*! @brief Adds the specified ENPH group to the list of next groups of
         *  this ENPH group.
         * 
         *  If the specified ENPH group is removed from its KMP after this
         *  function returns, it will be removed from the list of next groups
         *  of this ENPH group.
         *  
         *  Adding a ENPH group multiple times will increase the probability of
         *  it being chosen by the CPUs.
         * 
         *  @param[in] next The ENPH group to be added to the next groups
         * 
         *  @throw CTLib::KMPError If the specified ENPH group is `nullptr` or
         *  `this`, or not owned by the same KMP as this ENPH group, or this
         *  ENPH group already has MAX_ENPH_LINKS next groups.
         */
        void addNext(ENPH* next);

        /*! @brief Removes the specified ENPH group from the list of next groups
         *  of this ENPH group.
         * 
         *  @param[in] next The next ENPH group to be removed
         * 
         *  @throw CTLib::KMPError If the specified ENPH group is `nullptr` or
         *  `this`, or is not a next group of this ENPH group.
         */
        void removeNext(ENPH* next);

        /*! @brief Returns the first ENPT point that is part of this group, or
         *  `nullptr` if not set.
         */
        ENPT* getFirst() const;

        /*! @brief Returns the last ENPT point that is part of this group, or
         *  `nullptr` if not set.
         */
        ENPT* getLast() const;

        /*! @brief Returns the previous ENPH group count of this ENPH group. */
        uint8_t getPreviousCount() const;

        /*! @brief Returns the previous ENPH groups of this ENPH group. */
        std::vector<ENPH*> getPrevious() const;

        /*! @brief Returns the next ENPH group count of this ENPH group. */
        uint8_t getNextCount() const;

        /*! @brief Returns the next ENPH groups of this ENPH group. */
        std::vector<ENPH*> getNext() const;

    protected:

        //! invoked when a section entry is added
        void sectionAdded(Section* section) override;

        //! invoked when a section entry is removed
        void sectionRemoved(Section* section) override;

    private:

        // does nothing
        static void assertCanAdd(KMP* kmp);

        ENPH(KMP* kmp);

        // throws if 'links.size()' >= 'MAX_ENPH_LINKS'
        void assertCanAddLink(const std::vector<ENPH*>& links) const;

        // pointer to first ENPT entry in this group
        ENPT* first;

        // pointer to last ENPT entry in this group
        ENPT* last;

        // vector containing pointers to all previous ENPH groups
        std::vector<ENPH*> prevs;

        // vector containing pointers to all following ENPH groups
        std::vector<ENPH*> nexts;
    };

    /*! @brief Constructs an empty KMP object. */
    KMP();

    /*! @brief Delete copy constructor for move-only class. */
    KMP(const KMP&) = delete;

    /*! @brief Moves the contents of the specified KMP to this newly created
     *  one.
     *  
     *  The source KMP will then be empty, but can still be safely used.
     *  
     *  @param[in] src The KMP to be moved
     */
    KMP(KMP&& src);

    ~KMP();

    /*! @brief Adds a section entry of the specified template type.
     *  
     *  @tparam Type The Section type
     * 
     *  @throw CTLib::KMPError If the maximum number of entries per KMP for the
     *  specified template type was reached by this KMP.
     * 
     *  @return The newly created section entry
     */
    template <class Type>
    Type* add();

    /*! @brief Returns the section entry of the specified template type at the
     *  specified index.
     *  
     *  @tparam Type The Section type
     * 
     *  @param[in] index The Section index
     * 
     *  @throw CTLib::KMPError If the specified index is more than or equal to
     *  the entry count of the specified template type.
     * 
     *  @return The section entry at the specified index
     */
    template <class Type>
    Type* get(uint16_t index) const;

    /*! @brief Removes and _deletes_ the section entry of the specified template
     *  type at the specified index.
     * 
     *  If you have any reference left to the section entry to be removed, **DO
     *  NOT USE IT, AS IT WILL BE `DELETE`D BY THIS FUNCTION**.
     *  
     *  @tparam Type The Section type
     * 
     *  @param[in] index The Section index
     * 
     *  @throw CTLib::KMPError If the specified index is more than or equal to
     *  the entry count of the specified template type.
     */
    template <class Type>
    void remove(uint16_t index);

    /*! @brief Returns the index of the specified section entry in this KMP, or
     *  -1 if not found.
     * 
     *  @tparam Type The Section type
     * 
     *  @param[in] entry The Section entry
     * 
     *  @return The index of the specified entry, or -1.
     */
    template <class Type>
    int16_t indexOf(Type* entry) const;

    /*! @brief Returns a std::vector containing all section entries of the
     *  specified template type.
     * 
     *  @tparam Type The Section type
     * 
     *  @return The entries of the specified template type
     */
    template <class Type>
    std::vector<Type*> getAll() const;

    /*! @brief Returns the section entry count of the specified template type.
     * 
     *  @tparam Type The Section type
     * 
     *  @return The entry count of the specified template type
     */
    template <class Type>
    uint16_t count() const;

    /*! @brief Registers the specified SectionCallback. */
    void registerCallback(SectionCallback* cb);

    /*! @brief Unregisters the specified SectionCallback. */
    void unregisterCallback(SectionCallback* cb);

private:

    // calls SectionCallback::sectionAdded() on all registered callbacks
    void invokeSectionCallbacksAdd(Section* section) const;

    // calls SectionCallback::sectionRemoved() on all registered callbacks
    void invokeSectionCallbacksRemove(Section* section) const;

    std::vector<KTPT*> ktpts;
    std::vector<ENPT*> enpts;
    std::vector<ENPH*> enphs;

    // vector containing all section callbacks
    std::vector<SectionCallback*> callbacks;
};

/*! @brief KMPError is the error class used by the methods in this header. */
class KMPError final : public std::runtime_error
{

public:

    /*! @brief Constructs a CTLib::KMPError with the specified error message. */
    KMPError(const char* msg);

    /*! @brief Constructs a CTLib::KMPError with the specified error message. */
    KMPError(const std::string& msg);
};

/*! @} addtogroup kmp */
}
