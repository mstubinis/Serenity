#pragma once
#ifndef ENGINE_RENDERER_POSTPROCESS_DOF_H_INCLUDE_GUARD
#define ENGINE_RENDERER_POSTPROCESS_DOF_H_INCLUDE_GUARD

class  ShaderP;

namespace Engine {
namespace epriv {
    class  GBuffer;
    class  Postprocess_DepthOfField final {
        public:
            float bias;
            float focus;
            float blur_radius;
            float aspect_ratio;
            bool  dof;

            Postprocess_DepthOfField();
            ~Postprocess_DepthOfField();

            void pass(ShaderP&,GBuffer&,const unsigned int& fboWidth,const unsigned int& fboHeight,const unsigned int& sceneTextureEnum);

            static Postprocess_DepthOfField DOF;
    };
};
namespace Renderer {
namespace depthOfField {
    void enable(bool b = true);
    void disable();
    bool enabled();
    float getFocus();
    void setFocus(float);
    float getBias();
    void setBias(float);
    float getBlurRadius();
    void setBlurRadius(float);
};
};
};

#endif