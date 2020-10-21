#pragma once
#ifndef ENGINE_ECS_COMPONENT_BODY_H
#define ENGINE_ECS_COMPONENT_BODY_H

struct SceneOptions;
class  Collision;
class  ComponentModel;
class  ComponentBody;
class  btCollisionObject;
class  btRigidBody;
namespace Engine::priv {
    class  sparse_set_base;
    class  ComponentBody_System;
    struct ComponentBody_UpdateFunction;
    struct ComponentBody_EntityAddedToSceneFunction;
    struct ComponentBody_ComponentAddedToEntityFunction;
    struct ComponentBody_ComponentRemovedFromEntityFunction;
    struct ComponentBody_SceneEnteredFunction;
    struct ComponentBody_SceneLeftFunction;
};

#include <ecs/ECS.h>
#include <core/engine/physics/PhysicsIncludes.h>
#include <LinearMath/btDefaultMotionState.h>
#include <ecs/Entity.h>
#include <core/engine/events/Observer.h>

struct CollisionCallbackEventData final {
    ComponentBody&     ownerBody;
    ComponentBody&     otherBody;
    glm::vec3&         ownerHit;
    glm::vec3&         otherHit;
    glm::vec3&         ownerLocalHit;
    glm::vec3&         otherLocalHit;
    glm::vec3&         normalOnB;
    glm::vec3&         normalFromA;
    glm::vec3&         normalFromB;
    btCollisionObject* ownerCollisionObj = nullptr;
    btCollisionObject* otherCollisionObj = nullptr;
    size_t             ownerModelInstanceIndex = 0;
    size_t             otherModelInstanceIndex = 0;

    CollisionCallbackEventData() = delete;
    CollisionCallbackEventData(const CollisionCallbackEventData& other)                = delete;
    CollisionCallbackEventData& operator=(const CollisionCallbackEventData& other)     = delete;
    CollisionCallbackEventData(CollisionCallbackEventData&& other) noexcept            = delete;
    CollisionCallbackEventData& operator=(CollisionCallbackEventData&& other) noexcept = delete;

    CollisionCallbackEventData(ComponentBody& ownerBody_, ComponentBody& otherBody_, glm::vec3& ownerHit_, glm::vec3& otherHit_, glm::vec3& normal_,
        glm::vec3& ownerLocalHit_, glm::vec3& otherLocalHit_, glm::vec3& normalFromA_, glm::vec3& normalFromB_
    )
        : ownerBody{ ownerBody_ }
        , otherBody{ otherBody_ }
        , ownerHit{ ownerHit_ }
        , otherHit{ otherHit_ }
        , normalOnB{ normal_ }
        , ownerLocalHit{ ownerLocalHit_ }
        , otherLocalHit{ otherLocalHit_ }
        , normalFromA{ normalFromA_ }
        , normalFromB{ normalFromB_ }
    {}
};

struct ScreenBoxCoordinates final {
    bool      inBounds      = false;
    glm::vec2 topLeft       = glm::vec2(0.0f, 0.0f);
    glm::vec2 topRight      = glm::vec2(0.0f, 0.0f);
    glm::vec2 bottomLeft    = glm::vec2(0.0f, 0.0f);
    glm::vec2 bottomRight   = glm::vec2(0.0f, 0.0f);
};

class ComponentBody : public Engine::UserPointer {
    friend class  Engine::priv::ComponentBody_System;
    friend struct Engine::priv::ComponentBody_UpdateFunction;
    friend struct Engine::priv::ComponentBody_ComponentAddedToEntityFunction;
    friend struct Engine::priv::ComponentBody_ComponentRemovedFromEntityFunction;
    friend struct Engine::priv::ComponentBody_EntityAddedToSceneFunction;
    friend struct Engine::priv::ComponentBody_SceneEnteredFunction;
    friend struct Engine::priv::ComponentBody_SceneLeftFunction;
    friend class  ComponentModel;
    using CollisionCallbackFP = std::function<void(CollisionCallbackEventData& data)>;
    private:
        struct PhysicsData final {
            std::unique_ptr<Collision>    collision;
            std::unique_ptr<btRigidBody>  bullet_rigidBody;
            btDefaultMotionState          bullet_motionState     = btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1)));
            float                         mass                   = 0.0f;
            MaskType                      group                  = CollisionFilter::DefaultFilter;;
            MaskType                      mask                   = CollisionFilter::AllFilter;
            bool                          forcedOut              = false;

            PhysicsData() = default;
            PhysicsData(const PhysicsData& other)                  = delete;
            PhysicsData& operator=(const PhysicsData& other)       = delete;
            PhysicsData(PhysicsData&& other) noexcept; 
            PhysicsData& operator=(PhysicsData&& other) noexcept;
            ~PhysicsData();
        };
        struct NormalData final {
            glm_quat rotation       = glm_quat(1.0, 0.0, 0.0, 0.0);
            glm_vec3 scale          = glm_vec3(1.0);
            glm_vec3 position       = glm_vec3(0.0);
            glm_vec3 linearVelocity = glm_vec3(0.0);

            NormalData() = default;
            NormalData(const NormalData& other)                  = delete;
            NormalData& operator=(const NormalData& other)       = delete;
            NormalData(NormalData&& other) noexcept;
            NormalData& operator=(NormalData&& other) noexcept;
            ~NormalData() = default;
        };

        glm_vec3              m_Forward          = glm_vec3(0.0, 0.0, -1.0);
        glm_vec3              m_Right            = glm_vec3(1.0, 0.0, 0.0);
        glm_vec3              m_Up               = glm_vec3(0.0, 1.0, 0.0);
        CollisionCallbackFP   m_CollisionFunctor = [](CollisionCallbackEventData&) {};
        //union {
        std::unique_ptr<NormalData>       n;
        std::unique_ptr<PhysicsData>      p;
        //};
        void*                 m_UserPointer1     = nullptr;
        void*                 m_UserPointer2     = nullptr;
        Entity                m_Owner;
        bool                  m_Physics          = false;

        btVector3 internal_activate_and_get_vector(decimal x, decimal y, decimal z, bool local) noexcept;
        void internal_update_misc() noexcept;
        static void internal_recalculateAllParentChildMatrices(Engine::priv::ComponentBody_System& system);
    public:
        static void recalculateAllParentChildMatrices(Scene& scene);
    public:
        ComponentBody(Entity entity);
        ComponentBody(Entity entity, CollisionType collisionType);
        ComponentBody(const ComponentBody&)                  = delete;
        ComponentBody& operator=(const ComponentBody&)       = delete;
        ComponentBody(ComponentBody&&) noexcept;
        ComponentBody& operator=(ComponentBody&&) noexcept;
        ~ComponentBody();

        inline CONSTEXPR Entity getOwner() const noexcept { return m_Owner; }

        bool hasParent() const;

        void addChild(Entity child) const;
        inline void addChild(const ComponentBody& child) const noexcept { addChild(child.m_Owner); }

        void removeChild(Entity child) const;
        inline void removeChild(const ComponentBody& child) const noexcept { removeChild(child.m_Owner); }
        void removeAllChildren() const;

        inline void setCollisionFunctor(const CollisionCallbackFP& functor) noexcept { m_CollisionFunctor = functor; }
        inline void setCollisionFunctor(CollisionCallbackFP&& functor) noexcept { m_CollisionFunctor = std::move(functor); }

        void collisionResponse(CollisionCallbackEventData& data) const;

        void rebuildRigidBody(bool addBodyToPhysicsWorld = true, bool threadSafe = false);
        void removePhysicsFromWorld(bool force = true, bool threadSafe = false);
        void addPhysicsToWorld(bool force = true, bool threadSafe = false);

        void setInternalPhysicsUserPointer(void* userPtr);
        void setUserPointer1(void* userPtr) noexcept { m_UserPointer1 = userPtr; }
        void setUserPointer2(void* userPtr) noexcept { m_UserPointer2 = userPtr; }
        inline CONSTEXPR void* getUserPointer1() const noexcept { return m_UserPointer1; }
        inline CONSTEXPR void* getUserPointer2() const noexcept { return m_UserPointer2; }

        inline CONSTEXPR bool hasPhysics() const noexcept { return m_Physics; }
        decimal getLinearDamping() const;
        decimal getAngularDamping() const;

        inline MaskType getCollisionGroup() const noexcept { return (m_Physics) ? p->group : 0; }
        inline MaskType getCollisionMask() const noexcept { return (m_Physics) ? p->mask : 0; }

        MaskType getCollisionFlags() const;

        void alignTo(decimal dirX, decimal dirY, decimal dirZ);
        void alignTo(const glm_vec3& direction);

        inline void translate(const glm_vec3& translation, bool local = true) noexcept { translate(translation.x, translation.y, translation.z, local); }
        inline void translate(decimal translation, bool local = true) noexcept { translate(translation, translation, translation, local); }
        void translate(decimal x, decimal y, decimal z, bool local = true);


        inline void rotate(const glm_vec3& rotation, bool local = true) noexcept { rotate(rotation.x, rotation.y, rotation.z, local); }
        void rotate(decimal pitch_radians, decimal yaw_radians, decimal roll_radians, bool local = true);

        inline void scale(const glm_vec3& scaleAmount) noexcept { scale(scaleAmount.x, scaleAmount.y, scaleAmount.z); }
        inline void scale(decimal scaleAmount) noexcept { scale(scaleAmount, scaleAmount, scaleAmount); }
        void scale(decimal x, decimal y, decimal z);


        inline void setPosition(const glm_vec3& newPosition) noexcept { setPosition(newPosition.x, newPosition.y, newPosition.z); }
        inline void setPosition(decimal newPosition) noexcept { setPosition(newPosition, newPosition, newPosition); }
        void setPosition(decimal x, decimal y, decimal z);


        inline void setRotation(const glm_quat& newRotation) noexcept { setRotation(newRotation.x, newRotation.y, newRotation.z, newRotation.w); }
        void setRotation(decimal quat_x, decimal quat_y, decimal quat_z, decimal quat_w);

        void setScale(decimal x, decimal y, decimal z);
        inline void setScale(const glm_vec3& newScale) noexcept { setScale(newScale.x, newScale.y, newScale.z); }
        inline void setScale(decimal newScale) noexcept { setScale(newScale, newScale, newScale); }

        inline float mass() const noexcept { return (m_Physics) ? p->mass : 0.0f; }
        decimal getDistance(Entity other) const;
        unsigned long long getDistanceLL(Entity other) const;
        glm::vec3 getScreenCoordinates(bool clampToEdge = false) const;

        ScreenBoxCoordinates getScreenBoxCoordinates(float minOffset = 10.0f) const;

		glm_quat getRotation() const;
		glm_vec3 getScale() const;
		glm_vec3 getPosition() const;
        glm_vec3 getLocalPosition() const;
        glm::vec3 getPositionRender() const;

        
        inline CONSTEXPR const glm_vec3& forward() const noexcept { return m_Forward; }
        inline CONSTEXPR const glm_vec3& right() const noexcept { return m_Right; }
        inline CONSTEXPR const glm_vec3& up() const noexcept { return m_Up; }
        
		glm_vec3 getLinearVelocity() const;
		glm_vec3 getAngularVelocity() const;
		glm_mat4 modelMatrix() const;
        inline glm::mat4 modelMatrixRendering() const noexcept { return (glm::mat4)modelMatrix(); }
        inline btRigidBody& getBtBody() const noexcept { return *p->bullet_rigidBody; }

        void setCollision(CollisionType collisionType, float mass);

        //double check this...
        template<typename ... ARGS>
        Collision& setCollision(ARGS&&... args) {
            if (p->collision) {
                removePhysicsFromWorld(false, false);
            }
            p->collision.reset(NEW Collision(std::forward<ARGS>(args)...));
            internal_update_misc();
            if (p->bullet_rigidBody) {
                addPhysicsToWorld(false, false);
            }
            setInternalPhysicsUserPointer(this);
            return *p->collision.get();
        }
        inline Engine::view_ptr<Collision> getCollision() const noexcept { return (m_Physics) ? p->collision.get() : nullptr; }

        void setCollisionGroup(MaskType group);
        void setCollisionMask(MaskType mask);
        void setCollisionFlag(MaskType flag);
        inline void setCollisionGroup(CollisionFilter::Filter group) noexcept { setCollisionGroup((MaskType)group); }
        inline void setCollisionMask(CollisionFilter::Filter mask) noexcept { setCollisionMask((MaskType)mask); }
        inline void setCollisionFlag(CollisionFlag::Flag flag) noexcept { setCollisionFlag((MaskType)flag); }
        void addCollisionGroup(MaskType group);
        void addCollisionMask(MaskType mask);
        void addCollisionFlag(MaskType flag);
        inline void addCollisionGroup(CollisionFilter::Filter group) noexcept { addCollisionGroup((MaskType)group); }
        inline void addCollisionMask(CollisionFilter::Filter mask) noexcept { addCollisionMask((MaskType)mask); }
        inline void addCollisionFlag(CollisionFlag::Flag flag) noexcept { addCollisionFlag((MaskType)flag); }
        void removeCollisionGroup(MaskType group);
        void removeCollisionMask(MaskType mask);
        void removeCollisionFlag(MaskType flag);
        inline void removeCollisionGroup(CollisionFilter::Filter group) noexcept { removeCollisionGroup((MaskType)group); }
        inline void removeCollisionMask(CollisionFilter::Filter mask) noexcept { removeCollisionMask((MaskType)mask); }
        inline void removeCollisionFlag(CollisionFlag::Flag flag) noexcept { removeCollisionFlag((MaskType)flag); }

        void setDamping(decimal linear, decimal angular);

        void setDynamic(bool dynamic);
        void setMass(float mass);
        void setGravity(decimal x, decimal y, decimal z);

        void clearLinearForces();
        void clearAngularForces();
        void clearAllForces();

        void setLinearVelocity(decimal x, decimal y, decimal z, bool local = true);
        inline void setLinearVelocity(const glm_vec3& velocity, bool local = true) noexcept { setLinearVelocity(velocity.x, velocity.y, velocity.z, local); }

        void setAngularVelocity(decimal x, decimal y, decimal z, bool local = true);
        inline void setAngularVelocity(const glm_vec3& velocity, bool local = true) noexcept { setAngularVelocity(velocity.x, velocity.y, velocity.z, local); }

        void applyForce(decimal x, decimal y, decimal z, bool local = true);
        void applyForce(const glm_vec3& force, const glm_vec3& origin = glm_vec3(0.0f), bool local = true);

        void applyImpulse(decimal x, decimal y, decimal z, bool local = true);
        void applyImpulse(const glm_vec3& impulse, const glm_vec3& origin = glm_vec3(0.0f), bool local = true);

        void applyTorque(decimal x, decimal y, decimal z, bool local = true);
        inline void applyTorque(const glm_vec3& torque, bool local = true) noexcept { applyTorque(torque.x, torque.y, torque.z, local); }

        void applyTorqueImpulse(decimal x, decimal y, decimal z, bool local = true);
        inline void applyTorqueImpulse(const glm_vec3& torqueImpulse, bool local = true) noexcept { applyTorqueImpulse(torqueImpulse.x, torqueImpulse.y, torqueImpulse.z, local); }
};

class ComponentBody_System_CI : public Engine::priv::ECSSystemCI {
    public:
        ComponentBody_System_CI();
        ~ComponentBody_System_CI() = default;
};

namespace Engine::priv {
    class ComponentBody_System final : public Engine::priv::ECSSystem<ComponentBody> {
        class ParentChildVector final {
            private:
                inline uint32_t& getParent(uint32_t childID) noexcept {
                    return Parents[childID - 1U];
                }
                inline glm_mat4& getWorld(uint32_t ID) noexcept {
                    return WorldTransforms[ID - 1U];
                }
                inline glm_mat4& getLocal(uint32_t ID) noexcept {
                    return LocalTransforms[ID - 1U];
                }
                void reserve_from_insert(uint32_t parentID, uint32_t childID);
            public:
                std::vector<glm_mat4>         WorldTransforms;
                std::vector<glm_mat4>         LocalTransforms;
                std::vector<uint32_t>    Parents;
                std::vector<uint32_t>    Order;
                uint32_t                 OrderHead        = 0;

                void resize(size_t size);
                void reserve(size_t size);
                void insert(uint32_t parent, uint32_t child);
                void remove(uint32_t parent, uint32_t child);

                inline CONSTEXPR uint32_t size() const noexcept { return OrderHead; }
                inline size_t capacity() const noexcept { return Order.capacity(); }
        };
        public:
            ParentChildVector ParentChildSystem;

            ComponentBody_System(const SceneOptions& options, const Engine::priv::ECSSystemCI& systemCI, Engine::priv::sparse_set_base& inComponentPool);
            ~ComponentBody_System();
    };
};

#endif