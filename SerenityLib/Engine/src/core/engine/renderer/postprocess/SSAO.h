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