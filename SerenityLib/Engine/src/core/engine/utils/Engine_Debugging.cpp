#include "core/engine/utils/PrecompiledHeader.h"
#include <core/engine/utils/Engine_Debugging.h>

using namespace Engine;
using namespace std;


void priv::DebugManager::_init() {
    //TODO: check if this is still needed
	GLuint cast = (GLuint)queryID;
    glGenQueries(1, &cast);
    glQueryCounter(queryID, GL_TIMESTAMP);// dummy query to prevent OpenGL errors from popping out 
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
void priv::DebugManager::setTimeScale(float timeScale) {
    m_TimeScale = glm::max(0.0f, timeScale);
}

string& priv::DebugManager::reportTime() { return reportTime(decimals); }
string& priv::DebugManager::reportTime(unsigned int decimals_) {
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