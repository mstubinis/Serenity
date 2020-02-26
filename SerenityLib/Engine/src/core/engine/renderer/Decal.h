#pragma once
#ifndef ENGINE_RENDERER_DECAL_H
#define ENGINE_RENDERER_DECAL_H

#include <ecs/EntityWrapper.h>
#include <core/engine/math/Numbers.h>

namespace Engine {
    namespace priv {
        struct DefaultDecalBindFunctor;
        struct DefaultDecalUnbindFunctor;
    };
};

class Material;
class Decal final: public EntityWrapper{
    friend struct Engine::priv::DefaultDecalBindFunctor;
    friend struct Engine::priv::DefaultDecalUnbindFunctor;
    private:
        float      m_LifetimeCurrent;
        float      m_LifetimeMax;
        bool       m_Active;
        glm_vec3   m_InitialPosition;
        glm_quat   m_InitialRotation;
    public:
        Decal(
            Material& material,
            const glm_vec3& position,
            const glm::vec3& hitNormal,
            const float size,
            Scene& scene,
            const float lifetimeMax,
            const RenderStage::Stage stage = RenderStage::Decals
        );
        ~Decal();

        void update(const float dt);
        const bool active() const;

        const glm_vec3& initialPosition() const;
        const glm_quat& initialRotation() const;

        const glm_vec3 position() const;
        const glm_vec3 localPosition() const;
        const glm_quat rotation() const;
        const glm::vec3 getScale() const;
        void setPosition(const glm_vec3& position);
        void setPosition(const decimal& x, const decimal& y, const decimal& z);
        void setRotation(const glm_quat& rotation);
        void setRotation(const decimal& x, const decimal& y, const decimal& z, const decimal& w);


        const glm_vec3 position(const EntityDataRequest& request) const;
        const glm_vec3 localPosition(const EntityDataRequest& request) const;
        const glm_quat rotation(const EntityDataRequest& request) const;
        const glm::vec3 getScale(const EntityDataRequest& request) const;
        void setPosition(const EntityDataRequest& request, const glm_vec3& position);
        void setPosition(const EntityDataRequest& request, const decimal& x, const decimal& y, const decimal& z);
        void setRotation(const EntityDataRequest& request, const glm_quat& rotation);
        void setRotation(const EntityDataRequest& request, const decimal& x, const decimal& y, const decimal& z, const decimal& w);
};

#endif