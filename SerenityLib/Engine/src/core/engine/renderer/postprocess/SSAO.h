#pragma once
#ifndef ENGINE_RENDERER_POSTPROCESS_SSAO_H_INCLUDE_GUARD
#define ENGINE_RENDERER_POSTPROCESS_SSAO_H_INCLUDE_GUARD

#define SSAO_MAX_KERNEL_SIZE 16U
#define SSAO_NORMALMAP_SIZE 16U

class  Camera;
class  ShaderProgram;
class  Shader;
class  Viewport;

#include <glm/vec3.hpp>
#include <string>

namespace Engine::priv {
    class  GBuffer;
    class  Renderer;
    class  SSAO final {
        private:
            Shader*        m_Vertex_Shader         = nullptr;
            Shader*        m_Fragment_Shader       = nullptr;
            ShaderProgram* m_Shader_Program        = nullptr;
            Shader*        m_Vertex_Shader_Blur    = nullptr;
            Shader*        m_Fragment_Shader_Blur  = nullptr;
            ShaderProgram* m_Shader_Program_Blur   = nullptr;

            std::string    m_GLSL_frag_code        = "";
            std::string    m_GLSL_frag_code_blur   = "";
        public:
            bool           m_ssao                  = true;
            bool           m_ssao_do_blur          = true;
            unsigned int   m_ssao_samples          = 8;
            unsigned int   m_ssao_blur_num_passes  = 2;
            unsigned int   m_ssao_noise_texture    = 0;
            float          m_ssao_blur_radius      = 0.66f;
            float          m_ssao_blur_strength    = 0.48f;
            float          m_ssao_scale            = 1.0;
            float          m_ssao_intensity        = 1.8f;
            float          m_ssao_bias             = 0.048f;
            float          m_ssao_radius           = 0.175f;
            //glm::vec3      m_ssao_Kernels[SSAO_MAX_KERNEL_SIZE];

            SSAO() = default;
            ~SSAO();

            const bool init_shaders();

            void init();

            void passSSAO(GBuffer&, const Viewport& viewport, const Camera&, const Engine::priv::Renderer& renderer);
            void passBlur(GBuffer&, const Viewport& viewport, std::string_view type, const unsigned int texture, const Engine::priv::Renderer& renderer);

            static SSAO ssao;
    };
};
namespace Engine::Renderer::ssao {
    const bool enabled();
    void enable(const bool b = true);
    void disable();
    void enableBlur(const bool b = true);
    void disableBlur();
    const float getBlurRadius();
    void setBlurRadius(const float r);
    const float getBlurStrength();
    void setBlurStrength(const float s);
    const float getIntensity();
    void setIntensity(const float i);
    const float getRadius();
    void setRadius(const float r);
    const float getScale();
    void setScale(const float s);
    const float getBias();
    void setBias(const float b);
    const unsigned int getSamples();
    void setSamples(const unsigned int s);
};
#endif