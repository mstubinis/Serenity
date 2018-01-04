#pragma once
#ifndef ENGINE_TIME_CLASS
#define ENGINE_TIME_CLASS

#include <memory>
#include <string>
typedef unsigned int uint;

class EngineTime{
	private:
		class impl;
		std::unique_ptr<impl> m_i;
	public:
		EngineTime();
		~EngineTime();

		void calculate();

		void stop_update();
		void stop_physics();
		void stop_render();

		void calculate_update();
		void calculate_physics();
		void calculate_render();

		float dt();
		float applicationTime();

		float updateTime();
		float physicsTime();
		float renderTime();

		std::string& reportTime();
		std::string& reportTime(uint decimals);
};

#endif