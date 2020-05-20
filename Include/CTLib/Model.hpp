//////////////////////////////////////////////////
//  Copyright (c) 2020 Nara Hiero
//
// This file is licensed under GPLv3+
// Refer to the `License.txt` file included.
//////////////////////////////////////////////////

#pragma once

/*! @file Model.hpp
 * 
 *  @brief The header containing all model-related utilities.
 */


#include <map>
#include <vector>

#include <CTLib/Math.hpp>
#include <CTLib/Memory.hpp>


namespace CTLib
{

/*! @brief The model class is used to interact with graphics models. */
class Model final
{

public:

    /*! @brief Enumeration of the possible type values. */
    enum class Type
    {
        /*! @brief Vertex position. */
        Position0 = 0x100,

        /*! @brief Vertex normal. */
        Normal0 = 0x200,

        /*! @brief Texture coordinate 0. */
        TexCoord0 = 0x300,

        /*! @brief Texture coordinate 1. */
        TexCoord1 = 0x301,

        /*! @brief Texture coordinate 2. */
        TexCoord2 = 0x302,

        /*! @brief Texture coordinate 3. */
        TexCoord3 = 0x303,

        /*! @brief Texture coordinate 4. */
        TexCoord4 = 0x304,

        /*! @brief Texture coordinate 5. */
        TexCoord5 = 0x305,

        /*! @brief Texture coordinate 6. */
        TexCoord6 = 0x306,

        /*! @brief Texture coordinate 7. */
        TexCoord7 = 0x307,

        /*! @brief Vertex colour 0. */
        Colour0 = 0x400,

        /*! @brief Vertex colour 1. */
        Colour1 = 0x401,

        /*! @brief KCL flag. */
        KCLFlag = 0x10100
    };

    /*! @brief Enumeration of the possible data type values. */
    enum class DataType
    {
        /*! @brief Unsigned byte (unsigned, 8 bits). */
        UInt8 = 0x0,

        /*! @brief Signed byte (signed, 8 bits). */
        Int8 = 0x1,

        /*! @brief Unsigned short (unsigned, 16 bits). */
        UInt16 = 0x2,

        /*! @brief Signed short (signed, 16 bits). */
        Int16 = 0x3,

        /*! @brief Unsigned int (unsigned, 32 bits). */
        UInt32 = 0x4,

        /*! @brief Signed int (signed, 32 bits). */
        Int32 = 0x5,

        /*! @brief Float (floating-point, 32 bits). */
        Float = 0x10,

        /*! @brief Double (floating-point, 64 bits). */
        Double = 0x11
    };

    /*! @brief Class describing the format of data.
     *  
     *  The properties and their meaning are listed below:
     *  
     *  `type`: The primitive type of each _component_
     *  `size`: The number of _components_ per _value_
     *  `count`: The number of _values_ per _face_
     *  `indexed`: Whether the data is indexed
     *  `indexType`: The primitive type of each _index_
     */
    class DataFormat final
    {

        friend class Model;

    public:

        /*! @brief Returns the default DataFormat for the specified type. */
        static DataFormat getDefault(Type type);

        /*! @brief Constructs a DataFormat instance with the default values.
         *  
         *  The default value of each property is listed below:
         *  
         *  `type`: DataType::Float
         *  `size`: 3
         *  `count`: 3
         *  `indexed`: false
         *  `indexType`: DataType::UInt16
         */
        DataFormat();

        /*! @brief Constructs a DataFormat instance with the default values for
         *  the specified Type.
         *  
         *  The defaults from the default constructor are used if no defaults
         *  are set for the specified type.
         *  
         *  @param[in] type The Type to use defaults from
         */
        DataFormat(Type type);

        /*! @brief Constructs a DataFormat instance with the specified values.
         *  
         *  @param[in] type The primitive type of each component
         *  @param[in] size The number of component per value
         *  @param[in] count The number of values per face
         *  @param[in] indexed Whether the data is indexed
         *  @param[in] indexType The primitive type of each index
         * 
         *  @throw CTLib::ModelError If the specified size is 0, or the
         *  specified count is 0, or the specified index type is not integral
         *  or is not unsigned.
         */
        DataFormat(
            DataType type, uint32_t size, uint32_t count, bool indexed, DataType indexType
        );

        /*! @brief Constructs a copy of the specified DataFormat instance. */
        DataFormat(const DataFormat&);

        /*! @brief Returns whether this DataFormat equals the specified one. */
        bool operator==(const DataFormat& rhs) const;

        /*! @brief Sets the primitive type of data. */
        void setType(DataType type);

        /*! @brief Sets the number of components per value.
         *  
         *  @throw CTLib::ModelError If the specified size is 0.
         */
        void setSize(uint32_t size);

        /*! @brief Sets the number of values per face.
         *  
         *  @throw CTLib::ModelError If the specified count is 0.
         */
        void setCount(uint32_t count);

        /*! @brief Sets whether the data is indexed. */
        void setIsIndexed(bool indexed);

        /*! @brief Sets the primitive type of index data.
         *  
         *  @throw CTLib::ModelError If the specified DataType is not integral
         *  or is not unsigned.
         */
        void setIndexType(DataType type);

        /*! @brief Returns the primitive type of data. */
        DataType getType() const;

        /*! @brief Returns the number of components per value. */
        uint32_t getSize() const;

        /*! @brief Returns the number of value per face. */
        uint32_t getCount() const;

        /*! @brief Returns whether the data is indexed. */
        bool isIndexed() const;

        /*! @brief Returns the primitive type of index data. */
        DataType getIndexType() const;

    private:

        // throws if 'size' == 0
        void assertValidSize(uint32_t size) const;

        // throws if 'count' == 0
        void assertValidCount(uint32_t count) const;

        // throws if '!isIntegral(type)' or 'isSigned(type)'
        void assertValidIndexType(DataType type) const;

        // data type
        DataType type;

        // components per vertex
        uint32_t size;

        // vertex per face
        uint32_t count;

        // whether to use index data
        bool indexed;

        // index data type
        DataType indexType;
    };

    class FaceIterator;

    /*! @brief A face within a Model. */
    class Face final
    {

        friend class Model;

    public:

        /*! @brief A value within a Face. */
        class Value final
        {

            friend class Face;

        public:

            /*! @brief Constructs a copy of the specified Value. */
            Value(const Value&);

            /*! @brief Returns the size in bytes of the specified Value. */
            static uint32_t sizeOf(const Value& value);

            /*! @brief Returns the number of components in this Value. */
            uint32_t getSize() const;

            /*! @brief Returns the DataType of components in this Value. */
            DataType getType() const;

            /*! @brief Returns the specified byte component of this value.
             *  
             *  @param[in] index The component index
             *  
             *  @throw CTLib::ModelError If the DataType of this Value is not
             *  DataType::UInt8 or DataType::Int8, or the specified index is
             *  more than or equal to the size of this Value.
             */
            uint8_t asByte(uint32_t index = 0) const;

            /*! @brief Returns the specified short component of this value.
             *  
             *  @param[in] index The component index
             *  
             *  @throw CTLib::ModelError If the DataType of this Value is not
             *  DataType::UInt16 or DataType::Int16, or the specified index is
             *  more than or equal to the size of this Value.
             */
            uint16_t asShort(uint32_t index = 0) const;

            /*! @brief Returns the specified int component of this value.
             *  
             *  @param[in] index The component index
             *  
             *  @throw CTLib::ModelError If the DataType of this Value is not
             *  DataType::UInt32 or DataType::Int32, or the specified index is
             *  more than or equal to the size of this Value.
             */
            uint32_t asInt(uint32_t index = 0) const;

            /*! @brief Returns the specified float component of this value.
             *  
             *  @param[in] index The component index
             *  
             *  @throw CTLib::ModelError If the DataType of this Value is not
             *  DataType::Float, or the specified index is more than or equal to
             *  the size of this Value.
             */
            float asFloat(uint32_t index = 0) const;

            /*! @brief Returns the specified double component of this value.
             *  
             *  @param[in] index The component index
             *  
             *  @throw CTLib::ModelError If the DataType of this Value is not
             *  DataType::Double, or the specified index is more than or equal
             *  to the size of this Value.
             */
            double asDouble(uint32_t index = 0) const;

        private:

            Value(DataFormat format, Buffer& data, uint32_t index);

            // throws if 'types' does not contain 'type'
            void assertAnyType(const std::vector<DataType>& types) const;

            // throws if 'index' >= 'size'
            void assertValidIndex(uint32_t index) const;

            // component count
            const uint32_t size;

            // primitive type
            const DataType type;

            // buffer containing data
            Buffer data;
        };

        /*! @brief Returns the size in bytes of the specified Face. */
        static uint32_t sizeOf(const Face& face);

        /*! @brief Returns the number of values in this Face. */
        uint32_t getCount() const;

        /*! @brief Returns the Value at the specified index.
         *  
         *  @throw CTLib::ModelError If the specified index is more than or
         *  equal to the value count of this Face.
         */
        Value get(uint32_t index = 0) const;

    private:

        Face(DataFormat format, Buffer& data, const std::vector<uint32_t>& indices);

        // throws if 'index' >= 'count'
        void assertValidIndex(uint32_t index) const;

        // value count
        const uint32_t count;

        // component count of values
        const uint32_t size;

        // primitive type of components of values
        const DataType type;

        // vector containing values
        std::vector<Value> values;
    };

    /*! @brief A FaceIterator iterates over the faces in a Model.
     *  
     *  An example on how to use this class in a `for`-loop follows:
     *  
     *  ```{.cpp}
     *  Model model;
     *  
     *  // setup 'model'...
     *  
     *  for (auto it = model.iterateFaces(); it; ++it)
     *  {
     *      Model::Face face = it.get(Model::Type::Position0);
     *      
     *      // use 'face'...
     *  }
     *  ```
     */
    class FaceIterator final
    {

    public:

        /*! @brief Constructs a FaceIterator to iterate over the faces of the
         *  specified Model.
         *  
         *  @param[in] model A pointer to the Model to be iterated over
         */
        FaceIterator(const Model* model);

        /*! @brief Returns whether this FaceIterator is still in bounds.
         *  
         *  This FaceIterator is 'in bounds', if its current position is less
         *  than the return value of Model::getFaceCount().
         */
        operator bool() const;

        /*! @brief Pre-increments the position of this FaceIterator. */
        FaceIterator& operator++();

        /*! @brief Returns the Face of the specified Type at the current
         *  position.
         *  
         *  @param[in] type The type of data
         *  
         *  @throw CTLib::ModelError If the Model being iterated by this
         *  FaceIterator does not have data of the specified Type, or this
         *  FaceIterator is 'out of bounds', or the data of the specified Type
         *  is indexed and the current Face contains out of range indices.
         */
        Face get(Type type) const;

    private:

        // throws if '!(*this)'
        void assertInBounds() const;

        // pointer to model
        const Model* model;

        // current position
        uint32_t pos;
    };

    /*! @brief Returns the name of the specified Type. */
    static const char* nameOf(Type type);

    /*! @brief Returns the name of the specified DataType. */
    static const char* nameOf(DataType type);

    /*! @brief Returns the size in bytes of the specified DataType. */
    static uint8_t sizeOf(DataType type);

    /*! @brief Returns whether the specified DataType is integral. */
    static bool isIntegral(DataType type);

    /*! @brief Returns whether the specified DataType is signed
     *  (integral types only).
     */
    static bool isSigned(DataType type);

    /*! @brief Constructs an empty Model instance. */
    Model();

    /*! @brief Constructs a copy of the specified Model instance. */
    Model(const Model&);

    /*! @brief Moves the specified Model instance to a newly created one.
     *  
     *  The source Model will then be empty, but may still be safely used.
     */
    Model(Model&&);

    ~Model();

    /*! @brief Sets the data of the specified type of this Model to the data
     *  remaining in the specified buffer.
     * 
     *  @param[in] type The Type of data
     *  @param[in] data The buffer containing the data
     */
    void setData(Type type, Buffer& data);

    /*! @brief Sets the index data of the specified type of this Model to the
     *  data remaining in the specified buffer.
     *  
     *  @param[in] type The Type of data
     *  @param[in] data The buffer containing the index data
     */
    void setIndexData(Type type, Buffer& data);

    /*! @brief Sets the data format of the specified type of this Model to the
     *  specified data format.
     *  
     *  @param[in] type The Type of data
     *  @param[in] format The DataFormat
     */
    void setDataFormat(Type type, DataFormat format);

    /*! @brief Returns whether this Model has data of the specified Type.
     *  
     *  @param[in] type The type of data
     */
    bool hasData(Type type) const;

    /*! @brief Returns the data of the specified Type of this Model.
     *  
     *  The returned Buffer shares the same memory as the internal buffer of
     *  this Model, so changes made to it will be reflected to the Model's
     *  data.
     *  
     *  @param[in] type The type of data
     *  
     *  @throw CTLib::ModelError If this Model does not have data of the
     *  specified Type.
     */
    Buffer getData(Type type) const;

    /*! @brief Returns whether this Model has index data of the specified Type.
     *  
     *  @param[in] type The type of data
     */
    bool hasIndexData(Type type) const;

    /*! @brief Returns the index data of the specified Type of this Model.
     *  
     *  The returned Buffer shares the same memory as the internal buffer of
     *  this Model, so changes made to it will be reflected to the Model's
     *  index data.
     *  
     *  @param[in] type The type of data
     *  
     *  @throw CTLib::ModelError If this Model does not have index data of the
     *  specified Type.
     */
    Buffer getIndexData(Type type) const;

    /*! @brief Return the DataFormat of the specified Type of this Model.
     *  
     *  @param[in] type The type of data
     *  
     *  @throw CTLib::ModelError If this Model does not have data of the
     *  specified Type.
     */
    DataFormat getDataFormat(Type type) const;

    /*! @brief Returns the number of faces of the specified Type of this Model.
     *  
     *  @param[in] type The type of data
     * 
     *  @throw CTLib::ModelError If this Model does not have data of the
     *  specified Type.
     */
    uint32_t getFaceCount(Type type) const;

    /*! @brief Returns the number of 'complete' faces in this Model.
     *  
     *  A face is 'complete' if it has data for all set types.
     *  
     *  The returned value is simply the smallest value returned from calls to
     *  Model::getFaceCount(Type), with it invoked once for every Type of data
     *  set in this Model.
     */
    uint32_t getFaceCount() const;

    /*! @brief Returns the Face of the specified Type of this Model at the
     *  specified index.
     *  
     *  The index data (or global index data) is taken into consideration when
     *  applicable.
     * 
     *  @param[in] type The type of data
     *  @param[in] index The face index
     *  
     *  @throw CTLib::ModelError If this Model does not have data of the
     *  specified Type, or the specified index is more than or equal to the
     *  number of faces of the specified Type, or the data of the specified
     *  Type is indexed and the Face at the specified index contains out of
     *  range indices.
     */
    Face getFace(Type type, uint32_t index) const;

    /*! @brief Returns a FaceIterator to iterate over the faces of this Model.
     *  
     *  **Note**: Modifying this Model while using the returned FaceIterator is
     *  _undefined behaviour_.
     */
    FaceIterator iterateFaces() const;

private:

    // adds the default format for the specified type to 'formatMap'
    void addDefaultFormat(Type type);

    // returns the index at the current buffer position with the appropriate type
    uint32_t getFaceIndex(Buffer& buffer, DataType type) const;

    // throws if '!isIntegral(type)' or 'isSigned(type)'
    void assertValidIndexType(DataType type) const;

    // throws if 'dataMap.count(type) == 0'
    void assertHasData(Type type) const;

    // throws if 'indexMap.count(type) == 0'
    void assertHasIndexData(Type type) const;

    // throws if 'index' >= 'getFaceCount(type)'
    void assertValidFaceIndex(Type type, uint32_t index) const;

    // throws if 'index' >= the number of values
    void assertValidValueIndex(Type type, uint32_t index) const;

    // map containing all data of this model
    std::map<Type, Buffer> dataMap;

    // map containing all index data of this model
    std::map<Type, Buffer> indexMap;

    // map containing all data format of this model
    std::map<Type, DataFormat> formatMap;
};

/*! @brief ModelError is the error class used by the methods in this header. */
class ModelError : public std::runtime_error
{

public:

    /*! @brief Constructs a CTLib::ModelError with the specified message. */
    ModelError(const char* msg);

    /*! @brief Constructs a CTLib::ModelError with the specified message. */
    ModelError(const std::string& msg);
};
}
