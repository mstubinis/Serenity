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

#include <serenity/physics/PhysicsIncludes.h>
#include <serenity/events/Event.h>
#include <serenity/events/Observer.h>
#include <LinearMath/btVector3.h>
#include <serenity/ecs/entity/Entity.h>
#include <BulletCollision/CollisionShapes/btCollisionShape.h>
#include <serenity/resources/Handle.h>

class Collision final: public Observer {
    private:
        std::vector<Handle>                m_DeferredMeshes;
        std::function<void()>              m_DeferredLoadingFunction = []() {};
        btVector3                          m_BtInertia               = btVector3(0.0f, 0.0f, 0.0f);
        std::unique_ptr<btCollisionShape>  m_BtShape;
        CollisionType                      m_Type                    = CollisionType::None;
        Entity                             m_Owner;

        void internal_base_init(CollisionType, float mass);
        void internal_free_memory();

        static void internal_load_1(Engine::view_ptr<Collision>, CollisionType, Handle mesh);
        static void internal_load_2(Engine::view_ptr<Collision>, Engine::view_ptr<btCompoundShape>, std::vector<Engine::view_ptr<ModelInstance>>, float mass, CollisionType);

        Collision() = delete;
    public:
        Collision(ComponentBody&);
        Collision(ComponentBody&, CollisionType, Engine::view_ptr<ModelInstance>, float mass = 0);
        Collision(ComponentBody&, CollisionType, Handle mesh, float mass = 0);
        Collision(ComponentBody&, ComponentModel&, float mass = 0, CollisionType = CollisionType::ConvexHull);

        Collision(const Collision&)                  = delete;
        Collision& operator=(const Collision&)       = delete;
        Collision(Collision&&) noexcept              = default;
        Collision& operator=(Collision&&) noexcept   = default;

        ~Collision();

        inline void setBtShape(btCollisionShape* shape) noexcept { m_BtShape.reset(shape); }
        [[nodiscard]] inline const btVector3& getBtInertia() const noexcept { return m_BtInertia; }
        [[nodiscard]] inline btCollisionShape* getBtShape() const noexcept { return m_BtShape.get(); }
        [[nodiscard]] inline CollisionType getType() const noexcept { return m_Type; }

        void setMass(float mass) noexcept;

        void onEvent(const Event&) override;
};
#endif