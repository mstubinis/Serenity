#include "Game.h"
#include "Engine.h"
#include "Engine_Events.h"

using namespace Engine;
using namespace Engine::Events;

Game::Game(){}
Game::~Game(){
	for(auto solarSystem:m_SolarSystems)
		delete solarSystem.second;
	delete m_CurrentSolarSystem;
}
void Game::Init_Logic(){
	m_SolarSystems["Sol"] = new SolarSystem("");
	m_CurrentSolarSystem = m_SolarSystems["Sol"];
}
void Game::Init_Resources()
{
}
void Game::Update(float dt){
	m_CurrentSolarSystem->Update(dt);
}