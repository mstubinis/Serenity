#include "core/engine/Engine_Debugging.h"
#include <SFML/Window.hpp>
#include <iomanip>
#include <sstream>

using namespace Engine;
using namespace std;

class epriv::DebugManager::impl {
    public:
        sf::Clock clock;
        sf::Int64 logicTime, physicsTime, renderTime, soundTime, displayTime;
        sf::Int64 deltaTime;
        double divisor;
        uint output_frame_delay, output_frame;
        uint decimals;
        string output;

        //opengl timers
        uint queryID;
        GLuint queryObject;

        //general text debugging
        std::vector<std::string> text_queue;

        void _init(const char* name, uint& w, uint& h) {
            clock = sf::Clock();
            logicTime = physicsTime = renderTime = soundTime = displayTime = 0;

            deltaTime = 0;
            divisor = 1000000.0;

            output_frame_delay = 4;
            output_frame = 0;
            decimals = 8;
        }
        void _postInit(const char* name, uint& w, uint& h) {
            glGenQueries(1, &queryID);
            glQueryCounter(queryID, GL_TIMESTAMP);// dummy query to prevent OpenGL errors from popping out
        }
        void _beginQuery() {
            glBeginQuery(GL_TIME_ELAPSED, queryID);
        }
        void _endQuery(const char* tag) {
            string _s = string(tag) + ": %f ms\n";
            glEndQuery(GL_TIME_ELAPSED);
            glGetQueryObjectuiv(queryID, GL_QUERY_RESULT, &queryObject);
            printf(_s.c_str(), queryObject / 1000000.0);
        }
};
epriv::DebugManager::DebugManager(const char* name, uint w, uint h) :m_i(new impl) { m_i->_init(name, w, h); }
epriv::DebugManager::~DebugManager() {}
void epriv::DebugManager::_init(const char* name, uint w, uint h) { m_i->_postInit(name, w, h); }

void epriv::DebugManager::addDebugLine(const char* message) {
    m_i->text_queue.emplace_back(message);
}

void epriv::DebugManager::calculate() {
    auto& i = *m_i.get();
    i.deltaTime = i.logicTime + i.physicsTime + i.renderTime + i.displayTime + i.soundTime;

    ++i.output_frame;
    if (i.output_frame >= i.output_frame_delay) {
        i.output_frame = 0;
    }
}
void epriv::DebugManager::beginGLQuery() { m_i->_beginQuery(); }
void epriv::DebugManager::endGLQuery(const char* tag) { m_i->_endQuery(tag); }
void epriv::DebugManager::stop_clock() { m_i->clock.restart(); }

void epriv::DebugManager::calculate_logic() { m_i->logicTime = m_i->clock.restart().asMicroseconds(); }
void epriv::DebugManager::calculate_physics() { m_i->physicsTime = m_i->clock.restart().asMicroseconds(); }
void epriv::DebugManager::calculate_sounds() { m_i->soundTime = m_i->clock.restart().asMicroseconds(); }
void epriv::DebugManager::calculate_render() { m_i->renderTime = m_i->clock.restart().asMicroseconds(); }
void epriv::DebugManager::calculate_display() { m_i->displayTime = m_i->clock.restart().asMicroseconds(); }

const double epriv::DebugManager::dt() const { return (double)((double)m_i->deltaTime / m_i->divisor); }
const double epriv::DebugManager::logicTime() const { return (double)((double)m_i->logicTime / m_i->divisor); }
const double epriv::DebugManager::physicsTime() const { return (double)((double)m_i->physicsTime / m_i->divisor); }
const double epriv::DebugManager::renderTime() const { return (double)((double)m_i->renderTime / m_i->divisor); }
const double epriv::DebugManager::soundsTime() const { return (double)((double)m_i->soundTime / m_i->divisor); }
const double epriv::DebugManager::displayTime() const { return (double)((double)m_i->displayTime / m_i->divisor); }

string& epriv::DebugManager::reportTime() { return reportTime(m_i->decimals); }
string& epriv::DebugManager::reportTime(uint decimals) {
    auto& i = *m_i.get();
    i.decimals = decimals;
    if ((i.output_frame >= i.output_frame_delay - 1) || i.output_frame_delay == 0) {
        uint fps = uint(1.0 / ((double)i.deltaTime / m_i->divisor));
        stringstream st1, st2, st3, st4, st5, st6;
        st1 << std::fixed << std::setprecision(decimals) << logicTime() * 1000.0;
        st2 << std::fixed << std::setprecision(decimals) << physicsTime() * 1000.0;
        st5 << std::fixed << std::setprecision(decimals) << soundsTime() * 1000.0;
        st3 << std::fixed << std::setprecision(decimals) << renderTime() * 1000.0;
        st4 << std::fixed << std::setprecision(decimals) << dt() * 1000.0;
        st6 << std::fixed << std::setprecision(decimals) << displayTime() * 1000.0;
        string s1 = st1.str(); string s2 = st2.str(); string s3 = st3.str(); string s4 = st4.str(); string s5 = st5.str(); string s6 = st6.str();

        m_i->output = "Update Time:  " + s1 + " ms" + "\nPhysics Time: " + s2 + " ms" + "\nSounds Time:  " + s5 + " ms" +
            "\nRender Time:  " + s3 + " ms" + "\nDisplay Time: " + s6 + " ms" + "\nDelta Time:   " + s4 + " ms" +
            "\nFPS: " + to_string(fps);
    }
    return m_i->output;
}
string epriv::DebugManager::reportDebug() {
    string out = "\n";
    for (auto str : m_i->text_queue) {
        out += str + "\n";
    }
    m_i->text_queue.clear();
    return out;
}