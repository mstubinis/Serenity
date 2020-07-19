#pragma once
#ifndef ENGINE_UTILS_LOGGER_H
#define ENGINE_UTILS_LOGGER_H

class Logger final {
    public:
        struct Level final { enum Type: unsigned int {
            Debug,
            Release,
        };};
    private:
        Logger::Level::Type m_LevelType;

    public:
        Logger(Logger::Level::Type level) { m_LevelType = level; }

        void operator()(std::string_view message, const char* function, const char* file, int line) {
            if (m_LevelType == Logger::Level::Debug) {
                std::cout << "Function: " << function << ", File: " << file << ", Line: " << line << ", msg: " << message << "\n";
            }else if (m_LevelType == Logger::Level::Release) {
                //std::cout << message << "\n";
            }
        }
        constexpr Logger::Level::Type getLevel() const noexcept { return m_LevelType; }
};
inline Logger& Logger_Debug() {
    static Logger logger(Logger::Level::Debug);
    return logger;
}
inline Logger& Logger_Release() {
    static Logger logger(Logger::Level::Release);
    return logger;
}

#define ENGINE_LOG_FUNC(LOGGER, MESSAGE) LOGGER(static_cast<std::ostringstream&>(std::ostringstream().flush() << MESSAGE).str(), __FUNCTION__, __FILE__, __LINE__ );

#ifdef NDEBUG
    #define ENGINE_LOG(MESSAGE) ENGINE_LOG_FUNC(Logger_Release(), MESSAGE)
#else
    #define ENGINE_LOG(MESSAGE) ENGINE_LOG_FUNC(Logger_Debug(), MESSAGE)
#endif


#endif