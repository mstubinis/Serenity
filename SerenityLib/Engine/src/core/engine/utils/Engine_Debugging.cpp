#include "core/engine/utils/PrecompiledHeader.h"
#include <core/engine/utils/Engine_Debugging.h>

using namespace Engine;
using namespace std;


priv::DebugManager::DebugManager(){ 
    clock = sf::Clock();
    output = "";
    m_logicTime = m_physicsTime = m_renderTime = m_soundTime = 0;

    m_deltaTime = 0;
    m_totalTime = 0.0;
    divisor = 1000000.0;

    output_frame_delay = 4;
    output_frame = 0;
    decimals = 4;
    m_TimeScale = 1.0;
}
priv::DebugManager::~DebugManager() {
    cleanup();
}
void priv::DebugManager::cleanup() {

}
void priv::DebugManager::_init() {
    //TODO: check if this is still needed
	GLuint cast = static_cast<GLuint>(queryID);
    glGenQueries(1, &cast);
    glQueryCounter(queryID, GL_TIMESTAMP);// dummy query to prevent OpenGL errors from popping out 
}

void priv::DebugManager::addDebugLine(const char* message) {
    text_queue.emplace_back(message);
}
void priv::DebugManager::addDebugLine(string& message) {
    text_queue.emplace_back(message);
}
void priv::DebugManager::addDebugLine(string message) {
    text_queue.emplace_back(message);
}
void priv::DebugManager::calculate() {
    m_deltaTime = (m_logicTime + m_physicsTime + m_renderTime + m_soundTime);
    m_totalTime += dt();
    ++output_frame;
    if (output_frame >= output_frame_delay) {
        output_frame = 0;
    }
}
void priv::DebugManager::beginGLQuery() { 
    glBeginQuery(GL_TIME_ELAPSED, queryID); 
}
void priv::DebugManager::endGLQuery(const char* tag) { 
    string msg = string(tag) + ": %f ms\n";
    glEndQuery(GL_TIME_ELAPSED);
    glGetQueryObjectuiv(queryID, GL_QUERY_RESULT, &queryObject);
    printf(msg.c_str(), queryObject / 1000000.0);
}
void priv::DebugManager::stop_clock() { clock.restart(); }

void priv::DebugManager::calculate_logic() { m_logicTime = clock.restart().asMicroseconds(); }
void priv::DebugManager::calculate_physics() { m_physicsTime = clock.restart().asMicroseconds(); }
void priv::DebugManager::calculate_sounds() { m_soundTime = clock.restart().asMicroseconds(); }
void priv::DebugManager::calculate_render() { m_renderTime = clock.restart().asMicroseconds(); }

float priv::DebugManager::dt() const { return (float)((float)m_deltaTime / divisor); }
double priv::DebugManager::logicTime() const { return (double)((double)m_logicTime / divisor); }
double priv::DebugManager::physicsTime() const { return (double)((double)m_physicsTime / divisor); }
double priv::DebugManager::renderTime() const { return (double)((double)m_renderTime / divisor); }
double priv::DebugManager::soundsTime() const { return (double)((double)m_soundTime / divisor); }
double priv::DebugManager::totalTime() const{ return m_totalTime; }
float priv::DebugManager::timeScale() const { return m_TimeScale; }

void priv::DebugManager::setTimeScale(const float timeScale) {
    m_TimeScale = glm::max(0.0f, timeScale);
}

string& priv::DebugManager::reportTime() { return reportTime(decimals); }
string& priv::DebugManager::reportTime(const unsigned int decimals_) {
    decimals = decimals_;
    if ((output_frame >= output_frame_delay - 1) || output_frame_delay == 0) {
        uint fps = uint(1.0f / dt());
        stringstream st1, st2, st3, st4, st5;
        st1 << std::fixed << std::setprecision(decimals) << logicTime() * 1000.0;
        st2 << std::fixed << std::setprecision(decimals) << physicsTime() * 1000.0;
        st5 << std::fixed << std::setprecision(decimals) << soundsTime() * 1000.0;
        st3 << std::fixed << std::setprecision(decimals) << renderTime() * 1000.0;
        st4 << std::fixed << std::setprecision(decimals) << dt() * 1000.0f;
        string s1 = st1.str(); string s2 = st2.str(); string s3 = st3.str(); string s4 = st4.str(); string s5 = st5.str();

        output = "Update Time:  " + s1 + " ms" + 
               "\nPhysics Time: " + s2 + " ms" + 
                "\nSounds Time:  " + s5 + " ms" +
                "\nRender Time:  " + s3 + " ms" + 
                "\nDelta Time:   " + s4 + " ms" +
                "\nFPS: " + to_string(fps);
    }
    return output;
}
string priv::DebugManager::reportDebug() {
    string out = "\n";
    for (auto& str : text_queue) {
        out += str + "\n";
    }
    text_queue.clear();
    return out;
}