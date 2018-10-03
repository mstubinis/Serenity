#pragma once
#ifndef ENGINE_ECS_COMPONENT_POOL_H
#define ENGINE_ECS_COMPONENT_POOL_H

#include "Entity.h"
#include <algorithm> //std::swap (until C++11)
#include <utility>   //std::swap (since C++11)
#include <vector>

template <typename T>
class IComponentPool{
	public:
	    virtual T* addComponent(Entity&) = 0;
		virtual void removeComponent(Entity&) = 0;
		virtual T* getComponent(Entity&) = 0;
};

//sparse set concept.
template <typename T>
class ComponentPool: public IComponentPool<T>{
     private:
	     uint               size;   //number of components created
	     std::vector<uint>  sparse; //maps entity ID to component Index in dense
		 std::vector<T>     dense;  //actual component pool
	 public:
	     ComponentPool():size(0){
		 }
		 ~ComponentPool(){
			 size = 0;
			 sparse.clear();
			 dense.clear();
		 }
		 T* addComponent(Entity& _entity){
			 uint sparseID =  _entity.id - 1;
			 if(sparse.size() <= sparseID){
				 for(uint i = 0; i < 2048; ++i){ sparse.emplace_back(0); }
			 }
			 if(sparse[sparseID] != 0){
				 return nullptr;
			 }
			 dense.push_back(T());
			 ++size;
			 sparse[sparseID] = size;
			 return &dense[size];
		 }
		 void removeComponent(Entity& _entity){ 
			 uint sparseID =  _entity.id - 1;
			 if(sparse[sparseID] == 0){
				 return;
			 } 
			 uint removedCID = sparse[sparseID];
			 std::swap(dense[removedCID],dense[size]);
			 --size;
			 sparse[size] = removedCID;
			 dense.pop_back();	 
		 }
		 T* getComponent(Entity& _entity){
			 uint sparseID =  _entity.id - 1;
			 if(sparse[sparseID] == 0){
				 return nullptr;
			 }
			 return &(dense[sparse[sparseID]]);
		 }
}; 

#endif