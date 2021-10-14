#pragma once
#ifndef ENGINE_ECS_COMPONENT_RIGID_BODY_H
#define ENGINE_ECS_COMPONENT_RIGID_BODY_H

struct SceneOptions;
class  ComponentModel;
class  ComponentRigidBody;
class  ComponentTransform;
class  ComponentCollisionShape;
class  btCollisionObject;
class  btRigidBody;
class  btRigidBodyEnhanced;
class  SystemComponentRigidBody;
class  SystemTransformParentChild;
class  SystemSyncTransformToRigid;
class  SystemSyncRigidToTransform;
namespace Engine::priv {
    class  sparse_set_base;
};

#include <serenity/dependencies/glm.h>
#include <serenity/ecs/ECS.h>
#include <serenity/physics/PhysicsIncludes.h>
#include <LinearMath/btDefaultMotionState.h>
#include <serenity/events/Observer.h>
#include <serenity/ecs/components/ComponentBaseClass.h>
#include <serenity/renderer/RendererIncludes.h>

#ifdef ENGINE_RIGID_BODY_ENHANCED
#include <serenity/physics/btRigidBodyEnhanced.h>
using btRigidBodyType = btRigidBodyEnhanced;
#else
#include <BulletDynamics/Dynamics/btRigidBody.h>
using btRigidBodyType = btRigidBody;
#endif

struct RigidCollisionCallbackEventData final {
    ComponentRigidBody&  ownerRigid;
    ComponentRigidBody&  otherRigid;
    glm::vec3&           ownerHit;
    glm::vec3&           otherHit;
    glm::vec3&           ownerLocalHit;
    glm::vec3&           otherLocalHit;
    glm::vec3&           normalOnB;
    glm::vec3&           normalFromA;
    glm::vec3&           normalFromB;
    btCollisionObject*   btOwnerCollisionObj        = nullptr;
    btCollisionObject*   btOtherCollisionObj        = nullptr;
    btCollisionShape*    btOwnerCollisionShapeRoot  = nullptr;
    btCollisionShape*    btOtherCollisionShapeRoot  = nullptr;
    btCollisionShape*    btOwnerCollisionShape      = nullptr;
    btCollisionShape*    btOtherCollisionShape      = nullptr;
    uint32_t             ownerModelInstanceIndex    = 0;
    uint32_t             otherModelInstanceIndex    = 0;

    RigidCollisionCallbackEventData() = delete;
    RigidCollisionCallbackEventData(const RigidCollisionCallbackEventData&) = delete;
    RigidCollisionCallbackEventData& operator=(const RigidCollisionCallbackEventData&) = delete;
    RigidCollisionCallbackEventData(RigidCollisionCallbackEventData&&) noexcept = delete;
    RigidCollisionCallbackEventData& operator=(RigidCollisionCallbackEventData&&) noexcept = delete;

    RigidCollisionCallbackEventData(ComponentRigidBody& ownerRigid_, ComponentRigidBody& otherRigid_, glm::vec3& ownerHit_, glm::vec3& otherHit_, glm::vec3& normal_,
        glm::vec3& ownerLocalHit_, glm::vec3& otherLocalHit_, glm::vec3& normalFromA_, glm::vec3& normalFromB_
    )
        : ownerRigid{ ownerRigid_ }
        , otherRigid{ otherRigid_ }
        , ownerHit{ ownerHit_ }
        , otherHit{ otherHit_ }
        , normalOnB{ normal_ }
        , ownerLocalHit{ ownerLocalHit_ }
        , otherLocalHit{ otherLocalHit_ }
        , normalFromA{ normalFromA_ }
        , normalFromB{ normalFromB_ }
    {}
};

class ComponentRigidBody final : public ComponentBaseClass<ComponentRigidBody> {
    friend class  ComponentModel;
    friend class  SystemComponentRigidBody;
    friend class  ComponentCollisionShape;
    friend class  SystemTransformParentChild;
    friend class  SystemSyncTransformToRigid;
    friend class  SystemSyncRigidToTransform;
    using CollisionCallbackFPRigid = void(*)(RigidCollisionCallbackEventData&);
    public:
        class Flags {
            public:
                enum Type : int32_t {
                    DISABLE_WORLD_GRAVITY = 1,
                    ///ENABLE_GYROPSCOPIC_FORCE flags is enabled by default in Bullet 2.83 and onwards.
                    ///and it ENABLE_GYROPSCOPIC_FORCE becomes equivalent to ENABLE_GYROSCOPIC_FORCE_IMPLICIT_BODY
                    ///See Demos/GyroscopicDemo and computeGyroscopicImpulseImplicit
                    ENABLE_GYROSCOPIC_FORCE_EXPLICIT = 2,
                    ENABLE_GYROSCOPIC_FORCE_IMPLICIT_WORLD = 4,
                    ENABLE_GYROSCOPIC_FORCE_IMPLICIT_BODY = 8,
                    ENABLE_GYROPSCOPIC_FORCE = BT_ENABLE_GYROSCOPIC_FORCE_IMPLICIT_BODY,
                };
                BUILD_ENUM_CLASS_MEMBERS(Flags, Type)
        };
    private:
        CollisionCallbackFPRigid              m_CollisionFunctor  = [](RigidCollisionCallbackEventData&) {};
        std::unique_ptr<btRigidBodyType>      m_BulletRigidBody;
        btDefaultMotionState                  m_BulletMotionState = btDefaultMotionState{ btTransform{ btQuaternion{0, 0, 0, 1} } };
        void*                                 m_UserPointer       = nullptr;
        void*                                 m_UserPointer1      = nullptr;
        void*                                 m_UserPointer2      = nullptr;
        Entity                                m_Owner;
        float                                 m_Mass              = 0.0f;
        MaskType                              m_Group             = CollisionFilter::DefaultFilter;
        MaskType                              m_Mask              = CollisionFilter::AllFilter;

        btVector3 internal_activate_and_get_vector(decimal x, decimal y, decimal z, bool local) noexcept;
        void internal_update_misc() noexcept;
        void internal_set_matrix(const glm_mat4&);

        glm::vec3 internal_getScale() const noexcept;
        void internal_setScale(float x, float y, float z);
        inline void internal_setScale(const glm::vec3& scale) noexcept { internal_setScale(scale.x, scale.y, scale.z); }

        void internal_calculate_mass();

        btTransform internal_get_bt_transform() const;
        btTransform internal_get_bt_transform_motion_state() const;

        void cleanup();
        //bool removePhysicsFromWorldImmediate();
        //bool addPhysicsToWorldImmediate();
    public:
        ComponentRigidBody(Entity, const std::string& name = {});
        ComponentRigidBody(Entity, CollisionFilter group, CollisionFilter mask, const std::string& name = {});
        ComponentRigidBody(const ComponentRigidBody&)            = delete;
        ComponentRigidBody& operator=(const ComponentRigidBody&) = delete;
        ComponentRigidBody(ComponentRigidBody&&) noexcept;
        ComponentRigidBody& operator=(ComponentRigidBody&&) noexcept;
        ~ComponentRigidBody();

        [[nodiscard]] inline Entity getOwner() const noexcept { return m_Owner; }
        [[nodiscard]] inline float getMass() const noexcept { return m_Mass; }

        [[nodiscard]] ComponentRigidBody::Flags getFlags() const noexcept;
        void setFlags(ComponentRigidBody::Flags);
        void addFlags(ComponentRigidBody::Flags);
        void removeFlags(ComponentRigidBody::Flags);

        //immediately syncs physics object to graphics object without waiting for it to occur normally for this frame
        void forcePhysicsSync() noexcept;

        #ifdef ENGINE_RIGID_BODY_ENHANCED
            inline void setBtName(const std::string& name) { m_BulletRigidBody->setName(name); }
            [[nodiscard]] inline const std::string& getName() const noexcept { return m_BulletRigidBody->getName(); }
        #endif

        template<class FUNC> inline void setCollisionFunctor(FUNC&& functor) noexcept { m_CollisionFunctor = std::forward<FUNC>(functor); }

        glm_vec3 getPosition() const;
        glm::quat getRotation() const;
        glm_vec3 getPositionMotionState() const;
        glm::quat getRotationMotionState() const;
        glm_mat4 getWorldMatrix() const;
        glm_mat4 getWorldMatrixMotionState() const;

        void collisionResponse(RigidCollisionCallbackEventData&) const;

        bool rebuildRigidBody(bool addBodyToPhysicsWorld = true);
        bool removePhysicsFromWorld();
        bool addPhysicsToWorld();

        [[nodiscard]] inline int getBTObjectUserIndex() const noexcept { return m_BulletRigidBody->getUserIndex(); }
        [[nodiscard]] inline int getBTObjectUserIndex2() const noexcept { return m_BulletRigidBody->getUserIndex2(); }
        [[nodiscard]] inline int getBTObjectUserIndex3() const noexcept { return m_BulletRigidBody->getUserIndex2(); }

        inline void setBTObjectUserIndex(int idx) const noexcept { m_BulletRigidBody->setUserIndex(idx); }
        inline void setBTObjectUserIndex2(int idx) const noexcept { m_BulletRigidBody->setUserIndex2(idx); }
        inline void setBTObjectUserIndex3(int idx) const noexcept { m_BulletRigidBody->setUserIndex3(idx); }

        void setUserPtr(void* userPtr) noexcept { m_UserPointer = userPtr; }
        void setUserPtr1(void* userPtr) noexcept { m_UserPointer1 = userPtr; }
        void setUserPtr2(void* userPtr) noexcept { m_UserPointer2 = userPtr; }

        [[nodiscard]] inline void* getUserPtr() const noexcept { return m_UserPointer; }
        [[nodiscard]] inline void* getUserPtr1() const noexcept { return m_UserPointer1; }
        [[nodiscard]] inline void* getUserPtr2() const noexcept { return m_UserPointer2; }

        [[nodiscard]] decimal getLinearDamping() const;
        [[nodiscard]] decimal getAngularDamping() const;

        [[nodiscard]] inline MaskType getCollisionGroup() const noexcept { return m_Group; }
        [[nodiscard]] inline MaskType getCollisionMask() const noexcept { return m_Mask; }

        [[nodiscard]] MaskType getCollisionFlags() const;

        [[nodiscard]] glm_vec3 getLinearVelocity() const;
        [[nodiscard]] glm_vec3 getAngularVelocity() const;
        [[nodiscard]] inline btRigidBody* getBtBody() const noexcept { return m_BulletRigidBody.get(); }

        void setCollisionGroup(CollisionFilter group) noexcept;
        void setCollisionMask(CollisionFilter mask) noexcept;
        void setCollisionFlag(CollisionFlag flag) noexcept;
        void addCollisionGroup(CollisionFilter group) noexcept;
        void addCollisionMask(CollisionFilter mask) noexcept;
        void addCollisionFlag(CollisionFlag flag) noexcept;
        void removeCollisionGroup(CollisionFilter group) noexcept;
        void removeCollisionMask(CollisionFilter mask) noexcept;
        void removeCollisionFlag(CollisionFlag flag) noexcept;
        void setCollisionGroupAndMask(CollisionFilter group, CollisionFilter mask) noexcept;

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
        void applyTorqueX(decimal x, bool local = true);
        void applyTorqueY(decimal y, bool local = true);
        void applyTorqueZ(decimal z, bool local = true);
        inline void applyTorque(const glm_vec3& torque, bool local = true) noexcept { applyTorque(torque.x, torque.y, torque.z, local); }

        void applyTorqueImpulse(decimal x, decimal y, decimal z, bool local = true);
        inline void applyTorqueImpulse(const glm_vec3& torqueImpulse, bool local = true) noexcept { applyTorqueImpulse(torqueImpulse.x, torqueImpulse.y, torqueImpulse.z, local); }
};

#endif
