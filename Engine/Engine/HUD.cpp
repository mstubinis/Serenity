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

#include <boost/lexical_cast.hpp>

using namespace Engine;
using namespace Engine::Events;

HUD::HUD(){
    m_Font = new Font("data/Fonts/consolas.fnt");
    Engine::Math::setColor(m_Color,255,255,0);
    m_TargetIterator = 0;
    m_WarpIndicatorSize = glm::vec2(30,115);
}
HUD::~HUD(){
}

unsigned int count = 0;
void HUD::update(float dt){
    if(Keyboard::isKeyDownOnce(",")){
        SolarSystem* scene = static_cast<SolarSystem*>(Resources::getCurrentScene());
        
        std::vector<Planet*> objs;
        for(auto p:scene->getPlanets()){
            objs.push_back(p.second);
        }
		scene->getPlayer()->setTarget(Resources::getObject("Sun"));
        count++;
        if (count > scene->getPlanets().size()-1){ count = 0; }
    }
    else if(Keyboard::isKeyDownOnce(".")){
        SolarSystem* scene = static_cast<SolarSystem*>(Resources::getCurrentScene());
        
        std::vector<Planet*> objs;
        for(auto p:scene->getPlanets()){
            objs.push_back(p.second);
        }
        scene->getPlayer()->setTarget(objs[count]);
        count--;
        if (count <= 0){ count = scene->getPlanets().size()-1; }
    }
}
void HUD::render(bool debug){
    //render hud stuff
    SolarSystem* scene = static_cast<SolarSystem*>(Resources::getCurrentScene());
    Ship* player = scene->getPlayer();
    glm::vec2 winSize = glm::vec2(Resources::getWindow()->getSize().x,Resources::getWindow()->getSize().y);

    // render warp drive
    //Engine::Renderer::renderRectangle(glm::vec2(winSize.x/2 - 100,winSize.y - m_WarpIndicatorSize.y/2),glm::vec4(m_Color.x,m_Color.y,m_Color.z,0.3f),m_WarpIndicatorSize.x,m_WarpIndicatorSize.y,0,0);

    #pragma region renderCrossHairAndOtherInfo
    if(player->getTarget() != nullptr){
        glm::vec3 pos = player->getTarget()->getScreenCoordinates();
        glm::nType scl = glm::max(glm::nType(0.5f),glm::nType(player->getTarget()->getRadius()*35/player->getTarget()->getDistance(Resources::getActiveCamera())));

        if(pos.z == 1){
            Resources::getTexture("data/Textures/HUD/Crosshair.png")->render(glm::vec2(pos.x,pos.y),glm::vec4(m_Color.x,m_Color.y,m_Color.z,1),0,glm::vec2(scl,scl),0.1f);
            unsigned long long distanceInKm = (player->getTarget()->getDistanceLL(player) / 10);
            std::string stringRepresentation = "";
            if(distanceInKm > 0){
                stringRepresentation = Engine::convertNumToNumWithCommas(unsigned long long(distanceInKm)) + " Km";
            }
            else{
                glm::nType distanceInm = (player->getTarget()->getDistance(player))*100.0f;
                stringRepresentation = boost::lexical_cast<std::string>(uint(distanceInm)) + " m";
            }
            m_Font->renderText(player->getTarget()->getName() + "\n"+stringRepresentation,glm::vec2(pos.x+40,pos.y-15),glm::vec4(m_Color.x,m_Color.y,m_Color.z,1),0,glm::vec2(0.7f,0.7f),0.1f);
        }
        else{
            glm::vec2 winSize = glm::vec2(Resources::getWindow()->getSize().x,Resources::getWindow()->getSize().y);
            scl = 1;

            float angle = 0;
            if(pos.y > 2 && pos.y < winSize.y - 2)
                if(pos.x < 2)
                    angle = 45;
                else
                    angle = 225;
            else if(pos.y <= 1){
                if(pos.x <= 1)
                    angle = 0;
                else if(pos.x > winSize.x - 2)
                    angle = -90;
                else 
                    angle = -45;
            }
            else{
                if(pos.x < 2)
                    angle = 90;
                else if(pos.x > winSize.x - 2)
                    angle = 180;
                else
                    angle = 135;
            }
            Resources::getTexture("data/Textures/HUD/CrosshairArrow.png")->render(glm::vec2(pos.x,pos.y),glm::vec4(m_Color.x,m_Color.y,m_Color.z,1),angle,glm::vec2(scl,scl),0.1f);
        }

    }
    #pragma endregion

    #pragma region DrawDebugStuff
    if(debug){
        m_Font->renderText("Delta Time: " + boost::lexical_cast<std::string>(Resources::dt()) +
                           "\nFPS: " + boost::lexical_cast<std::string>(uint(1.0f/Resources::dt())) + 
                           "\nObject Count: " + boost::lexical_cast<std::string>(Resources::getCurrentScene()->getObjects().size()),
                           glm::vec2(10,10),glm::vec4(m_Color.x,m_Color.y,m_Color.z,1),0,glm::vec2(0.8f,0.8f),0.1f);
    }
    else{

		Object* earth = Resources::getObject("Earth");
		bool res = Resources::getActiveCamera()->sphereIntersectTest(earth);
		std::string res1 = "true";
		if(!res)
			res1 = "false";

        m_Font->renderText("FPS: " + boost::lexical_cast<std::string>(uint(1.0f/Resources::dt())) +
			"\nRes: " + res1,
                           glm::vec2(10,10),glm::vec4(m_Color.x,m_Color.y,m_Color.z,1),0,glm::vec2(0.8f,0.8f),0.1f);
    }
    #pragma endregion
}