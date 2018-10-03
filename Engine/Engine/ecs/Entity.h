#pragma once
#ifndef ENGINE_ECS_ENTITY_H
#define ENGINE_ECS_ENTITY_H

#include <cstdint>

typedef std::uint32_t uint;

const unsigned int MAX_ENTITIES = 16777216;


//Scenes,Entities, and Components should be given ids. an id of 0 means invalid or null.
//anything higher represents a valid id. An index into any array via id is array[id - 1]. only
//do this if the id != 0

class Scene;
struct Entity{
    uint ID : 24;     //16,777,216 possible entities (2 ^ 24)
	uint sceneID : 8; //256 possible scenes (2 ^ 8)
	Entity():ID(0),sceneID(0){
	}	
	Entity(uint _id):ID(_id),sceneID(0){
	}
	Entity(uint _id,Scene& _scene):ID(_id),sceneID(_scene.id()){
	}	
	~Entity(){ ID = 0; sceneID = 0; }
	inline uint arrayIndex() const{ return ID - 1; }
	inline operator uint() const{ return sceneID << 24 | ID; }
	
	//add a method to scene that adds entities
	
	
	
	template<typename T> T* addComponent(){
		//get RTTI info for type T
		//associate the type info to the component pool for type T
		//add the component to the pool	that uses type T
		//return the component so the user can do things with it instantly
		
		//auto& system = getSystem<T>();
		//return system.addComponent(this);
	}
	template<typename T> void removeComponent(){
		//get RTTI info for type T
		//associate the type info to the component pool for type T
		//add the component to the pool	that uses type T
		//return the component so the user can do things with it instantly		
		
		//auto& system = getSystem<T>();
		//system.removeComponent(this);
	}
	template<typename T> T* getComponent(){
		//get RTTI info for type T
		//associate the type info to the component pool for type T
		//retrieve the component from the pool
		
		//auto& system = getSystem<T>();
		//return system.getComponent(this);
	}
	void move(Scene& destination){
		//move from current scene to destination scene.
		if(destination.id() == sceneID) return;
		//do whatever is needed
		/*
		for(auto system: allComponentSystems){
			system.removeComponent(this);
		}
		*/
		sceneID = destination.id();
	}
};

#endif