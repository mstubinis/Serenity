#include "HUD.h"
#include "Engine_Resources.h"
#include "Engine_Events.h"
#include "Font.h"
#include "Texture.h"
#include "Scene.h"
#include "Ship.h"
#include "Camera.h"
#include "SolarSystem.h"
#include "Planet.h"

#include <boost/lexical_cast.hpp>

using namespace Engine;
using namespace Engine::Events;


HUD::HUD(PlayerShip* player){
	m_Font = new Font("Fonts/consolas.fnt");
	m_Player = player;
	m_Color = glm::vec3(55.0f/255.0f,
		                107.0f/255.0f,
						126.0f/255.0f);
	m_TargetIterator = 0;
	m_WarpIndicatorSize = glm::vec2(30,115);
}
HUD::~HUD(){
	delete m_Font;
}
void HUD::update(float dt){
	if(Keyboard::isKeyDownOnce(",")){
		SolarSystem* scene = static_cast<SolarSystem*>(Resources::getCurrentScene());
		/*
		unsigned int totalAmount = scene->getStars().size() +
								   scene->getPlanets().size() + 
								   scene->getMoons().size();
		if(m_Player->getTarget() == nullptr)
			m_Player->setTarget(scene->getStars().at(0));
		else{
			m_TargetIterator++;

			ObjectDisplay* target;

			if(scene->getStars().size() > m_TargetIterator){
				target = scene->getStars().at(m_TargetIterator);
				m_Player->setTarget(target);
			}
			else if(scene->getStars().size()+scene->getPlanets().size() > m_TargetIterator){
				target = scene->getPlanets().at(m_TargetIterator - scene->getStars().size());
				m_Player->setTarget(target);
			}
			else if(scene->getStars().size()+scene->getPlanets().size()+scene->getMoons().size() > m_TargetIterator){
				target = scene->getMoons().at(m_TargetIterator - scene->getStars().size() - scene->getPlanets().size());
				m_Player->setTarget(target);
			}
			else{
				m_Player->setTarget(scene->getStars().at(0));
				m_TargetIterator = 0;
			}
		}
		*/
		Object* target = scene->getObject("Earth Lagrange Point");
		m_Player->setTarget(target);

	}
	else if(Keyboard::isKeyDownOnce(".")){
		/*
		SolarSystem* scene = static_cast<SolarSystem*>(Resources::getCurrentScene());
		unsigned int totalAmount = scene->getStars().size() +
								   scene->getPlanets().size() + 
								   scene->getMoons().size();

		if(m_Player->getTarget() == nullptr)
			m_Player->setTarget(scene->getStars().at(0));
		else{
			m_TargetIterator++;
			if(scene->getStars().size() > m_TargetIterator){
				std::reverse(scene->getStars().begin(),scene->getStars().end());
				m_Player->setTarget(scene->getStars().at(m_TargetIterator));
			}
			else if(scene->getStars().size()+scene->getPlanets().size() > m_TargetIterator){
				std::reverse(scene->getPlanets().begin(),scene->getPlanets().end());
				m_Player->setTarget(scene->getPlanets().at(m_TargetIterator - scene->getStars().size()));
			}
			else if(scene->getStars().size()+scene->getPlanets().size()+scene->getMoons().size() > m_TargetIterator){
				std::reverse(scene->getMoons().begin(),scene->getMoons().end());
				m_Player->setTarget(scene->getMoons().at(m_TargetIterator - scene->getStars().size() - scene->getPlanets().size()));
			}
			else{
				m_Player->setTarget(scene->getStars().at(0));
				m_TargetIterator = 0;
			}
		}
		*/
	}
}
void HUD::render(bool debug){
	//render hud stuff
	glm::vec2 winSize = glm::vec2(Resources::getWindow()->getSize().x,Resources::getWindow()->getSize().y);

	// render warp drive
	//Engine::Renderer::renderRectangle(glm::vec2(winSize.x/2 - 100,winSize.y - m_WarpIndicatorSize.y/2),glm::vec4(m_Color.x,m_Color.y,m_Color.z,0.3f),m_WarpIndicatorSize.x,m_WarpIndicatorSize.y,0,0);

	#pragma region renderCrossHairAndOtherInfo
	if(m_Player->getTarget() != nullptr){
		glm::vec3 pos = m_Player->getTarget()->getScreenCoordinates();
		float scl = glm::max(0.5f,m_Player->getTarget()->getRadius()*35/m_Player->getTarget()->getDistance(Resources::getActiveCamera()));

		if(pos.z == 1){
			Resources::getTexture("Textures/HUD/Crosshair")->render(glm::vec2(pos.x,pos.y),glm::vec4(m_Color.x,m_Color.y,m_Color.z,1),0,glm::vec2(scl,scl),0);
			unsigned long long distanceInKm = (m_Player->getTarget()->getDistanceLL(m_Player) / 10.0f);
			std::string stringRepresentation = "";
			if(distanceInKm > 0){
				stringRepresentation = Engine::convertNumberToStringCommas(static_cast<unsigned long long>(distanceInKm)) + " Km";
			}
			else{
				float distanceInm = (m_Player->getTarget()->getDistance(m_Player))*100.0f;
				stringRepresentation = boost::lexical_cast<std::string>(static_cast<unsigned int>(distanceInm)) + " m";
			}
			m_Font->renderText(m_Player->getTarget()->getName() + "\n"+stringRepresentation,glm::vec2(pos.x+40,pos.y-15),glm::vec4(m_Color.x,m_Color.y,m_Color.z,1),0,glm::vec2(0.7f,0.7f),0);
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
			Resources::getTexture("Textures/HUD/CrosshairArrow")->render(glm::vec2(pos.x,pos.y),glm::vec4(m_Color.x,m_Color.y,m_Color.z,1),angle,glm::vec2(scl,scl),0);
		}

	}
	#pragma endregion

	#pragma region DrawDebugStuff
	if(debug){
		m_Font->renderText("Delta Time: " + boost::lexical_cast<std::string>(Resources::dt()) +
						   "\nFPS: " + boost::lexical_cast<std::string>(static_cast<unsigned int>(1.0f/Resources::dt())) + 
						   "\nObject Count: " + boost::lexical_cast<std::string>(Resources::getCurrentScene()->getObjects().size()),
						   glm::vec2(10,10),glm::vec4(m_Color.x,m_Color.y,m_Color.z,1),0,glm::vec2(0.8f,0.8f));
	}
	else{
		m_Font->renderText("FPS: " + boost::lexical_cast<std::string>(static_cast<unsigned int>(1.0f/Resources::dt())),
						   glm::vec2(10,10),glm::vec4(m_Color.x,m_Color.y,m_Color.z,1),0,glm::vec2(0.8f,0.8f));
	}
	#pragma endregion
}