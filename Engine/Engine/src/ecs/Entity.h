#pragma once
#ifndef ENGINE_ECS_ENTITY_H
#define ENGINE_ECS_ENTITY_H

#include "core/engine/Engine.h"
#include "core/Scene.h"
#include "ecs/ECS.h"

typedef std::uint32_t uint;

const uint MAX_ENTITIES = 16777216;
const uint MAX_SCENES = 256;

class Scene;
struct Entity{   
    uint ID : 24;     //16,777,216 possible entities (2 ^ 24)
	uint sceneID : 8; //256 possible scenes (2 ^ 8)

    Entity();
    Entity(uint _id);
    Entity(uint _id, Scene& _scene);

    Entity(const Entity& other);
    Entity& operator=(const Entity& other);
    Entity(Entity&& other) noexcept  = default;
    Entity& operator=(Entity&& other) noexcept = default;

    ~Entity();

    Scene& scene();
    inline operator uint() const;
    bool null();
	template<typename TComponent, typename... ARGS> TComponent* addComponent(ARGS&&... _args){
        auto& _this = *this;
        return Engine::epriv::InternalEntityPublicInterface::GetECS(_this).addComponent<TComponent>(_this, std::forward<ARGS>(_args)...);
	}
	template<typename TComponent> bool removeComponent(){
        auto& _this = *this;
        return Engine::epriv::InternalEntityPublicInterface::GetECS(_this).removeComponent<TComponent>(_this);
	}
	template<typename TComponent> TComponent* getComponent(){
        auto& _this = *this;
        return Engine::epriv::InternalEntityPublicInterface::GetECS(_this).getComponent<TComponent>(_this);
	}
    void move(Scene& destination);
    static Entity _null;
};

namespace Engine {
    namespace epriv {
        struct InternalEntityPublicInterface final {
            static ECS<Entity>& GetECS(Entity& _entity) {
                Scene& s = epriv::Core::m_Engine->m_ResourceManager._getSceneByID(_entity.sceneID);
                return Engine::epriv::InternalScenePublicInterface::GetECS(s);
            }
        };
    };
};

#endif