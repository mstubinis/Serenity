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
#include <core/engine/events/Engine_EventObject.h>
#include <LinearMath/btVector3.h>
#include <ecs/Entity.h>
#include <vector>
#include <functional>

class Collision final: public EventObserver {
    public:
        class DeferredLoading final {
            friend class Collision;
            private:
                static void load_1(Collision*, const CollisionType::Type, Mesh* mesh, const float mass);
                static void load_2(Collision*, btCompoundShape*, std::vector<ModelInstance*>, const float mass, const CollisionType::Type);
        };
    private:
        Entity                  m_Owner;
        std::vector<Mesh*>      m_DeferredMeshes;
        std::function<void()>   m_DeferredLoading;

        CollisionType::Type     m_Type              = CollisionType::None;
        btVector3               m_BtInertia         = btVector3(0.0f, 0.0f, 0.0f);
        btCollisionShape*       m_BtShape           = nullptr;

        void _baseInit(const CollisionType::Type type, const float mass);

        void free_memory();

    public:
        Collision(ComponentBody&);
        Collision(ComponentBody&, const CollisionType::Type, ModelInstance* modelInstance, const float mass = 0);
        Collision(ComponentBody&, const CollisionType::Type, Mesh& mesh, const float mass = 0);
        Collision(ComponentBody&, ComponentModel&, const float mass = 0, const CollisionType::Type = CollisionType::ConvexHull);

        Collision(const Collision& other) = delete;
        Collision& operator=(const Collision& other) = delete;
        Collision(Collision&& other) noexcept;
        Collision& operator=(Collision&& other) noexcept;

        ~Collision();

        void setBtShape(btCollisionShape* shape);
        void setMass(const float mass);
        const btVector3& getBtInertia() const;
        btCollisionShape* getBtShape() const;
        CollisionType::Type getType() const;

        void onEvent(const Event& event_) override;
};
#endif