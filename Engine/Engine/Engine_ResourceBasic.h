#pragma once
#ifndef ENGINE_RESOURCE_BASIC_H
#define ENGINE_RESOURCE_BASIC_H

#include <string>
#include <vector>
#include <boost/weak_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>

typedef unsigned int uint;

class EngineResource{
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