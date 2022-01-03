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
        SSAOLevel         m_SSAOLevel = SSAOLevel::Medium;
        int               m_NumSamples = 8;
        int               m_BlurNumPasses = 1;
        float             m_BlurRadius = 0.66f;
        float             m_BlurStrength = 0.48f;
        float             m_Scale = 1.0;
        float             m_Intensity = 2.1f;
        float             m_Bias = 0.018f;
        float             m_Radius = 0.205f;
        float             m_RangeCheckScale = 0.338f;
        bool              m_DoBlur = true;

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
    [[nodiscard]] float getBlurRadius() noexcept;
    void setBlurRadius(float blurRadius) noexcept;
    [[nodiscard]] float getBlurStrength() noexcept;
    void setBlurStrength(float blurStrength) noexcept;
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
    [[nodiscard]] float getRangeScale () noexcept;
    void setRangeScale(float rangeScale) noexcept;
};
#endif