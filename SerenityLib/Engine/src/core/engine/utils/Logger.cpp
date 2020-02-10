#include <core/engine/utils/Logger.h>

using namespace std;

Logger::Logger(const Logger::Level::Type l) {
    m_Type = l;
}
void Logger::operator()(const string& message, char const* function, char const* file, int line) {
    if (m_Type == Logger::Level::Debug) {
        std::cout << "Function: " << function << ", File: " << file << ", Line: " << line << ", msg: " << message << "\n";
    }else if (m_Type == Logger::Level::Release) {
        //std::cout << message << "\n";
    }
}
Logger& Logger_Debug() {
    static Logger logger(Logger::Level::Debug);
    return logger;
}
Logger& Logger_Release() {
    static Logger logger(Logger::Level::Release);
    return logger;
}