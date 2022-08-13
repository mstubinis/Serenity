#pragma once
#ifndef ENGINE_TEXTURE_CUBEMAP_H
#define ENGINE_TEXTURE_CUBEMAP_H

struct TextureRequest;
namespace Engine::priv {
    class GBuffer;
    template<typename T> class ResourceVector;
}
namespace Engine::priv::detail::opengl {
    class Impl;
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
        bool                                   m_Mipmapped       = false;
        bool                                   m_IsToBeMipmapped = false;

        void initFromFile();
        void initFromCubemap(const std::array<std::string_view, 6>& files, ImageInternalFormat intFmt);
    };
};

class TextureCubemap final : public Resource<TextureCubemap>, public Engine::priv::TextureBaseClass {
    public:
        class Impl;
    friend class  Engine::priv::ResourceVector<TextureCubemap>;
    friend class  Engine::priv::GBuffer;
    friend class  Engine::priv::TextureLoaderCubemap;
    friend struct TextureRequest;
    friend class  Engine::priv::detail::opengl::Impl;
    friend class  TextureCubemap::Impl;
    private:
        Engine::priv::TextureCubemapCPUData         m_CPUData;
        Handle                                      m_ConvolutionTextureHandle = {};
        Handle                                      m_PreEnvTextureHandle      = {};
    public:
        //Empty Texture
        TextureCubemap(std::string_view textureName = {}, bool mipMap = false);
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

        [[nodiscard]] const uint8_t* pixels();
        [[nodiscard]] inline int width() const noexcept {
            assert(!m_CPUData.m_ImagesDatas.empty());
            assert(!m_CPUData.m_ImagesDatas[0].m_Mipmaps.empty());
            return m_CPUData.m_ImagesDatas[0].m_Mipmaps[0].width;
        }
        [[nodiscard]] inline int height() const noexcept {
            assert(!m_CPUData.m_ImagesDatas.empty());
            assert(!m_CPUData.m_ImagesDatas[0].m_Mipmaps.empty());
            return m_CPUData.m_ImagesDatas[0].m_Mipmaps[0].height;
        }
        [[nodiscard]] inline glm::uvec2 size() const noexcept { return glm::uvec2(width(), height()); }
        [[nodiscard]] inline glm::vec2 sizeAsRatio() const noexcept { const auto size_ = glm::vec2(size()); return glm::vec2(size_ / glm::max(size_.x, size_.y)); }
        [[nodiscard]] inline constexpr bool mipmapped() const noexcept { return m_CPUData.m_Mipmapped; }
        [[nodiscard]] bool compressed() const {
            assert(!m_CPUData.m_ImagesDatas.empty() && !m_CPUData.m_ImagesDatas[0].m_Mipmaps.empty());
            return m_CPUData.m_ImagesDatas[0].m_Mipmaps[0].compressedSize > 0;
        }
        void setAnisotropicFiltering(float anisotropicFiltering);

        [[nodiscard]] inline ImageInternalFormat internalFormat() const noexcept {
            assert(!m_CPUData.m_ImagesDatas.empty());
            return m_CPUData.m_ImagesDatas[0].m_InternalFormat;
        }
        [[nodiscard]] inline ImagePixelFormat pixelFormat() const noexcept {
            assert(!m_CPUData.m_ImagesDatas.empty());
            return m_CPUData.m_ImagesDatas[0].m_PixelFormat;
        }
        [[nodiscard]] inline ImagePixelType pixelType() const noexcept {
            assert(!m_CPUData.m_ImagesDatas.empty());
            return m_CPUData.m_ImagesDatas[0].m_PixelType;
        }
        //does not include the base level image
        int getMaxMipmapLevelsPossible() const noexcept {
            const auto sz = size();
            return glm::max(0, (int)glm::floor(glm::log2(glm::max(sz.x, sz.y))));
        }

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
