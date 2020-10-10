#pragma once
#ifndef ENGINE_RENDERER_POSTPROCESS_GODRAYS_H
#define ENGINE_RENDERER_POSTPROCESS_GODRAYS_H

class  ShaderProgram;
class  Shader;
struct Entity;
class  Viewport;

#include <core/engine/resources/Handle.h>

namespace Engine::priv {
    class  GBuffer;
    class  RenderModule;
    class  GodRays final {
        private:
            Handle          m_Vertex_Shader;
            Handle          m_Fragment_Shader;
            Handle          m_Shader_Program;
            std::string     m_GLSL_frag_code  = "";

            GodRays() = default;
            ~GodRays() = default;
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


            bool init_shaders();

            void pass(GBuffer&, const Viewport& viewport, const glm::vec2& lightScrnPos, float alpha, const Engine::priv::RenderModule& renderer);

            static GodRays STATIC_GOD_RAYS;
    };
};
namespace Engine::Renderer::godRays {
    bool enabled();
    void enable(bool enabled);
    void disable();
    float getExposure();
    void setExposure(float exposure);
    float getFactor();
    void setFactor(float factor);
    float getDecay();
    void setDecay(float decay);
    float getDensity();
    void setDensity(float density);
    float getWeight();
    void setWeight(float weight);
    unsigned int getSamples();
    void setSamples(unsigned int numSamples);
    float getFOVDegrees();
    void setFOVDegrees(float fovInDegrees);
    float getAlphaFalloff();
    void setAlphaFalloff(float alphaFalloff);
    void setSun(Entity* sun);
    Entity* getSun();
};
#endif