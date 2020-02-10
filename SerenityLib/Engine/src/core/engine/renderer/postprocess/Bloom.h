#pragma once
#ifndef ENGINE_RENDERER_POSTPROCESS_BLOOM_H_INCLUDE_GUARD
#define ENGINE_RENDERER_POSTPROCESS_BLOOM_H_INCLUDE_GUARD

class  ShaderProgram;
class  Shader;

#include <string>

namespace Engine::priv {
    class  GBuffer;
    class  Bloom final {
        private:
            Shader*         m_Vertex_Shader;
            Shader*         m_Fragment_Shader;
            ShaderProgram*  m_Shader_Program;
            std::string     m_GLSL_frag_code;
        public:
            unsigned int num_passes;
            bool         bloom_active;
            float        blur_radius;
            float        blur_strength;
            float        scale;
            float        threshold;
            float        exposure;

            Bloom();
            ~Bloom();

            const bool init_shaders();

            void pass(GBuffer&,const unsigned int& fboWidth,const unsigned int& fboHeight,const unsigned int& sceneTextureEnum);

            static Bloom bloom;
    };
};
namespace Engine::Renderer::bloom {
    const unsigned int getNumPasses();
    void setNumPasses(const unsigned int);
    void enable(const bool b = true);
    void disable();
    const bool enabled();
    const float getThreshold();
    void setThreshold(const float t);
    const float getExposure();
    void setExposure(const float e);
    const float getBlurRadius();
    void setBlurRadius(const float r);
    const float getBlurStrength();
    void setBlurStrength(const float r);
    const float getScale();
    void setScale(const float s);
};
#endif