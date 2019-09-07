#pragma once
#ifndef ENGINE_RESOURCE_BASIC_H
#define ENGINE_RESOURCE_BASIC_H

#include <core/engine/utils/Utils.h>

class EngineResource{
    private:
        bool m_IsLoaded;
        std::string m_Name;
        uint m_UsageCount;
    public:
        EngineResource(const std::string& = "");
        virtual ~EngineResource();

        const std::string& name() const;
        void setName(const std::string&);

        const bool isLoaded() const;
        const uint useCount() const;

        void incrementUseCount();
        void decrementUseCount();

        virtual void load();
        virtual void unload();
};
#endif