#pragma once
#ifndef ENGINE_RENDERER_POSTPROCESS_HDR_H_INCLUDE_GUARD
#define ENGINE_RENDERER_POSTPROCESS_HDR_H_INCLUDE_GUARD

class  ShaderProgram;

struct HDRAlgorithm {enum Algorithm {
    Reinhard, Filmic, Exposure, Uncharted,
};};

namespace Engine {
namespace epriv {
    class  GBuffer;
    class  HDR final {
        public:
            bool                      hdr_active;
            float                     exposure;
            HDRAlgorithm::Algorithm   algorithm;

            HDR();
            ~HDR();

            void pass(ShaderProgram&,GBuffer&,const unsigned int& fboWidth,const unsigned int& fboHeight,const bool& godRays,const bool& lighting,const float& godRaysFactor);

            static HDR hdr;
    };
};
namespace Renderer {
namespace hdr {
    const bool enabled();
    void enable(const bool b = true);
    void disable();
    const float getExposure();
    void setExposure(const float e);
    void setAlgorithm(const HDRAlgorithm::Algorithm a);
};
};
};



#endif