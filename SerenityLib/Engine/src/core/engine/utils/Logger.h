#pragma once
#ifndef ENGINE_UTILS_LOGGER_H
#define ENGINE_UTILS_LOGGER_H

#include <string>
#include <iostream>
#include <ostream>
#include <sstream>

class Logger final {
    public:
        struct Level final { enum Type: unsigned int {
            Debug,
            Release,
        };};

    private:
        Logger::Level::Type m_Type;

    public:
        Logger(const Logger::Level::Type l);

        void operator()(const std::string& message, char const* function, char const* file, int line);
};
Logger& Logger_Debug();
Logger& Logger_Release();

#define ENGINE_LOG_FUNC(Logger_, Message_) Logger_(static_cast<std::ostringstream&>(std::ostringstream().flush() << Message_).str(), __FUNCTION__, __FILE__, __LINE__ );

#ifdef NDEBUG
    #define ENGINE_LOG(Message_) ENGINE_LOG_FUNC(Logger_Release(), Message_)
#else
    #define ENGINE_LOG(Message_) ENGINE_LOG_FUNC(Logger_Debug(), Message_)
#endif


#endif