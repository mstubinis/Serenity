#pragma once
#ifndef ENGINE_RENDERER_POSTPROCESS_BLOOM_H
#define ENGINE_RENDERER_POSTPROCESS_BLOOM_H

class  ShaderProgram;
class  Shader;
class  Viewport;

#include <core/engine/resources/Handle.h>

namespace Engine::priv {
    class  GBuffer;
    class  Renderer;
    class  Bloom final {
        private:
            Handle         m_Vertex_Shader;
            Handle         m_Fragment_Shader;
            Handle         m_Shader_Program;
            std::string    m_GLSL_frag_code  = "";
        public:
            unsigned int num_passes           = 3;
            bool         bloom_active         = true;
            float        blur_radius          = 1.24f;
            float        blur_strength        = 0.62f;
            float        scale                = 0.27f;
            float        threshold            = 0.55f;
            float        exposure             = 1.6f;

            Bloom() = default;
            ~Bloom() = default;

            bool init_shaders();

            void pass(GBuffer&, const Viewport& viewport, unsigned int sceneTexture, const Engine::priv::Renderer& renderer);

            static Bloom bloom;
    };
};
namespace Engine::Renderer::bloom {
    unsigned int getNumPasses();
    void setNumPasses(unsigned int numPasses);
    void enable(bool enabled = true);
    void disable();
    bool enabled();
    float getThreshold();
    void setThreshold(float threshold);
    float getExposure();
    void setExposure(float exposure);
    float getBlurRadius();
    void setBlurRadius(float blurRadius);
    float getBlurStrength();
    void setBlurStrength(float blurStrength);
    float getScale();
    void setScale(float scale);
};
#endif