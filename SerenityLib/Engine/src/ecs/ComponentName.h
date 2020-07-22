#pragma once
#ifndef ENGINE_ECS_COMPONENT_NAME_H
#define ENGINE_ECS_COMPONENT_NAME_H

#include <ecs/Entity.h>
#include <ecs/ECSSystemConstructorInfo.h>

class ComponentName {
    private:
        Entity m_Owner;
        std::string m_Data;
    public:
        ComponentName(Entity entity);
        ComponentName(Entity entity, const std::string& name);
        ComponentName(Entity entity, const char* name);

        ComponentName(const ComponentName& other)                = delete;
        ComponentName& operator=(const ComponentName& other)     = delete;
        ComponentName(ComponentName&& other) noexcept            = default;
        ComponentName& operator=(ComponentName&& other) noexcept = default;

        const std::string& name() const;
        void setName(const std::string& name);
        void setName(const char* name);
        size_t size() const;
        bool empty() const;

        ~ComponentName();
};
class ComponentName_System_CI : public Engine::priv::ECSSystemCI {
    public:
        ComponentName_System_CI();
        ~ComponentName_System_CI() = default;
};

#endif