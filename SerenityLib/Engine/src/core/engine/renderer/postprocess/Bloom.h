#pragma once
#ifndef ENGINE_RENDERER_POSTPROCESS_BLOOM_H_INCLUDE_GUARD
#define ENGINE_RENDERER_POSTPROCESS_BLOOM_H_INCLUDE_GUARD

class  ShaderProgram;
namespace Engine {
namespace epriv {
    class  GBuffer;
    class  Bloom final {
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

            void pass(ShaderProgram&,GBuffer&,const unsigned int& fboWidth,const unsigned int& fboHeight,const unsigned int& sceneTextureEnum);

            static Bloom bloom;
    };
};
namespace Renderer {
namespace bloom {
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
};
};


#endif