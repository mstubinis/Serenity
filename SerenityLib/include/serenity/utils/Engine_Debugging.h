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
#include <serenity/system/Macros.h>

class DebugTimerTypes {
    public:
        enum Type : uint16_t {
            Logic = 0,
            Physics,
            Sound,
            Render,
            Delta,
            _TOTAL
        };
        BUILD_ENUM_CLASS_MEMBERS(DebugTimerTypes, Type)
};

namespace Engine::priv {
    class DebugManager {
        private:
            sf::Clock m_Clock;
            sf::Clock m_ClockPhysics;

            std::array<std::chrono::nanoseconds, DebugTimerTypes::_TOTAL> m_TimesNanoPrev;
            std::array<std::chrono::nanoseconds, DebugTimerTypes::_TOTAL> m_TimesNano;

            float m_TimeScale             = 1.0;
            double m_TotalTime            = 0.0;
            uint32_t m_Decimals           = 4;

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

            void calculate(DebugTimerTypes type, std::chrono::nanoseconds timeNanoseconds) noexcept;
            void reset_timers();

            [[nodiscard]] std::string getTimeInMs(DebugTimerTypes) noexcept;
            [[nodiscard]] std::string deltaTimeInMs() noexcept;
            [[nodiscard]] std::string fps() const noexcept;

            [[nodiscard]] inline double totalTime() const noexcept { return m_TotalTime; }
            [[nodiscard]] inline float timeScale() const noexcept { return m_TimeScale; }
            inline void setTimeScale(float timeScale) noexcept { m_TimeScale = std::max(0.0f, timeScale); }

            inline std::string& reportTime() noexcept { return reportTime(m_Decimals); }
            std::string& reportTime(uint32_t decimals);
            std::string reportDebug();
    };
};
#endif