#pragma once
#ifndef ENGINE_RENDERER_POSTPROCESS_FOG_H
#define ENGINE_RENDERER_POSTPROCESS_FOG_H

namespace Engine::priv {
    class  GBuffer;
    class  Fog final {
        private:
            Fog() = default;
            ~Fog() = default;
        public:
            bool        fog_active = false;
            float       distNull   = 5.0f;
            float       distBlend  = 50.0f;
            glm::vec4   color      = glm::vec4(1.0f, 1.0f, 1.0f, 0.97f);

            static Fog  STATIC_FOG;
    };
};
namespace Engine::Renderer::fog {
    void enable(bool enabled = true);
    void disable();
    bool enabled();
    void setColor(const glm::vec4& color);
    void setColor(float r, float g, float b, float a);
    void setNullDistance(float nullDistance);
    void setBlendDistance(float blendDistance);
    float getNullDistance();
    float getBlendDistance();
};
#endif