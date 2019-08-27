#pragma once
#ifndef ENGINE_PHYSICS_COLLISION_H
#define ENGINE_PHYSICS_COLLISION_H

#include <core/engine/utils/Utils.h>
#include <LinearMath/btVector3.h>
#include <core/engine/physics/PhysicsEnums.h>

#include <glm/vec3.hpp>

class  btCollisionShape;
class  Mesh;
class  ComponentModel;
class  ComponentBody;
class  btHeightfieldTerrainShape;

class Collision final {
    private:
        CollisionType::Type   m_Type;
        btVector3             m_BtInertia;
        btCollisionShape*     m_BtShape;

        void _init(ComponentBody* body, const std::vector<Mesh*>& meshes, const float& mass);
        void _baseInit(const CollisionType::Type _type, const float& mass);
    public:
        Collision();
        Collision(ComponentBody*, ComponentModel&, const float& mass = 0);
        Collision(btHeightfieldTerrainShape&, const CollisionType::Type, const float& mass = 0);
        Collision(const CollisionType::Type, Mesh* mesh, const float& mass = 0);

        Collision(const Collision& other);
        Collision& operator=(const Collision& other);
        Collision(Collision&& other) noexcept;
        Collision& operator=(Collision&& other) noexcept;


        ~Collision();

        void setMass(float mass);
        const btVector3& getBtInertia() const;
        btCollisionShape* getBtShape() const;
        const uint getType() const;
};

#endif