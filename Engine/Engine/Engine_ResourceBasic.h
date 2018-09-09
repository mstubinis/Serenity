#pragma once
#ifndef ENGINE_RESOURCE_BASIC_H
#define ENGINE_RESOURCE_BASIC_H

#include <string>
#include "Engine_Utils.h"

class EngineResource: private Engine::epriv::noncopyable{
    private:
        bool m_IsLoaded;
        std::string m_Name;
        uint m_UsageCount;
    public:
        EngineResource(std::string = "");
        virtual ~EngineResource();

        std::string& name();
        void setName(std::string);

        bool isLoaded();
        uint useCount();

        void incrementUseCount();
        void decrementUseCount();

        virtual void load();
        virtual void unload();
};
#endif