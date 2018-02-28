#include "Engine.h"
#include "Engine_Events.h"
#include "Engine_Renderer.h"
#include "Engine_Sounds.h"
#include "Engine_Resources.h"
#include "Engine_Window.h"
#include "Engine_Noise.h"
#include "Engine_Networking.h"
#include "Engine_ThreadManager.h"
#include "SolarSystem.h"
#include "HUD.h"
#include "Ship.h"
#include "Planet.h"
#include "Lagrange.h"
#include "GameCamera.h"
#include "CapsuleSpace.h"
#include "Material.h"
#include "Texture.h"
#include "Light.h"

#include <unordered_map>
#include <iostream>
#include <glm/vec2.hpp>


using namespace Engine;

HUD* m_HUD;
Handle pbcSound;
Handle sniperSound;
void Game::cleanup(){
    delete m_HUD;
}

void jobAddMesh(std::string name, std::string file,CollisionType type, bool fromFile,float threshold){
	Resources::addMesh(name,file,type,fromFile,threshold);
}

void Game::initResources(){
	Engine_Window& window = *Resources::getWindow();
    window.setIcon("data/Textures/icon.png");

	Handle skyFromSpaceVert = Resources::addShader("AS_SkyFromSpace_Vert","data/Shaders/AS_skyFromSpace_vert.glsl",ShaderType::Vertex);
	Handle skyFromSpaceFrag = Resources::addShader("AS_SkyFromSpace_Frag","data/Shaders/AS_skyFromSpace_frag.glsl",ShaderType::Fragment);
    Resources::addShaderProgram("AS_SkyFromSpace",skyFromSpaceVert,skyFromSpaceFrag,ShaderRenderPass::Geometry);

	Handle skyFromAtVert = Resources::addShader("AS_SkyFromAtmosphere_Vert","data/Shaders/AS_skyFromAtmosphere_vert.glsl",ShaderType::Vertex);
	Handle skyFromAtFrag = Resources::addShader("AS_SkyFromAtmosphere_Frag","data/Shaders/AS_skyFromAtmosphere_frag.glsl",ShaderType::Fragment);
    Resources::addShaderProgram("AS_SkyFromAtmosphere",skyFromAtVert,skyFromAtFrag,ShaderRenderPass::Geometry);

	Handle groundFromSpaceVert = Resources::addShader("AS_GroundFromSpace_Vert","data/Shaders/AS_groundFromSpace_vert.glsl",ShaderType::Vertex);
	Handle groundFromSpaceFrag = Resources::addShader("AS_GroundFromSpace_Frag","data/Shaders/AS_groundFromSpace_frag.glsl",ShaderType::Fragment);
    Resources::addShaderProgram("AS_GroundFromSpace",groundFromSpaceVert,groundFromSpaceFrag,ShaderRenderPass::Geometry);

    Resources::addMesh("Test","data/Models/1911.fbx",CollisionType::None,true,0.0f);

    Resources::addMesh("Planet","data/Models/planet.obj");
    Resources::addMesh("Defiant","data/Models/defiant.obj",CollisionType::ConvexHull);
    Resources::addMesh("Akira","data/Models/akira.obj",CollisionType::ConvexHull);
    Resources::addMesh("Miranda","data/Models/miranda.obj",CollisionType::ConvexHull);
    Resources::addMesh("Intrepid","data/Models/intrepid.obj",CollisionType::ConvexHull);
    Resources::addMesh("Norway","data/Models/norway.obj",CollisionType::ConvexHull);
    Resources::addMesh("Starbase","data/Models/starbase.obj",CollisionType::TriangleShapeStatic);
    Resources::addMesh("Ring","data/Models/ring.obj");
    Resources::addMesh("Dreadnaught","data/Models/dreadnaught.obj",CollisionType::ConvexHull);
	Resources::addMesh("Venerex","data/Models/venerex.obj",CollisionType::ConvexHull);

    Resources::addMaterial("Starbase","data/Textures/starbase.png","data/Textures/starbase_Normal.png","data/Textures/starbase_Glow.png");
    Resources::addMaterial("Star","data/Textures/Planets/Sun.jpg","","","","");
    Resources::getMaterial("Star")->setShadeless(true);
    Resources::getMaterial("Star")->setGlow(0.21f);
    Resources::addMaterial("Earth","data/Textures/Planets/Earth.jpg","","data/Textures/Planets/EarthNight.jpg","","AS_GroundFromSpace");
    Resources::addMaterial("Dreadnaught","data/Textures/dreadnaught.png","data/Textures/dreadnaught_Normal.png","data/Textures/dreadnaught_Glow.png");
    Resources::addMaterial("Defiant","data/Textures/defiant.png","data/Textures/defiant_Normal.png","data/Textures/defiant_Glow.png");

    Resources::addMaterial("Akira","data/Textures/akira.png","data/Textures/akira_Normal.png","data/Textures/akira_Glow.png");
    Resources::addMaterial("Miranda","data/Textures/miranda.png","data/Textures/miranda_Normal.png","data/Textures/miranda_Glow.png");
    Resources::addMaterial("Intrepid","data/Textures/intrepid.png","data/Textures/intrepid_Normal.png","data/Textures/intrepid_Glow.png");
    Resources::addMaterial("Norway","data/Textures/norway.png","data/Textures/norway_Normal.png","data/Textures/norway_Glow.png");
    Resources::addMaterial("Venerex","data/Textures/venerex.png","data/Textures/venerex_Normal.png","data/Textures/venerex_Glow.png");
	Resources::addMaterial("Crosshair","data/Textures/HUD/Crosshair.png","","","");
    Resources::addMaterial("CrosshairArrow","data/Textures/HUD/CrosshairArrow.png","","","");
    Resources::addMaterial("SunFlare","data/Textures/Skyboxes/StarFlare.png");
    Resources::getMaterial("SunFlare")->setShadeless(true);

    Resources::addMaterial("Gold","");
    Resources::getMaterial("Gold")->setMaterialPhysics(MaterialPhysics::Gold);

	pbcSound = Resources::addSoundData("data/Sounds/Effects/pbc.ogg","pbc");
	sniperSound = Resources::addSoundData("data/Sounds/Effects/snipercannon.ogg","sniper");
}
void Game::initLogic(){
	Engine_Window& window = *Resources::getWindow();
    window.keepMouseInWindow(true);
    window.setMouseCursorVisible(false);
    window.setKeyRepeatEnabled(false);
	//apparently these 2 should not be used together, but i have not found any issues with it so far
	//window.setVerticalSyncEnabled(true);
	//window.setFramerateLimit(60);

    SolarSystem* sol = new SolarSystem("Sol","data/Systems/Sol.txt");
    CapsuleSpace* cap = new CapsuleSpace();

	Resources::setCurrentScene("Sol");
    m_HUD = new HUD();
}

void Game::update(float dt){
	if(Engine::isKeyDown(KeyboardKey::Escape)){
        Engine::stop();
    }
	if(Engine::isKeyDownOnce(KeyboardKey::F4)){
        Resources::setCurrentScene("Sol");
    }
	if(Engine::isKeyDownOnce(KeyboardKey::F5)){
        Resources::setCurrentScene("CapsuleSpace");
    }
    if(Engine::isKeyDownOnce("f7")){ Renderer::Settings::setAntiAliasingAlgorithm(AntiAliasingAlgorithm::None); }
    if(Engine::isKeyDownOnce("f8")){ Renderer::Settings::setAntiAliasingAlgorithm(AntiAliasingAlgorithm::SMAA); }
    if(Engine::isKeyDownOnce("f9")){ Renderer::Settings::setAntiAliasingAlgorithm(AntiAliasingAlgorithm::FXAA); }
	if(Engine::isKeyDownOnce("f10")){ Renderer::Settings::SSAO::enable(!Renderer::Settings::SSAO::enabled()); }

	if(Engine::isKeyDownOnce("space")){

	}
	/*
    if(Engine::isKeyDown("z")){
        Renderer::Settings::HDR::setExposure(Renderer::Settings::HDR::getExposure() - 0.03f);
    }
    else if(Engine::isKeyDown("x")){
        Renderer::Settings::HDR::setExposure(Renderer::Settings::HDR::getExposure() + 0.03f);
    }
    if(Engine::isKeyDown("c")){
        Renderer::Settings::setGamma(Renderer::Settings::getGamma() - 0.02f);
    }
    else if(Engine::isKeyDown("k")){
        Renderer::Settings::setGamma(Renderer::Settings::getGamma() + 0.02f);
    }
	*/
	if(Engine::isKeyDown(KeyboardKey::N)){
        Resources::getMaterial("Defiant")->setMetalness(Resources::getMaterial("Defiant")->metalness() - 0.02f);
        Resources::getMaterial("Intrepid")->setMetalness(Resources::getMaterial("Intrepid")->metalness() - 0.02f);
		Resources::getMaterial("Venerex")->setMetalness(Resources::getMaterial("Venerex")->metalness() - 0.02f);
		Resources::getMaterial("Miranda")->setMetalness(Resources::getMaterial("Miranda")->metalness() - 0.02f);
		Resources::getMaterial("Dreadnaught")->setMetalness(Resources::getMaterial("Dreadnaught")->metalness() - 0.02f);
    }
	else if(Engine::isKeyDown(KeyboardKey::M)){
        Resources::getMaterial("Defiant")->setMetalness(Resources::getMaterial("Defiant")->metalness() + 0.02f);
        Resources::getMaterial("Intrepid")->setMetalness(Resources::getMaterial("Intrepid")->metalness() + 0.02f);
		Resources::getMaterial("Venerex")->setMetalness(Resources::getMaterial("Venerex")->metalness() + 0.02f);
		Resources::getMaterial("Miranda")->setMetalness(Resources::getMaterial("Miranda")->metalness() + 0.02f);
		Resources::getMaterial("Dreadnaught")->setMetalness(Resources::getMaterial("Dreadnaught")->metalness() + 0.02f);
    }
	if(Engine::isKeyDown(KeyboardKey::V)){
        Resources::getMaterial("Defiant")->setSmoothness(Resources::getMaterial("Defiant")->smoothness() - 0.02f);
        Resources::getMaterial("Intrepid")->setSmoothness(Resources::getMaterial("Intrepid")->smoothness() - 0.02f);
		Resources::getMaterial("Venerex")->setSmoothness(Resources::getMaterial("Venerex")->smoothness() - 0.02f);
		Resources::getMaterial("Miranda")->setSmoothness(Resources::getMaterial("Miranda")->smoothness() - 0.02f);
		Resources::getMaterial("Dreadnaught")->setSmoothness(Resources::getMaterial("Dreadnaught")->smoothness() - 0.02f);
    }
	else if(Engine::isKeyDown(KeyboardKey::B)){
        Resources::getMaterial("Defiant")->setSmoothness(Resources::getMaterial("Defiant")->smoothness() + 0.02f);
        Resources::getMaterial("Intrepid")->setSmoothness(Resources::getMaterial("Intrepid")->smoothness() + 0.02f);
		Resources::getMaterial("Venerex")->setSmoothness(Resources::getMaterial("Venerex")->smoothness() + 0.02f);
		Resources::getMaterial("Miranda")->setSmoothness(Resources::getMaterial("Miranda")->smoothness() + 0.02f);
		Resources::getMaterial("Dreadnaught")->setSmoothness(Resources::getMaterial("Dreadnaught")->smoothness() + 0.02f);
    }
    m_HUD->update(dt);
}
void Game::render(){
    m_HUD->render();
}

#pragma region EventHandlers
void Game::onResize(uint width,uint height){
}
void Game::onClose(){
}
void Game::onLostFocus(){
    Engine::getWindow()->keepMouseInWindow(false);
    //Engine::getWindow()->setMouseCursorVisible(true);
}
void Game::onGainedFocus(){
    Engine::getWindow()->keepMouseInWindow(true);
    //Engine::getWindow()->setMouseCursorVisible(false);
    const glm::vec2 halfRes = glm::vec2(Resources::getWindowSize().x/2,Resources::getWindowSize().y/2);
    sf::Mouse::setPosition(sf::Vector2i(int(halfRes.x),int(halfRes.y)),*Resources::getWindow()->getSFMLHandle());

	Engine::setMousePosition(halfRes,true);
}
void Game::onTextEntered(uint unicode){
}
void Game::onKeyPressed(uint key){
}
void Game::onKeyReleased(uint key){
}
void Game::onMouseWheelMoved(int delta){
}
void Game::onMouseButtonPressed(uint button){
}
void Game::onMouseButtonReleased(uint button){
}
void Game::onMouseMoved(float mouseX,float mouseY){
}
void Game::onMouseEntered(){
    Engine::getWindow()->requestFocus();
    Engine::getWindow()->keepMouseInWindow(true);
    //Engine::getWindow()->setMouseCursorVisible(true);
}
void Game::onMouseLeft(){
    Engine::getWindow()->keepMouseInWindow(false);
    //Engine::getWindow()->setMouseCursorVisible(true);
}
void Game::onPreUpdate(float dt){
}
void Game::onPostUpdate(float dt){
    const glm::vec2 halfRes = glm::vec2(Resources::getWindowSize().x/2,Resources::getWindowSize().y/2);
    if(Resources::getWindow()->hasFocus()){
        glm::vec2 mousePos = Engine::getMousePosition();
        float mouseDistFromCenter = glm::distance(mousePos,halfRes);
        if(mouseDistFromCenter > 1.0f){
            sf::Mouse::setPosition(sf::Vector2i(int(halfRes.x),int(halfRes.y)),*Resources::getWindow()->getSFMLHandle());
			Engine::setMousePosition(halfRes,false,true);
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
