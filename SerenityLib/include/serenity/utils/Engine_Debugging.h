#pragma once
#ifndef ENGINE_DEBUGGING_CLASS_H
#define ENGINE_DEBUGGING_CLASS_H

#include <GL/glew.h>
#include <SFML/OpenGL.hpp>

#include <serenity/utils/Utils.h>
#include <SFML/System.hpp>
#include <vector>
#include <string>
#include <sstream>

namespace Engine::priv {
    class DebugManager {
        private:
            std::stringstream m_Strm;
            sf::Clock m_Clock;
            sf::Clock m_ClockPhysics;
            sf::Int64 m_LogicTime         = 0;
            sf::Int64 m_PhysicsTime       = 0;
            sf::Int64 m_RenderTime        = 0;
            sf::Int64 m_SoundTime         = 0;
            float m_TimeScale             = 1.0;
            double m_TotalTime            = 0.0;
            double m_Divisor              = 1'000'000.0;
            uint32_t m_Output_frame_delay = 4;
            uint32_t m_Output_frame       = 0;
            uint32_t m_Decimals           = 4;
            std::string m_Output;

            //opengl timers
            GLuint queryID                = 0;
            GLuint queryObject            = 0;

            //general text debugging
            std::vector<std::string> m_Text_Queue;
        public:
            void _init();

            inline void addDebugLine(const char* inMessage) noexcept { m_Text_Queue.emplace_back(inMessage); }
            inline void addDebugLine(const std::string& inMessage) noexcept { m_Text_Queue.emplace_back(inMessage); }
            inline void addDebugLine(std::string&& inMessage) noexcept { m_Text_Queue.emplace_back(std::move(inMessage)); }

            void calculate();
            void beginGLQuery();
            void endGLQuery(const char* tag);

            inline void stop_clock() noexcept { m_Clock.restart(); }
            inline void stop_clock_physics() noexcept { m_ClockPhysics.restart(); }
            inline void calculate_logic() noexcept { m_LogicTime = m_Clock.restart().asMicroseconds(); }
            inline void calculate_physics() noexcept { m_PhysicsTime = m_ClockPhysics.restart().asMicroseconds(); }
            inline void calculate_sounds() noexcept { m_SoundTime = m_Clock.restart().asMicroseconds(); }
            inline void calculate_render() noexcept { m_RenderTime = m_Clock.restart().asMicroseconds(); }

            [[nodiscard]] std::string updateTimeInMs() noexcept;
            [[nodiscard]] std::string physicsTimeInMs() noexcept;
            [[nodiscard]] std::string soundsTimeInMs() noexcept;
            [[nodiscard]] std::string renderTimeInMs() noexcept;
            [[nodiscard]] std::string deltaTimeInMs() noexcept;

            [[nodiscard]] std::string fps() const noexcept;

            [[nodiscard]] inline constexpr double logicTime() const noexcept { return (double)((double)m_LogicTime / m_Divisor); }
            [[nodiscard]] inline constexpr double physicsTime() const noexcept { return (double)((double)m_PhysicsTime / m_Divisor); }
            [[nodiscard]] inline constexpr double renderTime() const noexcept { return (double)((double)m_RenderTime / m_Divisor); }
            [[nodiscard]] inline constexpr double soundsTime() const noexcept { return (double)((double)m_SoundTime / m_Divisor); }
            [[nodiscard]] inline constexpr double totalTime() const noexcept { return m_TotalTime; }
            [[nodiscard]] inline constexpr float timeScale() const noexcept { return m_TimeScale; }
            inline void setTimeScale(float timeScale) noexcept { m_TimeScale = std::max(0.0f, timeScale); }

            inline std::string& reportTime() noexcept { return reportTime(m_Decimals); }
            std::string& reportTime(uint32_t decimals);
            std::string reportDebug();
    };
};
#endif