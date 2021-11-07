#pragma once
#ifndef ENGINE_ECS_COMPONENT_COLLISION_SHAPE_H
#define ENGINE_ECS_COMPONENT_COLLISION_SHAPE_H

struct SceneOptions;
class  ComponentModel;
class  ComponentRigidBody;
class  btCollisionObject;
class  btCollisionShape;
class  btCompoundShape;
class  btRigidBody;
class  ModelInstance;
namespace Engine::priv {
    class  sparse_set_base;
    class  ComponentCollisionShapeDeferredLoading;
};

#include <serenity/dependencies/glm.h>
#include <serenity/resources/Handle.h>
#include <serenity/physics/PhysicsIncludes.h>
#include <serenity/events/Observer.h>
#include <serenity/ecs/components/ComponentBaseClass.h>

class ComponentCollisionShape final : public ComponentBaseClass<ComponentCollisionShape> {
    friend class ::ComponentRigidBody;
    friend class Engine::priv::ComponentCollisionShapeDeferredLoading;
    private:
        struct ParentCompound final {
            btCompoundShape*  compoundShape = nullptr;
            int               index         = -1;
            Entity            parent;
        };
    private:
        btVector3                   m_BtInertia         = btVector3(0.0f, 0.0f, 0.0f);
        ParentCompound              m_ParentCompound;
        btCollisionShape*           m_BtCollisionShape  = nullptr;
        Entity                      m_Owner;

        void internal_free_memory();
        void internal_setScale(float x, float y, float z);
        void internal_update_ptrs();
    public:
        ComponentCollisionShape(Entity, CollisionType);
        ComponentCollisionShape(Entity, CollisionType, Handle colMesh, float mass);
        ComponentCollisionShape(Entity, CollisionType, ComponentModel&, float mass);
        ComponentCollisionShape(const ComponentCollisionShape&)            = delete;
        ComponentCollisionShape& operator=(const ComponentCollisionShape&) = delete;
        ComponentCollisionShape(ComponentCollisionShape&&) noexcept;
        ComponentCollisionShape& operator=(ComponentCollisionShape&&) noexcept;
        ~ComponentCollisionShape();

        void setMargin(float margin) noexcept;

        void setCollision(btCollisionShape* btShape);
        void setCollision(CollisionType, Handle mesh, float mass);
        void setCollision(CollisionType, ModelInstance*, float mass);
        void setCollision(CollisionType, ComponentModel&, float mass);
        void setCollision(CollisionType, const std::vector<Entity>&, float mass);

        [[nodiscard]] inline const btVector3& getInertia() const noexcept { return m_BtInertia; }
        [[nodiscard]] CollisionType getType() const noexcept;
        [[nodiscard]] inline bool isChildShape() const noexcept { return m_ParentCompound.compoundShape != nullptr; }
        [[nodiscard]] inline int getChildShapeIndex() const noexcept { return m_ParentCompound.index; }
        [[nodiscard]] inline btCompoundShape* getParentCompoundShape() noexcept { return m_ParentCompound.compoundShape; }
        [[nodiscard]] inline Entity getParent() noexcept { return m_ParentCompound.parent; }
        [[nodiscard]] inline Entity getOwner() const noexcept { return m_Owner; }
        [[nodiscard]] bool isStaticTriangleType() const noexcept;
        [[nodiscard]] inline btCollisionShape* getBtShape() const noexcept { return m_BtCollisionShape; }
        [[nodiscard]] inline void* getCollisionUserPointer() const noexcept;
        [[nodiscard]] inline int getCollisionUserIndex() const noexcept;
        [[nodiscard]] inline int getCollisionUserIndex2() const noexcept;

        inline void setCollisionUserPointer(void* ptr) const noexcept;
        inline void setCollisionUserIndex(int idx) const noexcept;
        inline void setCollisionUserIndex2(int idx) const noexcept;

        void calculateLocalInertia(float mass) noexcept;

        bool promoteToCompoundShape(CollisionType);
        bool promoteToCompoundShape(CollisionType, const std::vector<Entity>&);

        bool addChildShape(ComponentCollisionShape& other);
        bool updateChildShapeTransform(const glm_mat4& transformMatrix);

        //immediately syncs physics object to graphics object without waiting for it to occur normally for this frame
        void forcePhysicsSync() noexcept;
        void setMass(float mass);
};

namespace Engine::priv {
    class ComponentCollisionShapeDeferredLoading final : public Observer {
        friend class  ::ComponentCollisionShape;
        public:
            static inline ComponentCollisionShapeDeferredLoading& get() {
                static ComponentCollisionShapeDeferredLoading m_Instance;
                return m_Instance;
            }

        public:
            static void internal_load_single_mesh_impl(ComponentCollisionShape&, CollisionType, Handle mesh, float mass);
            static void internal_load_multiple_meshes_impl(ComponentCollisionShape&, std::vector<ModelInstance*>&, float mass, CollisionType);

            void onEvent(const Event&) override;
    };
};

#endif
