
#include <serenity/renderer/opengl/GLImageConstants.h>

#include <catch.hpp>

TEST_CASE("GL Image Types tests") {

    ImagePixelFormat pixelFormat       = ImagePixelFormat::RGBA;
    ImageInternalFormat internalFormat = ImageInternalFormat::COMPRESSED_SRGB_S3TC_DXT1_EXT;

    REQUIRE(internalFormat == ImageInternalFormat::COMPRESSED_SRGB_S3TC_DXT1_EXT);
    REQUIRE(pixelFormat == ImagePixelFormat::RGBA);
    REQUIRE(internalFormat.isCompressedType());
    internalFormat = ImageInternalFormat::RGBA8;
    REQUIRE(!internalFormat.isCompressedType());

    pixelFormat = internalFormat;
    REQUIRE(pixelFormat == ImagePixelFormat::RGBA);

    internalFormat = ImageInternalFormat::COMPRESSED_SRGB_S3TC_DXT1_EXT;
    pixelFormat = internalFormat;
    REQUIRE(pixelFormat == ImagePixelFormat::RGB);
}