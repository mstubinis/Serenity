#pragma once
#ifndef ENGINE_RENDERER_DECAL_H
#define ENGINE_RENDERER_DECAL_H

#include <ecs/EntityWrapper.h>
#include <core/engine/math/Numbers.h>

namespace Engine {
    namespace epriv {
        struct DefaultDecalBindFunctor;
        struct DefaultDecalUnbindFunctor;
    };
};

class Material;
class Decal final: public EntityWrapper{
    friend struct Engine::epriv::DefaultDecalBindFunctor;
    friend struct Engine::epriv::DefaultDecalUnbindFunctor;
    private:
        float m_LifetimeCurrent;
        float m_LifetimeMax;
        bool  m_Active;
        glm_vec3 m_InitialPosition;
        glm_quat m_InitialRotation;
    public:
        Decal(Material& material, const glm_vec3& position, const glm::vec3& hitNormal, const float& size, Scene& scene, const float& lifetimeMax, const RenderStage::Stage stage = RenderStage::Decals);
        ~Decal();

        void update(const double& dt);
        const bool& active() const;
        const glm_vec3 position();
        const glm_quat rotation();
        const glm_vec3& initialPosition() const;
        const glm_quat& initialRotation() const;
        const glm::vec3 getScale();
        void setPosition(const glm_vec3& position);
        void setPosition(const decimal& x, const decimal& y, const decimal& z);
        void setRotation(const glm_quat& rotation);
        void setRotation(const decimal& x, const decimal& y, const decimal& z, const decimal& w);
};

#endif