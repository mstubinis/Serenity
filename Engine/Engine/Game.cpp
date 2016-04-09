#include "Engine.h"
#include "Engine_Events.h"
#include "Engine_Renderer.h"
#include "Engine_Sounds.h"
#include "Engine_Resources.h"
#include "Engine_Window.h"
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
	Resources::getWindow()->setIcon("data/Textures/icon.png");

    Resources::addSoundAsMusic("Test","data/Sounds/Music/test.ogg");

    Resources::addShader("AS_SkyFromSpace","data/Shaders/AS_skyFromSpace_vert.glsl","data/Shaders/AS_skyFromSpace_frag.glsl");
    Resources::addShader("AS_SkyFromAtmosphere","data/Shaders/AS_skyFromAtmosphere_vert.glsl","data/Shaders/AS_skyFromAtmosphere_frag.glsl");
    Resources::addShader("AS_GroundFromSpace","data/Shaders/AS_groundFromSpace_vert.glsl","data/Shaders/AS_groundFromSpace_frag.glsl");

    Resources::addMesh("Planet","data/Models/planet.obj");
    Resources::addMesh("Defiant","data/Models/defiant.obj");
    Resources::addMesh("Akira","data/Models/akira.obj");
    Resources::addMesh("Starbase","data/Models/starbase.obj");
    Resources::addMesh("Ring","data/Models/ring.obj");
    Resources::addMesh("Dreadnaught","data/Models/dreadnaught.obj");

    Resources::addMaterial("Star","data/Textures/Planets/Sun.jpg");
    Resources::addMaterial("Earth","data/Textures/Planets/Earth.jpg","","data/Textures/Planets/EarthNight.jpg");
    Resources::addMaterial("Defiant","data/Textures/defiant.png","data/Textures/defiant_Normal.png","data/Textures/defiant_Glow.png");
    Resources::addMaterial("Akira","data/Textures/akira.png","data/Textures/akira_Normal.png","data/Textures/akira_Glow.png");
    Resources::addMaterial("Crosshair","data/Textures/HUD/Crosshair.png");
    Resources::addMaterial("CrosshairArrow","data/Textures/HUD/CrosshairArrow.png");
    Resources::addMaterial("SunFlare","data/Textures/Skyboxes/StarFlare.png");
    Resources::addMaterial("Smoke","data/Textures/Effects/Smoke.png");
    Resources::addMaterial("Dreadnaught","data/Textures/dreadnaught.png","data/Textures/dreadnaught_Normal.png","data/Textures/dreadnaught_Glow.png");

    Resources::addParticleInfo("Smoke","Smoke");
}
void Game::initLogic(){
	Engine::getWindow()->setMouseCursorVisible(false);
    Engine::getWindow()->setKeyRepeatEnabled(false);
    Engine::Renderer::Settings::enableSSAO(true);

    new SolarSystem("Sol","data/Systems/Sol.txt");
    //new SolarSystem("Sol","");
    new CapsuleSpace();
    
    Resources::setCurrentScene("Sol");
    Resources::setActiveCamera(static_cast<SolarSystem*>(Resources::getCurrentScene())->getPlayerCamera());

    Sound::setCurrentMusicAndPlay("Test");
    Sound::loop("Test");

    m_HUD = new HUD();
}
void Game::update(float dt){
    SolarSystem* s = static_cast<SolarSystem*>(Resources::getScene("Sol"));
    if(Events::Keyboard::isKeyDown("esc")){
        Engine::stop();
	}
    if(Events::Keyboard::isKeyDownOnce("f6")){
        Resources::setCurrentScene("CapsuleSpace");
        Resources::setActiveCamera(static_cast<SolarSystem*>(Resources::getCurrentScene())->getPlayerCamera());
    }
    if(Events::Keyboard::isKeyDownOnce("f7")){
        Resources::setCurrentScene("Sol");
        Resources::setActiveCamera(static_cast<SolarSystem*>(Resources::getCurrentScene())->getPlayerCamera());
    }
    if(Events::Keyboard::isKeyDownOnce("f8")){
        Renderer::Settings::enableSSAO(!Renderer::RendererInfo::ssao);
    }
    if(Events::Keyboard::isKeyDownOnce("f9")){
		Engine::setFullScreen(true);
    }
    if(Events::Keyboard::isKeyDownOnce("f10")){
        Engine::setFullScreen(false);
    }
    if(Events::Keyboard::isKeyDownOnce("f11")){
		Engine::getWindow()->setRenderingAPI(ENGINE_RENDERING_API_DIRECTX);
    }
    if(Events::Keyboard::isKeyDownOnce("f12")){
        Engine::getWindow()->setRenderingAPI(ENGINE_RENDERING_API_OPENGL);
    }
    m_HUD->update(dt);
}
void Game::render(){
    m_HUD->render(Engine::Renderer::RendererInfo::debug);
}