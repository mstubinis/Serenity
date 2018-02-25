#include "Engine_Time.h"
#include "Engine_Resources.h"
#include <SFML/Window.hpp>
#include <iomanip>
#include <sstream>

using namespace Engine;
using namespace std;

class epriv::TimeManager::impl{
    public:
        sf::Clock update_clock;  sf::Clock physics_clock;  sf::Clock render_clock;   sf::Clock sounds_clock;
        double updateTime;        double physicsTime;        double renderTime;      double soundTime;

		sf::Clock rendering_display_clock;  double rendering_display_Time;

        float applicationTime;
        float deltaTime;
        uint output_frame_delay;
        uint output_frame;
		uint decimals;
        string currOutput;
        string prevOutput;

        void _init(const char* name,uint& w,uint& h){
            update_clock = sf::Clock();  physics_clock = sf::Clock();  render_clock = sf::Clock();  sounds_clock = sf::Clock();
            updateTime = 0;              physicsTime = 0;              renderTime = 0;              soundTime = 0;

			rendering_display_clock = sf::Clock();   rendering_display_Time = 0;


            applicationTime = 0;
            deltaTime = 1.0f;

            output_frame_delay = 4;
            output_frame = 0;
			decimals = 6;
            string currOutput = "";
            string prevOutput = "";
        }
		void _postInit(const char* name,uint& w,uint& h){
		}
};
epriv::TimeManager::TimeManager(const char* name,uint w,uint h):m_i(new impl){m_i->_init(name,w,h);}
epriv::TimeManager::~TimeManager(){}
void epriv::TimeManager::_init(const char* name,uint w,uint h){ m_i->_postInit(name,w,h); }
void epriv::TimeManager::calculate(){
    m_i->deltaTime = float(m_i->updateTime + m_i->physicsTime + m_i->renderTime);
    m_i->applicationTime += m_i->deltaTime;

    m_i->output_frame++;
    if(m_i->output_frame >= m_i->output_frame_delay){
        m_i->output_frame = 0;
    }
}
void epriv::TimeManager::stop_update(){ m_i->update_clock.restart(); }
void epriv::TimeManager::stop_physics(){ m_i->physics_clock.restart(); }
void epriv::TimeManager::stop_sounds(){ m_i->sounds_clock.restart(); }
void epriv::TimeManager::stop_render(){ m_i->render_clock.restart(); }
void epriv::TimeManager::calculate_update(){ m_i->updateTime = m_i->update_clock.restart().asSeconds(); }
void epriv::TimeManager::calculate_physics(){ m_i->physicsTime = m_i->physics_clock.restart().asSeconds(); }
void epriv::TimeManager::calculate_sounds(){ m_i->soundTime = m_i->sounds_clock.restart().asSeconds(); }
void epriv::TimeManager::calculate_render(){ m_i->renderTime = m_i->render_clock.restart().asSeconds(); }
float epriv::TimeManager::dt(){ return m_i->deltaTime; }
float epriv::TimeManager::applicationTime(){ return m_i->applicationTime; }
float epriv::TimeManager::updateTime(){ return (float)m_i->updateTime; }
float epriv::TimeManager::physicsTime(){ return (float)m_i->physicsTime; }
float epriv::TimeManager::renderTime(){ return (float)m_i->renderTime; }
float epriv::TimeManager::soundsTime(){ return (float)m_i->soundTime; }

void epriv::TimeManager::stop_rendering_display(){ m_i->rendering_display_clock.restart(); }

void epriv::TimeManager::calculate_rendering_display(){ m_i->rendering_display_Time = m_i->rendering_display_clock.restart().asSeconds(); }

float epriv::TimeManager::rendering_displayTime(){ return (float)m_i->rendering_display_Time; }


string& epriv::TimeManager::reportTime(){ return epriv::TimeManager::reportTime(m_i->decimals); }
string& epriv::TimeManager::reportTime(uint decimals){
	m_i->decimals = decimals;
    m_i->prevOutput = m_i->currOutput;
    if((m_i->output_frame >= m_i->output_frame_delay-1) || m_i->output_frame_delay == 0){
		uint fps = uint(1.0f/m_i->deltaTime);
		stringstream st1, st2, st3, st4, st5, st6;
		st1 << std::fixed << std::setprecision(decimals) << m_i->updateTime;
		st2 << std::fixed << std::setprecision(decimals) << m_i->physicsTime;
		st5 << std::fixed << std::setprecision(decimals) << m_i->soundTime;
		st3 << std::fixed << std::setprecision(decimals) << m_i->renderTime;
		st4 << std::fixed << std::setprecision(decimals) << m_i->deltaTime;
		st6 << std::fixed << std::setprecision(decimals) << m_i->rendering_display_Time;
		string s1=st1.str(); string s2=st2.str(); string s3=st3.str(); string s4=st4.str(); string s5=st5.str(); string s6=st6.str();

        m_i->currOutput =   "Update Time:  " + s1 +
                          "\nPhysics Time: " + s2 +
                          "\nSounds Time:  " + s5 +
                          "\nRender Time:  " + s3 +
						  "\nDisplay Time: " + s6 +
                          "\nDelta Time:   " + s4 +
                          "\nFPS: " + to_string(fps);
        m_i->prevOutput = m_i->currOutput;
        return m_i->currOutput;
    }
    return m_i->prevOutput;
}