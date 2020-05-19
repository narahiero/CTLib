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
#include <memory>


namespace CTLib
{

/*! @brief A buffer similar to Java's `java.nio.ByteBuffer` class.
 * 
 *  The `CTLib::Buffer` class is heavily based on Java's `java.nio.ByteBuffer`,
 *  although there are some <a href="#__ctlib_buffer__diffs">differences</a>.
 * 
 *  This class defines various categories of operations. The operations used to
 *  modify the buffer's data are listed below:
 * 
 *  - _Absolute_ and _relative_ **put/get** methods to read and write
 *  fundamental types.
 * 
 *  - _Absolute_ and _relative_ **bulk put** methods to write arrays and
 *  contents of buffer objects.
 * 
 *  - _Absolute_ and _relative_ **bulk get** methods to read into an array.
 * 
 *  - Subscript and indirection operators to directly access and modify the
 *  buffer's memory.
 * 
 *  The operations to change the buffer's state are:
 * 
 *  - **clear**, **flip**, and **rewind** to change the position and limit
 *  based on their previous value.
 * 
 *  - **position** and **limit** to directly set the position and limit.
 * 
 *  - **order** to change the endianness (or byte order).
 * 
 *  The operations to create other buffers with shared memory:
 * 
 *  - **duplicate** to create another buffer sharing this buffer's memory,
 *  but with its position, limit, and order independent.
 * 
 *  - **slice** to create another buffer with the same properties as described
 *  in **duplicate**, but having access to only a subset of this buffer's
 *  memory.
 * 
 *  Finally, the operations for comparing buffers are:
 * 
 *  - **compareTo** and **equals**, both comparing two buffers' remaining data,
 *  the former comparing it lexicographically, and the latter only checking for
 *  equality.
 * 
 *  - Equality and relational operators, to compare the data of two buffers.
 *  
 *  <a name="__ctlib_buffer__diffs"></a>
 *  Here is a list of the main differences between this buffer class and Java's
 *  `java.nio.ByteBuffer`.<br>
 *  The `CTLib::Buffer` ...
 * 
 *  - ... has constructors instead of static methods, meaning that the
 *  `allocate()` and `allocateDirect()` methods are unavailable.
 * 
 *  - ... has overloaded operators, with the subscript, indirection, equality,
 *  and relational operators added.
 * 
 *  - ... has no **put/get** methods for Java's `char` type, meaning the
 *  `putChar()` and `getChar()` methods are unavailable.
 * 
 *  - ... has no subtypes such as `java.nio.IntBuffer`, meaning the
 *  `asShortBuffer()`, `asCharBuffer()`, `asIntBuffer()`, `asFloatBuffer()`,
 *  `asLongBuffer()`, and `asDoubleBuffer()` methods are unavailable.
 * 
 *  - ... has different names for **put/get** methods for arrays, which are
 *  `putArray()` and `getArray()`.
 * 
 *  - ... has support for _absolute_ **put/get** for buffers and arrays.
 * 
 *  - ... has no `offset` parameter with **put/get** for arrays. (Just add the
 *  offset directly to the pointer)
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
 * 
 *  Additionally, there is no such class as `java.nio.ByteOrder`, so:
 * 
 *  - `order()` takes and returns a `bool` instead, with the compile-time
 *  constants `CTLib::Buffer::BIG_ENDIAN` and `CTLib::Buffer::LITTLE_ENDIAN`,
 *  there values being respectively `false` and `true`.
 * 
 *  - `CTLib::Buffer::nativeOrder()` replaces
 *  `java.nio.ByteOrder::nativeOrder()`.
 */
class Buffer final
{

public:

    /*! @brief Value used to represent big endian. */
    static constexpr bool BIG_ENDIAN = false;
    
    /*! @brief Value used to represent little endian. */
    static constexpr bool LITTLE_ENDIAN = true;

    /*! @brief Returns the endianness of the system.
     * 
     *  The returned value will either be `CTLib::Buffer::BIG_ENDIAN` (`false`)
     *  or `CTLib::Buffer::LITTLE_ENDIAN` (`true`).
     * 
     *  @return The system endianness
     */
    static bool nativeOrder();

    /*! @brief Constructs a buffer of size 0.
     *  
     *  No memory will be allocated for this buffer.
     */
    Buffer();

    /*! @brief Constructs a buffer of the specified size.
     *
     *  This constructor will construct a new buffer of the specified size. The
     *  memory will be allocated on the heap, and will fully be managed by this
     *  class.
     * 
     *  @param[in] size The size of the buffer to be created.
     */
    Buffer(size_t size);

    /*! @brief Constructs a fully independent copy of the specified buffer.
     *
     *  This constructor will construct a new buffer with its state and data
     *  an exact copy of the passed buffer object. This buffer's state and
     *  memory will be fully independent from the original buffer.
     * 
     *  @param[in] src The buffer to be copied
     */
    Buffer(const Buffer& src);

    /*! @brief Constructs a that will move the data from the specified buffer.
     *
     *  This constructor will construct a new buffer with its state and data
     *  moved from the passed buffer object. The original buffer will then be
     *  of size 0.
     * 
     *  @param[in] src The buffer to be moved from
     */
    Buffer(Buffer&& src) noexcept;

    ~Buffer();

    /*! @brief Copies the state and data of the specified buffer.
     *
     *  The state and data of this buffer will be fully independent of the
     *  original's.
     * 
     *  @param[in] src The buffer to be copied.
     * 
     *  @return This buffer
     */
    Buffer& operator=(const Buffer& src);

    /*! @brief Moves the state and data of the specified buffer to this buffer.
     *
     *  The state and data of the passed buffer will be moved into this buffer.
     *  The original buffer will then be of size 0.
     * 
     *  @param[in] src The buffer to be moved from.
     * 
     *  @return This buffer
     */
    Buffer& operator=(Buffer&& src) noexcept;

    /*! @brief Returns the raw pointer to the data of this buffer.
     *
     *  Note that the returned pointer **must NOT** be `delete`-ed.
     * 
     *  @return The pointer to this buffer's data
     */
    uint8_t* operator*() const noexcept;

    /*! @brief Returns a reference to the byte at the specified index.
     *  
     *  Note that no bounds checking is performed when this method is called.
     * 
     *  @param[in] index The byte index
     * 
     *  @return A reference to the byte at the specified index
     */
    uint8_t& operator[](size_t index) const;

    /*! @brief Compares this buffer with the specified one for equality.
     *
     *  Unlike the @link CTLib::Buffer::equals(const CTLib::Buffer&) const
     *  equals@endlink method, the entire data of both buffers will be compared.
     * 
     *  @param[in] other The buffer to be compared
     * 
     *  @return Whether this buffer equals `other`
     */
    bool operator==(const Buffer& other) const;

    /*! @brief Compares this buffer with the specified one for inequality.
     * 
     *  This method will simply negate the value returned by the equality (==)
     *  operator.
     * 
     *  @param[in] other The buffer to be compared
     * 
     *  @return Whether this buffer does not equal `other`
     */
    bool operator!=(const Buffer& other) const;

    /*! @brief Compares lexicographically this buffer to the specified one.
     * 
     *  @param[in] other The buffer to be compared
     * 
     *  @return Whether this buffer's data is less than other's
     */
    bool operator<(const Buffer& other) const;

    /*! @brief Compares lexicographically this buffer to the specified one.
     * 
     *  @param[in] other The buffer to be compared
     * 
     *  @return Whether this buffer's data is less than or equal to other's
     */
    bool operator<=(const Buffer& other) const;

    /*! @brief Compares lexicographically this buffer to the specified one.
     * 
     *  @param[in] other The buffer to be compared
     * 
     *  @return Whether this buffer's data is more than other's
     */
    bool operator>(const Buffer& other) const;

    /*! @brief Compares lexicographically this buffer to the specified one.
     * 
     *  @param[in] other The buffer to be compared
     * 
     *  @return Whether this buffer's data is more than or equal to other's
     */
    bool operator>=(const Buffer& other) const;
    
    /*! @brief Creates a new buffer object with its data shared with this
     *  buffer.
     * 
     *  The created buffer will be an exact copy of this buffer. The state of
     *  the new buffer will be fully independent of the original's.
     *  
     *  @return A newly created buffer sharing this buffer's memory
     */
    Buffer duplicate() const;

    /*! @brief Creates a new buffer object containing only a subset of this 
     *  buffer's memory.
     * 
     *  The created buffer will have its memory shared with this buffer's. The
     *  state of the new buffer will be fully independent of the original's.
     * 
     *  The new buffer's position will be zero and the limit will be the data
     *  remaining in this buffer.
     * 
     *  @return A newly created buffer sharing a subset of this buffer's memory
     */
    Buffer slice() const;

    /*! @brief Sets the endianness of this buffer.
     *
     *  @param[in] order The endianness. (CTLib::Buffer::BIG_ENDIAN or
     *  CTLib::Buffer::LITTLE_ENDIAN)
     * 
     *  @return This buffer
     */
    Buffer& order(bool order) noexcept;

    /*! @brief Returns the endianness of this buffer. */
    bool order() const noexcept;

    /*! @brief Returns the capacity of this buffer. */
    size_t capacity() const noexcept;

    /*! @brief Sets the position of this buffer.
     * 
     *  @throw CTLib::BufferError (CTLib::BufferError::BUFFER_OVERFLOW) If
     *  `pos` is more than the limit.
     * 
     *  @return This buffer
     */
    Buffer& position(size_t pos);

    /*! @brief Returns the position of this buffer. */
    size_t position() const noexcept;

    /*! @brief Sets the limit of this buffer.
     *
     *  @throw CTLib::BufferError (CTLib::BufferError::INVALID_LIMIT) If
     *  `limit` is more than the capacity.
     * 
     *  @return This buffer
     */
    Buffer& limit(size_t limit);

    /*! @brief Returns the limit of this buffer. */
    size_t limit() const noexcept;

    /*! @brief Returns the amount of bytes remaining in this buffer. */
    size_t remaining() const noexcept;

    /*! @brief Returns whether this buffer has any bytes remaining. */
    bool hasRemaining() const noexcept;

    /*! @brief Ready this buffer for write operations.
     * 
     *  The position is set to zero and the limit is set to the capacity.
     * 
     *  This method can be invoked before **put** operations, or before passing
     *  this buffer to the @link CTLib::Buffer::put(CTLib::Buffer&) put@endlink
     *  method of another buffer.
     * 
     *  ~~~{.cpp}
     *  buffer.clear();
     *  buffer.put(array, arraySize);
     *  ~~~
     * 
     *  This method does not actually erase the buffer's data, but was named as
     *  such since it is used before writing over the old data.
     * 
     *  @return This buffer
     */
    Buffer& clear() noexcept;

    /*! @brief Ready this buffer for read operations.
     *  
     *  The limit is set to the position and the position is then set to zero.
     * 
     *  This method can be invoked before **get** operations.
     * 
     *  ~~~{.cpp}
     *  buffer.flip();
     *  while (buffer.hasRemaining())
     *  {
     *      uint8_t data = buffer.get();
     *      // ...
     *  }
     *  ~~~
     * 
     *  @return This buffer
     */
    Buffer& flip() noexcept;

    /*! @brief Rewinds this buffer.
     * 
     *  The position is set to zero, but the limit is left unchanged.
     * 
     *  @return This buffer
     */
    Buffer& rewind() noexcept;

    /*! @brief Moves the remaining data to the beginning of this buffer.
     * 
     *  The data between the position and the limit is moved at the beginning
     *  of this buffer. The position is then set to the amount of bytes moved
     *  and the limit is set to the capacity.
     * 
     *  @return This buffer
     */
    Buffer& compact();

    /*! @brief Puts the specified byte at the current position.
     *  
     *  The specified byte is put at the current position and the position is
     *  then incremented by 1.
     * 
     *  @param[in] data A byte
     * 
     *  @throw CTLib::BufferError (CTLib::BufferError::BUFFER_OVERFLOW) If
     *  there is less than 1 byte remaining in this buffer.
     * 
     *  @return This buffer
     */
    Buffer& put(uint8_t data);

    /*! @brief Puts the specified byte at the specified index.
     *  
     *  The specified byte is put at the specified index.
     * 
     *  @param[in] index The index
     *  @param[in] data A byte
     * 
     *  @throw CTLib::BufferError (CTLib::BufferError::BUFFER_OVERFLOW) If
     *  there is less than 1 byte remaining in this buffer at the specified
     *  index.
     * 
     *  @return This buffer
     */
    Buffer& put(size_t index, uint8_t data);

    /*! @brief Gets the byte at the current position.
     *  
     *  The byte at the current position is returned and the position is then
     *  incremented by 1.
     * 
     *  @throw CTLib::BufferError (CTLib::BufferError::BUFFER_OVERFLOW) If
     *  there is less than 1 byte remaining in this buffer.
     * 
     *  @return The byte at the current position
     */
    uint8_t get();
    
    /*! @brief Gets the byte at the specified index.
     * 
     *  @param[in] index The index
     * 
     *  @throw CTLib::BufferError (CTLib::BufferError::BUFFER_OVERFLOW) If
     *  there is less than 1 byte remaining in this buffer at the specified
     *  index.
     * 
     *  @return The byte at the specified index
     */
    uint8_t get(size_t index) const;

    /*! @brief Puts the remaining data of the specified buffer at the current
     *  position.
     *  
     *  The remaining data of the specified buffer is put at the current
     *  position and the position is then incremented by the amount of bytes
     *  written.
     * 
     *  @param[in] data A buffer
     * 
     *  @throw CTLib::BufferError (CTLib::BufferError::BUFFER_OVERFLOW) If
     *  there are less bytes remaining in this buffer than in the specified
     *  buffer.
     * 
     *  @return This buffer
     */
    Buffer& put(Buffer& data);

    /*! @brief Puts the remaining data of the specified buffer at the specified
     *  index. 
     * 
     *  The remaining data of the specified buffer is put at the specified
     *  index.
     * 
     *  @param[in] index The index
     *  @param[in] data A buffer
     * 
     *  @throw CTLib::BufferError (CTLib::BufferError::BUFFER_OVERFLOW) If
     *  there are less bytes remaining in this buffer at the specified index
     *  than in the specified buffer.
     * 
     *  @return This buffer
     */
    Buffer& put(size_t index, Buffer& data);

    /*! @brief Puts the data in the specified array at the current position.
     * 
     *  The data in the specified array is put at the current position and the
     *  position is then incremented by the size of the array.
     * 
     *  @param[in] data An array
     *  @param[in] size The size of the array
     * 
     *  @throw CTLib::BufferError (CTLib::BufferError::BUFFER_OVERFLOW) If
     *  there are less bytes than `size` remaining in this buffer.
     * 
     *  @return This buffer
     */
    Buffer& putArray(uint8_t* data, size_t size);

    /*! @brief Puts the data in the specified array at the current position.
     * 
     *  The data in the specified array is put at the specified index.
     * 
     *  @param[in] index The index
     *  @param[in] data An array
     *  @param[in] size The size of the array
     * 
     *  @throw CTLib::BufferError (CTLib::BufferError::BUFFER_OVERFLOW) If
     *  there are less bytes than `size` remaining in this buffer at the
     *  specified index.
     * 
     *  @return This buffer
     */
    Buffer& putArray(size_t index, uint8_t* data, size_t size);

    /*! @brief Gets `size` bytes at the current position and writes them to the
     *  specified array.
     * 
     *  The position is then incremented by the size of the array.
     * 
     *  @param[in] out The output array
     *  @param[in] size The size of the array
     * 
     *  @throw CTLib::BufferError (CTLib::BufferError::BUFFER_OVERFLOW) If
     *  there are less bytes than `size` remaining in this buffer.
     * 
     *  @return This buffer
     */
    Buffer& getArray(uint8_t* out, size_t size);

    /*! @brief Gets `size` bytes at the specified index and writes them to the
     *  specified array.
     * 
     *  @param[in] index The index
     *  @param[in] out The output array
     *  @param[in] size The size of the array
     * 
     *  @throw CTLib::BufferError (CTLib::BufferError::BUFFER_OVERFLOW) If
     *  there are less bytes than `size` remaining in this buffer at the
     *  specified index.
     * 
     *  @return This buffer
     */
    Buffer& getArray(size_t index, uint8_t* out, size_t size);

    /*! @brief Puts the specified short at the current position.
     *  
     *  The specified short is put at the current position and the position is
     *  then incremented by 2.
     * 
     *  @param[in] data A short
     * 
     *  @throw CTLib::BufferError (CTLib::BufferError::BUFFER_OVERFLOW) If
     *  there is less than 2 bytes remaining in this buffer.
     * 
     *  @return This buffer
     */
    Buffer& putShort(uint16_t data);

    /*! @brief Puts the specified short at the specified index.
     *  
     *  The specified short is put at the specified index.
     * 
     *  @param[in] index The index
     *  @param[in] data A short
     * 
     *  @throw CTLib::BufferError (CTLib::BufferError::BUFFER_OVERFLOW) If
     *  there is less than 2 bytes remaining in this buffer at the specified
     *  index.
     * 
     *  @return This buffer
     */
    Buffer& putShort(size_t index, uint16_t data);

    /*! @brief Gets the short at the current position.
     *  
     *  The short at the current position is returned and the position is then
     *  incremented by 2.
     * 
     *  @throw CTLib::BufferError (CTLib::BufferError::BUFFER_OVERFLOW) If
     *  there is less than 2 bytes remaining in this buffer.
     * 
     *  @return The short at the current position
     */
    uint16_t getShort();
    
    /*! @brief Gets the short at the specified index.
     * 
     *  @param[in] index The index
     * 
     *  @throw CTLib::BufferError (CTLib::BufferError::BUFFER_OVERFLOW) If
     *  there is less than 2 bytes remaining in this buffer at the specified
     *  index.
     * 
     *  @return The short at the specified index
     */
    uint16_t getShort(size_t index) const;

    /*! @brief Puts the specified integer at the current position.
     *  
     *  The specified integer is put at the current position and the position
     *  is then incremented by 4.
     * 
     *  @param[in] data An integer
     * 
     *  @throw CTLib::BufferError (CTLib::BufferError::BUFFER_OVERFLOW) If
     *  there is less than 4 bytes remaining in this buffer.
     * 
     *  @return This buffer
     */
    Buffer& putInt(uint32_t data);

    /*! @brief Puts the specified integer at the specified index.
     *  
     *  The specified integer is put at the specified index.
     * 
     *  @param[in] index The index
     *  @param[in] data An integer
     * 
     *  @throw CTLib::BufferError (CTLib::BufferError::BUFFER_OVERFLOW) If
     *  there is less than 4 bytes remaining in this buffer at the specified
     *  index.
     * 
     *  @return This buffer
     */
    Buffer& putInt(size_t index, uint32_t data);

    /*! @brief Gets the integer at the current position.
     *  
     *  The integer at the current position is returned and the position is then
     *  incremented by 4.
     * 
     *  @throw CTLib::BufferError (CTLib::BufferError::BUFFER_OVERFLOW) If
     *  there is less than 4 bytes remaining in this buffer.
     * 
     *  @return The integer at the current position
     */
    uint32_t getInt();
    
    /*! @brief Gets the integer at the specified index.
     * 
     *  @param[in] index The index
     * 
     *  @throw CTLib::BufferError (CTLib::BufferError::BUFFER_OVERFLOW) If
     *  there is less than 4 bytes remaining in this buffer at the specified
     *  index.
     * 
     *  @return The integer at the specified index
     */
    uint32_t getInt(size_t index) const;

    /*! @brief Puts the specified long at the current position.
     *  
     *  The specified long is put at the current position and the position is
     *  then incremented by 8.
     * 
     *  @param[in] data A long
     * 
     *  @throw CTLib::BufferError (CTLib::BufferError::BUFFER_OVERFLOW) If
     *  there is less than 8 bytes remaining in this buffer.
     * 
     *  @return This buffer
     */
    Buffer& putLong(uint64_t data);

    /*! @brief Puts the specified long at the specified index.
     *  
     *  The specified long is put at the specified index.
     * 
     *  @param[in] index The index
     *  @param[in] data A long
     * 
     *  @throw CTLib::BufferError (CTLib::BufferError::BUFFER_OVERFLOW) If
     *  there is less than 8 bytes remaining in this buffer at the specified
     *  index.
     * 
     *  @return This buffer
     */
    Buffer& putLong(size_t index, uint64_t data);

    /*! @brief Gets the long at the current position.
     *  
     *  The long at the current position is returned and the position is then
     *  incremented by 8.
     * 
     *  @throw CTLib::BufferError (CTLib::BufferError::BUFFER_OVERFLOW) If
     *  there is less than 8 bytes remaining in this buffer.
     * 
     *  @return The long at the current position
     */
    uint64_t getLong();
    
    /*! @brief Gets the long at the specified index.
     * 
     *  @param[in] index The index
     * 
     *  @throw CTLib::BufferError (CTLib::BufferError::BUFFER_OVERFLOW) If
     *  there is less than 8 bytes remaining in this buffer at the specified
     *  index.
     * 
     *  @return The long at the specified index
     */
    uint64_t getLong(size_t index) const;

    /*! @brief Puts the specified float at the current position.
     *  
     *  The specified float is put at the current position and the position is
     *  then incremented by 4.
     * 
     *  **IMPORTANT**: This method assumes the hardware float is formatted as
     *  IEEE 754 single-precision floating-point.
     * 
     *  @param[in] data A float
     * 
     *  @throw CTLib::BufferError (CTLib::BufferError::BUFFER_OVERFLOW) If
     *  there is less than 4 bytes remaining in this buffer.
     * 
     *  @return This buffer
     */
    Buffer& putFloat(float data);

    /*! @brief Puts the specified float at the specified index.
     *  
     *  The specified float is put at the specified index.
     * 
     *  **IMPORTANT**: This method assumes the hardware float is formatted as
     *  IEEE 754 single-precision floating-point.
     * 
     *  @param[in] index The index
     *  @param[in] data A float
     * 
     *  @throw CTLib::BufferError (CTLib::BufferError::BUFFER_OVERFLOW) If
     *  there is less than 4 bytes remaining in this buffer at the specified
     *  index.
     * 
     *  @return This buffer
     */
    Buffer& putFloat(size_t index, float data);

    /*! @brief Gets the float at the current position.
     *  
     *  The float at the current position is returned and the position is then
     *  incremented by 4.
     * 
     *  **IMPORTANT**: This method assumes the hardware float is formatted as
     *  IEEE 754 single-precision floating-point.
     * 
     *  @throw CTLib::BufferError (CTLib::BufferError::BUFFER_OVERFLOW) If
     *  there is less than 4 bytes remaining in this buffer.
     * 
     *  @return The float at the current position
     */
    float getFloat();
    
    /*! @brief Gets the float at the specified index.
     * 
     *  **IMPORTANT**: This method assumes the hardware float is formatted as
     *  IEEE 754 single-precision floating-point.
     * 
     *  @param[in] index The index
     * 
     *  @throw CTLib::BufferError (CTLib::BufferError::BUFFER_OVERFLOW) If
     *  there is less than 4 bytes remaining in this buffer at the specified
     *  index.
     * 
     *  @return The float at the specified index
     */
    float getFloat(size_t index) const;

    /*! @brief Puts the specified double at the current position.
     *  
     *  The specified byte is put at the current position and the position is
     *  then incremented by 8.
     * 
     *  **IMPORTANT**: This method assumes the hardware double is formatted as
     *  IEEE 754 double-precision floating-point.
     * 
     *  @param[in] data A double
     * 
     *  @throw CTLib::BufferError (CTLib::BufferError::BUFFER_OVERFLOW) If
     *  there is less than 8 bytes remaining in this buffer.
     * 
     *  @return This buffer
     */
    Buffer& putDouble(double data);

    /*! @brief Puts the specified double at the specified index.
     *  
     *  The specified double is put at the specified index.
     * 
     *  **IMPORTANT**: This method assumes the hardware double is formatted as
     *  IEEE 754 double-precision floating-point.
     * 
     *  @param[in] index The index
     *  @param[in] data A double
     * 
     *  @throw CTLib::BufferError (CTLib::BufferError::BUFFER_OVERFLOW) If
     *  there is less than 8 bytes remaining in this buffer at the specified
     *  index.
     * 
     *  @return This buffer
     */
    Buffer& putDouble(size_t index, double data);

    /*! @brief Gets the double at the current position.
     *  
     *  The double at the current position is returned and the position is then
     *  incremented by 8.
     * 
     *  **IMPORTANT**: This method assumes the hardware double is formatted as
     *  IEEE 754 double-precision floating-point.
     * 
     *  @throw CTLib::BufferError (CTLib::BufferError::BUFFER_OVERFLOW) If
     *  there is less than 8 bytes remaining in this buffer.
     * 
     *  @return The double at the current position
     */
    double getDouble();
    
    /*! @brief Gets the double at the specified index.
     * 
     *  **IMPORTANT**: This method assumes the hardware double is formatted as
     *  IEEE 754 double-precision floating-point.
     * 
     *  @param[in] index The index
     * 
     *  @throw CTLib::BufferError (CTLib::BufferError::BUFFER_OVERFLOW) If
     *  there is less than 8 bytes remaining in this buffer at the specified
     *  index.
     * 
     *  @return The double at the specified index
     */
    double getDouble(size_t index) const;

    /*! @brief Check the remaining data of this buffer and the specified one
     *  for equality.
     * 
     *  @param[in] other The buffer to be compared.
     * 
     *  @return Whether this buffer's remaining data matches `other`'s
     */
    bool equals(const Buffer& other) const;

    /*! @brief Compares the remaining data of this buffer and the specified
     *  one.
     * 
     *  The remaining data of the two buffers are compared lexicographically.
     * 
     *  @param[in] other The buffer to be compared.
     * 
     *  @return -1 if this buffer's remaining data is less than `other`'s,<br>
     *          0 if this buffer's remaining data is equal to `other`'s,<br>
     *          1 if this buffer's remaining data is more than `other`'s
     */
    int compareTo(const Buffer& other) const;

private:

    // constructor used for duplicate() and slice()
    Buffer(const Buffer*, size_t off);

    // throws BUFFER_OVERFLOW if pos is more than the limit
    void assertValidPos(size_t pos) const;

    // throws INVALID_LIMIT if limit is more than the capacity
    void assertValidLimit(size_t limit) const;

    // throws BUFFER_OVERFLOW if there is less than count remaining at index
    void assertRemaining(size_t index, size_t count) const;

    // used in the relational operators
    int fullCompare(const Buffer& other) const;

    // the data of this buffer
    std::shared_ptr<uint8_t[]> buffer;

    // the size of this buffer
    size_t size;

    // the offset in the data of this buffer
    size_t off;

    // the current position of this buffer
    size_t pos;

    // the current limit of this buffer
    size_t max;

    // the current endianness of this buffer
    bool endian;
};

/*! The error class used by the CTLib::Buffer class. */
class BufferError final : public std::runtime_error
{

public:

    /*! @brief Thrown when a **put/get** operation would surpass the limit. */
    static constexpr unsigned BUFFER_OVERFLOW = 0;

    /*! @brief Thrown when a limit larger than the capacity is set. */
    static constexpr unsigned INVALID_LIMIT = 1;

    /*! @brief Constructs a buffer error of the specified type with an optional
     *  error message.
     *
     *  @param[in] type The error type.
     *  @param[in] msg The optional error message.
     */
    BufferError(unsigned type, const std::string& msg = {});

    /*! @brief Returns the type of the error. */
    unsigned getType() const;

private:

    // index for name of constant
    static constexpr unsigned MSG_NAME = 0;

    // index for default message
    static constexpr unsigned MSG_DETAILS = 1;

    // contains message info to construct error messages
    static constexpr const char* MESSAGES[][2]
    {
        {
            "BUFFER_OVERFLOW",
            "Not enough bytes remaining in buffer."
        },
        {
            "INVALID_LIMIT",
            "Limit is greater than size."
        }
    };

    // constructs an error message using the specified type
    static std::string createMessage(unsigned type, const std::string&);

    // the error type
    const unsigned type;
};
}
