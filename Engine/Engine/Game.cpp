#include "Game.h"
#include "SolarSystem.h"
#include "Engine.h"
#include "Engine_Events.h"
#include "Engine_Resources.h"

using namespace Engine;
using namespace Engine::Events;

Game::Game(){}
Game::~Game(){
	for(auto solarSystem:m_SolarSystems)
		delete solarSystem.second;
}
void Game::Init_Logic(){
	m_SolarSystems["Sol"] = new SolarSystem("Sol","");
	Resources::setCurrentScene(m_SolarSystems["Sol"]);
}
void Game::Init_Resources()
{
}
void Game::Update(float dt){
	Resources::getCurrentScene()->Update(dt);
}