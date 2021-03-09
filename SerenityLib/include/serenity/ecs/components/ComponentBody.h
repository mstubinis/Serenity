#pragma once
#ifndef ENGINE_ECS_COMPONENT_BODY_H
#define ENGINE_ECS_COMPONENT_BODY_H

struct SceneOptions;
class  Collision;
class  ComponentModel;
class  ComponentBody;
class  btCollisionObject;
class  btRigidBody;
class  SystemComponentBody;
namespace Engine::priv {
    class  sparse_set_base;
};

#include <serenity/dependencies/glm.h>
#include <serenity/ecs/ECS.h>
#include <serenity/physics/PhysicsIncludes.h>
#include <LinearMath/btDefaultMotionState.h>
#include <serenity/ecs/entity/Entity.h>
#include <serenity/events/Observer.h>
#include <serenity/ecs/components/ComponentBaseClass.h>


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
    btCollisionObject* ownerCollisionObj       = nullptr;
    btCollisionObject* otherCollisionObj       = nullptr;
    uint32_t           ownerModelInstanceIndex = 0;
    uint32_t           otherModelInstanceIndex = 0;

    CollisionCallbackEventData() = delete;
    CollisionCallbackEventData(const CollisionCallbackEventData&)                = delete;
    CollisionCallbackEventData& operator=(const CollisionCallbackEventData&)     = delete;
    CollisionCallbackEventData(CollisionCallbackEventData&&) noexcept            = delete;
    CollisionCallbackEventData& operator=(CollisionCallbackEventData&&) noexcept = delete;

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
    glm::vec2 topLeft       = glm::vec2{ 0.0f, 0.0f };
    glm::vec2 topRight      = glm::vec2{ 0.0f, 0.0f };
    glm::vec2 bottomLeft    = glm::vec2{ 0.0f, 0.0f };
    glm::vec2 bottomRight   = glm::vec2{ 0.0f, 0.0f };
};

class ComponentBody : public ComponentBaseClass<ComponentBody> {
    friend class  ComponentModel;
    friend class  SystemComponentBody;
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
            PhysicsData(const PhysicsData&)                  = delete;
            PhysicsData& operator=(const PhysicsData&)       = delete;
            PhysicsData(PhysicsData&&) noexcept; 
            PhysicsData& operator=(PhysicsData&&) noexcept;
            ~PhysicsData();
        };
        struct NormalData final {
            glm_quat rotation       = glm_quat{ 1.0, 0.0, 0.0, 0.0 };
            glm_vec3 scale          = glm_vec3{ 1.0 };
            glm_vec3 position       = glm_vec3{ 0.0 };
            glm_vec3 linearVelocity = glm_vec3{ 0.0 };

            NormalData() = default;
            NormalData(const NormalData&)                  = delete;
            NormalData& operator=(const NormalData&)       = delete;
            NormalData(NormalData&&) noexcept;
            NormalData& operator=(NormalData&&) noexcept;
            ~NormalData() = default;
        };

        glm_vec3              m_Forward          = glm_vec3{ 0.0, 0.0, -1.0 };
        glm_vec3              m_Right            = glm_vec3{ 1.0, 0.0, 0.0 };
        glm_vec3              m_Up               = glm_vec3{ 0.0, 1.0, 0.0 };
        CollisionCallbackFP   m_CollisionFunctor = [](CollisionCallbackEventData&) {};
        //union {
        std::unique_ptr<NormalData>       n;
        std::unique_ptr<PhysicsData>      p;
        //};
        void*                 m_UserPointer      = nullptr;
        void*                 m_UserPointer1     = nullptr;
        void*                 m_UserPointer2     = nullptr;
        Entity                m_Owner            = Entity{};
        bool                  m_Physics          = false;

        btVector3 internal_activate_and_get_vector(decimal x, decimal y, decimal z, bool local) noexcept;
        void internal_update_misc() noexcept;
        static void internal_recalculateAllParentChildMatrices(SystemComponentBody& system);
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

        [[nodiscard]] inline constexpr Entity getOwner() const noexcept { return m_Owner; }

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
        void setUserPointer(void* userPtr) noexcept { m_UserPointer = userPtr; }
        void setUserPointer1(void* userPtr) noexcept { m_UserPointer1 = userPtr; }
        void setUserPointer2(void* userPtr) noexcept { m_UserPointer2 = userPtr; }
        [[nodiscard]] inline constexpr void* getUserPointer() const noexcept { return m_UserPointer; }
        [[nodiscard]] inline constexpr void* getUserPointer1() const noexcept { return m_UserPointer1; }
        [[nodiscard]] inline constexpr void* getUserPointer2() const noexcept { return m_UserPointer2; }

        [[nodiscard]] inline constexpr bool hasPhysics() const noexcept { return m_Physics; }
        [[nodiscard]] decimal getLinearDamping() const;
        [[nodiscard]] decimal getAngularDamping() const;

        [[nodiscard]] inline MaskType getCollisionGroup() const noexcept { return (m_Physics) ? p->group : 0; }
        [[nodiscard]] inline MaskType getCollisionMask() const noexcept { return (m_Physics) ? p->mask : 0; }

        [[nodiscard]] MaskType getCollisionFlags() const;

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

        [[nodiscard]] inline float mass() const noexcept { return (m_Physics) ? p->mass : 0.0f; }
        [[nodiscard]] decimal getDistance(Entity other) const;
        [[nodiscard]] unsigned long long getDistanceLL(Entity other) const;
        [[nodiscard]] glm::vec3 getScreenCoordinates(bool clampToEdge = false) const;

        [[nodiscard]] ScreenBoxCoordinates getScreenBoxCoordinates(float minOffset = 10.0f) const;

        [[nodiscard]] glm_quat getRotation() const;
        [[nodiscard]] glm_vec3 getScale() const;
        [[nodiscard]] glm_vec3 getPosition() const;
        [[nodiscard]] glm_vec3 getLocalPosition() const;
        
        [[nodiscard]] inline constexpr const glm_vec3& forward() const noexcept { return m_Forward; }
        [[nodiscard]] inline constexpr const glm_vec3& right() const noexcept { return m_Right; }
        [[nodiscard]] inline constexpr const glm_vec3& up() const noexcept { return m_Up; }
        
        [[nodiscard]] glm_vec3 getLinearVelocity() const;
        [[nodiscard]] glm_vec3 getAngularVelocity() const;
        [[nodiscard]] glm_mat4 modelMatrix() const;
        [[nodiscard]] inline glm::mat4 modelMatrixRendering() const noexcept { return (glm::mat4)modelMatrix(); }
        [[nodiscard]] inline btRigidBody& getBtBody() const noexcept { return *p->bullet_rigidBody; }

        void setCollision(CollisionType collisionType, float mass);

        //double check this...
        template<typename ... ARGS> Collision& setCollision(ARGS&&... args) {
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
        [[nodiscard]] inline Engine::view_ptr<Collision> getCollision() const noexcept { return (m_Physics) ? p->collision.get() : nullptr; }

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

#endif
