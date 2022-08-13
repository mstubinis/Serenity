
#include <serenity/resources/texture/Texture.h>
#include <serenity/resources/texture/TextureCubemap.h>
#include <serenity/resources/texture/TextureLoader.h>
#include <serenity/resources/texture/DDS.h>
#include <serenity/math/Engine_Math.h>
#include <serenity/system/Engine.h>
#include <serenity/renderer/FramebufferObject.h>
#include <serenity/events/Event.h>
#include <serenity/resources/texture/TextureRequest.h>

#include <serenity/renderer/opengl/APIStateOpenGL.h>
#include <serenity/resources/texture/DDS.h>

#include <fstream>
#include <filesystem>

using namespace Engine::priv;

namespace Engine::priv {
    void TextureLoader::LoadTexture2DIntoOpenGL(Texture& texture) {
        Engine::Renderer::bindTextureForModification(texture.getTextureType(), texture.m_TextureAddress);
        for (const auto& mipmap : texture.m_CPUData.m_ImagesDatas[0].m_Mipmaps) {
            Engine::opengl::createTexImage2D(texture, mipmap, texture.getTextureType());
            //TextureLoader::WithdrawPixelsFromOpenGLMemory(texture, 0, mipmap.level);
        }
        texture.setFilter(TextureFilter::Linear);
        texture.setWrapping(TextureWrap::Repeat);
    }
    void TextureLoader::LoadTextureFramebufferIntoOpenGL(Texture& texture) {
        Engine::Renderer::bindTextureForModification(texture.getTextureType(), texture.address());
        const auto& image = texture.m_CPUData.m_ImagesDatas[0];
        glTexImage2D(texture.getTextureType().toGLType(), 0, image.m_InternalFormat, image.m_Mipmaps[0].width, image.m_Mipmaps[0].height, 0, image.m_PixelFormat, image.m_PixelType, nullptr);
        texture.setFilter(TextureFilter::Linear);
        texture.setWrapping(TextureWrap::ClampToEdge);
    }
    void TextureLoader::LoadTextureCubemapIntoOpenGL(Texture& texture) {
        Engine::Renderer::bindTextureForModification(texture.getTextureType(), texture.address());
        uint32_t imageIndex = 0;
        for (const auto& image : texture.m_CPUData.m_ImagesDatas) {
            for (const auto& mipmap : image.m_Mipmaps) {
                Engine::opengl::createTexImage2D(texture, mipmap, TextureType::CubeMap_X_Pos + imageIndex);
            }
            ++imageIndex;
        }
        texture.setFilter(TextureFilter::Linear);
        texture.setWrapping(TextureWrap::ClampToEdge);
    }
    void TextureLoader::MirrorPixelsVertically(uint8_t* pixels, uint32_t width, uint32_t height, uint32_t colorsPerPixel) {
        auto swapPixels = [width, pixels, colorsPerPixel](uint32_t x, uint32_t y, uint32_t x2, uint32_t y2) {
            std::array<uint8_t, 4> temp;
            auto getIdx = [width, colorsPerPixel](uint32_t x, uint32_t y) {
                return ((x * width) + y) * colorsPerPixel;
            };
            for (uint32_t i = 0; i < colorsPerPixel; ++i) {
                temp[i] = pixels[getIdx(x, y) + i];
            }
            for (uint32_t i = 0; i < colorsPerPixel; ++i) {
                pixels[getIdx(x, y) + i] = pixels[getIdx(x2, y2) + i];
            }
            for (uint32_t i = 0; i < colorsPerPixel; ++i) {
                pixels[getIdx(x2, y2) + i] = temp[i];
            }
        };
        //mirror pixels along y-axis. this is a sfml needed thing
        for (uint32_t row = 0; row < height / 2; ++row) {
            for (uint32_t col = 0; col < width; ++col) {
                swapPixels(row, col, height - row - 1, col);
            }
        }
    }
    void TextureLoader::LoadCPU(TextureCPUData& cpuData, Handle inHandle) {
        for (auto& imageData : cpuData.m_ImagesDatas) {
            if (!imageData.m_Filename.empty()) {
                if (imageData.hasBlankMipmap()) {
                    const std::string extension = std::filesystem::path(imageData.m_Filename).extension().string();
                    if (extension == ".dds") {
                        Engine::priv::LoadDDSFile(cpuData, imageData);
                    } else {
                        sfImageLoader sfImage(imageData.m_Filename.c_str());
                        imageData.load(sfImage.getPixels(), sfImage.getWidth(), sfImage.getHeight(), imageData.m_Filename);
                    }
                }
            }
        }
        if (!inHandle.null()) {
            std::scoped_lock lock(*inHandle.getMutex());
            inHandle.get<Texture>()->m_CPUData = std::move(cpuData);
        }
    }
    void TextureLoader::LoadGPU(Handle textureHandle, bool dispatchEventLoaded) {
        LoadGPU(*textureHandle.get<Texture>(), dispatchEventLoaded);
    }
    void TextureLoader::LoadGPU(Texture& texture, bool dispatchEventLoaded) {
        Engine::opengl::genTexture(texture);
        Engine::opengl::bindTexture(texture);
        switch (texture.getTextureType()) {
            case TextureType::RenderTarget: {
                TextureLoader::LoadTextureFramebufferIntoOpenGL(texture);
                break;
            } case TextureType::Texture1D: {
                break;
            } case TextureType::Texture2D: {
                TextureLoader::LoadTexture2DIntoOpenGL(texture);
                break;
            } case TextureType::Texture3D: {
                break;
            } case TextureType::CubeMap: { // TODO: remove completely and make sure it is TextureCubemap class instead
                assert(false);
                TextureLoader::LoadTextureCubemapIntoOpenGL(texture);
                break;
            }
        }
        if (texture.m_CPUData.m_IsToBeMipmapped) {
            texture.generateMipmaps();
        }
#ifdef TEXTURE_COMMAND_QUEUE
        while (!texture.m_CommandQueue.empty()) {
            texture.m_CommandQueue.front()();
            texture.m_CommandQueue.pop();
        }
#endif
        texture.Resource::load(dispatchEventLoaded);
    }

    void TextureLoader::UnloadGPU(Texture& texture, bool dispatchEventLoaded) {
        Engine::opengl::deleteTexture(texture);
        texture.Resource::unload(dispatchEventLoaded);
    }


    void TextureLoader::Resize(Texture& texture, Engine::priv::FramebufferObject& fbo, int width, int height) {
        if (texture.getTextureType() != TextureType::RenderTarget) {
            ENGINE_PRODUCTION_LOG(__FUNCTION__ << "() error: Non-framebuffer texture cannot be resized. Returning...");
            return;
        }
        Engine::Renderer::bindTextureForModification(texture.getTextureType(), texture.address());
        const float divisor = fbo.divisor();
        const int fboWidth = int(float(width) * divisor);
        const int fboHeight = int(float(height) * divisor);
        auto& imageData = texture.m_CPUData.m_ImagesDatas[0];
        imageData.m_Mipmaps[0].width = fboWidth;
        imageData.m_Mipmaps[0].height = fboHeight;
        glTexImage2D(texture.getTextureType().toGLType(), 0, imageData.m_InternalFormat, fboWidth, fboHeight, 0, imageData.m_PixelFormat, imageData.m_PixelType, nullptr);
    }
}