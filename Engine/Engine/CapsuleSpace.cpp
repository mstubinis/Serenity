#include "CapsuleSpace.h"
#include "Engine_Resources.h"
#include "Material.h"
#include "Ship.h"
#include "GameCamera.h"
#include "Light.h"
#include "Engine_Renderer.h"
#include "Texture.h"
#include "GameSkybox.h"
#include "Mesh.h"
#include "MeshInstance.h"

#include "ResourceManifest.h"

#include <boost/lexical_cast.hpp>

using namespace Engine;
using namespace std;

CapsuleEnd::CapsuleEnd(float size,glm::vec3 pos, glm::vec3 color, Scene* scene):Entity(){
    scene->addEntity(this);

    ComponentModel* model = new ComponentModel(Mesh::Plane, ResourceManifest::CapsuleD,this);  addComponent(model);
    model->getModel()->setColor(color.x,color.y,color.z,1.0f);
    
    m_Body = new ComponentBasicBody();  addComponent(m_Body);
    m_Body->setPosition(pos);
    m_Body->setScale(size,size,size);	
}
CapsuleEnd::~CapsuleEnd(){
}
void CapsuleEnd::update(const float& dt){
}
CapsuleStar::CapsuleStar(float size,glm::vec3 pos,Scene* scene,bool makeLight):Entity(){
    scene->addEntity(this);
    
    ComponentModel* model = new ComponentModel(Mesh::Plane, ResourceManifest::SunFlareMaterial,this);  addComponent(model);
    model->getModel()->setColor(255,235,206,255);
    
    m_Body = new ComponentBasicBody();  addComponent(m_Body);
    m_Body->setPosition(pos);
    m_Body->setScale(size,size,size);

    m_Light = nullptr;
    if(makeLight){
        m_Light = new PointLight(pos/float(100),scene);
        m_Light->setAttenuation(LightRange::_50);
        m_Light->setColor(255,124,27,255);
    }
}
CapsuleStar::~CapsuleStar(){}
void CapsuleStar::update(const float& dt){
    glm::vec3 pos = m_Body->position();
    m_Body->translate(0,0,(45 * 50 ) * dt,false);
    if(pos.z >= 200 * 225){
        float x = float(((rand() % 200) - 100) / 100.0f) * 3.7f; if(x > 0) x += 1.5f; if(x < 0) x -= 1.5f;
        float y = float(((rand() % 200) - 100) / 100.0f) * 3.7f; if(y > 0) y += 1.5f; if(y < 0) y -= 1.5f;
        m_Body->setPosition( x * 50, y * 50, -200 * 225);
    }
    if(m_Light != nullptr){
        m_Light->setPosition(pos * 0.015f);
        if(glm::distance(m_Light->position(),Resources::getCurrentScene()->getActiveCamera()->getPosition()) > m_Light->getCullingRadius() * 75.0f){
            m_Light->deactivate();
        }
        else{
            m_Light->activate();
        }
    }
    m_Body->setRotation( Resources::getCurrentScene()->getActiveCamera()->getOrientation() );
}

CapsuleTunnel::CapsuleTunnel(float tunnelRadius,Handle& material, Scene* scene):Entity(){
    scene->addEntity(this);
    m_TunnelRadius = tunnelRadius;
    ComponentModel* model = new ComponentModel(ResourceManifest::CapsuleTunnelMesh,material,this);  addComponent(model);
    m_Body = new ComponentBasicBody();  addComponent(m_Body);
    m_Body->setPosition(0.0f,0.0f,0.0f);
    m_Body->setScale(m_TunnelRadius,m_TunnelRadius,m_TunnelRadius);
}
CapsuleTunnel::~CapsuleTunnel(){}

CapsuleRibbon::CapsuleRibbon(float tunnelRadius,Handle& material, Scene* scene):Entity(){
    scene->addEntity(this);
    m_TunnelRadius = tunnelRadius;
    ComponentModel* model = new ComponentModel(ResourceManifest::CapsuleRibbonMesh,material,this);  addComponent(model);
    m_Body = new ComponentBasicBody();  addComponent(m_Body);
    m_Body->setPosition(0.0f,0.0f,0.0f);
    m_Body->setScale(m_TunnelRadius,m_TunnelRadius,m_TunnelRadius);
}
CapsuleRibbon::~CapsuleRibbon(){}
void CapsuleRibbon::bind(){
    Renderer::GLDisable(GLState::DEPTH_TEST);
    Renderer::GLDisable(GLState::DEPTH_MASK);
}
void CapsuleRibbon::unbind(){
    Renderer::GLEnable(GLState::DEPTH_TEST);
    Renderer::GLEnable(GLState::DEPTH_MASK);
}

CapsuleSpace::CapsuleSpace():SolarSystem("CapsuleSpace","NULL"){
    m_Timer = 0;
    setSkybox(nullptr);
    setBackgroundColor(255.0f,0,0);

    this->getActiveCamera()->setNear(0.1f);
    this->getActiveCamera()->setFar(70000000.0f);

    PointLight* l = new PointLight(glm::vec3(0,1.7f,0),this);
    l->setColor(255,200,215,255);
    l->setSpecularIntensity(0.0f);
    l->setAttenuation(LightRange::_32);


    m_TunnelA = new CapsuleTunnel(5000,ResourceManifest::CapsuleA,this);
    m_TunnelB = new CapsuleTunnel(5000,ResourceManifest::CapsuleB,this);
    m_Ribbon = new CapsuleRibbon(5000,ResourceManifest::CapsuleC,this);

    m_FrontEnd = new CapsuleEnd(2250,glm::vec3(0,0,-25000),glm::vec3(1),this);
    m_BackEnd = new CapsuleEnd(1700,glm::vec3(0,0,25000),glm::vec3(0),this);

    m_BackEnd->m_Body->rotate(0.0f,glm::radians(180.0f),0.0f);

    glm::vec3 oldScale = m_TunnelB->m_Body->getScale();
    m_TunnelB->m_Body->setScale(oldScale.x-(0.62f * m_TunnelB->getTunnelRadius()),oldScale.y-(0.62f * m_TunnelB->getTunnelRadius()),oldScale.z);

    m_TunnelA->m_Body->setPosition(0,0,0);
    m_TunnelB->m_Body->setPosition(0,0,0);

    m_Ribbon->m_Body->setPosition(0,300,0);

    float step = -10.0f;
    for(uint i = 0; i < 300; ++i){
        float x = float(((rand() % 200) - 100)/100.0f) * 3.7f; if(x > 0) x += 1.5f; if(x < 0) x -= 1.5f;
        float y = float(((rand() % 200) - 100)/100.0f) * 3.7f; if(y > 0) y += 1.5f; if(y < 0) y -= 1.5f;

        glm::vec3 pos = glm::vec3(x,y,step) * glm::vec3(50);

        bool spawnLight = false;
        if(i % 3 == 0){
            spawnLight = true;
        }
        m_CapsuleStars.push_back(new CapsuleStar(50,pos,this,spawnLight));
        step -= 6.0f;
    }
    //this to just test. should set player / camera dynamically
    Ship* dread = new Ship(ResourceManifest::DreadnaughtMesh,ResourceManifest::DreadnaughtMaterial,true,"Dreadnaught",glm::vec3(0),glm::vec3(1),nullptr,this);
    ComponentRigidBody* playerBody = dread->getComponent<ComponentRigidBody>();

    setPlayer(dread);
    GameCamera* playerCamera = (GameCamera*)this->getActiveCamera();
    playerCamera->follow(dread);

    centerSceneToObject(dread);

    //LightProbe* lp = new LightProbe("CapsuleLightProbe",256,glm::vec3(0.0f),true,this);
    //dread->addChild(lp);
}
CapsuleSpace::~CapsuleSpace(){}
void CapsuleSpace::update(const float& dt){
    m_Timer += dt;

    float aRadius = m_TunnelA->getTunnelRadius();
    float bRadius = m_TunnelB->getTunnelRadius();

    m_TunnelA->m_Body->translate(0,0,(25 * aRadius) * dt,false);
    m_TunnelB->m_Body->translate(0,0,(8 * bRadius) * dt,false);
    m_Ribbon->m_Body->translate(0,0,(7 * aRadius) * dt,false);

    float tunnelARotRand = float(rand() % 3) + 2;
    float tunnelBRotRand = float(rand() % 2) + 2;

    m_TunnelA->m_Body->rotate(0,0,glm::radians(tunnelARotRand*15.0f)*dt);
    m_TunnelB->m_Body->rotate(0,0,-glm::radians(tunnelBRotRand*15.0f)*dt);
    m_BackEnd->m_Body->rotate(0,0,4.0f*dt);
    m_FrontEnd->m_Body->rotate(0,0,-4.0f*dt);

    glm::vec3 aPos = m_TunnelA->m_Body->position();
    glm::vec3 bPos = m_TunnelB->m_Body->position();
    glm::vec3 rPos = m_Ribbon->m_Body->position();

    if(aPos.z >= 12.112 * aRadius || aPos.z <= -12.112 * aRadius){
        m_TunnelA->m_Body->setPosition(0,0,0);
    }
    if(bPos.z >= 12.112 * bRadius || bPos.z <= -12.112 * bRadius){
        m_TunnelB->m_Body->setPosition(0,0,0);
    }
    if(rPos.z >= 20 * aRadius || rPos.z <= -20 * aRadius){
        m_Ribbon->m_Body->setPosition(0,300,0);
    }
    ComponentRigidBody& body = *getPlayer()->getComponent<ComponentRigidBody>();
    ComponentModel& model = *getPlayer()->getComponent<ComponentModel>();
    body.setPosition(0,0,0);

    float x = glm::sin(m_Timer * 2.4f) * 0.07f;
    float y = glm::cos(m_Timer * 2.4f) * 0.05f;

    float roll  = glm::sin(m_Timer * 2.4f) * 5.0f;
    float pitch = glm::sin(m_Timer * 2.4f) * 3.7f;


    model.getModel()->setPosition(glm::vec3(x * 1.2f, -y ,0));
    model.getModel()->setOrientation(  glm::radians(pitch), 0 , glm::radians(roll)  );


    //double check this (this is the light probe)
    //getPlayer()->getChildren().at(0)->setPosition(glm::vec3(x*1.2f,-y,0));
}
