//////////////////////////////////////////////////
//  Copyright (c) 2020 Nara Hiero
//
// This file is licensed under GPLv3+
// Refer to the `License.txt` file included.
//////////////////////////////////////////////////

#pragma once

/*! @file Image.hpp
 * 
 *  @brief The header containing all image-related utilities.
 */


#include <stdexcept>

#include <CTLib/Memory.hpp>


namespace CTLib
{

/*! @brief A 8-bit-per-component RGBA colour. */
struct RGBAColour
{
    /*! @brief 0: `r`, 1: `g`, 2: `b`, 3: `a` */
    uint8_t operator[](size_t index) const;

    /*! @brief The `red` component of this colour. */
    uint8_t r = 0x00;

    /*! @brief The `green` component of this colour. */
    uint8_t g = 0x00;

    /*! @brief The `blue` component of this colour. */
    uint8_t b = 0x00;

    /*! @brief The `alpha` component of this colour. */
    uint8_t a = 0x00;
};

/*! @brief An instance of the Image class is an immutable image representation
 *  with a width, height, and RGBA pixel data.
 */
class Image
{

    friend class ImageIO;

public:

    /*! @brief Constructs an image of the specified width, height, and pixel 
     *  data.
     * 
     *  The passed data will be copied in an appropriately sized buffer, that
     *  is to say, the width times the height times 4 (for RGBA).
     * 
     *  If the passed buffer has less bytes remaining than the size of the
     *  internal buffer, all remaining pixels will be set to the specified
     *  default colour.
     * 
     *  @param[in] width The width of the image, in pixels
     *  @param[in] height The height of the image, in pixels
     *  @param[in] data The RGBA pixel data of the image
     *  @param[in] colour The default colour
     */
    Image(uint32_t width, uint32_t height, const Buffer& data, RGBAColour colour = {});

    /*! @brief Constructs a copy of the specified image. */
    Image(const Image&);

    /*! @brief Constructs a new image and moves the contents of the specified
     *  image.
     * 
     *  The moved image _**MUST NOT**_ be used afterwards.
     */
    Image(Image&&);

    /*! @brief Returns the pointer to this image's data. */
    uint8_t* operator*() const;

    /*! @brief Returns whether the specified image equals this. */
    bool operator==(const Image&) const;

    /*! @brief Returns the width of this image, in pixels. */
    uint32_t getWidth() const;

    /*! @brief Returns the height of this image, in pixels. */
    uint32_t getHeight() const;

    /*! @brief Returns a @link CTLib::Buffer::duplicate() duplicated@endlink
     *  buffer of this image's data.
     */
    Buffer getData() const;

    /*! @brief Returns the offset in image data for the specified pixel. */
    size_t offsetFor(uint32_t x, uint32_t y) const;

private:

    // constructor used by ImageIO to prevent an additional copy
    Image(uint32_t width, uint32_t height, uint8_t* data);

    // the width of this image, in pixels
    const uint32_t width;

    // the height of this image, in pixels
    const uint32_t height;

    // the RGBA pixel data of this image
    Buffer buffer;
};

/*! @brief Enumeration of Nintendo's image formats. */
enum class ImageFormat
{
    /*! @brief 4 bits per pixel greyscale image. */
    I4 = 0x0,

    /*! @brief 8 bits per pixel greyscale image. */
    I8 = 0x1,

    /*! @brief 8 bits per pixel (4 for grey, 4 for alpha) greyscale image with
     *  alpha.
     */
    IA4 = 0x2,

    /*! @brief 16 bits per pixel (8 for grey, 8 for alpha) greyscale image with
     *  alpha.
     */
    IA8 = 0x3,

    /*! @brief 16 bits per pixel (5 for red, 6 for green, 5 for blue) RGB
     *  image.
     */
    RGB565 = 0x4,

    /*! @brief 16 bits per pixel with two different formats: with first bit set
     *  (4 for red, green, and blue, 3 for alpha) for RGB image with alpha,
     *  with first bit cleared (5 for red, green, and blue) for RGB image.
     */
    RGB5A3 = 0x5,

    /*! @brief 32 bits per pixel (8 for red, green, blue, and alpha) for RGB
     *  image with alpha.
     */
    RGBA8 = 0x6,

    /*! @brief 4 bits per pixel image with palette. **NOT YET SUPPORTED** */
    C4 = 0x8,

    /*! @brief 8 bits per pixel image with palette. **NOT YET SUPPORTED** */
    C8 = 0x9,

    /*! @brief 14 bits per pixel image with palette. **NOT YET SUPPORTED** */
    C14X2 = 0xA,

    /*! @brief 4 bits per pixel 'lossy-ly' compressed RGB image with alpha.
     *
     *  **WARNING**: Encoding yields incorrect result and is mostly unreliable.
     */
    CMPR = 0xE
};

/*! @brief The ImageCoder class contains methods to encode and decode image
 *  data.
 */
class ImageCoder
{

public:

    /*! @brief Encodes the specified image in the specified format.
     *  
     *  **NOTE**: The @link CTLib::ImageFormat::CMPR CMPR@endlink format
     *  currently results in an incorrect image.
     * 
     *  @param[in] image The image to be encoded
     *  @param[in] format The encoding format
     * 
     *  @throw CTLib::ImageError If the specified format is unsupported.
     * 
     *  @return The encoded image data
     */
    static Buffer encode(Image& image, ImageFormat format);

    /*! @brief Decodes the specified image data.
     *  
     *  @param[in] data The encoded image data
     *  @param[in] width The width of the encoded image data
     *  @param[in] height The height of the encoded image data
     *  @param[in] format The format of the encoded image data
     * 
     *  @throw CTLib::ImageError If the encoded image data is invalid, or the
     *  format is unsupported.
     * 
     *  @return The decoded image
     */
    static Image decode(Buffer& data, uint32_t width, uint32_t height, ImageFormat format);
};

/*! @brief Enumeration of supported image formats when writing. */
enum class ImageIOFormat
{
    BMP, JPEG, PNG, TGA
};

/*! @brief The ImageIO class contains methods to read and write images to
 *  and from files.
 */
class ImageIO
{

public:

    /*! @brief Reads an image from the specified file.
     *  
     *  Supported image formats are (taken from stb_image.h):
     * 
     *  JPEG baseline & progressive (12 bpc/arithmetic not supported, same as
     *  stock IJG lib)\n
     *  PNG 1/2/4/8/16-bit-per-channel\n
     * 
     *  TGA (not sure what subset, if a subset)\n
     *  BMP non-1bpp, non-RLE\n
     *  PSD (composited view only, no extra channels, 8/16 bit-per-channel)\n
     * 
     *  GIF (*comp always reports as 4-channel)\n
     *  HDR (radiance rgbE format)\n
     *  PIC (Softimage PIC)\n
     *  PNM (PPM and PGM binary only)\n
     * 
     *  @param[in] filename The path to the image file
     * 
     *  @throw CTLib::ImageError If the image file does not exists or the image
     *  data is invalid or corrupted.
     * 
     *  @return The contents of the image file
     */
    static Image read(const char* filename);

    /*! @brief See @link CTLib::ImageIO::read(const char*) read@endlink */
    static Image read(const std::string& filename);

    /*! @brief Writes the specified image to the specified file with the
     *  specified format.
     *
     *  @param[in] filename The path the the output image file
     *  @param[in] image The image to be written
     *  @param[in] format The image format, defaults to
     *  CTLib::ImageIOFormat::PNG
     * 
     *  @throw CTLib::ImageError If an error occurs while writing the image.
     */
    static void write(
        const char* filename, const Image& image, ImageIOFormat format = ImageIOFormat::PNG
    );

    /*! @brief See @link 
     *  ImageIO::write(const char*, const CTLib::Image&, CTLib::ImageIOFormat)
     *  write@endlink
     */
    static void write(
        const std::string& filename, const Image& image, ImageIOFormat format = ImageIOFormat::PNG
    );
};

/*! @brief ImageError is the error class used by the methods in this header. */
class ImageError : public std::runtime_error
{

public:

    /*! @brief Constructs a CTLib::ImageError with the specified message. */
    ImageError(const char* msg);

    /*! @brief Constructs a CTLib::ImageError with the specified message. */
    ImageError(const std::string& msg);
};
}
