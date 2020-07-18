#pragma once
#ifndef ENGINE_RENDERER_POSTPROCESS_GODRAYS_H
#define ENGINE_RENDERER_POSTPROCESS_GODRAYS_H

class  ShaderProgram;
class  Shader;
struct Entity;
class  Viewport;

namespace Engine::priv {
    class  GBuffer;
    class  Renderer;
    class  GodRays final {
        private:
            Shader*          m_Vertex_Shader   = nullptr;
            Shader*          m_Fragment_Shader = nullptr;
            ShaderProgram*   m_Shader_Program  = nullptr;
            std::string      m_GLSL_frag_code  = "";
        public:
            bool        godRays_active         = true;
            glm::vec4   clearColor             = glm::vec4(0.030f, 0.023f, 0.032f, 1.0f);
            float       exposure               = 0.03f;
            float       factor                 = 1.0f;
            float       decay                  = 0.97f;
            float       density                = 1.5f;
            float       weight                 = 0.567f;
            float       fovDegrees             = 75.0f;
            float       alphaFalloff           = 2.0f;
            int         samples                = 80;

            GodRays() = default;
            ~GodRays();

            const bool init_shaders();

            void pass(GBuffer&, const Viewport& viewport, const glm::vec2& lightScrnPos, const float alpha, const Engine::priv::Renderer& renderer);

            static GodRays godRays;
    };
};
namespace Engine::Renderer::godRays {
    const bool enabled();
    void enable(const bool b);
    void disable();
    const float getExposure();
    void setExposure(const float e);
    const float getFactor();
    void setFactor(const float f);
    const float getDecay();
    void setDecay(const float d);
    const float getDensity();
    void setDensity(const float d);
    const float getWeight();
    void setWeight(const float w);
    const unsigned int getSamples();
    void setSamples(const unsigned int s);
    const float getFOVDegrees();
    void setFOVDegrees(const float d);
    const float getAlphaFalloff();
    void setAlphaFalloff(const float a);
    void setSun(Entity*);
    Entity* getSun();
};
#endif