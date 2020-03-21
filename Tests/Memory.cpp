//////////////////////////////////////////////////
//  Copyright (c) 2020 Nara Hiero
//
// This file is licensed under GPLv3+
// Refer to the `License.txt` file included.
//////////////////////////////////////////////////

#include <gtest/gtest.h>

#include <CTLib/Memory.hpp>

TEST(BufferTests, Size)
{
    CTLib::Buffer buffer(16);
    EXPECT_EQ(16, buffer.capacity());

    CTLib::Buffer buffer2(64);
    EXPECT_EQ(64, buffer2.capacity());
}

TEST(BufferTests, CopyCtor)
{
    CTLib::Buffer buffer(4);
    buffer.position(2).limit(3);

    CTLib::Buffer copy{buffer};
    EXPECT_EQ(buffer.capacity(), copy.capacity());
    EXPECT_EQ(buffer.position(), copy.position());
    EXPECT_EQ(buffer.limit(), copy.limit());
}

TEST(BufferTests, MoveCtor)
{
    CTLib::Buffer buffer(4);
    buffer.position(2).limit(3);
    uint8_t* pointer = *buffer;

    CTLib::Buffer move{std::move(buffer)};
    EXPECT_EQ(4, move.capacity());
    EXPECT_EQ(2, move.position());
    EXPECT_EQ(3, move.limit());
    EXPECT_EQ(pointer, *move);
    EXPECT_EQ(nullptr, *buffer);
}

TEST(BufferTests, ZeroSizedAlloc)
{
    EXPECT_THROW({
        try
        {
            CTLib::Buffer(0);
        }
        catch (const CTLib::BufferError& e)
        {
            EXPECT_EQ(CTLib::BufferError::ALLOCATE_ZERO, e.getType());
            throw;
        }
    }, CTLib::BufferError);
}

TEST(BufferTests, PositionLimitAndRemaining)
{
    CTLib::Buffer buffer(8);
    EXPECT_EQ(0, buffer.position());
    EXPECT_EQ(8, buffer.limit());
    EXPECT_EQ(8, buffer.remaining());
    EXPECT_TRUE(buffer.hasRemaining());

    buffer.position(3);
    EXPECT_EQ(3, buffer.position());
    EXPECT_EQ(8, buffer.limit());
    EXPECT_EQ(5, buffer.remaining());
    EXPECT_TRUE(buffer.hasRemaining());

    buffer.position(8);
    EXPECT_EQ(8, buffer.position());
    EXPECT_EQ(8, buffer.limit());
    EXPECT_EQ(0, buffer.remaining());
    EXPECT_FALSE(buffer.hasRemaining());

    buffer.position(2).limit(5);
    EXPECT_EQ(2, buffer.position());
    EXPECT_EQ(5, buffer.limit());
    EXPECT_EQ(3, buffer.remaining());
}

TEST(BufferTests, FlipClearAndRewind)
{
    CTLib::Buffer buffer(8);
    buffer.position(6).flip();
    EXPECT_EQ(0, buffer.position());
    EXPECT_EQ(6, buffer.limit());
    
    buffer.position(4).clear();
    EXPECT_EQ(0, buffer.position());
    EXPECT_EQ(8, buffer.limit());

    buffer.position(5).limit(7).rewind();
    EXPECT_EQ(0, buffer.position());
    EXPECT_EQ(7, buffer.limit());
}

TEST(BufferTests, PutGet)
{
    CTLib::Buffer buffer(4);
    buffer.put(1, 0xAA).put(2, 0x10).put(0, 0x42);
    EXPECT_EQ(0, buffer.position());

    EXPECT_EQ(0xAA, buffer.get(1));
    EXPECT_EQ(0x42, buffer.get(0));
    EXPECT_EQ(0x10, buffer.get(2));
    EXPECT_EQ(0, buffer.position());
}

TEST(BufferTests, PutGetShort)
{
    CTLib::Buffer buffer(8);
    buffer.putShort(4, 0x1234).putShort(0, 0xABCD).putShort(2, 0x1000);
    EXPECT_EQ(0, buffer.position());

    EXPECT_EQ(0xABCD, buffer.getShort(0));
    EXPECT_EQ(0x1234, buffer.getShort(4));
    EXPECT_EQ(0x1000, buffer.getShort(2));
    EXPECT_EQ(0, buffer.position());
}

TEST(BufferTests, PutGetIntAndLong)
{
    CTLib::Buffer buffer(16);
    buffer.putInt(0, 0x12345678);
    buffer.putInt(12, 0xABCDEF00);
    buffer.putInt(4, 0x10000000);
    buffer.putInt(8, 0x64321608);
    EXPECT_EQ(0, buffer.position());

    EXPECT_EQ(0xABCDEF00, buffer.getInt(12));
    EXPECT_EQ(0x64321608, buffer.getInt(8));
    EXPECT_EQ(0x12345678, buffer.getInt(0));
    EXPECT_EQ(0x10000000, buffer.getInt(4));
    EXPECT_EQ(0, buffer.position());

    buffer.putLong(8, 0x0123456789ABCDEF);
    buffer.putLong(0, 0x1000000000000000);
    EXPECT_EQ(0, buffer.position());

    EXPECT_EQ(0x1000000000000000, buffer.getLong(0));
    EXPECT_EQ(0x0123456789ABCDEF, buffer.getLong(8));
    EXPECT_EQ(0, buffer.position());
}

TEST(BufferTests, PutGetFloatAndDouble)
{
    CTLib::Buffer buffer(16);
    buffer.putFloat(8, 5.f);
    buffer.putFloat(12, 123.45f);
    buffer.putFloat(0, -10.f);
    buffer.putFloat(4, 0.07f);
    EXPECT_EQ(0, buffer.position());

    EXPECT_EQ(123.45f, buffer.getFloat(12));
    EXPECT_EQ(-10.f, buffer.getFloat(0));
    EXPECT_EQ(5.f, buffer.getFloat(8));
    EXPECT_EQ(0.07f, buffer.getFloat(4));
    EXPECT_EQ(0, buffer.position());

    buffer.putDouble(8, 17329.423);
    buffer.putDouble(0, -0.00032146);
    EXPECT_EQ(0, buffer.position());

    EXPECT_EQ(-0.00032146, buffer.getDouble(0));
    EXPECT_EQ(17329.423, buffer.getDouble(8));
    EXPECT_EQ(0, buffer.position());
}

TEST(BufferTests, RelativePutGet)
{
    CTLib::Buffer buffer(4);
    buffer.put(0x64).put(0xAB);
    EXPECT_EQ(2, buffer.position());

    buffer.put(0x10);
    EXPECT_EQ(3, buffer.position());

    buffer.flip();
    EXPECT_EQ(0x64, buffer.get());
    EXPECT_EQ(0xAB, buffer.get());
    EXPECT_EQ(0x10, buffer.get());
}

TEST(BufferTests, RelativePutGetShort)
{
    CTLib::Buffer buffer(8);
    buffer.putShort(0x1234);
    buffer.putShort(0xABCD);
    buffer.putShort(0x1000);
    EXPECT_EQ(6, buffer.position());

    buffer.flip();
    EXPECT_EQ(0x1234, buffer.getShort());
    EXPECT_EQ(0xABCD, buffer.getShort());
    EXPECT_EQ(0x1000, buffer.getShort());
}

TEST(BufferTests, RelativePutGetIntAndLong)
{
    CTLib::Buffer buffer(16);
    buffer.putInt(0x12345678);
    buffer.putInt(0xABCDEF00);
    buffer.putInt(0x10000000);
    buffer.putInt(0x64321608);
    EXPECT_EQ(16, buffer.position());

    buffer.flip();
    EXPECT_EQ(0x12345678, buffer.getInt());
    EXPECT_EQ(0xABCDEF00, buffer.getInt());
    EXPECT_EQ(0x10000000, buffer.getInt());
    EXPECT_EQ(0x64321608, buffer.getInt());

    buffer.clear();
    buffer.putLong(0x0123456789ABCDEF);
    buffer.putLong(0x1000000000000000);
    EXPECT_EQ(16, buffer.position());

    buffer.flip();
    EXPECT_EQ(0x0123456789ABCDEF, buffer.getLong());
    EXPECT_EQ(0x1000000000000000, buffer.getLong());
}

TEST(BufferTests, RelativePutGetFloatAndDouble)
{
    CTLib::Buffer buffer(16);
    buffer.putFloat(5.f);
    buffer.putFloat(123.45f);
    buffer.putFloat(-10.f);
    buffer.putFloat(0.07f);
    EXPECT_EQ(16, buffer.position());

    buffer.flip();
    EXPECT_EQ(5.f, buffer.getFloat());
    EXPECT_EQ(123.45f, buffer.getFloat());
    EXPECT_EQ(-10.f, buffer.getFloat());
    EXPECT_EQ(0.07f, buffer.getFloat());
    EXPECT_EQ(16, buffer.position());

    buffer.clear();
    buffer.putDouble(17329.423);
    buffer.putDouble(-0.00032146);
    EXPECT_EQ(16, buffer.position());

    buffer.flip();
    EXPECT_EQ(17329.423, buffer.getDouble());
    EXPECT_EQ(-0.00032146, buffer.getDouble());
    EXPECT_EQ(16, buffer.position());
}

TEST(BufferTests, PutBuffer)
{
    CTLib::Buffer data(8);
    data.put(0x10).put(0x11).put(0x12).put(0x13);
    data.flip();

    CTLib::Buffer buffer(8);
    buffer.put(0xFF).put(data).put(0xAA);
    EXPECT_EQ(4, data.position());
    EXPECT_EQ(6, buffer.position());

    buffer.flip();
    EXPECT_EQ(0xFF, buffer.get());
    EXPECT_EQ(0x10, buffer.get());
    EXPECT_EQ(0x11, buffer.get());
    EXPECT_EQ(0x12, buffer.get());
    EXPECT_EQ(0x13, buffer.get());
    EXPECT_EQ(0xAA, buffer.get());

    data.clear();
    data.put(0xA0).put(0xB0).put(0xC0).put(0xD0).put(0xE0).put(0xF0);
    data.position(2).limit(5);

    buffer.clear();
    buffer.put(0x64).put(data).put(0xFF).put(0x32).put(0x10);
    EXPECT_EQ(5, data.position());
    EXPECT_EQ(7, buffer.position());

    buffer.flip();
    EXPECT_EQ(0x64, buffer.get());
    EXPECT_EQ(0xC0, buffer.get());
    EXPECT_EQ(0xD0, buffer.get());
    EXPECT_EQ(0xE0, buffer.get());
    EXPECT_EQ(0xFF, buffer.get());
    EXPECT_EQ(0x32, buffer.get());
    EXPECT_EQ(0x10, buffer.get());
}

TEST(BufferTests, PutArray)
{
    uint8_t arr[]{0x57, 0x43, 0x49, 0x19, 0xF4, 0x3B};
    CTLib::Buffer buffer(16);
    buffer.put(0x10).putArray(arr, 6).put(0x2D);
    EXPECT_EQ(8, buffer.position());

    buffer.flip();
    EXPECT_EQ(0x10, buffer.get());
    EXPECT_EQ(0x57, buffer.get());
    EXPECT_EQ(0x43, buffer.get());
    EXPECT_EQ(0x49, buffer.get());
    EXPECT_EQ(0x19, buffer.get());
    EXPECT_EQ(0xF4, buffer.get());
    EXPECT_EQ(0x3B, buffer.get());
    EXPECT_EQ(0x2D, buffer.get());

    buffer.clear();
    uint8_t arr2[]{0x21, 0xFF, 0x03, 0x11};
    buffer.put(0xA7).put(0x04).putArray(arr2 + 1, 3).put(0x43);
    EXPECT_EQ(6, buffer.position());
    
    buffer.flip();
    EXPECT_EQ(0xA7, buffer.get());
    EXPECT_EQ(0x04, buffer.get());
    EXPECT_EQ(0xFF, buffer.get());
    EXPECT_EQ(0x03, buffer.get());
    EXPECT_EQ(0x11, buffer.get());
    EXPECT_EQ(0x43, buffer.get());
}

TEST(BufferTests, GetArray)
{
    CTLib::Buffer buffer(8);
    buffer.put(0x84).put(0x21).put(0x43).put(0xFF).put(0x02);
    
    uint8_t out[4];
    buffer.flip().getArray(out, 4);
    EXPECT_EQ(4, buffer.position());
    EXPECT_EQ(0x84, out[0]);
    EXPECT_EQ(0x21, out[1]);
    EXPECT_EQ(0x43, out[2]);
    EXPECT_EQ(0xFF, out[3]);

    buffer.clear();
    buffer.put(0x98).put(0xDF).put(0x0B).put(0x7E).put(0x43).put(0x6F);
    
    uint8_t out2[8];
    buffer.flip().getArray(out2 + 2, 5);
    EXPECT_EQ(5, buffer.position());
    EXPECT_EQ(0x98, out2[2]);
    EXPECT_EQ(0xDF, out2[3]);
    EXPECT_EQ(0x0B, out2[4]);
    EXPECT_EQ(0x7E, out2[5]);
    EXPECT_EQ(0x43, out2[6]);
}

TEST(BufferTests, Compact)
{
    CTLib::Buffer buffer(8);
    buffer.put(0x10).put(0x22).put(0x34).put(0x46).put(0x58);
    buffer.flip().position(2).compact();
    EXPECT_EQ(3, buffer.position());
    EXPECT_EQ(8, buffer.limit());
    EXPECT_EQ(0x34, buffer.get(0));
    EXPECT_EQ(0x46, buffer.get(1));
    EXPECT_EQ(0x58, buffer.get(2));
}

TEST(BufferTests, IndirectionAndSubscriptOperators)
{
    CTLib::Buffer buffer(8);
    buffer.putLong(0xFEDCBA9876543210);

    uint8_t* ptr = *buffer;
    for (size_t i = 0, v = 0xFE; i < 8; ++i, v -= 0x22)
    {
        EXPECT_EQ(v, ptr[i]);
        EXPECT_EQ(v, buffer[i]);
    }
}

TEST(BufferTests, Order)
{
    CTLib::Buffer buffer(8);
    EXPECT_EQ(CTLib::Buffer::BIG_ENDIAN, buffer.order());

    buffer.order(CTLib::Buffer::LITTLE_ENDIAN);
    buffer.putShort(0xFF00);
    EXPECT_EQ(0x00, buffer[0]);
    EXPECT_EQ(0xFF, buffer[1]);

    buffer.flip();
    EXPECT_EQ(0xFF00, buffer.getShort());

    buffer.order(CTLib::Buffer::BIG_ENDIAN);
    buffer.flip();
    EXPECT_EQ(0x00FF, buffer.getShort());
}

TEST(BufferTests, DuplicateAndSlice)
{
    CTLib::Buffer buffer(16);
    CTLib::Buffer dup{buffer.duplicate()};
    EXPECT_EQ(16, dup.capacity());
    EXPECT_EQ(0, dup.position());
    EXPECT_EQ(16, dup.limit());

    buffer[0] = 0x20;
    buffer[2] = 0x48;
    EXPECT_EQ(0x20, dup[0]);
    EXPECT_EQ(0x48, dup[2]);

    dup[1] = 0x32;
    dup[2] = 0x10;
    EXPECT_EQ(0x32, buffer[1]);
    EXPECT_EQ(0x10, buffer[2]);

    buffer.position(4);
    CTLib::Buffer slice{buffer.slice()};
    EXPECT_EQ(12, slice.capacity());
    EXPECT_EQ(0, slice.position());
    EXPECT_EQ(12, slice.limit());

    buffer[4] = 0x64;
    buffer[5] = 0xAB;
    EXPECT_EQ(0x64, slice[0]);
    EXPECT_EQ(0xAB, slice[1]);

    slice[6] = 0x80;
    slice[8] = 0xFF;
    EXPECT_EQ(0x80, buffer[10]);
    EXPECT_EQ(0xFF, buffer[12]);
}

TEST(BufferTests, PutBufferWithSlice)
{
    CTLib::Buffer bigData(16);
    CTLib::Buffer data{bigData.position(3).slice()};
    data.put(0x44).put(0x24).put(0x92).put(0x46);
    data.flip();

    CTLib::Buffer bigBuffer(32);
    CTLib::Buffer buffer{bigBuffer.position(23).slice()};
    buffer.put(0x50).put(0x72).put(data).put(0x80);
    EXPECT_EQ(4, data.position());
    EXPECT_EQ(7, buffer.position());
    
    buffer.flip();
    EXPECT_EQ(0x50, buffer.get());
    EXPECT_EQ(0x72, buffer.get());
    EXPECT_EQ(0x44, buffer.get());
    EXPECT_EQ(0x24, buffer.get());
    EXPECT_EQ(0x92, buffer.get());
    EXPECT_EQ(0x46, buffer.get());
    EXPECT_EQ(0x80, buffer.get());
}

TEST(BufferTests, EqualityOperators)
{
    CTLib::Buffer a(5), b(6);
    a.putInt(0x00112233).put(0xFF);
    b.putInt(0x00112233).putShort(0xFF00).flip();
    EXPECT_FALSE(a == b);
    EXPECT_FALSE(b == a);
    EXPECT_TRUE(a != b);

    a.flip();
    b.limit(5);
    EXPECT_FALSE(a == b);

    b = CTLib::Buffer(5);
    b.put(a);
    EXPECT_TRUE(a == b);
    EXPECT_TRUE(b == a);
}

TEST(BufferTests, RelationalOperators)
{
    CTLib::Buffer a(6), b(6);
    a.putArray((uint8_t*) "apple", 6);
    b.putArray((uint8_t*) "grape", 6);
    EXPECT_TRUE(a < b);
    EXPECT_TRUE(a <= b);
    EXPECT_FALSE(a > b);
    EXPECT_FALSE(a >= b);
    EXPECT_FALSE(b < a);
    EXPECT_FALSE(b <= a);
    EXPECT_TRUE(b > a);
    EXPECT_TRUE(b >= a);

    b = a;
    EXPECT_FALSE(a < b);
    EXPECT_TRUE(a <= b);
    EXPECT_FALSE(a > b);
    EXPECT_TRUE(a >= b);
    EXPECT_FALSE(b < a);
    EXPECT_TRUE(b <= a);
    EXPECT_FALSE(b > a);
    EXPECT_TRUE(b >= a);

    a = CTLib::Buffer(7);
    a.putArray((uint8_t*) "apples", 7);
    EXPECT_FALSE(a < b);
    EXPECT_FALSE(a <= b);
    EXPECT_TRUE(a > b);
    EXPECT_TRUE(a >= b);
    EXPECT_TRUE(b < a);
    EXPECT_TRUE(b <= a);
    EXPECT_FALSE(b > a);
    EXPECT_FALSE(b >= a);
}

TEST(BufferTests, EqualsAndCompareTo)
{
    CTLib::Buffer a(4), b(6);
    a.putInt(0x331F9C30).flip();
    b.put(0xFC).putInt(0x331F9C30).put(0x2F).position(1).limit(5);
    EXPECT_TRUE(a.equals(b));

    b = a;
    b.position(1);
    EXPECT_FALSE(b.equals(a));

    a = CTLib::Buffer(10);
    b = CTLib::Buffer(10);
    a.putArray((uint8_t*) "apple", 6).flip();
    b.putArray((uint8_t*) "bapple", 7).flip();
    EXPECT_EQ(-1, a.compareTo(b)); // apple < bapple
    EXPECT_EQ(1, b.compareTo(a));

    b.position(1);
    EXPECT_EQ(0, a.compareTo(b)); // apple == bapple
    EXPECT_EQ(0, b.compareTo(a)); // ^^^^^    -^^^^^

    b.position(0);
    a.clear().putArray((uint8_t*) "zapple", 7).flip();
    EXPECT_EQ(1, a.compareTo(b)); // zapple > bapple
    EXPECT_EQ(-1, b.compareTo(a));
}

TEST(BufferTests, PosAndLimitOutOfBounds)
{
    CTLib::Buffer buffer(1);
    EXPECT_THROW({
        try
        {
            buffer.position(2);
        }
        catch (const CTLib::BufferError& e)
        {
            EXPECT_EQ(CTLib::BufferError::BUFFER_OVERFLOW, e.getType());
            throw;
        }
        
    }, CTLib::BufferError);
    
    buffer = CTLib::Buffer(4);
    EXPECT_THROW({
        try
        {
            buffer.limit(2).position(3);
        }
        catch (const CTLib::BufferError& e)
        {
            EXPECT_EQ(CTLib::BufferError::BUFFER_OVERFLOW, e.getType());
            throw;
        }
        
    }, CTLib::BufferError);
    
    buffer = CTLib::Buffer(1);
    EXPECT_THROW({
        try
        {
            buffer.limit(2);
        }
        catch (const CTLib::BufferError& e)
        {
            EXPECT_EQ(CTLib::BufferError::INVALID_LIMIT, e.getType());
            throw;
        }
        
    }, CTLib::BufferError);
    
    buffer = CTLib::Buffer(4);
    CTLib::Buffer slice{buffer.position(2).slice()};
    EXPECT_THROW({
        try
        {
            slice.limit(3);
        }
        catch (const CTLib::BufferError& e)
        {
            EXPECT_EQ(CTLib::BufferError::INVALID_LIMIT, e.getType());
            throw;
        }
        
    }, CTLib::BufferError);
    
    buffer = CTLib::Buffer(4);
    slice = CTLib::Buffer{buffer.position(2).slice()};
    EXPECT_THROW({
        try
        {
            slice.limit(1).position(2);
        }
        catch (const CTLib::BufferError& e)
        {
            EXPECT_EQ(CTLib::BufferError::BUFFER_OVERFLOW, e.getType());
            throw;
        }
        
    }, CTLib::BufferError);
}

TEST(BufferTests, SliceWithSamePositionAndLimit)
{
    CTLib::Buffer buffer(2);
    EXPECT_THROW({
        try
        {
            CTLib::Buffer slice{buffer.position(2).slice()};
        }
        catch (const CTLib::BufferError& e)
        {
            EXPECT_EQ(CTLib::BufferError::ALLOCATE_ZERO, e.getType());
            throw;
        }
        
    }, CTLib::BufferError);

    buffer = CTLib::Buffer(3);
    CTLib::Buffer slice0{buffer.position(1).slice()};
    EXPECT_THROW({
        try
        {
            CTLib::Buffer slice1{slice0.position(2).slice()};
        }
        catch (const CTLib::BufferError& e)
        {
            EXPECT_EQ(CTLib::BufferError::ALLOCATE_ZERO, e.getType());
            throw;
        }
        
    }, CTLib::BufferError);
}

TEST(BufferTests, RelativePutGetOutOfBounds)
{
    CTLib::Buffer buffer(1);
    EXPECT_THROW({
        try
        {
            buffer.put(0x64).put(0x20);
        }
        catch (const CTLib::BufferError& e)
        {
            EXPECT_EQ(CTLib::BufferError::BUFFER_OVERFLOW, e.getType());
            throw;
        }
    }, CTLib::BufferError);

    buffer = CTLib::Buffer(1);
    EXPECT_THROW({
        try
        {
            buffer.putShort(0xABCD);
        }
        catch (const CTLib::BufferError& e)
        {
            EXPECT_EQ(CTLib::BufferError::BUFFER_OVERFLOW, e.getType());
            throw;
        }
    }, CTLib::BufferError);
    
    buffer = CTLib::Buffer(1);
    EXPECT_THROW({
        try
        {
            buffer.getInt();
        }
        catch (const CTLib::BufferError& e)
        {
            EXPECT_EQ(CTLib::BufferError::BUFFER_OVERFLOW, e.getType());
            throw;
        }
    }, CTLib::BufferError);
    
    buffer = CTLib::Buffer(2);
    CTLib::Buffer slice{buffer.position(1).slice()};
    EXPECT_THROW({
        try
        {
            slice.getShort();
        }
        catch (const CTLib::BufferError& e)
        {
            EXPECT_EQ(CTLib::BufferError::BUFFER_OVERFLOW, e.getType());
            throw;
        }
    }, CTLib::BufferError);
}
