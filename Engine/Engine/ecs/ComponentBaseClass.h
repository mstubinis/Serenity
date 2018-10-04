#pragma once
#ifndef ENGINE_ECS_COMPONENT_BASE_CLASS_H
#define ENGINE_ECS_COMPONENT_BASE_CLASS_H

#include "Entity.h"
#include <boost/type_index.hpp>
/*
struct ComponentBaseClass{
    BOOST_TYPE_INDEX_REGISTER_CLASS
    uint entityOwnerID;
    
	ComponentBaseClass():entityOwnerID(0){
	}
	ComponentBaseClass(Entity& _owner):entityOwnerID(_owner.ID){
	}	
	virtual ~ComponentBaseClass(){
	}
};
*/
#endif