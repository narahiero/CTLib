//////////////////////////////////////////////////
//  Copyright (c) 2020 Nara Hiero
//
// This file is licensed under GPLv3+
// Refer to the `License.txt` file included.
//////////////////////////////////////////////////

#include <CTLib/Image.hpp>

#include <stb_dxt.h>

namespace CTLib
{

using PixelFunc = void (*)(Buffer&, Buffer&, uint32_t, uint32_t, uint32_t, uint32_t);
using BlockFunc = void (*)(Buffer&, Buffer&, uint32_t, uint32_t, uint32_t, uint32_t);

Buffer encodeBase(const Image& image, uint8_t bw, uint8_t bh, int8_t sshift, PixelFunc func)
{
    uint32_t nw = (image.getWidth() & ~(bw - 1)) + ((image.getWidth() & (bw - 1)) > 0 ? bw : 0);
    uint32_t nh = (image.getHeight() & ~(bh - 1)) + ((image.getHeight() & (bh - 1)) > 0 ? bh : 0);

    Buffer data(sshift < 0 ? ((nw * nh) >> -sshift) : ((nw * nh) << sshift));

    for (uint32_t gy = 0; gy < nh; gy += bh)
    {
        for (uint32_t gx = 0; gx < nw; gx += bw)
        {
            for (uint32_t y = 0; y < bh; ++y)
            {
                for (uint32_t x = 0; x < bw; ++x)
                {
                    func(
                        image.getData(), data, image.getWidth(), image.getHeight(),
                        gx + x, gy + y
                    );
                }
            }
        }
    }

    return data.flip();
}

Buffer encodeBlock(const Image& image, uint8_t bw, uint8_t bh, int8_t sshift, BlockFunc func)
{
    uint32_t nw = (image.getWidth() & ~(bw - 1)) + ((image.getWidth() & (bw - 1)) > 0 ? bw : 0);
    uint32_t nh = (image.getHeight() & ~(bh - 1)) + ((image.getHeight() & (bh - 1)) > 0 ? bh : 0);

    Buffer data(sshift < 0 ? ((nw * nh) >> -sshift) : ((nw * nh) << sshift));
    Buffer block(sshift < 0 ? ((bw * bh) >> -sshift) : ((bw * bh) << sshift));

    for (uint32_t gy = 0; gy < nh; gy += bh)
    {
        for (uint32_t gx = 0; gx < nw; gx += bw)
        {
            func(image.getData(), block, image.getWidth(), image.getHeight(), gx, gy);
            data.put(block);
            block.clear();
        }
    }
    
    return data.flip();
}

uint8_t computeGreyscale(uint8_t rB, uint8_t gB, uint8_t bB)
{
    constexpr double RED_FACTOR = .2126, GREEN_FACTOR = .7152, BLUE_FACTOR = .0722;
    double r = rB / 255., g = gB / 255., b = bB / 255.;
    double add = (r * RED_FACTOR) + (g * GREEN_FACTOR) + (b * BLUE_FACTOR);
    return static_cast<uint8_t>(add * 255);
}

Buffer encodeI4(const Image& image)
{
    return encodeBase(image, 8, 8, -1,
        [](Buffer& img, Buffer& data, uint32_t width, uint32_t height, uint32_t x, uint32_t y)
        {
            if (x >= width || y >= height)
            {
                if ((x & 1) == 0)
                {
                    data.put(0);
                }
            }
            else
            {
                size_t off = ((y * width) + x) * 4;
                uint8_t grey = computeGreyscale(
                    img.get(off + 0), img.get(off + 1), img.get(off + 2)
                );

                if ((x & 1) == 0)
                {
                    data.put(grey & 0xF0);
                }
                else
                {
                    data.position(data.position() - 1);
                    uint8_t old = data[data.position()];
                    data.put(old | (grey >> 4));
                }
            }
        }
    );
}

Buffer encodeI8(const Image& image)
{
    return encodeBase(image, 8, 4, 0,
        [](Buffer& img, Buffer& data, uint32_t width, uint32_t height, uint32_t x, uint32_t y)
        {
            if (x >= width || y >= height)
            {
                data.put(0);
            }
            else
            {
                size_t off = ((y * width) + x) * 4;
                data.put(computeGreyscale(
                    img.get(off + 0), img.get(off + 1), img.get(off + 2)    
                ));
            }
        }
    );
}

Buffer encodeIA4(const Image& image)
{
    return encodeBase(image, 8, 4, 0,
        [](Buffer& img, Buffer& data, uint32_t width, uint32_t height, uint32_t x, uint32_t y)
        {
            if (x >= width || y >= height)
            {
                data.put(0);
            }
            else
            {
                size_t off = ((y * width) + x) * 4;
                uint8_t grey = computeGreyscale(
                    img.get(off + 0), img.get(off + 1), img.get(off + 2)    
                );
                data.put((img.get(off + 3) & 0xF0) | (grey >> 4));
            }
        }
    );
}

Buffer encodeIA8(const Image& image)
{
    return encodeBase(image, 4, 4, 1,
        [](Buffer& img, Buffer& data, uint32_t width, uint32_t height, uint32_t x, uint32_t y)
        {
            if (x >= width || y >= height)
            {
                data.putShort(0);
            }
            else
            {
                size_t off = ((y * width) + x) * 4;
                data.put(img.get(off + 3));
                data.put(computeGreyscale(
                    img.get(off + 0), img.get(off + 1), img.get(off + 2)    
                ));
            }
        }
    );
}

Buffer encodeRGB565(const Image& image)
{
    return encodeBase(image, 4, 4, 1,
        [](Buffer& img, Buffer& data, uint32_t width, uint32_t height, uint32_t x, uint32_t y)
        {
            if (x >= width || y >= height)
            {
                data.putShort(0);
            }
            else
            {
                size_t off = ((y * width) + x) * 4;
                data.putShort((static_cast<uint16_t>(img.get(off + 0) & 0xF8) << 8)
                    | (static_cast<uint16_t>(img.get(off + 1) & 0xFC) << 3)
                    | (static_cast<uint16_t>(img.get(off + 2)) >> 3));
            }
        }
    );
}

Buffer encodeRGB5A3(const Image& image)
{
    return encodeBase(image, 4, 4, 1,
        [](Buffer& img, Buffer& data, uint32_t width, uint32_t height, uint32_t x, uint32_t y)
        {
            if (x >= width || y >= height)
            {
                data.putShort(0);
            }
            else
            {
                size_t off = ((y * width) + x) * 4;
                uint8_t alpha = img.get(off + 3);
                if (alpha < 0xE0) // add alpha
                {
                    data.putShort((static_cast<uint16_t>(img.get(off + 3) & 0xE0) << 7)
                        | (static_cast<uint16_t>(img.get(off + 0) & 0xF0) << 4)
                        | static_cast<uint16_t>(img.get(off + 1) & 0xF0)
                        | (static_cast<uint16_t>(img.get(off + 2)) >> 4));
                }
                else // no alpha
                {
                    data.putShort(0x8000 // first bit set
                        | (static_cast<uint16_t>(img.get(off + 0) & 0xF8) << 7)
                        | (static_cast<uint16_t>(img.get(off + 1) & 0xF8) << 2)
                        | (static_cast<uint16_t>(img.get(off + 2)) >> 3));
                }
            }
        }
    );
}

Buffer encodeRGBA8(const Image& image)
{
    return encodeBlock(image, 4, 4, 2,
        [](Buffer& img, Buffer& block, uint32_t width, uint32_t height, uint32_t gx, uint32_t gy)
        {
            for (uint32_t y = 0; y < 4; ++y)
            {
                for (uint32_t x = 0; x < 4; ++x)
                {
                    size_t off = (((gy + y) * width) + (gx + x)) * 4;
                    size_t offAR = ((y * 4) + x) * 2;
                    size_t offGB = offAR + 0x20;
                    if ((gx + x) >= width || (gy + y) >= height)
                    {
                        block.put(offAR + 0, 0).put(offAR + 1, 0);
                        block.put(offGB + 0, 0).put(offGB + 1, 0);
                    }
                    else
                    {
                        block.put(offAR + 0, img.get(off + 3)).put(offAR + 1, img.get(off + 0));
                        block.put(offGB + 0, img.get(off + 1)).put(offGB + 1, img.get(off + 2));
                    }
                }
            }
        }
    );
}

void getDataBlock(Buffer& imgBlock, Buffer& img, uint32_t w, uint32_t h, uint32_t x, uint32_t y)
{
    for (uint32_t l = 0; l < 4; ++l)
    {
        if ((y + l) >= h)
        {
            // copy first row
            imgBlock.putArray(*imgBlock, 4 * 4);
            continue;
        }

        for (uint32_t p = 0; p < 4; ++p)
        {
            if ((x + p) >= w)
            {
                // copy first pixel of row
                imgBlock.putInt(imgBlock.getInt((y + l) * w * 4));
            }
            else
            {
                imgBlock.putInt(img.getInt((((y + l) * w) + (x + p)) * 4));
            }
        }
    }
}

void compressBlockDXT1(Buffer& block, Buffer& imgBlock)
{
    stb_compress_dxt_block(
        *block + block.position(), *imgBlock, false, STB_DXT_DITHER | STB_DXT_HIGHQUAL
    );

    block.order(Buffer::LITTLE_ENDIAN);

    // I know c0 and c1 are supposed to be called c1 and c2 but whatever... :P
    uint16_t c0 = block.getShort(block.position()), c1 = block.getShort(block.position() + 2);
    block.order(Buffer::BIG_ENDIAN).putShort(block.position(), c0).putShort(block.position() + 2, c1);

    // flip block horizontally
    uint32_t indices = block.getInt(block.position() + 4);
    for (size_t i = 0; i < 4; ++i)
    {
        const size_t shift = i * 8;
        uint8_t row = (indices >> shift) & 0xFF;
        row = ((row & 0x03) << 6) | ((row & 0x0C) << 2) | ((row & 0x30) >> 2) | ((row & 0xC0) >> 6);
        indices = (indices & ~(0xFF << shift)) | (row << shift);
    }
    block.putInt(block.position() + 4, indices);

    block.position(block.position() + 8);
}

Buffer encodeCMPR(const Image& image)
{
    return encodeBlock(image, 8, 8, -1,
        [](Buffer& img, Buffer& block, uint32_t width, uint32_t height, uint32_t gx, uint32_t gy)
        {
            Buffer imgBlock(0x40); // 4x4 RGBA block
            for (uint32_t y = 0; y < 8; y += 4)
            {
                for (uint32_t x = 0; x < 8; x += 4)
                {
                    imgBlock.clear();
                    getDataBlock(imgBlock, img, width, height, gx + x, gy + y);
                    compressBlockDXT1(block, imgBlock);
                }
            }
            block.flip();
        }
    );
}

Buffer ImageCoder::encode(const Image& image, ImageFormat format)
{
    switch (format)
    {
    case ImageFormat::I4:
        return encodeI4(image);

    case ImageFormat::I8:
        return encodeI8(image);

    case ImageFormat::IA4:
        return encodeIA4(image);
    
    case ImageFormat::IA8:
        return encodeIA8(image);

    case ImageFormat::RGB565:
        return encodeRGB565(image);

    case ImageFormat::RGB5A3:
        return encodeRGB5A3(image);

    case ImageFormat::RGBA8:
        return encodeRGBA8(image);

    case ImageFormat::CMPR:
        return encodeCMPR(image);

    default:
        throw ImageError("Unsupported encoding format!");
    }
}

struct FormatInfo
{
    uint8_t bw, bh;
    int8_t sshift;
};

FormatInfo formatInfo(ImageFormat format)
{
    switch (format)
    {
    case ImageFormat::I4:
        return {8, 8, -1};

    case ImageFormat::I8:
        return {8, 4, 0};

    case ImageFormat::IA4:
        return {8, 4, 0};

    case ImageFormat::IA8:
        return {4, 4, 1};

    case ImageFormat::RGB565:
        return {4, 4, 1};

    case ImageFormat::RGB5A3:
        return {4, 4, 1};

    case ImageFormat::RGBA8:
        return {4, 4, 2};

    case ImageFormat::CMPR:
        return {8, 8, -1};

    default:
        throw ImageError("Unsupported encoding format!");
    }
}

size_t ImageCoder::sizeFor(uint32_t width, uint32_t height, ImageFormat format)
{
    FormatInfo info = formatInfo(format);

    uint32_t nw = (width & ~(info.bw - 1)) + ((width & (info.bw - 1)) > 0 ? info.bw : 0);
    uint32_t nh = (height & ~(info.bh - 1)) + ((height & (info.bh - 1)) > 0 ? info.bh : 0);

    return info.sshift < 0 ? ((nw * nh) >> -info.sshift) : ((nw * nh) << info.sshift);
}
}
