#pragma once
#ifndef ENGINE_RENDERER_POSTPROCESS_SSAO_H
#define ENGINE_RENDERER_POSTPROCESS_SSAO_H

#include <serenity/system/TypeDefs.h>

constexpr uint32_t SSAO_MAX_KERNEL_SIZE = 16U;
constexpr uint32_t SSAO_NORMALMAP_SIZE  = 16U;

class  Camera;
class  ShaderProgram;
class  Shader;
class  Viewport;

struct SSAOLevel { enum Level : uint8_t {
    Off = 0_uc,
    Low,
    Medium,
    High,
    Ultra,
};};

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
            Handle       m_Vertex_Shader;
            Handle       m_Fragment_Shader;
            Handle       m_Shader_Program;
            Handle       m_Vertex_Shader_Blur;
            Handle       m_Fragment_Shader_Blur;
            Handle       m_Shader_Program_Blur;

            std::string  m_GLSL_frag_code;
            std::string  m_GLSL_frag_code_blur;

            void internal_generate_kernel(std::uniform_real_distribution<float>& rand_dist, std::default_random_engine& gen) noexcept;
            void internal_generate_noise(std::uniform_real_distribution<float>& rand_dist, std::default_random_engine& gen) noexcept;

            void internal_init_blur_fragment_code();
            void internal_init_fragment_code();
            
            SSAO() = default;
            ~SSAO();
        public:
            SSAOLevel::Level  m_SSAOLevel             = SSAOLevel::Medium;
            uint32_t          m_ssao_samples          = 8;
            uint32_t          m_ssao_blur_num_passes  = 2;
            uint32_t          m_ssao_noise_texture    = 0;
            float             m_ssao_blur_radius      = 0.66f;
            float             m_ssao_blur_strength    = 0.48f;
            float             m_ssao_scale            = 1.0;
            float             m_ssao_intensity        = 1.8f;
            float             m_ssao_bias             = 0.048f;
            float             m_ssao_radius           = 0.175f;
            //glm::vec3       m_ssao_Kernels[SSAO_MAX_KERNEL_SIZE];
            bool              m_ssao_do_blur = true;

            static bool init();

            void passSSAO(GBuffer&, const Viewport& viewport, const Camera&, const Engine::priv::RenderModule& renderer);
            void passBlur(GBuffer&, const Viewport& viewport, std::string_view type, uint32_t texture, const Engine::priv::RenderModule& renderer);

            static SSAO STATIC_SSAO;
    };
};
namespace Engine::Renderer::ssao {
    void setLevel(SSAOLevel::Level);

    inline void enableBlur(bool blurEnabled = true) noexcept { Engine::priv::SSAO::STATIC_SSAO.m_ssao_do_blur = blurEnabled; }
    inline void disableBlur() noexcept { Engine::priv::SSAO::STATIC_SSAO.m_ssao_do_blur = false; }
    inline float getBlurRadius() noexcept { return Engine::priv::SSAO::STATIC_SSAO.m_ssao_blur_radius; }
    inline void setBlurRadius(float blurRadius) noexcept { Engine::priv::SSAO::STATIC_SSAO.m_ssao_blur_radius = std::max(0.0f, blurRadius); }
    inline float getBlurStrength() noexcept { return Engine::priv::SSAO::STATIC_SSAO.m_ssao_blur_strength; }
    inline void setBlurStrength(float blurStrength) noexcept { Engine::priv::SSAO::STATIC_SSAO.m_ssao_blur_strength = std::max(0.0f, blurStrength); }
    inline float getIntensity() noexcept { return Engine::priv::SSAO::STATIC_SSAO.m_ssao_intensity; }
    inline void setIntensity(float intensity) noexcept { Engine::priv::SSAO::STATIC_SSAO.m_ssao_intensity = std::max(0.0f, intensity); }
    inline float getRadius() noexcept { return Engine::priv::SSAO::STATIC_SSAO.m_ssao_radius; }
    inline void setRadius(float radius) noexcept { Engine::priv::SSAO::STATIC_SSAO.m_ssao_radius = std::max(0.0f, radius); }
    inline float getScale() noexcept { return Engine::priv::SSAO::STATIC_SSAO.m_ssao_scale; }
    inline void setScale(float scale) noexcept { Engine::priv::SSAO::STATIC_SSAO.m_ssao_scale = std::max(0.0f, scale); }
    inline float getBias() noexcept { return Engine::priv::SSAO::STATIC_SSAO.m_ssao_bias; }
    inline void setBias(float bias) noexcept { Engine::priv::SSAO::STATIC_SSAO.m_ssao_bias = bias; }
    inline uint32_t getSamples() noexcept { return Engine::priv::SSAO::STATIC_SSAO.m_ssao_samples; }
    inline void setSamples(uint32_t numSamples) noexcept { Engine::priv::SSAO::STATIC_SSAO.m_ssao_samples = glm::clamp(std::max(0U, numSamples), 0U, SSAO_MAX_KERNEL_SIZE); }
};
#endif