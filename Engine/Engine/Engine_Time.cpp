#include "Engine_Time.h"
#include "Engine_Resources.h"
#include <SFML/Window.hpp>
#include <iomanip>
#include <sstream>

using namespace std;

class Engine::impl::TimeManager::impl{
    public:
        sf::Clock update_clock;  sf::Clock physics_clock;  sf::Clock render_clock;   sf::Clock sounds_clock;
        double updateTime;        double physicsTime;        double renderTime;      double soundTime;

		//rendering data
        sf::Clock rendering_geometry_clock; double rendering_geometry_Time;
		sf::Clock rendering_lighting_clock; double rendering_lighting_Time;
		sf::Clock rendering_ssao_clock;     double rendering_ssao_Time;
		sf::Clock rendering_godrays_clock;  double rendering_godrays_Time;
		sf::Clock rendering_aa_clock;       double rendering_aa_Time;
		sf::Clock rendering_display_clock;  double rendering_display_Time;

        float applicationTime;
        float deltaTime;
        uint output_frame_delay;
        uint output_frame;
		uint decimals;
        string currOutput;
        string prevOutput;

        void _init(){
            update_clock = sf::Clock();  physics_clock = sf::Clock();  render_clock = sf::Clock();  sounds_clock = sf::Clock();
            updateTime = 0;              physicsTime = 0;              renderTime = 0;              soundTime = 0;


			rendering_geometry_clock = sf::Clock();  rendering_geometry_Time = 0;
			rendering_lighting_clock = sf::Clock();  rendering_lighting_Time = 0;
			rendering_ssao_clock = sf::Clock();      rendering_ssao_Time = 0;
			rendering_godrays_clock = sf::Clock();   rendering_godrays_Time = 0;
			rendering_aa_clock = sf::Clock();        rendering_aa_Time = 0;
			rendering_display_clock = sf::Clock();   rendering_display_Time = 0;


            applicationTime = 0;
            deltaTime = 1.0f;

            output_frame_delay = 4;
            output_frame = 0;
			decimals = 6;
            string currOutput = "";
            string prevOutput = "";
        }
};
Engine::impl::TimeManager::TimeManager():m_i(new impl){m_i->_init();}
Engine::impl::TimeManager::~TimeManager(){}
void Engine::impl::TimeManager::calculate(){
    m_i->deltaTime = float(m_i->updateTime + m_i->physicsTime + m_i->renderTime);
    m_i->applicationTime += m_i->deltaTime;

    m_i->output_frame++;
    if(m_i->output_frame >= m_i->output_frame_delay){
        m_i->output_frame = 0;
    }
}
void Engine::impl::TimeManager::stop_update(){ m_i->update_clock.restart(); }
void Engine::impl::TimeManager::stop_physics(){ m_i->physics_clock.restart(); }
void Engine::impl::TimeManager::stop_sounds(){ m_i->sounds_clock.restart(); }
void Engine::impl::TimeManager::stop_render(){ m_i->render_clock.restart(); }
void Engine::impl::TimeManager::calculate_update(){ m_i->updateTime = m_i->update_clock.restart().asSeconds(); }
void Engine::impl::TimeManager::calculate_physics(){ m_i->physicsTime = m_i->physics_clock.restart().asSeconds(); }
void Engine::impl::TimeManager::calculate_sounds(){ m_i->soundTime = m_i->sounds_clock.restart().asSeconds(); }
void Engine::impl::TimeManager::calculate_render(){ m_i->renderTime = m_i->render_clock.restart().asSeconds(); }
float Engine::impl::TimeManager::dt(){ return m_i->deltaTime; }
float Engine::impl::TimeManager::applicationTime(){ return m_i->applicationTime; }
float Engine::impl::TimeManager::updateTime(){ return m_i->updateTime; }
float Engine::impl::TimeManager::physicsTime(){ return m_i->physicsTime; }
float Engine::impl::TimeManager::renderTime(){ return m_i->renderTime; }
float Engine::impl::TimeManager::soundsTime(){ return m_i->soundTime; }



void Engine::impl::TimeManager::stop_rendering_geometry(){ m_i->rendering_geometry_clock.restart(); }
void Engine::impl::TimeManager::stop_rendering_lighting(){ m_i->rendering_lighting_clock.restart(); }
void Engine::impl::TimeManager::stop_rendering_ssao(){ m_i->rendering_ssao_clock.restart(); }
void Engine::impl::TimeManager::stop_rendering_aa(){ m_i->rendering_aa_clock.restart(); }
void Engine::impl::TimeManager::stop_rendering_godrays(){ m_i->rendering_godrays_clock.restart(); }
void Engine::impl::TimeManager::stop_rendering_display(){ m_i->rendering_display_clock.restart(); }

void Engine::impl::TimeManager::calculate_rendering_geometry(){ m_i->rendering_geometry_Time = m_i->rendering_geometry_clock.restart().asSeconds(); }
void Engine::impl::TimeManager::calculate_rendering_lighting(){ m_i->rendering_lighting_Time = m_i->rendering_lighting_clock.restart().asSeconds(); }
void Engine::impl::TimeManager::calculate_rendering_ssao(){ m_i->rendering_ssao_Time = m_i->rendering_ssao_clock.restart().asSeconds(); }
void Engine::impl::TimeManager::calculate_rendering_aa(){ m_i->rendering_aa_Time = m_i->rendering_aa_clock.restart().asSeconds(); }
void Engine::impl::TimeManager::calculate_rendering_godrays(){ m_i->rendering_godrays_Time = m_i->rendering_godrays_clock.restart().asSeconds(); }
void Engine::impl::TimeManager::calculate_rendering_display(){ m_i->rendering_display_Time = m_i->rendering_display_clock.restart().asSeconds(); }

float Engine::impl::TimeManager::rendering_geometryTime(){ return m_i->rendering_geometry_Time; }
float Engine::impl::TimeManager::rendering_lightingTime(){ return m_i->rendering_lighting_Time; }
float Engine::impl::TimeManager::rendering_ssaoTime(){ return m_i->rendering_ssao_Time; }
float Engine::impl::TimeManager::rendering_aaTime(){ return m_i->rendering_aa_Time; }
float Engine::impl::TimeManager::rendering_godraysTime(){ return m_i->rendering_godrays_Time; }
float Engine::impl::TimeManager::rendering_displayTime(){ return m_i->rendering_display_Time; }


std::string& Engine::impl::TimeManager::reportTime(){ return Engine::impl::TimeManager::reportTime(m_i->decimals); }
std::string& Engine::impl::TimeManager::reportTime(uint decimals){
	m_i->decimals = decimals;
    m_i->prevOutput = m_i->currOutput;
    if((m_i->output_frame >= m_i->output_frame_delay-1) || m_i->output_frame_delay == 0){
		uint fps = uint(1.0f/m_i->deltaTime);
		stringstream st1, st2, st3, st4, st5;
		st1 << std::fixed << std::setprecision(decimals) << m_i->updateTime;
		st2 << std::fixed << std::setprecision(decimals) << m_i->physicsTime;
		st5 << std::fixed << std::setprecision(decimals) << m_i->soundTime;
		st3 << std::fixed << std::setprecision(decimals) << m_i->renderTime;
		st4 << std::fixed << std::setprecision(decimals) << m_i->deltaTime;
		string s1=st1.str(); string s2=st2.str(); string s3=st3.str(); string s4=st4.str(); string s5=st5.str();

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
std::string& Engine::impl::TimeManager::reportTimeRendering(){ return Engine::impl::TimeManager::reportTimeRendering(m_i->decimals); }
std::string& Engine::impl::TimeManager::reportTimeRendering(uint decimals){
	m_i->decimals = decimals;
    m_i->prevOutput = m_i->currOutput;
    if((m_i->output_frame >= m_i->output_frame_delay-1) || m_i->output_frame_delay == 0){
		uint fps = uint(1.0f/m_i->deltaTime);
		stringstream st1, st2, st3, st4, st5, st6, st7;
		st1 << std::fixed << std::setprecision(decimals) << m_i->rendering_geometry_Time;
		st2 << std::fixed << std::setprecision(decimals) << m_i->rendering_lighting_Time;
		st3 << std::fixed << std::setprecision(decimals) << m_i->rendering_godrays_Time;
		st4 << std::fixed << std::setprecision(decimals) << m_i->rendering_ssao_Time;
		st5 << std::fixed << std::setprecision(decimals) << m_i->rendering_aa_Time;
		st6 << std::fixed << std::setprecision(decimals) << m_i->renderTime;
		st7 << std::fixed << std::setprecision(decimals) << m_i->rendering_display_Time;
		string s1=st1.str(); string s2=st2.str(); string s3=st3.str(); string s4=st4.str(); string s5=st5.str(); string s6=st6.str(); string s7=st7.str();

        m_i->currOutput =   "Geometry Time: " + s1 +
                          "\nLighting Time: " + s2 +
                          "\nGodRays Time:  " + s3 +
                          "\nSSAO Time:     " + s4 +
                          "\nAA Time:       " + s5 +
                          "\nDisplay Time:  " + s7 +
                          "\nRender Time:   " + s6 +
                          "\nFPS: " + to_string(fps);
        m_i->prevOutput = m_i->currOutput;
        return m_i->currOutput;
    }
    return m_i->prevOutput;
}