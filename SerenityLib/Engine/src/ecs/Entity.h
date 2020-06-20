#pragma once
#ifndef ENGINE_ECS_ENTITY_H
#define ENGINE_ECS_ENTITY_H

namespace Engine::priv {
    template<typename T> class ECS;
};

#include <ecs/ECSIncludes.h>
#include <core/engine/scene/Scene.h>
#include <core/engine/lua/LuaIncludes.h>
#include <core/engine/lua/LuaState.h>

namespace Engine::priv {
    struct packed_data final {
        std::uint32_t        ID : ID_BIT_POSITIONS;
        std::uint32_t   sceneID : SCENE_BIT_POSITIONS;
        std::uint32_t versionID : VERSION_BIT_POSITIONS;
    };
}
/*
The Entity class used in the ECS framework.
*/

struct Entity {
    public:
        std::uint32_t m_Data = 0;

        Entity() = default;
        Entity(Scene& scene);
        Entity(std::uint32_t entityID, std::uint32_t sceneID, std::uint32_t versionID);
        ~Entity() = default;

        Entity(const Entity& other);
        Entity& operator=(const Entity& other);
        Entity(Entity&& other) noexcept;
        Entity& operator=(Entity&& other) noexcept;

        void destroy();
        bool isDestroyed() const;

        inline constexpr std::uint32_t id() const {
            return id(m_Data);
        }
        inline constexpr std::uint32_t sceneID() const {
            return sceneID(m_Data);
        }
        inline constexpr std::uint32_t versionID() const {
            return versionID(m_Data);
        }
        static inline constexpr std::uint32_t id(Entity entity) {
            return id(entity.m_Data);
        }
        static inline constexpr std::uint32_t sceneID(Entity entity) {
            return sceneID(entity.m_Data);
        }
        static inline constexpr std::uint32_t versionID(Entity entity) {
            return versionID(entity.m_Data);
        }
        static inline constexpr std::uint32_t id(std::uint32_t data) {
            Engine::priv::packed_data p{};
            p.ID = (data & 4'194'303U) >> (ENTITY_BIT_SIZE - VERSION_BIT_POSITIONS - SCENE_BIT_POSITIONS - ID_BIT_POSITIONS);
            return p.ID;
        }
        static inline constexpr std::uint32_t sceneID(std::uint32_t data) {
            Engine::priv::packed_data p{};
            p.sceneID = (data & 534'773'760U) >> (ENTITY_BIT_SIZE - VERSION_BIT_POSITIONS - SCENE_BIT_POSITIONS);
            return p.sceneID;
        }
        static inline constexpr std::uint32_t versionID(std::uint32_t data) {
            Engine::priv::packed_data p{};
            p.versionID = (data & 4'026'531'840U) >> (ENTITY_BIT_SIZE - VERSION_BIT_POSITIONS);
            return p.versionID;
        }


        inline constexpr bool operator==(const Entity other) const {
            return (m_Data == other.m_Data);
        }
        inline constexpr bool operator!=(const Entity other) const {
            return (m_Data != other.m_Data);
        }

        Scene& scene() const;
        inline constexpr bool null() const {
            return (m_Data == 0U);
        }

        bool hasParent() const;

        void addChild(Entity child) const;
        void removeChild(Entity child) const;

        template<typename T, typename... ARGS> inline void addComponent(ARGS&&... args) {
            Engine::priv::InternalEntityPublicInterface::GetECS(*this).addComponent<T>(*this, std::forward<ARGS>(args)...);
        }
        template<typename T> inline bool removeComponent() {
            return Engine::priv::InternalEntityPublicInterface::GetECS(*this).removeComponent<T>(*this);
        }
        template<typename T> inline constexpr T* getComponent() const {
            return Engine::priv::InternalEntityPublicInterface::GetECS(*this).getComponent<T>(*this);
        }
        template<class... Types> inline constexpr std::tuple<Types*...> getComponents() const {
            return Engine::priv::InternalEntityPublicInterface::GetECS(*this).getComponents<Types...>(*this);
        }

        void addComponent(const std::string& componentClassName, luabridge::LuaRef a1, luabridge::LuaRef a2, luabridge::LuaRef a3, luabridge::LuaRef a4, luabridge::LuaRef a5, luabridge::LuaRef a6, luabridge::LuaRef a7, luabridge::LuaRef a8);
        bool removeComponent(const std::string& componentClassName);
        luabridge::LuaRef getComponent(const std::string& componentClassName);
};

namespace Engine::priv {
    struct InternalEntityPublicInterface final {
        static ECS<Entity>& GetECS(Entity entity) {
            return Engine::priv::InternalScenePublicInterface::GetECS(entity.scene());
        }

        template<typename T> static luabridge::LuaRef GetComponent(lua_State* L, Entity entity, const char* globalName){
            luabridge::setGlobal(L, entity.getComponent<T>(), globalName);
            return luabridge::getGlobal(L, globalName);
        }
    };
};

#endif