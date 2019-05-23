#pragma once
#ifndef ENGINE_RENDERER_POSTPROCESS_SSAO_H_INCLUDE_GUARD
#define ENGINE_RENDERER_POSTPROCESS_SSAO_H_INCLUDE_GUARD

class  ShaderP;
class  GBuffer;
class  Camera;

#include <glm/vec3.hpp>

namespace Engine {
namespace epriv {
    class Postprocess_SSAO final {
        public:
            static const int SSAO_KERNEL_COUNT = 32;
            static const int SSAO_NORMALMAP_SIZE = 16;

            bool m_ssao;
            bool m_ssao_do_blur;
            unsigned int m_ssao_samples;
            unsigned int m_ssao_blur_num_passes;
            float m_ssao_blur_radius;
            float m_ssao_blur_strength;
            float m_ssao_scale;
            float m_ssao_intensity;
            float m_ssao_bias;
            float m_ssao_radius;
            glm::vec3 m_ssao_Kernels[SSAO_KERNEL_COUNT];
            GLuint m_ssao_noise_texture;

            Postprocess_SSAO();
            ~Postprocess_SSAO();

            static Postprocess_SSAO SSAO;

            void pass(ShaderP&, GBuffer&, const uint& fboWidth, const uint& fboHeight, Camera&);
            void passBlur(ShaderP&, GBuffer&, const uint& fboWidth, const uint& fboHeight, string type, const GLuint& texture);
    };
};

namespace renderer {
namespace ssao {
    bool enabled();
    void enable(bool b = true);
    void disable();
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
};
};

#endif