#include "CapsuleSpace.h"
#include "ResourceManifest.h"
#include "Ship.h"
#include "GameCamera.h"
#include "GameSkybox.h"

#include <core/engine/resources/Engine_Resources.h>
#include <core/Material.h>
#include <core/engine/lights/Light.h>
#include <core/engine/renderer/Engine_Renderer.h>
#include <core/engine/Engine_GLStateMachine.h>
#include <core/engine/textures/Texture.h>
#include <core/engine/mesh/Mesh.h>
#include <core/MeshInstance.h>

#include <boost/lexical_cast.hpp>

using namespace Engine;
using namespace std;

struct CapsuleEndLogicFunctor final {void operator()(ComponentLogic& _component, const float& dt) const {
    //CapsuleEnd& end = *(CapsuleEnd*)_component.getUserPointer();
}};
struct CapsuleStarLogicFunctor final {void operator()(ComponentLogic& _component, const float& dt) const {
    CapsuleStar& star = *(CapsuleStar*)_component.getUserPointer();
    auto& m_Body = *star.m_Entity.getComponent<ComponentBody>();
    auto& activeCamera = *Resources::getCurrentScene()->getActiveCamera();
    m_Body.setRotation(activeCamera.getOrientation());
    if (Engine::paused()) return;

    const auto& pos = m_Body.position();
    m_Body.translate(0, 0, (45 * 50) * dt, false);
    if (pos.z >= 200 * 225) {
        float x = float(((rand() % 200) - 100) / 100.0f) * 3.7f; if (x > 0) x += 1.5f; if (x < 0) x -= 1.5f;
        float y = float(((rand() % 200) - 100) / 100.0f) * 3.7f; if (y > 0) y += 1.5f; if (y < 0) y -= 1.5f;
        m_Body.setPosition(x * 50, y * 50, -200 * 225);
    }
    if (star.m_Light) {
        auto& light = *star.m_Light;
        light.setPosition(pos * 0.015f);
        if (glm::distance(light.position(), activeCamera.getPosition()) > light.getCullingRadius() * 75.0f) {
            light.deactivate();
        }else{
            light.activate();
        }
    } 
}};

CapsuleEnd::CapsuleEnd(float size,glm::vec3 pos, glm::vec3 color, CapsuleSpace* scene):EntityWrapper(*scene){
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
CapsuleStar::CapsuleStar(float size,glm::vec3 pos, CapsuleSpace* scene,bool makeLight) :EntityWrapper(*scene) {
    ComponentModel* model = m_Entity.addComponent<ComponentModel>(Mesh::Plane, ResourceManifest::StarFlareMaterial);
    model->getModel().setColor(1.0f,0.92156f,0.8078f,1.0f);
    
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
CapsuleStar::~CapsuleStar(){
}
CapsuleTunnel::CapsuleTunnel(float tunnelRadius,Handle& material, CapsuleSpace* scene) :EntityWrapper(*scene) {
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

CapsuleRibbon::CapsuleRibbon(float tunnelRadius, Handle& mesh,Handle& material, CapsuleSpace* scene) :EntityWrapper(*scene) {
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

CapsuleSpace::CapsuleSpace():Scene("CapsuleSpace"){
    GameCamera* playerCamera = new GameCamera(0.1f, 70000000.0f, this);
    setActiveCamera(*playerCamera);
    m_Objects.push_back(playerCamera);

    m_Timer = 0;
    setSkybox(nullptr);
    setBackgroundColor(255.0f,0,0);

    PointLight* l = new PointLight(glm::vec3(0, 1.7f, 0), this);
    l->setColor(1.0f * 2.5f, 0.784f * 2.5f, 0.843f * 2.5f);
    l->setSpecularIntensity(0.0f);
    l->setAttenuation(LightRange::_32);
    m_Objects.push_back(l);

    m_TunnelA = new CapsuleTunnel(5000,ResourceManifest::CapsuleA,this);
    m_TunnelB = new CapsuleTunnel(5000,ResourceManifest::CapsuleB,this);
    m_RibbonA = new CapsuleRibbon(5000,ResourceManifest::CapsuleRibbonMeshA,ResourceManifest::CapsuleC,this);
    m_RibbonB = new CapsuleRibbon(5000, ResourceManifest::CapsuleRibbonMeshB, ResourceManifest::CapsuleC, this);

    m_FrontEnd = new CapsuleEnd(2250,glm::vec3(0,0,-25000),glm::vec3(1),this);
    m_BackEnd = new CapsuleEnd(1700,glm::vec3(0,0,25000),glm::vec3(0),this);

    auto& backBody = *(m_BackEnd->m_Entity.getComponent<ComponentBody>());
    auto& tunnelBBody = *(m_TunnelB->m_Entity.getComponent<ComponentBody>());
    auto& tunnelABody = *(m_TunnelA->m_Entity.getComponent<ComponentBody>());
    auto& ribbonBodyA = *(m_RibbonA->m_Entity.getComponent<ComponentBody>());
    auto& ribbonBodyB = *(m_RibbonB->m_Entity.getComponent<ComponentBody>());

    backBody.rotate(0.0f,glm::radians(180.0f),0.0f);

    glm::vec3 oldScale = tunnelBBody.getScale();
    tunnelBBody.setScale(oldScale.x-(0.62f * m_TunnelB->getTunnelRadius()),oldScale.y-(0.62f * m_TunnelB->getTunnelRadius()),oldScale.z);

    tunnelABody.setPosition(0,0,0);
    tunnelBBody.setPosition(0,0,0);

    ribbonBodyA.setPosition(0, 300, 0);
    ribbonBodyB.setPosition(0, 300, 0);

    float step = -10.0f;
    for(uint i = 0; i < 300; ++i){
        float x = float(((rand() % 200) - 100)/100.0f) * 3.7f; if(x > 0) x += 1.5f; if(x < 0) x -= 1.5f;
        float y = float(((rand() % 200) - 100)/100.0f) * 3.7f; if(y > 0) y += 1.5f; if(y < 0) y -= 1.5f;

        glm::vec3 pos = glm::vec3(x,y,step) * glm::vec3(50);

        bool spawnLight = false;
        if(i % 2 == 0){
            spawnLight = true;
        }
        m_CapsuleStars.push_back(new CapsuleStar(50,pos,this,spawnLight));
        step -= 6.0f;
    }
    Ship* d = new Ship(ResourceManifest::DreadnaughtMesh,ResourceManifest::DreadnaughtMaterial,true,"Dreadnaught",glm::vec3(0),glm::vec3(1),CollisionType::None,this);
    ComponentBody* playerBody = d->entity().getComponent<ComponentBody>();

    setPlayer(d);
    playerCamera->follow(d->entity());
    centerSceneToObject(d->entity());
}
CapsuleSpace::~CapsuleSpace(){
    SAFE_DELETE_VECTOR(m_Objects);
}
void CapsuleSpace::update(const float& dt){
    if (Engine::paused()) return;

    m_Timer += dt;

    float aRadius = m_TunnelA->getTunnelRadius();
    float bRadius = m_TunnelB->getTunnelRadius();

    auto& frontBody = *(m_FrontEnd->m_Entity.getComponent<ComponentBody>());
    auto& backBody = *(m_BackEnd->m_Entity.getComponent<ComponentBody>());
    auto& tunnelBBody = *(m_TunnelB->m_Entity.getComponent<ComponentBody>());
    auto& tunnelABody = *(m_TunnelA->m_Entity.getComponent<ComponentBody>());
    auto& ribbonBodyA = *(m_RibbonA->m_Entity.getComponent<ComponentBody>());
    auto& ribbonBodyB = *(m_RibbonB->m_Entity.getComponent<ComponentBody>());

    tunnelABody.translate(0, 0, (25 * aRadius) * dt, false);
    tunnelBBody.translate(0, 0, (8 * bRadius) * dt, false);
    ribbonBodyA.translate(0, 0, (7 * aRadius) * dt, false);
    ribbonBodyB.translate(0, 0, (7 * aRadius) * dt, false);

    float tunnelARotRand = float(rand() % 3) + 2;
    float tunnelBRotRand = float(rand() % 2) + 2;

    tunnelABody.rotate(0,0,glm::radians(tunnelARotRand*15.0f)*dt);
    tunnelBBody.rotate(0,0,-glm::radians(tunnelBRotRand*15.0f)*dt);
    backBody.rotate(0,0,4.0f*dt);
    frontBody.rotate(0,0,-4.0f*dt);

    const auto& aPos = tunnelABody.position();
    const auto& bPos = tunnelBBody.position();
    const auto& rPosA = ribbonBodyA.position();
    const auto& rPosB = ribbonBodyB.position();

    if (aPos.z >= 12.112 * aRadius || aPos.z <= -12.112 * aRadius) {
        tunnelABody.setPosition(0, 0, 0);
    }
    if (bPos.z >= 12.112 * bRadius || bPos.z <= -12.112 * bRadius) {
        tunnelBBody.setPosition(0, 0, 0);
    }
    if (rPosA.z >= 20 * aRadius || rPosA.z <= -20 * aRadius) {
        ribbonBodyA.setPosition(0, 300, 0);
    }
    if (rPosB.z >= 20 * aRadius || rPosB.z <= -20 * aRadius) {
        ribbonBodyB.setPosition(0, 300, 0);
    }
    auto& playerEntity = getPlayer()->entity();
    epriv::EntitySerialization _s(playerEntity);
    ComponentBody& body = *playerEntity.getComponent<ComponentBody>(_s);
    ComponentModel& model = *playerEntity.getComponent<ComponentModel>(_s);
    body.setPosition(0,0,0);

    const auto& sine = glm::sin(m_Timer * 2.4f);
    const auto& cose = glm::cos(m_Timer * 2.4f);

    float x = sine * 0.07f;
    float y = cose * 0.05f;

    float roll  = glm::radians(sine * 5.0f);
    float pitch = glm::radians(sine * 3.7f);

    model.getModel().setPosition(x * 1.2f, -y ,0);
    model.getModel().setOrientation(pitch, 0, roll);
}
