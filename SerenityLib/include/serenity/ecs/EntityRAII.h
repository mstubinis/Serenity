#pragma once
#ifndef ENGINE_ECS_ENTITY_RAII_H
#define ENGINE_ECS_ENTITY_RAII_H

#include <serenity/ecs/Entity.h>

/*
This entity is functionally equivalent to Entity, but will call Entity::destroy() upon its destruction. 
Do not use this as a handle to pass around, use Entity for that.
*/
class EntityRAII final {
    private:
        Entity m_Entity = Entity{};
    public:
        constexpr EntityRAII() = default;
        EntityRAII(Scene& scene)
            : m_Entity{ scene }
        {}
        constexpr EntityRAII(uint32_t entityID, uint32_t sceneID, uint32_t versionID)
            : m_Entity{ entityID, sceneID, versionID }
        {}
        ~EntityRAII() {
            m_Entity.destroy();
        }

        EntityRAII(const EntityRAII& other)            = delete;
        EntityRAII& operator=(const EntityRAII& other) = delete;
        EntityRAII(EntityRAII&& other) noexcept 
            : m_Entity{ std::exchange(other.m_Entity, Entity{}) }
        {}
        EntityRAII& operator=(EntityRAII&& other) noexcept {
            m_Entity = std::exchange(other.m_Entity, Entity{});
            return *this;
        }

        inline operator uint32_t() const noexcept { return m_Entity.m_Data; }
        inline operator bool() const noexcept { return !m_Entity.null(); }
        inline operator Entity() const noexcept { return m_Entity; }
        inline bool operator==(const EntityRAII other) const noexcept { return (m_Entity.m_Data == other.m_Entity.m_Data); }
        inline bool operator!=(const EntityRAII other) const noexcept { return (m_Entity.m_Data != other.m_Entity.m_Data); }
        inline bool operator==(const Entity other) const noexcept { return (m_Entity.m_Data == other.m_Data); }
        inline bool operator!=(const Entity other) const noexcept { return (m_Entity.m_Data != other.m_Data); }
        inline bool null() const noexcept { return (m_Entity.m_Data == 0U); }

        inline bool isDestroyed() const noexcept { return m_Entity.isDestroyed(); }

        inline uint32_t id() const noexcept { return id(m_Entity.m_Data); }
        inline uint32_t sceneID() const noexcept { return sceneID(m_Entity.m_Data); }
        inline uint32_t versionID() const noexcept { return versionID(m_Entity.m_Data); }

        static inline uint32_t id(uint32_t data) noexcept { return Entity::id(data); }
        static inline uint32_t sceneID(uint32_t data) noexcept { return Entity::sceneID(data); }
        static inline uint32_t versionID(uint32_t data) noexcept { return Entity::versionID(data); }

        inline Engine::view_ptr<Scene> scene() const noexcept { return m_Entity.scene(); }
        inline bool hasParent() const noexcept { return m_Entity.hasParent(); }

        inline void addChild(Entity child) const noexcept { m_Entity.addChild(child); }
        inline void removeChild(Entity child) const noexcept { m_Entity.removeChild(child); }
        inline void removeAllChildren() const noexcept { m_Entity.removeAllChildren(); }

        template<typename T, typename... ARGS> inline bool addComponent(ARGS&&... args) noexcept {
            return m_Entity.addComponent<T>(std::forward<ARGS>(args)...);
        }
        template<typename T> inline bool removeComponent() noexcept {
            return m_Entity.removeComponent<T>();
        }
        template<typename T> inline Engine::view_ptr<T> getComponent() const noexcept {
            return m_Entity.getComponent<T>();
        }
        template<class ... TYPES> inline std::tuple<TYPES*...> getComponents() const noexcept {
            return m_Entity.getComponents<TYPES...>();
        }

        inline bool addComponent(const std::string& componentClassName, luabridge::LuaRef a1, luabridge::LuaRef a2, luabridge::LuaRef a3, luabridge::LuaRef a4, luabridge::LuaRef a5, luabridge::LuaRef a6, luabridge::LuaRef a7, luabridge::LuaRef a8) {
            return m_Entity.addComponent(componentClassName, a1, a2, a3, a4, a5, a6, a7, a8);
        }
        inline bool removeComponent(const std::string& componentClassName) {
            return m_Entity.removeComponent(componentClassName);
        }
        inline luabridge::LuaRef getComponent(const std::string& componentClassName) {
            return m_Entity.getComponent(componentClassName);
        }
};

#endif