#pragma once
#ifndef ENGINE_RESOURCE_BASIC_H
#define ENGINE_RESOURCE_BASIC_H

#include <core/engine/resources/ResourcesIncludes.h>
#include <string>

class EngineResource{
    private:
        bool                 m_IsLoaded;
        std::string          m_Name;
        unsigned int         m_UsageCount;
        ResourceType::Type   m_ResourceType;
    public:
        EngineResource(const ResourceType::Type type, const std::string& name = "");
        virtual ~EngineResource();

        const std::string& name() const;
        void setName(const std::string&);

        const ResourceType::Type& type() const;
        const bool isLoaded() const;
        const unsigned int useCount() const;

        void incrementUseCount();
        void decrementUseCount();

        virtual void load();
        virtual void unload();
};
#endif