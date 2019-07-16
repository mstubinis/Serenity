#pragma once
#ifndef ENGINE_DEBUGGING_CLASS_INCLUDE_GUARD
#define ENGINE_DEBUGGING_CLASS_INCLUDE_GUARD

#include <core/engine/Engine_Utils.h>
#include <core/engine/resources/Engine_Resources.h>

namespace Engine {
namespace epriv {
    class DebugManager {
        private:
            sf::Clock clock;
            sf::Int64 m_logicTime, m_physicsTime, m_renderTime, m_soundTime, m_displayTime;
            sf::Int64 m_deltaTime;
            double m_totalTime;
            double divisor;
            uint output_frame_delay, output_frame;
            uint decimals;
            std::string output;

            //opengl timers
            uint queryID;
            GLuint queryObject;

            //general text debugging
            std::vector<std::string> text_queue;
        public:
            DebugManager(const char* name, uint w, uint h);
            ~DebugManager();

            void _init(const char* name, uint w, uint h);

            void addDebugLine(const char* message);
            void addDebugLine(std::string& message);
            void addDebugLine(std::string message);

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
            const double totalTime() const;

            std::string timestamp();
            std::string& reportTime();
            std::string& reportTime(uint decimals);
            std::string reportDebug();
    };
};
};
#endif