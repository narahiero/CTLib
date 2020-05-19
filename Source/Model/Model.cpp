//////////////////////////////////////////////////
//  Copyright (c) 2020 Nara Hiero
//
// This file is licensed under GPLv3+
// Refer to the `License.txt` file included.
//////////////////////////////////////////////////

#include <CTLib/Model.hpp>

#include <CTLib/Utilities.hpp>
#include <iostream>
namespace CTLib
{

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////
////   Model class
////

const char* Model::nameOf(Type type)
{
    switch (type)
    {
    case Type::Position0:
        return "Position0";

    case Type::Normal0:
        return "Normal0";

    case Type::TexCoord0:
        return "TexCoord0";

    case Type::TexCoord1:
        return "TexCoord1";

    case Type::TexCoord2:
        return "TexCoord2";

    case Type::TexCoord3:
        return "TexCoord3";

    case Type::TexCoord4:
        return "TexCoord4";

    case Type::TexCoord5:
        return "TexCoord5";

    case Type::TexCoord6:
        return "TexCoord6";

    case Type::TexCoord7:
        return "TexCoord7";

    case Type::Colour0:
        return "Colour0";

    case Type::Colour1:
        return "Colour1";

    case Type::KCLFlag:
        return "KCLFlag";

    default:
        return "";
    }
}

const char* Model::nameOf(DataType type)
{
    switch (type)
    {
    case DataType::UInt8:
        return "UInt8";

    case DataType::Int8:
        return "Int8";

    case DataType::UInt16:
        return "UInt16";

    case DataType::Int16:
        return "Int16";

    case DataType::UInt32:
        return "UInt32";

    case DataType::Int32:
        return "Int32";

    case DataType::Float:
        return "Float";

    case DataType::Double:
        return "Double";

    default:
        return "";
    }
}

uint8_t Model::sizeOf(DataType type)
{
    switch (type)
    {
    case DataType::UInt8:
    case DataType::Int8:
        return 1;

    case DataType::UInt16:
    case DataType::Int16:
        return 2;

    case DataType::UInt32:
    case DataType::Int32:
    case DataType::Float:
        return 4;

    case DataType::Double:
        return 8;

    default:
        return 0;
    }
}

bool Model::isIntegral(DataType type)
{
    switch (type)
    {
    case DataType::UInt8:
    case DataType::Int8:
    case DataType::UInt16:
    case DataType::Int16:
    case DataType::UInt32:
    case DataType::Int32:
        return true;

    default:
        return false;
    }
}

bool Model::isSigned(DataType type)
{
    switch (type)
    {
    case DataType::Int8:
    case DataType::Int16:
    case DataType::Int32:
        return true;

    default:
        return false;
    }
}

Model::Model() :
    globalIndices{false},
    forceIndices{false},
    globalIndexData{},
    globalIndexType{DataType::UInt16},
    dataMap{},
    indexMap{},
    formatMap{}
{

}

Model::Model(const Model& src) :
    globalIndices{src.globalIndices},
    forceIndices{src.forceIndices},
    globalIndexData{src.globalIndexData},
    globalIndexType{src.globalIndexType},
    dataMap{src.dataMap},
    indexMap{src.indexMap},
    formatMap{src.formatMap}
{

}

Model::Model(Model&& src) :
    globalIndices{src.globalIndices},
    forceIndices{src.forceIndices},
    globalIndexData{std::move(src.globalIndexData)},
    globalIndexType{src.globalIndexType},
    dataMap{std::move(src.dataMap)},
    indexMap{std::move(src.indexMap)},
    formatMap{std::move(src.formatMap)}
{

}

Model::~Model()
{

}

void Model::setGlobalIndicesMode(bool enable, bool force)
{
    globalIndices = enable;
    forceIndices = force;
}

void Model::setGlobalIndexData(Buffer& data, DataType type)
{
    assertValidIndexType(type);

    globalIndexData = Buffer(data.remaining());
    globalIndexData.put(data).flip();

    globalIndexType = type;
}

void Model::setData(Type type, Buffer& data)
{
    if (formatMap.count(type) == 0)
    {
        addDefaultFormat(type);
    }
    
    Buffer buf(data.remaining());
    buf.put(data).flip();
    dataMap[type] = std::move(buf);
}

void Model::setIndexData(Type type, Buffer& data)
{
    if (formatMap.count(type) == 0)
    {
        addDefaultFormat(type);
    }

    Buffer buf(data.remaining());
    buf.put(data).flip();
    indexMap[type] = std::move(buf);
}

void Model::setDataFormat(Type type, DataFormat format)
{
    formatMap[type] = format;
}

bool Model::isGlobalIndexModeEnabled() const
{
    return globalIndices;
}

bool Model::isForceUseIndicesEnabled() const
{
    return forceIndices;
}

Buffer Model::getGlobalIndexData() const
{
    return globalIndexData.duplicate();
}

Model::DataType Model::getGlobalIndexDataType() const
{
    return globalIndexType;
}

bool Model::hasData(Type type) const
{
    return dataMap.count(type) > 0;
}

Buffer Model::getData(Type type) const
{
    assertHasData(type);
    return dataMap.at(type);
}

bool Model::hasIndexData(Type type) const
{
    return indexMap.count(type) > 0;
}

Buffer Model::getIndexData(Type type) const
{
    assertHasIndexData(type);
    return indexMap.at(type);
}

Model::DataFormat Model::getDataFormat(Type type) const
{
    assertHasData(type);
    return formatMap.at(type);
}

uint32_t Model::getFaceCount(Type type) const
{
    assertHasData(type);

    DataFormat format = formatMap.at(type);
    if (globalIndices && (forceIndices || format.indexed))
    {
        return static_cast<uint32_t>(
            globalIndexData.capacity() / sizeOf(globalIndexType) / format.count
        );
    }
    else if (format.indexed)
    {
        return static_cast<uint32_t>(
            indexMap.at(type).capacity() / sizeOf(format.indexType) / format.count
        );
    }
    else
    {
        return static_cast<uint32_t>(
            dataMap.at(type).capacity() / sizeOf(format.type) / format.size / format.count
        );
    }
}

void Model::addDefaultFormat(Type type)
{
    formatMap.insert(std::map<Type, DataFormat>::value_type(type, DataFormat(type)));
}

void Model::assertValidIndexType(DataType type) const
{
    if (!isIntegral(type) || isSigned(type))
    {
        throw ModelError(Strings::format(
            "Invalid index type! Type must be integral and unsigned! (%s)",
            nameOf(type)
        ));
    }
}

void Model::assertHasData(Type type) const
{
    if (dataMap.count(type) == 0)
    {
        throw ModelError(Strings::format(
            "This model has no data of the specified type! (%s)",
            nameOf(type)
        ));
    }
}

void Model::assertHasIndexData(Type type) const
{
    if (indexMap.count(type) == 0)
    {
        throw ModelError(Strings::format(
            "This model has no index data of the specified type! (%s)",
            nameOf(type)
        ));
    }
}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////
////   DataFormat class
////

Model::DataFormat Model::DataFormat::getDefault(Type type)
{
    switch (type)
    {
    case Type::Position0:
        return {DataType::Float, 3, 3, false, DataType::UInt16};

    case Type::Normal0:
        return {DataType::Float, 3, 3, false, DataType::UInt16};

    case Type::TexCoord0:
    case Type::TexCoord1:
    case Type::TexCoord2:
    case Type::TexCoord3:
    case Type::TexCoord4:
    case Type::TexCoord5:
    case Type::TexCoord6:
    case Type::TexCoord7:
        return {DataType::Float, 2, 3, false, DataType::UInt16};

    case Type::Colour0:
    case Type::Colour1:
        return {DataType::UInt32, 1, 3, false, DataType::UInt16};

    case Type::KCLFlag:
        return {DataType::UInt16, 1, 1, false, DataType::UInt16};

    default:
        return {};
    }
}

Model::DataFormat::DataFormat() :
    DataFormat(DataType::Float, 3, 3, false, DataType::UInt16)
{

}

Model::DataFormat::DataFormat(Type type) :
    DataFormat(getDefault(type))
{

}

Model::DataFormat::DataFormat(
    DataType type, uint32_t size, uint32_t count, bool indexed, DataType indexType
) :
    type{type},
    size{size},
    count{count},
    indexed{indexed},
    indexType{indexType}
{
    assertValidSize(size);
    assertValidCount(count);
    assertValidIndexType(indexType);
}

Model::DataFormat::DataFormat(const DataFormat& src) :
    DataFormat(src.type, src.size, src.count, src.indexed, src.indexType)
{

}

bool Model::DataFormat::operator==(const DataFormat& rhs) const
{
    return type == rhs.type && size == rhs.size && count == rhs.count
        && indexed == rhs.indexed && indexType == rhs.indexType;
}

void Model::DataFormat::setType(DataType type)
{
    this->type = type;
}

void Model::DataFormat::setSize(uint32_t size)
{
    assertValidSize(size);
    this->size = size;
}

void Model::DataFormat::setCount(uint32_t count)
{
    assertValidCount(count);
    this->count = count;
}

void Model::DataFormat::setIsIndexed(bool indexed)
{
    this->indexed = indexed;
}

void Model::DataFormat::setIndexType(DataType type)
{
    assertValidIndexType(type);
    indexType = type;
}

Model::DataType Model::DataFormat::getType() const
{
    return type;
}

uint32_t Model::DataFormat::getSize() const
{
    return size;
}

uint32_t Model::DataFormat::getCount() const
{
    return count;
}

bool Model::DataFormat::isIndexed() const
{
    return indexed;
}

Model::DataType Model::DataFormat::getIndexType() const
{
    return indexType;
}

void Model::DataFormat::assertValidSize(uint32_t size) const
{
    if (size == 0)
    {
        throw ModelError("Invalid component count (size)! Size is 0!");
    }
}

void Model::DataFormat::assertValidCount(uint32_t count) const
{
    if (count == 0)
    {
        throw ModelError("Invalid number of values per face (count)! Count is 0!");
    }
}

void Model::DataFormat::assertValidIndexType(DataType type) const
{
    if (!isIntegral(type) || isSigned(type))
    {
        throw ModelError(Strings::format(
            "Invalid model index type! Type must be integral and unsigned! (%s)",
            nameOf(type)
        ));
    }
}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////
////   Model error class
////

ModelError::ModelError(const char* msg) : std::runtime_error(msg)
{

}

ModelError::ModelError(const std::string& msg) : std::runtime_error(msg)
{

}
}
