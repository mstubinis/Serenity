#pragma once
#ifndef ENGINE_RENDERER_POSTPROCESS_FOG_H_INCLUDE_GUARD
#define ENGINE_RENDERER_POSTPROCESS_FOG_H_INCLUDE_GUARD

#include <glm/vec4.hpp>

namespace Engine::priv {
    class  GBuffer;
    class  Fog final {
        public:
            bool        fog_active = false;
            float       distNull   = 5.0f;
            float       distBlend  = 50.0f;
            glm::vec4   color      = glm::vec4(1.0f, 1.0f, 1.0f, 0.97f);

            Fog() = default;
            ~Fog();

            static Fog fog;
    };
};
namespace Engine::Renderer::fog {
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
#endif