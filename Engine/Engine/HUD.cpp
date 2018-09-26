#include "HUD.h"
#include "Engine_Resources.h"
#include "Engine_Events.h"
#include "Engine_Renderer.h"
#include "Font.h"
#include "Texture.h"
#include "Scene.h"
#include "Ship.h"
#include "Camera.h"
#include "SolarSystem.h"
#include "Planet.h"
#include "Engine_Window.h"
#include "Engine_Math.h"
#include "Material.h"

#include "ResourceManifest.h"

#include <glm/vec4.hpp>

using namespace Engine;
using namespace std;

HUD::HUD(){
    m_Font = Resources::addFont("data/Fonts/consolas.fnt");
    Engine::Math::setColor(m_Color,255,255,0);
    m_TargetIterator = 0;
    m_WarpIndicatorSize = glm::vec2(30,115);
}
HUD::~HUD(){
}
const glm::vec3 HUD::getColor() const { return m_Color; }
void HUD::setColor(glm::vec3 c){ m_Color = c; }


uint _count = 0;
void HUD::update(const float& dt){
    if(Engine::isKeyDownOnce(",")){
        SolarSystem* scene = (SolarSystem*)(Resources::getCurrentScene());
        const unordered_map<string,Planet*>& planets = scene->getPlanets();
        uint a = 0;
        for(auto p:planets){
            if(a == _count){
                //scene->getPlayer()->setTarget("Moon");
                break;
            }
            ++a;
        }
        ++_count;
        if (_count > scene->getPlanets().size()-1){ _count = 0; }
    }
    else if(Engine::isKeyDownOnce(".")){
        SolarSystem* scene = (SolarSystem*)(Resources::getCurrentScene());
        const unordered_map<string,Planet*>& planets = scene->getPlanets();
        uint a = 0;
        for(auto p:planets){
            if(a == _count){
                //scene->getPlayer()->setTarget("Moon");
                break;
            }
            ++a;
        }
        --_count;
        if (_count <= 0){ _count = scene->getPlanets().size()-1; }
    }
}
void HUD::render(){
    //render hud stuff
    SolarSystem* scene = (SolarSystem*)(Resources::getCurrentScene());
    Ship* player = scene->getPlayer();
    glm::vec2 winSize = glm::vec2(Resources::getWindow()->getSize().x,Resources::getWindow()->getSize().y);

    // render warp drive
    //Engine::Renderer::renderRectangle(glm::vec2(winSize.x/2 - 100,winSize.y - m_WarpIndicatorSize.y/2),glm::vec4(m_Color.x,m_Color.y,m_Color.z,0.3f),m_WarpIndicatorSize.x,m_WarpIndicatorSize.y,0,0);

    Font* font = Resources::getFont(m_Font);

    #pragma region renderCrossHairAndOtherInfo
    
    if(player->getTarget()){
        auto* body = player->getTarget()->getComponent<ComponentBody>();
        auto* model = player->getTarget()->getComponent<ComponentModel>();
        const glm::vec3& pos = body->getScreenCoordinates();
        float scl = glm::max(0.5f,model->radius()*23.0f / Resources::getCurrentScene()->getActiveCamera()->getDistance(player->getTarget()));
        if(pos.z == 1){
            Material* crosshair = (Material*)ResourceManifest::CrosshairMaterial.get();
            crosshair->getComponent(MaterialComponentType::Diffuse)->texture()->render(glm::vec2(pos.x,pos.y),glm::vec4(m_Color.x,m_Color.y,m_Color.z,1),0,glm::vec2(scl,scl),0.1f);
            //unsigned long long distanceInKm = (player->getTarget()->getDistanceLL(player) / 10);
            string stringRepresentation = "";
            //if(distanceInKm > 0){
                //stringRepresentation = Engine::convertNumToNumWithCommas(unsigned long long(distanceInKm)) + " Km";
            //}
            //else{
                //float distanceInm = (player->getTarget()->getDistance(player))*100.0f;
                //stringRepresentation = to_string(uint(distanceInm)) + " m";
            //}
            font->renderText(/*player->getTarget()->name() + */"\n"+stringRepresentation,glm::vec2(pos.x+40,pos.y-15),glm::vec4(m_Color.x,m_Color.y,m_Color.z,1),0,glm::vec2(0.7f,0.7f),0.1f);
        }
        else{
            Material* crosshairArrow = (Material*)ResourceManifest::CrosshairArrowMaterial.get();
            glm::vec2 winSize = glm::vec2(Resources::getWindow()->getSize().x,Resources::getWindow()->getSize().y);
            scl = 1;

            float angle = 0;
            if(pos.y > 2 && pos.y < winSize.y - 2)
                if(pos.x < 2)                      angle = 45;
                else                               angle = 225;
            else if(pos.y <= 1){
                if(pos.x <= 1)                     angle = 0;
                else if(pos.x > winSize.x - 2)     angle = -90;
                else                               angle = -45;
            }
            else{
                if(pos.x < 2)                      angle = 90;
                else if(pos.x > winSize.x - 2)     angle = 180;
                else                               angle = 135;
            }
            crosshairArrow->getComponent(MaterialComponentType::Diffuse)->texture()->render(glm::vec2(pos.x,pos.y),glm::vec4(m_Color.x,m_Color.y,m_Color.z,1),glm::radians(angle),glm::vec2(scl,scl),0.1f);
        }
    }
    
    #pragma endregion

    #pragma region DrawDebugStuff

    string stuff = "true";
    if (!Renderer::Settings::General::enabled1())
        stuff = "false";

    font->renderText(Engine::Data::reportTime() + 
                     "\nSSAO Bias: " + to_string(Renderer::Settings::SSAO::getBias()) + 
                     "\nSSAO Radius: " + to_string(Renderer::Settings::SSAO::getRadius()) +
                     "\nSSAO Scale: " + to_string(Renderer::Settings::SSAO::getScale()) +
                     "\nSSAO Blur Strength: " + to_string(Renderer::Settings::SSAO::getBlurStrength()) +
                     "\nSSAO Blur Radius: " + to_string(Renderer::Settings::SSAO::getBlurRadius()) +
                     "\nSSAO Intensity: " + to_string(Renderer::Settings::SSAO::getIntensity()) +
                     "\nSSAO Samples: " + to_string(Renderer::Settings::SSAO::getSamples()),
                     glm::vec2(10,Resources::getWindowSize().y-10),glm::vec4(m_Color.x,m_Color.y,m_Color.z,1),0,glm::vec2(0.8f,0.8f),0.1f);

    #pragma endregion
}
