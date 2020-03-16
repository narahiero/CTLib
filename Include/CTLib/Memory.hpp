//////////////////////////////////////////////////
//  Copyright (c) 2020 Nara Hiero
//
// This file is licensed under GPLv3+
// Refer to the `License.txt` file included.
//////////////////////////////////////////////////

#pragma once

/*! @file Memory.hpp
 * 
 *  @brief The header containing all memory utilities for CT Lib.
 */


#include <cstdint>
#include <stdexcept>
#include <string>


namespace CTLib
{

const class BufferMemory;

/*! @brief A buffer similar to Java's `java.nio.ByteBuffer` class.
 * 
 *  The `CTLib::Buffer` class is heavily based on Java's `java.nio.ByteBuffer`,
 *  although there are some <a href="#__ctlib_buffer__diffs">differences</a>.
 *  
 *  <a name="__ctlib_buffer__diffs"></a>
 *  Here is a list of the main differences.<br>
 *  The `CTLib::Buffer` ...
 * 
 *  - ... is resizable, with the `resize()` and `truncate()` methods added.
 * 
 *  - ... has overloaded operators, with the indirection (`*`) and
 *  subscript (`[]`) operators added.
 * 
 *  - ... has no **put/get** methods for Java's `char` type, meaning the
 *  `putChar()` and `getChar()` methods are unavailable.
 * 
 *  - ... has no subtypes such as `java.nio.IntBuffer`, meaning the
 *  `asShortBuffer()`, `asCharBuffer()`, `asIntBuffer()`, `asFloatBuffer()`,
 *  `asLongBuffer()`, and `asDoubleBuffer()` methods are unavailable.
 * 
 *  - ... has no mark, meaning that the `mark()` and `reset()` methods are
 *  unavailable.
 * 
 *  - ... has no backing array, meaning that the `array()`, `arrayOffset()`,
 *  and `hasArray()` methods are unavailable.
 * 
 *  - ... is always 'direct', meaning that the `isDirect()` method is
 *  unavailable.
 * 
 *  - ... has no 'read-only' attribute, meaning that the `asReadOnlyBuffer()`
 *  and `isReadOnly()` methods are unavailable. (Just use `const` variables)
 */
const class Buffer
{

public:

    static constexpr bool BIG_ENDIAN = false;
    
    static constexpr bool LITTLE_ENDIAN = true;

    Buffer(size_t size);

    Buffer(const Buffer&);

    Buffer(Buffer&&) noexcept;

    virtual ~Buffer();

    Buffer& operator=(const Buffer&);

    Buffer& operator=(Buffer&&) noexcept;

    uint8_t* operator*() const noexcept;

    uint8_t operator[](size_t index) const;

    Buffer& resize(size_t newsize);

    Buffer& truncate();

    Buffer& order(bool order) noexcept;

    bool order() const noexcept;

    size_t capacity() const noexcept;

    Buffer& position(size_t pos);

    size_t position() const noexcept;

    Buffer& limit(size_t limit);

    size_t limit() const noexcept;

    size_t remaining() const noexcept;

    bool hasRemaining() const noexcept;

    Buffer& clear() noexcept;

    Buffer& flip() noexcept;

    Buffer& rewind() noexcept;

    Buffer& compact();

    Buffer& put(uint8_t data);

    Buffer& put(size_t index, uint8_t data);

    uint8_t get();

    uint8_t get(size_t index);

    Buffer& putShort(uint16_t data);

    Buffer& putShort(size_t index, uint16_t data);

    uint16_t getShort();

    uint16_t getShort(size_t index);

    Buffer& putInt(uint32_t data);

    Buffer& putInt(size_t index, uint32_t data);

    uint32_t getInt();

    uint32_t getInt(size_t index);

    Buffer& putLong(uint64_t data);

    Buffer& putLong(size_t index, uint64_t data);

    uint64_t getLong();

    uint64_t getLong(size_t index);

    Buffer& putFloat(float data);

    Buffer& putFloat(size_t index, float data);

    float getFloat();

    float getFloat(size_t index);

    Buffer& putDouble(double data);

    Buffer& putDouble(size_t index, double data);

    double getDouble();

    double getDouble(size_t index);

    bool equals(const Buffer&) const;

    int compareTo(const Buffer&) const;

private:

    Buffer();

    uint8_t* buffer;

    size_t size;

    size_t pos;

    size_t max;

    bool endian;

private:

    void assertValidSize(size_t size) const;

    void assertValidPos(size_t pos) const;

    void assertValidLimit(size_t limit) const;

    void assertRemaining(size_t index, size_t count) const;
};

const class BufferError : public std::runtime_error
{

public:

    static constexpr unsigned ALLOCATE_ZERO = 0;
    static constexpr unsigned BUFFER_OVERFLOW = 1;
    static constexpr unsigned INVALID_LIMIT = 2;

    BufferError(unsigned type, const std::string& msg = {});

    unsigned getType() const;

private:

    static constexpr unsigned MSG_NAME = 0;
    static constexpr unsigned MSG_DETAILS = 1;

    static constexpr const char* MESSAGES[][2]
    {
        {
            "ALLOCATE_ZERO",
            "Tried to allocate a buffer of size 0."
        },
        {
            "BUFFER_OVERFLOW",
            "Not enough bytes remaining in buffer."
        },
        {
            "INVALID_LIMIT",
            "Limit is greater than size."
        }
    };

    static std::string createMessage(unsigned, const std::string&);

    const unsigned type;
};
}
