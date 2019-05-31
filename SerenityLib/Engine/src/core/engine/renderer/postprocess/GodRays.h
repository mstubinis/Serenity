#pragma once
#ifndef ENGINE_RENDERER_POSTPROCESS_GODRAYS_H_INCLUDE_GUARD
#define ENGINE_RENDERER_POSTPROCESS_GODRAYS_H_INCLUDE_GUARD

#include <glm/vec2.hpp>
#include <glm/vec4.hpp>

class  ShaderP;
struct Entity;
namespace Engine {
namespace epriv {
    class  GBuffer;
    class  Postprocess_GodRays final {
        public:
            bool        godRays;
            glm::vec4   clearColor;
            float       exposure;
            float       factor;
            float       decay;
            float       density;
            float       weight;
            float       fovDegrees;
            float       alphaFalloff;
            Entity*     sun;
            int         samples;

            Postprocess_GodRays();
            ~Postprocess_GodRays();

            void pass(
                ShaderP&,
                GBuffer&,
                const unsigned int& fboWidth,
                const unsigned int& fboHeight,
                const glm::vec2& lightScrnPos,
                const float& alpha
            );

            static Postprocess_GodRays GodRays;
    };
};
namespace Renderer {
namespace godRays {
    bool enabled();
    void enable(bool b);
    void disable();
    float getExposure();
    void setExposure(float e);
    float getFactor();
    void setFactor(float f);
    float getDecay();
    void setDecay(float d);
    float getDensity();
    void setDensity(float d);
    float getWeight();
    void setWeight(float w);
    unsigned int getSamples();
    void setSamples(unsigned int s);
    float getFOVDegrees();
    void setFOVDegrees(float d);
    float getAlphaFalloff();
    void setAlphaFalloff(float a);
    void setSun(Entity*);
    Entity* getSun();
};
};
};


#endif