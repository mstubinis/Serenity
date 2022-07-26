#pragma once
#ifndef ENGINE_TYPE_TIMER_H
#define ENGINE_TYPE_TIMER_H

#include <cstdint>
#include <string>
#include <glm/common.hpp>

class Timer {
    public:
        enum class SecondsValues : uint32_t {
            Week = 604'800,
            Day  = 86'400,
            Hour = 3'600,
            Min  = 60,
        };
    private:
        uint32_t  m_Seconds           = 0;
        float     m_SecondAccumulator = 0.0f;
    public:
        Timer() = default;
        Timer(uint32_t seconds_, float secondFraction_) {
            setTimer(seconds_, secondFraction_);
        }
        Timer(uint32_t seconds_)
            : Timer{ seconds_, 0.0f }
        {}

        bool isNegativeOrZero() const noexcept;

        float getTimeInSeconds() const noexcept;

        void setTimer(float seconds) noexcept;
        void setTimer(uint32_t seconds) noexcept;
        void setTimer(uint32_t seconds, float fraction) noexcept;

        void update(float dt) noexcept;
        void restart() noexcept;

        std::string toString() const;
};

#endif