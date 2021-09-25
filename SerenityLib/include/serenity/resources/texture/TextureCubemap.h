#pragma once
#ifndef ENGINE_TEXTURE_CUBEMAP_H
#define ENGINE_TEXTURE_CUBEMAP_H

struct TextureRequest;
namespace Engine::priv {
    class GBuffer;
    template<typename T> class ResourceVector;
}

#include <serenity/resources/Handle.h>
#include <serenity/resources/texture/TextureBaseClass.h>
#include <serenity/resources/texture/TextureLoaderCubemap.h>
#include <serenity/resources/Resource.h>
#include <serenity/dependencies/glm.h>

#include <vector>
#include <string>

namespace Engine::priv {
    struct TextureCubemapCPUData final {
        std::vector<Engine::priv::ImageData>   m_ImagesDatas     = { Engine::priv::ImageData {} };
        std::string                            m_Name;
        uint32_t                               m_MinFilter       = GL_LINEAR; //used to determine filter type for mipmaps
        bool                                   m_Mipmapped       = false;
        bool                                   m_IsToBeMipmapped = false;

        void initFromFile();
        void initFromCubemap(const std::array<std::string_view, 6>& files, ImageInternalFormat intFmt);
    };
};

class TextureCubemap final : public Resource<TextureCubemap>, public Engine::priv::TextureBaseClass {
    friend class  Engine::priv::ResourceVector<TextureCubemap>;
    friend class  Engine::priv::GBuffer;
    friend class  Engine::priv::TextureLoaderCubemap;
    friend struct TextureRequest;
    private:
        Engine::priv::TextureCubemapCPUData         m_CPUData;
        Handle                                      m_ConvolutionTextureHandle = {};
        Handle                                      m_PreEnvTextureHandle      = {};
    public:
        //Empty Texture
        TextureCubemap(std::string_view textureName = "", bool mipMap = false);
        //Single File
        TextureCubemap(std::string_view filename, bool generateMipmaps, ImageInternalFormat);
        //Cubemap from 6 files
        TextureCubemap(const std::array<std::string_view, 6>& files, std::string_view textureName, bool generateMipmaps, ImageInternalFormat);

        TextureCubemap(const TextureCubemap&)                 = delete;
        TextureCubemap& operator=(const TextureCubemap&)      = delete;
        TextureCubemap(TextureCubemap&&) noexcept;
        TextureCubemap& operator=(TextureCubemap&&) noexcept;
        ~TextureCubemap();

        inline bool operator==(bool rhs) const noexcept { return (rhs == true && m_TextureAddress != 0); }
        explicit inline operator bool() const noexcept { return m_TextureAddress != 0; }

        [[nodiscard]] inline bool hasGlobalIlluminationData() const noexcept { return (bool)(!m_ConvolutionTextureHandle.null() && !m_PreEnvTextureHandle.null()); }

        [[nodiscard]] inline TextureType getTextureType() const noexcept { return TextureType::CubeMap; }
        [[nodiscard]] inline Handle getConvolutionTexture() const noexcept { return m_ConvolutionTextureHandle; }
        [[nodiscard]] inline Handle getPreEnvTexture() const noexcept { return m_PreEnvTextureHandle; }

        [[nodiscard]] uint8_t* pixels();
        [[nodiscard]] inline int width() const noexcept {
            ASSERT(m_CPUData.m_ImagesDatas.size() > 0, __FUNCTION__ << "(): m_CPUData.m_ImagesDatas.size() was 0!");
            ASSERT(m_CPUData.m_ImagesDatas[0].m_Mipmaps.size() > 0, __FUNCTION__ << "(): m_CPUData.m_ImagesDatas[0].m_Mipmaps.size() was 0!");
            return m_CPUData.m_ImagesDatas[0].m_Mipmaps[0].width;
        }
        [[nodiscard]] inline int height() const noexcept {
            ASSERT(m_CPUData.m_ImagesDatas.size() > 0, __FUNCTION__ << "(): m_CPUData.m_ImagesDatas.size() was 0!");
            ASSERT(m_CPUData.m_ImagesDatas[0].m_Mipmaps.size() > 0, __FUNCTION__ << "(): m_CPUData.m_ImagesDatas[0].m_Mipmaps.size() was 0!");
            return m_CPUData.m_ImagesDatas[0].m_Mipmaps[0].height;
        }
        [[nodiscard]] inline glm::uvec2 size() const noexcept { return glm::uvec2(width(), height()); }
        [[nodiscard]] inline glm::vec2 sizeAsRatio() const noexcept { const auto size_ = glm::vec2(size()); return glm::vec2(size_ / glm::max(size_.x, size_.y)); }
        [[nodiscard]] inline constexpr bool mipmapped() const noexcept { return m_CPUData.m_Mipmapped; }
        [[nodiscard]] bool compressed() const;
        void setAnisotropicFiltering(float anisotropicFiltering);

        [[nodiscard]] inline ImageInternalFormat internalFormat() const noexcept {
            ASSERT(m_CPUData.m_ImagesDatas.size() > 0, __FUNCTION__ << "(): m_CPUData.m_ImagesDatas.size() was 0!");
            return m_CPUData.m_ImagesDatas[0].m_InternalFormat;
        }
        [[nodiscard]] inline ImagePixelFormat pixelFormat() const noexcept {
            ASSERT(m_CPUData.m_ImagesDatas.size() > 0, __FUNCTION__ << "(): m_CPUData.m_ImagesDatas.size() was 0!");
            return m_CPUData.m_ImagesDatas[0].m_PixelFormat;
        }
        [[nodiscard]] inline ImagePixelType pixelType() const noexcept {
            ASSERT(m_CPUData.m_ImagesDatas.size() > 0, __FUNCTION__ << "(): m_CPUData.m_ImagesDatas.size() was 0!");
            return m_CPUData.m_ImagesDatas[0].m_PixelType;
        }
        //does not include the base level image
        int getMaxMipmapLevelsPossible() const noexcept;

        bool generateMipmaps();

        void setXWrapping(TextureWrap);
        void setYWrapping(TextureWrap);
        void setZWrapping(TextureWrap);
        void setWrapping(TextureWrap);

        void setMinFilter(TextureFilter);
        void setMaxFilter(TextureFilter);
        void setFilter(TextureFilter);
};
#endif
