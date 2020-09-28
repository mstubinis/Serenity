#pragma once
#ifndef ENGINE_TEXTURE_H
#define ENGINE_TEXTURE_H

namespace Engine::priv {
    struct TextureRequestStaticImpl;
};

#include <core/engine/textures/TextureLoader.h>
#include <core/engine/resources/Resource.h>
#include <core/engine/renderer/GLImageConstants.h>

#include <GL/glew.h>
#include <SFML/OpenGL.hpp>

class Texture: public Resource {
    friend struct Engine::priv::TextureLoader;
    friend struct Engine::priv::InternalTexturePublicInterface;
    friend struct Engine::priv::TextureRequestStaticImpl;
    public:
        static Texture *White, *Black, *Checkers, *BRDF; //loaded in renderer. TODO: move these to built in class (separate from client side interface)
    private:
        std::queue< std::function<void()> >               m_CommandQueue;              //for commands that were not available until the texture was properly loaded
        std::vector<Engine::priv::ImageLoadedStructure>   m_ImagesDatas;
        GLuint                                            m_TextureAddress       = 0U; 
        unsigned int                                      m_Type                 = 0U;
        TextureType                                       m_TextureType          = TextureType::Unknown;
        bool                                              m_Mipmapped            = false;
        bool                                              m_IsToBeMipmapped      = false;
        unsigned int                                      m_MinFilter            = 0U; //used to determine filter type for mipmaps
        Texture* m_ConvolutionTexture = nullptr;
        Texture* m_PreEnvTexture      = nullptr;

        Texture();

        bool internal_bind_if_not_bound(unsigned int requestedAddress) noexcept;
        GLuint& internal_get_address_for_generation() noexcept { return m_TextureAddress; }
    public:
        //Empty Texture
        Texture(
            const std::string& textureName,
            TextureType textureType,
            unsigned int width,
            unsigned int height,
            bool mipMap
        );
        //Framebuffer
        Texture(
            unsigned int renderTgtWidth, 
            unsigned int renderTgtHeight,
            ImagePixelType pixelType, 
            ImagePixelFormat pixelFormat, 
            ImageInternalFormat internalFormat,
            float divisor = 1.0f
        );
        //Single File
        Texture(
            const std::string& filename,
            bool generateMipmaps = true,
            ImageInternalFormat internalFormat = ImageInternalFormat::SRGB8_ALPHA8,
            unsigned int openglTexureType = GL_TEXTURE_2D
        );
        //Pixels From Memory
        Texture(
            const sf::Image& sfImage,
            const std::string& textureName = "CustomTexture",
            bool generateMipmaps = false, 
            ImageInternalFormat internalFormat = ImageInternalFormat::SRGB8_ALPHA8,
            unsigned int openglTexureType = GL_TEXTURE_2D
        );
        //Cubemap from 6 files
        Texture(
            const std::string files[],
            const std::string& textureName = "Cubemap", 
            bool generateMipmaps = false,
            ImageInternalFormat internalFormat = ImageInternalFormat::SRGB8_ALPHA8
        );
        virtual ~Texture();

        inline bool operator==(bool rhs) const noexcept { return (rhs == true) ? (m_TextureAddress != 0) : !(m_TextureAddress != 0); }
        explicit inline operator bool() const noexcept { return m_TextureAddress != 0; }

        inline bool hasGlobalIlluminationData() const noexcept { return (bool)(m_ConvolutionTexture != nullptr && m_PreEnvTexture != nullptr); }

        inline const Texture& getConvolutionTexture() const noexcept { return *m_ConvolutionTexture; }
        inline const Texture& getPreEnvTexture() const noexcept { return *m_PreEnvTexture; }

        Texture(const Texture&)                      = delete;
        Texture& operator=(const Texture&)           = delete;
        Texture(Texture&& other) noexcept            = delete;
        Texture& operator=(Texture&& other) noexcept = delete;

        unsigned char* pixels();
        GLuint address() const noexcept { return m_TextureAddress; }
        inline CONSTEXPR unsigned int type() const noexcept { return m_Type; }
        inline unsigned int width() const noexcept { return m_ImagesDatas[0].m_Mipmaps[0].width; }
        inline unsigned int height() const noexcept { return m_ImagesDatas[0].m_Mipmaps[0].height; }
        inline glm::uvec2 size() const noexcept { return glm::uvec2(width(), height()); }
        glm::vec2 sizeAsRatio() const;
        inline CONSTEXPR bool mipmapped() const noexcept { return m_Mipmapped; }
        bool compressed() const;
        void setAnisotropicFiltering(float anisotropicFiltering);
        
        inline CONSTEXPR ImageInternalFormat internalFormat() const noexcept { return m_ImagesDatas[0].m_InternalFormat; }
        inline CONSTEXPR ImagePixelFormat pixelFormat() const noexcept { return m_ImagesDatas[0].m_PixelFormat; }
        inline CONSTEXPR ImagePixelType pixelType() const noexcept { return m_ImagesDatas[0].m_PixelType; }

        void setXWrapping(TextureWrap);
        void setYWrapping(TextureWrap);
        void setZWrapping(TextureWrap);
        void setWrapping(TextureWrap);

        void setMinFilter(TextureFilter);
        void setMaxFilter(TextureFilter);
        void setFilter(TextureFilter);

        static void setXWrapping(unsigned int type, TextureWrap);
        static void setYWrapping(unsigned int type, TextureWrap);
        static void setZWrapping(unsigned int type, TextureWrap);
        static void setWrapping(unsigned int type, TextureWrap);

        static void setMinFilter(unsigned int type, TextureFilter);
        static void setMaxFilter(unsigned int type, TextureFilter);
        static void setFilter(unsigned int type, TextureFilter);
};
#endif
