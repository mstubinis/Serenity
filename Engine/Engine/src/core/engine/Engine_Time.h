#pragma once
#ifndef ENGINE_TIME_CLASS_H
#define ENGINE_TIME_CLASS_H

#include "core/engine/Engine_Utils.h"
#include "core/engine/Engine_Resources.h"

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
                void beginGLQuery();
                void endGLQuery(std::string& tag);
                void stop_clock();

                void calculate_logic();
                void calculate_physics();
                void calculate_sounds();
                void calculate_render();
                void calculate_display();

                const double dt() const;
                const double logicTime() const;
                const double physicsTime() const;
                const double soundsTime() const;
                const double renderTime() const;
                const double displayTime() const;

                std::string& reportTime();
                std::string& reportTime(uint decimals);
        };
    };
};
#endif