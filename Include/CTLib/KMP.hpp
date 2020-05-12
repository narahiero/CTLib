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

    class KTPT;
    class ENPT;
    class ENPH;
    class ITPT;
    class ITPH;
    class CKPT;
    class CKPH;
    class GOBJ;
    class POTI;
    class AREA;
    class CAME;
    class JGPT;
    class CNPT;
    class MSPT;
    class STGI;

    /*! @brief Reads a KMP from the specified Buffer.
     *  
     *  @throw CTLib::KMPError If the specified data is invalid.
     */
    static KMP read(Buffer& data);

    /*! @brief Writes the specified KMP to a new Buffer. */
    static Buffer write(const KMP& kmp);

    /*! @brief Enumeration of the possible section type values. */
    enum class SectionType
    {
        /*! @brief KTPT section. (_Kart Point_) */
        KTPT,

        /*! @brief ENPT section. (_Enemy Point_) */
        ENPT,

        /*! @brief ENPH section. (_Enemy Path_) */
        ENPH,

        /*! @brief ITPT section. (_Item Point_) */
        ITPT,

        /*! @brief ITPH section. (_Item Path_) */
        ITPH,

        /*! @brief CKPT section. (_Checkpoint_) */
        CKPT,

        /*! @brief CKPH section. (_Checkpoint Group_) */
        CKPH,

        /*! @brief GOBJ section. (_Game Object_) */
        GOBJ,

        /*! @brief POTI section. (_Point Information_) */
        POTI,

        /*! @brief AREA section. (_Area_) */
        AREA,

        /*! @brief CAME section. (_Camera_) */
        CAME,

        /*! @brief JGPT section. (_Jugem Point_) */
        JGPT,

        /*! @brief CNPT section. (_Cannon Point_) */
        CNPT,

        /*! @brief MSPT section. (_Mission Point_) */
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

    /*! @brief Superclass of ENPH, ITPH, and CKPH, as they are identical. */
    template <class PH, class PT>
    class GroupSection : public Section, public SectionCallback
    {

        friend class KMP;

    public:

        /*! @brief The maximum number of group links. */
        static constexpr uint8_t MAX_LINKS = 6;

        ~GroupSection();

        /*! @brief Sets the first point that is part of this group.
         *  
         *  Passing `nullptr` will set the first and last entries of this group
         *  to that value.
         *  
         *  If the specified entry is removed from its KMP after this function
         *  returns, the first entry will be replaced by the entry directly
         *  after it, or `nullptr` if it also was the last entry.
         * 
         *  @param[in] first The first entry
         * 
         *  @throw CTLib::KMPError If the specified entry is not owned by the
         *  same KMP as this group, or the specified entry is already owned by
         *  another group, or the last entry of this group is set and its index
         *  is lower than the specified entry's.
         */
        virtual void setFirst(PT* first);

        /*! @brief Sets the last point that is part of this group.
         *  
         *  If the specified entry is removed from its KMP after this function
         *  returns, the last entry of this group will be replaced by the entry
         *  directly before it, or `nullptr` if it also was the first entry.
         * 
         *  @param[in] last The last entry
         * 
         *  @throw CTLib::KMPError If the specified entry is not owned by the
         *  same KMP as this group, or the specified entry is already owned by
         *  another group, or the first entry of this group is not set, or the
         *  first entry of this group is set and its index is higher than the
         *  specified entry's.
         */
        virtual void setLast(PT* last);

        /*! @brief Adds the specified group to the list of previous groups of
         *  this group.
         * 
         *  If the specified group is removed from its KMP after this function
         *  returns, it will be removed from the list of previous groups of this
         *  group.
         * 
         *  @param[in] prev The group to be added to the previous groups
         * 
         *  @throw CTLib::KMPError If the specified group is `nullptr`, or not
         *  owned by the same KMP as this group, or this group already has
         *  MAX_LINKS previous groups.
         */
        void addPrevious(PH* prev);

        /*! @brief Removes the specified group from the list of previous groups
         *  of this group.
         * 
         *  @param[in] prev The previous group to remove
         * 
         *  @throw CTLib::KMPError If the specified group is `nullptr`, or is
         *  not a previous group of this group.
         */
        void removePrevious(PH* prev);

        /*! @brief Adds the specified group to the list of next groups of this
         *  group.
         * 
         *  If the specified group is removed from its KMP after this function
         *  returns, it will be removed from the list of next groups of this
         *  group.
         * 
         *  @param[in] next The group to be added to the next groups
         * 
         *  @throw CTLib::KMPError If the specified group is `nullptr`, or not
         *  owned by the same KMP as this group, or this group already has
         *  MAX_LINKS next groups.
         */
        void addNext(PH* next);

        /*! @brief Removes the specified group from the list of next groups of
         *  this group.
         * 
         *  @param[in] next The next group to be removed
         * 
         *  @throw CTLib::KMPError If the specified group is `nullptr`, or is
         *  not a next group of this group.
         */
        void removeNext(PH* next);

        /*! @brief Returns the first point that is part of this group, or
         *  `nullptr` if not set.
         */
        PT* getFirst() const;

        /*! @brief Returns the last point that is part of this group, or
         *  `nullptr` if not set.
         */
        PT* getLast() const;

        /*! @brief Returns the previous group count of this group. */
        uint8_t getPreviousCount() const;

        /*! @brief Returns the previous groups of this group. */
        std::vector<PH*> getPrevious() const;

        /*! @brief Returns the next group count of this group. */
        uint8_t getNextCount() const;

        /*! @brief Returns the next groups of this group. */
        std::vector<PH*> getNext() const;

    protected:

        //! invoked when a section entry is added
        void sectionAdded(Section* section) override;

        //! invoked when a section entry is removed
        void sectionRemoved(Section* section) override;

    private:

        GroupSection(KMP* kmp);

        // sets the parent of entries between 'first' and 'last' to 'value'
        void setParent(PT* first, PT* last, PH* value);

        // throws if 'entry->parent' != `nullptr`
        void assertNoParent(PT* entry) const;

        // throws if 'indexOf(last)' < 'indexOf(first)'
        void assertValidFirst(PT* first) const;

        // throws if 'first' == `nullptr` or 'indexOf(first)' > 'indexOf(last)'
        void assertValidLast(PT* last) const;

        // throws if 'links.size()' >= 'MAX_LINKS'
        void assertCanAddLink(const std::vector<PH*>& links) const;

        // pointer to first point in this group
        PT* first;

        // pointer to last point in this group
        PT* last;

        // vector containing pointers to all previous groups
        std::vector<PH*> prevs;

        // vector containing pointers to all following groups
        std::vector<PH*> nexts;
    };

    /*! @brief Superclass of ENPT, ITPT, and CKPT, for parent link. */
    template <class PH>
    class PointSection : public Section, public SectionCallback
    {

        friend class KMP;

    public:

        ~PointSection();

        /*! @brief Returns the group owning this entry. */
        PH* getParent() const;

    protected:

        //! invoked when a section entry is added
        void sectionAdded(Section* section) override;

        //! invoked when a section entry is removed
        void sectionRemoved(Section* section) override;

    private:

        PointSection(KMP* kmp);

        // pointer to parent group
        PH* parent;
    };

    /*! @brief Starting positions of racers. */
    class KTPT final : public Section
    {

        friend class KMP;

    public:

        /*! @brief The SectionType of this class. */
        static constexpr SectionType TYPE = SectionType::KTPT;

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
         *  index must be 0.
         * 
         *  For battle tracks, there must be 12 KTPT entries (each with unique
         *  index), with indices 0 to 5 for red team and indices 6 to 11 for
         *  blue team.
         * 
         *  An additional KTPT entry can be added with index -1 to specify start
         *  line position if using LE-CODE. This additional entry **MUST** be
         *  added to the KMP **after** the default one (index 0).
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
    class ENPT final : public PointSection<ENPH>
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

        /*! @brief The SectionType of this class. */
        static constexpr SectionType TYPE = SectionType::ENPT;

        /*! @brief Maximum number of ENPT entries per KMP. */
        static constexpr uint16_t MAX_ENTRY_COUNT = 255;

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

        // throws if specified KMP already has 'MAX_ENTRY_COUNT' ENPTs
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
    class ENPH final : public GroupSection<ENPH, ENPT>
    {

        friend class KMP;

    public:

        /*! @brief The SectionType of this class. */
        static constexpr SectionType TYPE = SectionType::ENPH;

        ~ENPH();

        /*! @brief Returns SectionType::ENPH. */
        SectionType getType() const override;

    private:

        // does nothing
        static void assertCanAdd(KMP* kmp);

        ENPH(KMP* kmp);
    };

    /*! @brief Item route points. */
    class ITPT final : public PointSection<ITPH>
    {

        friend class KMP;

    public:

        /*! @brief Enumeration of the possible bullet bill control values. */
        enum class BulletControl
        {
            /*! @brief Follow item route exactly, but gets affected by gravity
             *  if below route height. _Recommended_
             */
            Default,

            /*! @brief Follow item route horizontally and gets affected by
             *  gravity. Good if point height is poorly set.
             */
            Falling,

            /*! @brief Follow item route exactly. */
            Exact,

            /*! @brief Same as Default, but for bouncy mushrooms. */
            Mushroom
        };

        /*! @brief The SectionType of this class. */
        static constexpr SectionType TYPE = SectionType::ITPT;

        /*! @brief Maximum number of ITPT entries per KMP. */
        static constexpr uint16_t MAX_ENTRY_COUNT = 255;

        ~ITPT();

        /*! @brief Returns SectionType::ITPT. */
        SectionType getType() const override;

        /*! @brief Sets the position of this point. */
        void setPosition(Vector3f position);

        /*! @brief Sets the bullet bill range of this point.
         *  
         *  This value controls how much the player can move left or right from
         *  this point when using a bullet bill.
         */
        void setBulletRange(float range);

        /*! @brief Sets the BulletControl of this point. */
        void setBulletControl(BulletControl control);

        /*! @brief Sets whether this point prevents the bullet bill from
         *  dropping the player.
         */
        void setForceBullet(bool force);

        /*! @brief Sets whether this point should be ignored by red/blue shells.
         *  
         *  **Note**: This value only works on the first point of a ITPH group.
         */
        void setShellIgnore(bool ignore);

        /*! @brief Returns the position of this point. */
        Vector3f getPosition() const;

        /*! @brief Returns the bullet bill range of this point. */
        float getBulletRange() const;

        /*! @brief Returns the BulletControl of this point. */
        BulletControl getBulletControl() const;

        /*! @brief Returns whether this point prevents the bullet bill from
         *  dropping the player.
         */
        bool isForceBullet() const;

        /*! @brief Returns whether this point should be ignored by shells. */
        bool isShellIgnore() const;

    private:

        // throws if specified KMP already has 'MAX_ENTRY_COUNT' ITPTs
        static void assertCanAdd(KMP* kmp);

        ITPT(KMP* kmp);

        // position
        Vector3f pos;

        // max bullet bill deviation from pos
        float bulletRange;

        // bullet bill control
        BulletControl bulletCtrl;

        // prevent bullet from dropping
        bool forceBullet;

        // do not follow players taking this route
        bool shellIgnore;
    };

    /*! @brief Item route point groups. */
    class ITPH final : public GroupSection<ITPH, ITPT>
    {

        friend class KMP;

    public:

        /*! @brief The SectionType of this class. */
        static constexpr SectionType TYPE = SectionType::ITPH;

        ~ITPH();

        /*! @brief Returns SectionType::ITPH. */
        SectionType getType() const override;

    private:

        // does nothing
        static void assertCanAdd(KMP* kmp);

        ITPH(KMP* kmp);
    };

    /*! @brief Checkpoints. */
    class CKPT final : public PointSection<CKPH>
    {

        friend class KMP;

    public:

        /*! @brief The SectionType of this class. */
        static constexpr SectionType TYPE = SectionType::CKPT;

        /*! @brief Maximum number of CKPT entries per KMP. */
        static constexpr uint16_t MAX_ENTRY_COUNT = 255;

        ~CKPT();

        /*! @brief Returns SectionType::CKPT. */
        SectionType getType() const override;

        /*! @brief Sets the position of the left point of the checkpoint line.
         */
        void setLeft(Vector2f position);

        /*! @brief Sets the position of the right point of the checkpoint line.
         */
        void setRight(Vector2f position);

        /*! @brief Sets the JGPT respawn point of this checkpoint.
         *  
         *  @param[in] respawn The JGPT respawn point
         * 
         *  @throw CTLib::KMPError If the specified JGPT entry is not owned by
         *  the same KMP as this CKPT.
         */
        void setRespawn(JGPT* respawn);

        /*! @brief Sets the type ID of this checkpoint.
         *  
         *  `0x00`: Lap count trigger checkpoint (using more than one in a
         *  single track will cause a position glitch online).
         * 
         *  `0x01` - `0xFE`: Key checkpoint (all of them must be passed in order
         *  for the lap count trigger to work).
         * 
         *  `0xFF`: Regular checkpoint (can be skipped, used for positioning and
         *  respawn points).
         */
        void setTypeID(uint8_t type);

        /*! @brief Returns the position of the left point of the checkpoint
         *  line.
         */
        Vector2f getLeft() const;

        /*! @brief Returns the position of the right point of the checkpoint
         *  line.
         */
        Vector2f getRight() const;

        /*! @brief Returns the JGPT respawn point of this checkpoint, `nullptr`
         *  if not set.
         */
        JGPT* getRespawn() const;

        /*! @brief Returns the type ID of this checkpoint. */
        uint8_t getTypeID() const;

    protected:

        //! invoked when a section entry is added
        void sectionAdded(Section* section) override;

        //! invoked when a section entry is removed
        void sectionRemoved(Section* section) override;

    private:

        // throws if specified KMP already has 'MAX_ENTRY_COUNT' CKPTs
        static void assertCanAdd(KMP* kmp);

        CKPT(KMP* kmp);

        // position of left line point
        Vector2f left;

        // position of right line point
        Vector2f right;

        // respawn point
        JGPT* respawn;

        // type ID
        uint8_t type;
    };

    /*! @brief Checkpoint groups. */
    class CKPH final : public GroupSection<CKPH, CKPT>
    {

        friend class KMP;

    public:

        /*! @brief The SectionType of this class. */
        static constexpr SectionType TYPE = SectionType::CKPH;

        ~CKPH();

        /*! @brief Returns SectionType::CKPH. */
        SectionType getType() const override;

    private:

        // does nothing
        static void assertCanAdd(KMP* kmp);

        CKPH(KMP* kmp);
    };

    /*! @brief Game objects. */
    class GOBJ final : public Section
    {

        friend class KMP;

    public:

        /*! @brief The SectionType of this class. */
        static constexpr SectionType TYPE = SectionType::GOBJ;

        ~GOBJ();

        /*! @brief Returns SectionType::GOBJ. */
        SectionType getType() const override;

    private:

        // does nothing
        static void assertCanAdd(KMP* kmp);

        GOBJ(KMP* kmp);
    };

    /*! @brief Game object routes. */
    class POTI final : public Section
    {

        friend class KMP;

    public:

        /*! @brief The SectionType of this class. */
        static constexpr SectionType TYPE = SectionType::POTI;

        ~POTI();

        /*! @brief Returns SectionType::POTI. */
        SectionType getType() const override;

    private:

        // does nothing
        static void assertCanAdd(KMP* kmp);

        POTI(KMP* kmp);
    };

    /*! @brief Areas. */
    class AREA final : public Section
    {

        friend class KMP;

    public:

        /*! @brief The SectionType of this class. */
        static constexpr SectionType TYPE = SectionType::AREA;

        ~AREA();

        /*! @brief Returns SectionType::AREA. */
        SectionType getType() const override;

    private:

        // does nothing
        static void assertCanAdd(KMP* kmp);

        AREA(KMP* kmp);
    };

    /*! @brief Cameras. */
    class CAME final : public Section
    {

        friend class KMP;

    public:

        /*! @brief The SectionType of this class. */
        static constexpr SectionType TYPE = SectionType::CAME;

        ~CAME();

        /*! @brief Returns SectionType::CAME. */
        SectionType getType() const override;

    private:

        // does nothing
        static void assertCanAdd(KMP* kmp);

        CAME(KMP* kmp);
    };

    /*! @brief Respawn points. */
    class JGPT final : public Section
    {

        friend class KMP;

    public:

        /*! @brief The SectionType of this class. */
        static constexpr SectionType TYPE = SectionType::JGPT;

        /*! @brief Maximum number of JGPT entries per KMP. */
        static constexpr uint16_t MAX_ENTRY_COUNT = 256;

        ~JGPT();

        /*! @brief Returns SectionType::JGPT. */
        SectionType getType() const override;

        /*! @brief Sets the position of this respawn point. */
        void setPosition(Vector3f position);

        /*! @brief Sets the rotation of this respawn point. */
        void setRotation(Vector3f rotation);

        /*! @brief Returns the position of this respawn point. */
        Vector3f getPosition() const;

        /*! @brief Returns the rotation of this respawn point. */
        Vector3f getRotation() const;

    private:

        // throws if specified KMP already has 'MAX_ENTRY_COUNT' JGPTs
        static void assertCanAdd(KMP* kmp);

        JGPT(KMP* kmp);

        // position
        Vector3f pos;

        // rotation
        Vector3f rot;

        float range;
    };

    /*! @brief Cannon points. */
    class CNPT final : public Section
    {

        friend class KMP;

    public:

        /*! @brief Enumeration of the possible cannon type values. */
        enum class CannonType : int16_t
        {
            /*! @brief Fast cannon shooting straight. */
            Default = 0,

            /*! @brief Fast cannon shooting curved. */
            Curved = 1,

            /*! @brief Slow cannon shooting curved. */
            Slow = 2,

            /*! @brief Custom value. (Same as Default) */
            Custom = -1
        };

        /*! @brief The SectionType of this class. */
        static constexpr SectionType TYPE = SectionType::CNPT;

        ~CNPT();

        /*! @brief Returns SectionType::CNPT. */
        SectionType getType() const override;

        /*! @brief Sets the destination of this cannon. */
        void setDestination(Vector3f destination);

        /*! @brief Sets the direction of this cannon.
         *  
         *  **Note**: Only the Y axis is required. The X and Z axes are used to
         *  rotate the vehicle in the landing area.
         */
        void setDirection(Vector3f direction);

        /*! @brief Sets the CannonType of this cannon.
         *  
         *  **Note**: This method sets the same property as the method
         *  `setTypeID(int16_t)`.
         */
        void setCannonType(CannonType type);

        /*! @brief Sets the type ID of this cannon. (Useful if using LE-CODE)
         *  
         *  Setting the type ID to a value higher than 2 requires LE-CODE and
         *  will make the track incompatible with distributions without LE-CODE.
         * 
         *  A negative type ID will be clamped to 0 (Default) by the game.
         *  
         *  **Note**: This method sets the same property as the method
         *  `setCannonType(CannonType)`.
         */
        void setTypeID(int16_t type);

        /*! @brief Returns the destination of this cannon. */
        Vector3f getDestination() const;

        /*! @brief Returns the direction of this cannon. */
        Vector3f getDirection() const;

        /*! @brief Returns the CannonType of this cannon.
         *  
         *  **Note**: The value CannonType::Custom is returned if type was set
         *  to a non-convertible value using `setTypeID(int16_6)`.
         */
        CannonType getCannonType() const;

        /*! @brief Returns the type ID of this cannon. */
        int16_t getTypeID() const;

    private:

        // does nothing
        static void assertCanAdd(KMP* kmp);

        CNPT(KMP* kmp);

        // cannon destination
        Vector3f dest;

        // cannon direction
        Vector3f rot;

        // cannon type
        int16_t type;
    };

    /*! @brief Position of racers on battle and competition end. */
    class MSPT final : public Section
    {

        friend class KMP;

    public:

        /*! @brief The SectionType of this class. */
        static constexpr SectionType TYPE = SectionType::MSPT;

        ~MSPT();

        /*! @brief Returns SectionType::MSPT. */
        SectionType getType() const override;

        /*! @brief Sets the position of this battle end point. */
        void setPosition(Vector3f position);

        /*! @brief Sets the rotation of this battle end point. */
        void setRotation(Vector3f rotation);

        /*! @brief Returns the position of this battle end point. */
        Vector3f getPosition() const;

        /*! @brief Returns the rotation of this battle end point. */
        Vector3f getRotation() const;

    private:

        // does nothing
        static void assertCanAdd(KMP* kmp);

        MSPT(KMP* kmp);

        // position
        Vector3f pos;

        // rotation
        Vector3f rot;
    };

    /*! @brief Stage information. */
    class STGI final : public Section
    {

        friend class KMP;

    public:

        /*! @brief Enumeration of the possible race start side values. */
        enum class StartSide : uint8_t
        {
            /*! @brief First place starts on the left side of the finish line. */
            Left = 0x0,

            /*! @brief First place starts on the right side of the finish line. */
            Right = 0x1
        };

        /*! @brief The SectionType of this class. */
        static constexpr SectionType TYPE = SectionType::STGI;

        /*! @brief Maximum number of STGI entries per KMP. */
        static constexpr uint16_t MAX_ENTRY_COUNT = 1;

        /*! @brief The maximum number of laps. */
        static constexpr uint8_t MAX_LAP_COUNT = 9;

        ~STGI();

        /*! @brief Returns SectionType::STGI. */
        SectionType getType() const override;

        /*! @brief Sets the lap count of the race track.
         *  
         *  **Note**: The
         *  <a href="http://wiki.tockdom.com/wiki/Lap_%26_Speed_Modifier">
         *  Lap & Speed Modifier</a> cheat code is **required** if using any
         *  other value than 3.
         * 
         *  **Note 2**: Using a lap count of 9 will show 8 laps in-game, but
         *  the player must still complete 9 laps.
         *  
         *  @param[in] count The lap count
         * 
         *  @throw CTLib::KMPError If the specified count is less than 1 or
         *  more than MAX_LAP_COUNT.
         */
        void setLapCount(uint8_t count);

        /*! @brief Sets which side of the finish line the racer in first place
         *  begins the race.
         */
        void setStartSide(StartSide side);

        /*! @brief Sets whether the starting positions narrow mode is enabled. */
        void setNarrowMode(bool enable);

        /*! @brief Sets whether lens flare flashing is enabled. */
        void setLensFlareEnabled(bool enable);

        /*! @brief Sets lens flare flashing colour, formatted as `0xRRGGBBAA`. */
        void setLensFlareColour(uint32_t colour);

        /*! @brief Sets the speed factor of the track.
         *  
         *  **Note**: The
         *  <a href="http://wiki.tockdom.com/wiki/Lap_%26_Speed_Modifier">
         *  Lap & Speed Modifier</a> cheat code is **required** if using any
         *  other value than `1.0`.
         * 
         *  @param[in] factor The speed factor
         * 
         *  @throw CTLib::KMPError If the specified speed factor is less than or
         *  equal to `0.0`.
         */
        void setSpeedFactor(float factor);

        /*! @brief Returns the lap count of the race track. */
        uint8_t getLapCount() const;

        /*! @brief Returns which side of the finish line the racer in first
         *  place begins the race.
         */
        StartSide getStartSide() const;

        /*! @brief Returns whether narrow mode is enabled. */
        bool isNarrowMode() const;

        /*! @brief Returns whether lens flare flashing is enabled. */
        bool isLensFlareEnabled() const;

        /*! @brief Returns the lens flare flashing colour. */
        uint32_t getLensFlareColour() const;

        /*! @brief Returns the speed factor of the track. */
        float getSpeedFactor() const;

    private:

        // throws if specified KMP already has a STGI entry
        static void assertCanAdd(KMP* kmp);

        STGI(KMP* kmp);

        // throws if 'count' < 1 or > 'MAX_LAP_COUNT'
        void assertValidLapCount(uint8_t count) const;

        // throws if 'factor' <= 0.f
        void assertValidSpeedFactor(float factor) const;

        // lap count (CHEAT CODE REQUIRED)
        uint8_t lapCount;

        // race start side of racers
        StartSide startSide;

        // whether lens flare flashing is enabled
        bool lensFlare;

        // lens flare flashing colour, formatted as 0xRRGGBBAA
        uint32_t lensColour;

        // whether to use narrow starting positions
        bool narrow;

        // speed factor (CHEAT CODE REQUIRED)
        float speedFactor;
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
    std::vector<ITPT*> itpts;
    std::vector<ITPH*> itphs;
    std::vector<CKPT*> ckpts;
    std::vector<CKPH*> ckphs;
    std::vector<GOBJ*> gobjs;
    std::vector<POTI*> potis;
    std::vector<AREA*> areas;
    std::vector<CAME*> cames;
    std::vector<JGPT*> jgpts;
    std::vector<CNPT*> cnpts;
    std::vector<MSPT*> mspts;
    std::vector<STGI*> stgis;

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
