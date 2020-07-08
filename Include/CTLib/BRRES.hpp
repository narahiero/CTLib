//////////////////////////////////////////////////
//  Copyright (c) 2020 Nara Hiero
//
// This file is licensed under GPLv3+
// Refer to the `License.txt` file included.
//////////////////////////////////////////////////

#pragma once

/*! @file BRRES.hpp
 * 
 *  @brief The header for BRRES module.
 */


#include <map>
#include <stdexcept>
#include <vector>

#include <CTLib/Image.hpp>
#include <CTLib/Math.hpp>
#include <CTLib/Memory.hpp>


namespace CTLib
{

/*! @defgroup brres BRRES
 * 
 *  @addtogroup brres
 * 
 *  @brief The BRRES module contains all classes and methods that can be used to
 *  interact with Nintendo's BRRES data format and subformats.
 *  @{
 */

class BRRES;

class MDL0;
class TEX0;

/*! @brief Superclass of all BRRES sub files. */
class BRRESSubFile
{

    friend class BRRES;
    
public:

    virtual ~BRRESSubFile();

    /*! @brief Returns the BRRES owning this subfile. */
    BRRES* getBRRES() const;

    /*! @brief Returns the name of this subfile. */
    std::string getName() const;

protected:

    //! all subclasses constructors must delegate to this
    BRRESSubFile(BRRES* brres, const std::string& name);

    //! BRRES object owning this sub file
    BRRES* brres;

    //! name of this subfile
    std::string name;

private:

    BRRESSubFile(const BRRESSubFile&) = delete;

    BRRESSubFile(BRRESSubFile&&) = delete;
};

/*! @brief Called when a BRRES subfile is added or removed. */
class BRRESSubFileCallback
{

    friend class BRRES;

protected:

    /*! @brief Called when a subfile is added. */
    virtual void subfileAdded(BRRESSubFile* subfile) = 0;

    /*! @brief Called when a subfile is removed. */
    virtual void subfileRemoved(BRRESSubFile* subfile) = 0;
};

/*! @brief A model within a BRRES. */
class MDL0 final : public BRRESSubFile
{

    friend class BRRES;

    friend class Links;
    friend class Bone;
    friend class Object;
    friend class TextureLink;

private:

    template <class Type> class SectionContainer;

public:

    class Links;
    class Bone;
    class VertexArray;
    class NormalArray;
    class ColourArray;
    class TexCoordArray;
    class Material;
    class Shader;
    class Object;
    class TextureLink;

    /*! @brief Enumeration of the MDL0 sections. */
    enum class SectionType
    {
        /*! @brief Links section (#0) */
        Links = 0x0,

        /*! @brief Bone section (#1) */
        Bone = 0x1,

        /*! @brief VertexArray section (#2) */
        VertexArray = 0x2,

        /*! @brief NormalArray section (#3) */
        NormalArray = 0x3,

        /*! @brief ColourArray section (#4) */
        ColourArray = 0x4,

        /*! @brief TexCoordArray section (#5) */
        TexCoordArray = 0x5,

        /*! @brief Material section (#8) */
        Material = 0x8,

        /*! @brief Shader section (#9) */
        Shader = 0x9,

        /*! @brief Object section (#10) */
        Object = 0xA,

        /*! @brief TextureLink section (#11) */
        TextureLink = 0xB,

        /*! @brief An instance of one of the section superclasses. */
        NONE = ~0x0
    };

    /*! @brief The superclass of all sections in a MDL0. */
    class Section
    {

    public:

        /*! @brief Returns a string representation of the specified type. */
        static const char* nameForType(SectionType type);

        virtual ~Section();

        /*! @brief Returns the SectionType of this section. */
        virtual SectionType getType() const;

        /*! @brief Returns the MDL0 owning this section. */
        MDL0* getMDL0() const;

        /*! @brief Returns the name of this section. */
        std::string getName() const;

    protected:

        //! constructs a section for the specified MDL0
        Section(MDL0* mdl0, const std::string& name);

        //! pointer to the MDL0 owning this section object
        MDL0* mdl0;

        //! name of this section
        std::string name;

    private:

        Section(const Section&) = delete;
        Section(Section&&) = delete;
    };

    /*! @brief The superclass of sections containing only an array of data. */
    class ArraySection : public Section
    {

    public:

        ~ArraySection();

        /*! @brief Returns the number of elements in this section. */
        uint16_t getCount() const;

        /*! @brief Returns the data of this section. */
        Buffer getData() const;

    protected:

        //! simply invokes the Section's constructor with matching arguments
        ArraySection(MDL0* mdl0, const std::string& name);

        //! element count
        uint16_t count;

        //! buffer containing actual array data
        Buffer data;
    };

    /*! @brief Superclass of sections that needs an entry callback. */
    class SectionCallback
    {

        friend class MDL0;

    public:

        virtual ~SectionCallback();

    protected:

        //! entry callback
        virtual void entryCallback(Section* instance, bool add) = 0;
    };

    ////////////////////////////////////////////////////////
    ///  Actual section classes
    ////////////////////////////////////////////////////////

    /*! @brief MDL0 sections links. (Section #0) */
    class Links final : public Section, public SectionCallback
    {

        friend class MDL0;

        template <class> friend class SectionContainer;

    public:

        /*! @brief Enumeration of the possible Links type. */
        enum class Type
        {
            /*! @brief Specifies bone hierarchy. */
            NodeTree = 0x02,

            /*! @brief Links Objects, Materials, and Bones. */
            DrawOpa = 0x04,
        };

        /*! @brief A link in the DrawOpa section. */
        struct DrawOpaLink
        {
            /*! @brief The Object linked. */
            Object* obj;

            /*! @brief The Material linked. */
            Material* mat;

            /*! @brief The Bone linked. */
            Bone* bone;
        };

        /*! @brief The SectionType of this class. */
        constexpr static SectionType TYPE = SectionType::Links;

        /*! @brief Returns the section name for the specified type. */
        static std::string nameForType(Type type);

        ~Links();

        /*! @brief Returns SectionType::Links. */
        SectionType getType() const override;

        /*! @brief Returns the Type of this links section. */
        Type getLinksType() const;

        /*! @brief Returns the command count of this links section. */
        uint32_t getCount() const;

        /*! @brief Links the specified Object, Material, and Bone together.
         *  
         *  @param[in] obj The Object to link
         *  @param[in] mat The Material to link
         *  @param[in] bone The Bone to link
         * 
         *  @throw CTLib::BRRESError If this section is not of type
         *  Type::DrawOpa.
         */
        void link(Object* obj, Material* mat, Bone* bone);

        /*! @brief Returns a std::vector containing all DrawOpaLinks.
         *  
         *  @throw CTLib::BRRESError If this section is not of type
         *  Type::DrawOpa.
         */
        std::vector<DrawOpaLink> getLinks() const;

    private:

        Links(MDL0* mdl0, const std::string& name);

        // private ctor used internally
        Links(MDL0* mdl0, Type type);

        // called when a section entry is added/removed
        void entryCallback(Section* instance, bool add) override;

        // throws if 'linksType' is not Type::DrawOpa
        void assertDrawOpaSection() const;

        const Type linksType;

        // vector containing the links of the DrawOpa section
        std::vector<DrawOpaLink> drawOpaLinks;
    };

    /*! @brief A single bone of a MDL0. (Section #1) */
    class Bone final : public Section
    {

        friend class MDL0;

        template <class> friend class SectionContainer;

    public:

        /*! @brief The SectionType of this class. */
        constexpr static SectionType TYPE = SectionType::Bone;

        /*! @brief Returns the bone after the specified one in a 'flat'
         *  hierarchy.
         *  
         *  The logic goes as follows:
         * 
         *  If the specified bone has any child, return the first child.
         *  Else, if the specified bone has a 'next' sibling, return it.
         *  Else, if the specified bone has at least one parent, returns the
         *  'next' sibling of the first parent to have one, if any.
         *  Else return `nullptr`.
         * 
         *  If the root bone of a MDL0 is passed and this method is repeatedly
         *  invoked with the return value of the previous call until `nullptr`
         *  is returned, then all bones in the MDL0 will be returned exactly
         *  once.
         * 
         *  Passing `nullptr` will simply make this function return `nullptr`
         *  back.
         * 
         *  @param[in] bone The bone to query the 'flat next'
         * 
         *  @return The 'flat next' of the specified bone
         */
        static Bone* flatNext(Bone* bone);

        ~Bone();

        /*! @brief Returns SectionType::Bone. */
        SectionType getType() const override;

        /*! @brief Creates, inserts, and returns a bone with the specified name
         *  as a child of this bone.
         * 
         *  The created bone will also be added to the MDL0 owning this bone.
         *  
         *  **Note**: The bone hierarchy (next and previous of childs) will be
         *  modified by this method, as childs are ordered alphabetically.
         * 
         *  @param[in] name The name of the bone to be created
         * 
         *  @throw CTLib::BRRESError If another bone with the specified name
         *  exists in the MDL0 owning this bone.
         * 
         *  @return The newly created bone
         */
        Bone* insert(const std::string& name);

        /*! @brief Moves this bone as a child of the specified bone, `nullptr`
         *  to move at root level.
         * 
         *  This bone will keep its children, and its children will not be
         *  modified, but this bone's siblings will be changed.
         *  
         *  @param[in] bone The new parent of this bone
         * 
         *  @throw CTLib::BRRESError If the specified bone is a child of this
         *  bone.
         */
        void moveTo(Bone* bone);

        /*! @brief Returns the parent bone, or `nullptr` if none. */
        Bone* getParent() const;

        /*! @brief Returns whether the specified bone is a child of this.
         *  
         *  @throw CTLib::BRRESError If the specified bone is `nullptr`.
         */
        bool isChild(Bone* bone) const;

        /*! @brief Returns whether the specified bone is a direct child of this.
         *  
         *  @throw CTLib::BRRESError If the specified bone is `nullptr`.
         */
        bool isDirectChild(Bone* bone) const;

        /*! @brief Returns whether this bone has a child with the specified 
         *  name.
         * 
         *  @param[in] name The bone name to check
         */
        bool hasChild(const std::string& name) const;

        /*! @brief Returns whethter this bone has a direct child with the
         *  specified name.
         *  
         *  @param[in] name The bone name to check
         */
        bool hasDirectChild(const std::string& name) const;

        /*! @brief Returns the **direct** child bone with the specified name.
         *  
         *  @param[in] name The child bone name
         * 
         *  @throw CTLib::BRRESError If this bone has no direct child with the
         *  specified name.
         */
        Bone* getChild(const std::string& name) const;

        /*! @brief Returns the first child bone, or `nullptr` if none. */
        Bone* getFirstChild() const;

        /*! @brief Returns the next sibling bone, or `nullptr` if none. */
        Bone* getNext() const;

        /*! @brief Returns the previous sibling bone, or `nullptr` if none. */
        Bone* getPrevious() const;

        /*! @brief Sets the position of this bone. */
        void setPosition(Vector3f position);

        /*! @brief Sets the rotation of this bone. */
        void setRotation(Vector3f rotation);

        /*! @brief Sets the scale of this bone. */
        void setScale(Vector3f scale);

        /*! @brief Sets whether this bone is visible. */
        void setVisible(bool visible);

        /*! @brief Returns the position of this bone. */
        Vector3f getPosition() const;

        /*! @brief Returns the rotation of this bone. */
        Vector3f getRotation() const;

        /*! @brief Returns the scale of this bone. */
        Vector3f getScale() const;

        /*! @brief Returns whether this bone is visible. */
        bool isVisible() const;

    private:

        // inserts 'bone' at the appropriate location in 'first' bone chain
        // returns the entry 'bone' was inserted after, or nullptr if first
        static Bone* insertPrivate(Bone* first, Bone* bone);

        // called when MDL0::remove<MDL0::Bone>() is called
        static void removePrivate(MDL0* mdl0, Bone* bone);

        // removes 'bone' from the 'first' bone chain
        // returns the entry before 'bone', or nullptr if none
        static Bone* removePrivate(Bone* first, Bone* bone);

        Bone(MDL0* mdl0, const std::string& name);

        // throws if the MDL0 owning this bone does not own the specified bone
        void assertSameMDL0(Bone* bone) const;

        // throws if the specified bone is this
        void assertNotThis(Bone* bone) const;

        // throws if the specified bone is a child of this bone
        void assertNotChild(Bone* bone) const;

        // throws if this bone has no direct child with the specified name
        void assertHasDirectChild(const std::string& name) const;

        // throws if 'bone' is `nullptr`
        void assertNotNull(Bone* bone) const;

        // pointer to parent bone; nullptr if none
        Bone* parent;

        // pointer to first child bone; nullptr if none
        Bone* child;

        // pointer to next sibling bone; nullptr if none
        Bone* next;

        // pointer to previous sibling bone; nullptr if none
        Bone* prev;

        // position
        Vector3f pos;

        // rotation (Euler, degrees)
        Vector3f rot;

        // scale
        Vector3f scale;

        // visibility flag
        bool visible;
    };

    /*! @brief Contains vertex data of a MDL0. (Section #2) */
    class VertexArray final : public ArraySection
    {

        template <class> friend class SectionContainer;

    public:

        /*! @brief The SectionType of this class. */
        constexpr static SectionType TYPE = SectionType::VertexArray;

        /*! @brief Enumeration of the valid vertex components type. */
        enum class Components
        {
            /*! @brief 2 values per vertex (1 for X, 1 for Y). */
            XY = 0x0,

            /*! @brief 3 values per vertex (1 for X, 1 for Y, 1 for Z). */
            XYZ = 0x1
        };

        /*! @brief Enumeration of the possible primitive type values. */
        enum class Format
        {
            /*! @brief Unsigned byte (8-bit integer). */
            UInt8 = 0x0,

            /*! @brief Signed byte (8-bit integer). */
            Int8 = 0x1,

            /*! @brief Unsigned short (16-bit integer). */
            UInt16 = 0x2,

            /*! @brief Signed short (16-bit integer). */
            Int16 = 0x3,

            /*! @brief Float (32-bit single-precision floating-point). */
            Float = 0x4
        };

        /*! @brief Returns the number of components for the specified type. */
        static uint8_t componentCount(Components comps);

        /*! @brief Returns the size in bytes of the specified format. */
        static uint8_t byteCount(Format format);

        ~VertexArray();

        /*! @brief Returns SectionType::VertexArray. */
        SectionType getType() const override;

        /*! @brief Sets the vertex data, components type, and format of this
         *  vertex section.
         *  
         *  **Note**: If the specified format is not Float, the divisor should
         *  be set _before_ invoking this method. **Failure to do so will result
         *  in the box minimum and maximum values to be incorrectly set!**
         *  
         *  @param[in] data The vertex data
         *  @param[in] comps The vertex components type
         *  @param[in] format The components primitive type
         */
        void setData(Buffer& data, Components comps = Components::XYZ,
            Format format = Format::Float);

        /*! @brief Sets the divisor of this vertex section.
         *  
         *  If the format of this vertex section is not Float, each component
         *  will be divided by `2 ^ divisor` in game.
         */
        void setDivisor(uint8_t divisor);

        /*! @brief Sets the box minimum of this vertex section.
         *  
         *  **Note**: Invoking setData() after this method will overwrite the
         *  value set here.
         */
        void setBoxMin(const Vector3f& boxMin);

        /*! @brief Sets the box maximum of this vertex section.
         *  
         *  **Note**: Invoking setData() after this method will overwrite the
         *  value set here.
         */
        void setBoxMax(const Vector3f& boxMax);

        /*! @brief Returns the components type of this vertex section. */
        Components getComponentsType() const;

        /*! @brief Returns the components' format of this vertex section. */
        Format getFormat() const;

        /*! @brief Returns the divisor of this vertex section. */
        uint8_t getDivisor() const;

        /*! @brief Returns the box minimum of this vertex section. */
        Vector3f getBoxMin() const;

        /*! @brief Returns the box maximum of this vertex section. */
        Vector3f getBoxMax() const;

    private:

        VertexArray(MDL0* mdl0, const std::string& name);

        // helper method for setData()
        float getComponent(Buffer& buffer);

        // the components type
        Components comps;

        // the components' primitive type
        Format format;

        // the component divisor
        uint8_t divisor;

        // vec3f of box min
        Vector3f boxMin;

        // vec3f of box max
        Vector3f boxMax;
    };

    /*! @brief Contains normal data of a MDL0. (Section #3) */
    class NormalArray final : public ArraySection
    {

        template <class> friend class SectionContainer;

    public:

        /*! @brief The SectionType of this class. */
        constexpr static SectionType TYPE = SectionType::NormalArray;

        /*! @brief Enumeration of the possible normal component types. */
        enum class Components
        {
            /*! @brief 3 values per normal (1 for X, 1 for Y, 1 for Z). */
            Normal = 0x0,

            /*! @brief 9 values per normal. */
            Normal_BiNormal_Tangent = 0x1,

            /*! @brief 3 values per normal. */
            Normal_OR_BiNormal_OR_Tangent = 0x2
        };

        /*! @brief Enumeration of the possible primitive type values. */
        enum class Format
        {
            /*! @brief Unsigned byte (8-bit integer). */
            UInt8 = 0x0,

            /*! @brief Signed byte (8-bit integer). */
            Int8 = 0x1,

            /*! @brief Unsigned short (16-bit integer). */
            UInt16 = 0x2,

            /*! @brief Signed short (16-bit integer). */
            Int16 = 0x3,

            /*! @brief Float (32-bit single-precision floating-point). */
            Float = 0x4
        };

        /*! @brief Returns the number of components for the specified type. */
        static uint8_t componentCount(Components comps);

        /*! @brief Returns the size in bytes of the specified format. */
        static uint8_t byteCount(Format format);

        ~NormalArray();

        /*! @brief Returns SectionType::NormalArray. */
        SectionType getType() const override;

        /*! @brief Sets the normal data, components type, and format of this
         *  normal section.
         *  
         *  @param[in] data The normal data
         *  @param[in] comps The normal components type
         *  @param[in] format The components primitive type
         */
        void setData(Buffer& data, Components comps = Components::Normal,
            Format format = Format::Float);

        /*! @brief Sets the divisor of this normal section.
         *  
         *  If the format of this normal section is not Float, each component
         *  will be divided by `2 ^ divisor` in game.
         */
        void setDivisor(uint8_t divisor);

        /*! @brief Returns the components type of this normal section. */
        Components getComponentsType() const;

        /*! @brief Returns the components' format of this normal section. */
        Format getFormat() const;

        /*! @brief Returns the divisor of this normal section. */
        uint8_t getDivisor() const;

    private:

        NormalArray(MDL0* mdl0, const std::string& name);

        // the components type
        Components comps;

        // the components' primitive type
        Format format;

        // the component divisor
        uint8_t divisor;
    };

    /*! @brief Contains colour data of a MDL0. (Section #4) */
    class ColourArray final : public ArraySection
    {

        template <class> friend class SectionContainer;

    public:

        /*! @brief The SectionType of this class. */
        constexpr static SectionType TYPE = SectionType::ColourArray;

        /*! @brief Enumeration of the possible colour formats. */
        enum class Format
        {
            /*! @brief 16 bits (5 for red, 6 for green, 5 for blue). */
            RGB565 = 0x0,

            /*! @brief 24 bits (8 for red, 8 for green, 8 for blue). */
            RGB8 = 0x1,

            /*! @brief 32 bits (8 for red, 8 for green, 8 for blue, 8 ignored). */
            RGBX8 = 0x2,

            /*! @brief 16 bits (4 for red, 4 for green, 4 for blue, 4 for alpha). */
            RGBA4 = 0x3,

            /*! @brief 24 bits (6 for red, 6 for green, 6 for blue, 6 for alpha). */
            RGBA6 = 0x4,

            /*! @brief 32 bits (8 for red, 8 for green, 8 for blue, 8 for alpha). */
            RGBA8 = 0x5
        };

        /*! @brief Returns the number of bytes per colour for the specified
         *  format.
         */
        static uint8_t byteCount(Format format);

        /*! @brief Returns the number of components for the specified format. */
        static uint8_t componentCount(Format format);

        ~ColourArray();

        /*! @brief Returns SectionType::ColourArray. */
        SectionType getType() const override;

        /*! @brief Sets the colour data and format of this colour section.
         *  
         *  @param[in] data The colour data, formatted as specified
         *  @param[in] format The colour format
         */
        void setData(Buffer& data, Format format = Format::RGBA8);

        /*! @brief Returns the colour format of this colour section. */
        Format getFormat() const;

    private:

        ColourArray(MDL0* mdl0, const std::string& name);

        // colour format
        Format format;
    };

    /*! @brief Contains texture coord data of a MDL0. (Section #5) */
    class TexCoordArray final : public ArraySection
    {

        template <class> friend class SectionContainer;

    public:

        /*! @brief The SectionType of this class. */
        constexpr static SectionType TYPE = SectionType::TexCoordArray;

        /*! @brief Enumeration of the possible texture coord components type. */
        enum class Components
        {
            /*! @brief 1 value per coord (1 for S). */
            S = 0x0,

            /*! @brief 2 values per coord (1 for S, 1 for T). */
            ST = 0x1
        };

        /*! @brief Enumeration of the possible primitive type values. */
        enum class Format
        {
            /*! @brief Unsigned byte (8-bit integer). */
            UInt8 = 0x0,

            /*! @brief Signed byte (8-bit integer). */
            Int8 = 0x1,

            /*! @brief Unsigned short (16-bit integer). */
            UInt16 = 0x2,

            /*! @brief Signed short (16-bit integer). */
            Int16 = 0x3,

            /*! @brief Float (32-bit single-precision floating-point). */
            Float = 0x4
        };

        /*! @brief Returns the number of components for the specified type. */
        static uint8_t componentCount(Components comps);

        /*! @brief Returns the size in bytes of the specified format. */
        static uint8_t byteCount(Format format);

        ~TexCoordArray();

        /*! @brief Returns SectionType::TexCoordArray. */
        SectionType getType() const override;

        /*! @brief Sets the texture coord data, components type, and format of
         *  this texture coord section.
         *  
         *  **Note**: If the specified format is not Float, the divisor should
         *  be set _before_ invoking this method. **Failure to do so will result
         *  in the box minimum and maximum values to be incorrectly set!**
         *  
         *  @param[in] data The texture coord data
         *  @param[in] comps The texture coord components type
         *  @param[in] format The components primitive type
         */
        void setData(Buffer& data, Components comps = Components::ST,
            Format format = Format::Float);

        /*! @brief Sets the divisor of this texture coord section.
         *  
         *  If the format of this texture coord section is not Float, each
         *  component will be divided by `2 ^ divisor` in game.
         */
        void setDivisor(uint8_t divisor);

        /*! @brief Sets the box minimum of this texture coord section.
         *  
         *  **Note**: Invoking setData() after this method will overwrite the
         *  value set here.
         */
        void setBoxMin(const Vector2f& boxMin);

        /*! @brief Sets the box maximum of this texture coord section.
         *  
         *  **Note**: Invoking setData() after this method will overwrite the
         *  value set here.
         */
        void setBoxMax(const Vector2f& boxMax);

        /*! @brief Returns the components type of this texture coord section. */
        Components getComponentsType() const;

        /*! @brief Returns the components' format of this texture coord section.
         */
        Format getFormat() const;

        /*! @brief Returns the divisor of this texture coord section. */
        uint8_t getDivisor() const;

        /*! @brief Returns the box minimum of this texture coord section. */
        Vector2f getBoxMin() const;

        /*! @brief Returns the box maximum of this texture coord section. */
        Vector2f getBoxMax() const;

    private:

        TexCoordArray(MDL0* mdl0, const std::string& name);

        // helper method for setData()
        float getComponent(Buffer& buffer);

        // components type
        Components comps;

        // the components' primitive type
        Format format;

        // the component divisor
        uint8_t divisor;

        // vec2f of box min
        Vector2f boxMin;

        // vec2f of box max
        Vector2f boxMax;
    };

    /*! @brief A material within a MDL0. (Section #8) */
    class Material final : public Section, public SectionCallback
    {

        template <class> friend class SectionContainer;

    public:

        /*! @brief The SectionType of this class. */
        constexpr static SectionType TYPE = SectionType::Material;

        /*! @brief A material layer. */
        class Layer final
        {

            friend class Material;

        public:

            /*! @brief Enumeration of the possible texture wrap modes. */
            enum class TextureWrap
            {
                /*! @brief Clamp on edges. */
                Clamp = 0x0,

                /*! @brief Repeat on edges. */
                Repeat = 0x1,

                /*! @brief Mirror on edges. */
                Mirror = 0x2
            };

            /*! @brief Enumeration of the possible min filter values. */
            enum class MinFilter
            {
                /*! @brief Use nearest pixel colour. */
                Nearest = 0x0,

                /*! @brief Linear interpolation of pixel colours. */
                Linear = 0x1,

                /*! @brief Use nearest mipmap and nearest pixel colour. */
                Nearest_Mipmap_Nearest = 0x2,

                /*! @brief Use nearest mipmap and linear interpolation of pixel
                 *  colours.
                 */
                Linear_Mipmap_Nearest = 0x3,

                /*! @brief Use linear interpolation of closest mipmaps and
                 *  nearest pixel colour.
                 */
                Nearest_Mipmap_Linear = 0x4,

                /*! @brief Use linear interpolation of closest mipmaps and
                 *  linear interpolation of pixel colours.
                 */
                Linear_Mipmap_Linear = 0x5
            };

            /*! @brief Enumeration of the possible mag filter values. */
            enum class MagFilter
            {
                /*! @brief Use nearest pixel colour. */
                Nearest = 0x0,

                /*! @brief Linear interpolation of nearest pixel colours. */
                Linear = 0x1
            };

            /*! @brief Enumeration of the possible anisotropy filtering values. */
            enum class AnisotropyFiltering
            {
                /*! @brief `1` (aka none). */
                One = 0x0,

                Two = 0x1,

                Four = 0x2
            };

            ~Layer();

            /*! @brief Returns the material owning this Layer. */
            Material* getMaterial() const;

            /*! @brief Returns the texture used by this Layer. */
            TextureLink* getTextureLink() const;

            /*! @brief Sets the TextureWrap mode of this Layer. */
            void setTextureWrapMode(TextureWrap mode);

            /*! @brief Sets the MinFilter of this layer. */
            void setMinFilter(MinFilter filter);

            /*! @brief Sets the MagFilter of this layer. */
            void setMagFilter(MagFilter filter);

            /*! @brief Sets the LOD bias of this layer. */
            void setLODBias(float bias);

            /*! @brief Sets the maximum AnisotropyFiltering of this layer. */
            void setMaxAnisotropyFiltering(AnisotropyFiltering val);

            /*! @brief Sets whether clamp bias is enabled for this layer. */
            void setClampBiasEnabled(bool enable);

            /*! @brief Sets whether this layer uses texel interpolate. */
            void setUsesTexelInterpolate(bool enable);

            /*! @brief Returns the TextureWrap mode of this layer. */
            TextureWrap getTextureWrapMode() const;

            /*! @brief Returns the MinFilter of this layer. */
            MinFilter getMinFilter() const;

            /*! @brief Returns the MagFilter of this layer. */
            MagFilter getMagFilter() const;

            /*! @brief Returns the LOD bias of this layer. */
            float getLODBias() const;

            /*! @brief Returns the maximum AnisotropyFiltering of this layer. */
            AnisotropyFiltering getMaxAnisotropyFiltering() const;

            /*! @brief Returns whether clamp bias is enabled for this layer. */
            bool isClampBiasEnabled() const;

            /*! @brief Returns whether this layer uses texel interpolate. */
            bool usesTexelInterpolate() const;

        private:

            Layer(Material* material, TextureLink* link);

            // reference to the Material owning this Layer
            Material* mat;

            // the texture used by this layer
            TextureLink* link;

            TextureWrap wrapMode;

            MinFilter minFilter;

            MagFilter magFilter;

            float lodBias;

            AnisotropyFiltering anisotropyFiltering;

            bool clampBias;

            bool texelInterpolate;
        };

        /*! @brief Enumeration of the possible cull modes. */
        enum class CullMode
        {
            /*! @brief No side of face culled. */
            None = 0x0,

            /*! @brief Outer (front) side of face culled. */
            Outside = 0x1,

            /*! @brief Inner (back) side of face culled. */
            Inside = 0x2,

            /*! @brief Both sides of face culled. */
            All = 0x3
        };

        /*! @brief Alpha blending settings. */
        struct AlphaMode final
        {

            /*! @brief Enumeration of the possible source values. */
            enum class Source : uint8_t
            {
                Zero = 0x0,
                One = 0x1,
                SourceColour = 0x2,
                InverseSourceColour = 0x3,
                SourceAlpha = 0x4,
                InverseSourceAlpha = 0x5,
                DestAlpha = 0x6,
                InverseDestAlpha = 0x7
            };

            /*! @brief Enumeration of the possible destination values. */
            enum class Dest : uint8_t
            {
                Zero = 0x0,
                One = 0x1,
                DestColour = 0x2,
                InverseDestColour = 0x3,
                SourceAlpha = 0x4,
                InverseSourceAlpha = 0x5,
                DestAlpha = 0x6,
                InverseDestAlpha = 0x7
            };

            /*! @brief Whether alpha blending is enabled. */
            bool enabled;

            /*! @brief Alpha blending source. */
            Source source;

            /*! @brief Alpha blending destination. */
            Dest dest;
        };

        /*! @brief Maximum amount of layers per material. */
        constexpr static uint32_t MAX_LAYER_COUNT = 8;

        ~Material();

        /*! @brief Returns SectionType::Material. */
        SectionType getType() const override;

        /*! @brief Creates, adds, and returns a newly created Layer with the
         *  specified texture.
         * 
         *  @param[in] link The link to the TEX0
         *  
         *  @throw CTLib::BRRESError If this Material already has
         *  `MAX_LAYER_COUNT` layers.
         */
        Layer* addLayer(TextureLink* link);

        /*! @brief Returns a std::vector containing all layers. */
        std::vector<Layer*> getLayers() const;

        /*! @brief Returns the Layer count of this Material. */
        uint32_t getLayerCount() const;

        /*! @brief Sets whether this material is translucent (XLU). */
        void setIsXLU(bool xlu);

        /*! @brief Sets the CullMode of this material. */
        void setCullMode(CullMode mode);

        /*! @brief Sets the AlphaMode of this material. */
        void setAlphaMode(AlphaMode mode);

        /*! @brief Sets the Shader used by this Material. */
        void setShader(Shader* shader);

        /*! @brief Returns whether this material is translucent (XLU). */
        bool isXLU() const;

        /*! @brief Returns the CullMode of this material. */
        CullMode getCullMode() const;

        /*! @brief Returns the AlphaMode of this material. */
        AlphaMode getAlphaMode() const;

        /*! @brief Returns the Shader used by this Material. */
        Shader* getShader() const;

    protected:

        //! entry callback
        void entryCallback(Section* instance, bool add) override;

    private:

        Material(MDL0* mdl0, const std::string& name);

        // throws if the specified section is not owned by the same MDL0 as this
        void assertSameMDL0(Section* instance) const;

        // throws if 'layers.size() == MAX_LAYER_COUNT'
        void assertHasRemainingLayers() const;

        // whether material is translucent (XLU)
        bool xlu;

        // cull mode of this material
        CullMode cullMode;

        // alpha mode of this material
        AlphaMode alphaMode;

        // vector containing all layers in this Material
        std::vector<Layer*> layers;

        // the Shader used by this material
        Shader* shader;
    };

    /*! @brief A shader within a MDL0. (Section #9) */
    class Shader final : public Section
    {

        template <class> friend class SectionContainer;

    public:

        /*! @brief The SectionType of this class. */
        constexpr static SectionType TYPE = SectionType::Shader;

        class Stage final
        {

            friend class Shader;

        public:

            /*! @brief Enumeration of the possible raster input values. */
            enum class RasterInput : uint8_t
            {
                Colour0 = 0x0,
                Colour1 = 0x1,
                AlphaBump = 0x5,
                NormalizedAlphaBump = 0x6,
                Zero = 0x7
            };

            /*! @brief Enumeration of the possible bias values. */
            enum class Bias : uint8_t
            {
                Zero = 0x0,
                Half = 0x1,
                MinusHalf = 0x2,
                Unknown = 0x3
            };

            /*! @brief Enumeration of the possible op values. */
            enum class Op : uint8_t
            {
                Add = 0x0,
                Sub = 0x1
            };

            /*! @brief Enumeration of the possible scale values. */
            enum class Scale : uint8_t
            {
                MultiplyBy1 = 0x0,
                MultiplyBy2 = 0x1,
                MultiplyBy4 = 0x2,
                DivideBy2 = 0x3
            };

            /*! @brief Enumeration of the possible destination values. */
            enum class Dest : uint8_t
            {
                PixelOutput = 0x0,
                Temp0 = 0x1,
                Temp1 = 0x2,
                Temp2 = 0x3
            };

            /*! @brief The colour operation of a Shader Stage. */
            struct ColourOp final
            {

                /*! @brief Enumeration of the possible stage argument types. */
                enum class Arg : uint8_t
                {
                    PixelOutput = 0x0,
                    PixelOutputAlpha = 0x1,
                    Temp0 = 0x2,
                    Temp0Alpha = 0x3,
                    Temp1 = 0x4,
                    Temp1Alpha = 0x5,
                    Temp2 = 0x6,
                    Temp2Alpha = 0x7,
                    Texture = 0x8,
                    TextureAlpha = 0x9,
                    Raster = 0xA,
                    RasterAlpha = 0xB,
                    One = 0xC,
                    Half = 0xD,
                    Constant = 0xE,
                    Zero = 0xF
                };

                /*! @brief Argument A in colour operation. */
                Arg argA;

                /*! @brief Argument B in colour operation. */
                Arg argB;

                /*! @brief Argument C in colour operation. */
                Arg argC;

                /*! @brief Argument D in colour operation. */
                Arg argD;

                /*! @brief Bias in colour operation. */
                Bias bias;

                /*! @brief Op in colour operation. */
                Op op;

                /*! @brief Whether clamp output between `0.0` and `1.0`. */
                bool clamp;

                /*! @brief Scale in colour operation. */
                Scale scale;

                /*! @brief Dest in colour operation. */
                Dest dest;
            };

            /*! @brief The alpha operation of a Shader Stage. */
            struct AlphaOp final
            {

                /*! @brief Enumeration of the possible arg values. */
                enum class Arg : uint8_t
                {
                    PixelOutput = 0x0,
                    Temp0 = 0x1,
                    Temp1 = 0x2,
                    Temp2 = 0x3,
                    Texture = 0x4,
                    Raster = 0x5,
                    Constant = 0x6,
                    Zero = 0x7
                };

                /*! @brief Argument A in alpha operation. */
                Arg argA;

                /*! @brief Argument B in alpha operation. */
                Arg argB;

                /*! @brief Argument C in alpha operation. */
                Arg argC;

                /*! @brief Argument D in alpha operation. */
                Arg argD;

                /*! @brief Bias in alpha operation. */
                Bias bias;

                /*! @brief Op in alpha operation. */
                Op op;

                /*! @brief Whether clamp output between `0.0` and `1.0`. */
                bool clamp;

                /*! @brief Scale in alpha operation. */
                Scale scale;

                /*! @brief Dest in alpha operation. */
                Dest dest;
            };

            ~Stage();

            /*! @brief Sets whether this Shader Stage uses a texture. */
            void setUsesTexture(bool enable);

            /*! @brief Sets the texture coord index of this Shader Stage.
             *  
             *  @throw CTLib::BRRESError If the specified index is more than or
             *  equal to Object::TEX_COORD_ARRAY_COUNT.
             */
            void setTexCoordIndex(uint8_t index);

            /*! @brief Sets the raster input of this Shader Stage. */
            void setRasterInput(RasterInput in);

            /*! @brief Sets the colour operation of this Shader Stage. */
            void setColourOp(ColourOp op);

            /*! @brief Sets the alpha operation of this Shader Stage. */
            void setAlphaOp(AlphaOp op);

            /*! @brief Returns whether this Shader Stage uses a texture. */
            bool usesTexture() const;

            /*! @brief Returns the texture coord index of this Shader Stage. */
            uint8_t getTexCoordIndex() const;

            /*! @brief Returns the raster input of this Shader Stage. */
            RasterInput getRasterInput() const;

            /*! @brief Returns the colour operation of this Shader Stage. */
            ColourOp getColourOp() const;

            /*! @brief Returns the alpha operation of this Shader Stage. */
            AlphaOp getAlphaOp() const;

        private:

            Stage(Shader* shader);

            // throws if 'index' >= 'Object::TEX_COORD_ARRAY_COUNT'
            void assertValidTexCoordIndex(uint8_t index) const;

            // pointer to the Shader owning this Stage
            Shader* shader;

            // whether this stage uses a texture
            bool useTex;

            // texture coord index
            uint8_t texCoordIdx;

            // raster input loc
            RasterInput rasIn;

            ColourOp colourOp;
            AlphaOp alphaOp;
        };

        /*! @brief Max number of stages per shader. */
        constexpr static uint8_t MAX_STAGE_COUNT = 8;

        /*! @brief Max number of texture references per shader. */
        constexpr static uint8_t MAX_TEX_REF = 8;

        /*! @brief Represents an unused material layer index. */
        constexpr static uint8_t UNUSED = 0xFF;

        ~Shader();

        /*! @brief Returns SectionType::Shader. */
        SectionType getType() const override;

        /*! @brief Creates and returns a new Shader Stage instance.
         *  
         *  @throw CTLib::BRRESError If the current Shader Stage count is
         *  `Shader::MAX_SHADER_COUNT - 1`.
         */
        Stage* addStage();

        /*! @brief Sets the material layer to use for the texture ref at the
         *  specified index.
         *  
         *  @param[in] index The texture ref index (0-7)
         *  @param[in] layer The material layer index
         *  
         *  @throw CTLib::BRRESError If the specified texture ref index is more
         *  than or equal to Shader::MAX_TEX_REF, or the specified material
         *  layer index is more than or equal to Material::MAX_LAYER_COUNT and
         *  not equal to Shader::UNUSED.
         */
        void setTexRef(uint8_t index, uint8_t layer);

        /*! @brief Returns the Shader Stage at the specified index.
         *  
         *  @throw CTLib::BRRESError If the specified index is more than or
         *  equal to the stage count.
         */
        Stage* getStage(uint8_t index) const;

        /*! @brief Returns a std::vector containing all stages. */
        std::vector<Stage*> getStages() const;

        /*! @brief Returns the number of stages in this shader. */
        uint8_t getStageCount() const;

        /*! @brief Returns the materal layer index for the texture ref at the
         *  specified index, or Shader::UNUSED if not set.
         * 
         *  @param[in] index The texture ref index (0-7)
         * 
         *  @throw CTLib::BRRESError If the specified texture ref index is more
         *  than or equal to Shader::MAX_TEX_REF.
         */
        uint8_t getTexRef(uint8_t index) const;

    private:

        Shader(MDL0* mdl0, const std::string& name);

        // throws if 'getStageCount()' == 'MAX_STAGE_COUNT' - 1
        void assertCanAddStage() const;

        // throws if 'index' >= 'getStageCount()'
        void assertValidStageIndex(uint8_t index) const;

        // throws if 'index' >= 'MAX_TEX_REF'
        void assertValidTexRefIndex(uint8_t index) const;

        // throws if 'layer' >= 'Material::MAX_LAYER_COUNT' and != 'UNUSED'
        void assertValidLayerRef(uint8_t layer) const;

        // material layer references
        uint8_t texRefs[MAX_TEX_REF];

        // vector containing all stages in this shader
        std::vector<Stage*> stages;
    };

    /*! @brief An object within a MDL0. (Section #10) */
    class Object final : public Section, public SectionCallback
    {

        template <class> friend class SectionContainer;

    public:

        /*! @brief The SectionType of this class. */
        constexpr static SectionType TYPE = SectionType::Object;

        /*! @brief The maximum number of colour arrays that an Object section
         *  can reference.
         */
        constexpr static uint32_t COLOUR_ARRAY_COUNT = 2;

        /*! @brief The maximum number of texture coord arrays that an Object
         *  section can reference. 
         */
        constexpr static uint32_t TEX_COORD_ARRAY_COUNT = 8;

        /*! @brief Enumeration of the possible primitive types. */
        enum class PrimitiveType : uint8_t
        {
            /*! @brief Draw quads (4 vertices per face). */
            QUADS = 0x80,

            /*! @brief Draw triangles (3 vertices per face). */
            TRIANGLES = 0x90,

            /*! @brief Draw triangle strip (First 3 vertices makes a triangle,
             *  remaining triangles are made up of vertices at index _N - 2_,
             *  _N - 1_, and _N_). 
             */
            TRIANGLE_STRIP = 0x98,

            /*! @brief Draw triangle fan (First 3 vertices makes a triangle,
             *  remaining triangles are made up of vertices at index _N - 1_,
             *  _N_, and _0_).
             */
            TRIANGLE_FAN = 0xA0,

            /*! @brief Draw lines (2 vertices per line). */
            LINES = 0xA8,

            /*! @brief Draw line strip (First 2 vertices makes a line,
             *  remaining lines are made up of vertices at index _N - 1_ and
             *  _N_). */
            LINE_STRIP = 0xB0,

            /*! @brief Draw points (1 vertex per point). */
            POINTS = 0xB8
        };

        /*! @brief Returns the recommended size of indices for the specified
         *  ArraySection.
         * 
         *  `1`: 8 bits \n
         *  `2`: 16 bits
         */
        static uint8_t indexSizeFor(ArraySection* instance);

        ~Object();

        /*! @brief Returns SectionType::Object. */
        SectionType getType() const override;

        /*! @brief Sets the bone reference of this object.
         *  
         *  **Note**: Only single bone references are supported at the moment.
         * 
         *  @param[in] bone The Bone reference
         */
        void setBone(Bone* bone);

        /*! @brief Sets this Object's VertexArray with the specified one. */
        void setVertexArray(VertexArray* array);

        /*! @brief Sets the size of the VertexArray indices.
         *  
         *  `0`: auto \n
         *  `1`: 8 bits \n
         *  `2`: 16 bits
         *  
         *  @param[in] size The indices size
         * 
         *  @throw CTLib::BRRESError If the specified size is more than 2.
         */
        void setVertexArrayIndexSize(uint8_t size);

        /*! @brief Sets this Object's NormalArray with the specified one. */
        void setNormalArray(NormalArray* array);

        /*! @brief Sets the size of the NormalArray indices.
         *  
         *  `0`: auto \n
         *  `1`: 8 bits \n
         *  `2`: 16 bits
         *  
         *  @param[in] size The indices size
         * 
         *  @throw CTLib::BRRESError If the specified size is more than 2.
         */
        void setNormalArrayIndexSize(uint8_t size);

        /*! @brief Sets this Object's ColourArray at the specified index.
         *  
         *  @param[in] array The ColourArray
         *  @param[in] index The ColourArray index
         * 
         *  @throw CTLib::BRRESError If the specified index is more than or
         *  equal to COLOUR_ARRAY_COUNT.
         */
        void setColourArray(ColourArray* array, uint32_t index);

        /*! @brief Sets the size of the ColourArray indices.
         *  
         *  `0`: auto \n
         *  `1`: 8 bits \n
         *  `2`: 16 bits
         *  
         *  @param[in] index The ColourArray index
         *  @param[in] size The indices size
         * 
         *  @throw CTLib::BRRESError If the specified index is more than or
         *  equal to COLOUR_ARRAY_COUNT, or the specified size is more than
         *  2.
         */
        void setColourArrayIndexSize(uint32_t index, uint8_t size);

        /*! @brief Sets this Object's TexCoordArray at the specified index.
         *  
         *  @param[in] array The TexCoordArray
         *  @param[in] index The TexCoordArray index
         * 
         *  @throw CTLib::BRRESError If the specified index is more than or
         *  equal to TEX_COORD_ARRAY_COUNT.
         */
        void setTexCoordArray(TexCoordArray* array, uint32_t index);

        /*! @brief Sets the size of the TexCoordArray indices.
         *  
         *  `0`: auto \n
         *  `1`: 8 bits \n
         *  `2`: 16 bits
         *  
         *  @param[in] index The TexCoordArray index
         *  @param[in] size The indices size
         * 
         *  @throw CTLib::BRRESError If the specified index is more than or
         *  equal to TEX_COORD_ARRAY_COUNT, or the specified size is more than
         *  2.
         */
        void setTexCoordArrayIndexSize(uint32_t index, uint8_t size);

        /*! @brief Sets the geometry data of this Object.
         *  
         *  **Important Note**: All array sections used by the geometry data
         *  must be set _before_ calling this method.
         *  
         *  @param[in] data The geometry data
         *  
         *  @throw CTLib::BRRESError If the VertexArray of this Object is not
         *  set, or the specified geometry data is invalid.
         */
        void setGeometryData(const Buffer& data);

        /*! @brief Returns the bone reference of this Object. */
        Bone* getBone() const;

        /*! @brief Returns this Object's VertexArray. */
        VertexArray* getVertexArray() const;

        /*! @brief Returns the size of the VertexArray indices.
         *  
         *  `1`: 8 bits \n
         *  `2`: 16 bits
         * 
         *  @throw CTLib::BRRESError If the VertexArray of this Object is not
         *  set.
         */
        uint8_t getVertexArrayIndexSize() const;

        /*! @brief Returns this Object's NormalArray. */
        NormalArray* getNormalArray() const;

        /*! @brief Returns the size of the NormalArray indices.
         *  
         *  `1`: 8 bits \n
         *  `2`: 16 bits
         * 
         *  @throw CTLib::BRRESError If the NormalArray of this Object is not
         *  set.
         */
        uint8_t getNormalArrayIndexSize() const;

        /*! @brief Returns this Object's ColourArray at the specified index.
         *  
         *  @param[in] index The ColourArray index
         * 
         *  @throw CTLib::BRRESError If the specified index is more than or
         *  equal to COLOUR_ARRAY_COUNT.
         * 
         *  @return The ColourArray at the specified index
         */
        ColourArray* getColourArray(uint32_t index) const;

        /*! @brief Returns the size of the ColourArray indices.
         *  
         *  `1`: 8 bits \n
         *  `2`: 16 bits
         *  
         *  @param[in] index The ColourArray index
         * 
         *  @throw CTLib::BRRESError If the specified index is more than or
         *  equal to COLOUR_ARRAY_COUNT, or the ColourArray at the specified
         *  index is not set.
         */
        uint8_t getColourArrayIndexSize(uint32_t index) const;

        /*! @brief Returns this Object's TexCoordArray at the specified index.
         *  
         *  @param[in] index The TexCoordArray index
         * 
         *  @throw CTLib::BRRESError If the specified index is more than or
         *  equal to TEX_COORD_ARRAY_COUNT.
         * 
         *  @return The TexCoordArray at the specified index
         */
        TexCoordArray* getTexCoordArray(uint32_t index) const;

        /*! @brief Returns the size of the TexCoordArray indices.
         *  
         *  `1`: 8 bits \n
         *  `2`: 16 bits
         *  
         *  @param[in] index The TexCoordArray index
         * 
         *  @throw CTLib::BRRESError If the specified index is more than or
         *  equal to TEX_COORD_ARRAY_COUNT, or the TexCoordArray at the
         *  specified index is not set.
         */
        uint8_t getTexCoordArrayIndexSize(uint32_t index) const;

        /*! @brief Returns the geometry data of this Object.
         *  
         *  **See**: @link MDL0::Object::setGeometryData(const Buffer&)
         *  setGeometryData()'s documentation@endlink for more information on
         *  the format of the returned data.
         */
        Buffer getGeometryData() const;

        /*! @brief Returns the geometry data size of this Object. */
        uint32_t getGeometryDataSize() const;

        /*! @brief Returns the vertex count of the geometry in this Object. */
        uint32_t getVertexCount() const;

        /*! @brief Returns the face count of the geometry in this Object. */
        uint32_t getFaceCount() const;

        /*! @brief Returns the size of each vertex of the geometry data. */
        uint32_t getVertexSize() const;

    private:

        Object(MDL0* mdl0, const std::string& name);

        // called when a section entry is added/removed
        void entryCallback(Section* instance, bool add) override;

        // throws if 'instance' is `null`
        void assertNotNull(Section* instance) const;

        // throws if 'index' is >= COLOUR_ARRAY_COUNT
        void assertValidColourArrayIndex(uint32_t index) const;

        // throws if 'index' is >= TEX_COORD_ARRAY_COUNT
        void assertValidTexCoordArrayIndex(uint32_t index) const;

        // throws if 'vertexArray' is `nullptr`
        void assertVertexArraySet() const;

        // throws if 'normalArray' is `nullptr`
        void assertNormalArraySet() const;

        // throws if 'colourArrays[index]' is `nullptr`
        void assertColourArraySet(uint32_t index) const;

        // throws if 'texCoordArrays[index] is `nullptr`
        void assertTexCoordArraySet(uint32_t index) const;

        // throws if 'size' is > 2
        void assertValidArrayIndexSize(uint8_t size) const;

        // single bone reference
        Bone* bone;

        // vertex array used by this object
        VertexArray* vertexArray;
        uint8_t vertexIndexSize;

        // normal array used by this object
        NormalArray* normalArray;
        uint8_t normalIndexSize;

        // colour arrays used by this object
        ColourArray* colourArrays[COLOUR_ARRAY_COUNT];
        uint8_t colourIndexSizes[COLOUR_ARRAY_COUNT];

        // texture coord arrays used by this object
        TexCoordArray* texCoordArrays[TEX_COORD_ARRAY_COUNT];
        uint8_t texCoordIndexSizes[TEX_COORD_ARRAY_COUNT];

        // compiled geometry data
        Buffer data;
        uint32_t vertexCount;
        uint32_t faceCount;
    };

    /*! @brief Links a TEX0 to a MDL0. (Section #11) */
    class TextureLink final : public Section, public BRRESSubFileCallback, public SectionCallback
    {

        friend class MDL0;

        friend class Material;

        template <class> friend class SectionContainer;

    public:

        /*! @brief The SectionType of this class. */
        constexpr static SectionType TYPE = SectionType::TextureLink;

        ~TextureLink();

        /*! @brief Returns the TEX0 linked by this TextureLink instance. */
        TEX0* getTEX0() const;

        /*! @brief Returns the number of reference to this TextureLink. */
        uint32_t getCount() const;

        /*! @brief Returns a std::vector containing all Material layers
         *  referencing this TextureLink instance.
         */
        std::vector<Material::Layer*> getReferences() const;

    protected:

        //! does nothing
        void subfileAdded(BRRESSubFile* subfile) override;

        //! removes this section if 'subfile == tex0'
        void subfileRemoved(BRRESSubFile* subfile) override;

        //! removes deleted reference
        void entryCallback(Section* instance, bool add) override;

    private:

        TextureLink(MDL0* mdl0, const std::string& name);

        // used by MDL0::linkTEX0()
        TextureLink(MDL0* mdl0, TEX0* tex0);

        // called by Material::addLayer()
        void addReference(Material::Layer* layer);

        // the TEX0 linked
        TEX0* tex0;

        // vector containing all Material Layer referencing to this
        std::vector<Material::Layer*> references;
    };

    ~MDL0();

    /*! @brief Adds and returns a section of the specified template type with
     *  the specified name.
     * 
     *  @tparam Type The section type
     * 
     *  @param[in] name The section name
     * 
     *  @throw CTLib::BRRESError If another section of the same type has the
     *  specified name.
     * 
     *  @return The newly created section
     */
    template <class Type>
    Type* add(const std::string& name);

    /*! @brief Returns the section of the specified template type with the
     *  specified name.
     * 
     *  @tparam Type The section type
     * 
     *  @param[in] name The section name
     * 
     *  @throw CTLib::BRRESError If no section of the specified type has the
     *  specified name.
     * 
     *  @return The section with the specified name
     */
    template <class Type>
    Type* get(const std::string& name) const;

    /*! @brief Returns whether a section of the specified template type with
     *  the specified name exists.
     * 
     *  @tparam Type The section type
     * 
     *  @param[in] name The section name
     * 
     *  @return Whether a section with the specified name exists
     */
    template <class Type>
    bool has(const std::string& name) const;

    /*! @brief Removes and _deletes_ the section of the specified template type
     *  with the specified name.
     *  
     *  If you have any reference to this section pointer left, _**DO NOT USE
     *  IT**_, as it will have been `delete`d when this function returns.
     * 
     *  @tparam Type The section type
     * 
     *  @param[in] name The section name
     * 
     *  @throw CTLib::BRRESError If no section of the specified type has the
     *  specified name.
     */
    template <class Type>
    void remove(const std::string& name);

    /*! @brief Returns a std::vector containing all section entries of the
     *  specified template type.
     * 
     *  @tparam Type The section type
     */
    template <class Type>
    std::vector<Type*> getAll() const;

    /*! @brief Returns the number of section entries of the specified template
     *  type.
     *  
     *  @tparam Type The section type
     */
    template <class Type>
    uint16_t count() const;

    /// Model info methods /////////////

    /*! @brief Sets the box minimum of this MDL0. */
    void setBoxMin(const Vector3f& boxMin);

    /*! @brief Sets the box maximum of this MDL0. */
    void setBoxMax(const Vector3f& boxMax);

    /*! @brief Returns the box minimum of this MDL0.
     *  
     *  **Note**: This value is not calculated automatically.
     */
    Vector3f getBoxMin() const;

    /*! @brief Returns the box maximum of this MDL0.
     *  
     *  **Note**: This value is not calculated automatically.
     */
    Vector3f getBoxMax() const;

    /// Section-specific methods ///////

    /*! @brief Returns the DrawOpa Links section instance. */
    Links* getDrawOpaSection() const;

    /*! @brief Returns the first (root) bone in the bone hierarchy, or nullptr
     *  if this MDL0 has no bone.
     */
    Bone* getRootBone() const;

    /*! @brief Links the specified TEX0 to this MDL0.
     *  
     *  **Warning**: If the specified TEX0 instance is removed from its BRRES,
     *  _THE RETURNED TextureLink INSTANCE WILL GET REMOVED AND **DELETED**!!!_
     * 
     *  @param[in] tex0 The TEX0 to link
     * 
     *  @throw CTLib::BRRESError If the specified TEX0 is not owned by the same
     *  BRRES as this MDL0, or the specified TEX0 is `nullptr`.
     * 
     *  @return A TextureLink instance
     */
    TextureLink* linkTEX0(TEX0* tex0);

private:

    // only works with named sections
    template <class Type>
    class SectionContainer final
    {

        friend class MDL0;

    public:

        // Constructs an empty section container instance.
        SectionContainer(MDL0* mdl0);

        SectionContainer(const SectionContainer&) = delete;
        SectionContainer(SectionContainer&&) = delete;

        ~SectionContainer();

        // creates and add an instance with the specified name
        Type* add(const std::string& name);

        // returns the instance with the specified name
        Type* get(const std::string& name) const;

        // returns whether an instance with the specified name exists
        bool has(const std::string& name) const;

        // removes the instance with the specified name
        void remove(const std::string& name);

        // removes and _delete_ all instances in this container
        void deleteAll();

    private:

        // used by MDL0 to add Links sections
        void directAdd(Type* instance);

        // throws if this container contains an entry with the specified name
        void assertUniqueName(const std::string& name) const;

        // throws if this container has no entry with the specified name
        void assertExists(const std::string& name) const;

        // pointer to the MDL0 owning this container
        MDL0* mdl0;

        // vector containing all section instances
        std::vector<Type*> sections;

        // map of <name, index> containing the index for name in 'container'
        std::map<std::string, uint32_t> nameLinks;
    };

    MDL0(BRRES* brres, const std::string& name);

    // registers the specified entry callback
    void addCallback(SectionCallback* cb);

    // unregisters the specified entry callback
    void removeCallback(SectionCallback* cb);

    // called when a section instance is added
    void sectionAdded(Section* instance);

    // called when a section instance is removed
    void sectionRemoved(Section* instance);

    // throws if subfile is not owned by the same BRRES as this
    void assertSameBRRES(BRRESSubFile* subfile) const;

    SectionContainer<Links> linksSections;
    SectionContainer<Bone> boneSections;
    SectionContainer<VertexArray> verticesSections;
    SectionContainer<NormalArray> normalsSections;
    SectionContainer<ColourArray> coloursSections;
    SectionContainer<TexCoordArray> texCoordsSections;
    SectionContainer<Material> materialSections;
    SectionContainer<Shader> shaderSections;
    SectionContainer<Object> objectSections;
    SectionContainer<TextureLink> textureLinkSections;

    std::vector<SectionCallback*> entryCallbacks;

    // box minimum and maximum
    Vector3f boxMin, boxMax;

    // pointer to first (root) bone
    Bone* rootBone;
};

/*! @brief A texture within a BRRES. */
class TEX0 final : public BRRESSubFile
{

    friend class BRRES;

public:

    ~TEX0();

    /*! @brief Encodes the specified image in the specified format and sets it
     *  as base texture data.
     * 
     *  This will also delete all mipmaps.
     *  
     *  @param[in] image The texture data
     *  @param[in] format The encoding format
     * 
     *  @throw CTLib::ImageError If format is not supported.
     */
    void setTextureData(const Image& image, ImageFormat format);

    /*! @brief Encodes the specified image in the current format of this TEX0
     *  and sets it as the base texture data.
     * 
     *  This will also delete all mipmaps.
     * 
     *  @param[in] image The texture data
     * 
     *  @throw CTLib::ImageError If this TEX0's current format is not
     *  supported.
     */
    void setTextureData(const Image& image);

    /*! @brief Sets the texture data of this TEX0 to the contents of the
     *  specified buffer, and sets the width, height, and format of this
     *  TEX0.
     *  
     *  This will also delete all mipmaps.
     *  
     *  @param[in] data The encoded texture data
     *  @param[in] width The width of the encoded texture data
     *  @param[in] height The height of the encoded texture data
     *  @param[in] format The format of the encoded texture data
     *  
     *  @throw CTLib::BRRESError If there are not enough bytes remaining in the
     *  texture data buffer.
     *  
     *  @throw CTLib::ImageError If the specified format is not supported.
     */
    void setTextureData(Buffer& data, uint16_t width, uint16_t height, ImageFormat format);

    /*! @brief Sets the texture data of this TEX0 to the contents of the
     *  specified buffer, and sets the width and height of this TEX0.
     *  
     *  This will also delete all mipmaps.
     *  
     *  @param[in] data The encoded texture data
     *  @param[in] width The width of the encoded texture data
     *  @param[in] height The height of the encoded texture data
     *  
     *  @throw CTLib::BRRESError If there are not enough bytes remaining in the
     *  texture data buffer.
     *  
     *  @throw CTLib::ImageError If this TEX0's current format is not
     *  supported.
     */
    void setTextureData(Buffer& data, uint16_t width, uint16_t height);

    /*! @brief Returns the width of the base texture data of this TEX0. */
    uint16_t getWidth() const;

    /*! @brief Returns the height of the base texture data of this TEX0. */
    uint16_t getHeight() const;

    /*! @brief Returns the format of all texture data in this TEX0. */
    ImageFormat getFormat() const;

    /*! @brief Returns the base texture data of this TEX0. */
    Buffer getTextureData() const;

    /*! @brief Sets the texture data of the mipmap at the specified index.
     *  
     *  The specified image will be encoded in the current format of this TEX0.
     *  
     *  @param[in] index The 0-based mipmap index
     *  @param[in] image The mipmap texture data
     * 
     *  @throw CTLib::BRRESError If the image's width does not match the return
     *  value of `getMipmapWidth(index)`, or the image's height does not match
     *  `getMipmapHeight(index)`, or the mipmap count is less than the index.
     * 
     *  @throw CTLib::ImageError If this TEX0's current format is not
     *  supported.
     */
    void setMipmapTextureData(uint32_t index, const Image& image);

    /*! @brief Sets the texture data of the mipmap at the specified index.
     *  
     *  @param[in] index The 0-based mipmap index
     *  @param[in] image The encoded mipmap texture data
     *  
     *  @throw CTLib::BRRESError If there are not enough bytes remaining in the
     *  texture data buffer, or the specified index is greater than the mipmap
     *  count of this TEX0.
     *  
     *  @throw CTLib::ImageError If this TEX0's current format is not
     *  supported.
     */
    void setMipmapTextureData(uint32_t index, Buffer& data);

    /*! @brief Generates the specified count of mipmaps based on the specified
     *  image.
     * 
     *  Any previously existing mipmaps will be deleted.
     * 
     *  If you still have access to the image used for the base texture data,
     *  it is recommended to use this function instead of
     *  `generateMipmaps(uint32_t)` as it will avoid an additional decode.
     * 
     *  @param[in] count The amount of mipmaps to generate
     *  @param[in] image The image to use when generating mipmaps
     * 
     *  @throw CTLib::BRRESError If the width or height of at least one of the
     *  generated mipmaps is zero, i.e., too many mipmaps.
     */
    void generateMipmaps(uint32_t count, const Image& image);

    /*! @brief Generates the specified count of mipmaps based on the base
     *  texture data.
     * 
     *  Any previously existing mipmaps will be deleted.
     * 
     *  If you still have access to the image used for the base texture data,
     *  it is recommended to use `generateMipmaps(uint32_t, const Image&)`
     *  instead of this one as it will avoid an additional decode.
     * 
     *  @param[in] count The amount of mipmaps to generate
     * 
     *  @throw CTLib::BRRESError If the width or height of at least one of the
     *  generated mipmaps is zero, i.e., too many mipmaps.
     */
    void generateMipmaps(uint32_t count);

    /*! @brief Removes all mipmaps from this TEX0. */
    void deleteMipmaps();

    /*! @brief Returns the number of mipmaps in this TEX0. */
    uint32_t getMipmapCount() const;

    /*! @brief Returns the width the mipmap at the specified index must have.
     *  
     *  @param[in] index The 0-based mipmap index
     * 
     *  @return The width of the mipmap at the specified index
     */
    uint16_t getMipmapWidth(uint32_t index) const;

    /*! @brief Returns the height the mipmap at the specified index must have.
     *  
     *  @param[in] index The 0-based mipmap index
     * 
     *  @return The height of the mipmap at the specified index
     */
    uint16_t getMipmapHeight(uint32_t index) const;

    /*! @brief Returns the texture data of the mipmap at the specified index.
     *  
     *  @param[in] index The 0-based mipmap index
     * 
     *  @throw CTLib::BRRESError If index is more than or equal to the
     *  mipmap count.
     * 
     *  @return The texture data of the specified mipmap
     */
    Buffer getMipmapTextureData(uint32_t index) const;

private:

    TEX0(BRRES* brres, const std::string& name);

    // throws if data.remaining() < ImageCoder::sizeFor(width, height, format)
    void assertValidTextureData(
        const Buffer& data, uint16_t width, uint16_t height, ImageFormat format) const;

    // throws if index >= mipmaps.size()
    void assertValidMipmap(uint32_t index) const;

    // throws if count is 0 or any mipmap between 0 and count would have a dimension of 0
    void assertValidMipmapCount(uint32_t count) const;

    // throws if mipmaps.size() < index
    void assertValidMipmapInsert(uint32_t index) const;

    // throws if the dimensions of image are invalid for index
    void assertValidMipmapImage(uint32_t index, const Image& image) const;

    // throws if data.remaining() < buffer size for the specified mipmap index
    void assertValidMipmapTextureData(uint32_t index, const Buffer& data) const;

    // width of the texture data
    uint16_t width;

    // height of the texture data
    uint16_t height;

    // data format
    ImageFormat format;

    // encoded texture data
    Buffer data;

    // vector containing mipmap texture data
    std::vector<Buffer> mipmaps;
};

class BRRES final
{

public:

    /*! @brief Reads a BRRES from the specified buffer.
     *  
     *  @throw CTLib::BRRESError If the specified data cannot be parsed as a
     *  valid BRRES.
     */
    static BRRES read(Buffer& data);

    /*! @brief Writes the specified BRRES to a new buffer. */
    static Buffer write(const BRRES& brres);

    /*! @brief Constructs an empty BRRES. */
    BRRES();

    /*! @brief Delete copy constructor for move-only class. */
    BRRES(const BRRES&) = delete;

    /*! @brief Moves the contents of the specified BRRES in this newly created
     *  one.
     * 
     *  The source BRRES will then be empty, but can still be safely used.
     * 
     *  @param[in] src The BRRES to be moved
     */
    BRRES(BRRES&& src);

    ~BRRES();

    /*! @brief Returns the total subfile count in this BRRES. */
    uint16_t getSubfileCount() const;

    /*! @brief Creates, add, and return a newly created instance of the
     *  specified template type with the specified name.
     * 
     *  @tparam Type The entry type 
     * 
     *  @param[in] name The name of the entry to be created
     * 
     *  @throw CTLib::BRRESError If another entry of the specified template
     *  type in this BRRES has the specified name.
     * 
     *  @return The newly created instance
     */
    template <class Type>
    Type* add(const std::string& name);

    /*! @brief Returns the instance of the specified template type with the
     *  specified name.
     * 
     *  @tparam Type The entry type
     *  
     *  @param[in] name The name of the entry
     * 
     *  @throw CTLib::BRRESError If this BRRES has no entry of the specified
     *  template type with the specified name.
     * 
     *  @return The instance with the specified name
     */
    template <class Type>
    Type* get(const std::string& name) const;

    /*! @brief Returns whether this BRRES has an instance of the specified
     *  template type with the specified name.
     * 
     *  @tparam Type The entry type
     * 
     *  @param[in] name The name of the entry
     * 
     *  @return Whether the specified entry is present
     */
    template <class Type>
    bool has(const std::string& name) const;

    /*! @brief Removes and _deletes_ the instance of the specified template
     *  type with the specified name.
     *  
     *  If you have any reference to this entry left, _**DO NOT USE IT**_, as it
     *  will have been `delete`d when this function returns.
     * 
     *  @tparam Type The entry type
     * 
     *  @param[in] name The name of the entry
     * 
     *  @throw CTLib::BRRESError If this BRRES has no entry of the specified
     *  template type with the specified name.
     */
    template <class Type>
    void remove(const std::string& name);

    /*! @brief Returns a std::vector containing all instances of the specified
     *  template type in this BRRES.
     * 
     *  @tparam Type The entry type
     * 
     *  @return A std::vector containing all entries of the specified type
     */
    template <class Type>
    std::vector<Type*> getAll() const;

    /*! @brief Returns the number of instances of the specified template type
     *  in this BRRES.
     *  
     *  @tparam Type Type entry type
     * 
     *  @return The entry count of the specified type
     */
    template <class Type>
    uint16_t count() const;

    /*! @brief Registers the specified subfile callback. */
    void registerCallback(BRRESSubFileCallback* cb);

    /*! @brief Unregisters the specified subfile callback. */
    void unregisterCallback(BRRESSubFileCallback* cb);

private:

    // map of <name, MDL0> containing all MDL0s in this BRRES
    std::map<std::string, MDL0*> mdl0s;

    // map of <name, TEX0> containing all TEX0s in this BRRES
    std::map<std::string, TEX0*> tex0s;

    std::vector<BRRESSubFileCallback*> callbacks;
};

/*! @brief BRRESError is the error class used by the methods in this header. */
class BRRESError final : public std::runtime_error
{

public:
    
    /*! @brief Constructs a CTLib::BRRESError with the specified message. */
    BRRESError(const char* msg);

    /*! @brief Constructs a CTLib::BRRESError with the specified message. */
    BRRESError(const std::string& msg);
};

/*! @} addtogroup brres */
}
