#pragma once
#ifndef ENGINE_ECS_ENTITY_H
#define ENGINE_ECS_ENTITY_H

//class Scene;
struct EntityDataRequest;
namespace Engine::priv {
    template<typename T> class ECS;
};

#include <core/engine/scene/Scene.h>
//#include <ecs/ECS.h>
#include <ecs/ECSIncludes.h>
#include <core/engine/lua/LuaIncludes.h>
#include <core/engine/lua/LuaState.h>

/*
The Entity class used in the ECS framework.
*/
class Entity {
    friend struct EntityDataRequest;
    protected:
        std::uint32_t m_Data = 0U;
    public:
        Entity() = default;
        Entity(Scene& scene);
        Entity(std::uint32_t entityID, std::uint32_t sceneID, std::uint32_t versionID);
        virtual ~Entity() = default;

        Entity(const Entity& other);
        Entity& operator=(const Entity& other);
        Entity(Entity&& other) noexcept;
        Entity& operator=(Entity&& other) noexcept;

        static Entity null_;

        virtual void destroy();

        inline std::uint32_t id() const {
            return (m_Data & 4'194'303U) >> (ENTITY_SIZE - VERSION_BIT_POSITIONS - SCENE_BIT_POSITIONS - ID_BIT_POSITIONS);
        }
        inline std::uint32_t sceneID() const {
            return (m_Data & 534'773'760U) >> (ENTITY_SIZE - VERSION_BIT_POSITIONS - SCENE_BIT_POSITIONS);
        }
        inline std::uint32_t versionID() const {
            return (m_Data & 4'026'531'840U) >> (ENTITY_SIZE - VERSION_BIT_POSITIONS);
        }
        inline bool operator==(const Entity& other) const {
            return (m_Data == other.m_Data);
        }
        inline bool operator!=(const Entity& other) const {
            return (m_Data != other.m_Data);
        }

        Scene& scene() const;
        inline bool null() const {
            return (m_Data == 0U);
        }

        bool hasParent() const;

        void addChild(Entity& child) const;
        void removeChild(Entity& child) const;

        template<typename T, typename... ARGS> inline void addComponent(ARGS&&... args) {
            Engine::priv::InternalEntityPublicInterface::GetECS(*this).addComponent<T>(*this, std::forward<ARGS>(args)...);
        }
        template<typename T, typename... ARGS> inline void addComponent(EntityDataRequest& request, ARGS&&... args) {
            Engine::priv::InternalEntityPublicInterface::GetECS(*this).addComponent<T>(request, *this, std::forward<ARGS>(args)...);
        }
        template<typename T> inline bool removeComponent() {
            return Engine::priv::InternalEntityPublicInterface::GetECS(*this).removeComponent<T>(*this);
        }

        #pragma region 1 component get
            template<typename T> inline T* getComponent() const {
                return Engine::priv::InternalEntityPublicInterface::GetECS(*this).getComponent<T>(*this);
            }
            template<typename T> inline T* getComponent(const EntityDataRequest& dataRequest) const {
                return Engine::priv::InternalEntityPublicInterface::GetECS(*this).getComponent<T>(dataRequest);
            }
        #pragma endregion

        #pragma region variadic component get
            template<class... Types> inline std::tuple<Types*...> getComponents() const {
                return Engine::priv::InternalEntityPublicInterface::GetECS(*this).getComponents<Types...>(*this);
            }
            template<class... Types> inline std::tuple<Types*...> getComponents(const EntityDataRequest& dataRequest) const {
                return Engine::priv::InternalEntityPublicInterface::GetECS(*this).getComponents<Types...>(dataRequest);
            }
        #pragma endregion

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