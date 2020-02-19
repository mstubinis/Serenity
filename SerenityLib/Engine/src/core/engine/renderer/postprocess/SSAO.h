#pragma once
#ifndef ENGINE_RENDERER_POSTPROCESS_SSAO_H_INCLUDE_GUARD
#define ENGINE_RENDERER_POSTPROCESS_SSAO_H_INCLUDE_GUARD

#define SSAO_MAX_KERNEL_SIZE 16
#define SSAO_NORMALMAP_SIZE 16

class  Camera;
class  ShaderProgram;
class  Shader;

#include <glm/vec3.hpp>
#include <string>

namespace Engine::priv {
    class  GBuffer;
    class  SSAO final {
        private:
            Shader* m_Vertex_Shader;
            Shader* m_Fragment_Shader;
            ShaderProgram* m_Shader_Program;


            Shader* m_Vertex_Shader_Blur;
            Shader* m_Fragment_Shader_Blur;
            ShaderProgram* m_Shader_Program_Blur;

            std::string     m_GLSL_frag_code;
            std::string     m_GLSL_frag_code_blur;
        public:
            bool           m_ssao;
            bool           m_ssao_do_blur;
            unsigned int   m_ssao_samples;
            unsigned int   m_ssao_blur_num_passes;
            unsigned int   m_ssao_noise_texture;
            float          m_ssao_blur_radius;
            float          m_ssao_blur_strength;
            float          m_ssao_scale;
            float          m_ssao_intensity;
            float          m_ssao_bias;
            float          m_ssao_radius;
            //glm::vec3      m_ssao_Kernels[SSAO_MAX_KERNEL_SIZE];

            SSAO();
            ~SSAO();

            const bool init_shaders();

            void init();

            void passSSAO(GBuffer&, const unsigned int& fboWidth, const unsigned int& fboHeight, const Camera&);
            void passBlur(GBuffer&, const unsigned int& fboWidth, const unsigned int& fboHeight, const std::string& type, const unsigned int& texture);

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