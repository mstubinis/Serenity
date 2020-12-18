#pragma once
#ifndef ENGINE_UTILS_LOGGER_H
#define ENGINE_UTILS_LOGGER_H

#include <serenity/core/engine/system/TypeDefs.h>
#include <serenity/core/engine/system/Macros.h>
#include <sstream>
#include <string>
#include <string_view>

class Logger final {
    public:
        enum class Level : uint8_t {
            Debug,
            Release,
        };
    private:
        Logger::Level m_LevelType;

    public:
        Logger(Logger::Level level) { m_LevelType = level; }

        void operator()(std::string_view message, const char* function, const char* file, int line) noexcept {
            if (m_LevelType == Logger::Level::Debug) {
                ENGINE_PRODUCTION_LOG("Function: " << function << ", File: " << file << ", Line: " << line << ", msg: " << message)
            }else if (m_LevelType == Logger::Level::Release) {
                //ENGINE_PRODUCTION_LOG(message)
            }
        }
        inline constexpr Logger::Level getLevel() const noexcept { return m_LevelType; }
};
inline Logger& Logger_Debug() noexcept {
    static Logger logger(Logger::Level::Debug);
    return logger;
}
inline Logger& Logger_Release() noexcept {
    static Logger logger(Logger::Level::Release);
    return logger;
}

#define ENGINE_LOG_FUNC(LOGGER, MESSAGE) LOGGER(static_cast<std::ostringstream&>(std::ostringstream().flush() << MESSAGE).str(), __FUNCTION__, __FILE__, __LINE__);

#ifdef NDEBUG
    #define ENGINE_LOG(MESSAGE) ENGINE_LOG_FUNC(Logger_Release(), MESSAGE)
#else
    #define ENGINE_LOG(MESSAGE) ENGINE_LOG_FUNC(Logger_Debug(), MESSAGE)
#endif


#endif