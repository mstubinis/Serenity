#pragma once
#ifndef ENGINE_RENDERER_POSTPROCESS_FOG_H_INCLUDE_GUARD
#define ENGINE_RENDERER_POSTPROCESS_FOG_H_INCLUDE_GUARD

#include <glm/vec4.hpp>

class  ShaderProgram;
namespace Engine {
namespace epriv {
    class  GBuffer;
    class  Postprocess_Fog final {
        public:
            bool        fog;
            float       distNull;
            float       distBlend;
            glm::vec4   color;

            Postprocess_Fog();
            ~Postprocess_Fog();

            static Postprocess_Fog Fog;
        };
};
namespace Renderer {
namespace fog {
    void enable(const bool b = true);
    void disable();
    const bool enabled();
    void setColor(const glm::vec4& color);
    void setColor(const float r, const float g, const float b, const float a);
    void setNullDistance(const float d);
    void setBlendDistance(const float d);
    const float getNullDistance();
    const float getBlendDistance();
};
};
};


#endif