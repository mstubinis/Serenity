
#include <serenity/resources/texture/TextureCubemap.h>
#include <serenity/resources/texture/TextureLoaderCubemap.h>
#include <serenity/resources/texture/DDS.h>
#include <serenity/math/Engine_Math.h>
#include <serenity/renderer/FramebufferObject.h>
#include <serenity/events/Event.h>

#include <serenity/renderer/opengl/APIStateOpenGL.h>
#include <serenity/resources/texture/DDS.h>

#include <serenity/system/EngineIncludes.h>
#include <serenity/resources/Engine_Resources.h>

#include <fstream>
#include <filesystem>

namespace Engine::priv {
    void TextureLoaderCubemap::GeneratePBRData(TextureCubemap& cubemap, int convoludeTextureSize, int preEnvFilterSize) {
        auto cubemapConvolution            = Engine::getResourceManager().m_ResourceModule.get<TextureCubemap>(cubemap.name() + "Convolution");
        auto cubemapPreEnvFilter           = Engine::getResourceManager().m_ResourceModule.get<TextureCubemap>(cubemap.name() + "PreEnvFilter");
        cubemap.m_ConvolutionTextureHandle = cubemapConvolution.m_Handle;
        cubemap.m_PreEnvTextureHandle      = cubemapPreEnvFilter.m_Handle;
        if (!cubemapConvolution.m_Resource) {
            cubemap.m_ConvolutionTextureHandle = Engine::Resources::addResource<TextureCubemap>(cubemap.name() + "Convolution", false);
            cubemapConvolution.m_Resource      = cubemap.m_ConvolutionTextureHandle.get<TextureCubemap>();
            Engine::opengl::genTexture(*cubemapConvolution.m_Resource);
            Engine::opengl::bindTexture(*cubemapConvolution.m_Resource);
            for (uint32_t i = 0; i < 6; ++i) {
                glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, convoludeTextureSize, convoludeTextureSize, 0, GL_RGB, GL_FLOAT, nullptr);
            }
            cubemapConvolution.m_Resource->setWrapping(TextureWrap::ClampToEdge);
            cubemapConvolution.m_Resource->setFilter(TextureFilter::Linear);
        }
        if (!cubemapPreEnvFilter.m_Resource) {
            cubemap.m_PreEnvTextureHandle  = Engine::Resources::addResource<TextureCubemap>(cubemap.name() + "PreEnvFilter", false);
            cubemapPreEnvFilter.m_Resource = cubemap.m_PreEnvTextureHandle.get<TextureCubemap>();
            Engine::opengl::genTexture(*cubemapPreEnvFilter.m_Resource);
            Engine::opengl::bindTexture(*cubemapPreEnvFilter.m_Resource);
            for (uint32_t i = 0; i < 6; ++i) {
                glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, preEnvFilterSize, preEnvFilterSize, 0, GL_RGB, GL_FLOAT, nullptr);
            }
            cubemapPreEnvFilter.m_Resource->setWrapping(TextureWrap::ClampToEdge);
            cubemapPreEnvFilter.m_Resource->setMinFilter(TextureFilter::Linear_Mipmap_Linear);
            cubemapPreEnvFilter.m_Resource->setMaxFilter(TextureFilter::Linear);
            //glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
            Engine::opengl::generateMipmaps(*cubemapPreEnvFilter.m_Resource);
        }
        Engine::getRenderer()._genPBREnvMapData(cubemap, cubemap.m_ConvolutionTextureHandle, cubemap.m_PreEnvTextureHandle, convoludeTextureSize, preEnvFilterSize);
    }
    void TextureLoaderCubemap::LoadCPU(TextureCubemapCPUData& cpuData, Handle inHandle) {
        for (auto& imageData : cpuData.m_ImagesDatas) {
            if (!imageData.m_Filename.empty() && imageData.hasBlankMipmap()) {
                const std::string extension = std::filesystem::path(imageData.m_Filename).extension().string();
                if (extension == ".dds") {
                    Engine::priv::LoadDDSFile(cpuData, imageData);
                } else {
                    sfImageLoader sfImage(imageData.m_Filename.c_str());
                    imageData.load(sfImage.getPixels(), sfImage.getWidth(), sfImage.getHeight(), imageData.m_Filename);
                }
            }
        }
        if (!inHandle.null()) {
            //std::scoped_lock lock{ inHandle };
            inHandle.get<TextureCubemap>()->m_CPUData = std::move(cpuData);
        }
    }
    void TextureLoaderCubemap::LoadGPU(Handle textureHandle) {
        LoadGPU(*textureHandle.get<TextureCubemap>());
    }
    void TextureLoaderCubemap::LoadGPU(TextureCubemap& cubemap) {
        Engine::opengl::genTexture(cubemap);
        Engine::opengl::bindTexture(cubemap);
        for (size_t imageIndex = 0; imageIndex != cubemap.m_CPUData.m_ImagesDatas.size(); ++imageIndex) {
            for (const auto& mipmap : cubemap.m_CPUData.m_ImagesDatas[imageIndex].m_Mipmaps) {
                Engine::opengl::createTexImage2D(cubemap, mipmap, TextureType::CubeMap_X_Pos + imageIndex);
            }
        }
        cubemap.setFilter(TextureFilter::Linear);
        cubemap.setWrapping(TextureWrap::ClampToEdge);



        if (cubemap.m_CPUData.m_IsToBeMipmapped) {
            cubemap.generateMipmaps();
        }
#ifdef TEXTURE_COMMAND_QUEUE
        while (!cubemap.m_CommandQueue.empty()) {
            cubemap.m_CommandQueue.front()();
            cubemap.m_CommandQueue.pop();
        }
#endif
        cubemap.Resource::load();
    }

    void TextureLoaderCubemap::UnloadGPU(TextureCubemap& cubemap) {
        Engine::opengl::deleteTexture(cubemap);
        cubemap.Resource::unload();
    }
}