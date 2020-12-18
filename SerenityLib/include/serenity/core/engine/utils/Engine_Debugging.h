#pragma once
#ifndef ENGINE_DEBUGGING_CLASS_H
#define ENGINE_DEBUGGING_CLASS_H

#include <GL/glew.h>
#include <SFML/OpenGL.hpp>

#include <serenity/core/engine/utils/Utils.h>
#include <SFML/System.hpp>
#include <vector>
#include <string>

namespace Engine::priv {
    class DebugManager {
        private:
            sf::Clock clock          = sf::Clock();
            sf::Int64 m_logicTime    = 0;
            sf::Int64 m_physicsTime  = 0;
            sf::Int64 m_renderTime   = 0;
            sf::Int64 m_soundTime    = 0;
            sf::Int64 m_deltaTime    = 0;
            float m_TimeScale        = 1.0;
            double m_totalTime       = 0.0;
            double divisor           = 1000000.0;
            unsigned int output_frame_delay  = 4;
            unsigned int output_frame        = 0;
            unsigned int decimals            = 4;
            std::string output       = "";

            //opengl timers
            unsigned int queryID             = 0;
            GLuint queryObject       = 0;

            //general text debugging
            std::vector<std::string> text_queue;
        public:
            DebugManager() = default;
            ~DebugManager() = default;

            void _init();

            void addDebugLine(const char* inMessage) noexcept { text_queue.emplace_back(inMessage); }
            void addDebugLine(const std::string& inMessage) noexcept { text_queue.emplace_back(inMessage); }
            void addDebugLine(std::string&& inMessage) noexcept { text_queue.emplace_back(std::move(inMessage)); }

            void calculate();
            void beginGLQuery();
            void endGLQuery(const char* tag);

            void stop_clock() noexcept { clock.restart(); }
            void calculate_logic() noexcept { m_logicTime = clock.restart().asMicroseconds(); }
            void calculate_physics() noexcept { m_physicsTime = clock.restart().asMicroseconds(); }
            void calculate_sounds() noexcept { m_soundTime = clock.restart().asMicroseconds(); }
            void calculate_render() noexcept { m_renderTime = clock.restart().asMicroseconds(); }

            inline constexpr float dt() const noexcept { return (float)((float)m_deltaTime / divisor); }
            inline constexpr double logicTime() const noexcept { return (double)((double)m_logicTime / divisor); }
            inline constexpr double physicsTime() const noexcept { return (double)((double)m_physicsTime / divisor); }
            inline constexpr double renderTime() const noexcept { return (double)((double)m_renderTime / divisor); }
            inline constexpr double soundsTime() const noexcept { return (double)((double)m_soundTime / divisor); }

            inline constexpr double totalTime() const noexcept { return m_totalTime; }
            inline constexpr float timeScale() const noexcept { return m_TimeScale; }

            void setTimeScale(float timeScale);

            std::string& reportTime();
            std::string& reportTime(unsigned int decimals);
            std::string reportDebug();
    };
};
#endif