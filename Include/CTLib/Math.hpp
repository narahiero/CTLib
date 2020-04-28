//////////////////////////////////////////////////
//  Copyright (c) 2020 Nara Hiero
//
// This file is licensed under GPLv3+
// Refer to the `License.txt` file included.
//////////////////////////////////////////////////

#pragma once

/*! @file Math.hpp
 * 
 *  @brief The header containing all math utilities for CT Lib.
 */


#include <cstdint>
#include <type_traits>

#include <CTLib/Memory.hpp>


namespace CTLib
{

/*! @brief A fixed-size mathematical vector of the specified type.
 *  
 *  The entire vector lives where it is declared, so it is not recommended to
 *  use as local/member variables with large Type and/or Size.
 * 
 *  Type must be default constructible. Size must be more than 0.
 * 
 *  @tparam Type The element type
 *  @tparam Size The element count
 */
template <class Type, uint32_t Size, typename = typename std::enable_if_t<
    std::is_default_constructible_v<Type> && (Size > 0)
>>
class Vector
{

public:

    /*! @brief Constructs a vector instance with all of its components
     *  initialized to a default constructed Type instance.
     */
    Vector() noexcept
    {
        setDefault();
    }

    /*! @brief Constructs a vector instance with its components set to the
     *  specified vector's.
     */
    Vector(const Vector& src) noexcept
    {
        for (uint32_t i = 0; i < Size; ++i)
        {
            array[i] = src.array[i];
        }
    }

    virtual ~Vector() = default;

    /*! @brief Sets the components of this vector to the specified vector's. */
    Vector& operator=(const Vector& src) noexcept
    {
        for (uint32_t i = 0; i < Size; ++i)
        {
            array[i] = src.array[i];
        }
        return *this;
    }

    /*! @brief Returns a pointer to the internal array. */
    inline Type* operator*() noexcept
    {
        return array;
    }

    /*! @brief Returns a pointer to the internal array. */
    inline const Type* operator*() const noexcept
    {
        return array;
    }

    /*! @brief Returns a reference to the component at the specified index. */
    inline Type& operator[](uint32_t index) noexcept
    {
        return array[index];
    }

    /*! @brief Returns a reference to the component at the specified index. */
    const inline Type& operator[](uint32_t index) const noexcept
    {
        return array[index];
    }

    /*! @brief Returns the component count of this vector. */
    constexpr inline uint32_t size() const noexcept
    {
        return Size;
    }

    /*! @brief Sets all components to a default constructed value. */
    void setDefault() noexcept
    {
        for (uint32_t i = 0; i < Size; ++i)
        {
            array[i] = Type();
        }
    }

protected:

    //! array containing the elements of this vector
    Type array[Size];
};

/*! @brief Returns whether the specified vectors are equal component-wise. */
template <class Type, uint32_t Size>
bool operator==(const Vector<Type, Size>& lhs, const Vector<Type, Size>& rhs)
{
    if (*lhs == *rhs)
    {
        return true;
    }
    for (uint32_t i = 0; i < Size; ++i)
    {
        if (!(lhs[i] == rhs[i]))
        {
            return false;
        }
    }
    return true;
}

/*! @brief Returns whether the specified vectors are not equal component-wise. */
template <class Type, uint32_t Size>
bool operator!=(const Vector<Type, Size>& lhs, const Vector<Type, Size>& rhs)
{
    return !(lhs == rhs);
}

template <uint32_t Size>
class Vectorf : public Vector<float, Size>
{

public:

    Vectorf() : Vector()
    {

    }

    ~Vectorf() = default;

    /*! @brief Puts `Size` floats on the specified buffer.
     *  
     *  @param[in] buffer The buffer to write to
     * 
     *  @throw CTLib::BufferError If there is less than `Size * 4` bytes
     *  remaining in the output buffer.
     * 
     *  @return `this`
     */
    Vectorf& put(Buffer& buffer)
    {
        for (uint32_t i = 0; i < Size; ++i)
        {
            buffer.putFloat(array[i]);
        }
        return *this;
    }

    /*! @brief Gets `Size` floats from the specified buffer, and set the
     *  components of this vector to the read values.
     * 
     *  @param[in] buffer The buffer to read from
     * 
     *  @throw CTLib::BufferError If there is less than `Size * 4` bytes
     *  remaining in the input buffer.
     * 
     *  @return `this`
     */
    Vectorf& get(Buffer& buffer)
    {
        for (uint32_t i = 0; i < Size; ++i)
        {
            array[i] = buffer.getFloat();
        }
        return *this;
    }
};

class Vector2f final : public Vectorf<2>
{

public:

    Vector2f();

    Vector2f(float x, float y);

    ~Vector2f();
};

class Vector3f final : public Vectorf<3>
{

public:

    Vector3f();

    Vector3f(float x, float y, float z);

    ~Vector3f();
};
}
