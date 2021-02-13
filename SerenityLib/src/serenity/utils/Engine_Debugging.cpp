
#include <serenity/utils/Engine_Debugging.h>
#include <iomanip>
#include <sstream>
#include <serenity/system/Engine.h>

using namespace Engine;

void priv::DebugManager::_init() {
    //TODO: check if this is still needed
	GLuint cast = (GLuint)queryID;
    glGenQueries(1, &cast);
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

std::string& priv::DebugManager::reportTime(uint32_t decimals_) {
    m_Decimals     = decimals_;
    if ((m_Output_frame >= m_Output_frame_delay - 1) || m_Output_frame_delay == 0) {
        std::stringstream st1, st2, st3, st4, st5;
        const auto fps = Engine::priv::Core::m_Engine->m_Misc.m_FPS.fps();
        const auto dt  = (Engine::priv::Core::m_Engine->m_Misc.m_Dt * 1000.0);
        st1 << std::fixed << std::setprecision(m_Decimals) << logicTime()   * 1000.0;
        st2 << std::fixed << std::setprecision(m_Decimals) << physicsTime() * 1000.0;
        st3 << std::fixed << std::setprecision(m_Decimals) << soundsTime()  * 1000.0;
        st4 << std::fixed << std::setprecision(m_Decimals) << renderTime()  * 1000.0;
        st5 << std::fixed << std::setprecision(m_Decimals) << dt;

        m_Output = "Update Time:  " + st1.str() + " ms" +
                 "\nPhysics Time: " + st2.str() + " ms" +
                 "\nSounds Time:  " + st3.str() + " ms" +
                 "\nRender Time:  " + st4.str() + " ms" +
                 "\nDelta Time:   " + st5.str() + " ms" +
                 "\nFPS: " + std::to_string(fps);
    }
    return m_Output;
}
std::string priv::DebugManager::reportDebug() {
    std::string out = "\n";
    for (const auto& str : m_Text_Queue) {
        out += str + '\n';
    }
    m_Text_Queue.clear();
    return out;
}