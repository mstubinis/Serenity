#pragma once
#ifndef ENGINE_RENDERER_POSTPROCESS_SSAO_H
#define ENGINE_RENDERER_POSTPROCESS_SSAO_H

constexpr std::uint32_t SSAO_MAX_KERNEL_SIZE = 16U;
constexpr std::uint32_t SSAO_NORMALMAP_SIZE  = 16U;

class  Camera;
class  ShaderProgram;
class  Shader;
class  Viewport;

struct SSAOLevel { enum Level : unsigned char {
    Off = 0_uc,
    Low,
    Medium,
    High,
    Ultra,
};};

namespace Engine::priv {
    class  GBuffer;
    class  Renderer;
    class  SSAO final {
        private:
            std::unique_ptr<Shader>          m_Vertex_Shader;
            std::unique_ptr<Shader>          m_Fragment_Shader;
            std::unique_ptr<ShaderProgram>   m_Shader_Program;
            std::unique_ptr<Shader>          m_Vertex_Shader_Blur;
            std::unique_ptr<Shader>          m_Fragment_Shader_Blur;
            std::unique_ptr<ShaderProgram>   m_Shader_Program_Blur;

            std::string                      m_GLSL_frag_code       = "";
            std::string                      m_GLSL_frag_code_blur  = "";

            void internal_generate_kernel(std::uniform_real_distribution<float>& rand_dist, std::default_random_engine& gen) noexcept;
            void internal_generate_noise(std::uniform_real_distribution<float>& rand_dist, std::default_random_engine& gen) noexcept;

            SSAO() = default;
            ~SSAO();
        public:
            SSAOLevel::Level  m_SSAOLevel             = SSAOLevel::Medium;
            bool              m_ssao_do_blur          = true;
            unsigned int      m_ssao_samples          = 8;
            unsigned int      m_ssao_blur_num_passes  = 2;
            unsigned int      m_ssao_noise_texture    = 0;
            float             m_ssao_blur_radius      = 0.66f;
            float             m_ssao_blur_strength    = 0.48f;
            float             m_ssao_scale            = 1.0;
            float             m_ssao_intensity        = 1.8f;
            float             m_ssao_bias             = 0.048f;
            float             m_ssao_radius           = 0.175f;
            //glm::vec3       m_ssao_Kernels[SSAO_MAX_KERNEL_SIZE];

            bool init_shaders();
            void init();

            void passSSAO(GBuffer&, const Viewport& viewport, const Camera&, const Engine::priv::Renderer& renderer);
            void passBlur(GBuffer&, const Viewport& viewport, std::string_view type, unsigned int texture, const Engine::priv::Renderer& renderer);

            static SSAO STATIC_SSAO;
    };
};
namespace Engine::Renderer::ssao {
    void setLevel(SSAOLevel::Level);

    void enableBlur(bool b = true);
    void disableBlur();
    float getBlurRadius();
    void setBlurRadius(float r);
    float getBlurStrength();
    void setBlurStrength(float s);
    float getIntensity();
    void setIntensity(float i);
    float getRadius();
    void setRadius(float r);
    float getScale();
    void setScale(float s);
    float getBias();
    void setBias(float b);
    unsigned int getSamples();
    void setSamples(unsigned int s);
};
#endif