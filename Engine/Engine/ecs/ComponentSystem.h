#pragma once
#ifndef ENGINE_ECS_COMPONENT_SYSTEM_H
#define ENGINE_ECS_COMPONENT_SYSTEM_H

#include "ComponentPool.h"

class IComponentSystem{
    public:
	    virtual void update(const float& dt) = 0;
};

template<typename T>
class ComponentSystem: public IComponentSystem{
	private:
	    ComponentPool<T> pool;
	public:
	
	    ComponentSystem(){}
		~ComponentSystem(){}
	
	    virtual void update(const float& dt){}
};

#endif