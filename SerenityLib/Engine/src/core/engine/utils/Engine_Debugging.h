#pragma once
#ifndef ENGINE_DEBUGGING_CLASS_INCLUDE_GUARD
#define ENGINE_DEBUGGING_CLASS_INCLUDE_GUARD

#include <GL/glew.h>
#include <SFML/OpenGL.hpp>

#include <core/engine/utils/Utils.h>
#include <SFML/System.hpp>

namespace Engine {
    namespace priv {
        class DebugManager {
            private:
                sf::Clock clock;
                sf::Int64 m_logicTime, m_physicsTime, m_renderTime, m_soundTime;
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
                DebugManager();
                ~DebugManager();

                void cleanup();

                void _init();

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

                const double dt() const;
                const double logicTime() const;
                const double physicsTime() const;
                const double soundsTime() const;
                const double renderTime() const;
                const double totalTime() const;

                std::string timestamp();
                std::string& reportTime();
                std::string& reportTime(uint decimals);
                std::string reportDebug();
        };
    };
};
#endif