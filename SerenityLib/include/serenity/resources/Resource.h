#pragma once
#ifndef ENGINE_RESOURCE_BASIC_H
#define ENGINE_RESOURCE_BASIC_H

#include <serenity/resources/ResourcesIncludes.h>
#include <string>

class Resource {
    private:
        std::string    m_Name;
        //uint32_t       m_UsageCount    = 0U;
        ResourceType   m_ResourceType  = ResourceType::Unknown;
        bool           m_IsLoaded      = false;
    public:
        Resource() = default;
        Resource(ResourceType type);
        Resource(ResourceType type, std::string_view name);

        Resource(const Resource&)                 = delete;
        Resource& operator=(const Resource&)      = delete;
        Resource(Resource&&) noexcept;
        Resource& operator=(Resource&&) noexcept;
        virtual ~Resource() {}

        [[nodiscard]] inline constexpr ResourceType type() const noexcept { return m_ResourceType; }
        [[nodiscard]] inline constexpr const std::string& name() const noexcept { return m_Name;}
        [[nodiscard]] inline constexpr bool isLoaded() const noexcept { return m_IsLoaded; }
        //[[nodiscard]]inline constexpr uint32_t useCount() const noexcept { return m_UsageCount; }

        inline void setName(std::string_view name) noexcept { m_Name = name; }

        //inline void incrementUseCount() noexcept { ++m_UsageCount; }
        //inline void decrementUseCount() noexcept { if (m_UsageCount > 0) { --m_UsageCount; } }

        virtual void load();
        virtual void unload();
};
#endif