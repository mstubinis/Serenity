#pragma once
#ifndef ENGINE_TEXTURES_TEXTURE_LOADER_H
#define ENGINE_TEXTURES_TEXTURE_LOADER_H

#include <string>
#include <core/engine/textures/TextureIncludes.h>

namespace Engine {
    namespace epriv {
        struct TextureLoader final {
            friend class Texture;

            static void LoadDDSFile(Texture& texture, const std::string& filename, epriv::ImageLoadedStructure& image);

            static void LoadTexture2DIntoOpenGL(Texture& texture);
            static void LoadTextureFramebufferIntoOpenGL(Texture& texture);
            static void LoadTextureCubemapIntoOpenGL(Texture& texture);

            static void EnumWrapToGL(unsigned int& gl, const TextureWrap::Wrap& wrap);
            static void EnumFilterToGL(unsigned int& gl, const TextureFilter::Filter& filter, const bool& min);
            static const bool IsCompressedType(const ImageInternalFormat::Format&);

            static void GenerateMipmapsOpenGL(Texture& texture);
            static void WithdrawPixelsFromOpenGLMemory(Texture& texture, const uint& imageIndex = 0, const uint & mipmapLevel = 0);
            static void ChoosePixelFormat(ImagePixelFormat::Format& outPxlFormat, const ImageInternalFormat::Format& inInternalFormat);

            static void GeneratePBRData(Texture&, const unsigned int& convoludeTextureSize, const unsigned int& preEnvFilterSize);
            static void ImportIntoOpengl(Texture&, const Engine::epriv::ImageMipmap& mipmap, const GLuint& openGLType);
            static void InitCommon(Texture&, const GLuint& openglTextureType, const bool& toBeMipmapped);


            static void InitFramebuffer(Texture&, const uint& w, const uint& h, const ImagePixelType::Type& pxlType, const ImagePixelFormat::Format& pxlFormat, const ImageInternalFormat::Format& _internal, const float& divisor);
            static void InitFromMemory(Texture&, const sf::Image& sfImage, const std::string& name, const bool& genMipMaps, const ImageInternalFormat::Format& _internal, const GLuint& openglTextureType);
            static void InitFromFile(Texture&, const std::string& filename, const bool& genMipMaps, const ImageInternalFormat::Format& _internal, const GLuint& openglTextureType);
            static void InitFromFilesCubemap(Texture&, const std::string files[], const std::string& name, const bool& genMipMaps, const ImageInternalFormat::Format& _internal);
        };
        struct InternalTexturePublicInterface final {
            static void LoadCPU(Texture&);
            static void LoadGPU(Texture&);
            static void UnloadCPU(Texture&);
            static void UnloadGPU(Texture&);
            static void Load(Texture&);
            static void Unload(Texture&);

            static void Resize(Texture& texture, Engine::epriv::FramebufferObject&, const uint& width, const uint& height);
        };
    };
};

#endif