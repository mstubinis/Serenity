#pragma once
#ifndef ENGINE_ECS_ENTITY_RAII_BODY_RIGID_H
#define ENGINE_ECS_ENTITY_RAII_BODY_RIGID_H

#include <serenity/ecs/entity/EntityBodyRigid.h>

class EntityRAIIBodyRigid final {
    private:
        EntityBodyRigid m_Entity;
    public:
        EntityRAIIBodyRigid() = default;
        EntityRAIIBodyRigid(Scene& scene)
            : m_Entity{ scene }
        {}
        EntityRAIIBodyRigid(const EntityBodyRigid& other)
            : m_Entity{ other }
        {}
        EntityRAIIBodyRigid(const Entity& other)
            : m_Entity{ other }
        {}
        EntityRAIIBodyRigid(uint32_t entityID, uint32_t sceneID, uint32_t versionID)
            : m_Entity{ entityID, sceneID, versionID }
        {}
        EntityRAIIBodyRigid(const EntityRAIIBodyRigid& other) = delete;
        EntityRAIIBodyRigid& operator=(const EntityRAIIBodyRigid& other) = delete;
        EntityRAIIBodyRigid(EntityRAIIBodyRigid&& other) noexcept
            : m_Entity{ std::exchange(other.m_Entity, EntityBodyRigid{}) }
        {}
        EntityRAIIBodyRigid& operator=(EntityRAIIBodyRigid&& other) noexcept {
            m_Entity = std::exchange(other.m_Entity, EntityBodyRigid{});
            return *this;
        }
        ~EntityRAIIBodyRigid() {
            m_Entity.destroy();
        }

        [[nodiscard]] inline uint32_t id() const noexcept { return m_Entity.id(); }
        [[nodiscard]] inline uint32_t sceneID() const noexcept { return m_Entity.sceneID(); }
        [[nodiscard]] inline uint32_t versionID() const noexcept { return m_Entity.versionID(); }

        [[nodiscard]] inline bool null() const noexcept { return m_Entity.null(); }
        [[nodiscard]] inline bool isDestroyed() const noexcept { return m_Entity.isDestroyed(); }

        inline operator Entity() const noexcept { return m_Entity; }
        inline operator EntityBodyRigid() const noexcept { return m_Entity; }

        [[nodiscard]] inline Engine::view_ptr<Scene> scene() const noexcept { return m_Entity.scene(); }
        [[nodiscard]] inline bool hasParent() const noexcept { return m_Entity.hasParent(); }

        inline void addChild(Entity child) const noexcept { m_Entity.addChild(child); }
        inline void removeChild(Entity child) const noexcept { m_Entity.removeChild(child); }
        //inline void removeAllChildren() const noexcept { m_Entity.removeAllChildren(); }

        template<typename T, typename... ARGS> inline bool addComponent(ARGS&&... args) noexcept {
            return m_Entity.addComponent<T>(std::forward<ARGS>(args)...);
        }
        template<typename T> inline bool removeComponent() noexcept {
            return m_Entity.removeComponent<T>();
        }
        template<typename T> [[nodiscard]] inline Engine::view_ptr<T> getComponent() const noexcept {
            return m_Entity.getComponent<T>();
        }
        template<class ... TYPES> [[nodiscard]] inline std::tuple<TYPES*...> getComponents() const noexcept {
            return m_Entity.getComponents<TYPES...>();
        }

        inline bool addComponent(const std::string& componentClassName, luabridge::LuaRef a1, luabridge::LuaRef a2, luabridge::LuaRef a3, luabridge::LuaRef a4, luabridge::LuaRef a5, luabridge::LuaRef a6, luabridge::LuaRef a7, luabridge::LuaRef a8) {
            return m_Entity.addComponent(componentClassName, a1, a2, a3, a4, a5, a6, a7, a8);
        }
        inline bool removeComponent(const std::string& componentClassName) {
            return m_Entity.removeComponent(componentClassName);
        }
        [[nodiscard]] inline luabridge::LuaRef getComponent(const std::string& componentClassName) {
            return m_Entity.getComponent(componentClassName);
        }

        //

        [[nodiscard]] inline glm::quat getRotation() const noexcept { return m_Entity.getRotation(); }
        [[nodiscard]] inline glm::quat getLocalRotation() const noexcept { return getRotation(); }
        [[nodiscard]] inline glm::vec3 getScale() const noexcept { return m_Entity.getScale(); }
        [[nodiscard]] inline glm_vec3 getPosition() const noexcept { return m_Entity.getPosition(); }
        [[nodiscard]] inline glm_vec3 getLocalPosition() const noexcept { return m_Entity.getLocalPosition(); }

        [[nodiscard]] inline const glm::vec3& getForward() const noexcept { return m_Entity.getForward(); }
        [[nodiscard]] inline const glm::vec3& getRight() const noexcept { return m_Entity.getRight(); }
        [[nodiscard]] inline const glm::vec3& getUp() const noexcept { return m_Entity.getUp(); }

        [[nodiscard]] inline glm_vec3 getLinearVelocity() const noexcept { return m_Entity.getLinearVelocity(); }
        [[nodiscard]] inline glm_vec3 getAngularVelocity() const noexcept { return m_Entity.getAngularVelocity(); }

        inline void translate(const glm_vec3& translation, bool local = true) noexcept { m_Entity.translate(translation, local); }
        inline void translate(decimal x, decimal y, decimal z, bool local = true) noexcept { m_Entity.translate(x, y, z, local); }
        inline void translate(decimal t, bool local = true) noexcept { m_Entity.translate(t, local); }

        inline void rotate(const glm::vec3& rotation, bool local = true) noexcept { m_Entity.rotate(rotation, local); }
        inline void rotate(float pitch_radians, float yaw_radians, float roll_radians, bool local = true) noexcept { m_Entity.rotate(pitch_radians, yaw_radians, roll_radians, local); }

        inline void scale(const glm::vec3& amount) noexcept { m_Entity.scale(amount); }
        inline void scale(float x, float y, float z) noexcept { m_Entity.scale(x, y, z); }
        inline void scale(float s) noexcept { m_Entity.scale(s); }

        inline void setPosition(const glm_vec3& newPosition) noexcept { m_Entity.setPosition(newPosition); }
        inline void setPosition(decimal x, decimal y, decimal z) noexcept { m_Entity.setPosition(x, y, z); }
        inline void setPosition(decimal p) noexcept { m_Entity.setPosition(p); }

        inline void setRotation(const glm::quat& newRotation) noexcept { m_Entity.setRotation(newRotation); }
        inline void setRotation(float quat_x, float quat_y, float quat_z, float quat_w) noexcept { m_Entity.setRotation(quat_x, quat_y, quat_z, quat_w); }

        inline void setScale(const glm::vec3& newScale) noexcept { m_Entity.setScale(newScale); }
        inline void setScale(float x, float y, float z) noexcept { m_Entity.setScale(x, y, z); }
        inline void setScale(float s) noexcept { m_Entity.setScale(s); }

        inline void setDamping(decimal linear, decimal angular) noexcept { m_Entity.setDamping(linear, angular); }

        inline void setDynamic(bool dynamic) noexcept { m_Entity.setDynamic(dynamic); }
        inline void setMass(float mass) noexcept { m_Entity.setMass(mass); }
        inline void setGravity(decimal x, decimal y, decimal z) noexcept { m_Entity.setGravity(x, y, z); }

        inline void clearLinearForces() noexcept { m_Entity.clearLinearForces(); }
        inline void clearAngularForces() noexcept { m_Entity.clearAngularForces(); }
        inline void clearAllForces() noexcept { m_Entity.clearAllForces(); }

        inline void setLinearVelocity(decimal x, decimal y, decimal z, bool local = true) noexcept { m_Entity.setLinearVelocity(x, y, z, local); }
        inline void setLinearVelocity(const glm_vec3& velocity, bool local = true) noexcept { m_Entity.setLinearVelocity(velocity, local); }

        inline void setAngularVelocity(decimal x, decimal y, decimal z, bool local = true) noexcept { m_Entity.setAngularVelocity(x, y, z, local); }
        inline void setAngularVelocity(const glm_vec3& velocity, bool local = true) noexcept { m_Entity.setAngularVelocity(velocity, local); }

        inline void applyForce(decimal x, decimal y, decimal z, bool local = true) noexcept { m_Entity.applyForce(x, y, z, local); }
        inline void applyForce(const glm_vec3& force, const glm_vec3& origin = glm_vec3(0.0f), const bool local = true) noexcept { m_Entity.applyForce(force, origin, local); }

        inline void applyImpulse(decimal x, decimal y, decimal z, bool local = true) noexcept { m_Entity.applyImpulse(x, y, z, local); }
        inline void applyImpulse(const glm_vec3& impulse, const glm_vec3& origin = glm_vec3(0.0f), bool local = true) noexcept { m_Entity.applyImpulse(impulse, origin, local); }

        inline void applyTorque(decimal x, decimal y, decimal z, bool local = true) noexcept { m_Entity.applyTorque(x, y, z, local); }
        inline void applyTorque(const glm_vec3& torque, bool local = true) noexcept { m_Entity.applyTorque(torque, local); }

        inline void applyTorqueImpulse(decimal x, decimal y, decimal z, bool local = true) noexcept { m_Entity.applyTorqueImpulse(x, y, z, local); }
        inline void applyTorqueImpulse(const glm_vec3& torqueImpulse, bool local = true) noexcept { m_Entity.applyTorqueImpulse(torqueImpulse, local); }
};

#endif