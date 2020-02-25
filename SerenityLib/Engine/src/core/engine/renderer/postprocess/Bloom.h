#pragma once
#ifndef ENGINE_RENDERER_POSTPROCESS_BLOOM_H_INCLUDE_GUARD
#define ENGINE_RENDERER_POSTPROCESS_BLOOM_H_INCLUDE_GUARD

class  ShaderProgram;
class  Shader;
class  Viewport;

#include <string>

namespace Engine::priv {
    class  GBuffer;
    class  Renderer;
    class  Bloom final {
        private:
            Shader*         m_Vertex_Shader   = nullptr;
            Shader*         m_Fragment_Shader = nullptr;
            ShaderProgram*  m_Shader_Program  = nullptr;
            std::string     m_GLSL_frag_code  = "";
        public:
            unsigned int num_passes           = 3;
            bool         bloom_active         = true;
            float        blur_radius          = 1.24f;
            float        blur_strength        = 0.62f;
            float        scale                = 0.27f;
            float        threshold            = 0.55f;
            float        exposure             = 1.6f;

            Bloom() = default;
            ~Bloom();

            const bool init_shaders();

            void pass(GBuffer&,const Viewport& viewport,const unsigned int& sceneTextureEnum, const Engine::priv::Renderer& renderer);

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