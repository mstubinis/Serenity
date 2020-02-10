#pragma once
#ifndef ENGINE_TEXTURE_INCLUDE_GUARD
#define ENGINE_TEXTURE_INCLUDE_GUARD

#include <core/engine/textures/TextureLoader.h>
#include <core/engine/resources/Engine_ResourceBasic.h>
#include <core/engine/renderer/GLImageConstants.h>

#include <glm/vec2.hpp>
#include <glm/vec4.hpp>
#include <GL/glew.h>
#include <SFML/OpenGL.hpp>
#include <memory>
#include <queue>
#include <functional>

namespace Engine {
    namespace priv {
        struct InternalTextureRequestPublicInterface;
    };
};

class Texture: public EngineResource{
    friend struct Engine::priv::TextureLoader;
    friend struct Engine::priv::InternalTexturePublicInterface;
    friend struct Engine::priv::InternalTextureRequestPublicInterface;

    public:
        static Texture *White, *Black, *Checkers, *BRDF; //loaded in renderer. TODO: move these to built in class (separate from client side interface)
    private:
        std::queue<std::function<void()>>                                   m_CommandQueue; //for commands that were not available until the texture was properly loaded
        std::vector<std::unique_ptr<Engine::priv::ImageLoadedStructure>>    m_ImagesDatas;
        std::vector<GLuint>                                                 m_TextureAddress;
        GLuint                                                              m_Type;
        TextureType::Type                                                   m_TextureType;
        bool                                                                m_Mipmapped;
        bool                                                                m_IsToBeMipmapped;
        GLuint                                                              m_MinFilter; //used to determine filter type for mipmaps

        Texture();
    public:
        //Framebuffer
        Texture(const uint& renderTgtWidth,const uint& renderTgtHeight,const ImagePixelType::Type&,const ImagePixelFormat::Format&,const ImageInternalFormat::Format&,const float& divisor = 1.0f);
        //Single File
        Texture(const std::string& filename,const bool& genMipmaps = true,const ImageInternalFormat::Format& = ImageInternalFormat::Format::SRGB8_ALPHA8,const GLuint& openglTexType = GL_TEXTURE_2D);
        //Pixels From Memory
        Texture(const sf::Image& sfImage,const std::string& name = "CustomTexture",const bool& genMipmaps = false,const ImageInternalFormat::Format& = ImageInternalFormat::Format::SRGB8_ALPHA8,const GLuint& openglTexType = GL_TEXTURE_2D);
        //Cubemap from 6 files
        Texture(const std::string files[],const std::string& name = "Cubemap",const bool& genMipmaps = false,const ImageInternalFormat::Format& = ImageInternalFormat::Format::SRGB8_ALPHA8);
        virtual ~Texture();

        bool operator==(const bool& rhs) const;
        explicit operator bool() const;

        Texture(const Texture&)                      = delete;
        Texture& operator=(const Texture&)           = delete;
        Texture(Texture&& other) noexcept            = delete;
        Texture& operator=(Texture&& other) noexcept = delete;

        const unsigned char* pixels();
        const GLuint address(const uint& index = 0) const;
        const GLuint type() const;
        const uint width() const;
        const uint height() const;
        const glm::uvec2 size() const;
        const size_t numAddresses() const;
        const bool mipmapped() const;
        const bool compressed() const;
        void setAnisotropicFiltering(const float& anisotropicFiltering);
        
        const ImageInternalFormat::Format internalFormat() const;
        const ImagePixelFormat::Format pixelFormat() const;
        const ImagePixelType::Type pixelType() const;

        void setXWrapping(const TextureWrap::Wrap&);
        void setYWrapping(const TextureWrap::Wrap&);
        void setZWrapping(const TextureWrap::Wrap&);
        void setWrapping(const TextureWrap::Wrap&);

        void setMinFilter(const TextureFilter::Filter&);
        void setMaxFilter(const TextureFilter::Filter&);
        void setFilter(const TextureFilter::Filter&);

        static void setXWrapping(const GLuint& type, const TextureWrap::Wrap&);
        static void setYWrapping(const GLuint& type, const TextureWrap::Wrap&);
        static void setZWrapping(const GLuint& type, const TextureWrap::Wrap&);
        static void setWrapping(const GLuint& type, const TextureWrap::Wrap&);

        static void setMinFilter(const GLuint& type, const TextureFilter::Filter&);
        static void setMaxFilter(const GLuint& type, const TextureFilter::Filter&);
        static void setFilter(const GLuint& type, const TextureFilter::Filter&);
};
#endif
