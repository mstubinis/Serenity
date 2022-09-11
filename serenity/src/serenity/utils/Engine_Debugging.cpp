
#include <serenity/utils/Engine_Debugging.h>
#include <iomanip>
#include <serenity/system/Engine.h>


Engine::priv::DebugManager::DebugManager() {
    //TODO: check if this is still needed
    glGenQueries(1, &queryID);
    glQueryCounter(queryID, GL_TIMESTAMP);// dummy query to prevent OpenGL errors from popping out 
    m_OutputBuffer.reserve(128);
}

void Engine::priv::DebugManager::calculate() {
    m_TotalTime += Engine::priv::Core::m_Engine->m_Misc.m_Dt;
    ++m_Output_frame;
    if (m_Output_frame >= m_Output_frame_delay) {
        m_Output_frame = 0;
    }
}
void Engine::priv::DebugManager::beginGLQuery() {
    glBeginQuery(GL_TIME_ELAPSED, queryID); 
}
void Engine::priv::DebugManager::endGLQuery(const char* tag) {
    std::string msg = std::string(tag) + ": %f ms\n";
    glEndQuery(GL_TIME_ELAPSED);
    glGetQueryObjectuiv(queryID, GL_QUERY_RESULT, &queryObject);
    printf(msg.c_str(), queryObject / 1'000'000.0);
}
void Engine::priv::DebugManager::calculate(DebugTimerTypes type, std::chrono::nanoseconds timeNanoseconds) noexcept {
    m_TimesNano[type] += timeNanoseconds;
}
void Engine::priv::DebugManager::reset_timers() {
    m_TimesNanoPrev = m_TimesNano;
    for (auto& itr : m_TimesNano) {
        itr = std::chrono::nanoseconds(0);
    }
}
std::string Engine::priv::DebugManager::getTimeInMs(DebugTimerTypes type) noexcept {
    std::stringstream strstrm;
    strstrm << std::fixed << std::setprecision(m_Decimals);
    std::chrono::duration<double, std::milli> time = m_TimesNanoPrev[type];
    strstrm << time.count();
    return strstrm.str();
}
std::string Engine::priv::DebugManager::deltaTimeInMs() noexcept {
    std::stringstream strstrm;
    strstrm << std::fixed << std::setprecision(m_Decimals);
    const auto dt = Engine::priv::Core::m_Engine->m_Misc.m_Dt * 1000.0;
    strstrm << dt;
    return strstrm.str();
}
std::string Engine::priv::DebugManager::fps() const noexcept {
    const auto fps = Engine::priv::Core::m_Engine->m_Misc.m_FPS.fps();
    return std::to_string(fps);
}
std::string& Engine::priv::DebugManager::reportTime(uint32_t decimals_) {
    std::stringstream strstrm;
    m_Decimals = decimals_;
    strstrm << std::fixed << std::setprecision(m_Decimals);

    if ((m_Output_frame >= m_Output_frame_delay - 1) || m_Output_frame_delay == 0) {
        m_OutputBuffer = "Update Time:  " + getTimeInMs(DebugTimerTypes::Logic) + " ms" +
                      "\nPhysics Time: " + getTimeInMs(DebugTimerTypes::Physics) + " ms" +
                      "\nSounds Time:  " + getTimeInMs(DebugTimerTypes::Sound) + " ms" +
                      "\nRender Time:  " + getTimeInMs(DebugTimerTypes::Render) + " ms" +
                      "\nDelta Time:   " + deltaTimeInMs() + " ms" +
                      "\nFPS: " + fps();
    }
    return m_OutputBuffer;
}
std::string Engine::priv::DebugManager::reportDebug() {
    std::string out = "\n";
    size_t capacity = 1;
    for (const auto& str : m_Text_Queue) {
        capacity += str.size() + 1; // + 1 because each element appends a '\n'
    }
    out.reserve(capacity);
    for (const auto& str : m_Text_Queue) {
        out += str + '\n';
    }
    m_Text_Queue.clear();
    return out;
}