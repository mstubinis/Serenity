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
            bool  dof;

            Postprocess_DepthOfField();
            ~Postprocess_DepthOfField();

            void pass(ShaderP&,GBuffer&,const unsigned int& fboWidth,const unsigned int& fboHeight,const unsigned int& sceneTextureEnum);

            static Postprocess_DepthOfField DOF;
    };
};
namespace Renderer {
namespace depthOfField {
    void enable(const bool b = true);
    void disable();
    bool enabled();
    float getFocus();
    void setFocus(const float);
    float getBias();
    void setBias(const float);
    float getBlurRadius();
    void setBlurRadius(const float);
};
};
};

#endif