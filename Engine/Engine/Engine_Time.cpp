#include "Engine_Time.h"
#include "Engine_Resources.h"
#include <SFML/Window.hpp>
#include <iomanip>
#include <sstream>

class EngineTime::impl{
    public:
        sf::Clock update_clock;  sf::Clock physics_clock;  sf::Clock render_clock;   sf::Clock sounds_clock;
        double updateTime;        double physicsTime;        double renderTime;      double soundTime;

        float applicationTime;
        float deltaTime;
        uint output_frame_delay;
        uint output_frame;
		uint decimals;
        std::string currOutput;
        std::string prevOutput;

        void _init(){
            update_clock = sf::Clock();  physics_clock = sf::Clock();  render_clock = sf::Clock();  sounds_clock = sf::Clock();
            updateTime = 0;              physicsTime = 0;              renderTime = 0;              soundTime = 0;

            applicationTime = 0;
            deltaTime = 1.0f;

            output_frame_delay = 4;
            output_frame = 0;
			decimals = 6;
            std::string currOutput = "";
            std::string prevOutput = "";
        }
};
EngineTime::EngineTime():m_i(new impl){m_i->_init();}
EngineTime::~EngineTime(){}
void EngineTime::calculate(){
    m_i->deltaTime = float(m_i->updateTime + m_i->physicsTime + m_i->renderTime);
    m_i->applicationTime += m_i->deltaTime;

    m_i->output_frame++;
    if(m_i->output_frame >= m_i->output_frame_delay){
        m_i->output_frame = 0;
    }
}
void EngineTime::stop_update(){ m_i->update_clock.restart(); }
void EngineTime::stop_physics(){ m_i->physics_clock.restart(); }
void EngineTime::stop_sounds(){ m_i->sounds_clock.restart(); }
void EngineTime::stop_render(){ m_i->render_clock.restart(); }
void EngineTime::calculate_update(){ m_i->updateTime = m_i->update_clock.restart().asSeconds(); }
void EngineTime::calculate_physics(){ m_i->physicsTime = m_i->physics_clock.restart().asSeconds(); }
void EngineTime::calculate_sounds(){ m_i->soundTime = m_i->sounds_clock.restart().asSeconds(); }
void EngineTime::calculate_render(){ m_i->renderTime = m_i->render_clock.restart().asSeconds(); }
float EngineTime::dt(){ return m_i->deltaTime; }
float EngineTime::applicationTime(){ return m_i->applicationTime; }
float EngineTime::updateTime(){ return m_i->updateTime; }
float EngineTime::physicsTime(){ return m_i->physicsTime; }
float EngineTime::renderTime(){ return m_i->renderTime; }
float EngineTime::soundsTime(){ return m_i->soundTime; }
std::string& EngineTime::reportTime(){ return EngineTime::reportTime(m_i->decimals); }
std::string& EngineTime::reportTime(uint decimals){
	m_i->decimals = decimals;
    m_i->prevOutput = m_i->currOutput;
    if((m_i->output_frame >= m_i->output_frame_delay-1) || m_i->output_frame_delay == 0){
		uint fps = uint(1.0f/m_i->deltaTime);
		std::stringstream stream1;std::stringstream stream2;std::stringstream stream3;std::stringstream stream4;std::stringstream stream5;
		stream1 << std::fixed << std::setprecision(decimals) << m_i->updateTime;
		stream2 << std::fixed << std::setprecision(decimals) << m_i->physicsTime;
		stream5 << std::fixed << std::setprecision(decimals) << m_i->soundTime;
		stream3 << std::fixed << std::setprecision(decimals) << m_i->renderTime;
		stream4 << std::fixed << std::setprecision(decimals) << m_i->deltaTime;
		std::string s1 = stream1.str(); std::string s2 = stream2.str(); std::string s3 = stream3.str(); std::string s4 = stream4.str(); std::string s5 = stream5.str();

        m_i->currOutput =   "Update Time:  " + s1 +
                          "\nPhysics Time: " + s2 +
                          "\nSounds Time:  " + s5 +
                          "\nRender Time:  " + s3 +
                          "\nDelta Time:   " + s4 +
                          "\nFPS: " + to_string(fps);
        m_i->prevOutput = m_i->currOutput;
        return m_i->currOutput;
    }
    return m_i->prevOutput;
}