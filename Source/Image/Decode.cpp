//////////////////////////////////////////////////
//  Copyright (c) 2020 Nara Hiero
//
// This file is licensed under GPLv3+
// Refer to the `License.txt` file included.
//////////////////////////////////////////////////

#include <CTLib/Image.hpp>

namespace CTLib
{

using PixelFunc = void (*)(Buffer&, Buffer&, uint32_t, uint32_t, uint32_t, uint32_t);
using BlockFunc = void (*)(Buffer&, Buffer&, uint32_t, uint32_t, uint32_t, uint32_t);

Image decodeBase(
    Buffer& data, uint32_t width, uint32_t height, uint8_t bw, uint8_t bh, PixelFunc func
)
{
    Buffer out(width * height * 4);

    uint32_t nw = (width & ~(bw - 1)) + ((width & (bw - 1)) > 0 ? bw : 0);
    uint32_t nh = (height & ~(bh - 1)) + ((height & (bh - 1)) > 0 ? bh : 0);

    for (uint32_t gy = 0; gy < nh; gy += bh)
    {
        for (uint32_t gx = 0; gx < nw; gx += bw)
        {
            for (uint32_t y = 0; y < bh; ++y)
            {
                for (uint32_t x = 0; x < bw; ++x)
                {
                    func(data, out, width, height, gx + x, gy + y);
                }
            }
        }
    }

    return Image(width, height, out);
}

Image decodeBlock(
    Buffer& data, uint32_t width, uint32_t height, uint8_t bw, uint8_t bh, int8_t sshift,
    BlockFunc func
)
{
    Buffer out(width * height * 4);

    uint32_t nw = (width & ~(bw - 1)) + ((width & (bw - 1)) > 0 ? bw : 0);
    uint32_t nh = (height & ~(bh - 1)) + ((height & (bh - 1)) > 0 ? bh : 0);

    size_t bSize = sshift < 0 ? ((bw * bh) >> -sshift) : ((bw * bh) << sshift);
    Buffer block(bSize);

    for (uint32_t gy = 0; gy < nh; gy += bh)
    {
        for (uint32_t gx = 0; gx < nw; gx += bw)
        {
            block.put(data.limit(data.position() + bSize));
            func(block, out, width, height, gx, gy);
            block.clear();
        }
    }

    return Image(width, height, out);
}

Image decodeI4(Buffer& data, uint32_t width, uint32_t height)
{
    return decodeBase(data, width, height, 8, 8,
        [](Buffer& data, Buffer& out, uint32_t width, uint32_t height, uint32_t x, uint32_t y)
        {
            uint8_t grey = data.get();
            if ((x & 1) == 0)
            {
                grey = (grey >> 4) * 0x11;
                data.position(data.position() - 1);
            }
            else
            {
                grey = (grey & 0xF) * 0x11;
            }

            if (x < width && y < height)
            {
                size_t off = ((y * width) + x) * 4;
                out.put(off + 0, grey);
                out.put(off + 1, grey);
                out.put(off + 2, grey);
                out.put(off + 3, 0xFF);
            }
        }
    );
}

Image decodeI8(Buffer& data, uint32_t width, uint32_t height)
{
    return decodeBase(data, width, height, 8, 4,
        [](Buffer& data, Buffer& out, uint32_t width, uint32_t height, uint32_t x, uint32_t y)
        {
            uint8_t grey = data.get();
            if (x < width && y < height)
            {
                size_t off = ((y * width) + x) * 4;
                out.put(off + 0, grey);
                out.put(off + 1, grey);
                out.put(off + 2, grey);
                out.put(off + 3, 0xFF);
            }
        }
    );
}

Image decodeIA4(Buffer& data, uint32_t width, uint32_t height)
{
    return decodeBase(data, width, height, 8, 4,
        [](Buffer& data, Buffer& out, uint32_t width, uint32_t height, uint32_t x, uint32_t y)
        {
            uint8_t px = data.get();
            uint8_t grey = (px & 0xF) * 0x11;
            if (x < width && y < height)
            {
                size_t off = ((y * width) + x) * 4;
                out.put(off + 0, grey);
                out.put(off + 1, grey);
                out.put(off + 2, grey);
                out.put(off + 3, (px >> 4) * 0x11);
            }
        }
    );
}

Image decodeIA8(Buffer& data, uint32_t width, uint32_t height)
{
    return decodeBase(data, width, height, 4, 4,
        [](Buffer& data, Buffer& out, uint32_t width, uint32_t height, uint32_t x, uint32_t y)
        {
            uint8_t alpha = data.get();
            uint8_t grey = data.get();
            if (x < width && y < height)
            {
                size_t off = ((y * width) + x) * 4;
                out.put(off + 0, grey);
                out.put(off + 1, grey);
                out.put(off + 2, grey);
                out.put(off + 3, alpha);
            }
        }
    );
}

Image decodeRGB565(Buffer& data, uint32_t width, uint32_t height)
{
    return decodeBase(data, width, height, 4, 4,
        [](Buffer& data, Buffer& out, uint32_t width, uint32_t height, uint32_t x, uint32_t y)
        {
            uint16_t rgb565 = data.getShort();
            if (x < width && y < height)
            {
                size_t off = ((y * width) + x) * 4;
                out.put(off + 0, (rgb565 & 0xF800) >> 8);
                out.put(off + 1, (rgb565 & 0x07E0) >> 3);
                out.put(off + 2, (rgb565 & 0x001F) << 3);
                out.put(off + 3, 0xFF);
            }
        }
    );
}

Image decodeRGB5A3(Buffer& data, uint32_t width, uint32_t height)
{
    return decodeBase(data, width, height, 4, 4,
        [](Buffer& data, Buffer& out, uint32_t width, uint32_t height, uint32_t x, uint32_t y)
        {
            uint16_t rgb5a3 = data.getShort();
            if (x < width && y < height)
            {
                size_t off = ((y * width) + x) * 4;
                if ((rgb5a3 & 0x8000) == 0) // with alpha
                {
                    out.put(off + 0, ((rgb5a3 & 0x0F00) >> 8) * 0x11);
                    out.put(off + 1, ((rgb5a3 & 0x00F0) >> 4) * 0x11);
                    out.put(off + 2, (rgb5a3 & 0x000F) * 0x11);
                    out.put(off + 3, (rgb5a3 & 0x7000) >> 7);
                }
                else // no alpha
                {
                    out.put(off + 0, (rgb5a3 & 0x7C00) >> 7);
                    out.put(off + 1, (rgb5a3 & 0x03E0) >> 2);
                    out.put(off + 2, (rgb5a3 & 0x001F) << 3);
                    out.put(off + 3, 0xFF);
                }
            }
        }
    );
}

Image decodeRGBA8(Buffer& data, uint32_t width, uint32_t height)
{
    return decodeBlock(data, width, height, 4, 4, 2,
        [](Buffer& block, Buffer& out, uint32_t width, uint32_t height, uint32_t gx, uint32_t gy)
        {
            for (uint32_t y = 0; y < 4; ++y)
            {
                for (uint32_t x = 0; x < 4; ++x)
                {
                    size_t off = (((gy + y) * width) + (gx + x)) * 4;
                    size_t offAR = ((y * 4) + x) * 2;
                    size_t offGB = offAR + 0x20;
                    if ((gx + x) < width && (gy + y) < height)
                    {
                        out.put(off + 0, block.get(offAR + 1));
                        out.put(off + 1, block.get(offGB + 0));
                        out.put(off + 2, block.get(offGB + 1));
                        out.put(off + 3, block.get(offAR + 0));
                    }
                }
            }
        }
    );
}

RGBAColour rgb565ToRGBA(uint16_t rgb565)
{
    return
    {
        static_cast<uint8_t>((rgb565 & 0xF800) >> 8),
        static_cast<uint8_t>((rgb565 & 0x07E0) >> 3),
        static_cast<uint8_t>((rgb565 & 0x001F) << 3),
        static_cast<uint8_t>(0xFF)
    };
}

RGBAColour colourHalf(RGBAColour a, RGBAColour b)
{
    return
    {
        static_cast<uint8_t>((a.r / 2) + (b.r / 2)),
        static_cast<uint8_t>((a.g / 2) + (b.g / 2)),
        static_cast<uint8_t>((a.b / 2) + (b.b / 2)),
        static_cast<uint8_t>((a.a / 2) + (b.a / 2))
    };
}

RGBAColour colourOneThird(RGBAColour a, RGBAColour b)
{
    return
    {
        static_cast<uint8_t>((a.r / 3) + ((b.r / 3) * 2)),
        static_cast<uint8_t>((a.g / 3) + ((b.g / 3) * 2)),
        static_cast<uint8_t>((a.b / 3) + ((b.b / 3) * 2)),
        static_cast<uint8_t>((a.a / 3) + ((b.a / 3) * 2))
    };
}

Image decodeCMPR(Buffer& data, uint32_t width, uint32_t height)
{
    return decodeBlock(data, width, height, 8, 8, -1,
        [](Buffer& block, Buffer& out, uint32_t width, uint32_t height, uint32_t gx, uint32_t gy)
        {
            block.clear();
            for (uint32_t y = 0; y < 8; y += 4)
            {
                for (uint32_t x = 0; x < 8; x += 4)
                {
                    uint16_t c0 = block.getShort(), c1 = block.getShort();
                    RGBAColour c[4] = {rgb565ToRGBA(c0), rgb565ToRGBA(c1), {}, {}};
                    if (c0 < c1)
                    {
                        c[2] = colourHalf(c[0], c[1]);
                        c[3] = {0x00, 0x00, 0x00, 0x00};
                    }
                    else
                    {
                        c[2] = colourOneThird(c[1], c[0]);
                        c[3] = colourOneThird(c[0], c[1]);
                    }

                    uint32_t indices = block.getInt();
                    for (uint32_t py = 0; py < 4; ++py)
                    {
                        for (uint32_t px = 0; px < 4; ++px)
                        {
                            size_t off = (((gy + y + py) * width) + (gx + x + px)) * 4;
                            if ((gx + x + px) < width && (gy + y + py) < height)
                            {
                                uint8_t idx = (indices >> (30 - (((py * 4) + px) * 2))) & 3;
                                out.put(off + 0, c[idx].r);
                                out.put(off + 1, c[idx].g);
                                out.put(off + 2, c[idx].b);
                                out.put(off + 3, c[idx].a);
                            }
                        }
                    }
                }
            }
            block.clear();
        }
    );
}

Image ImageCoder::decode(Buffer& data, uint32_t width, uint32_t height, ImageFormat format)
{
    try
    {
        switch (format)
        {
        case ImageFormat::I4:
            return decodeI4(data, width, height);

        case ImageFormat::I8:
            return decodeI8(data, width, height);

        case ImageFormat::IA4:
            return decodeIA4(data, width, height);
        
        case ImageFormat::IA8:
            return decodeIA8(data, width, height);

        case ImageFormat::RGB565:
            return decodeRGB565(data, width, height);

        case ImageFormat::RGB5A3:
            return decodeRGB5A3(data, width, height);

        case ImageFormat::RGBA8:
            return decodeRGBA8(data, width, height);

        case ImageFormat::CMPR:
            return decodeCMPR(data, width, height);

        default:
            throw ImageError("Unsupported format!");
        }
    }
    catch (const BufferError&)
    {
        throw ImageError("Not enough bytes in encoded buffer!");
    }
}
}
