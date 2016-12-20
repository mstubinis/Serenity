#pragma once
#ifndef ENGINE_RESOURCEBASIC_H
#define ENGINE_RESOURCEBASIC_H

#include <string>

class EngineResource{
	private:
		std::string* m_Name;
	public:
		EngineResource(std::string = "");
		~EngineResource();

		std::string& name();
		std::string* namePtr();
		void setName(std::string);
};
#endif