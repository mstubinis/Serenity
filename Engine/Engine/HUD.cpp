#include "HUD.h"
#include "Engine_Resources.h"
#include "Font.h"
#include "Texture.h"
#include "Scene.h"
#include "Ship.h"
#include "Camera.h"

#include <boost/lexical_cast.hpp>

using namespace Engine;

HUD::HUD(PlayerShip* player){
	m_Font = new Font("Fonts/consolas.fnt");
	m_Player = player;
	m_Color = glm::vec3(1,1,1);
}
HUD::~HUD(){
	delete m_Font;
}
void HUD::update(float dt){
}
void HUD::render(){
	//render hud stuff
	m_Font->renderText("Delta Time: " + boost::lexical_cast<std::string>(Resources::dt()) +"\nFPS: " + boost::lexical_cast<std::string>(static_cast<unsigned int>(1.0f/Resources::dt())),glm::vec2(10,10),glm::vec3(1,1,1),0,glm::vec2(0.8f,0.8f));

	if(m_Player->getTarget() != nullptr)
	{
		glm::vec3 pos = m_Player->getTarget()->getScreenCoordinates();
		float scl = glm::max(1.0f,m_Player->getTarget()->getRadius()*35/m_Player->getTarget()->getDistance(Resources::getActiveCamera()));
		Resources::getTexture("Textures/HUD/Crosshair")->render(glm::vec2(pos.x,pos.y),m_Color,0,glm::vec2(scl,scl),0);
	}
}