#pragma once
#ifndef ENGINE_RENDERER_POSTPROCESS_SSAO_H
#define ENGINE_RENDERER_POSTPROCESS_SSAO_H

class  Camera;
class  ShaderProgram;
class  Shader;
class  Viewport;

#include <serenity/system/TypeDefs.h>
#include <serenity/system/Macros.h>

constexpr int SSAO_MAX_KERNEL_SIZE = 16;

class SSAOLevel { 
    public:
        enum Type : uint8_t {
            Off = 0_uc,
            Low,
            Medium,
            High,
            Ultra,
        };
        BUILD_ENUM_CLASS_MEMBERS(SSAOLevel, Type)
};

#include <serenity/resources/Handle.h>
#include <serenity/dependencies/glm.h>
#include <random>
#include <string>
#include <string_view>

namespace Engine::priv {
    class  GBuffer;
    class  RenderModule;
    class  SSAO final {
        private:
            SSAO() = default;
            ~SSAO();
        public:
            SSAOLevel         m_SSAOLevel             = SSAOLevel::Medium;
            int               m_ssao_samples          = 8;
            int               m_ssao_blur_num_passes  = 2;
            float             m_ssao_blur_radius      = 0.66f;
            float             m_ssao_blur_strength    = 0.48f;
            float             m_ssao_scale            = 1.0;
            float             m_ssao_intensity        = 1.8f;
            float             m_ssao_bias             = 0.048f;
            float             m_ssao_radius           = 0.175f;
            bool              m_ssao_do_blur          = true;

            static bool init();
            void passSSAO(GBuffer&, const Viewport&, const Camera&, const Engine::priv::RenderModule&);
            void passBlur(GBuffer&, const Viewport&, std::string_view type, uint32_t texture, const Engine::priv::RenderModule&);

            static SSAO STATIC_SSAO;
    };
};
namespace Engine::Renderer::ssao {
    void setLevel(const SSAOLevel);

    void enableBlur(bool blurEnabled = true) noexcept;
    void disableBlur() noexcept;
    [[nodiscard]] float getBlurRadius() noexcept;
    void setBlurRadius(float blurRadius) noexcept;
    [[nodiscard]] float getBlurStrength() noexcept;
    void setBlurStrength(float blurStrength) noexcept;
    [[nodiscard]] int getBlurNumPasses() noexcept;
    void setBlurNumPasses(int numPasses) noexcept;
    [[nodiscard]] float getIntensity() noexcept;
    void setIntensity(float intensity) noexcept;
    [[nodiscard]] float getRadius() noexcept;
    void setRadius(float radius) noexcept;
    [[nodiscard]] float getScale() noexcept;
    void setScale(float scale) noexcept;
    [[nodiscard]] float getBias() noexcept;
    void setBias(float bias) noexcept;
    [[nodiscard]] int getSamples() noexcept;
    void setSamples(int numSamples) noexcept;
};
#endif