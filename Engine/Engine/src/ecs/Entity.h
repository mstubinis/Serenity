#pragma once
#ifndef ENGINE_ECS_ENTITY_H_INCLUDE_GUARD
#define ENGINE_ECS_ENTITY_H_INCLUDE_GUARD

#include "ecs/ECS.h"

typedef std::uint32_t uint;

const uint MAX_ENTITIES = 2097152;
const uint MAX_SCENES = 128;
const uint MAX_VERSIONS = 16;

class Scene;
struct Entity{   
    uint data;

    Entity() { data = 0; }
    Entity(uint _id, uint _sceneID, uint _versionID) { serialize(_id, _sceneID, _versionID); }

    void serialize(const uint& _id, const uint& _sceneID, const uint& _versionID) { data = _versionID << 28 | _sceneID << 21 | _id; }

    Entity(const Entity& other) = delete;
    Entity& operator=(const Entity& other) = delete;
    Entity(Entity&& other) noexcept = delete;
    Entity& operator=(Entity&& other) noexcept = delete;

    ~Entity() { data = 0; }

    Scene& scene();
    bool null();
	template<typename TComponent, typename... ARGS> TComponent* addComponent(ARGS&&... _args){
        auto& _this = *this; auto& _ecs = Engine::epriv::InternalEntityPublicInterface::GetECS(_this);
        return _ecs.addComponent<TComponent>(_this, std::forward<ARGS>(_args)...);
	}
	template<typename TComponent> bool removeComponent(){
        auto& _this = *this; auto& _ecs = Engine::epriv::InternalEntityPublicInterface::GetECS(_this);
        return _ecs.removeComponent<TComponent>(_this);
	}
	template<typename TComponent> TComponent* getComponent(){
        auto& _this = *this; auto& _ecs = Engine::epriv::InternalEntityPublicInterface::GetECS(_this);
        return _ecs.getComponent<TComponent>(_this);
	}
    void move(Scene& destination);
    static Entity _null;
};

namespace Engine {
    namespace epriv {
        struct InternalEntityPublicInterface final {
            static ECS& GetECS(Entity&);
        };
    };
};

#endif