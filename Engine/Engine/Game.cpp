#include "SolarSystem.h"
#include "Engine.h"
#include "HUD.h"
#include "Ship.h"
#include "Planet.h"
#include "Lagrange.h"

using namespace Engine; 
using namespace Engine::Events;

HUD* m_HUD;
std::unordered_map<std::string, SolarSystem*> m_SolarSystems;

void Game::cleanup(){
	for(auto solarSystem:m_SolarSystems)
		delete solarSystem.second;
	delete m_HUD;
}
void Game::initResources(){
	Resources::addShader("AS_SkyFromSpace","Shaders/AS_skyFromSpace_vert.glsl","Shaders/AS_skyFromSpace_frag.glsl");
	Resources::addShader("AS_SkyFromAtmosphere","Shaders/AS_skyFromAtmosphere_vert.glsl","Shaders/AS_skyFromAtmosphere_frag.glsl");
	Resources::addShader("AS_GroundFromSpace","Shaders/AS_groundFromSpace_vert.glsl","Shaders/AS_groundFromSpace_frag.glsl");

	Resources::addMesh("Skybox","Models/skybox.obj");
	Resources::addMesh("DEBUGLight","Models/debugLight.obj");
	Resources::addMesh("Planet","Models/planet.obj");
	Resources::addMesh("Defiant","Models/defiant.obj");
	Resources::addMesh("Akira","Models/akira.obj");
	Resources::addMesh("Starbase","Models/starbase.obj");
	Resources::addMesh("Ring","Models/ring.obj");
	Resources::addMesh("Dreadnought","Models/Dreadnought.obj");
	Lagrange::_genBuffers();

	Resources::addMaterial("Star","Textures/Planets/Sun.jpg");
	Resources::addMaterial("Default","Textures/Planets/Sun.jpg");
	Resources::addMaterial("Earth","Textures/Planets/Earth.jpg","","Textures/Planets/EarthNight.jpg");
	Resources::addMaterial("Defiant","Textures/defiant.png","Textures/defiantNormal.png","Textures/defiantGlow.png");
	Resources::addMaterial("Akira","Textures/akira.png","Textures/akiraNormal.png","Textures/akiraGlow.png");
	Resources::addMaterial("Crosshair","Textures/HUD/Crosshair.png");
	Resources::addMaterial("CrosshairArrow","Textures/HUD/CrosshairArrow.png");
	Resources::addMaterial("SunFlare","Textures/Skyboxes/StarFlare.png");
	Resources::addMaterial("Smoke","Textures/Effects/Smoke.png");
	Resources::addMaterial("Dreadnought","Textures/dreadnought.png","Textures/dreadnoughtNormal.png","Textures/dreadnoughtGlow.png");

	Resources::addParticleInfo("Smoke","Smoke");
}
void Game::initLogic(){
	m_SolarSystems["Sol"] = new SolarSystem("Sol","Systems/Sol.txt");
	//m_SolarSystems["Sol"] = new SolarSystem("Sol","");
	Resources::setCurrentScene(m_SolarSystems["Sol"]);

	m_HUD = new HUD(m_SolarSystems["Sol"]->getPlayer());

	Engine::Renderer::Settings::enableSSAO(false); //i dont feel ssao is needed here
}
void Game::update(float dt){
	if(Keyboard::isKeyDown("esc"))
		Engine::stop();

	m_HUD->update(dt);
}
void Game::render(){
	m_HUD->render(Engine::Renderer::RendererInfo::debug);
}