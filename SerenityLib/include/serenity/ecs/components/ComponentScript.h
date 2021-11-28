#pragma once
#ifndef ENGINE_ECS_COMPONENT_SCRIPT_H
#define ENGINE_ECS_COMPONENT_SCRIPT_H

#include <serenity/ecs/components/ComponentBaseClass.h>
#include <serenity/lua/LuaScript.h>

class ComponentScript : public ComponentBaseClass<ComponentScript> {
    private:
        LuaScript  m_LUAScript;
        Entity     m_Owner;
    public:
        ComponentScript(Entity);
        ComponentScript(Entity, std::string_view scriptFilePath);
        ComponentScript(const ComponentScript&)                   = default;
        ComponentScript& operator=(const ComponentScript&)        = default;
        ComponentScript(ComponentScript&&) noexcept               = default;
        ComponentScript& operator=(ComponentScript&&) noexcept    = default;
        ~ComponentScript();

        void init(std::string_view scriptFilePath);
};

#endif