#pragma once
#ifndef ENGINE_TEXTURE_H
#define ENGINE_TEXTURE_H

struct TextureRequest;
namespace Engine::priv {
    class GBuffer;
    template<typename T> class ResourceVector;
}

#include <serenity/core/engine/resources/Handle.h>
#include <serenity/core/engine/textures/TextureLoader.h>
#include <serenity/core/engine/resources/Resource.h>
#include <serenity/core/engine/renderer/GLImageConstants.h>
#include <serenity/core/engine/dependencies/glm.h>

#include <GL/glew.h>
#include <SFML/OpenGL.hpp>
#include <queue>
#include <functional>
#include <vector>
#include <string>

namespace Engine::priv {
    struct TextureCPUData final {
        std::vector<Engine::priv::ImageData>   m_ImagesDatas     = { Engine::priv::ImageData {} };
        std::string                            m_Name            = "";
        uint32_t                               m_MinFilter       = GL_LINEAR; //used to determine filter type for mipmaps
        TextureType                            m_TextureType     = TextureType::Unknown;
        bool                                   m_Mipmapped       = false;
        bool                                   m_IsToBeMipmapped = false;

        void initFromMemory(const sf::Image& sfImage);
        void initFromFile();
        void initFromCubemap(const std::array<std::string, 6>& files, ImageInternalFormat intFmt);
    };
};

class Texture: public Resource {
    friend class  Engine::priv::ResourceVector<Texture>;
    friend class  Engine::priv::GBuffer;
    friend struct Engine::priv::TextureLoader;
    friend struct TextureRequest;
    public:
        static Handle White, Black, Checkers, BRDF; //loaded in renderer. TODO: move these to built in class (separate from client side interface)
    private:
        Engine::priv::TextureCPUData                m_CPUData;
        std::queue<std::function<void()>>           m_CommandQueue;              //for commands that were not available until the texture was properly loaded
        GLuint                                      m_TextureAddress           = 0U; 
        Handle                                      m_ConvolutionTextureHandle = Handle{};
        Handle                                      m_PreEnvTextureHandle      = Handle{};

        bool internal_bind_if_not_bound(uint32_t requestedAddress) noexcept;
        GLuint& internal_get_address_for_generation() noexcept { return m_TextureAddress; }

    public:
        //Empty Texture
        Texture(
            const std::string& textureName = "",
            TextureType textureType = TextureType::Texture2D,
            bool mipMap = false
        );

        //Framebuffer
        Texture(
            uint32_t renderTgtWidth,
            uint32_t renderTgtHeight,
            ImagePixelType pixelType,
            ImagePixelFormat pixelFormat,
            ImageInternalFormat internalFormat,
            float divisor = 1.0f
        );

        //Single File
        Texture(
            const std::string& filename,
            bool generateMipmaps,
            ImageInternalFormat internalFormat,
            TextureType openglTexureType
        );

        //Pixels From SFML Image Memory
        Texture(
            const sf::Image& sfImage,
            const std::string& textureName,
            bool generateMipmaps, 
            ImageInternalFormat internalFormat,
            TextureType openglTexureType
        );

        //Cubemap from 6 files
        Texture(
            const std::array<std::string, 6>& files,
            const std::string& textureName, 
            bool generateMipmaps,
            ImageInternalFormat internalFormat
        );

        Texture(const Texture& other)                = delete;
        Texture& operator=(const Texture& other)     = delete;
        Texture(Texture&& other) noexcept;
        Texture& operator=(Texture&& other) noexcept;

        virtual ~Texture();

        inline bool operator==(bool rhs) const noexcept { return (rhs == true && m_TextureAddress != 0); }
        explicit inline operator bool() const noexcept { return m_TextureAddress != 0; }

        inline bool hasGlobalIlluminationData() const noexcept { return (bool)(!m_ConvolutionTextureHandle.null() && !m_PreEnvTextureHandle.null()); }

        inline Handle getConvolutionTexture() const noexcept { return m_ConvolutionTextureHandle; }
        inline Handle getPreEnvTexture() const noexcept { return m_PreEnvTextureHandle; }

        uint8_t* pixels();
        GLuint address() const noexcept { return m_TextureAddress; }
        inline constexpr TextureType getTextureType() const noexcept { return m_CPUData.m_TextureType; }
        inline int width() const noexcept { 
            ASSERT(m_CPUData.m_ImagesDatas.size() > 0, __FUNCTION__ << "(): m_CPUData.m_ImagesDatas.size() was 0!");
            ASSERT(m_CPUData.m_ImagesDatas[0].m_Mipmaps.size() > 0, __FUNCTION__ << "(): m_CPUData.m_ImagesDatas[0].m_Mipmaps.size() was 0!");
            return m_CPUData.m_ImagesDatas[0].m_Mipmaps[0].width; 
        }
        inline int height() const noexcept { 
            ASSERT(m_CPUData.m_ImagesDatas.size() > 0, __FUNCTION__ << "(): m_CPUData.m_ImagesDatas.size() was 0!");
            ASSERT(m_CPUData.m_ImagesDatas[0].m_Mipmaps.size() > 0, __FUNCTION__ << "(): m_CPUData.m_ImagesDatas[0].m_Mipmaps.size() was 0!");
            return m_CPUData.m_ImagesDatas[0].m_Mipmaps[0].height; 
        }
        inline glm::uvec2 size() const noexcept { return glm::uvec2(width(), height()); }
        inline glm::vec2 sizeAsRatio() const noexcept { const auto size_ = glm::vec2(size()); return glm::vec2(size_ / glm::max(size_.x, size_.y)); }
        inline constexpr bool mipmapped() const noexcept { return m_CPUData.m_Mipmapped; }
        bool compressed() const;
        void setAnisotropicFiltering(float anisotropicFiltering);
        
        inline ImageInternalFormat internalFormat() const noexcept { 
            ASSERT(m_CPUData.m_ImagesDatas.size() > 0, __FUNCTION__ << "(): m_CPUData.m_ImagesDatas.size() was 0!");
            return m_CPUData.m_ImagesDatas[0].m_InternalFormat; 
        }
        inline ImagePixelFormat pixelFormat() const noexcept { 
            ASSERT(m_CPUData.m_ImagesDatas.size() > 0, __FUNCTION__ << "(): m_CPUData.m_ImagesDatas.size() was 0!");
            return m_CPUData.m_ImagesDatas[0].m_PixelFormat; 
        }
        inline ImagePixelType pixelType() const noexcept { 
            ASSERT(m_CPUData.m_ImagesDatas.size() > 0, __FUNCTION__ << "(): m_CPUData.m_ImagesDatas.size() was 0!");
            return m_CPUData.m_ImagesDatas[0].m_PixelType; 
        }

        void setXWrapping(TextureWrap);
        void setYWrapping(TextureWrap);
        void setZWrapping(TextureWrap);
        void setWrapping(TextureWrap);

        void setMinFilter(TextureFilter);
        void setMaxFilter(TextureFilter);
        void setFilter(TextureFilter);
        static void setFilter(TextureType type, TextureFilter filter) noexcept { setMinFilter(type, filter); setMaxFilter(type, filter); }

        static void setXWrapping(TextureType, TextureWrap);
        static void setYWrapping(TextureType, TextureWrap);
        static void setZWrapping(TextureType, TextureWrap);
        static void setWrapping(TextureType, TextureWrap);

        static void setMinFilter(TextureType, TextureFilter);
        static void setMaxFilter(TextureType, TextureFilter);
};
#endif
