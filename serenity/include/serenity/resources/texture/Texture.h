#pragma once
#ifndef ENGINE_TEXTURE_H
#define ENGINE_TEXTURE_H

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
        
        TextureCPUData() = default;

        TextureCPUData(const TextureCPUData&) = default;
        TextureCPUData& operator=(const TextureCPUData&) = default;

        TextureCPUData(TextureCPUData&&) noexcept = default;
        TextureCPUData& operator=(TextureCPUData&&) noexcept = default;

        void init(const TextureConstructorInfo&);
    };
};

class Texture final : public Resource<Texture>, public Engine::priv::TextureBaseClass {
    public:
        class Impl;
    friend class  Engine::priv::ResourceVector<Texture>;
    friend class  Engine::priv::GBuffer;
    friend class  Engine::priv::TextureLoader;
    friend class  Engine::priv::detail::opengl::Impl;
    friend class  Texture::Impl;
    public:
        static Handle White, Black, Checkers, BRDF; //loaded in renderer. TODO: move these to built in class (separate from client side interface)
    private:
        Engine::priv::TextureCPUData    m_CPUData;
    public:
        Texture(const TextureConstructorInfo&, bool dispatchEventLoaded = true);

        Texture(const Texture&)                = delete;
        Texture& operator=(const Texture&)     = delete;
        Texture(Texture&&) noexcept;
        Texture& operator=(Texture&&) noexcept;
        ~Texture();

        inline bool operator==(bool rhs) const noexcept { return (rhs == true && m_TextureAddress != 0); }
        explicit inline operator bool() const noexcept { return m_TextureAddress != 0; }

        [[nodiscard]] const uint8_t* pixels();
        [[nodiscard]] inline TextureType getTextureType() const noexcept { return m_CPUData.m_TextureType; }
        [[nodiscard]] int width() const noexcept;
        [[nodiscard]] int height() const noexcept;
        [[nodiscard]] inline glm::uvec2 size() const noexcept { return glm::uvec2{ width(), height() }; }
        [[nodiscard]] inline glm::vec2 sizeAsRatio() const noexcept { const auto size_ = glm::vec2{ size() }; return glm::vec2(size_ / glm::max(size_.x, size_.y)); }
        [[nodiscard]] inline constexpr bool mipmapped() const noexcept { return m_CPUData.m_Mipmapped; }

        [[nodiscard]] bool compressed() const;

        void setAnisotropicFiltering(float anisotropicFiltering);
        
        [[nodiscard]] ImageInternalFormat internalFormat() const noexcept;
        [[nodiscard]] ImagePixelFormat pixelFormat() const noexcept;
        [[nodiscard]] ImagePixelType pixelType() const noexcept;

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
