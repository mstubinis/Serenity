#pragma once
#ifndef ENGINE_UTILS_BLOCK_PROFILER_H
#define ENGINE_UTILS_BLOCK_PROFILER_H

#include <GL/glew.h>
#include <SFML/OpenGL.hpp>
#include <iomanip>

namespace Engine {
    template<typename LOGGER, typename DURATION_TYPE = std::chrono::nanoseconds>
    class block_profiler final {
        private:
            LOGGER&                                              m_Logger;
            std::chrono::time_point<std::chrono::steady_clock>   m_Start;
            std::string                                          m_Tag;
        public:
            block_profiler(LOGGER& logger, const std::string& tag = "")
                : m_Logger{ logger }
                , m_Start{ std::chrono::high_resolution_clock::now() }
                , m_Tag{ tag }
            {}
            ~block_profiler() {
                m_Logger << m_Tag << (std::chrono::duration_cast<DURATION_TYPE>(std::chrono::high_resolution_clock::now() - m_Start)).count() << '\n';
            }
    };


    template<typename LOGGER>
    class block_profiler_opengl final {
        private:
            LOGGER&              m_Logger;
            std::string          m_Tag;

            GLuint64 m_StartTime, m_StopTime;
            GLuint   m_QueryID[2];
        public:
            block_profiler_opengl(LOGGER& logger, const std::string& tag = "")
                : m_Logger{ logger }
                , m_Tag{ tag }
            {
                glGenQueries(2, m_QueryID);
                glQueryCounter(m_QueryID[0], GL_TIMESTAMP);
            }
            ~block_profiler_opengl() {
                glQueryCounter(m_QueryID[1], GL_TIMESTAMP);
                // wait until the results are available
                GLint stopTimerAvailable = 0;
                while (!stopTimerAvailable) {
                    glGetQueryObjectiv(m_QueryID[1], GL_QUERY_RESULT_AVAILABLE, &stopTimerAvailable);
                }
                // get query results
                glGetQueryObjectui64v(m_QueryID[0], GL_QUERY_RESULT, &m_StartTime);
                glGetQueryObjectui64v(m_QueryID[1], GL_QUERY_RESULT, &m_StopTime);
                const auto msTime = (m_StopTime - m_StartTime) / 1000000.0;
                std::cout << m_Tag << ": " << std::fixed << std::setprecision(4) << msTime << " ms\n";
                glDeleteQueries(2, m_QueryID);
            }
    };

};

#endif