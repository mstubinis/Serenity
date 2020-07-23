#pragma once
#ifndef ENGINE_PHYSICS_COLLISION_H
#define ENGINE_PHYSICS_COLLISION_H

class  btCollisionShape;
class  Mesh;
class  ModelInstance;
class  ComponentModel;
class  ComponentBody;
class  btHeightfieldTerrainShape;
class  btCompoundShape;

#include <core/engine/physics/PhysicsIncludes.h>
#include <core/engine/events/Event.h>
#include <core/engine/events/Observer.h>
#include <LinearMath/btVector3.h>
#include <ecs/Entity.h>

class Collision final: public Observer {
    public:
        class DeferredLoading final {
            friend class Collision;
            private:
                static void load_1(Collision*, CollisionType, Mesh* mesh, float mass);
                static void load_2(Collision*, btCompoundShape*, std::vector<ModelInstance*>, float mass, CollisionType);
        };
    private:
        Entity                  m_Owner;
        std::vector<Mesh*>      m_DeferredMeshes;
        std::function<void()>   m_DeferredLoading;

        CollisionType           m_Type              = CollisionType::None;
        btVector3               m_BtInertia         = btVector3(0.0f, 0.0f, 0.0f);
        btCollisionShape*       m_BtShape           = nullptr;

        void internal_base_init(CollisionType type, float mass);

        void free_memory();
    public:
        Collision(ComponentBody&);
        Collision(ComponentBody&, CollisionType, ModelInstance* modelInstance, float mass = 0);
        Collision(ComponentBody&, CollisionType, Mesh& mesh, float mass = 0);
        Collision(ComponentBody&, ComponentModel&, float mass = 0, CollisionType = CollisionType::ConvexHull);

        Collision(const Collision& other) = delete;
        Collision& operator=(const Collision& other) = delete;
        Collision(Collision&& other) noexcept;
        Collision& operator=(Collision&& other) noexcept;

        ~Collision();

        inline void setBtShape(btCollisionShape* shape) noexcept { m_BtShape = shape; }
        inline CONSTEXPR const btVector3& getBtInertia() const noexcept { return m_BtInertia; }
        inline CONSTEXPR btCollisionShape* getBtShape() const noexcept { return m_BtShape; }
        inline CONSTEXPR CollisionType getType() const noexcept { return m_Type; }

        void setMass(float mass);

        void onEvent(const Event& event_) override;
};
#endif