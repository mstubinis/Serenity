#include "Engine.h"
#include "Engine_Events.h"
#include "Engine_Renderer.h"
#include "Engine_Sounds.h"
#include "Engine_Resources.h"
#include "Engine_Window.h"
#include "Engine_Noise.h"
#include "Engine_Networking.h"
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
void Game::cleanup(){
    delete m_HUD;
}
void Game::initResources(){
    Resources::getWindow()->setIcon("data/Textures/icon.png");

    Resources::addShaderProgram("AS_GroundFromSpace","data/Shaders/AS_groundFromSpace_vert.glsl","data/Shaders/AS_groundFromSpace_frag.glsl",ShaderRenderPass::Geometry);
    Resources::addShaderProgram("AS_SkyFromSpace","data/Shaders/AS_skyFromSpace_vert.glsl","data/Shaders/AS_skyFromSpace_frag.glsl",ShaderRenderPass::None);
    Resources::addShaderProgram("AS_SkyFromAtmosphere","data/Shaders/AS_skyFromAtmosphere_vert.glsl","data/Shaders/AS_skyFromAtmosphere_frag.glsl",ShaderRenderPass::None);

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
    Resources::addMaterial("Norway","data/Textures/norway.png","data/Textures/norway_Normal.png","data/Textures/norway_Glow.png","data/Textures/norway_Specular.png");
    Resources::addMaterial("Crosshair","data/Textures/HUD/Crosshair.png","","","","Deferred_HUD");
    Resources::addMaterial("CrosshairArrow","data/Textures/HUD/CrosshairArrow.png","","","","Deferred_HUD");
    Resources::addMaterial("SunFlare","data/Textures/Skyboxes/StarFlare.png");
    Resources::getMaterial("SunFlare")->setShadeless(true);

    Resources::addMaterial("Iron","data/Textures/iron_diffuse.png","data/Textures/iron_normal.png");
    Resources::getMaterial("Iron")->addComponentMetalness("data/Textures/iron_metallic.png");
    Resources::getMaterial("Iron")->addComponentSmoothness("data/Textures/iron_smoothness.png");

    Resources::addMaterial("Gold","data/Textures/gold.png","data/Textures/gold_Normal.png");
    Resources::getMaterial("Gold")->setMaterialPhysics(MaterialPhysics::Gold);
}
void Game::initLogic(){
    Engine::getWindow()->keepMouseInWindow(true);
    Engine::getWindow()->setMouseCursorVisible(false);
    Engine::getWindow()->setKeyRepeatEnabled(false);

    //Resources::Settings::enableDynamicMemory();

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
    if(Events::Keyboard::isKeyDownOnce("f4")){
        Resources::setCurrentScene("Sol");
        Resources::setActiveCamera(static_cast<SolarSystem*>(Resources::getCurrentScene())->getPlayerCamera());
    }
    if(Events::Keyboard::isKeyDownOnce("f5")){
        Resources::setCurrentScene("CapsuleSpace");
        Resources::setActiveCamera(static_cast<SolarSystem*>(Resources::getCurrentScene())->getPlayerCamera());
    }
    if(Events::Keyboard::isKeyDownOnce("f7")){ Renderer::Settings::setAntiAliasingAlgorithm(AntiAliasingAlgorithm::None); }
    if(Events::Keyboard::isKeyDownOnce("f8")){ Renderer::Settings::setAntiAliasingAlgorithm(AntiAliasingAlgorithm::SMAA); }
    if(Events::Keyboard::isKeyDownOnce("f9")){ Renderer::Settings::setAntiAliasingAlgorithm(AntiAliasingAlgorithm::FXAA); }
    if(Events::Keyboard::isKeyDownOnce("f10")){ Renderer::Settings::SSAO::enable(!Renderer::Detail::RendererInfo::SSAOInfo::ssao); }
	if(Events::Keyboard::isKeyDownOnce("f11")){ Renderer::Detail::RendererInfo::GeneralInfo::stencil = !Renderer::Detail::RendererInfo::GeneralInfo::stencil; }

    if(Events::Keyboard::isKeyDown("z")){
        Renderer::Settings::HDR::setExposure(Renderer::Settings::HDR::getExposure() - 0.03f);
    }
    else if(Events::Keyboard::isKeyDown("x")){
        Renderer::Settings::HDR::setExposure(Renderer::Settings::HDR::getExposure() + 0.03f);
    }
    if(Events::Keyboard::isKeyDown("c")){
        Renderer::Settings::setGamma(Renderer::Settings::getGamma() - 0.02f);
    }
    else if(Events::Keyboard::isKeyDown("k")){
        Renderer::Settings::setGamma(Renderer::Settings::getGamma() + 0.02f);
    }

    if(Events::Keyboard::isKeyDown("n")){
        Resources::getMaterial("Defiant")->setMetalness(Resources::getMaterial("Defiant")->metalness() - 0.02f);
        Resources::getMaterial("Dreadnaught")->setMetalness(Resources::getMaterial("Dreadnaught")->metalness() - 0.02f);
    }
    else if(Events::Keyboard::isKeyDown("m")){
        Resources::getMaterial("Defiant")->setMetalness(Resources::getMaterial("Defiant")->metalness() + 0.02f);
        Resources::getMaterial("Dreadnaught")->setMetalness(Resources::getMaterial("Dreadnaught")->metalness() + 0.02f);
    }
    if(Events::Keyboard::isKeyDown("v")){
        Resources::getMaterial("Defiant")->setSmoothness(Resources::getMaterial("Defiant")->smoothness() - 0.02f);
        Resources::getMaterial("Dreadnaught")->setSmoothness(Resources::getMaterial("Dreadnaught")->smoothness() - 0.02f);
    }
    else if(Events::Keyboard::isKeyDown("b")){
        Resources::getMaterial("Defiant")->setSmoothness(Resources::getMaterial("Defiant")->smoothness() + 0.02f);
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
