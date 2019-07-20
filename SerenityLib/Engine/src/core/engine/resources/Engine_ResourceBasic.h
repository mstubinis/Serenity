#pragma once
#ifndef ENGINE_RESOURCE_BASIC_H
#define ENGINE_RESOURCE_BASIC_H

#include <string>
#include <cstdint>
#include <core/engine/Engine_Utils.h>

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

        bool isLoaded();
        uint useCount();

        void incrementUseCount();
        void decrementUseCount();

        virtual void load();
        virtual void unload();
};
#endif