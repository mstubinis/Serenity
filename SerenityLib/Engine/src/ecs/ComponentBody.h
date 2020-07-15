#pragma once
#ifndef ENGINE_ECS_COMPONENT_BODY_H
#define ENGINE_ECS_COMPONENT_BODY_H

class Collision;
class ComponentModel;
class ComponentBody;
class btCollisionObject;
class btRigidBody;

#include <ecs/ECS.h>
#include <core/engine/physics/PhysicsIncludes.h>
#include <LinearMath/btDefaultMotionState.h>
#include <ecs/Entity.h>

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
    size_t             ownerModelInstanceIndex = 0;
    size_t             otherModelInstanceIndex = 0;

    CollisionCallbackEventData(ComponentBody& ownerBody_, ComponentBody& otherBody_, glm::vec3& ownerHit_, glm::vec3& otherHit_, glm::vec3& normal_, glm::vec3& ownerLocalHit_, glm::vec3& otherLocalHit_, glm::vec3& normalFromA_, glm::vec3& normalFromB_);
};

struct ScreenBoxCoordinates {
    bool      inBounds;
    glm::vec2 topLeft;
    glm::vec2 topRight;
    glm::vec2 bottomLeft;
    glm::vec2 bottomRight;
};

namespace Engine::priv {
    class  ComponentBody_System;
    struct ComponentBody_UpdateFunction;
    struct ComponentBody_EntityAddedToSceneFunction;
    struct ComponentBody_ComponentAddedToEntityFunction;
    struct ComponentBody_ComponentRemovedFromEntityFunction;
    struct ComponentBody_SceneEnteredFunction;
    struct ComponentBody_SceneLeftFunction;
};

class ComponentBody : public Observer, public Engine::UserPointer {
    friend class  Engine::priv::ComponentBody_System;
    friend struct Engine::priv::ComponentBody_UpdateFunction;
    friend struct Engine::priv::ComponentBody_ComponentAddedToEntityFunction;
    friend struct Engine::priv::ComponentBody_ComponentRemovedFromEntityFunction;
    friend struct Engine::priv::ComponentBody_EntityAddedToSceneFunction;
    friend struct Engine::priv::ComponentBody_SceneEnteredFunction;
    friend struct Engine::priv::ComponentBody_SceneLeftFunction;
    friend class  ComponentModel;
    private:
        struct PhysicsData final {
            Collision*           collision              = nullptr;
            btRigidBody*         bullet_rigidBody       = nullptr;
            btDefaultMotionState bullet_motionState;
            float                mass                   = 0.0f;
            unsigned short       group                  = CollisionFilter::DefaultFilter;;
            unsigned short       mask                   = CollisionFilter::AllFilter;
            bool                 forcedOut              = false;

            PhysicsData() = default;
            ~PhysicsData();

            PhysicsData(const PhysicsData& other)            = delete;
            PhysicsData& operator=(const PhysicsData& other) = delete;
            PhysicsData& operator=(PhysicsData&& other) noexcept;
            PhysicsData(PhysicsData&& other) noexcept;        
        };
        struct NormalData final {
            glm_vec3 scale          = glm_vec3(1.0);
            glm_vec3 position       = glm_vec3(0.0);
            glm_quat rotation       = glm_quat(1.0, 0.0, 0.0, 0.0);
            glm_vec3 linearVelocity = glm_vec3(0.0);

            NormalData() = default;
            ~NormalData() = default;

            NormalData(const NormalData& other)            = delete;
            NormalData& operator=(const NormalData& other) = delete;
            NormalData& operator=(NormalData&& other) noexcept;
            NormalData(NormalData&& other) noexcept;
        };
        union {
            NormalData*  n   = nullptr;
            PhysicsData* p;
        } data;
        bool  m_Physics      = false;
        void* m_UserPointer1 = nullptr;
        void* m_UserPointer2 = nullptr;

        glm_vec3 m_Forward   = glm_vec3(0.0,  0.0, -1.0);
        glm_vec3 m_Right     = glm_vec3(1.0,  0.0,  0.0);
        glm_vec3 m_Up        = glm_vec3(0.0,  1.0,  0.0);

        Entity m_Owner;

        std::function<void(CollisionCallbackEventData& data)> m_CollisionFunctor = [](CollisionCallbackEventData&) {};

        static void internal_recalculateAllParentChildMatrices(Engine::priv::ComponentBody_System& system);
    public:
        static void recalculateAllParentChildMatrices(Scene& scene);

        //BOOST_TYPE_INDEX_REGISTER_CLASS
        ComponentBody(Entity entity);
        ComponentBody(Entity entity, CollisionType::Type collisionType);

        ComponentBody& operator=(const ComponentBody& other) = delete;
        ComponentBody(const ComponentBody& other)            = delete;
        ComponentBody(ComponentBody&& other) noexcept;
        ComponentBody& operator=(ComponentBody&& other) noexcept;

        ~ComponentBody();

        constexpr Entity getOwner() const noexcept { return m_Owner; }

        void onEvent(const Event& event_) override;

        bool hasParent() const;

        void addChild(Entity child) const;
        void addChild(const ComponentBody& child) const;
        void removeChild(Entity child) const;
        void removeChild(const ComponentBody& child) const;


        void setCollisionFunctor(std::function<void(CollisionCallbackEventData& data)> functor);
        void collisionResponse(CollisionCallbackEventData& data) const;

        void rebuildRigidBody(bool addBodyToPhysicsWorld = true, bool threadSafe = false);
        void removePhysicsFromWorld(bool force = true, bool threadSafe = false);
        void addPhysicsToWorld(bool force = true, bool threadSafe = false);

        void setInternalPhysicsUserPointer(void* userPtr);
        void setUserPointer1(void* userPtr) noexcept { m_UserPointer1 = userPtr; }
        void setUserPointer2(void* userPtr) noexcept { m_UserPointer2 = userPtr; }
        constexpr void* getUserPointer1() const noexcept { return m_UserPointer1; }
        constexpr void* getUserPointer2() const noexcept { return m_UserPointer2; }

        bool hasPhysics() const { return m_Physics; }
        decimal getLinearDamping() const;
        decimal getAngularDamping() const;
        unsigned short getCollisionGroup() const; //get the groups this body belongs to
        unsigned short getCollisionMask() const;  //get the groups this body will register collisions with
        unsigned short getCollisionFlags() const;

        void alignTo(const glm_vec3& direction);

        void translate(const glm_vec3& translation, bool local = true);
        void translate(decimal x, decimal y, decimal z, bool local = true);
        void translate(decimal t, bool local = true);

        void rotate(const glm_vec3& rotation, bool local = true);
        void rotate(decimal pitch_radians, decimal yaw_radians, decimal roll_radians, bool local = true);

        void scale(const glm_vec3& amount);
        void scale(decimal x, decimal y, decimal z);
        void scale(decimal s);

        void setPosition(const glm_vec3& newPosition);
        void setPosition(decimal x, decimal y, decimal z);
        void setPosition(decimal p);

        void setRotation(const glm_quat& newRotation);
        void setRotation(decimal quat_x, decimal quat_y, decimal quat_z, decimal quat_w);

        void setScale(const glm_vec3& newScale);
        void setScale(decimal x, decimal y, decimal z);
        void setScale(decimal s);

	    float mass() const;
        decimal getDistance(Entity other) const;
        unsigned long long getDistanceLL(Entity other) const;
        glm::vec3 getScreenCoordinates(bool clampToEdge = false) const;

        ScreenBoxCoordinates getScreenBoxCoordinates(float minOffset = 10.0f) const;

		glm_quat getRotation() const;
		glm_vec3 getScale() const;
		glm_vec3 getPosition() const;
        glm_vec3 getLocalPosition() const;
        glm::vec3 getPositionRender() const;
        const glm_vec3& forward() const { return m_Forward; }
        const glm_vec3& right() const { return m_Right; }
        const glm_vec3& up() const { return m_Up; }
		glm_vec3 getLinearVelocity() const;
		glm_vec3 getAngularVelocity() const;
		glm_mat4 modelMatrix() const;
        glm::mat4 modelMatrixRendering() const;
	    btRigidBody& getBtBody() const;

        void setCollision(CollisionType::Type collisionType, float mass);
        void setCollision(Collision* collision);
        Collision* getCollision() const;

        void setCollisionGroup(short group);  //set the groups this body belongs to
        void setCollisionMask(short mask); //set the groups this body will register collisions with
        void setCollisionFlag(short flag);
        void setCollisionGroup(CollisionFilter::Filter group);  //set the groups this body belongs to
        void setCollisionMask(CollisionFilter::Filter mask); //set the groups this body will register collisions with
        void setCollisionFlag(CollisionFlag::Flag flag);
        void addCollisionGroup(short group);  //add to the groups this body belongs to
        void addCollisionMask(short mask); //add to the groups this body will register collisions with
        void addCollisionFlag(short flag);
        void addCollisionGroup(CollisionFilter::Filter group); //add to the groups this body belongs to
        void addCollisionMask(CollisionFilter::Filter mask); //add to the groups this body will register collisions with
        void addCollisionFlag(CollisionFlag::Flag flag);
        void removeCollisionGroup(short group);
        void removeCollisionMask(short mask);
        void removeCollisionFlag(short flag);
        void removeCollisionGroup(CollisionFilter::Filter group);
        void removeCollisionMask(CollisionFilter::Filter mask);
        void removeCollisionFlag(CollisionFlag::Flag flag);

        void setDamping(decimal linear, decimal angular);

        void setDynamic(bool dynamic);
        void setMass(float mass);
        void setGravity(decimal x, decimal y, decimal z);

        void clearLinearForces();
        void clearAngularForces();
        void clearAllForces();

        void setLinearVelocity(decimal x, decimal y, decimal z, bool local = true);
        void setLinearVelocity(const glm_vec3& velocity, bool local = true);

        void setAngularVelocity(decimal x, decimal y, decimal z, bool local = true);
        void setAngularVelocity(const glm_vec3& velocity, bool local = true);

        void applyForce(decimal x, decimal y, decimal z, bool local = true);
        void applyForce(const glm_vec3& force, const glm_vec3& origin = glm_vec3(0.0f), bool local = true);

        void applyImpulse(decimal x, decimal y, decimal z, bool local = true);
        void applyImpulse(const glm_vec3& impulse, const glm_vec3& origin = glm_vec3(0.0f), bool local = true);

        void applyTorque(decimal x, decimal y, decimal z, bool local = true);
        void applyTorque(const glm_vec3& torque, bool local = true);

        void applyTorqueImpulse(decimal x, decimal y, decimal z, bool local = true);
        void applyTorqueImpulse(const glm_vec3& torqueImpulse, bool local = true);
};

class ComponentBody_System_CI : public Engine::priv::ECSSystemCI {
    public:
        ComponentBody_System_CI();
        ~ComponentBody_System_CI() = default;
};

namespace Engine::priv {
    class ComponentBody_System final : public Engine::priv::ECSSystem<Entity, ComponentBody> {
        class ParentChildVector final {
            private:
                inline std::uint32_t& getParent(std::uint32_t childID) {
                    return Parents[childID - 1U];
                }
                inline glm_mat4& getWorld(std::uint32_t ID) {
                    return WorldTransforms[ID - 1U];
                }
                inline glm_mat4& getLocal(std::uint32_t ID) {
                    return LocalTransforms[ID - 1U];
                }
                void reserve_from_insert(std::uint32_t parentID, std::uint32_t childID);
            public:
                std::vector<glm_mat4>         WorldTransforms;
                std::vector<glm_mat4>         LocalTransforms;
                std::vector<std::uint32_t>    Parents;
                std::vector<std::uint32_t>    Order;
                std::uint32_t                 OrderHead        = 0;

                void resize(size_t size);
                void reserve(size_t size);
                void insert(std::uint32_t parent, std::uint32_t child);
                void remove(std::uint32_t parent, std::uint32_t child);

                std::uint32_t size() const;
                size_t capacity() const;
        };
        public:
            ParentChildVector ParentChildSystem;

            ComponentBody_System(const Engine::priv::ECSSystemCI& systemCI, Engine::priv::ECS<Entity>& ecs);
            ~ComponentBody_System();
    };
};

#endif