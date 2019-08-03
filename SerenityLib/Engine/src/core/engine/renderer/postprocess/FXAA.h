#pragma once
#ifndef ENGINE_RENDERER_POSTPROCESS_FXAA_H_INCLUDE_GUARD
#define ENGINE_RENDERER_POSTPROCESS_FXAA_H_INCLUDE_GUARD

class  ShaderProgram;
namespace Engine {
namespace epriv {
    class  GBuffer;
    class  FXAA final {
        public:
            float reduce_min;
            float reduce_mul;
            float span_max;

            FXAA();
            ~FXAA();

            void pass(ShaderProgram&,GBuffer&,const unsigned int& fboWidth,const unsigned int& fboHeight,const unsigned int& sceneTextureEnum);

            static FXAA fxaa;
    };
};
namespace Renderer {
namespace fxaa {
    void setReduceMin(const float r);
    const float getReduceMin();
    void setReduceMul(const float r);
    const float getReduceMul();
    void setSpanMax(const float r);
    const float getSpanMax();
};
};
};


#endif