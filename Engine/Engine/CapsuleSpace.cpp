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
using namespace std;

CapsuleEnd::CapsuleEnd(float size,glm::vec3 pos, glm::vec3 color, std::string name, Scene* scene):ObjectDisplay("Plane","Capsule_D",pos,glm::vec3(size),name,scene){setColor(color.x,color.y,color.z,1);}
CapsuleEnd::~CapsuleEnd(){}
void CapsuleEnd::update(float dt){ObjectDisplay::update(dt);}

CapsuleStar::CapsuleStar(float size,glm::vec3 pos, std::string name,Scene* scene,bool makeLight):ObjectDisplay("Plane","SunFlare",pos,glm::vec3(size),name,scene){
    m_Light = nullptr;
    if(makeLight){
        m_Light = new PointLight(name + " Light",pos/float(100),scene);
        m_Light->setAttenuation(LightRange::_65);
        m_Light->setColor(255,124,27,255);
    }
    setColor(255,235,206,255);
    this->m_Shadeless = true;
}
CapsuleStar::~CapsuleStar(){}
void CapsuleStar::update(float dt){
    glm::vec3 pos = getPosition();
    translate(0,0,(120 * 50 ) * dt,false);
    if(pos.z >= 200 * 225){
        float x = float(((rand() % 200) - 100)/100.0f) * 3.7f; if(x > 0) x += 1.5f; if(x < 0) x -= 1.5f;
        float y = float(((rand() % 200) - 100)/100.0f) * 3.7f; if(y > 0) y += 1.5f; if(y < 0) y -= 1.5f;
        setPosition(x*50,y*50,-200*225);
    }
    if(m_Light != nullptr){
        m_Light->setPosition(pos * 0.015f);
        if(glm::distance(m_Light->getPosition(),Resources::getActiveCamera()->getPosition()) > m_Light->getCullingRadius() + 15.0f){
            m_Light->deactivate();
        }
        else{
            m_Light->activate();
        }
    }
    this->m_Orientation = Resources::getActiveCamera()->getOrientation();
    ObjectDisplay::update(dt);
}

CapsuleTunnel::CapsuleTunnel(float tunnelRadius, string name, string material, Scene* scene):ObjectDisplay("CapsuleTunnel",material,glm::vec3(0),glm::vec3(1),name,scene){
    m_TunnelRadius = tunnelRadius;
    setScale(m_TunnelRadius,m_TunnelRadius,m_TunnelRadius);
}
CapsuleTunnel::~CapsuleTunnel(){}

CapsuleRibbon::CapsuleRibbon(float tunnelRadius, string name, string material, Scene* scene):ObjectDisplay("CapsuleRibbon",material,glm::vec3(0),glm::vec3(1),name,scene){
    m_TunnelRadius = tunnelRadius;
    setScale(m_TunnelRadius,m_TunnelRadius,m_TunnelRadius);
}
CapsuleRibbon::~CapsuleRibbon(){}
void CapsuleRibbon::bind(){
    Renderer::Settings::disableDepthTest();
    Renderer::Settings::disableDepthMask();
    BindableResource::bind();
}
void CapsuleRibbon::unbind(){
    BindableResource::unbind();
    Renderer::Settings::enableDepthTest();
    Renderer::Settings::enableDepthMask();
}

CapsuleSpace::CapsuleSpace():SolarSystem("CapsuleSpace","NULL"){
    setSkybox(nullptr);
    setBackgroundColor(255.0f,0,0);

    PointLight* l = new PointLight("Capsule_Static_Light",glm::vec3(0,1.7f,0),this);

    l->setColor(255,225,235,255);
    l->setSpecularIntensity(0.0f);

    if(!Resources::getMaterial("CapsuleTunnel")){
        Resources::addMesh("CapsuleTunnel","data/Models/capsuleTunnel.obj",CollisionType::None);
    }
    if(!Resources::getMaterial("CapsuleRibbon")){
        Resources::addMesh("CapsuleRibbon","data/Models/capsuleRibbon.obj",CollisionType::None);
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
        Resources::getMaterial("Capsule_C")->setGlow(0.01f);
    }
    if(!Resources::getMaterial("Capsule_D")){
        Resources::addMaterial("Capsule_D","data/Textures/Effects/capsule_d.png");
        Resources::getMaterial("Capsule_D")->setShadeless(true);
    }

    m_TunnelA = new CapsuleTunnel(5000,"AAAA","Capsule_A",this);
    m_TunnelB = new CapsuleTunnel(5000,"AAAB","Capsule_B",this);
    m_Ribbon = new CapsuleRibbon(5000,"AAAC","Capsule_C",this);

    m_FrontEnd = new CapsuleEnd(2250,glm::vec3(0,0,-25000),glm::vec3(1),"AAAD",this);
    m_BackEnd = new CapsuleEnd(1650,glm::vec3(0,0,25000),glm::vec3(0),"AAAE",this);

    m_BackEnd->rotate(0,180,0,false);

    glm::vec3 oldScale = m_TunnelB->getScale();
    m_TunnelB->setScale(oldScale.x-(0.62f * m_TunnelB->getTunnelRadius()),oldScale.y-(0.62f * m_TunnelB->getTunnelRadius()),oldScale.z);

    m_TunnelA->setPosition(0,0,0);
    m_TunnelB->setPosition(0,0,0);

    m_Ribbon->setPosition(0,300,0);

    float step = -10.0f;
    for(uint i = 0; i < 300; i++){
        float x = float(((rand() % 200) - 100)/100.0f) * 3.7f; if(x > 0) x += 1.5f; if(x < 0) x -= 1.5f;
        float y = float(((rand() % 200) - 100)/100.0f) * 3.7f; if(y > 0) y += 1.5f; if(y < 0) y -= 1.5f;

        glm::vec3 pos = glm::vec3(x,y,step) * glm::vec3(50);

        bool spawnLight = false;
        if(i % 3 == 0){
            spawnLight = true;
        }
        m_CapsuleStars.push_back(new CapsuleStar(50,pos,"AAAF_Star_" + boost::lexical_cast<string>(i),this,spawnLight));
        step -= 6.0f;
    }
    //this to just test. should set player / camera dynamically
    Ship* dread = new Ship("Dreadnaught","Dreadnaught",true,"Dreadnaught",glm::vec3(0),glm::vec3(1),nullptr,this);
    setPlayer(dread);
    setPlayerCamera(static_cast<GameCamera*>(Resources::getActiveCamera()));
    getPlayerCamera()->follow(getPlayer());

    LightProbe* lp = new LightProbe("CapsuleLightProbe",1024,glm::vec3(0.0f),true,this);
    dread->addChild(lp);
}
CapsuleSpace::~CapsuleSpace(){}
void CapsuleSpace::update(float dt){
    m_Timer += dt;
    SolarSystem::update(dt);

    float aRadius = m_TunnelA->getTunnelRadius();
    float bRadius = m_TunnelB->getTunnelRadius();

    m_TunnelA->translate(0,0,(45 * aRadius) * dt,false);
    m_TunnelB->translate(0,0,(18 * bRadius) * dt,false);
    m_Ribbon->translate(0,0,(15 * aRadius) * dt,false);

    float tunnelARotRand = float(rand() % 4) + 14.0f;
    float tunnelBRotRand = float(rand() % 2) + 2;

    m_TunnelA->rotate(0,0,glm::radians(tunnelARotRand));
    m_TunnelB->rotate(0,0,-glm::radians(tunnelBRotRand));
    m_BackEnd->rotate(0,0,2.61f);
    m_FrontEnd->rotate(0,0,-2.61f);

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

    float x = glm::sin(m_Timer*2.4f)*0.03f;
    float y = glm::cos(m_Timer*2.4f)*0.022f;

    float roll = glm::sin(m_Timer*2.4f)*5.0f;
    float pitch = glm::sin(m_Timer*2.4f)*0.7f;

    for(auto item:getPlayer()->getDisplayItems()){
        item->setPosition(glm::vec3(x*1.2f,-y,0));
        item->setOrientation(glm::radians(pitch),0,glm::radians(roll));
        
        //double check this (this is the light probe)
        //getPlayer()->getChildren().at(0)->setPosition(glm::vec3(x*1.2f,-y,0));
    }
}
