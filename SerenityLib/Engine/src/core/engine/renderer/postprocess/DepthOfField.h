#pragma once
#ifndef ENGINE_RENDERER_POSTPROCESS_DOF_H_INCLUDE_GUARD
#define ENGINE_RENDERER_POSTPROCESS_DOF_H_INCLUDE_GUARD

class  ShaderProgram;

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

            void pass(ShaderProgram&,GBuffer&,const unsigned int& fboWidth,const unsigned int& fboHeight,const unsigned int& sceneTextureEnum);

            static Postprocess_DepthOfField DOF;
    };
};
namespace Renderer {
namespace depthOfField {
    void enable(const bool b = true);
    void disable();
    const bool enabled();
    const float getFocus();
    void setFocus(const float);
    const float getBias();
    void setBias(const float);
    const float getBlurRadius();
    void setBlurRadius(const float);
};
};
};

#endif