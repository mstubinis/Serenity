#pragma once
#ifndef ENGINE_RENDERER_DECAL_H
#define ENGINE_RENDERER_DECAL_H

class Material;
namespace Engine::priv {
    struct DefaultDecalBindFunctor;
    struct DefaultDecalUnbindFunctor;
};

#include <ecs/EntityBody.h>

class Decal final: public EntityBody {
    friend struct Engine::priv::DefaultDecalBindFunctor;
    friend struct Engine::priv::DefaultDecalUnbindFunctor;
    private:
        float      m_LifetimeCurrent  = 0.0f;
        float      m_LifetimeMax;
        bool       m_Active           = true;
        glm_vec3   m_InitialPosition  = glm_vec3(0.0, 0.0, 0.0);
        glm_quat   m_InitialRotation  = glm_quat(1.0, 0.0, 0.0, 0.0);
    public:
        Decal(
            Material& material,
            const glm_vec3& position,
            const glm::vec3& hitNormal,
            float size,
            Scene& scene,
            float lifetimeMax,
            RenderStage::Stage stage = RenderStage::Decals
        );
        ~Decal();

        void update(const float dt);
        bool active() const;

        const glm_vec3& initialPosition() const;
        const glm_quat& initialRotation() const;
};
#endif