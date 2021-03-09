#pragma once
#ifndef ENGINE_ECS_COMPONENT_NAME_H
#define ENGINE_ECS_COMPONENT_NAME_H

#include <serenity/ecs/entity/Entity.h>
#include <serenity/ecs/components/ComponentBaseClass.h>

class ComponentName : public ComponentBaseClass<ComponentName> {
    private:
        std::string  m_Data;
        Entity       m_Owner;

        ComponentName() = delete;
    public:
        ComponentName(Entity entity);
        ComponentName(Entity entity, std::string_view name);

        [[nodiscard]] inline const std::string& name() const noexcept { return m_Data; }
        inline void setName(std::string_view name) noexcept { m_Data = name; }
        [[nodiscard]] inline size_t size() const noexcept { return m_Data.size(); }
        [[nodiscard]] inline bool empty() const noexcept { return m_Data.empty(); }
};

#endif