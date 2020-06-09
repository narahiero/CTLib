//////////////////////////////////////////////////
//  Copyright (c) 2020 Nara Hiero
//
// This file is licensed under GPLv3+
// Refer to the `License.txt` file included.
//////////////////////////////////////////////////

#include <CTLib/Memory.hpp>

#include <sstream>

#ifndef CT_LIB_NO_SAFETY_CHECKS
#define ASSERT_VALID_POS(p) assertValidPos(p)
#define ASSERT_VALID_LIMIT(l) assertValidLimit(l)
#define ASSERT_REMAINING(i, n) assertRemaining(i, n)
#else
#define ASSERT_VALID_POS(p) do { } while (false)
#define ASSERT_VALID_LIMIT(l) do { } while (false)
#define ASSERT_REMAINING(i, n) do { } while (false)
#endif

namespace CTLib
{

bool Buffer::nativeOrder()
{
    static uint16_t u16 = 0x01;
    static uint8_t* u8 = reinterpret_cast<uint8_t*>(&u16);
    return *u8;
}

Buffer::Buffer() :
    buffer{nullptr},
    size{0},
    off{0},
    pos{0},
    max{0},
    endian{BIG_ENDIAN}
{

}

Buffer::Buffer(size_t size) :
    buffer{nullptr},
    size{size},
    off{0},
    pos{0},
    max{size},
    endian{BIG_ENDIAN}
{
    if (size > 0) 
    {
        buffer = std::shared_ptr<uint8_t[]>(new uint8_t[size]);
    }
}

Buffer::Buffer(const Buffer& src) :
    buffer{nullptr},
    size{src.getSize()},
    off{src.offset()},
    pos{src.position()},
    max{src.limit()},
    endian{src.order()}
{
    if (size > 0)
    {
        buffer = std::shared_ptr<uint8_t[]>(new uint8_t[size]);
        for (size_t i = 0; i < size; ++i)
        {
            (*this)[i] = src[i];
        }
    }
}

Buffer::Buffer(Buffer&& src) noexcept :
    buffer{std::move(src.buffer)},
    size{src.getSize()},
    off{src.offset()},
    pos{src.position()},
    max{src.limit()},
    endian{src.order()}
{
    src.setSize(0);
    src.offset(0);
    src.position(0);
    src.limit(0);
}

Buffer::Buffer(const Buffer* src, size_t off) :
    buffer{src->buffer},
    size{src->getSize()},
    off{src->offset() + off},
    pos{src->position() - off},
    max{src->limit() - off},
    endian{src->order()}
{

}

Buffer::~Buffer()
{
    
}

Buffer& Buffer::operator=(const Buffer& src)
{
    if (this == &src)
    {
        return *this;
    }
    if (src.getSize() > 0)
    {
        buffer = std::shared_ptr<uint8_t[]>(new uint8_t[src.getSize()]);
    }
    setSize(src.getSize());
    offset(src.offset());
    position(src.position());
    limit(src.limit());
    order(src.order());
    for (size_t i = 0; i < getSize(); ++i)
    {
        (*this)[i] = src[i];
    }
    return *this;
}

Buffer& Buffer::operator=(Buffer&& src) noexcept
{
    buffer = std::move(src.buffer);
    setSize(src.getSize());
    offset(src.offset());
    position(src.position());
    limit(src.limit());
    order(src.order());
    src.setSize(0);
    src.offset(0);
    src.position(0);
    src.limit(0);
    return *this;
}

uint8_t* Buffer::operator*() const noexcept
{
    return buffer.get() + offset();
}

uint8_t& Buffer::operator[](size_t index) const
{
    return buffer[offset() + index];
}

bool Buffer::operator==(const Buffer& other) const
{
    if (this == &other)
    {
        return true;
    }
    size_t c = capacity();
    if (c != other.capacity())
    {
        return false;
    }
    for (size_t i = 0; i < c; ++i)
    {
        if((*this)[i] != other[i])
        {
            return false;
        }
    }
    return true;
}

bool Buffer::operator!=(const Buffer& other) const
{
    return !((*this) == other);
}

bool Buffer::operator<(const Buffer& other) const
{
    return fullCompare(other) < 0;
}

bool Buffer::operator<=(const Buffer& other) const
{
    return fullCompare(other) <= 0;
}

bool Buffer::operator>(const Buffer& other) const
{
    return fullCompare(other) > 0;
}

bool Buffer::operator>=(const Buffer& other) const
{
    return fullCompare(other) >= 0;
}

Buffer Buffer::duplicate() const
{
    return Buffer(this, 0);
}

Buffer Buffer::slice() const
{
    return Buffer(this, position());
}

Buffer& Buffer::order(bool endian) noexcept
{
#ifdef CT_LIB_USE_ATOMIC_BUFFER_STATES
    this->endian.store(endian, std::memory_order_relaxed);
#else
    this->endian = endian;
#endif
    return *this;
}

bool Buffer::order() const noexcept
{
#ifdef CT_LIB_USE_ATOMIC_BUFFER_STATES
    return endian.load(std::memory_order_relaxed);
#else
    return endian;
#endif
}

size_t Buffer::capacity() const noexcept
{
#ifdef CT_LIB_USE_ATOMIC_BUFFER_STATES
    return size.load(std::memory_order_relaxed) - offset();
#else
    return size - off;
#endif
}

Buffer& Buffer::position(size_t pos)
{
    ASSERT_VALID_POS(pos);
#ifdef CT_LIB_USE_ATOMIC_BUFFER_STATES
    this->pos.store(pos, std::memory_order_relaxed);
#else
    this->pos = pos;
#endif
    return *this;
}

size_t Buffer::position() const noexcept
{
#ifdef CT_LIB_USE_ATOMIC_BUFFER_STATES
    return pos.load(std::memory_order_relaxed);
#else
    return pos;
#endif
}

Buffer& Buffer::limit(size_t limit)
{
    ASSERT_VALID_LIMIT(limit);
#ifdef CT_LIB_USE_ATOMIC_BUFFER_STATES
    max.store(limit, std::memory_order_relaxed);
    pos.store(position() > limit ? limit : position());
#else
    max = limit;
    pos = pos > limit ? limit : pos;
#endif
    return *this;
}

size_t Buffer::limit() const noexcept
{
#ifdef CT_LIB_USE_ATOMIC_BUFFER_STATES
    return max.load(std::memory_order_relaxed);
#else
    return max;
#endif
}

size_t Buffer::remaining() const noexcept
{
    return limit() - position();
}

bool Buffer::hasRemaining() const noexcept
{
    return position() < limit();
}

Buffer& Buffer::clear() noexcept
{
    position(0);
    limit(capacity());
    return *this;
}

Buffer& Buffer::flip() noexcept
{
    limit(position());
    position(0);
    return *this;
}

Buffer& Buffer::rewind() noexcept
{
    position(0);
    return *this;
}

Buffer& Buffer::compact()
{
    size_t r = remaining();
    for (size_t i = 0; i < r; ++i)
    {
        (*this)[i] = (*this)[position() + i];
    }
    position(r);
    limit(capacity());
    return *this;
}

Buffer& Buffer::put(uint8_t data)
{
    put(position(), data); // doing this in two statements prevents position from
    move(1);               // incrementing in case of buffer overflow
    return *this;
}

Buffer& Buffer::put(size_t index, uint8_t data)
{
    ASSERT_REMAINING(index, 1);
    (*this)[index] = data;
    return *this;
}

uint8_t Buffer::get()
{
    uint8_t ret = get(position()); // same as put()
    move(1);
    return ret;
}

uint8_t Buffer::get(size_t index) const
{
    ASSERT_REMAINING(index, 1);
    return (*this)[index];
}

Buffer& Buffer::put(Buffer& data)
{
    size_t r = data.remaining();
    put(position(), data);
    move(r);
    return *this;
}

Buffer& Buffer::put(size_t index, Buffer& data)
{
    ASSERT_REMAINING(index, data.remaining());
    for (size_t i = index; data.hasRemaining(); ++i)
    {
        (*this)[i] = data.get();
    }
    return *this;
}

Buffer& Buffer::putArray(uint8_t* data, size_t size)
{
    putArray(position(), data, size);
    move(size);
    return *this;
}

Buffer& Buffer::putArray(size_t index, uint8_t* data, size_t size)
{
    ASSERT_REMAINING(index, size);
    for (size_t i = 0; i < size; ++i)
    {
        (*this)[index + i] = data[i];
    }
    return *this;
}

Buffer& Buffer::getArray(uint8_t* out, size_t size)
{
    getArray(position(), out, size);
    move(size);
    return *this;
}

Buffer& Buffer::getArray(size_t index, uint8_t* out, size_t size)
{
    ASSERT_REMAINING(index, size);
    for (size_t i = 0; i < size; ++i)
    {
        out[i] = (*this)[i];
    }
    return *this;
}

Buffer& Buffer::putShort(uint16_t data)
{
    putShort(position(), data);
    move(2);
    return *this;
}

Buffer& Buffer::putShort(size_t index, uint16_t data)
{
    ASSERT_REMAINING(index, 2);
    (*this)[index + (    order())] = (data >> 8) & 0xFF;
    (*this)[index + (1 - order())] = (data     ) & 0xFF;
    return *this;
}

uint16_t Buffer::getShort()
{
    uint16_t ret = getShort(position());
    move(2);
    return ret;
}

uint16_t Buffer::getShort(size_t index) const
{
    ASSERT_REMAINING(index, 2);
    return ((*this)[index + (order())] << 8) + (*this)[index + (1 - order())];
}

Buffer& Buffer::putInt(uint32_t data)
{
    putInt(position(), data);
    move(4);
    return *this;
}

Buffer& Buffer::putInt(size_t index, uint32_t data)
{
    ASSERT_REMAINING(index, 4);
    int32_t c = (order() ? 4 : -1), m = (order() * -2) + 1;
    (*this)[index + (c += m)] = (data >> 24) & 0xFF;
    (*this)[index + (c += m)] = (data >> 16) & 0xFF;
    (*this)[index + (c += m)] = (data >>  8) & 0xFF;
    (*this)[index + (c += m)] = (data      ) & 0xFF;
    return *this;
}

uint32_t Buffer::getInt()
{
    uint32_t ret = getInt(position());
    move(4);
    return ret;
}

uint32_t Buffer::getInt(size_t index) const
{
    ASSERT_REMAINING(index, 4);
    int32_t c = (order() ? 4 : -1), m = (order() * -2) + 1;
    uint32_t ret = static_cast<uint32_t>((*this)[index + (c += m)]) << 24;
    ret +=         static_cast<uint32_t>((*this)[index + (c += m)]) << 16;
    ret +=         static_cast<uint32_t>((*this)[index + (c += m)]) <<  8;
    ret +=         static_cast<uint32_t>((*this)[index + (c += m)]);
    return ret;
}

Buffer& Buffer::putLong(uint64_t data)
{
    putLong(position(), data);
    move(8);
    return *this;
}

Buffer& Buffer::putLong(size_t index, uint64_t data)
{
    ASSERT_REMAINING(index, 8);
    int32_t c = (order() ? 8 : -1), m = (order() * -2) + 1;
    (*this)[index + (c += m)] = (data >> 56) & 0xFF;
    (*this)[index + (c += m)] = (data >> 48) & 0xFF;
    (*this)[index + (c += m)] = (data >> 40) & 0xFF;
    (*this)[index + (c += m)] = (data >> 32) & 0xFF;
    (*this)[index + (c += m)] = (data >> 24) & 0xFF;
    (*this)[index + (c += m)] = (data >> 16) & 0xFF;
    (*this)[index + (c += m)] = (data >>  8) & 0xFF;
    (*this)[index + (c += m)] = (data      ) & 0xFF;
    return *this;
}

uint64_t Buffer::getLong()
{
    uint64_t ret = getLong(position());
    move(8);
    return ret;
}

uint64_t Buffer::getLong(size_t index) const
{
    ASSERT_REMAINING(index, 8);
    int32_t c = (order() ? 8 : -1), m = (order() * -2) + 1;
    uint64_t ret = static_cast<uint64_t>((*this)[index + (c += m)]) << 56;
    ret +=         static_cast<uint64_t>((*this)[index + (c += m)]) << 48;
    ret +=         static_cast<uint64_t>((*this)[index + (c += m)]) << 40;
    ret +=         static_cast<uint64_t>((*this)[index + (c += m)]) << 32;
    ret +=         static_cast<uint64_t>((*this)[index + (c += m)]) << 24;
    ret +=         static_cast<uint64_t>((*this)[index + (c += m)]) << 16;
    ret +=         static_cast<uint64_t>((*this)[index + (c += m)]) <<  8;
    ret +=         static_cast<uint64_t>((*this)[index + (c += m)]);
    return ret;
}

Buffer& Buffer::putFloat(float data)
{
    putFloat(position(), data);
    move(4);
    return *this;
}

Buffer& Buffer::putFloat(size_t index, float data)
{
    return putInt(index, reinterpret_cast<uint32_t&>(data));
}

float Buffer::getFloat()
{
    float ret = getFloat(position());
    move(4);
    return ret;
}

float Buffer::getFloat(size_t index) const
{
    uint32_t val = getInt(index);
    return reinterpret_cast<float&>(val);
}

Buffer& Buffer::putDouble(double data)
{
    putDouble(position(), data);
    move(8);
    return *this;
}

Buffer& Buffer::putDouble(size_t index, double data)
{
    return putLong(index, reinterpret_cast<uint64_t&>(data));
}

double Buffer::getDouble()
{
    double ret = getDouble(position());
    move(8);
    return ret;
}

double Buffer::getDouble(size_t index) const
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
        if ((*this)[position() + i] != other[other.position() + i])
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
        if ((*this)[position() + i] != other[other.position() + i])
        {
            return ((*this)[position() + i] > other[other.position() + i]) ? 1 : -1;
        }
    }
    return r0 > r1 ? 1 : r0 < r1 ? -1 : 0;
}

void Buffer::setSize(size_t size)
{
#ifdef CT_LIB_USE_ATOMIC_BUFFER_STATES
    this->size.store(size, std::memory_order_relaxed);
#else
    this->size = size;
#endif
}

size_t Buffer::getSize() const
{
#ifdef CT_LIB_USE_ATOMIC_BUFFER_STATES
    return size.load(std::memory_order_relaxed);
#else
    return size;
#endif;
}

void Buffer::offset(size_t off)
{
#ifdef CT_LIB_USE_ATOMIC_BUFFER_STATES
    this->off.store(off, std::memory_order_relaxed);
#else
    this->off = off;
#endif
}

size_t Buffer::offset() const
{
#ifdef CT_LIB_USE_ATOMIC_BUFFER_STATES
    return off.load(std::memory_order_relaxed);
#else
    return off;
#endif
}

void Buffer::assertValidPos(size_t pos) const
{
    if (pos > limit())
    {
        throw BufferError(BufferError::BUFFER_OVERFLOW);
    }
}

void Buffer::assertValidLimit(size_t limit) const
{
    if (limit > capacity())
    {
        throw BufferError(BufferError::INVALID_LIMIT);
    }
}

void Buffer::assertRemaining(size_t index, size_t count) const
{
    if ((index + count) > limit())
    {
        throw BufferError(BufferError::BUFFER_OVERFLOW);
    }
}

int Buffer::fullCompare(const Buffer& other) const
{
    if (this == &other)
    {
        return 0;
    }
    size_t tc = capacity(), oc = other.capacity();
    size_t c = tc > oc ? oc : tc;
    for (size_t i = 0; i < c; ++i)
    {
        if ((*this)[i] != other[i])
        {
            return ((*this)[i] > other[i]) ? 1 : -1;
        }
    }
    return tc > oc ? 1 : tc < oc ? -1 : 0;
}

void Buffer::move(size_t amount)
{
#ifdef CT_LIB_USE_ATOMIC_BUFFER_STATES
    pos.fetch_add(amount, std::memory_order_relaxed);
#else
    pos += amount;
#endif
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
