#pragma once
#ifndef ENGINE_TEXTURE_H
#define ENGINE_TEXTURE_H

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
#include <serenity/resources/texture/TextureLoader.h>
#include <serenity/resources/Resource.h>
#include <serenity/dependencies/glm.h>

#include <vector>
#include <string>

namespace Engine::priv {
    struct TextureCPUData final {
        std::vector<Engine::priv::ImageData>   m_ImagesDatas     = { Engine::priv::ImageData {} };
        std::string                            m_Name;
        TextureType                            m_TextureType     = TextureType::Unknown;
        bool                                   m_Mipmapped       = false;
        bool                                   m_IsToBeMipmapped = false; //TODO: move somewhere else or try to get rid of altogether?

        void initFromMemory(const uint8_t* pixels, int inWidth, int inHeight);
        void initFromFile();
    };
};

class Texture final : public Resource<Texture>, public Engine::priv::TextureBaseClass {
    public:
        class Impl;
    friend class  Engine::priv::ResourceVector<Texture>;
    friend class  Engine::priv::GBuffer;
    friend class  Engine::priv::TextureLoader;
    friend class  Engine::priv::detail::opengl::Impl;
    friend struct TextureRequest;
    friend class  Texture::Impl;
    public:
        static Handle White, Black, Checkers, BRDF; //loaded in renderer. TODO: move these to built in class (separate from client side interface)
    private:
        Engine::priv::TextureCPUData    m_CPUData;
    public:
        //From detailed constructor
        Texture(const TextureConstructorInfo&, bool dispatchEventLoaded = true);
        //Empty Texture
        Texture(std::string_view textureName = {}, TextureType = TextureType::Texture2D, bool mipMap = false, bool dispatchEventLoaded = true);
        //Framebuffer render target
        Texture(uint32_t renderTgtWidth, uint32_t renderTgtHeight, ImagePixelType, ImagePixelFormat, ImageInternalFormat, float divisor = 1.0f, bool dispatchEventLoaded = true);
        //Single File
        Texture(std::string_view filename, bool generateMipmaps, ImageInternalFormat, TextureType, bool dispatchEventLoaded = true);
        //Pixels From Memory
        Texture(uint8_t* pixels, uint32_t width, uint32_t height, std::string_view textureName, bool generateMipmaps, ImageInternalFormat, TextureType, bool dispatchEventLoaded = true);

        Texture(const Texture&)                = delete;
        Texture& operator=(const Texture&)     = delete;
        Texture(Texture&&) noexcept;
        Texture& operator=(Texture&&) noexcept;
        ~Texture();

        inline bool operator==(bool rhs) const noexcept { return (rhs == true && m_TextureAddress != 0); }
        explicit inline operator bool() const noexcept { return m_TextureAddress != 0; }

        [[nodiscard]] const uint8_t* pixels();
        [[nodiscard]] inline TextureType getTextureType() const noexcept { return m_CPUData.m_TextureType; }
        [[nodiscard]] inline int width() const noexcept {
            assert(!m_CPUData.m_ImagesDatas.empty() && !m_CPUData.m_ImagesDatas[0].m_Mipmaps.empty());
            return m_CPUData.m_ImagesDatas[0].m_Mipmaps[0].width; 
        }
        [[nodiscard]] inline int height() const noexcept {
            assert(!m_CPUData.m_ImagesDatas.empty() && !m_CPUData.m_ImagesDatas[0].m_Mipmaps.empty());
            return m_CPUData.m_ImagesDatas[0].m_Mipmaps[0].height; 
        }
        [[nodiscard]] inline glm::uvec2 size() const noexcept { return glm::uvec2{ width(), height() }; }
        [[nodiscard]] inline glm::vec2 sizeAsRatio() const noexcept { const auto size_ = glm::vec2{ size() }; return glm::vec2(size_ / glm::max(size_.x, size_.y)); }
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
            return glm::max(0, static_cast<int>(glm::floor(glm::log2(glm::max(sz.x, sz.y)))));
        }

        bool generateMipmaps();

        void setXWrapping(TextureWrap);
        void setYWrapping(TextureWrap);
        void setWrapping(TextureWrap);

        void setMinFilter(TextureFilter);
        void setMaxFilter(TextureFilter);
        void setFilter(TextureFilter);
};
#endif
