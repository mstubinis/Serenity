#pragma once
#ifndef ENGINE_RENDERER_DECAL_H
#define ENGINE_RENDERER_DECAL_H

#include <ecs/Entity.h>

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
        glm::vec3 m_InitialPosition;
        glm::quat m_InitialRotation;
    public:
        Decal(Material& material, const glm::vec3& position, const glm::vec3& hitNormal, const float& size, Scene& scene, const float& lifetimeMax, const RenderStage::Stage stage = RenderStage::Decals);
        ~Decal();

        void update(const double& dt);
        const bool& active() const;
        const glm::vec3 position();
        const glm::quat rotation();
        const glm::vec3& initialPosition() const;
        const glm::quat& initialRotation() const;
        const glm::vec3 getScale();
        void setPosition(const glm::vec3& position);
        void setPosition(const float& x, const float& y, const float& z);
        void setRotation(const glm::quat& rotation);
        void setRotation(const float& x, const float& y, const float& z, const float& w);
};

#endif