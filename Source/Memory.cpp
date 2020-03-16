//////////////////////////////////////////////////
//  Copyright (c) 2020 Nara Hiero
//
// This file is licensed under GPLv3+
// Refer to the `License.txt` file included.
//////////////////////////////////////////////////

#include <CTLib/Memory.hpp>

#include <sstream>

#ifndef CT_LIB_NO_SAFETY_CHECKS
#define ASSERT_VALID_SIZE(s) assertValidSize(s)
#define ASSERT_VALID_POS(p) assertValidPos(p)
#define ASSERT_VALID_LIMIT(l) assertValidLimit(l)
#define ASSERT_REMAINING(i, n) assertRemaining(i, n)
#else
#define ASSERT_VALID_SIZE(s) do { } while (false)
#define ASSERT_VALID_POS(p) do { } while (false)
#define ASSERT_VALID_LIMIT(l) do { } while (false)
#define ASSERT_REMAINING(i, n) do { } while (false)
#endif

namespace CTLib
{

Buffer::Buffer(size_t size) :
    buffer{nullptr},
    size{size},
    pos{0},
    max{size},
    endian{BIG_ENDIAN}
{
    ASSERT_VALID_SIZE(size);
    buffer = new uint8_t[size];
}

Buffer::Buffer(const Buffer& src) :
    buffer{nullptr},
    size{src.size},
    pos{src.pos},
    max{src.max},
    endian{src.endian}
{
    buffer = new uint8_t[size];
    for (size_t i = 0; i < size; ++i)
    {
        buffer[i] = src.buffer[i];
    }
}

Buffer::Buffer(Buffer&& src) noexcept :
    buffer{src.buffer},
    size{src.size},
    pos{src.pos},
    max{src.max},
    endian{src.endian}
{
    src.buffer = nullptr;
}

Buffer::~Buffer()
{
    delete [] buffer;
}

Buffer& Buffer::operator=(const Buffer& src)
{
    if (this == &src)
    {
        return *this;
    }
    uint8_t* tmp = new uint8_t[size];
    for (size_t i = 0; i < size; ++i)
    {
        tmp[i] = src.buffer[i];
    }
    delete [] buffer;
    buffer = tmp;
    size = src.size;
    pos = src.pos;
    max = src.max;
    endian = src.endian;
    return *this;
}

Buffer& Buffer::operator=(Buffer&& src) noexcept
{
    delete [] buffer;
    buffer = src.buffer;
    src.buffer = nullptr;
    size = src.size;
    pos = src.pos;
    max = src.max;
    endian = src.endian;
    return *this;
}

uint8_t* Buffer::operator*() const noexcept
{
    return buffer;
}

uint8_t Buffer::operator[](size_t index) const
{
    return buffer[index];
}

Buffer& Buffer::resize(size_t newsize)
{
    ASSERT_VALID_SIZE(newsize);
    if (newsize == size)
    {
        return *this;
    }
    uint8_t* tmp = new uint8_t[newsize];
    size_t i;
    for (i = 0; i < (newsize > size ? size : newsize); ++i)
    {
        tmp[i] = buffer[i];
    }
    for (; i < newsize; ++i)
    {
        tmp[i] = 0;
    }
    delete [] buffer;
    buffer = tmp;
    size = newsize;
    pos = pos > size ? size : pos;
    max = max > size ? size : max;
    return *this;
}

Buffer& Buffer::truncate()
{
    ASSERT_VALID_SIZE(pos);
    if (pos == size)
    {
        return *this;
    }
    uint8_t* tmp = new uint8_t[pos];
    for (size_t i = 0; i < pos; ++i)
    {
        tmp[i] = buffer[i];
    }
    delete [] buffer;
    buffer = tmp;
    size = pos;
    max = size;
    return *this;
}

Buffer& Buffer::order(bool endian) noexcept
{
    this->endian = endian;
    return *this;
}

bool Buffer::order() const noexcept
{
    return endian;
}

size_t Buffer::capacity() const noexcept
{
    return size;
}

Buffer& Buffer::position(size_t pos)
{
    ASSERT_VALID_POS(pos);
    this->pos = pos;
    return *this;
}

size_t Buffer::position() const noexcept
{
    return pos;
}

Buffer& Buffer::limit(size_t limit)
{
    ASSERT_VALID_LIMIT(limit);
    max = limit;
    pos = pos > limit ? limit : pos;
    return *this;
}

size_t Buffer::limit() const noexcept
{
    return max;
}

size_t Buffer::remaining() const noexcept
{
    return max - pos;
}

bool Buffer::hasRemaining() const noexcept
{
    return pos < max;
}

Buffer& Buffer::clear() noexcept
{
    pos = 0;
    max = size;
    return *this;
}

Buffer& Buffer::flip() noexcept
{
    max = pos;
    pos = 0;
    return *this;
}

Buffer& Buffer::rewind() noexcept
{
    pos = 0;
    return *this;
}

Buffer& Buffer::compact()
{
    size_t r = remaining();
    for (size_t i = 0; i < r; ++i)
    {
        buffer[i] = buffer[pos + i];
    }
    pos = r;
    max = size;
    return *this;
}

Buffer& Buffer::put(uint8_t data)
{
    put(pos, data); // doing this in two statements prevents position from
    ++pos;          // incrementing in case of buffer overflow
    return *this;
}

Buffer& Buffer::put(size_t index, uint8_t data)
{
    ASSERT_REMAINING(index, 1);
    buffer[index] = data;
    return *this;
}

uint8_t Buffer::get()
{
    uint8_t ret = get(pos); // same as put()
    ++pos;
    return ret;
}

uint8_t Buffer::get(size_t index)
{
    ASSERT_REMAINING(index, 1);
    return buffer[index];
}

Buffer& Buffer::putShort(uint16_t data)
{
    putShort(pos, data);
    pos += 2;
    return *this;
}

Buffer& Buffer::putShort(size_t index, uint16_t data)
{
    ASSERT_REMAINING(index, 2);
    buffer[index + (    endian)] = (data >> 8) & 0xFF;
    buffer[index + (1 - endian)] = (data     ) & 0xFF;
    return *this;
}

uint16_t Buffer::getShort()
{
    uint16_t ret = getShort(pos);
    pos += 2;
    return ret;
}

uint16_t Buffer::getShort(size_t index)
{
    ASSERT_REMAINING(index, 2);
    return (buffer[index + (endian)] << 8) + buffer[index + (1 - endian)];
}

Buffer& Buffer::putInt(uint32_t data)
{
    putInt(pos, data);
    pos += 4;
    return *this;
}

Buffer& Buffer::putInt(size_t index, uint32_t data)
{
    ASSERT_REMAINING(index, 4);
    int32_t c = (endian ? 4 : -1), m = (endian * -2) + 1;
    buffer[index + (c += m)] = (data >> 24) & 0xFF;
    buffer[index + (c += m)] = (data >> 16) & 0xFF;
    buffer[index + (c += m)] = (data >>  8) & 0xFF;
    buffer[index + (c += m)] = (data      ) & 0xFF;
    return *this;
}

uint32_t Buffer::getInt()
{
    uint32_t ret = getInt(pos);
    pos += 4;
    return ret;
}

uint32_t Buffer::getInt(size_t index)
{
    ASSERT_REMAINING(index, 4);
    int32_t c = (endian ? 4 : -1), m = (endian * -2) + 1;
    uint32_t ret = static_cast<uint32_t>(buffer[index + (c += m)]) << 24;
    ret +=         static_cast<uint32_t>(buffer[index + (c += m)]) << 16;
    ret +=         static_cast<uint32_t>(buffer[index + (c += m)]) <<  8;
    ret +=         static_cast<uint32_t>(buffer[index + (c += m)]);
    return ret;
}

Buffer& Buffer::putLong(uint64_t data)
{
    putLong(pos, data);
    pos += 8;
    return *this;
}

Buffer& Buffer::putLong(size_t index, uint64_t data)
{
    ASSERT_REMAINING(index, 8);int32_t c = (endian ? 8 : -1), m = (endian * -2) + 1;
    buffer[index + (c += m)] = (data >> 56) & 0xFF;
    buffer[index + (c += m)] = (data >> 48) & 0xFF;
    buffer[index + (c += m)] = (data >> 40) & 0xFF;
    buffer[index + (c += m)] = (data >> 32) & 0xFF;
    buffer[index + (c += m)] = (data >> 24) & 0xFF;
    buffer[index + (c += m)] = (data >> 16) & 0xFF;
    buffer[index + (c += m)] = (data >>  8) & 0xFF;
    buffer[index + (c += m)] = (data      ) & 0xFF;
    return *this;
}

uint64_t Buffer::getLong()
{
    uint64_t ret = getLong(pos);
    pos += 8;
    return ret;
}

uint64_t Buffer::getLong(size_t index)
{
    ASSERT_REMAINING(index, 8);
    int32_t c = (endian ? 8 : -1), m = (endian * -2) + 1;
    uint64_t ret = static_cast<uint64_t>(buffer[index + (c += m)]) << 56;
    ret +=         static_cast<uint64_t>(buffer[index + (c += m)]) << 48;
    ret +=         static_cast<uint64_t>(buffer[index + (c += m)]) << 40;
    ret +=         static_cast<uint64_t>(buffer[index + (c += m)]) << 32;
    ret +=         static_cast<uint64_t>(buffer[index + (c += m)]) << 24;
    ret +=         static_cast<uint64_t>(buffer[index + (c += m)]) << 16;
    ret +=         static_cast<uint64_t>(buffer[index + (c += m)]) <<  8;
    ret +=         static_cast<uint64_t>(buffer[index + (c += m)]);
    return ret;
}

Buffer& Buffer::putFloat(float data)
{
    putFloat(pos, data);
    pos += 4;
    return *this;
}

Buffer& Buffer::putFloat(size_t index, float data)
{
    return putInt(index, reinterpret_cast<uint32_t&>(data));
}

float Buffer::getFloat()
{
    float ret = getFloat(pos);
    pos += 4;
    return ret;
}

float Buffer::getFloat(size_t index)
{
    uint32_t val = getInt(index);
    return reinterpret_cast<float&>(val);
}

Buffer& Buffer::putDouble(double data)
{
    putDouble(pos, data);
    pos += 8;
    return *this;
}

Buffer& Buffer::putDouble(size_t index, double data)
{
    return putLong(index, reinterpret_cast<uint64_t&>(data));
}

double Buffer::getDouble()
{
    double ret = getDouble(pos);
    pos += 8;
    return ret;
}

double Buffer::getDouble(size_t index)
{
    uint64_t val = getLong(index);
    return reinterpret_cast<double&>(val);
}

bool Buffer::equals(const Buffer& other) const
{
    if (this == &other)
    {
        return true;
    }
    size_t r = remaining();
    if (r != other.remaining())
    {
        return false;
    }
    for (size_t i = 0; i < r; ++i)
    {
        if (buffer[pos + i] != other.buffer[other.pos + i])
        {
            return false;
        }
    }
    return true;
}

int Buffer::compareTo(const Buffer& other) const
{
    if (this == &other)
    {
        return 0;
    }
    size_t r0 = remaining(), r1 = other.remaining();
    size_t c = r0 > r1 ? r1 : r0;
    for (size_t i = 0; i < c; ++i)
    {
        if (buffer[pos + i] != other.buffer[other.pos + i])
        {
            return (buffer[pos + i] > other.buffer[other.pos + i]) ? 1 : -1;
        }
    }
    return r0 > r1 ? 1 : r0 < r1 ? -1 : 0;
}

void Buffer::assertValidSize(size_t size) const
{
    if (size == 0)
    {
        throw BufferError(BufferError::ALLOCATE_ZERO);
    }
}

void Buffer::assertValidPos(size_t pos) const
{
    if (pos > max)
    {
        throw BufferError(BufferError::BUFFER_OVERFLOW);
    }
}

void Buffer::assertValidLimit(size_t limit) const
{
    if (limit > size)
    {
        throw BufferError(BufferError::INVALID_LIMIT);
    }
}

void Buffer::assertRemaining(size_t index, size_t count) const
{
    if ((index + count) > size)
    {
        throw BufferError(BufferError::BUFFER_OVERFLOW);
    }
}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////
////   BUFFER ERROR STUFF
////
////

BufferError::BufferError(unsigned type, const std::string& msg) :
    std::runtime_error{createMessage(type, msg)},
    type{type}
{
    
}

unsigned BufferError::getType() const
{
    return type;
}

std::string BufferError::createMessage(unsigned type, const std::string& msg)
{
    std::stringstream str;
    str << MESSAGES[type][MSG_NAME];
    str << ": ";
    str << (msg.empty() ? MESSAGES[type][MSG_DETAILS] : msg);
    return str.str();
}
}
