#include "Engine_Time.h"
#include "Engine_Resources.h"
#include <SFML/Window.hpp>
#include <iomanip>
#include <sstream>

class EngineTime::impl{
    public:
        sf::Clock update_clock;  sf::Clock physics_clock;  sf::Clock render_clock;   sf::Clock sounds_clock;
        double updateTime;        double physicsTime;        double renderTime;      double soundTime;

		//rendering data
        sf::Clock rendering_geometry_clock; double rendering_geometry_Time;
		sf::Clock rendering_lighting_clock; double rendering_lighting_Time;
		sf::Clock rendering_ssao_clock;     double rendering_ssao_Time;
		sf::Clock rendering_godrays_clock;  double rendering_godrays_Time;
		sf::Clock rendering_aa_clock;       double rendering_aa_Time;

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


			rendering_geometry_clock = sf::Clock();  rendering_geometry_Time = 0;
			rendering_lighting_clock = sf::Clock();  rendering_lighting_Time = 0;
			rendering_ssao_clock = sf::Clock();      rendering_ssao_Time = 0;
			rendering_godrays_clock = sf::Clock();   rendering_godrays_Time = 0;
			rendering_aa_clock = sf::Clock();        rendering_aa_Time = 0;


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



void EngineTime::stop_rendering_geometry(){ m_i->rendering_geometry_clock.restart(); }
void EngineTime::stop_rendering_lighting(){ m_i->rendering_lighting_clock.restart(); }
void EngineTime::stop_rendering_ssao(){ m_i->rendering_ssao_clock.restart(); }
void EngineTime::stop_rendering_aa(){ m_i->rendering_aa_clock.restart(); }
void EngineTime::stop_rendering_godrays(){ m_i->rendering_godrays_clock.restart(); }

void EngineTime::calculate_rendering_geometry(){ m_i->rendering_geometry_Time = m_i->rendering_geometry_clock.restart().asSeconds(); }
void EngineTime::calculate_rendering_lighting(){ m_i->rendering_lighting_Time = m_i->rendering_lighting_clock.restart().asSeconds(); }
void EngineTime::calculate_rendering_ssao(){ m_i->rendering_ssao_Time = m_i->rendering_ssao_clock.restart().asSeconds(); }
void EngineTime::calculate_rendering_aa(){ m_i->rendering_aa_Time = m_i->rendering_aa_clock.restart().asSeconds(); }
void EngineTime::calculate_rendering_godrays(){ m_i->rendering_godrays_Time = m_i->rendering_godrays_clock.restart().asSeconds(); }

float EngineTime::rendering_geometryTime(){ return m_i->rendering_geometry_Time; }
float EngineTime::rendering_lightingTime(){ return m_i->rendering_lighting_Time; }
float EngineTime::rendering_ssaoTime(){ return m_i->rendering_ssao_Time; }
float EngineTime::rendering_aaTime(){ return m_i->rendering_aa_Time; }
float EngineTime::rendering_godraysTime(){ return m_i->rendering_godrays_Time; }



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
std::string& EngineTime::reportTimeRendering(){ return EngineTime::reportTimeRendering(m_i->decimals); }
std::string& EngineTime::reportTimeRendering(uint decimals){
	m_i->decimals = decimals;
    m_i->prevOutput = m_i->currOutput;
    if((m_i->output_frame >= m_i->output_frame_delay-1) || m_i->output_frame_delay == 0){
		uint fps = uint(1.0f/m_i->deltaTime);
		std::stringstream stream1;std::stringstream stream2;std::stringstream stream3;std::stringstream stream4;std::stringstream stream5;std::stringstream stream6;
		stream1 << std::fixed << std::setprecision(decimals) << m_i->rendering_geometry_Time;
		stream2 << std::fixed << std::setprecision(decimals) << m_i->rendering_lighting_Time;
		stream3 << std::fixed << std::setprecision(decimals) << m_i->rendering_godrays_Time;
		stream4 << std::fixed << std::setprecision(decimals) << m_i->rendering_ssao_Time;
		stream5 << std::fixed << std::setprecision(decimals) << m_i->rendering_aa_Time;
		stream6 << std::fixed << std::setprecision(decimals) << m_i->renderTime;
		std::string s1 = stream1.str(); std::string s2 = stream2.str(); std::string s3 = stream3.str(); std::string s4 = stream4.str(); std::string s5 = stream5.str();std::string s6 = stream6.str();

        m_i->currOutput =   "Geometry Time: " + s1 +
                          "\nLighting Time: " + s2 +
                          "\nGodRays Time:  " + s3 +
                          "\nSSAO Time:     " + s4 +
                          "\nAA Time:       " + s5 +
                          "\nRender Time:   " + s6 +
                          "\nFPS: " + to_string(fps);
        m_i->prevOutput = m_i->currOutput;
        return m_i->currOutput;
    }
    return m_i->prevOutput;
}