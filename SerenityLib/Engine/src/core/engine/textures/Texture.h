#pragma once
#ifndef ENGINE_TEXTURE_H
#define ENGINE_TEXTURE_H

struct TextureRequest;
namespace Engine::priv {
    class GBuffer;
    template<typename T> class ResourceVector;
}

#include <core/engine/resources/Handle.h>
#include <core/engine/textures/TextureLoader.h>
#include <core/engine/resources/Resource.h>
#include <core/engine/renderer/GLImageConstants.h>

#include <GL/glew.h>
#include <SFML/OpenGL.hpp>

namespace Engine::priv {
    struct TextureCPUData final {
        std::vector<Engine::priv::ImageData>   m_ImagesDatas     = { Engine::priv::ImageData {} };
        std::string                            m_Name            = "";
        unsigned int                           m_MinFilter       = GL_LINEAR; //used to determine filter type for mipmaps
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
        Engine::priv::TextureCPUData                      m_CPUData;
        std::queue<std::function<void()>>                 m_CommandQueue;              //for commands that were not available until the texture was properly loaded
        GLuint                                            m_TextureAddress           = 0U; 
        Handle                                            m_ConvolutionTextureHandle = Handle{};
        Handle                                            m_PreEnvTextureHandle      = Handle{};

        bool internal_bind_if_not_bound(unsigned int requestedAddress) noexcept;
        GLuint& internal_get_address_for_generation() noexcept { return m_TextureAddress; }

        //Framebuffer
        Texture(
            unsigned int renderTgtWidth,
            unsigned int renderTgtHeight,
            ImagePixelType pixelType,
            ImagePixelFormat pixelFormat,
            ImageInternalFormat internalFormat,
            float divisor = 1.0f
        );
    public:
        //Empty Texture
        Texture(
            const std::string& textureName = "",
            TextureType textureType = TextureType::Texture2D,
            bool mipMap = false
        );
        //Single File
        Texture(
            const std::string& filename,
            bool generateMipmaps,
            ImageInternalFormat internalFormat,
            TextureType openglTexureType
        );
        //Pixels From Memory
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

        unsigned char* pixels();
        GLuint address() const noexcept { return m_TextureAddress; }
        inline CONSTEXPR TextureType getTextureType() const noexcept { return m_CPUData.m_TextureType; }
        inline unsigned int width() const noexcept { return m_CPUData.m_ImagesDatas[0].m_Mipmaps[0].width; }
        inline unsigned int height() const noexcept { return m_CPUData.m_ImagesDatas[0].m_Mipmaps[0].height; }
        inline glm::uvec2 size() const noexcept { return glm::uvec2(width(), height()); }
        glm::vec2 sizeAsRatio() const;
        inline CONSTEXPR bool mipmapped() const noexcept { return m_CPUData.m_Mipmapped; }
        bool compressed() const;
        void setAnisotropicFiltering(float anisotropicFiltering);
        
        inline CONSTEXPR ImageInternalFormat internalFormat() const noexcept { return m_CPUData.m_ImagesDatas[0].m_InternalFormat; }
        inline CONSTEXPR ImagePixelFormat pixelFormat() const noexcept { return m_CPUData.m_ImagesDatas[0].m_PixelFormat; }
        inline CONSTEXPR ImagePixelType pixelType() const noexcept { return m_CPUData.m_ImagesDatas[0].m_PixelType; }

        void setXWrapping(TextureWrap);
        void setYWrapping(TextureWrap);
        void setZWrapping(TextureWrap);
        void setWrapping(TextureWrap);

        void setMinFilter(TextureFilter);
        void setMaxFilter(TextureFilter);
        void setFilter(TextureFilter);

        static void setXWrapping(TextureType type, TextureWrap);
        static void setYWrapping(TextureType type, TextureWrap);
        static void setZWrapping(TextureType type, TextureWrap);
        static void setWrapping(TextureType type, TextureWrap);

        static void setMinFilter(TextureType type, TextureFilter);
        static void setMaxFilter(TextureType type, TextureFilter);
        static void setFilter(TextureType type, TextureFilter);
};
#endif
