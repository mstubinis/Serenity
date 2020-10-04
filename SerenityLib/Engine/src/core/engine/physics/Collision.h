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
#include <BulletCollision/CollisionShapes/btCollisionShape.h>
#include <core/engine/resources/Handle.h>

class Collision final: public Observer {
    private:
        Entity                             m_Owner;
        std::vector<Handle>                m_DeferredMeshes;
        std::function<void()>              m_DeferredLoadingFunction = []() {};

        CollisionType                      m_Type                    = CollisionType::None;
        btVector3                          m_BtInertia               = btVector3(0.0f, 0.0f, 0.0f);
        std::unique_ptr<btCollisionShape>  m_BtShape;

        void internal_base_init(CollisionType collisionType, float mass);
        void internal_free_memory();

        static void internal_load_1(Collision* collision, CollisionType collisionType, Handle mesh, float mass);
        static void internal_load_2(Collision* collision, btCompoundShape* BTCompoundShape, std::vector<ModelInstance*> modelInstances, float mass, CollisionType collisionType);

        Collision() = delete;
    public:
        Collision(ComponentBody& componentBody);
        Collision(ComponentBody& componentBody, CollisionType collisionType, ModelInstance* modelInstance, float mass = 0);
        Collision(ComponentBody& componentBody, CollisionType collisionType, Handle mesh, float mass = 0);
        Collision(ComponentBody& componentBody, ComponentModel& componentModel, float mass = 0, CollisionType collisionType = CollisionType::ConvexHull);

        Collision(const Collision& other)                  = delete;
        Collision& operator=(const Collision& other)       = delete;
        Collision(Collision&& other) noexcept              = default;
        Collision& operator=(Collision&& other) noexcept   = default;

        ~Collision();

        inline void setBtShape(btCollisionShape* shape) noexcept { m_BtShape.reset(shape); }
        inline CONSTEXPR const btVector3& getBtInertia() const noexcept { return m_BtInertia; }
        inline CONSTEXPR btCollisionShape* getBtShape() const noexcept { return m_BtShape.get(); }
        inline CONSTEXPR CollisionType getType() const noexcept { return m_Type; }

        void setMass(float mass) noexcept;

        void onEvent(const Event& e) override;
};
#endif