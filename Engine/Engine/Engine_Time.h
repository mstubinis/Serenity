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
		void stop_sounds();
		void stop_render();

		void calculate_update();
		void calculate_physics();
		void calculate_sounds();
		void calculate_render();


		void stop_rendering_geometry();
		void stop_rendering_lighting();
		void stop_rendering_ssao();
		void stop_rendering_aa();
		void stop_rendering_godrays();

		void calculate_rendering_geometry();
		void calculate_rendering_lighting();
		void calculate_rendering_ssao();
		void calculate_rendering_aa();
		void calculate_rendering_godrays();

		float rendering_geometryTime();
		float rendering_lightingTime();
		float rendering_ssaoTime();
		float rendering_aaTime();
		float rendering_godraysTime();

		float dt();
		float applicationTime();

		float updateTime();
		float physicsTime();
		float soundsTime();
		float renderTime();

		std::string& reportTime();
		std::string& reportTime(uint decimals);

		std::string& reportTimeRendering();
		std::string& reportTimeRendering(uint decimals);
};

#endif