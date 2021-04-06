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
#include <serenity/ecs/ECS.h>
#include <serenity/physics/PhysicsIncludes.h>
#include <serenity/events/Observer.h>
#include <serenity/ecs/components/ComponentBaseClass.h>
#include <serenity/renderer/RendererIncludes.h>
#include <LinearMath/btVector3.h>
#include <BulletCollision/CollisionShapes/btCollisionShape.h>

class ComponentCollisionShape : public ComponentBaseClass<ComponentCollisionShape> {
    friend class ComponentRigidBody;
    friend class Engine::priv::ComponentCollisionShapeDeferredLoading;
    private:
        static void internal_load_single_mesh(Entity, CollisionType, Handle mesh, float mass);
        static void internal_load_multiple_meshes(Entity, std::vector<ModelInstance*>&, float mass, CollisionType);
        static void internal_load_single_mesh_impl(ComponentCollisionShape&, CollisionType, Handle mesh, float mass);
        static void internal_load_multiple_meshes_impl(ComponentCollisionShape&, std::vector<ModelInstance*>&, float mass, CollisionType);
    private:
        btVector3                                   m_BtInertia      = btVector3(0.0f, 0.0f, 0.0f);
        std::tuple<btCompoundShape*, int, Entity>   m_ParentCompound = { nullptr, -1, Entity{} };
        std::unique_ptr<btCollisionShape>           m_CollisionShape;
        Entity                                      m_Owner;

        void internal_free_memory();
        void internal_setScale(float x, float y, float z);
        void internal_update_ptrs();
    public:
        ComponentCollisionShape(Entity entity, CollisionType collisionType);
        ComponentCollisionShape(const ComponentCollisionShape&)            = delete;
        ComponentCollisionShape& operator=(const ComponentCollisionShape&) = delete;
        ComponentCollisionShape(ComponentCollisionShape&&) noexcept;
        ComponentCollisionShape& operator=(ComponentCollisionShape&&) noexcept;
        ~ComponentCollisionShape();

        inline void setMargin(float margin) noexcept {
            ASSERT(margin >= 0.0f, __FUNCTION__ << "(): margin was negative!");
            m_CollisionShape->setMargin(margin);
        }

        void setCollision(btCollisionShape* shape);
        void setCollision(CollisionType collisionType, Handle mesh, float mass);
        void setCollision(CollisionType collisionType, ModelInstance*, float mass);
        void setCollision(CollisionType collisionType, ComponentModel&, float mass);
        void setCollision(CollisionType collisionType, const std::vector<Entity>&, float mass);

        [[nodiscard]] inline btCollisionShape* getBtShape() const noexcept { return m_CollisionShape.get(); }
        [[nodiscard]] inline void* getCollisionUserPointer() const noexcept { return getBtShape()->getUserPointer(); }
        [[nodiscard]] inline int getCollisionUserIndex() const noexcept { return getBtShape()->getUserIndex(); }
        [[nodiscard]] inline int getCollisionUserIndex2() const noexcept { return getBtShape()->getUserIndex2(); }

        inline void setCollisionUserPointer(void* ptr) const noexcept { getBtShape()->setUserPointer(ptr); }
        inline void setCollisionUserIndex(int idx) const noexcept { getBtShape()->setUserIndex(idx); }
        inline void setCollisionUserIndex2(int idx) const noexcept { getBtShape()->setUserIndex2(idx); }

        inline bool isStaticTriangleType() const noexcept { return (getType() == CollisionType::TRIANGLE_MESH_SHAPE_PROXYTYPE || getType() == CollisionType::SCALED_TRIANGLE_MESH_SHAPE_PROXYTYPE || getType() == CollisionType::TRIANGLE_SHAPE_PROXYTYPE); }

        void calculateLocalInertia(float mass) noexcept;

        [[nodiscard]] inline bool isChildShape() const noexcept { return std::get<0>(m_ParentCompound) != nullptr; }
        [[nodiscard]] inline int getChildShapeIndex() const noexcept { return std::get<1>(m_ParentCompound); }
        [[nodiscard]] inline btCompoundShape* getParentCompoundShape() noexcept { return std::get<0>(m_ParentCompound); }
        [[nodiscard]] inline Entity getParent() noexcept { return std::get<2>(m_ParentCompound); }
        [[nodiscard]] inline Entity getOwner() const noexcept { return m_Owner; }

        bool promoteToCompoundShape(CollisionType);
        bool promoteToCompoundShape(CollisionType, const std::vector<Entity>&);

        bool addChildShape(ComponentCollisionShape& other);

        [[nodiscard]] inline const btVector3& getInertia() const noexcept { return m_BtInertia; }
        [[nodiscard]] inline CollisionType getType() const noexcept { return static_cast<CollisionType>(m_CollisionShape->getShapeType()); }

        //immediately syncs physics object to graphics object without waiting for it to occur normally for this frame
        void forcePhysicsSync() noexcept;
        void setMass(float mass);
};

#include <serenity/events/Observer.h>
namespace Engine::priv {
    class ComponentCollisionShapeDeferredLoading final : public Observer {
        friend class  ComponentCollisionShape;
        private:
            std::unordered_map<Entity, std::tuple<Handle, float, CollisionType> >                        m_DeferredLoadingCollisionsSingle;
            std::unordered_map<Entity, std::tuple<std::vector<ModelInstance*>, float, CollisionType> >   m_DeferredLoadingCollisionsMulti;
        public:
            static inline ComponentCollisionShapeDeferredLoading& get() {
                static ComponentCollisionShapeDeferredLoading m_Instance;
                return m_Instance;
            }

            static inline void deferredLoadSingle(Entity owner, Handle mesh, float mass, CollisionType collisionType) {
                auto& single = get().m_DeferredLoadingCollisionsSingle;
                if (!single.contains(owner)) {
                    single.emplace(owner, std::make_tuple(mesh, mass, collisionType));
                }
            }
            static inline void deferredLoadMultiple(Entity owner, std::vector<ModelInstance*>& instances, float mass, CollisionType collisionType) {
                auto& multi = get().m_DeferredLoadingCollisionsMulti;
                if (!multi.contains(owner)) {
                    multi.emplace( owner, std::make_tuple(instances, mass, collisionType) );
                }
            }

            void onEvent(const Event&) override;
    };
};

#endif
