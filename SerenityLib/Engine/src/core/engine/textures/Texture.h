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

        static void LoadDDSFile(Texture& texture, std::string filename,epriv::ImageLoadedStructure& image);

        static void LoadTexture2DIntoOpenGL(Texture& texture);
        static void LoadTextureFramebufferIntoOpenGL(Texture& texture);
        static void LoadTextureCubemapIntoOpenGL(Texture& texture);

        static void EnumWrapToGL(uint& gl, TextureWrap::Wrap& wrap);
        static void EnumFilterToGL(uint& gl, TextureFilter::Filter& filter,bool min);
        static bool IsCompressedType(ImageInternalFormat::Format);

        static void GenerateMipmapsOpenGL(Texture& texture);
        static void WithdrawPixelsFromOpenGLMemory(Texture& texture,uint imageIndex = 0,uint mipmapLevel = 0);
        static void ChoosePixelFormat(ImagePixelFormat::Format& outPxlFormat,ImageInternalFormat::Format& inInternalFormat);
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
        static Texture *White, *Black; //loaded in renderer
    private:
        class impl; std::unique_ptr<impl> m_i;
    public:
        //Framebuffer
        Texture(uint renderTgtWidth,uint renderTgtHeight,ImagePixelType::Type,ImagePixelFormat::Format,ImageInternalFormat::Format,float divisor = 1.0f);
        //Single File
        Texture(std::string filename,bool genMipmaps = true,ImageInternalFormat::Format = ImageInternalFormat::Format::SRGB8_ALPHA8,GLuint openglTexType = GL_TEXTURE_2D);
        //Pixels From Memory
        Texture(const sf::Image& sfmlImage,std::string name = "CustomTexture",bool genMipmaps = false,ImageInternalFormat::Format = ImageInternalFormat::Format::SRGB8_ALPHA8,GLuint openglTexType = GL_TEXTURE_2D);
        //Cubemap from 6 files
        Texture(std::string files[],std::string name = "Cubemap",bool genMipmaps = false,ImageInternalFormat::Format = ImageInternalFormat::Format::SRGB8_ALPHA8);
        virtual ~Texture();

        uchar* pixels();
        GLuint& address(uint = 0);
        GLuint type();
        uint width();
        uint height();
        uint numAddresses();
        bool mipmapped();
        bool compressed();
        void setAnisotropicFiltering(float aniso);
        void resize(Engine::epriv::FramebufferObject&,uint width,uint height);

        ImageInternalFormat::Format internalFormat();
        ImagePixelFormat::Format pixelFormat();
        ImagePixelType::Type pixelType();

        virtual void load();
        virtual void unload();

        void setXWrapping(TextureWrap::Wrap);
        void setYWrapping(TextureWrap::Wrap);
        void setZWrapping(TextureWrap::Wrap);
        void setWrapping(TextureWrap::Wrap);

        void setMinFilter(TextureFilter::Filter);
        void setMaxFilter(TextureFilter::Filter);
        void setFilter(TextureFilter::Filter);

        static void setXWrapping(GLuint type,TextureWrap::Wrap);
        static void setYWrapping(GLuint type,TextureWrap::Wrap);
        static void setZWrapping(GLuint type,TextureWrap::Wrap);
        static void setWrapping(GLuint type,TextureWrap::Wrap);

        static void setMinFilter(GLuint type,TextureFilter::Filter);
        static void setMaxFilter(GLuint type,TextureFilter::Filter);
        static void setFilter(GLuint type,TextureFilter::Filter);

        void render(glm::vec2 pos, glm::vec4 color,float angle = 0.0f, glm::vec2 scl = glm::vec2(1.0f,1.0f), float depth = 0.1f);

        void genPBREnvMapData(uint convoludeTextureSize,uint preEnvFilterSize);
};
#endif