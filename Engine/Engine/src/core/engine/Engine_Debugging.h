#pragma once
#ifndef ENGINE_DEBUGGING_CLASS_H
#define ENGINE_DEBUGGING_CLASS_H

#include "core/engine/Engine_Utils.h"
#include "core/engine/Engine_Resources.h"

namespace Engine {
    namespace epriv {
        class DebugManager {
            private:
                class impl; std::unique_ptr<impl> m_i;
            public:
                DebugManager(const char* name, uint w, uint h);
                ~DebugManager();

                void _init(const char* name, uint w, uint h);

                void addDebugLine(const char* message);


                void calculate();
                void beginGLQuery();
                void endGLQuery(const char* tag);
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
                std::string reportDebug();
        };
    };
};
#endif