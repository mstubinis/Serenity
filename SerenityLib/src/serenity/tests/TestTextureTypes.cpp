
#include <serenity/resources/texture/TextureIncludes.h>
#include <serenity/resources/texture/Texture.h>

#include <catch.hpp>

TEST_CASE("Texture Types tests") {
    TextureWrap wrap = TextureWrap::ClampToEdge;
    TextureType type = TextureType::RenderTarget;
    TextureFilter filter = TextureFilter::Nearest_Mipmap_Linear;

    REQUIRE(wrap == TextureWrap::ClampToEdge);
    REQUIRE(type == TextureType::RenderTarget);
    REQUIRE(filter == TextureFilter::Nearest_Mipmap_Linear);

    GLuint wrapGLType = (GLuint)wrap;
    REQUIRE(wrapGLType == GL_CLAMP_TO_EDGE);
    wrap = TextureWrap::RepeatMirrored;
    wrapGLType = (GLuint)wrap;
    REQUIRE(wrapGLType == GL_MIRRORED_REPEAT);

    GLuint filterGLType;
    filterGLType = filter.toGLType(true);
    REQUIRE(filterGLType == GL_NEAREST_MIPMAP_LINEAR);


    filter = TextureFilter::Linear_Mipmap_Linear;
    filterGLType = filter.toGLType(false);
    REQUIRE(filterGLType == GL_LINEAR);

    GLuint typeGLType;
    typeGLType = (GLuint)type;
    REQUIRE(typeGLType == GL_TEXTURE_2D);
    type = TextureType::CubeMap;
    typeGLType = (GLuint)type;
    REQUIRE(typeGLType == GL_TEXTURE_CUBE_MAP);

    REQUIRE(type.toGLType() == GL_TEXTURE_CUBE_MAP);
    type = TextureType::Texture2D;
    REQUIRE(type.toGLType() == GL_TEXTURE_2D);
    type = TextureType::RenderTarget;
    REQUIRE(type.toGLType() == GL_TEXTURE_2D);
    type = TextureType::CubeMap_Y_Pos;
    REQUIRE(type.toGLType() == GL_TEXTURE_CUBE_MAP_POSITIVE_Y);
    type = TextureType::Texture1D;
    REQUIRE(type.toGLType() == GL_TEXTURE_1D);
}