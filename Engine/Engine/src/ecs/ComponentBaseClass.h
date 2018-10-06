#pragma once
#ifndef ENGINE_ECS_COMPONENT_BASE_CLASS_H
#define ENGINE_ECS_COMPONENT_BASE_CLASS_H

#include "ecs/Entity.h"

struct ComponentBaseClass{
    Entity& owner;
    //ComponentBaseClass():owner(Entity::_null){
    //}
    ComponentBaseClass(Entity& owner_):owner(owner_){
    }
    virtual ~ComponentBaseClass() = default;
};

#endif