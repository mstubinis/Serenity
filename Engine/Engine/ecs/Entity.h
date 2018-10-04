#pragma once
#ifndef ENGINE_ECS_ENTITY_H
#define ENGINE_ECS_ENTITY_H

#include <cstdint>
#include "ECS.h"
#include "ECSRegistry.h"
#include "../Scene.h"

typedef std::uint32_t uint;

const unsigned int MAX_ENTITIES = 16777216;
const unsigned int MAX_SCENES = 256;

//Scenes,Entities, and Components should be given ids. an id of 0 means invalid or null.
//anything higher represents a valid id. An index into any array via id is array[id - 1]. only
//do this if the id != 0

struct Entity{
    static Entity _null;
    uint ID : 24;     //16,777,216 possible entities (2 ^ 24)
	uint sceneID : 8; //256 possible scenes (2 ^ 8)
	Entity():ID(0),sceneID(0){}	
	Entity(uint _id):ID(_id),sceneID(0){}
	Entity(uint _id,Scene& _scene):ID(_id),sceneID(_scene.id()){}	
	~Entity(){ ID = 0; sceneID = 0; }
    Scene& scene() {
        //TODO: implement this by changing the container in ResourceManager to a vector and making a getter to it
        Scene* s = nullptr;
        return *s;
    }
	inline uint arrayIndex() const{ return ID - 1; }
	inline operator uint() const{ return sceneID << 24 | ID; }
    bool null() { if (ID == 0) return true; return false; } //also check sceneID if 0?
	template<typename T> T* addComponent(){
        //auto& _this = *this;
        //return Engine::epriv::InternalEntityPublicInterface::GetECS(_this).addComponent<T>(_this);
        return nullptr;
	}
	template<typename T> void removeComponent(){
        //auto& _this = *this;
        //Engine::epriv::InternalEntityPublicInterface::GetECS(_this).removeComponent<T>(_this);
	}
	template<typename T> T* getComponent(){
        //auto& _this = *this;
        //return Engine::epriv::InternalEntityPublicInterface::GetECS(_this).getComponent<T>(_this);
        return nullptr;
	}
	void move(Scene& destination){
		if(destination.id() == sceneID) return;
		//do whatever is needed
		//for(auto system: allComponentSystems){
		//	system.removeComponent(this);
		//}
		sceneID = destination.id();
	}
};
namespace Engine {
    namespace epriv {
        class InternalEntityPublicInterface final {
            //public:
                //static ECS<Entity>& GetECS(Entity& _entity) {
                    //TODO: implement this
                //}
        };
    };
};

#endif