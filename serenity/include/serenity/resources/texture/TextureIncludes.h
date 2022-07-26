#pragma once
#ifndef ENGINE_TEXTURE_INCLUDES_H
#define ENGINE_TEXTURE_INCLUDES_H

class Texture;
namespace sf {
    class Image;
}
namespace Engine::priv {
    class  FramebufferObject;
};

#include <GL/glew.h>
#include <SFML/OpenGL.hpp>
#include <array>
#include <vector>
#include <serenity/system/Macros.h>
#include <serenity/system/TypeDefs.h>
#include <SFML/Graphics/Image.hpp>

class TextureWrap final {
    public: 
        enum Type : uint32_t {
            Repeat = 0,
            RepeatMirrored,
            ClampToEdge,
            ClampToBorder,
            _TOTAL,
        };
        BUILD_ENUM_CLASS_MEMBERS(TextureWrap, Type)
    private:
        static constexpr const std::array<GLuint, TextureWrap::_TOTAL> GL_VALUES { {
            GL_REPEAT,
            GL_MIRRORED_REPEAT,
            GL_CLAMP_TO_EDGE,
            GL_CLAMP_TO_BORDER,
        } };
    public:
        [[nodiscard]] inline constexpr GLuint toGLType() const noexcept {
            ASSERT(m_Type < TextureWrap::_TOTAL, "TextureWrap::m_Type is an invalid value!");
            return GL_VALUES[m_Type];
        }
};

class TextureFilter final {
    public: 
        enum Type : uint32_t {
            Linear = 0,
            Nearest,
            Nearest_Mipmap_Nearest,
            Nearest_Mipmap_Linear,
            Linear_Mipmap_Nearest,
            Linear_Mipmap_Linear,
            _TOTAL,
        };
        BUILD_ENUM_CLASS_MEMBERS(TextureFilter, Type)
    public:
        [[nodiscard]] constexpr GLuint toGLType(bool min) const noexcept {
            ASSERT(m_Type < TextureFilter::_TOTAL, "TextureFilter::m_Type is an invalid value!");
            if      (m_Type == TextureFilter::Linear)                  return GLuint(min ? GL_LINEAR : GL_LINEAR);
            else if (m_Type == TextureFilter::Nearest)                 return GLuint(min ? GL_NEAREST : GL_NEAREST);
            else if (m_Type == TextureFilter::Linear_Mipmap_Linear)    return GLuint(min ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
            else if (m_Type == TextureFilter::Linear_Mipmap_Nearest)   return GLuint(min ? GL_LINEAR_MIPMAP_NEAREST : GL_LINEAR);
            else if (m_Type == TextureFilter::Nearest_Mipmap_Linear)   return GLuint(min ? GL_NEAREST_MIPMAP_LINEAR : GL_NEAREST);
            else if (m_Type == TextureFilter::Nearest_Mipmap_Nearest)  return GLuint(min ? GL_NEAREST_MIPMAP_NEAREST : GL_NEAREST);
            return GLuint(0);
        }
};

class TextureType final {
    public: 
        enum Type : uint32_t {
            Unknown = 0,
            Texture1D = 1,
            Texture2D,
            Texture3D,
            CubeMap,
            RenderTarget,
            CubeMap_X_Pos,
            CubeMap_X_Neg,
            CubeMap_Y_Pos,
            CubeMap_Y_Neg,
            CubeMap_Z_Pos,
            CubeMap_Z_Neg,
            _TOTAL,
        };
        BUILD_ENUM_CLASS_MEMBERS(TextureType, Type)
    private:
        static constexpr const std::array<GLuint, TextureType::_TOTAL> GL_VALUES { {
            0,
            GL_TEXTURE_1D,
            GL_TEXTURE_2D,
            GL_TEXTURE_3D,
            GL_TEXTURE_CUBE_MAP,
            GL_TEXTURE_2D,
            GL_TEXTURE_CUBE_MAP_POSITIVE_X,
            GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
            GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
            GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
            GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
            GL_TEXTURE_CUBE_MAP_NEGATIVE_Z,
        } };
    public:
        [[nodiscard]] inline constexpr GLuint toGLType() const noexcept {
            ASSERT(m_Type > TextureType::Unknown && m_Type < TextureType::_TOTAL, "TextureType::m_Type is an invalid value!");
            return GL_VALUES[m_Type];
        }
};

#include <serenity/renderer/opengl/GLImageConstants.h>

namespace Engine::priv {
    struct ImageMipmap final {
        std::vector<uint8_t>   pixels;
        uint32_t               compressedSize = 0;
        int                    width          = 0;
        int                    height         = 0;
        int                    level          = 0;

        [[nodiscard]] inline bool isNull() const noexcept { return pixels.size() == 0; }
        void assignPixels(const uint8_t* inData, int inWidth, int inHeight) {
            width = inWidth;
            height = inHeight;
            pixels.assign(inData, inData + inWidth * inHeight * 4);
        }
        void assignPixels(const uint8_t* inData) {
            pixels.assign(inData, inData + width * height * 4);
        }
    };
    struct ImageData final {
        std::vector<ImageMipmap>     m_Mipmaps         = { ImageMipmap{} };
        std::string                  m_Filename;
        ImageInternalFormat          m_InternalFormat  = ImageInternalFormat::Unknown;
        ImagePixelFormat             m_PixelFormat     = ImagePixelFormat::Unknown;
        ImagePixelType               m_PixelType       = ImagePixelType::UNSIGNED_BYTE;

        void setInternalFormat(ImageInternalFormat);
        void load(int width, int height, ImagePixelType, ImagePixelFormat, ImageInternalFormat);
        //void load(const sf::Image& sfImage, const std::string& filename = {});
        void load(const uint8_t* inPixels, int inWidth, int inHeight, const std::string& filename = {});

        [[nodiscard]] bool hasBlankMipmap() const noexcept {
            bool res = m_Mipmaps.size() == 0;
            if (!res) {
                for (const auto& mip : m_Mipmaps) {
                    if (mip.isNull()) {
                        return true;
                    }
                }
            }
            return res;
        }
    };
};

class sfImageLoaderFlags {
    public:
        enum Type : uint32_t {
            None             = 0,
            FlipHorizontally = 1 << 0,
            FlipVertically   = 1 << 1,
        };
        BUILD_ENUM_CLASS_MEMBERS(sfImageLoaderFlags, Type)
};

class sfImageLoader {
    private:
        sf::Image m_SFImage;
    public:
        sfImageLoader() = default;
        sfImageLoader(const char* filename, sfImageLoaderFlags = sfImageLoaderFlags::None);

        void loadFromFile(const char* filename, sfImageLoaderFlags = sfImageLoaderFlags::None);
        void loadFromMemory(const void* data, size_t size, sfImageLoaderFlags = sfImageLoaderFlags::None);

        [[nodiscard]] inline operator sf::Image&() noexcept { return m_SFImage; }
        [[nodiscard]] inline operator const sf::Image&() const noexcept { return m_SFImage; }
        [[nodiscard]] inline const uint8_t* getPixels() const noexcept { return m_SFImage.getPixelsPtr(); }
        [[nodiscard]] inline uint32_t getWidth() const noexcept { return m_SFImage.getSize().x; }
        [[nodiscard]] inline uint32_t getHeight() const noexcept { return m_SFImage.getSize().y; }
};

#endif