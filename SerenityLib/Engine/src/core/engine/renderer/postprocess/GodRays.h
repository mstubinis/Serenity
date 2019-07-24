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

            void pass(ShaderP&,GBuffer&,const unsigned int& fboWidth,const unsigned int& fboHeight,const glm::vec2& lightScrnPos,const float& alpha);

            static Postprocess_GodRays GodRays;
    };
};
namespace Renderer {
namespace godRays {
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
};
};


#endif