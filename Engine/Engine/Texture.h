#pragma once
#ifndef ENGINE_TEXTURE_H
#define ENGINE_TEXTURE_H

#include "Engine_ResourceBasic.h"
#include <glm/fwd.hpp>
#include "GLImageConstants.h"
#include <GL/glew.h>
#include <SFML/OpenGL.hpp>
#include <memory>

typedef unsigned int uint;
typedef unsigned char uchar;
typedef unsigned short ushort;

class Texture;
class TextureWrap{public: enum Wrap{
    Repeat,RepeatMirrored,ClampToEdge,ClampToBorder,
};};
class TextureFilter{public: enum Filter{
    Linear,Nearest,Nearest_Mipmap_Nearest,Nearest_Mipmap_Linear,Linear_Mipmap_Nearest,Linear_Mipmap_Linear,
};};

namespace sf{ class Image; }
namespace Engine{
    namespace epriv{
		struct ImageMipmap;
		struct ImageLoadedStructure;
        class FramebufferTexture;
        class TextureLoader final{
			friend class ::Texture;
            public:

				static void LoadDDSFile(Texture* texture, std::string filename,epriv::ImageLoadedStructure& image);

                static void LoadTexture2DIntoOpenGL(Texture* texture);
				static void LoadTextureFramebufferIntoOpenGL(Texture* texture);
                static void LoadTextureCubemapIntoOpenGL(Texture* texture);

                static void EnumWrapToGL(uint& gl, TextureWrap::Wrap& wrap);
                static void EnumFilterToGL(uint& gl, TextureFilter::Filter& filter,bool min);
				static bool IsCompressedType(ImageInternalFormat::Format);

				static void GenerateMipmapsOpenGL(Texture* texture);
				static void WithdrawPixelsFromOpenGLMemory(Texture* texture);
				static void ChoosePixelFormat(ImagePixelFormat::Format& outPxlFormat,ImageInternalFormat::Format& inInternalFormat);
        };

    };
};
class Texture: public EngineResource{
	friend class Engine::epriv::TextureLoader;
    private:
        class impl; std::unique_ptr<impl> m_i;
    public:
		//Framebuffer
        Texture(uint renderTgtWidth,uint renderTgtHeight,ImagePixelType::Type,ImagePixelFormat::Format,ImageInternalFormat::Format,float divisor = 1.0f);
		//Single File
        Texture(std::string filename,GLuint openglTexType = GL_TEXTURE_2D,bool genMipmaps = true,ImageInternalFormat::Format = ImageInternalFormat::Format::SRGB8_ALPHA8);
		//Pixels From Memory
        Texture(const sf::Image& sfmlImage,std::string name = "CustomTexture",GLuint openglTexType = GL_TEXTURE_2D,bool genMipmaps = true,ImageInternalFormat::Format = ImageInternalFormat::Format::SRGB8_ALPHA8);
		//Cubemap from 6 files
        Texture(std::string files[],std::string name = "Cubemap",bool genMipmaps = true,ImageInternalFormat::Format = ImageInternalFormat::Format::SRGB8_ALPHA8);
        virtual ~Texture();

        uchar* pixels();
        GLuint& address();
        GLuint& address(uint);
        GLuint type();
        uint width();
        uint height();
        uint numAddresses();
        ushort mipmapLevels();
        bool mipmapped();
        void resize(Engine::epriv::FramebufferTexture*,uint width,uint height);

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

        void render(glm::vec2& pos, glm::vec4& color,float angle, glm::vec2& scl, float depth);

        void genPBREnvMapData(uint convoludeTextureSize,uint preEnvFilterSize);
};
#endif
