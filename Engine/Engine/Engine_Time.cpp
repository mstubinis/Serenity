#include "Engine_Time.h"
#include <SFML/Window.hpp>
#include <iomanip>
#include <sstream>

using namespace Engine;
using namespace std;

class epriv::TimeManager::impl{
    public:
        sf::Clock clock;
        float logicTime, physicsTime, renderTime, soundTime, displayTime, deltaTime;
        uint output_frame_delay, output_frame;
        uint decimals;
        string output;

		//opengl timers
		uint queryID;
		GLuint queryObject;

        void _init(const char* name,uint& w,uint& h){
            clock = sf::Clock();
            logicTime = physicsTime = renderTime = soundTime = displayTime = 0.0f;

            deltaTime = 0.016666f;

            output_frame_delay = 4;
            output_frame = 0;
            decimals = 6;
        }
        void _postInit(const char* name,uint& w,uint& h){
			glGenQueries(1, &queryID);
			glQueryCounter(queryID, GL_TIMESTAMP);// dummy query to prevent OpenGL errors from popping out
        }
		void _beginQuery(){
			glBeginQuery(GL_TIME_ELAPSED,queryID);
		}
		void _endQuery(string& tag){
			string _s = tag + ": %f ms\n";
			glEndQuery(GL_TIME_ELAPSED);

			glGetQueryObjectuiv(queryID, GL_QUERY_RESULT,&queryObject);
			
			printf(_s.c_str(),queryObject / 1000000.0);
		}
};
epriv::TimeManager::TimeManager(const char* name,uint w,uint h):m_i(new impl){m_i->_init(name,w,h);}
epriv::TimeManager::~TimeManager(){}
void epriv::TimeManager::_init(const char* name,uint w,uint h){ m_i->_postInit(name,w,h); }
void epriv::TimeManager::calculate(){
	auto& i = *m_i.get();
    i.deltaTime = float(i.logicTime + i.physicsTime + i.renderTime + i.displayTime + i.soundTime);

    ++i.output_frame;
    if(i.output_frame >= i.output_frame_delay){
        i.output_frame = 0;
    }
}
void epriv::TimeManager::beginGLQuery(){ m_i->_beginQuery(); }
void epriv::TimeManager::endGLQuery(std::string& tag){ m_i->_endQuery(tag); }
void epriv::TimeManager::stop_clock(){ m_i->clock.restart(); }

void epriv::TimeManager::calculate_logic(){ m_i->logicTime = m_i->clock.restart().asSeconds(); }
void epriv::TimeManager::calculate_physics(){ m_i->physicsTime = m_i->clock.restart().asSeconds(); }
void epriv::TimeManager::calculate_sounds(){ m_i->soundTime = m_i->clock.restart().asSeconds(); }
void epriv::TimeManager::calculate_render(){ m_i->renderTime = m_i->clock.restart().asSeconds(); }
void epriv::TimeManager::calculate_display(){ m_i->displayTime = m_i->clock.restart().asSeconds(); }

float& epriv::TimeManager::dt() const{ return m_i->deltaTime; }
const float& epriv::TimeManager::logicTime() const{ return m_i->logicTime; }
const float& epriv::TimeManager::physicsTime() const{ return m_i->physicsTime; }
const float& epriv::TimeManager::renderTime() const{ return m_i->renderTime; }
const float& epriv::TimeManager::soundsTime() const{ return m_i->soundTime; }
const float& epriv::TimeManager::displayTime() const{ return m_i->displayTime; }

string& epriv::TimeManager::reportTime(){ return epriv::TimeManager::reportTime(m_i->decimals); }
string& epriv::TimeManager::reportTime(uint decimals){
	auto& i = *m_i.get();
    i.decimals = decimals;
    if((i.output_frame >= i.output_frame_delay - 1) || i.output_frame_delay == 0){
        uint fps = uint(1.0f / i.deltaTime);
        stringstream st1, st2, st3, st4, st5, st6;
        st1 << std::fixed << std::setprecision(decimals) << i.logicTime * 1000.0f;
        st2 << std::fixed << std::setprecision(decimals) << i.physicsTime * 1000.0f;
        st5 << std::fixed << std::setprecision(decimals) << i.soundTime * 1000.0f;
        st3 << std::fixed << std::setprecision(decimals) << i.renderTime * 1000.0f;
        st4 << std::fixed << std::setprecision(decimals) << i.deltaTime * 1000.0f;
        st6 << std::fixed << std::setprecision(decimals) << i.displayTime * 1000.0f;
        string s1=st1.str(); string s2=st2.str(); string s3=st3.str(); string s4=st4.str(); string s5=st5.str(); string s6=st6.str();

        m_i->output =        "Update Time:  " + s1 + " ms" + "\nPhysics Time: " + s2 + " ms" + "\nSounds Time:  " + s5 + " ms" +
                          "\nRender Time:  " + s3 + " ms" + "\nDisplay Time: " + s6 + " ms" + "\nDelta Time:   " + s4 + " ms" +
                          "\nFPS: " + to_string(fps);
    }
    return m_i->output;
}