#pragma once
#ifndef ENGINE_TEXTURE_INCLUDE_GUARD
#define ENGINE_TEXTURE_INCLUDE_GUARD

namespace Engine::priv {
    struct TextureRequestStaticImpl;
};

#include <core/engine/textures/TextureLoader.h>
#include <core/engine/resources/Resource.h>
#include <core/engine/renderer/GLImageConstants.h>

#include <glm/vec2.hpp>
#include <glm/vec4.hpp>
#include <GL/glew.h>
#include <SFML/OpenGL.hpp>
#include <memory>
#include <queue>
#include <functional>

class Texture: public Resource {
    friend struct Engine::priv::TextureLoader;
    friend struct Engine::priv::InternalTexturePublicInterface;
    friend struct Engine::priv::TextureRequestStaticImpl;
    public:
        static Texture *White, *Black, *Checkers, *BRDF; //loaded in renderer. TODO: move these to built in class (separate from client side interface)
    private:
        std::queue< std::function<void()> >               m_CommandQueue;              //for commands that were not available until the texture was properly loaded
        std::vector<Engine::priv::ImageLoadedStructure>   m_ImagesDatas;
        std::vector<GLuint>                               m_TextureAddresses; 
        unsigned int                                      m_Type                 = 0U;
        TextureType::Type                                 m_TextureType          = TextureType::Texture2D;
        bool                                              m_Mipmapped            = false;
        bool                                              m_IsToBeMipmapped      = false;
        unsigned int                                      m_MinFilter            = 0U; //used to determine filter type for mipmaps

        Texture();
    public:
        //Framebuffer
        Texture(
            unsigned int renderTgtWidth, 
            unsigned int renderTgtHeight,
            ImagePixelType::Type pixelType, 
            ImagePixelFormat::Format pixelFormat, 
            ImageInternalFormat::Format internalFormat,
            float divisor = 1.0f
        );
        //Single File
        Texture(
            const std::string& filename,
            bool generateMipmaps = true,
            ImageInternalFormat::Format internalFormat = ImageInternalFormat::Format::SRGB8_ALPHA8,
            unsigned int openglTexureType = GL_TEXTURE_2D
        );
        //Pixels From Memory
        Texture(
            const sf::Image& sfImage,
            const std::string& textureName = "CustomTexture",
            bool generateMipmaps = false, 
            ImageInternalFormat::Format internalFormat = ImageInternalFormat::Format::SRGB8_ALPHA8,
            unsigned int openglTexureType = GL_TEXTURE_2D
        );
        //Cubemap from 6 files
        Texture(
            const std::string files[],
            const std::string& textureName = "Cubemap", 
            bool generateMipmaps = false,
            ImageInternalFormat::Format internalFormat = ImageInternalFormat::Format::SRGB8_ALPHA8
        );
        virtual ~Texture();

        bool operator==(const bool rhs) const;
        explicit operator bool() const;

        Texture(const Texture&)                      = delete;
        Texture& operator=(const Texture&)           = delete;
        Texture(Texture&& other) noexcept            = delete;
        Texture& operator=(Texture&& other) noexcept = delete;

        unsigned char* pixels();
        GLuint address(unsigned int index = 0) const;
        unsigned int type() const;
        unsigned int width() const;
        unsigned int height() const;
        glm::uvec2 size() const;
        size_t numAddresses() const;
        bool mipmapped() const;
        bool compressed() const;
        void setAnisotropicFiltering(float anisotropicFiltering);
        
        ImageInternalFormat::Format internalFormat() const;
        ImagePixelFormat::Format pixelFormat() const;
        ImagePixelType::Type pixelType() const;

        void setXWrapping(TextureWrap::Wrap);
        void setYWrapping(TextureWrap::Wrap);
        void setZWrapping(TextureWrap::Wrap);
        void setWrapping(TextureWrap::Wrap);

        void setMinFilter(TextureFilter::Filter);
        void setMaxFilter(TextureFilter::Filter);
        void setFilter(TextureFilter::Filter);

        static void setXWrapping(unsigned int type, TextureWrap::Wrap);
        static void setYWrapping(unsigned int type, TextureWrap::Wrap);
        static void setZWrapping(unsigned int type, TextureWrap::Wrap);
        static void setWrapping(unsigned int type, TextureWrap::Wrap);

        static void setMinFilter(unsigned int type, TextureFilter::Filter);
        static void setMaxFilter(unsigned int type, TextureFilter::Filter);
        static void setFilter(unsigned int type, TextureFilter::Filter);
};
#endif
