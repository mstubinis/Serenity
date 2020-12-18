#pragma once
#ifndef ENGINE_TEXTURE_INCLUDES_H
#define ENGINE_TEXTURE_INCLUDES_H

#include <GL/glew.h>
#include <SFML/OpenGL.hpp>
#include <array>
#include <vector>
#include <serenity/core/engine/system/Macros.h>
#include <serenity/core/engine/system/TypeDefs.h>

class TextureWrap final {
    public: enum Type {
        Repeat,
        RepeatMirrored,
        ClampToEdge,
        ClampToBorder,
        _TOTAL,
    };
    private:
        TextureWrap::Type m_Type = TextureWrap::Repeat;

        static constexpr const std::array<GLuint, (size_t)TextureWrap::_TOTAL> GL_VALUES { {
            GL_REPEAT,
            GL_MIRRORED_REPEAT,
            GL_CLAMP_TO_BORDER,
            GL_CLAMP_TO_EDGE,
        } };

    public:
        constexpr TextureWrap() = default;
        constexpr TextureWrap(const TextureWrap::Type other) { m_Type = other; }
        inline constexpr bool operator==(const TextureWrap::Type other) const noexcept { return m_Type == other; }
        inline constexpr bool operator!=(const TextureWrap::Type other) const noexcept { return !operator==(other); }
        inline constexpr void operator=(const TextureWrap::Type other) noexcept { m_Type = other; }
        inline explicit constexpr operator TextureWrap::Type() const noexcept { return m_Type; }
        inline explicit constexpr operator GLuint() const noexcept { return static_cast<GLuint>(toGLType()); }
        constexpr GLuint toGLType() const noexcept {
            ASSERT(m_Type < TextureWrap::_TOTAL, "TextureWrap::m_Type is an invalid value!");
            return GL_VALUES[m_Type];
        }
};

class TextureFilter final {
    public: enum Type {
        Linear,
        Nearest,
        Nearest_Mipmap_Nearest,
        Nearest_Mipmap_Linear,
        Linear_Mipmap_Nearest,
        Linear_Mipmap_Linear,
        _TOTAL,
    };
    private:
        TextureFilter::Type m_Type = TextureFilter::Linear;

    public:
        constexpr TextureFilter() = default;
        constexpr TextureFilter(const TextureFilter::Type other) { m_Type = other; }
        inline constexpr bool operator==(const TextureFilter::Type other) const noexcept { return m_Type == other; }
        inline constexpr bool operator!=(const TextureFilter::Type other) const noexcept { return !operator==(other); }
        inline constexpr void operator=(const TextureFilter::Type other) noexcept { m_Type = other; }
        inline explicit constexpr operator TextureFilter::Type() const noexcept { return m_Type; }
        constexpr GLuint toGLType(bool min) const noexcept {
            ASSERT(m_Type < TextureFilter::_TOTAL, "TextureFilter::m_Type is an invalid value!");
            if (min) {
                if      (m_Type == TextureFilter::Linear)                  return static_cast<GLuint>(GL_LINEAR);
                else if (m_Type == TextureFilter::Nearest)                 return static_cast<GLuint>(GL_NEAREST);
                else if (m_Type == TextureFilter::Linear_Mipmap_Linear)    return static_cast<GLuint>(GL_LINEAR_MIPMAP_LINEAR);
                else if (m_Type == TextureFilter::Linear_Mipmap_Nearest)   return static_cast<GLuint>(GL_LINEAR_MIPMAP_NEAREST);
                else if (m_Type == TextureFilter::Nearest_Mipmap_Linear)   return static_cast<GLuint>(GL_NEAREST_MIPMAP_LINEAR);
                else if (m_Type == TextureFilter::Nearest_Mipmap_Nearest)  return static_cast<GLuint>(GL_NEAREST_MIPMAP_NEAREST);
            }else {
                if      (m_Type == TextureFilter::Linear)                  return static_cast<GLuint>(GL_LINEAR);
                else if (m_Type == TextureFilter::Nearest)                 return static_cast<GLuint>(GL_NEAREST);
                else if (m_Type == TextureFilter::Linear_Mipmap_Linear)    return static_cast<GLuint>(GL_LINEAR);
                else if (m_Type == TextureFilter::Linear_Mipmap_Nearest)   return static_cast<GLuint>(GL_LINEAR);
                else if (m_Type == TextureFilter::Nearest_Mipmap_Linear)   return static_cast<GLuint>(GL_NEAREST);
                else if (m_Type == TextureFilter::Nearest_Mipmap_Nearest)  return static_cast<GLuint>(GL_NEAREST);
            }
            return static_cast<GLuint>(0);
        }
};

class TextureType final {
    public: enum Type {
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
    private:
        TextureType::Type m_Type = TextureType::Unknown;

        static constexpr const std::array<GLuint, (size_t)TextureType::_TOTAL> GL_VALUES { {
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
        constexpr TextureType() = default;
        constexpr TextureType(const uint32_t other) { 
            m_Type = static_cast<TextureType::Type>(other); 
            ASSERT(m_Type > TextureType::Unknown && m_Type < TextureType::_TOTAL, "TextureType::m_Type is an invalid value!");
        }
        constexpr TextureType(const TextureType::Type other) { m_Type = other; }
        inline constexpr bool operator==(const TextureType::Type other) const noexcept { return m_Type == other; }
        inline constexpr bool operator!=(const TextureType::Type other) const noexcept { return !operator==(other); }
        inline constexpr void operator=(const TextureType::Type other) noexcept { m_Type = other; }
        inline constexpr bool operator>(const TextureType::Type other) noexcept { return m_Type > other; }
        inline constexpr bool operator<(const TextureType::Type other) noexcept { return m_Type < other; }
        inline constexpr bool operator>=(const TextureType::Type other) noexcept { return m_Type >= other; }
        inline constexpr bool operator<=(const TextureType::Type other) noexcept { return m_Type <= other; }
        inline explicit constexpr operator TextureType::Type() const noexcept { return m_Type; }
        constexpr GLuint toGLType() const noexcept {
            ASSERT(m_Type > TextureType::Unknown && m_Type < TextureType::_TOTAL, "TextureType::m_Type is an invalid value!");
            return GL_VALUES[m_Type];
        }
};

class Texture;
namespace sf { 
    class Image; 
}
namespace Engine::priv {
    class  FramebufferObject;
};

#include <serenity/core/engine/renderer/GLImageConstants.h>

namespace Engine::priv {
    struct ImageMipmap final {
        std::vector<uint8_t>   pixels;
        uint32_t               compressedSize = 0U;
        int                         width          = 0U;
        int                         height         = 0U;
        int                         level          = 0U;

        inline bool null() const noexcept { return pixels.size() == 0; }
    };
    struct ImageData final {
        std::vector<priv::ImageMipmap>  m_Mipmaps         = { priv::ImageMipmap {} };
        std::string                     m_Filename        = "";
        ImageInternalFormat             m_InternalFormat  = ImageInternalFormat::Unknown;
        ImagePixelFormat                m_PixelFormat     = ImagePixelFormat::Unknown;
        ImagePixelType                  m_PixelType       = ImagePixelType::UNSIGNED_BYTE;

        void setInternalFormat(ImageInternalFormat);
        void load(int width, int height, ImagePixelType pixelType, ImagePixelFormat pixelFormat, ImageInternalFormat internalFormat);
        void load(const sf::Image& sfImage, const std::string& filename = "");

        bool hasBlankMipmap() const noexcept {
            bool res = m_Mipmaps.size() == 0;
            if (!res) {
                for (const auto& mip : m_Mipmaps) {
                    if (mip.null()) {
                        return true;
                    }
                }
            }
            return res;
        }
    };
};


#endif