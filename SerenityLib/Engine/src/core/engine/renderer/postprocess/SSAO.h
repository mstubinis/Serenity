#pragma once
#ifndef ENGINE_RENDERER_POSTPROCESS_SSAO_H_INCLUDE_GUARD
#define ENGINE_RENDERER_POSTPROCESS_SSAO_H_INCLUDE_GUARD


class  Camera;
class  ShaderP;

#include <glm/vec3.hpp>
#include <string>

namespace Engine {
namespace epriv {
    class  GBuffer;
    class  Postprocess_SSAO final {
        public:
            static const int SSAO_KERNEL_COUNT   = 32;
            static const int SSAO_NORMALMAP_SIZE = 16;

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
            glm::vec3      m_ssao_Kernels[SSAO_KERNEL_COUNT];
            
            Postprocess_SSAO();
            ~Postprocess_SSAO();

            void init();

            void passSSAO(ShaderP&,GBuffer&,const unsigned int& fboWidth,const unsigned int& fboHeight,Camera&);
            void passBlur(ShaderP&,GBuffer&,const unsigned int& fboWidth,const unsigned int& fboHeight,const std::string& type,const unsigned int& texture);

            static Postprocess_SSAO SSAO;
    };
};

namespace Renderer {
namespace ssao {
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
};
};

#endif