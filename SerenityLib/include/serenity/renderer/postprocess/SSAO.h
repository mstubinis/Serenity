#pragma once
#ifndef ENGINE_RENDERER_POSTPROCESS_SSAO_H
#define ENGINE_RENDERER_POSTPROCESS_SSAO_H

class  Camera;
class  ShaderProgram;
class  Shader;
class  Viewport;

#include <serenity/system/TypeDefs.h>
#include <serenity/system/Macros.h>

constexpr int   SSAO_MAX_KERNEL_SIZE       = 64;
constexpr float SSAO_DEFAULT_SCALE         = 0.143f;
constexpr float SSAO_DEFAULT_RADIUS        = 0.522f;
constexpr float SSAO_DEFAULT_INTENSITY     = 1.6f;
constexpr float SSAO_DEFAULT_BIAS          = 0.005f;
constexpr float SSAO_BLUR_DEFAULT_STRENGTH = 0.48f;
constexpr float SSAO_BLUR_DEFAULT_RADIUS   = 0.257f;

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
            SSAOLevel         m_SSAOLevel     = SSAOLevel::Medium;
            int               m_NumSamples    = 64;
            int               m_BlurNumPasses = 1;
            float             m_Scale         = SSAO_DEFAULT_SCALE;
            float             m_Intensity     = SSAO_DEFAULT_INTENSITY;
            float             m_Bias          = SSAO_DEFAULT_BIAS;
            float             m_Radius        = SSAO_DEFAULT_RADIUS;
            bool              m_DoBlur        = true;

            static bool init();
            void passSSAO(GBuffer&, const Viewport&, const Camera&, const Engine::priv::RenderModule&);
            void passBlur(GBuffer&, const Viewport&, uint32_t texture, const Engine::priv::RenderModule&);
            void passBlurCopyPixels(GBuffer&, const Viewport&, uint32_t texture, const Engine::priv::RenderModule&);

            static SSAO STATIC_SSAO;
    };
};
namespace Engine::Renderer::ssao {
    void setLevel(const SSAOLevel);

    void enableBlur(bool blurEnabled = true) noexcept;
    void disableBlur() noexcept;
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