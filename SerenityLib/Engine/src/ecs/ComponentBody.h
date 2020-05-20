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
    btCollisionObject* ownerCollisionObj;
    btCollisionObject* otherCollisionObj;
    size_t             ownerModelInstanceIndex;
    size_t             otherModelInstanceIndex;

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
    struct ComponentBody_UpdateFunction;
    struct ComponentBody_EntityAddedToSceneFunction;
    struct ComponentBody_ComponentAddedToEntityFunction;
    struct ComponentBody_ComponentRemovedFromEntityFunction;
    struct ComponentBody_SceneEnteredFunction;
    struct ComponentBody_SceneLeftFunction;
};

class ComponentBody : public EventObserver, public Engine::UserPointer {
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

    public:
        BOOST_TYPE_INDEX_REGISTER_CLASS
        ComponentBody(const Entity);
        ComponentBody(const Entity, const CollisionType::Type);

        ComponentBody& operator=(const ComponentBody& other) = delete;
        ComponentBody(const ComponentBody& other)            = delete;
        ComponentBody(ComponentBody&& other) noexcept;
        ComponentBody& operator=(ComponentBody&& other) noexcept;

        ~ComponentBody();

        const Entity getOwner() const;

        void onEvent(const Event& event_) override;

        const bool hasParent() const;

        void addChild(const Entity child) const;
        void addChild(const ComponentBody& child) const;
        void removeChild(const Entity child) const;
        void removeChild(const ComponentBody& child) const;


        template<typename T> void setCollisionFunctor(const T& functor) {
            m_CollisionFunctor = std::bind<void>(functor, std::placeholders::_1);
        }
        void collisionResponse(CollisionCallbackEventData& data) const;

        void rebuildRigidBody(const bool addBodyToPhysicsWorld = true, const bool threadSafe = false);

        void removePhysicsFromWorld(const bool force = true, const bool threadSafe = false);
        void addPhysicsToWorld(const bool force = true, const bool threadSafe = false);

        void setInternalPhysicsUserPointer(void* userPtr);
        void setUserPointer1(void* userPtr);
        void setUserPointer2(void* userPtr);
        void* getUserPointer1() const;
        void* getUserPointer2() const;

        const bool hasPhysics() const;
        const decimal getLinearDamping() const;
        const decimal getAngularDamping() const;
        const unsigned short getCollisionGroup() const; //get the groups this body belongs to
        const unsigned short getCollisionMask() const;  //get the groups this body will register collisions with
        const unsigned short getCollisionFlags() const;

        void alignTo(const glm_vec3& direction);

        void translate(const glm_vec3& translation, const bool local = true);
        void translate(const decimal& x, const decimal& y, const decimal& z, const bool local = true);
        void translate(const decimal& t, const bool local = true);

        void rotate(const glm_vec3& rotation, const bool local = true);
        void rotate(const decimal& pitch_radians, const decimal& yaw_radians, const decimal& roll_radians, const bool local = true);

        void scale(const glm_vec3& amount);
        void scale(const decimal& x, const decimal& y, const decimal& z);
        void scale(const decimal& s);

        void setPosition(const glm_vec3& newPosition);
        void setPosition(const decimal& x, const decimal& y, const decimal& z);
        void setPosition(const decimal& p);

        void setRotation(const glm_quat& newRotation);
        void setRotation(const decimal& quat_x, const decimal& quat_y, const decimal& quat_z, const decimal& quat_w);

        void setScale(const glm_vec3& newScale);
        void setScale(const decimal& x, const decimal& y, const decimal& z);
        void setScale(const decimal& s);

		const float mass() const;
        const decimal getDistance(const Entity other) const;
        const unsigned long long getDistanceLL(const Entity other) const;
        const glm::vec3 getScreenCoordinates(const bool clampToEdge = false) const;

        const ScreenBoxCoordinates getScreenBoxCoordinates(const float minOffset = 10.0f) const;

		glm_quat rotation() const;
		glm_vec3 getScale() const;
		glm_vec3 position() const;
        glm_vec3 localPosition() const;
        glm::vec3 position_render() const;
		const glm_vec3& forward() const;
		const glm_vec3& right() const;
		const glm_vec3& up() const;
		glm_vec3 getLinearVelocity() const;
		glm_vec3 getAngularVelocity() const;
		glm_mat4 modelMatrix() const;
        glm::mat4 modelMatrixRendering() const;
	    btRigidBody& getBtBody() const;

        void setCollision(const CollisionType::Type collisionType, const float mass);
        void setCollision(Collision* collision);
        Collision* getCollision() const;

        void setCollisionGroup(const short group);  //set the groups this body belongs to
        void setCollisionMask(const short mask); //set the groups this body will register collisions with
        void setCollisionFlag(const short flag);
        void setCollisionGroup(const CollisionFilter::Filter group);  //set the groups this body belongs to
        void setCollisionMask(const CollisionFilter::Filter mask); //set the groups this body will register collisions with
        void setCollisionFlag(const CollisionFlag::Flag flag);
        void addCollisionGroup(const short group);  //add to the groups this body belongs to
        void addCollisionMask(const short mask); //add to the groups this body will register collisions with
        void addCollisionFlag(const short flag);
        void addCollisionGroup(const CollisionFilter::Filter group); //add to the groups this body belongs to
        void addCollisionMask(const CollisionFilter::Filter mask); //add to the groups this body will register collisions with
        void addCollisionFlag(const CollisionFlag::Flag flag);
        void removeCollisionGroup(const short group);
        void removeCollisionMask(const short mask);
        void removeCollisionFlag(const short flag);
        void removeCollisionGroup(const CollisionFilter::Filter group);
        void removeCollisionMask(const CollisionFilter::Filter mask);
        void removeCollisionFlag(const CollisionFlag::Flag flag);

        void setDamping(const decimal& linear, const decimal& angular);

        void setDynamic(const bool dynamic);
        void setMass(const float mass);
        void setGravity(const decimal& x, const decimal& y, const decimal& z);

        void clearLinearForces();
        void clearAngularForces();
        void clearAllForces();

        void setLinearVelocity(const decimal& x, const decimal& y, const decimal& z, const bool local = true);
        void setLinearVelocity(const glm_vec3& velocity, const bool local = true);

        void setAngularVelocity(const decimal& x, const decimal& y, const decimal& z, const bool local = true);
        void setAngularVelocity(const glm_vec3& velocity, const bool local = true);

        void applyForce(const decimal& x, const decimal& y, const decimal& z, const bool local = true);
        void applyForce(const glm_vec3& force, const glm_vec3& origin = glm_vec3(0.0f), const bool local = true);

        void applyImpulse(const decimal& x, const decimal& y, const decimal& z, const bool local = true);
        void applyImpulse(const glm_vec3& impulse, const glm_vec3& origin = glm_vec3(0.0f), const bool local = true);

        void applyTorque(const decimal& x, const decimal& y, const decimal& z, const bool local = true);
        void applyTorque(const glm_vec3& torque, const bool local = true);

        void applyTorqueImpulse(const decimal& x, const decimal& y, const decimal& z, const bool local = true);
        void applyTorqueImpulse(const glm_vec3& torqueImpulse, const bool local = true);
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
                inline std::uint32_t& getParent(const std::uint32_t childID) {
                    return Parents[childID - 1U];
                }
                inline glm_mat4& getWorld(const std::uint32_t ID) {
                    return WorldTransforms[ID - 1U];
                }
                inline glm_mat4& getLocal(const std::uint32_t ID) {
                    return LocalTransforms[ID - 1U];
                }

                void reserve_from_insert(const std::uint32_t parentID, const std::uint32_t childID);

            public:
                std::vector<glm_mat4>         WorldTransforms;
                std::vector<glm_mat4>         LocalTransforms;
                std::vector<std::uint32_t>    Parents;
                std::vector<std::uint32_t>    Order;
                std::uint32_t                 OrderHead        = 0;

                void resize(const size_t size);
                void reserve(const size_t size);
                void insert(const std::uint32_t parent, const std::uint32_t child);
                void remove(const std::uint32_t parent, const std::uint32_t child);

                const std::uint32_t size() const;
                const size_t capacity() const;
        };
        public:
            ParentChildVector ParentChildSystem;

            ComponentBody_System(const Engine::priv::ECSSystemCI& systemCI, Engine::priv::ECS<Entity>& ecs);
            ~ComponentBody_System();
    };
};

#endif