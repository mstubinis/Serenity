#include "CapsuleSpace.h"
#include "ObjectDisplay.h"
#include "Engine_Resources.h"
#include "Material.h"
#include "Ship.h"
#include "GameCamera.h"
#include "Light.h"
#include "Engine_Renderer.h"
#include "Texture.h"
#include <boost/lexical_cast.hpp>

using namespace Engine;

CapsuleStar::CapsuleStar(float size,glm::v3 pos, std::string name,Scene* scene):ObjectDisplay("Plane","SunFlare",pos,glm::vec3(size),name,scene){
	m_Light = new PointLight(name + " Light",pos,scene);

	m_Light->setConstant(0.000003f);
	m_Light->setLinear(0.000003f);
	m_Light->setExponent(0.000003f);

	setColor(1,235.0f/255.0f,206.0f/255.0f,1);
	m_Light->setColor(1,75.0f/255.0f,0,1);
	this->m_Shadeless = true;
}
CapsuleStar::~CapsuleStar(){
}
void CapsuleStar::update(float dt){
	glm::v3 pos = getPosition();

	translate(0,0,(-55 * 100 ) * dt);

	if(pos.z >= 70 * 100){
		float x = static_cast<float>(((rand() % 200) - 100)/100.0f) * 3.7f; if(x > 0) x += 1.5f; if(x < 0) x -= 1.5f;
		float y = static_cast<float>(((rand() % 200) - 100)/100.0f) * 3.7f; if(y > 0) y += 1.5f; if(y < 0) y -= 1.5f;
		setPosition(x*100,y*100,-70*100);
	}

	m_Light->setPosition(pos);

	this->m_Orientation = Resources::getActiveCamera()->getOrientation();

	ObjectBasic::update(dt);
}
void CapsuleStar::draw(GLuint shader,bool debug){
	ObjectDisplay::draw(shader,debug);
}

CapsuleTunnel::CapsuleTunnel(float tunnelRadius, std::string name, std::string material, Scene* scene):ObjectDisplay("CapsuleTunnel",material,glm::v3(0),glm::vec3(1),name,scene){
	m_TunnelRadius = tunnelRadius;
	setScale(m_TunnelRadius,m_TunnelRadius,m_TunnelRadius);
}
CapsuleTunnel::~CapsuleTunnel(){
}
void CapsuleTunnel::draw(GLuint shader,bool debug){
	glEnablei(GL_BLEND,0);
	glBlendFunci(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA,0);

	ObjectDisplay::draw(shader,debug);
}
CapsuleRibbon::CapsuleRibbon(float tunnelRadius, std::string name, std::string material, Scene* scene):ObjectDisplay("CapsuleRibbon",material,glm::v3(0),glm::vec3(1),name,scene){
	m_TunnelRadius = tunnelRadius;
	setScale(m_TunnelRadius,m_TunnelRadius,m_TunnelRadius);
}
CapsuleRibbon::~CapsuleRibbon(){
}
void CapsuleRibbon::draw(GLuint shader,bool debug){
	glDisable(GL_DEPTH_TEST);
	ObjectDisplay::draw(shader,debug);
	glEnable(GL_DEPTH_TEST);
}

CapsuleSpace::CapsuleSpace():SolarSystem("CapsuleSpace","NULL"){
	setSkybox(nullptr);
	setAmbientLightColor(82.0f/255.0f,29.0f/255.0f,0);
	setBackgroundColor(1,0,0);

	if(!Resources::Detail::ResourceManagement::m_Meshes.count("CapsuleTunnel")){
		Resources::addMesh("CapsuleTunnel","Models/capsuleTunnel.obj");
	}
	if(!Resources::Detail::ResourceManagement::m_Meshes.count("CapsuleRibbon")){
		Resources::addMesh("CapsuleRibbon","Models/capsuleRibbon.obj");
	}
	if(!Resources::Detail::ResourceManagement::m_Materials.count("Capsule_A")){
		Resources::addMaterial("Capsule_A","Textures/Effects/capsule_a.png");
		Resources::getMaterial("Capsule_A")->setShadeless(true);
	}
	if(!Resources::Detail::ResourceManagement::m_Materials.count("Capsule_B")){
		Resources::addMaterial("Capsule_B","Textures/Effects/capsule_b.png");
		Resources::getMaterial("Capsule_B")->setShadeless(true);
	}
	if(!Resources::Detail::ResourceManagement::m_Materials.count("Capsule_C")){
		Resources::addMaterial("Capsule_C","Textures/Effects/capsule_c.png");
		Resources::getMaterial("Capsule_C")->setShadeless(true);
		Resources::getMaterial("Capsule_C")->setBaseGlow(0.2f);
	}

	m_TunnelA = new CapsuleTunnel(5000,"Capsule_Tunnel_A","Capsule_A",this);
	m_TunnelB = new CapsuleTunnel(5000,"Capsule_Tunnel_B","Capsule_B",this);
	m_Ribbon = new CapsuleRibbon(5000,"Capsule_Tunnel_C_Ribbon","Capsule_C",this);

	glm::vec3 oldScale = m_TunnelB->getScale();
	m_TunnelB->setScale(oldScale.x-(0.62f * m_TunnelB->getTunnelRadius()),
		                oldScale.y-(0.62f * m_TunnelB->getTunnelRadius()),
						oldScale.z);

	m_TunnelA->setPosition(0,0,0);
	m_TunnelB->setPosition(0,0,0);
	m_Ribbon->setPosition(0,300,0);

	float step = -200.0f;
	for(unsigned int i = 0; i < 30; i++){

		float x = static_cast<float>(((rand() % 200) - 100)/100.0f) * 3.7f; if(x > 0) x += 1.5f; if(x < 0) x -= 1.5f;
		float y = static_cast<float>(((rand() % 200) - 100)/100.0f) * 3.7f; if(y > 0) y += 1.5f; if(y < 0) y -= 1.5f;

		glm::v3 pos = glm::v3(x,y,step)*glm::v3(100);

		m_CapsuleStars.push_back(new CapsuleStar(100,pos,"ZZZZZZ_CapsuleStar" + boost::lexical_cast<std::string>(i),this));

		step -= 8.0f;
	}

	//this to just test. should set player / camera dynamically
	setPlayer(new Ship("Defiant","Defiant",true,"Dreadnought",glm::v3(0),glm::vec3(1),nullptr,this));
	setPlayerCamera(static_cast<GameCamera*>(Resources::getActiveCamera()));
	getPlayerCamera()->follow(getPlayer());

}
CapsuleSpace::~CapsuleSpace(){
}
void CapsuleSpace::update(float dt){
	SolarSystem::update(dt);

	float aRadius = m_TunnelA->getTunnelRadius();
	float bRadius = m_TunnelB->getTunnelRadius();

	m_TunnelA->translate(0,0,(45 * aRadius) * dt);
	m_TunnelB->translate(0,0,(18 * bRadius) * dt);
	m_Ribbon->translate(0,0,(-15 * aRadius) * dt);

	float tunnelARotRand = static_cast<float>(rand() % 4) + 14.0f;
	float tunnelBRotRand = static_cast<float>(rand() % 2) + 2;

	m_TunnelA->rotate(0,0,tunnelARotRand);
	m_TunnelB->rotate(0,0,-tunnelBRotRand);

	if(m_TunnelA->getPosition().z >= 12.112 * aRadius || m_TunnelA->getPosition().z <= -12.112 * aRadius){
		m_TunnelA->setPosition(0,0,0);
	}
	if(m_TunnelB->getPosition().z >= 12.112 * bRadius || m_TunnelB->getPosition().z <= -12.112 * bRadius){
		m_TunnelB->setPosition(0,0,0);
	}
	if(m_Ribbon->getPosition().z >= 11.6 * aRadius || m_Ribbon->getPosition().z <= -11.6 * aRadius){
		m_Ribbon->setPosition(0,300,0);
	}
	getPlayer()->setPosition(0,0,0);
	getPlayer()->setOrientation(glm::quat());
}