#pragma once
#ifndef ENGINE_TEXTURE_INCLUDE_GUARD
#define ENGINE_TEXTURE_INCLUDE_GUARD

#include <core/engine/textures/TextureIncludes.h>
#include <core/engine/resources/Engine_ResourceBasic.h>
#include <core/engine/renderer/GLImageConstants.h>

#include <glm/fwd.hpp>
#include <GL/glew.h>
#include <SFML/OpenGL.hpp>

namespace Engine{
namespace epriv{
    struct TextureLoader final{
        friend class ::Texture;

        static void LoadDDSFile(Texture& texture, const std::string& filename, epriv::ImageLoadedStructure& image);

        static void LoadTexture2DIntoOpenGL(Texture& texture);
        static void LoadTextureFramebufferIntoOpenGL(Texture& texture);
        static void LoadTextureCubemapIntoOpenGL(Texture& texture);

        static void EnumWrapToGL(uint& gl, const TextureWrap::Wrap& wrap);
        static void EnumFilterToGL(uint& gl, const TextureFilter::Filter& filter, const bool& min);
        static const bool IsCompressedType(const ImageInternalFormat::Format&);

        static void GenerateMipmapsOpenGL(Texture& texture);
        static void WithdrawPixelsFromOpenGLMemory(Texture& texture, const uint& imageIndex = 0, const uint& mipmapLevel = 0);
        static void ChoosePixelFormat(ImagePixelFormat::Format& outPxlFormat, const ImageInternalFormat::Format& inInternalFormat);
    };
    struct InternalTexturePublicInterface final {
        static void LoadCPU(Texture&);
        static void LoadGPU(Texture&);
        static void UnloadCPU(Texture&);
        static void UnloadGPU(Texture&);
    };
};
};

class Texture: public EngineResource{
    friend struct Engine::epriv::TextureLoader;
    friend struct Engine::epriv::InternalTexturePublicInterface;

    public:
        static Texture *White, *Black, *Checkers; //loaded in renderer
    private:
        class impl; std::unique_ptr<impl> m_i;
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

        const uchar* pixels();
        const GLuint& address(const uint& index = 0) const;
        const GLuint type() const;
        const uint width() const;
        const uint height() const;
        const uint numAddresses() const;
        const bool mipmapped() const;
        const bool compressed() const;
        void setAnisotropicFiltering(const float& anisotropicFiltering);
        void resize(Engine::epriv::FramebufferObject&, const uint& width, const uint& height);

        const ImageInternalFormat::Format internalFormat() const;
        const ImagePixelFormat::Format pixelFormat() const;
        const ImagePixelType::Type pixelType() const;

        virtual void load();
        virtual void unload();

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

        void render(
            const glm::vec2& pos,
            const glm::vec4& color,
            const float& angle = 0.0f,
            const glm::vec2& scale = glm::vec2(1.0f),
            const float& depth = 0.1f
        );

        void genPBREnvMapData(const uint& convoludeTextureSize, const uint& preEnvFilterSize);
};
#endif
