
#include <serenity/utils/Engine_Debugging.h>
#include <iomanip>
#include <serenity/system/Engine.h>

using namespace Engine;

void priv::DebugManager::_init() {
    //TODO: check if this is still needed
    glGenQueries(1, &queryID);
    glQueryCounter(queryID, GL_TIMESTAMP);// dummy query to prevent OpenGL errors from popping out 
    m_Output.reserve(400);
}
void priv::DebugManager::calculate() {
    m_TotalTime += Engine::priv::Core::m_Engine->m_Misc.m_Dt;
    ++m_Output_frame;
    if (m_Output_frame >= m_Output_frame_delay) {
        m_Output_frame = 0;
    }
}
void priv::DebugManager::beginGLQuery() { 
    glBeginQuery(GL_TIME_ELAPSED, queryID); 
}
void priv::DebugManager::endGLQuery(const char* tag) { 
    std::string msg = std::string(tag) + ": %f ms\n";
    glEndQuery(GL_TIME_ELAPSED);
    glGetQueryObjectuiv(queryID, GL_QUERY_RESULT, &queryObject);
    printf(msg.c_str(), queryObject / 1'000'000.0);
}
std::string priv::DebugManager::updateTimeInMs() noexcept {
    m_Strm.str({});
    m_Strm.clear();
    m_Strm << logicTime() * 1000.0;
    return m_Strm.str();
}
std::string priv::DebugManager::physicsTimeInMs() noexcept {
    m_Strm.str({});
    m_Strm.clear();
    m_Strm << physicsTime() * 1000.0;
    return m_Strm.str();
}
std::string priv::DebugManager::soundsTimeInMs() noexcept {
    m_Strm.str({});
    m_Strm.clear();
    m_Strm << soundsTime() * 1000.0;
    return m_Strm.str();
}
std::string priv::DebugManager::renderTimeInMs() noexcept {
    m_Strm.str({});
    m_Strm.clear();
    m_Strm << renderTime() * 1000.0;
    return m_Strm.str();
}
std::string priv::DebugManager::deltaTimeInMs() noexcept {
    m_Strm.str({});
    m_Strm.clear();
    const auto dt = Engine::priv::Core::m_Engine->m_Misc.m_Dt * 1000.0;
    m_Strm << dt;
    return m_Strm.str();
}
std::string priv::DebugManager::fps() const noexcept {
    const auto fps = Engine::priv::Core::m_Engine->m_Misc.m_FPS.fps();
    return std::to_string(fps);
}
std::string& priv::DebugManager::reportTime(uint32_t decimals_) {
    m_Decimals     = decimals_;
    m_Strm.str({});
    m_Strm.clear();
    m_Strm << std::fixed << std::setprecision(m_Decimals);
    if ((m_Output_frame >= m_Output_frame_delay - 1) || m_Output_frame_delay == 0) {
        m_Output = "Update Time:  " + updateTimeInMs() + " ms" +
                 "\nPhysics Time: " + physicsTimeInMs() + " ms" +
                 "\nSounds Time:  " + soundsTimeInMs() + " ms" +
                 "\nRender Time:  " + renderTimeInMs() + " ms" +
                 "\nDelta Time:   " + deltaTimeInMs() + " ms" +
                 "\nFPS: " + fps();
    }
    return m_Output;
}
std::string priv::DebugManager::reportDebug() {
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