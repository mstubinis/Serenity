#pragma once
#ifndef ENGINE_UTILS_BLOCK_PROFILER_H
#define ENGINE_UTILS_BLOCK_PROFILER_H

namespace Engine {
    template<typename LOGGER, typename DURATION_TYPE = std::chrono::nanoseconds>
    class block_profiler final {
        private:
            LOGGER&                                              m_Logger;
            std::chrono::time_point<std::chrono::steady_clock>   m_Start;
        public:
            block_profiler(LOGGER& logger)
                : m_Logger{ logger }
                , m_Start{ std::chrono::high_resolution_clock::now() }
            {}
            ~block_profiler() {
                m_Logger << (std::chrono::duration_cast<DURATION_TYPE>(std::chrono::high_resolution_clock::now() - m_Start)).count() << '\n';
            }
    };
};

#endif