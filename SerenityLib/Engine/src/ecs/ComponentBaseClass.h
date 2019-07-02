#pragma once
#ifndef ENGINE_ECS_COMPONENT_BASE_CLASS_H
#define ENGINE_ECS_COMPONENT_BASE_CLASS_H

#include <ecs/Entity.h>

class ComponentBaseClass{
	protected:
		Entity owner;
	public:
		inline ComponentBaseClass() { owner = Entity::_null; }
		inline ComponentBaseClass(const Entity& p_Owner) { owner.data = p_Owner.data; }

		ComponentBaseClass& operator=(const ComponentBaseClass& p_Other) = default;
		ComponentBaseClass(const ComponentBaseClass& p_Other) = default;
		ComponentBaseClass(ComponentBaseClass&& p_Other) noexcept = default;
		ComponentBaseClass& operator=(ComponentBaseClass&& p_Other) noexcept = default;

		virtual ~ComponentBaseClass() = default;
};

#endif