#pragma once
#ifndef ENGINE_RENDERER_POSTPROCESS_GODRAYS_H_INCLUDE_GUARD
#define ENGINE_RENDERER_POSTPROCESS_GODRAYS_H_INCLUDE_GUARD

class  ShaderProgram;
class  Shader;
struct Entity;
class  Viewport;

#include <glm/vec2.hpp>
#include <glm/vec4.hpp>
#include <string>

namespace Engine::priv {
    class  GBuffer;
    class  GodRays final {
        private:
            Shader*          m_Vertex_Shader;
            Shader*          m_Fragment_Shader;
            ShaderProgram*   m_Shader_Program;
            std::string      m_GLSL_frag_code;
        public:
            bool        godRays_active;
            glm::vec4   clearColor;
            float       exposure;
            float       factor;
            float       decay;
            float       density;
            float       weight;
            float       fovDegrees;
            float       alphaFalloff;
            int         samples;

            GodRays();
            ~GodRays();

            const bool init_shaders();

            void pass(GBuffer&,const Viewport& viewport,const glm::vec2& lightScrnPos,const float& alpha);

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