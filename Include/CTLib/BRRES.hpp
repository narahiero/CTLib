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

/*! @brief Superclass of all BRRES sub files. */
class BRRESSubFile
{

    friend class BRRES;
    
public:

    virtual ~BRRESSubFile();

    /*! @brief Returns the name of this sub file. */
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

/*! @brief A model within a BRRES. */
class MDL0 final : public BRRESSubFile
{

    friend class BRRES;

    friend class Bone;

private:

    template <class Type> class SectionContainer;

public:

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

        /*! @brief ColorArray section (#4) */
        ColorArray = 0x4,

        /*! @brief TexCoordArray section (#5) */
        TexCoordArray = 0x5,

        /*! @brief An instance of one of the section superclasses. */
        NONE = ~0x0
    };

    /*! @brief The superclass of all sections in a MDL0. */
    class Section
    {

    public:

        virtual ~Section();

        /*! @brief Returns the SectionType of this section. */
        virtual SectionType getType() const;

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

    ////////////////////////////////////////////////////////
    ///  Actual section classes
    ////////////////////////////////////////////////////////

    /*! @brief MDL0 sections links. (Section #0) */
    class Links final : public Section
    {

        friend class MDL0;

        template <class> friend class SectionContainer;

    public:

        /*! @brief Enumeration of the possible Links type. */
        enum class Type
        {
            /*! @brief Specifies bone hierarchy. */
            NodeTree = 0x02
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

    private:

        Links(MDL0* mdl0, const std::string& name);

        // private ctor used internally
        Links(MDL0* mdl0, Type type);

        const Type linksType;
    };

    /*! @brief A single bone of a MDL0. (Section #1) */
    class Bone final : public Section
    {

        friend class MDL0;

        template <class> friend class SectionContainer;

    public:

        /*! @brief The SectionType of this class. */
        constexpr static SectionType TYPE = SectionType::Bone;

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

        /*! @brief Returns whether the specified bone is a child of this. */
        bool isChild(Bone* bone) const;

        /*! @brief Returns whether the specified bone is a direct child of this.
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

        /*! @brief Returns the position of this bone. */
        Vector3f getPosition() const;

        /*! @brief Returns the rotation of this bone. */
        Vector3f getRotation() const;

        /*! @brief Returns the scale of this bone. */
        Vector3f getScale() const;

        /*! @brief Returns the box minimum of this bone. */
        Vector3f getBoxMin() const;

        /*! @brief Returns the box maximum of this bone. */
        Vector3f getBoxMax() const;

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

        // box minimum
        Vector3f boxMin;

        // box maximum
        Vector3f boxMax;
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

        /*! @brief Returns the number of components for the specified type. */
        static uint8_t componentCount(Components comps);

        ~VertexArray();

        /*! @brief Returns SectionType::VertexArray. */
        SectionType getType() const override;

        /*! @brief Sets the vertex data and components type of this vertex
         *  section.
         *  
         *  @param[in] data The vertex data, containing floats
         *  @param[in] comps The vertex components type
         */
        void setData(Buffer& data, Components comps = Components::XYZ);

        /*! @brief Returns the components type of this vertex section. */
        Components getComponentsType() const;

        /*! @brief Returns the box minimum of this vertex section. */
        Vector3f getBoxMin() const;

        /*! @brief Returns the box maximum of this vertex section. */
        Vector3f getBoxMax() const;

    private:

        VertexArray(MDL0* mdl0, const std::string& name);

        // the components type
        Components comps;

        // vec3f of box min
        Vector3f boxMin;

        // vec3f of box max
        Vector3f boxMax;
    };

    /*! @brief Contains normal data of a MDL0. (Section #3) */
    class NormalArray : public ArraySection
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

        /*! @brief Returns the number of components for the specified type. */
        static uint8_t componentCount(Components comps);

        ~NormalArray();

        /*! @brief Returns SectionType::NormalArray. */
        SectionType getType() const override;

        /*! @brief Sets the normal data and components type of this normal
         *  section.
         *  
         *  @param[in] data The normal data, containing floats
         *  @param[in] comps The normal components type
         */
        void setData(Buffer& data, Components comps = Components::Normal);

        /*! @brief Returns the components type of this normal section. */
        Components getComponentsType() const;

    private:

        NormalArray(MDL0* mdl0, const std::string& name);

        // the components type
        Components comps;
    };

    /*! @brief Contains color data of a MDL0. (Section #4) */
    class ColorArray : public ArraySection
    {

        template <class> friend class SectionContainer;

    public:

        /*! @brief The SectionType of this class. */
        constexpr static SectionType TYPE = SectionType::ColorArray;

        /*! @brief Enumeration of the possible color formats. */
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

        /*! @brief Returns the number of bytes per color for the specified
         *  format.
         */
        static uint8_t byteCount(Format format);

        /*! @brief Returns the number of components for the specified format. */
        static uint8_t componentCount(Format format);

        ~ColorArray();

        /*! @brief Returns SectionType::ColorArray. */
        SectionType getType() const override;

        /*! @brief Sets the color data and format of this color section.
         *  
         *  @param[in] data The color data, formatted as specified
         *  @param[in] format The color format
         */
        void setData(Buffer& data, Format format = Format::RGBA8);

        /*! @brief Returns the color format of this color section. */
        Format getFormat() const;

    private:

        ColorArray(MDL0* mdl0, const std::string& name);

        // color format
        Format format;
    };

    /*! @brief Contains texture coord data of a MDL0. (Section #5) */
    class TexCoordArray : public ArraySection
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

        /*! @brief Returns the number of components for the specified type. */
        static uint8_t componentCount(Components comps);

        ~TexCoordArray();

        /*! @brief Returns SectionType::TexCoordArray. */
        SectionType getType() const override;

        /*! @brief Sets the texture coord data and components type of this
         *  texture coord section.
         *  
         *  @param[in] data The texture coord data, containing floats
         *  @param[in] comps The texture coord components type
         */
        void setData(Buffer& data, Components comps = Components::ST);

        /*! @brief Returns the components type of this texture coord section. */
        Components getComponentsType() const;

        /*! @brief Returns the box minimum of this texture coord section. */
        Vector2f getBoxMin() const;

        /*! @brief Returns the box maximum of this texture coord section. */
        Vector2f getBoxMax() const;

    private:

        TexCoordArray(MDL0* mdl0, const std::string& name);

        // components type
        Components comps;

        // vec2f of box min
        Vector2f boxMin;

        // vec2f of box max
        Vector2f boxMax;
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

    /// Section-specific methods ///////

    /*! @brief Returns the first (root) bone in the bone hierarchy, or nullptr
     *  if this MDL0 has no bone.
     */
    Bone* getRootBone() const;

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

    // called when a section instance is added
    void sectionAdded(Section* instance);

    // called when a section instance is removed
    void sectionRemoved(Section* instance);

    SectionContainer<Links> linksSections;
    SectionContainer<Bone> boneSections;
    SectionContainer<VertexArray> verticesSections;
    SectionContainer<NormalArray> normalsSections;
    SectionContainer<ColorArray> colorsSections;
    SectionContainer<TexCoordArray> texCoordsSections;

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

    // throws if index >= mipmaps.size()
    void assertValidMipmap(uint32_t index) const;

    // throws if count is 0 or any mipmap between 0 and count would have a dimension of 0
    void assertValidMipmapCount(uint32_t count) const;

    // throws if mipmaps.size() < index
    void assertValidMipmapInsert(uint32_t index) const;

    // throws if the dimensions of image are invalid for index
    void assertValidMipmapImage(uint32_t index, const Image& image) const;

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

    /*! @brief  */
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

private:

    // map of <name, MDL0> containing all MDL0s in this BRRES
    std::map<std::string, MDL0*> mdl0s;

    // map of <name, TEX0> containing all TEX0s in this BRRES
    std::map<std::string, TEX0*> tex0s;
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
