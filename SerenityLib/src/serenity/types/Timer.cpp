#include <serenity/types/Timer.h>

namespace {
    std::string toStringWithLeadingZeros(uint32_t val, uint8_t numDigits) {
        std::string result(numDigits--, '0');
        for (uint32_t v = val; numDigits >= 0 && v != 0; --numDigits, v /= 10)
            result[numDigits] = '0' + v % 10;
        return result;
    }
    std::string m_ToStringBuffer;
}



void Timer::setTimer(float seconds) noexcept {
    int wholeSecond = static_cast<int>(seconds);
    seconds -= static_cast<float>(wholeSecond);
    m_Seconds = wholeSecond;
    m_SecondAccumulator = seconds;
}
void Timer::setTimer(uint32_t seconds) noexcept {
    m_Seconds = seconds;
    m_SecondAccumulator = 0.0f;
}
void Timer::setTimer(uint32_t seconds, float fraction) noexcept {
    m_Seconds = seconds;
    m_SecondAccumulator = glm::clamp(fraction, 0.0f, 1.0f);
}
void Timer::update(float dt) noexcept {
    m_SecondAccumulator += std::abs(dt);
    int wholeSecond      = static_cast<int>(std::abs(m_SecondAccumulator));
    m_SecondAccumulator -= static_cast<float>(wholeSecond);
    if (dt > 0.0f) { 
        m_Seconds += wholeSecond;
    } else {
        m_Seconds = (m_Seconds >= wholeSecond) ? m_Seconds - static_cast<uint32_t>(wholeSecond) : 0;
    }
}
bool Timer::isNegativeOrZero() const noexcept {
    return m_Seconds == 0 && m_SecondAccumulator <= 0.0f;
}
float Timer::getTimeInSeconds() const noexcept {
    return static_cast<float>(m_Seconds) + m_SecondAccumulator;
}
void Timer::restart() noexcept {
    m_Seconds = 0;
    m_SecondAccumulator = 0.0f;
}
std::string Timer::toString() const {
    uint32_t secondsTemp = m_Seconds;
    uint16_t weeks       = 0;
    uint16_t days        = 0;
    uint8_t hours        = 0;
    uint8_t mins         = 0;
    auto calculate_time = [](uint32_t& secs, uint32_t compare, auto& counter) {
        while (secs >= compare) {
            secs -= compare;
            ++counter;
        }
    };
    calculate_time(secondsTemp, static_cast<uint32_t>(SecondsValues::Week), weeks);
    calculate_time(secondsTemp, static_cast<uint32_t>(SecondsValues::Day), days);
    calculate_time(secondsTemp, static_cast<uint32_t>(SecondsValues::Hour), hours);
    calculate_time(secondsTemp, static_cast<uint32_t>(SecondsValues::Min), mins);
    //secondsTemp will now be in seconds

    m_ToStringBuffer.clear();
    if (weeks > 0)
        m_ToStringBuffer += std::to_string(weeks) + ":";
    if (days > 0 || weeks > 0)
        m_ToStringBuffer += std::to_string(days) + ":"; //only post a zero if weeks was > 0
    if (hours > 0 || days > 0)
        m_ToStringBuffer += toStringWithLeadingZeros(hours, 2) + ":"; //should always be two digits, use leading zeros
    if (mins > 0 || hours > 0)
        m_ToStringBuffer += toStringWithLeadingZeros(mins, 2) + ":"; //should always be two digits, use leading zeros
    if (secondsTemp > 0 || mins > 0)
        m_ToStringBuffer += toStringWithLeadingZeros(secondsTemp, 2) + ":"; //should always be two digits, use leading zeros
    if (m_ToStringBuffer.size() > 0 && m_ToStringBuffer.back() == ':') {
        m_ToStringBuffer.pop_back();
    }
    return m_ToStringBuffer;
}