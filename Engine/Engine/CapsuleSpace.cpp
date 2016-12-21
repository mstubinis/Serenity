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

CapsuleEnd::CapsuleEnd(float size,glm::v3 pos, glm::vec3 color, std::string name, Scene* scene):ObjectDisplay("Plane","Capsule_D",pos,glm::vec3(size),name,scene){
    setColor(color.x,color.y,color.z,1);
}
CapsuleEnd::~CapsuleEnd(){}
void CapsuleEnd::update(float dt){
    ObjectBasic::update(dt);
}
void CapsuleEnd::draw(GLuint shader,bool debug,bool godsRays){
    ObjectDisplay::draw(shader,debug,godsRays);
}
CapsuleStar::CapsuleStar(float size,glm::v3 pos, std::string name,Scene* scene,bool makeLight):ObjectDisplay("Plane","SunFlare",pos,glm::vec3(size),name,scene){
    m_Light = nullptr;
    if(makeLight){
        m_Light = new PointLight(name + " Light",pos/glm::num(100),scene);
        m_Light->setConstant(0.1f);m_Light->setLinear(0.1f);m_Light->setExponent(0.1f);m_Light->setColor(255,124,27,255);
        m_Light->setSpecularPower(2.0f);
    }
    setColor(255,235,206,255);
    this->m_Shadeless = true;
}
CapsuleStar::~CapsuleStar(){}
void CapsuleStar::update(float dt){
    glm::v3 pos = getPosition();
    translate(0,0,(-120 * 50 ) * dt);
    if(pos.z >= 200 * 225){
        float x = float(((rand() % 200) - 100)/100.0f) * 3.7f; if(x > 0) x += 1.5f; if(x < 0) x -= 1.5f;
        float y = float(((rand() % 200) - 100)/100.0f) * 3.7f; if(y > 0) y += 1.5f; if(y < 0) y -= 1.5f;
        setPosition(x*50,y*50,-200*225);
    }
	if(m_Light != nullptr){
		m_Light->setPosition(pos/glm::num(75));
		if(glm::distance(m_Light->getPosition(),Resources::getActiveCamera()->getPosition()) > m_Light->getLightRadius() * 1.1f){m_Light->deactivate();}
		else{m_Light->activate();}
	}
    this->m_Orientation = Resources::getActiveCamera()->getOrientation();
    ObjectBasic::update(dt);
}
void CapsuleStar::draw(GLuint shader,bool debug, bool godsRays){
	Renderer::Settings::disableDepthMask();
    ObjectDisplay::draw(shader,debug,godsRays);
	Renderer::Settings::enableDepthMask();
}

CapsuleTunnel::CapsuleTunnel(float tunnelRadius, std::string name, std::string material, Scene* scene):ObjectDisplay("CapsuleTunnel",material,glm::v3(0),glm::vec3(1),name,scene){
    m_TunnelRadius = tunnelRadius;
    setScale(m_TunnelRadius,m_TunnelRadius,m_TunnelRadius);
}
CapsuleTunnel::~CapsuleTunnel(){}
void CapsuleTunnel::draw(GLuint shader,bool debug,bool godsRays){
    ObjectDisplay::draw(shader,debug,godsRays);
}
CapsuleRibbon::CapsuleRibbon(float tunnelRadius, std::string name, std::string material, Scene* scene):ObjectDisplay("CapsuleRibbon",material,glm::v3(0),glm::vec3(1),name,scene){
    m_TunnelRadius = tunnelRadius;
    setScale(m_TunnelRadius,m_TunnelRadius,m_TunnelRadius);
}
CapsuleRibbon::~CapsuleRibbon(){}
void CapsuleRibbon::draw(GLuint shader,bool debug,bool godsRays){
	Renderer::Settings::disableDepthTest();
    ObjectDisplay::draw(shader,debug,godsRays);
	Renderer::Settings::enableDepthTest();
}

CapsuleSpace::CapsuleSpace():SolarSystem("CapsuleSpace","NULL"){
    setSkybox(nullptr);
    setAmbientLightColor(20.0f,12.0f,0);
    setBackgroundColor(255.0f,0,0);

    PointLight* l = new PointLight("Capsule_Static_Light",glm::v3(0,1.7f,0),this);
    l->setColor(255,225,235,255);
    l->setSpecularPower(0.0f);

    if(!Resources::getMaterial("CapsuleTunnel")){
        Resources::addMesh("CapsuleTunnel","data/Models/capsuleTunnel.obj",COLLISION_TYPE_NONE);
    }
    if(!Resources::getMaterial("CapsuleRibbon")){
        Resources::addMesh("CapsuleRibbon","data/Models/capsuleRibbon.obj",COLLISION_TYPE_NONE);
    }
    if(!Resources::getMaterial("Capsule_A")){
        Resources::addMaterial("Capsule_A","data/Textures/Effects/capsule_a.png");
        Resources::getMaterial("Capsule_A")->setShadeless(true);
    }
    if(!Resources::getMaterial("Capsule_B")){
        Resources::addMaterial("Capsule_B","data/Textures/Effects/capsule_b.png");
        Resources::getMaterial("Capsule_B")->setShadeless(true);
    }
    if(!Resources::getMaterial("Capsule_C")){
        Resources::addMaterial("Capsule_C","data/Textures/Effects/capsule_c.png");
        Resources::getMaterial("Capsule_C")->setShadeless(true);
        Resources::getMaterial("Capsule_C")->setGlow(1.0f);
    }
    if(!Resources::getMaterial("Capsule_D")){
        Resources::addMaterial("Capsule_D","data/Textures/Effects/capsule_d.png");
        Resources::getMaterial("Capsule_D")->setShadeless(true);
    }

    m_TunnelA = new CapsuleTunnel(5000,"AAAA","Capsule_A",this);
    m_TunnelB = new CapsuleTunnel(5000,"AAAB","Capsule_B",this);
    m_Ribbon = new CapsuleRibbon(5000,"AAAC","Capsule_C",this);

    m_FrontEnd = new CapsuleEnd(2250,glm::v3(0,0,-25000),glm::vec3(1),"AAAD",this);
    m_BackEnd = new CapsuleEnd(1650,glm::v3(0,0,25000),glm::vec3(0),"AAAE",this);

    m_BackEnd->rotate(0,180,0,false);

    glm::vec3 oldScale = m_TunnelB->getScale();
    m_TunnelB->setScale(oldScale.x-(0.62f * m_TunnelB->getTunnelRadius()),oldScale.y-(0.62f * m_TunnelB->getTunnelRadius()),oldScale.z);

    m_TunnelA->setPosition(0,0,0);
    m_TunnelB->setPosition(0,0,0);

    m_Ribbon->setPosition(0,600,0);

    float step = -10.0f;
    for(uint i = 0; i < 300; i++){
        float x = float(((rand() % 200) - 100)/100.0f) * 3.7f; if(x > 0) x += 1.5f; if(x < 0) x -= 1.5f;
        float y = float(((rand() % 200) - 100)/100.0f) * 3.7f; if(y > 0) y += 1.5f; if(y < 0) y -= 1.5f;

        glm::v3 pos = glm::v3(x,y,step)*glm::v3(50);

        bool spawnLight = false;
        if(i % 3 == 0){
            spawnLight = true;
        }
        m_CapsuleStars.push_back(new CapsuleStar(50,pos,"AAAF_Star_" + boost::lexical_cast<std::string>(i),this,spawnLight));
        step -= 6.0f;
    }

    //this to just test. should set player / camera dynamically
    setPlayer(new Ship("Dreadnaught","Dreadnaught",true,"Dreadnaught",glm::v3(0),glm::vec3(1),nullptr,this));
    setPlayerCamera(static_cast<GameCamera*>(Resources::getActiveCamera()));
    getPlayerCamera()->follow(getPlayer());

}
CapsuleSpace::~CapsuleSpace(){
}
void CapsuleSpace::update(float dt){
    m_Timer += dt*2;
    SolarSystem::update(dt);

    float aRadius = m_TunnelA->getTunnelRadius();
    float bRadius = m_TunnelB->getTunnelRadius();

    m_TunnelA->translate(0,0,(45 * aRadius) * dt);
    m_TunnelB->translate(0,0,(18 * bRadius) * dt);
    m_Ribbon->translate(0,0,(-15 * aRadius) * dt);

    float tunnelARotRand = float(rand() % 4) + 14.0f;
    float tunnelBRotRand = float(rand() % 2) + 2;

    m_TunnelA->rotate(0,0,tunnelARotRand);
    m_TunnelB->rotate(0,0,-tunnelBRotRand);
    m_BackEnd->rotate(0,0,150);
    m_FrontEnd->rotate(0,0,-150);

    if(m_TunnelA->getPosition().z >= 12.112 * aRadius || m_TunnelA->getPosition().z <= -12.112 * aRadius){
        m_TunnelA->setPosition(0,0,0);
    }
    if(m_TunnelB->getPosition().z >= 12.112 * bRadius || m_TunnelB->getPosition().z <= -12.112 * bRadius){
        m_TunnelB->setPosition(0,0,0);
    }
    if(m_Ribbon->getPosition().z >= 20 * aRadius || m_Ribbon->getPosition().z <= -20 * aRadius){
        m_Ribbon->setPosition(0,300,0);
    }
    getPlayer()->setPosition(0,0,0);
    getPlayer()->setOrientation(glm::quat());

    float x = glm::sin(m_Timer)*0.035f;
    float y = glm::cos(m_Timer)*0.035f;

    float rot = glm::sin(m_Timer)*13;

    for(auto item:getPlayer()->getDisplayItems()){
        item->setPosition(glm::vec3(x*1.2f,-y,0));
		item->setOrientation(0,0,rot);
    }
}