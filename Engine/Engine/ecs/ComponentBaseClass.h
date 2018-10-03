#pragma once
#ifndef ENGINE_ECS_COMPONENT_BASE_CLASS_H
#define ENGINE_ECS_COMPONENT_BASE_CLASS_H

#include "Entity.h"

struct ComponentBaseClass{
    uint entityOwnerID;
    
	ComponentBaseClass():entityOwnerID(0){
	}
	ComponentBaseClass(Entity& _owner):entityOwnerID(_owner.id){
	}	
	virtual ~ComponentBaseClass(){
	}
};

#endif