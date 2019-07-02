#pragma once
#ifndef ENGINE_RENDERER_POSTPROCESS_FXAA_H_INCLUDE_GUARD
#define ENGINE_RENDERER_POSTPROCESS_FXAA_H_INCLUDE_GUARD

class  ShaderP;
namespace Engine {
namespace epriv {
    class  GBuffer;
    class  Postprocess_FXAA final {
        public:
            float reduce_min;
            float reduce_mul;
            float span_max;

            Postprocess_FXAA();
            ~Postprocess_FXAA();

            void pass(ShaderP&,GBuffer&,const unsigned int& fboWidth,const unsigned int& fboHeight,const unsigned int& sceneTextureEnum);

            static Postprocess_FXAA FXAA;
    };
};
namespace Renderer {
namespace fxaa {
    void setReduceMin(const float r);
    float getReduceMin();
    void setReduceMul(const float r);
    float getReduceMul();
    void setSpanMax(const float r);
    float getSpanMax();
};
};
};


#endif