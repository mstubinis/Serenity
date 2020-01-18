#pragma once
#ifndef ENGINE_PHYSICS_COLLISION_H
#define ENGINE_PHYSICS_COLLISION_H

class  btCollisionShape;
class  Mesh;
class  ModelInstance;
class  ComponentModel;
class  ComponentBody;
class  btHeightfieldTerrainShape;

#include <core/engine/math/Numbers.h>
#include <core/engine/physics/PhysicsIncludes.h>
#include <LinearMath/btVector3.h>
#include <vector>

class Collision final {
    private:
        CollisionType::Type   m_Type;
        btVector3             m_BtInertia;
        btCollisionShape*     m_BtShape;

        void _init(ComponentBody* body, std::vector<ModelInstance*>& modelInstances, const float& mass, const CollisionType::Type _type = CollisionType::ConvexHull);
        void _baseInit(const CollisionType::Type _type, const float& mass);
    public:
        Collision();
        Collision(btHeightfieldTerrainShape&, const CollisionType::Type, const float& mass = 0);
        Collision(const CollisionType::Type, ModelInstance* modelInstance, const float& mass = 0);
        Collision(const CollisionType::Type, Mesh& mesh, const float& mass = 0);
        Collision(ComponentBody* body, ComponentModel&, const float& mass = 0, const CollisionType::Type = CollisionType::ConvexHull);

        Collision(const Collision& other);
        Collision& operator=(const Collision& other);
        Collision(Collision&& other) noexcept;
        Collision& operator=(Collision&& other) noexcept;

        ~Collision();

        void setMass(const float mass);
        const btVector3& getBtInertia() const;
        btCollisionShape* getBtShape() const;
        const CollisionType::Type& getType() const;
};
#endif