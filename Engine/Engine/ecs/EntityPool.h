#pragma once
#ifndef ENGINE_ECS_ENTITY_POOL_H
#define ENGINE_ECS_ENTITY_POOL_H

#include "Entity.h"
#include <algorithm> //std::swap (until C++11)
#include <utility>   //std::swap (since C++11)
#include <vector>

//per scene basis

//also add the per scene component pools here too? possibly rename this class to ECS?

class Scene;
class EntityPool{
     private:
	     uint lastIndex;
         std::vector<Entity> pool;
	 public:
	     EntityPool():lastIndex(0){
		 }
		 ~EntityPool(){
			 lastIndex = 0;
			 pool.clear();
		 }
		 Entity* createEntity(Scene& _scene){
			 Entity e = Entity(lastIndex+1,_scene);
			 pool.push_back(std::move(_entity));
			 ++lastIndex;
			 return &pool[pool.size()-1];
		 }
		 void addEntity(Entity& _entity){
			 pool.push_back(std::move(_entity));
			 ++lastIndex;
		 }
		 //example: pool = {1,2,3,4,5,6,7} (lastIndex = 6)
		 //remove entityID: 3
		 
		 void removeEntity(uint _entityID){
			 uint indexToRemove = _entityID - 1;
			 if(indexToRemove != lastIndex){
				 std::swap(pool[indexToRemove],pool[lastIndex]);
			 }
			 Entity& e = pool[pool.size()];
			 /*
			 for(auto system : componentSystems){
				 system.removeComponent(this);
			 }
			 */
			 pool.pop_back();
			 --lastIndex;
		 }
		 void removeEntity(Entity& _entity){ removeEntity(_entity.ID); }
		 Entity* getEntity(uint _entityID){
			 if(_entityID == 0) return nullptr;
			 return &pool[_entityID-1];
		 }
		 
		 void moveEntity(EntityPool& other,uint _entityID){
			 Entity& e = pool[_entityID - 1];
			 other.addEntity(e);
			 removeEntity(e);
		 }
		 void moveEntity(EntityPool& other,Entity& _entity){ moveEntity(other,_entity.ID); }
}; 

#endif