#pragma once
#ifndef ENGINE_ECS_COMPONENT_BASE_CLASS_H
#define ENGINE_ECS_COMPONENT_BASE_CLASS_H

#include "ecs/Entity.h"

struct ComponentBaseClass{
    Entity owner;

    ComponentBaseClass() = default;
    ComponentBaseClass(Entity& owner_) { owner.data = owner_.data; }

    ComponentBaseClass& operator=(const ComponentBaseClass& other) = default;
    ComponentBaseClass(const ComponentBaseClass& other) = default;
    ComponentBaseClass(ComponentBaseClass&& other) noexcept = default;
    ComponentBaseClass& operator=(ComponentBaseClass&& other) noexcept = default;

    virtual ~ComponentBaseClass() = default;
};

#endif