#pragma once
#ifndef ENGINE_PHYSICS_COLLISION_H
#define ENGINE_PHYSICS_COLLISION_H

#include <core/engine/utils/Utils.h>
#include <LinearMath/btVector3.h>
#include <core/engine/physics/PhysicsEnums.h>

class  btCollisionShape;
class  Mesh;
class  ComponentModel;
class  btHeightfieldTerrainShape;

class Collision final {
    private:
        CollisionType::Type   m_Type;
        btVector3             m_Inertia;
        btCollisionShape* m_Shape;

        void _init(const std::vector<Mesh*>& meshes, const float& mass);
        void _baseInit(const CollisionType::Type _type, const float& mass);
    public:
        Collision();
        Collision(const std::vector<Mesh*>& meshes, const float& mass = 0);
        Collision(ComponentModel&, const float& mass = 0);
        Collision(btHeightfieldTerrainShape&, const CollisionType::Type, const float& mass = 0);
        Collision(const CollisionType::Type, Mesh* mesh, const float& mass = 0);

        Collision(const Collision& other);
        Collision& operator=(const Collision& other);
        Collision(Collision&& other) noexcept;
        Collision& operator=(Collision&& other) noexcept;


        ~Collision();

        void setMass(float mass);
        const btVector3& getInertia() const;
        btCollisionShape* getShape() const;
        const uint getType() const;
};

#endif