#pragma once
#ifndef ENGINE_ECS_COMPONENT_NAME_H
#define ENGINE_ECS_COMPONENT_NAME_H

#include <serenity/ecs/Entity.h>
#include <serenity/ecs/ECSSystemConstructorInfo.h>

class ComponentName {
    private:
        std::string  m_Data;
        Entity       m_Owner = Entity();

        ComponentName() = delete;
    public:
        ComponentName(Entity entity);
        ComponentName(Entity entity, std::string_view name);

        [[nodiscard]] inline constexpr const std::string& name() const noexcept { return m_Data; }
        inline void setName(std::string_view name) noexcept { m_Data = name; }
        [[nodiscard]] inline size_t size() const noexcept { return m_Data.size(); }
        [[nodiscard]] inline bool empty() const noexcept { return m_Data.empty(); }

};
class ComponentName_System_CI : public Engine::priv::ECSSystemCI {
    public:
        //ComponentName_System_CI();
        //~ComponentName_System_CI() = default;
};

#endif