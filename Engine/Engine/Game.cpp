#include "Engine.h"
#include "Engine_Events.h"
#include "Engine_Renderer.h"
#include "Engine_Sounds.h"
#include "Engine_Resources.h"
#include "Engine_Window.h"
#include "Engine_Noise.h"
#include "SolarSystem.h"
#include "HUD.h"
#include "Ship.h"
#include "Planet.h"
#include "Lagrange.h"
#include "Station.h"
#include "GameCamera.h"
#include "CapsuleSpace.h"
#include "Material.h"
#include "Texture.h"

#include <unordered_map>

using namespace Engine;

HUD* m_HUD;

void Game::cleanup(){
    for(auto part:stationPartsInfo)
        delete part.second;
    delete m_HUD;
}
void Game::initResources(){
    Resources::getWindow()->setIcon("data/Textures/icon.png");

    Resources::addShaderProgram("AS_SkyFromSpace","data/Shaders/AS_skyFromSpace_vert.glsl","data/Shaders/AS_skyFromSpace_frag.glsl",SHADER_PIPELINE_STAGE_GEOMETRY);
    Resources::addShaderProgram("AS_SkyFromAtmosphere","data/Shaders/AS_skyFromAtmosphere_vert.glsl","data/Shaders/AS_skyFromAtmosphere_frag.glsl",SHADER_PIPELINE_STAGE_GEOMETRY);
    Resources::addShaderProgram("AS_GroundFromSpace","data/Shaders/AS_groundFromSpace_vert.glsl","data/Shaders/AS_groundFromSpace_frag.glsl",SHADER_PIPELINE_STAGE_GEOMETRY);

    Resources::addMesh("Planet","data/Models/planet.obj",COLLISION_TYPE_NONE);
    Resources::addMesh("Defiant","data/Models/defiant.obj");
    Resources::addMesh("Akira","data/Models/akira.obj");
	Resources::addMesh("Intrepid","data/Models/intrepid.obj");
	Resources::addMesh("Norway","data/Models/norway.obj");
	Resources::addMesh("Starbase","data/Models/starbase.obj",COLLISION_TYPE_STATIC_TRIANGLESHAPE);
    Resources::addMesh("Ring","data/Models/ring.obj",COLLISION_TYPE_NONE);
    Resources::addMesh("Dreadnaught","data/Models/dreadnaught.obj");

	Resources::addMaterial("Starbase","data/Textures/starbase.png","","data/Textures/starbase_Glow.png");
    Resources::addMaterial("Star","data/Textures/Planets/Sun.jpg");
	Resources::getMaterial("Star")->setShadeless(true);
    Resources::getMaterial("Star")->setGlow(0.21f);
    Resources::addMaterial("Earth","data/Textures/Planets/Earth.jpg","","data/Textures/Planets/EarthNight.jpg","","AS_GroundFromSpace");
	Resources::addMaterial("Dreadnaught","data/Textures/dreadnaught.png","data/Textures/dreadnaught_Normal.png","data/Textures/dreadnaught_Glow.png");
    Resources::addMaterial("Defiant","data/Textures/defiant.png","data/Textures/defiant_Normal.png","data/Textures/defiant_Glow.png");
    Resources::addMaterial("Akira","data/Textures/akira.png","data/Textures/akira_Normal.png","data/Textures/akira_Glow.png");
	Resources::addMaterial("Intrepid","data/Textures/intrepid.png","data/Textures/intrepid_Normal.png","data/Textures/intrepid_Glow.png");
	Resources::addMaterial("Norway","data/Textures/norway.png","data/Textures/norway_Normal.png","data/Textures/norway_Glow.png","data/Textures/norway_Specular.png");
    Resources::addMaterial("Crosshair","data/Textures/HUD/Crosshair.png","","","","Deferred_HUD");
    Resources::addMaterial("CrosshairArrow","data/Textures/HUD/CrosshairArrow.png","","","","Deferred_HUD");
    Resources::addMaterial("SunFlare","data/Textures/Skyboxes/StarFlare.png");
	Resources::getMaterial("SunFlare")->setShadeless(true);

	//custom cubemap
	/*
    std::string front = "data/Textures/Effects/Right.jpg";
    std::string back = "data/Textures/Effects/Left.jpg";
    std::string left = "data/Textures/Effects/Top.jpg";
    std::string right = "data/Textures/Effects/Bottom.jpg";
    std::string top = "data/Textures/Effects/Front.jpg";
    std::string bottom = "data/Textures/Effects/Back.jpg";
    std::string names[6] = {front,back,left,right,top,bottom};

	new Texture(names,"CubemapGold");

	//Resources::getMaterial("Defiant")->addComponentReflection("CubemapGold","data/Textures/defiant_Reflection.png");
	//Resources::getMaterial("Defiant")->addComponentRefraction("CubemapGold","data/Textures/defiant_Reflection.png",1.0f,1.53f);
	*/
}
void Game::initLogic(){
	Engine::getWindow()->keepMouseInWindow(true);
    Engine::getWindow()->setMouseCursorVisible(false);
    Engine::getWindow()->setKeyRepeatEnabled(false);

    new SolarSystem("Sol","data/Systems/Sol.txt");
    //new SolarSystem("Sol","");
    new CapsuleSpace();
    
    Resources::setCurrentScene("Sol");
    Resources::setActiveCamera(static_cast<SolarSystem*>(Resources::getCurrentScene())->getPlayerCamera());

    m_HUD = new HUD();
}
void Game::update(float dt){
    SolarSystem* s = static_cast<SolarSystem*>(Resources::getScene("Sol"));
    if(Events::Keyboard::isKeyDown("esc")){
        Engine::stop();
    }

    if(Events::Keyboard::isKeyDownOnce("f9")){
        Resources::setCurrentScene("CapsuleSpace");
        Resources::setActiveCamera(static_cast<SolarSystem*>(Resources::getCurrentScene())->getPlayerCamera());
    }
    if(Events::Keyboard::isKeyDownOnce("f10")){
        Resources::setCurrentScene("Sol");
        Resources::setActiveCamera(static_cast<SolarSystem*>(Resources::getCurrentScene())->getPlayerCamera());
    }
    if(Events::Keyboard::isKeyDownOnce("f11")){
		Renderer::Settings::SSAO::enable(!Renderer::Detail::RendererInfo::SSAOInfo::ssao);
    }
    m_HUD->update(dt);
}
void Game::render(){
    m_HUD->render(Renderer::Detail::RendererInfo::DebugDrawingInfo::debug);
}

#pragma region EventHandlers
void Game::onResize(uint width,uint height){
}
void Game::onClose(){
}
void Game::onLostFocus(){
	Engine::getWindow()->keepMouseInWindow(false);
}
void Game::onGainedFocus(){
	Engine::getWindow()->keepMouseInWindow(true);
	const glm::vec2 halfRes = glm::vec2(Resources::getWindowSize().x/2,Resources::getWindowSize().y/2);
	sf::Mouse::setPosition(sf::Vector2i(int(halfRes.x),int(halfRes.y)),*Resources::getWindow()->getSFMLHandle());
    Events::Mouse::MouseProcessing::m_Position = Events::Mouse::MouseProcessing::m_Position_Previous = halfRes;
	Events::Mouse::MouseProcessing::m_Difference = glm::vec2(0.0f);
}
void Game::onTextEntered(sf::Event::TextEvent textEvent){
}
void Game::onKeyPressed(uint key){
}
void Game::onKeyReleased(uint key){
}
void Game::onMouseWheelMoved(sf::Event::MouseWheelEvent mwEvent){
}
void Game::onMouseButtonPressed(sf::Event::MouseButtonEvent mbEvent){
}
void Game::onMouseButtonReleased(sf::Event::MouseButtonEvent mbEvent){
}
void Game::onMouseMoved(sf::Event::MouseMoveEvent mmEvent){
}
void Game::onMouseEntered(){
	Engine::getWindow()->requestFocus();
	Engine::getWindow()->keepMouseInWindow(true);
}
void Game::onMouseLeft(){
	Engine::getWindow()->keepMouseInWindow(false);
}
void Game::onPreUpdate(float dt){
}
void Game::onPostUpdate(float dt){
	const glm::vec2 halfRes = glm::vec2(Resources::getWindowSize().x/2,Resources::getWindowSize().y/2);
    if(Resources::getWindow()->hasFocus()){
        glm::vec2 mousePos = Events::Mouse::getMousePosition();
        float mouseDistFromCenter = glm::distance(mousePos,halfRes);
        if(mouseDistFromCenter > 1.0f){
			sf::Mouse::setPosition(sf::Vector2i(int(halfRes.x),int(halfRes.y)),*Resources::getWindow()->getSFMLHandle());
            Events::Mouse::MouseProcessing::m_Position = Events::Mouse::MouseProcessing::m_Position_Previous = halfRes;
        }
    }
}
void Game::onJoystickButtonPressed(){
}
void Game::onJoystickButtonReleased(){
}
void Game::onJoystickMoved(){
}
void Game::onJoystickConnected(){
}
void Game::onJoystickDisconnected(){
}
#pragma endregion