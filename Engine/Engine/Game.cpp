#include "Engine.h"
#include "SolarSystem.h"
#include "HUD.h"
#include "Ship.h"
#include "Planet.h"
#include "Lagrange.h"
#include "Station.h"
#include "GameCamera.h"
#include "CapsuleSpace.h"

using namespace Engine;

HUD* m_HUD;

void Game::cleanup(){
	for(auto part:stationPartsInfo)
		delete part.second;
	delete m_HUD;
}
void Game::initResources(){
	Resources::addShader("AS_SkyFromSpace","Shaders/AS_skyFromSpace_vert.glsl","Shaders/AS_skyFromSpace_frag.glsl");
	Resources::addShader("AS_SkyFromAtmosphere","Shaders/AS_skyFromAtmosphere_vert.glsl","Shaders/AS_skyFromAtmosphere_frag.glsl");
	Resources::addShader("AS_GroundFromSpace","Shaders/AS_groundFromSpace_vert.glsl","Shaders/AS_groundFromSpace_frag.glsl");

	Resources::addMesh("Skybox","Models/skybox.obj");
	Resources::addMesh("Planet","Models/planet.obj");
	Resources::addMesh("Defiant","Models/defiant.obj");
	Resources::addMesh("Akira","Models/akira.obj");
	Resources::addMesh("Starbase","Models/starbase.obj");
	Resources::addMesh("Ring","Models/ring.obj");
	Resources::addMesh("Dreadnaught","Models/dreadnaught.obj");
	Lagrange::_genBuffers();

	Resources::addMaterial("Star","Textures/Planets/Sun.jpg");
	Resources::addMaterial("Earth","Textures/Planets/Earth.jpg","","Textures/Planets/EarthNight.jpg");
	Resources::addMaterial("Defiant","Textures/defiant.png","Textures/defiant_Normal.png","Textures/defiant_Glow.png");
	Resources::addMaterial("Akira","Textures/akira.png","Textures/akira_Normal.png","Textures/akira_Glow.png");
	Resources::addMaterial("Crosshair","Textures/HUD/Crosshair.png");
	Resources::addMaterial("CrosshairArrow","Textures/HUD/CrosshairArrow.png");
	Resources::addMaterial("SunFlare","Textures/Skyboxes/StarFlare.png");
	Resources::addMaterial("Smoke","Textures/Effects/Smoke.png");
	Resources::addMaterial("Dreadnaught","Textures/dreadnaught.png","Textures/dreadnaught_Normal.png","Textures/dreadnaught_Glow.png");

	Resources::addParticleInfo("Smoke","Smoke");
}
void Game::initLogic(){
	Engine::Renderer::Settings::enableSSAO(false); //i dont feel ssao is needed here

	new SolarSystem("Sol","Systems/Sol.txt");
	new CapsuleSpace();
	//new SolarSystem("Sol","");
	Resources::setCurrentScene("Sol");
	Resources::setActiveCamera(static_cast<SolarSystem*>(Resources::getCurrentScene())->getPlayerCamera());
	////Resources::setCurrentScene("CapsuleSpace");

	m_HUD = new HUD();
}
void Game::update(float dt){

	SolarSystem* s = static_cast<SolarSystem*>(Resources::getScene("Sol"));

	if(Events::Keyboard::isKeyDown("esc"))
		Engine::stop();
	if(Events::Keyboard::isKeyDownOnce("f6")){
		Resources::setCurrentScene("CapsuleSpace");
		Resources::setActiveCamera(static_cast<SolarSystem*>(Resources::getCurrentScene())->getPlayerCamera());
	}
	if(Events::Keyboard::isKeyDownOnce("f7")){
		Resources::setCurrentScene("Sol");
		Resources::setActiveCamera(static_cast<SolarSystem*>(Resources::getCurrentScene())->getPlayerCamera());
	}

	m_HUD->update(dt);
}
void Game::render(){
	m_HUD->render(Engine::Renderer::RendererInfo::debug);
}