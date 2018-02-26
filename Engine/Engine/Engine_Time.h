#pragma once
#ifndef ENGINE_TIME_CLASS
#define ENGINE_TIME_CLASS

#include <memory>
#include <string>
typedef unsigned int uint;

namespace Engine{
	namespace epriv{
		class TimeManager{
			private:
				class impl; std::unique_ptr<impl> m_i;
			public:
				TimeManager(const char* name,uint w,uint h);
				~TimeManager();

				void _init(const char* name,uint w,uint h);

				void calculate();

				void stop_update();
				void stop_physics();
				void stop_sounds();
				void stop_render();

				void calculate_update();
				void calculate_physics();
				void calculate_sounds();
				void calculate_render();

				void stop_rendering_display();
				void calculate_rendering_display();
				float rendering_displayTime();

				float& dt();
				float applicationTime();

				float updateTime();
				float physicsTime();
				float soundsTime();
				float renderTime();

				std::string& reportTime();
				std::string& reportTime(uint decimals);
		};
	};
};
#endif