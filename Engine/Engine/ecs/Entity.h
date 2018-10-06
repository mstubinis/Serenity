#pragma once
#ifndef ENGINE_ECS_ENTITY_H
#define ENGINE_ECS_ENTITY_H

#include "../Engine.h"
#include "../Scene.h"
#include "ECS.h"

typedef std::uint32_t uint;

const uint MAX_ENTITIES = 16777216;
const uint MAX_SCENES = 256;

//Scenes,Entities, and Components should be given ids. an id of 0 means invalid or null.
//anything higher represents a valid id. An index into any array via id is array[id - 1]. only
//do this if the id != 0

class Scene;
struct Entity{   
    uint ID : 24;     //16,777,216 possible entities (2 ^ 24)
	uint sceneID : 8; //256 possible scenes (2 ^ 8)

    Entity();
    Entity(uint _id);
    Entity(uint _id, Scene& _scene);
    ~Entity();
    Scene& scene();
    inline uint arrayIndex() const;
    inline operator uint() const;
    bool null();
	template<typename T> T* addComponent(){
        auto& _this = *this;
        //return Engine::epriv::InternalEntityPublicInterface::GetECS<Entity>(_this).addComponent<T>(_this);
        return nullptr;
	}
	template<typename T> bool removeComponent(){
        auto& _this = *this;
        //return Engine::epriv::InternalEntityPublicInterface::GetECS<Entity>(_this).removeComponent<T>(_this);
        return true;
	}
	template<typename T> T* getComponent(){
        auto& _this = *this;
        //return Engine::epriv::InternalEntityPublicInterface::GetECS<Entity>(_this).getComponent<T>(_this);
        return nullptr;
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