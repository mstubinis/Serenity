#pragma once
#ifndef ENGINE_RESOURCEBASIC_H
#define ENGINE_RESOURCEBASIC_H

#include <string>
#include <vector>
#include <boost/weak_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>

class EngineResource{
	private:
		bool m_IsLoaded;
        std::string m_Name;
    public:
        EngineResource(std::string = "");
        virtual ~EngineResource();

        std::string& name();
        void setName(std::string);

        bool isLoaded();

        virtual void load();
        virtual void unload();
};
#endif