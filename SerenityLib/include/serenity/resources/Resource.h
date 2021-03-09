#pragma once
#ifndef ENGINE_RESOURCE_BASIC_H
#define ENGINE_RESOURCE_BASIC_H

#include <serenity/resources/ResourcesIncludes.h>
#include <string>
#include <atomic>

class ResourceBaseClass {
    protected:
        std::string    m_Name;
        ResourceType   m_ResourceType = ResourceType::Unknown;
        bool           m_IsLoaded     = false;
    public:
        ResourceBaseClass() = default;
        ResourceBaseClass(ResourceType type);
        ResourceBaseClass(ResourceType type, std::string_view name);

        ResourceBaseClass(const ResourceBaseClass&)                 = delete;
        ResourceBaseClass& operator=(const ResourceBaseClass&)      = delete;
        ResourceBaseClass(ResourceBaseClass&&) noexcept;
        ResourceBaseClass& operator=(ResourceBaseClass&&) noexcept;
        virtual ~ResourceBaseClass() = default;

        [[nodiscard]] inline constexpr ResourceType type() const noexcept { return m_ResourceType; }
        [[nodiscard]] inline constexpr const std::string& name() const noexcept { return m_Name; }
        [[nodiscard]] inline constexpr bool isLoaded() const noexcept { return m_IsLoaded; }

        inline void setName(std::string_view name) noexcept { m_Name = name; }

        virtual void load();
        virtual void unload();
};

template<class RESOURCE>
class Resource : public ResourceBaseClass {
    public:
        //static inline std::atomic<uint32_t> TYPE_ID = 0;
        static inline uint32_t TYPE_ID = 0;
    public:
        Resource() = default;
        Resource(ResourceType type) 
            : ResourceBaseClass { type }
        {}
        Resource(ResourceType type, std::string_view name) 
            : ResourceBaseClass { type, name }
        {}

        Resource(const Resource&)                = delete;
        Resource& operator=(const Resource&)     = delete;
        Resource(Resource&&) noexcept            = default;
        Resource& operator=(Resource&&) noexcept = default;
        virtual ~Resource() = default;
};
#endif