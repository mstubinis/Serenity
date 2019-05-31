#pragma once
#ifndef ENGINE_RENDERER_POSTPROCESS_BLOOM_H_INCLUDE_GUARD
#define ENGINE_RENDERER_POSTPROCESS_BLOOM_H_INCLUDE_GUARD

class  ShaderP;
namespace Engine {
namespace epriv {
    class  GBuffer;
    class  Postprocess_Bloom final {
        public:
            unsigned int num_passes;
            bool         bloom;
            float        blur_radius;
            float        blur_strength;
            float        scale;
            float        threshold;
            float        exposure;

            Postprocess_Bloom();
            ~Postprocess_Bloom();

            void pass(ShaderP&,GBuffer&,const unsigned int& fboWidth,const unsigned int& fboHeight,const unsigned int& sceneTextureEnum);

            static Postprocess_Bloom Bloom;
    };
};
namespace Renderer {
namespace bloom {
    unsigned int getNumPasses();
    void setNumPasses(unsigned int);
    void enable(bool b = true);
    void disable();
    bool enabled();
    float getThreshold();
    void setThreshold(float t);
    float getExposure();
    void setExposure(float e);
    float getBlurRadius();
    void setBlurRadius(float r);
    float getBlurStrength();
    void setBlurStrength(float r);
    float getScale();
    void setScale(float s);
};
};
};


#endif