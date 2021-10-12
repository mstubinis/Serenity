#ifndef ENGINE_ECS_ENTITY_H
#define ENGINE_ECS_ENTITY_H

class Scene;
namespace Engine::priv {
    class  ECSEntityPool;
    struct PublicEntity;
};

#include <serenity/system/TypeDefs.h>
#include <serenity/ecs/ECSIncludes.h>
#include <serenity/types/ViewPointer.h>
#include <serenity/lua/LuaIncludes.h>
#include <serenity/lua/LuaState.h>


constexpr const uint32_t ENTITY_NULL_ID = std::numeric_limits<uint32_t>().max();
inline constexpr uint32_t getMaxEntityIDBits(uint32_t bits) { return (1 << bits) - 1; }

/* The Entity class used in the ECS framework. */
class Entity {
    friend class  Engine::priv::ECSEntityPool;
    private:
        uint32_t        m_ID : ID_BIT_POSITIONS      = ENTITY_NULL_ID;
        uint32_t   m_SceneID : SCENE_BIT_POSITIONS   = ENTITY_NULL_ID;
        uint32_t m_VersionID : VERSION_BIT_POSITIONS = ENTITY_NULL_ID;
    public:
        constexpr Entity() = default;
        Entity(Scene&);
        constexpr Entity(uint32_t entityID, uint32_t sceneID, uint32_t versionID)
            : m_ID{ entityID }
            , m_SceneID{ sceneID }
            , m_VersionID{ versionID }
        {}

        Entity(const Entity&)                = default;
        Entity& operator=(const Entity&)     = default;
        Entity(Entity&&) noexcept            = default;
        Entity& operator=(Entity&&) noexcept = default;

        inline constexpr operator bool() const noexcept { return !null(); }
        inline constexpr bool null() const noexcept { return (
            m_ID == getMaxEntityIDBits(ID_BIT_POSITIONS) && 
            m_SceneID == getMaxEntityIDBits(SCENE_BIT_POSITIONS) && 
            m_VersionID == getMaxEntityIDBits(VERSION_BIT_POSITIONS)); 
        }

        inline constexpr bool operator<(uint32_t otherID) const noexcept { return m_ID < otherID; }
        inline constexpr bool operator<=(uint32_t otherID) const noexcept { return m_ID <= otherID; }
        inline constexpr bool operator>(uint32_t otherID) const noexcept { return m_ID > otherID; }
        inline constexpr bool operator>=(uint32_t otherID) const noexcept { return m_ID >= otherID; }
        inline constexpr bool operator==(uint32_t otherID) const noexcept { return (m_ID == otherID); }
        inline constexpr bool operator!=(uint32_t otherID) const noexcept { return !Entity::operator==(otherID); }

        inline constexpr bool operator<(Entity other) const noexcept { return m_ID < other.m_ID; }
        inline constexpr bool operator<=(Entity other) const noexcept { return m_ID <= other.m_ID; }
        inline constexpr bool operator>(Entity other) const noexcept { return m_ID > other.m_ID; }
        inline constexpr bool operator>=(Entity other) const noexcept { return m_ID >= other.m_ID; }
        inline constexpr bool operator==(Entity other) const noexcept { return (m_ID == other.m_ID && m_SceneID == other.m_SceneID && m_VersionID == other.m_VersionID); }
        inline constexpr bool operator!=(Entity other) const noexcept { return !Entity::operator==(other); }

        inline std::string toString() const { return std::to_string(m_ID) + "," + std::to_string(m_SceneID) + "," + std::to_string(m_VersionID); }
        void fill(uint32_t entityID, uint32_t sceneID, uint32_t versionID) noexcept {
            m_ID        = entityID;
            m_SceneID   = sceneID;
            m_VersionID = versionID;
        }
        inline void fill(Entity other) noexcept { fill(other.m_ID, other.m_SceneID, other.m_VersionID); }

        void destroy() noexcept;
        bool isDestroyed() const noexcept;

        [[nodiscard]] inline constexpr uint32_t id() const noexcept { return m_ID; }
        [[nodiscard]] inline constexpr uint32_t sceneID() const noexcept { return m_SceneID; }
        [[nodiscard]] inline constexpr uint32_t versionID() const noexcept { return m_VersionID; }
        [[nodiscard]] Engine::view_ptr<Scene> scene() const noexcept;
        [[nodiscard]] bool hasParent() const noexcept;
        [[nodiscard]] Entity getParent() const noexcept;
        [[nodiscard]] std::vector<Entity> getChildren() const noexcept;

        void addChild(Entity) const noexcept;
        void removeChild(Entity) const noexcept;
        //void removeAllChildren() const noexcept;

        template<class COMPONENT, class ... ARGS>
        bool addComponent(ARGS&&... args) noexcept {
            return Engine::priv::PublicEntity::GetECS(*this)->addComponent<COMPONENT>(*this, std::forward<ARGS>(args)...);
        }
        template<class COMPONENT> bool removeComponent() noexcept;
        template<class COMPONENT> [[nodiscard]] Engine::view_ptr<COMPONENT> getComponent() const noexcept;

        template<class ... TYPES> [[nodiscard]] std::tuple<TYPES*...> getComponents() const noexcept {
            return Engine::priv::PublicEntity::GetECS(*this)->getComponents<TYPES...>(*this);
        }

        bool addComponent(std::string_view componentClassName, luabridge::LuaRef a1, luabridge::LuaRef a2, luabridge::LuaRef a3, luabridge::LuaRef a4, luabridge::LuaRef a5, luabridge::LuaRef a6, luabridge::LuaRef a7, luabridge::LuaRef a8);
        bool removeComponent(std::string_view componentClassName);
        [[nodiscard]] luabridge::LuaRef getComponent(std::string_view componentClassName);
};

namespace std {
    template <>
    struct hash<Entity> {
        std::size_t operator()(const Entity& entity) const {
            using std::hash;
            return ((hash<uint32_t>()(entity.id()) ^ (hash<uint32_t>()(entity.sceneID()) << 1)) >> 1) ^ (hash<uint32_t>()(entity.versionID()) << 1);
        }
    };
};

namespace Engine::priv {
    class ECS;
    struct PublicEntity final {
        static Engine::view_ptr<Engine::priv::ECS> GetECS(Entity entity);

        template<class COMPONENT> [[nodiscard]] static luabridge::LuaRef GetComponent(lua_State* L, Entity entity, const char* globalName);
    };
};

#include <serenity/ecs/entity/Entity.inl>

#endif