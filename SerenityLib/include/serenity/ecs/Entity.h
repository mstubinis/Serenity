#pragma once
#ifndef ENGINE_ECS_ENTITY_H
#define ENGINE_ECS_ENTITY_H

class Scene;
namespace Engine::priv {
    struct PublicEntity;
};

#include <serenity/system/TypeDefs.h>
#include <serenity/types/ViewPointer.h>

#include <serenity/ecs/ECSIncludes.h>

namespace Engine::priv {
    struct entity_packed_data final {
        uint32_t        ID : ID_BIT_POSITIONS;
        uint32_t   sceneID : SCENE_BIT_POSITIONS;
        uint32_t versionID : VERSION_BIT_POSITIONS;
    };
}
#include <serenity/lua/LuaIncludes.h>
#include <serenity/lua/LuaState.h>


/* The Entity class used in the ECS framework. */
struct Entity {
    public:
        uint32_t m_Data = 0;

        constexpr Entity() = default;
        Entity(Scene& scene);
        constexpr Entity(uint32_t entityID, uint32_t sceneID, uint32_t versionID) {
            m_Data = 
                versionID << (ENTITY_BIT_SIZE - VERSION_BIT_POSITIONS) |    
                sceneID << (ENTITY_BIT_SIZE - VERSION_BIT_POSITIONS - SCENE_BIT_POSITIONS) |    
                entityID;
        }
        ~Entity() = default;

        Entity(const Entity& other)                = default;
        Entity& operator=(const Entity& other)     = default;
        Entity(Entity&& other) noexcept            = default;
        Entity& operator=(Entity&& other) noexcept = default;

        inline constexpr operator uint32_t() const noexcept { return m_Data; }
        inline constexpr operator bool() const noexcept { return !null(); }
        inline constexpr bool operator==(const Entity other) const noexcept { return (m_Data == other.m_Data); }
        inline constexpr bool operator!=(const Entity other) const noexcept { return (m_Data != other.m_Data); }
        inline constexpr bool null() const noexcept { return (m_Data == 0U); }

        void destroy() noexcept;
        bool isDestroyed() const noexcept;

        [[nodiscard]] inline constexpr uint32_t id() const noexcept { return id(m_Data); }
        [[nodiscard]] inline constexpr uint32_t sceneID() const noexcept { return sceneID(m_Data); }
        [[nodiscard]] inline constexpr uint32_t versionID() const noexcept { return versionID(m_Data); }

        [[nodiscard]] static inline constexpr uint32_t id(uint32_t data) noexcept {
            Engine::priv::entity_packed_data p{};
            p.ID = (data & 4'194'303U) >> (ENTITY_BIT_SIZE - VERSION_BIT_POSITIONS - SCENE_BIT_POSITIONS - ID_BIT_POSITIONS);
            return p.ID;
        }
        [[nodiscard]] static inline constexpr uint32_t sceneID(uint32_t data) noexcept {
            Engine::priv::entity_packed_data p{};
            p.sceneID = (data & 534'773'760U) >> (ENTITY_BIT_SIZE - VERSION_BIT_POSITIONS - SCENE_BIT_POSITIONS);
            return p.sceneID;
        }
        [[nodiscard]] static inline constexpr uint32_t versionID(uint32_t data) noexcept {
            Engine::priv::entity_packed_data p{};
            p.versionID = (data & 4'026'531'840U) >> (ENTITY_BIT_SIZE - VERSION_BIT_POSITIONS);
            return p.versionID;
        }

        [[nodiscard]] Engine::view_ptr<Scene> scene() const noexcept;
        [[nodiscard]] bool hasParent() const noexcept;

        void addChild(Entity child) const noexcept;
        void removeChild(Entity child) const noexcept;
        void removeAllChildren() const noexcept;

        template<typename T, typename... ARGS> inline bool addComponent(ARGS&&... args) noexcept {
            return Engine::priv::PublicEntity::GetECS(*this)->addComponent<T>(*this, std::forward<ARGS>(args)...);
        }
        template<typename T> inline bool removeComponent() noexcept {
            return Engine::priv::PublicEntity::GetECS(*this)->removeComponent<T>(*this);
        }
        template<typename T> [[nodiscard]] inline Engine::view_ptr<T> getComponent() const noexcept {
            return Engine::priv::PublicEntity::GetECS(*this)->getComponent<T>(*this);
        }
        template<class... Types> [[nodiscard]] inline std::tuple<Types*...> getComponents() const noexcept {
            return Engine::priv::PublicEntity::GetECS(*this)->getComponents<Types...>(*this);
        }

        bool addComponent(const std::string& componentClassName, luabridge::LuaRef a1, luabridge::LuaRef a2, luabridge::LuaRef a3, luabridge::LuaRef a4, luabridge::LuaRef a5, luabridge::LuaRef a6, luabridge::LuaRef a7, luabridge::LuaRef a8);
        bool removeComponent(const std::string& componentClassName);
        [[nodiscard]] luabridge::LuaRef getComponent(const std::string& componentClassName);
};

namespace Engine::priv {
    class ECS;
    struct PublicEntity final {
        static Engine::view_ptr<Engine::priv::ECS> GetECS(Entity entity);

        template<typename T> [[nodiscard]] static luabridge::LuaRef GetComponent(lua_State* L, Entity entity, const char* globalName){
            luabridge::setGlobal(L, entity.getComponent<T>(), globalName);
            return luabridge::getGlobal(L, globalName);
        }
    };
};

#endif