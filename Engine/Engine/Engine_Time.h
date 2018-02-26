#pragma once
#ifndef ENGINE_TIME_CLASS_H
#define ENGINE_TIME_CLASS_H

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

				void stop_clock();

				void calculate_logic();
				void calculate_physics();
				void calculate_sounds();
				void calculate_render();
				void calculate_display();

				float& dt() const;

				const float& logicTime() const;
				const float& physicsTime() const;
				const float& soundsTime() const;
				const float& renderTime() const;
				const float& displayTime() const;

				std::string& reportTime();
				std::string& reportTime(uint decimals);
		};
	};
};
#endif