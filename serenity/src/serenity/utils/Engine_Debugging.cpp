
#include <serenity/utils/Engine_Debugging.h>
#include <iomanip>
#include <serenity/system/Engine.h>

namespace {
    std::stringstream   STR_STREAM;
    std::string         OUTPUT_BUFFER;

    //opengl timers
    GLuint queryID                = 0;
    GLuint queryObject            = 0;
    uint32_t m_Output_frame_delay = 4;
    uint32_t m_Output_frame       = 0;
}

Engine::priv::DebugManager::DebugManager() {
    //TODO: check if this is still needed
    glGenQueries(1, &queryID);
    glQueryCounter(queryID, GL_TIMESTAMP);// dummy query to prevent OpenGL errors from popping out 
    OUTPUT_BUFFER.reserve(400);
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
    STR_STREAM.str({});
    STR_STREAM.clear();
    std::chrono::duration<double, std::milli> time = m_TimesNanoPrev[type];
    STR_STREAM << time.count();
    return STR_STREAM.str();
}
std::string Engine::priv::DebugManager::deltaTimeInMs() noexcept {
    STR_STREAM.str({});
    STR_STREAM.clear();
    const auto dt = Engine::priv::Core::m_Engine->m_Misc.m_Dt * 1000.0;
    STR_STREAM << dt;
    return STR_STREAM.str();
}
std::string Engine::priv::DebugManager::fps() const noexcept {
    const auto fps = Engine::priv::Core::m_Engine->m_Misc.m_FPS.fps();
    return std::to_string(fps);
}
std::string& Engine::priv::DebugManager::reportTime(uint32_t decimals_) {
    m_Decimals = decimals_;
    STR_STREAM.str({});
    STR_STREAM.clear();
    STR_STREAM << std::fixed << std::setprecision(m_Decimals);
    if ((m_Output_frame >= m_Output_frame_delay - 1) || m_Output_frame_delay == 0) {
        OUTPUT_BUFFER = "Update Time:  " + getTimeInMs(DebugTimerTypes::Logic) + " ms" +
                      "\nPhysics Time: " + getTimeInMs(DebugTimerTypes::Physics) + " ms" +
                      "\nSounds Time:  " + getTimeInMs(DebugTimerTypes::Sound) + " ms" +
                      "\nRender Time:  " + getTimeInMs(DebugTimerTypes::Render) + " ms" +
                      "\nDelta Time:   " + deltaTimeInMs() + " ms" +
                      "\nFPS: " + fps();
    }
    return OUTPUT_BUFFER;
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