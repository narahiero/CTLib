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

    buffer.resize(8);
    EXPECT_EQ(8, buffer.capacity());

    CTLib::Buffer buffer2(64);
    EXPECT_EQ(64, buffer2.capacity());

    buffer2.resize(128);
    EXPECT_EQ(128, buffer2.capacity());
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

TEST(BufferTests, Truncate)
{
    CTLib::Buffer buffer(4);
    buffer.position(2).limit(3).truncate();
    EXPECT_EQ(2, buffer.capacity());
    EXPECT_EQ(2, buffer.position());
    EXPECT_EQ(2, buffer.limit());
}

TEST(BufferTests, ZeroSizedAllocs)
{
    EXPECT_THROW({
        try
        {
            CTLib::Buffer(0);
        }
        catch(const CTLib::BufferError& e)
        {
            EXPECT_EQ(CTLib::BufferError::ALLOCATE_ZERO, e.getType());
            throw;
        }
    }, CTLib::BufferError);

    EXPECT_THROW({
        try
        {
            CTLib::Buffer buffer(1);
            buffer.resize(0);
        }
        catch(const CTLib::BufferError& e)
        {
            EXPECT_EQ(CTLib::BufferError::ALLOCATE_ZERO, e.getType());
            throw;
        }
    }, CTLib::BufferError);

    EXPECT_THROW({
        try
        {
            CTLib::Buffer buffer(1);
            buffer.truncate();
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
