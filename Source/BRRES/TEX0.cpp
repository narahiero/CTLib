//////////////////////////////////////////////////
//  Copyright (c) 2020 Nara Hiero
//
// This file is licensed under GPLv3+
// Refer to the `License.txt` file included.
//////////////////////////////////////////////////

#include <CTLib/BRRES.hpp>

#include <CTLib/Utilities.hpp>

namespace CTLib
{

TEX0::TEX0(BRRES* brres, const std::string& name) :
    BRRESSubFile(brres, name),
    width{0},
    height{0},
    format{ImageFormat::I4},
    data{},
    mipmaps{}
{

}

TEX0::~TEX0() = default;

void TEX0::setTextureData(const Image& image, ImageFormat format)
{
    this->format = format;
    setTextureData(image);
}

void TEX0::setTextureData(const Image& image)
{
    deleteMipmaps();
    data = ImageCoder::encode(image, format);
    width = static_cast<uint16_t>(image.getWidth());
    height = static_cast<uint16_t>(image.getHeight());
}

void TEX0::setTextureData(Buffer& data, uint16_t width, uint16_t height, ImageFormat format)
{
    this->format = format;
    setTextureData(data, width, height);
}

void TEX0::setTextureData(Buffer& data, uint16_t width, uint16_t height)
{
    assertValidTextureData(data, width, height, format);

    deleteMipmaps();

    const size_t size = ImageCoder::sizeFor(width, height, format);
    this->data = Buffer(size);
    this->data.putArray(*data + data.position(), size).clear();

    this->width = width;
    this->height = height;

    data.position(data.position() + size);
}

uint16_t TEX0::getWidth() const
{
    return width;
}

uint16_t TEX0::getHeight() const
{
    return height;
}

ImageFormat TEX0::getFormat() const
{
    return format;
}

Buffer TEX0::getTextureData() const
{
    return data.duplicate();
}

void TEX0::setMipmapTextureData(uint32_t index, const Image& image)
{
    assertValidMipmapCount(index + 1);
    assertValidMipmapInsert(index);
    assertValidMipmapImage(index, image);

    Buffer mipmapData = ImageCoder::encode(image, format);
    if (mipmaps.size() == index)
    {
        mipmaps.push_back(std::move(mipmapData));
    }
    else
    {
        mipmaps.at(index) = std::move(mipmapData);
    }
}

void TEX0::setMipmapTextureData(uint32_t index, Buffer& data)
{
    assertValidMipmapCount(index + 1);
    assertValidMipmapInsert(index);
    assertValidMipmapTextureData(index, data);

    const size_t size = ImageCoder::sizeFor(getMipmapWidth(index), getMipmapHeight(index), format);
    Buffer mipmapData(size);
    mipmapData.putArray(*data + data.position(), size).clear();

    if (mipmaps.size() == index)
    {
        mipmaps.push_back(std::move(mipmapData));
    }
    else
    {
        mipmaps.at(index) = std::move(mipmapData);
    }

    data.position(data.position() + size);
}

void TEX0::generateMipmaps(uint32_t count, const Image& image)
{
    assertValidMipmapCount(count);
    
    deleteMipmaps();
    for (uint32_t i = 0; i < count; ++i)
    {
        Image mipmapImg = image.resize(getMipmapWidth(i), getMipmapHeight(i));
        mipmaps.push_back(ImageCoder::encode(mipmapImg, format));
    }
}

void TEX0::generateMipmaps(uint32_t count)
{
    // data is duplicated in order to avoid changing this buffer's state
    generateMipmaps(count, ImageCoder::decode(data.duplicate(), width, height, format));
}

void TEX0::deleteMipmaps()
{
    mipmaps.clear();
}

uint32_t TEX0::getMipmapCount() const
{
    return static_cast<uint32_t>(mipmaps.size());
}

uint16_t TEX0::getMipmapWidth(uint32_t index) const
{
    return width >> (index + 1);
}

uint16_t TEX0::getMipmapHeight(uint32_t index) const
{
    return height >> (index + 1);
}

Buffer TEX0::getMipmapTextureData(uint32_t index) const
{
    assertValidMipmap(index);
    return mipmaps[index].duplicate();
}

void TEX0::assertValidTextureData(
    const Buffer& data, uint16_t width, uint16_t height, ImageFormat format) const
{
    const size_t size = ImageCoder::sizeFor(width, height, format);
    if (data.remaining() < size)
    {
        throw BRRESError(Strings::format(
            "TEX0: Not enough bytes remaining in buffer for base texture data! (%d < %d)",
            data.remaining(), size
        ));
    }
}

void TEX0::assertValidMipmap(uint32_t index) const
{
    if (index > mipmaps.size())
    {
        throw BRRESError(Strings::format(
            "TEX0: Mipmap index is more than or equal to mipmap count! (%d >= %d)",
            index, mipmaps.size()
        ));
    }
}

void TEX0::assertValidMipmapCount(uint32_t count) const
{
    if (count == 0)
    {
        throw BRRESError("TEX0: Mipmap count cannot be 0!");
    }
    if ((width >> count) < 1 || (height >> count) < 1)
    {
        throw BRRESError(Strings::format(
            "TEX0: Too many mipmaps (%d)! Width or height of mipmap less than 1!",
            count
        ));
    }
}

void TEX0::assertValidMipmapInsert(uint32_t index) const
{
    if (mipmaps.size() < index)
    {
        throw BRRESError(Strings::format(
            "TEX0: 1 or more mipmaps between the last one and index (%d) are missing!",
            index
        ));
    }
}

void TEX0::assertValidMipmapImage(uint32_t index, const Image& image) const
{
    if (image.getWidth() != getMipmapWidth(index))
    {
        throw BRRESError(Strings::format(
            "TEX0: Invalid image width for mipmap index (%d)! (%d != %d)",
            index, image.getWidth(), getMipmapWidth(index)
        ));
    }
    if (image.getHeight() != getMipmapHeight(index))
    {
        throw BRRESError(Strings::format(
            "TEX0: Invalid image height for mipmap index (%d)! (%d != %d)",
            index, image.getHeight(), getMipmapHeight(index)
        ));
    }
}

void TEX0::assertValidMipmapTextureData(uint32_t index, const Buffer& data) const
{
    const size_t size = ImageCoder::sizeFor(getMipmapWidth(index), getMipmapHeight(index), format);
    if (data.remaining() < size)
    {
        throw BRRESError(Strings::format(
            "TEX0: Not enough bytes remaining in buffer for mipmap (%d) texture data! (%d < %d)",
            index, data.remaining(), size
        ));
    }
}
}
