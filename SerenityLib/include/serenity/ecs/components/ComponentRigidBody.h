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
    btCollisionObject*   ownerCollisionObj        = nullptr;
    btCollisionObject*   otherCollisionObj        = nullptr;
    btCollisionShape*    ownerCollisionShapeRoot  = nullptr;
    btCollisionShape*    otherCollisionShapeRoot  = nullptr;
    btCollisionShape*    ownerCollisionShape      = nullptr;
    btCollisionShape*    otherCollisionShape      = nullptr;
    uint32_t             ownerModelInstanceIndex  = 0;
    uint32_t             otherModelInstanceIndex  = 0;

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

class ComponentRigidBody : public ComponentBaseClass<ComponentRigidBody> {
    friend class  ComponentModel;
    friend class  SystemComponentRigidBody;
    friend class  ComponentCollisionShape;
    friend class  SystemTransformParentChild;
    friend class  SystemSyncTransformToRigid;
    friend class  SystemSyncRigidToTransform;
    using CollisionCallbackFPRigid = void(*)(RigidCollisionCallbackEventData&);
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

        bool removePhysicsFromWorldImmediate();
        bool addPhysicsToWorldImmediate();
    public:
        ComponentRigidBody(Entity);
        ComponentRigidBody(const ComponentRigidBody&)            = delete;
        ComponentRigidBody& operator=(const ComponentRigidBody&) = delete;
        ComponentRigidBody(ComponentRigidBody&&) noexcept;
        ComponentRigidBody& operator=(ComponentRigidBody&&) noexcept;
        ~ComponentRigidBody();

        [[nodiscard]] inline Entity getOwner() const noexcept { return m_Owner; }
        [[nodiscard]] inline float getMass() const noexcept { return m_Mass; }

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
        void setCollisionGroupAndMask(MaskType group, MaskType mask);

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

#endif