#pragma once
#ifndef ENGINE_ECS_COMPONENT_NAME_H
#define ENGINE_ECS_COMPONENT_NAME_H

#include <serenity/ecs/ECS.h>

class ComponentName : public ComponentBaseClass<ComponentName> {
    private:
        std::string  m_Data;

        ComponentName() = delete;
    public:
        ComponentName(Entity);
        ComponentName(Entity, std::string_view name);
        ComponentName(const ComponentName&)                = default;
        ComponentName& operator=(const ComponentName&)     = default;
        ComponentName(ComponentName&&) noexcept            = default;
        ComponentName& operator=(ComponentName&&) noexcept = default;

        [[nodiscard]] inline const std::string& name() const noexcept { return m_Data; }
        inline void setName(std::string_view name) noexcept { m_Data = name; }
        [[nodiscard]] inline size_t size() const noexcept { return m_Data.size(); }
        [[nodiscard]] inline bool empty() const noexcept { return m_Data.empty(); }
};

namespace Engine::priv {
    class ComponentNameLUABinder {
        private:
            Entity m_Owner;

            ComponentNameLUABinder() = delete;
        public:
            ComponentNameLUABinder(Entity owner)
                : m_Owner{ owner }
            {}

            const std::string& name() const;
            size_t size() const;
            bool empty() const;
    };
}

#endif