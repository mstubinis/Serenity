#pragma once
#ifndef ENGINE_ECS_ENTITY_H
#define ENGINE_ECS_ENTITY_H

class  Entity;

#include <core/engine/scene/Scene.h>
#include <ecs/ECS.h>
#include <ecs/EntityDataRequest.h>

class Entity {  
    public:
        std::uint32_t data = 0;

        Entity() = default;
        Entity(Scene&);
        Entity(const unsigned int entityID, const unsigned int sceneID, const unsigned int versionID);
        ~Entity() = default;

        Entity(const Entity& other);
        Entity& operator=(const Entity& other);
        Entity(Entity&& other) noexcept;
        Entity& operator=(Entity&& other) noexcept;

        static Entity null_;

        virtual void destroy();

        std::uint32_t id() const;
        std::uint32_t sceneID() const;
        std::uint32_t versionID() const;

        bool operator==(const Entity other) const;
        bool operator!=(const Entity other) const;

        Scene& scene() const;
        bool null() const;

        bool hasParent() const;

        void addChild(const Entity child) const;
        void removeChild(const Entity child) const;

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

};

namespace Engine::priv {
    struct InternalEntityPublicInterface final {
        static ECS<Entity>& GetECS(const Entity entity) {
            return Engine::priv::InternalScenePublicInterface::GetECS(entity.scene());
        }
    };
};

#endif