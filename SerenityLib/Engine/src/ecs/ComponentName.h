#pragma once
#ifndef ENGINE_ECS_COMPONENT_NAME_H
#define ENGINE_ECS_COMPONENT_NAME_H

#include <ecs/Entity.h>
#include <ecs/ECSSystemConstructorInfo.h>

class ComponentName {
    private:
        Entity       m_Owner = Entity();
        std::string  m_Data  = "";

        ComponentName() = delete;
    public:
        ComponentName(Entity entity);
        ComponentName(Entity entity, const std::string& name);
        ComponentName(Entity entity, const char* name);

        inline CONSTEXPR const std::string& name() const noexcept { return m_Data; }
        inline void setName(const std::string& name) noexcept { m_Data = name; }
        inline void setName(const char* name) noexcept { m_Data = name; }
        inline size_t size() const noexcept { return m_Data.size(); }
        inline bool empty() const noexcept { return m_Data.empty(); }

};
class ComponentName_System_CI : public Engine::priv::ECSSystemCI {
    public:
        //ComponentName_System_CI();
        //~ComponentName_System_CI() = default;
};

#endif