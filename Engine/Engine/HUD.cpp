#include "HUD.h"
#include "Engine_Resources.h"
#include "Font.h"
#include "Texture.h"
#include "Scene.h"

#include <boost/lexical_cast.hpp>

using namespace Engine;

HUD::HUD(){
	m_Font = new Font("Fonts/consolas.fnt");
}
HUD::~HUD(){
	delete m_Font;
}
void HUD::update(float dt){
}
void HUD::render(){
	//render hud stuff
	m_Font->renderText("Delta Time: " + boost::lexical_cast<std::string>(Resources::dt()) +"\nFPS: " + boost::lexical_cast<std::string>(static_cast<unsigned int>(1.0f/Resources::dt())),glm::vec2(10,10),glm::vec3(1,1,1),0,glm::vec2(0.8f,0.8f));
}