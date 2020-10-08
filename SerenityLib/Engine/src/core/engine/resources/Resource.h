#pragma once
#ifndef ENGINE_RESOURCE_BASIC_H
#define ENGINE_RESOURCE_BASIC_H

#include <core/engine/resources/ResourcesIncludes.h>

class Resource {
    private:
        bool           m_IsLoaded      = false;
        std::string    m_Name          = "";
        uint32_t  m_UsageCount    = 0U;
        ResourceType   m_ResourceType  = ResourceType::Unknown;
    public:
        Resource() = default;
        Resource(ResourceType type);
        Resource(ResourceType type, const std::string& name);

        Resource(const Resource& other)                 = delete;
        Resource& operator=(const Resource& other)      = delete;
        Resource(Resource&& other) noexcept;
        Resource& operator=(Resource&& other) noexcept;
        virtual ~Resource() {}

        inline CONSTEXPR ResourceType type() const noexcept { return m_ResourceType; }
        inline CONSTEXPR const std::string& name() const noexcept { return m_Name;}
        inline CONSTEXPR bool isLoaded() const noexcept { return m_IsLoaded; }
        inline CONSTEXPR uint32_t useCount() const noexcept { return m_UsageCount; }

        inline void setName(const std::string& name) noexcept { m_Name = name; }

        inline void incrementUseCount() noexcept { ++m_UsageCount; }
        inline void decrementUseCount() noexcept { if (m_UsageCount > 0) { --m_UsageCount; } }

        virtual void load();
        virtual void unload();
};
#endif