#include "CapsuleSpace.h"
#include "core/engine/Engine_Resources.h"
#include "core/Material.h"
#include "Ship.h"
#include "GameCamera.h"
#include "core/Light.h"
#include "core/engine/Engine_Renderer.h"
#include "core/engine/Engine_GLStateMachine.h"
#include "core/Texture.h"
#include "GameSkybox.h"
#include "core/Mesh.h"
#include "core/MeshInstance.h"

#include "ResourceManifest.h"

#include <boost/lexical_cast.hpp>

using namespace Engine;
using namespace std;

struct CapsuleEndLogicFunctor final {void operator()(ComponentLogic& _component, const float& dt) const {
    CapsuleEnd& end = *(CapsuleEnd*)_component.getUserPointer();
}};
struct CapsuleStarLogicFunctor final {void operator()(ComponentLogic& _component, const float& dt) const {
    CapsuleStar& star = *(CapsuleStar*)_component.getUserPointer();

    auto& m_Body = *star.m_Entity.getComponent<ComponentBody>();

    glm::vec3 pos = m_Body.position();
    m_Body.translate(0, 0, (45 * 50) * dt, false);
    if (pos.z >= 200 * 225) {
        float x = float(((rand() % 200) - 100) / 100.0f) * 3.7f; if (x > 0) x += 1.5f; if (x < 0) x -= 1.5f;
        float y = float(((rand() % 200) - 100) / 100.0f) * 3.7f; if (y > 0) y += 1.5f; if (y < 0) y -= 1.5f;
        m_Body.setPosition(x * 50, y * 50, -200 * 225);
    }
    if (star.m_Light) {
        star.m_Light->setPosition(pos * 0.015f);
        if (glm::distance(star.m_Light->position(), Resources::getCurrentScene()->getActiveCamera()->getPosition()) > star.m_Light->getCullingRadius() * 75.0f) {
            star.m_Light->deactivate();
        }else{
            star.m_Light->activate();
        }
    }
    m_Body.setRotation(Resources::getCurrentScene()->getActiveCamera()->getOrientation());
}};



CapsuleEnd::CapsuleEnd(float size,glm::vec3 pos, glm::vec3 color, SolarSystem* scene):EntityWrapper(*scene){
    ComponentModel* model = m_Entity.addComponent<ComponentModel>(Mesh::Plane, ResourceManifest::CapsuleD);
    model->getModel().setColor(color.x,color.y,color.z,1.0f);
    
    auto* m_Body = m_Entity.addComponent<ComponentBody>();
    m_Body->setPosition(pos);
    m_Body->setScale(size,size,size);	 

    m_Entity.addComponent<ComponentLogic>(CapsuleEndLogicFunctor(), this);
    scene->m_Objects.push_back(this);
}
CapsuleEnd::~CapsuleEnd(){
}
CapsuleStar::CapsuleStar(float size,glm::vec3 pos, SolarSystem* scene,bool makeLight) :EntityWrapper(*scene) {
    ComponentModel* model = m_Entity.addComponent<ComponentModel>(Mesh::Plane, ResourceManifest::StarFlareMaterial);
    model->getModel().setColor(255,235,206,255);
    
    auto* m_Body = m_Entity.addComponent<ComponentBody>();
    m_Body->setPosition(pos);
    m_Body->setScale(size,size,size);

    m_Entity.addComponent<ComponentLogic>(CapsuleStarLogicFunctor(), this);

    m_Light = nullptr;
    if(makeLight){
        m_Light = new PointLight(pos/float(100),scene);
        m_Light->setAttenuation(LightRange::_50);
        m_Light->setColor(1.0f, 0.486f, 0.106f);
        scene->m_Objects.push_back(m_Light);
    }
    scene->m_Objects.push_back(this);
}
CapsuleStar::~CapsuleStar(){}

CapsuleTunnel::CapsuleTunnel(float tunnelRadius,Handle& material, SolarSystem* scene) :EntityWrapper(*scene) {
    m_TunnelRadius = tunnelRadius;
    ComponentModel* model = m_Entity.addComponent<ComponentModel>(ResourceManifest::CapsuleTunnelMesh,material);
    auto* m_Body = m_Entity.addComponent<ComponentBody>();
    m_Body->setPosition(0.0f,0.0f,0.0f);
    m_Body->setScale(m_TunnelRadius,m_TunnelRadius,m_TunnelRadius);  
    scene->m_Objects.push_back(this);
}
CapsuleTunnel::~CapsuleTunnel(){}

struct RibbonBindFunctor {void operator()(EngineResource* r) const {
    Renderer::GLDisable(GLState::DEPTH_TEST);
    Renderer::GLDisable(GLState::DEPTH_MASK);

    epriv::DefaultMeshInstanceBindFunctor()(r);
}};
struct RibbonUnbindFunctor {void operator()(EngineResource* r) const {
    Renderer::GLEnable(GLState::DEPTH_TEST);
    Renderer::GLEnable(GLState::DEPTH_MASK);
}};

CapsuleRibbon::CapsuleRibbon(float tunnelRadius, Handle& mesh,Handle& material, SolarSystem* scene) :EntityWrapper(*scene) {
    m_TunnelRadius = tunnelRadius;
    ComponentModel* model = m_Entity.addComponent<ComponentModel>(mesh,material);

    model->getModel().setCustomBindFunctor(RibbonBindFunctor());
    model->getModel().setCustomUnbindFunctor(RibbonUnbindFunctor());

    auto* m_Body = m_Entity.addComponent<ComponentBody>();
    m_Body->setPosition(0.0f,0.0f,0.0f);
    m_Body->setScale(m_TunnelRadius,m_TunnelRadius,m_TunnelRadius);  
    scene->m_Objects.push_back(this);
}
CapsuleRibbon::~CapsuleRibbon(){}

CapsuleSpace::CapsuleSpace():SolarSystem("CapsuleSpace","NULL"){
    m_Timer = 0;
    setSkybox(nullptr);
    setBackgroundColor(255.0f,0,0);

    this->getActiveCamera()->setNear(0.1f);
    this->getActiveCamera()->setFar(70000000.0f);

    PointLight* l = new PointLight(glm::vec3(0,1.7f,0),this);
    l->setColor(1.0f * 1.3f,0.784f * 1.3f,0.843f * 1.3f);
    l->setSpecularIntensity(0.0f);
    l->setAttenuation(LightRange::_32);
    m_Objects.push_back(l);


    m_TunnelA = new CapsuleTunnel(5000,ResourceManifest::CapsuleA,this);
    m_TunnelB = new CapsuleTunnel(5000,ResourceManifest::CapsuleB,this);
    m_RibbonA = new CapsuleRibbon(5000,ResourceManifest::CapsuleRibbonAMesh,ResourceManifest::CapsuleC,this);
    m_RibbonB = new CapsuleRibbon(5000,ResourceManifest::CapsuleRibbonBMesh,ResourceManifest::CapsuleC,this);

    m_FrontEnd = new CapsuleEnd(2250,glm::vec3(0,0,-25000),glm::vec3(1),this);
    m_BackEnd = new CapsuleEnd(1700,glm::vec3(0,0,25000),glm::vec3(0),this);

    auto& backBody = *(m_BackEnd->m_Entity.getComponent<ComponentBody>());
    auto& tunnelBBody = *(m_TunnelB->m_Entity.getComponent<ComponentBody>());
    auto& tunnelABody = *(m_TunnelA->m_Entity.getComponent<ComponentBody>());
    auto& ribbonBBody = *(m_RibbonB->m_Entity.getComponent<ComponentBody>());
    auto& ribbonABody = *(m_RibbonA->m_Entity.getComponent<ComponentBody>());

    backBody.rotate(0.0f,glm::radians(180.0f),0.0f);

    glm::vec3 oldScale = tunnelBBody.getScale();
    tunnelBBody.setScale(oldScale.x-(0.62f * m_TunnelB->getTunnelRadius()),oldScale.y-(0.62f * m_TunnelB->getTunnelRadius()),oldScale.z);

    tunnelABody.setPosition(0,0,0);
    tunnelBBody.setPosition(0,0,0);

    ribbonBBody.setPosition(0,300,0);
    ribbonABody.setPosition(0, 300, 0);

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
    Ship* dread = new Ship(ResourceManifest::DreadnaughtMesh,ResourceManifest::DreadnaughtMaterial,true,"Dreadnaught",glm::vec3(0),glm::vec3(1),CollisionType::None,this);
    ComponentBody* playerBody = dread->entity().getComponent<ComponentBody>();

    setPlayer(dread);
    GameCamera* playerCamera = (GameCamera*)getActiveCamera();
    playerCamera->follow(dread->entity());

    centerSceneToObject(dread->entity());

    //LightProbe* lp = new LightProbe("CapsuleLightProbe",256,glm::vec3(0.0f),true,this);
    //dread->addChild(lp);
}
CapsuleSpace::~CapsuleSpace(){}
void CapsuleSpace::update(const float& dt){
    m_Timer += dt;

    float aRadius = m_TunnelA->getTunnelRadius();
    float bRadius = m_TunnelB->getTunnelRadius();

    auto& frontBody = *(m_FrontEnd->m_Entity.getComponent<ComponentBody>());
    auto& backBody = *(m_BackEnd->m_Entity.getComponent<ComponentBody>());
    auto& tunnelBBody = *(m_TunnelB->m_Entity.getComponent<ComponentBody>());
    auto& tunnelABody = *(m_TunnelA->m_Entity.getComponent<ComponentBody>());
    auto& ribbonBBody = *(m_RibbonB->m_Entity.getComponent<ComponentBody>());
    auto& ribbonABody = *(m_RibbonA->m_Entity.getComponent<ComponentBody>());


    tunnelABody.translate(0,0,(25 * aRadius) * dt,false);
    tunnelBBody.translate(0,0,(8 * bRadius) * dt,false);
    ribbonABody.translate(0,0,(7 * aRadius) * dt,false);
    ribbonBBody.translate(0, 0, (7 * aRadius) * dt, false);

    float tunnelARotRand = float(rand() % 3) + 2;
    float tunnelBRotRand = float(rand() % 2) + 2;

    tunnelABody.rotate(0,0,glm::radians(tunnelARotRand*15.0f)*dt);
    tunnelBBody.rotate(0,0,-glm::radians(tunnelBRotRand*15.0f)*dt);
    backBody.rotate(0,0,4.0f*dt);
    frontBody.rotate(0,0,-4.0f*dt);

    glm::vec3 aPos = tunnelABody.position();
    glm::vec3 bPos = tunnelBBody.position();
    glm::vec3 rPosA = ribbonABody.position();
    glm::vec3 rPosB = ribbonBBody.position();

    if(aPos.z >= 12.112 * aRadius || aPos.z <= -12.112 * aRadius){
        tunnelABody.setPosition(0,0,0);
    }
    if(bPos.z >= 12.112 * bRadius || bPos.z <= -12.112 * bRadius){
        tunnelBBody.setPosition(0,0,0);
    }
    if(rPosA.z >= 20 * aRadius || rPosA.z <= -20 * aRadius){
        ribbonABody.setPosition(0,300,0);
    }
    if (rPosB.z >= 20 * aRadius || rPosB.z <= -20 * aRadius) {
        ribbonBBody.setPosition(0, 300, 0);
    }
    auto& playerEntity = getPlayer()->entity();
    epriv::EntitySerialization _s(playerEntity);

    ComponentBody& body = *playerEntity.getComponent<ComponentBody>(_s);
    ComponentModel& model = *playerEntity.getComponent<ComponentModel>(_s);
    body.setPosition(0,0,0);

    float x = glm::sin(m_Timer * 2.4f) * 0.07f;
    float y = glm::cos(m_Timer * 2.4f) * 0.05f;

    float roll  = glm::sin(m_Timer * 2.4f) * 5.0f;
    float pitch = glm::sin(m_Timer * 2.4f) * 3.7f;

    model.getModel().setPosition(glm::vec3(x * 1.2f, -y ,0));
    model.getModel().setOrientation(  glm::radians(pitch), 0 , glm::radians(roll)  );

    //double check this (this is the light probe)
    //getPlayer()->getChildren()[0]->setPosition(glm::vec3(x*1.2f,-y,0));
}
