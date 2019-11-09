#include "Ship.h"
#include "GameCamera.h"
#include "map/Map.h"
#include "networking/Packet.h"
#include "Helper.h"
#include "map/Anchor.h"
#include "ResourceManifest.h"
#include "ships/Ships.h"
#include "ai/AI.h"
#include "ai/FireAtWill.h"

#include <core/engine/mesh/Mesh.h>
#include <core/engine/Engine.h>
#include <core/engine/math/Engine_Math.h>
#include <core/engine/lights/Lights.h>
#include <core/engine/materials/Material.h>
#include <core/engine/renderer/Decal.h>
#include <core/engine/renderer/Particle.h>
#include <core/engine/scene/Skybox.h>
#include <core/engine/textures/Texture.h>

#include <core/engine/utils/Engine_Debugging.h>

#include <BulletCollision/CollisionShapes/btCollisionShape.h>

#include <ecs/ComponentName.h>

#include "ships/shipSystems/ShipSystemCloakingDevice.h"
#include "ships/shipSystems/ShipSystemMainThrusters.h"
#include "ships/shipSystems/ShipSystemPitchThrusters.h"
#include "ships/shipSystems/ShipSystemReactor.h"
#include "ships/shipSystems/ShipSystemRollThrusters.h"
#include "ships/shipSystems/ShipSystemSensors.h"
#include "ships/shipSystems/ShipSystemShields.h"
#include "ships/shipSystems/ShipSystemWarpDrive.h"
#include "ships/shipSystems/ShipSystemYawThrusters.h"
#include "ships/shipSystems/ShipSystemWeapons.h"
#include "ships/shipSystems/ShipSystemHull.h"

#include "hud/SensorStatusDisplay.h"
#include "hud/HUD.h"

#include <glm/gtx/norm.hpp>


using namespace Engine;
using namespace std;


void ShipModelInstanceBindFunctor::operator()(EngineResource* r) const {
    auto& i = *static_cast<ModelInstance*>(r);
    const auto& stage = i.stage();
    auto& scene = *Resources::getCurrentScene();
    Camera& cam = *scene.getActiveCamera();
    glm::vec3 camPos = cam.getPosition();
    Entity& parent = i.parent();
    auto& body = *(parent.getComponent<ComponentBody>());
    glm::mat4 parentModel = body.modelMatrixRendering();

    Renderer::sendUniform4Safe("Object_Color", i.color());
    Renderer::sendUniform3Safe("Gods_Rays_Color", i.godRaysColor());

    if (stage == RenderStage::ForwardTransparentTrianglesSorted || stage == RenderStage::ForwardTransparent || stage == RenderStage::ForwardOpaque) {
        auto& lights = epriv::InternalScenePublicInterface::GetLights(scene);
        int maxLights = glm::min(static_cast<int>(lights.size()), MAX_LIGHTS_PER_PASS);
        Renderer::sendUniform1Safe("numLights", maxLights);
        for (int i = 0; i < maxLights; ++i) {
            auto& light = *lights[i];
            const auto& lightType = light.type();
            const auto start = "light[" + to_string(i) + "].";
            switch (lightType) {
                case LightType::Sun: {
                    SunLight& s = static_cast<SunLight&>(light);
                    auto& body = *s.getComponent<ComponentBody>();
                    const glm::vec3& pos = body.position();
                    Renderer::sendUniform4Safe((start + "DataA").c_str(), s.getAmbientIntensity(), s.getDiffuseIntensity(), s.getSpecularIntensity(), 0.0f);
                    Renderer::sendUniform4Safe((start + "DataC").c_str(), 0.0f, pos.x, pos.y, pos.z);
                    Renderer::sendUniform4Safe((start + "DataD").c_str(), s.color().x, s.color().y, s.color().z, static_cast<float>(lightType));
                    break;
                }case LightType::Directional: {
                    DirectionalLight& d = static_cast<DirectionalLight&>(light);
                    auto& body = *d.getComponent<ComponentBody>();
                    const glm::vec3& _forward = body.forward();
                    Renderer::sendUniform4Safe((start + "DataA").c_str(), d.getAmbientIntensity(), d.getDiffuseIntensity(), d.getSpecularIntensity(), _forward.x);
                    Renderer::sendUniform4Safe((start + "DataB").c_str(), _forward.y, _forward.z, 0.0f, 0.0f);
                    Renderer::sendUniform4Safe((start + "DataD").c_str(), d.color().x, d.color().y, d.color().z, static_cast<float>(lightType));
                    break;
                }case LightType::Point: {
                    PointLight& p = static_cast<PointLight&>(light);
                    auto& body = *p.getComponent<ComponentBody>();
                    const glm::vec3& pos = body.position();
                    Renderer::sendUniform4Safe((start + "DataA").c_str(), p.getAmbientIntensity(), p.getDiffuseIntensity(), p.getSpecularIntensity(), 0.0f);
                    Renderer::sendUniform4Safe((start + "DataB").c_str(), 0.0f, 0.0f, p.getConstant(), p.getLinear());
                    Renderer::sendUniform4Safe((start + "DataC").c_str(), p.getExponent(), pos.x, pos.y, pos.z);
                    Renderer::sendUniform4Safe((start + "DataD").c_str(), p.color().x, p.color().y, p.color().z, static_cast<float>(lightType));
                    Renderer::sendUniform4Safe((start + "DataE").c_str(), 0.0f, 0.0f, static_cast<float>(p.getAttenuationModel()), 0.0f);
                    break;
                }case LightType::Spot: {
                    SpotLight& s = static_cast<SpotLight&>(light);
                    auto& body = *s.getComponent<ComponentBody>();
                    const glm::vec3& pos = body.position();
                    const glm::vec3 _forward = body.forward();
                    Renderer::sendUniform4Safe((start + "DataA").c_str(), s.getAmbientIntensity(), s.getDiffuseIntensity(), s.getSpecularIntensity(), _forward.x);
                    Renderer::sendUniform4Safe((start + "DataB").c_str(), _forward.y, _forward.z, s.getConstant(), s.getLinear());
                    Renderer::sendUniform4Safe((start + "DataC").c_str(), s.getExponent(), pos.x, pos.y, pos.z);
                    Renderer::sendUniform4Safe((start + "DataD").c_str(), s.color().x, s.color().y, s.color().z, static_cast<float>(lightType));
                    Renderer::sendUniform4Safe((start + "DataE").c_str(), s.getCutoff(), s.getCutoffOuter(), static_cast<float>(s.getAttenuationModel()), 0.0f);
                    break;
                }case LightType::Rod: {
                    RodLight& r = static_cast<RodLight&>(light);
                    auto& body = *r.getComponent<ComponentBody>();
                    const glm::vec3& pos = body.position();
                    const float cullingDistance = r.rodLength() + (r.getCullingRadius() * 2.0f);
                    const float half = r.rodLength() / 2.0f;
                    const glm::vec3& firstEndPt = pos + (glm::vec3(body.forward()) * half);
                    const glm::vec3& secndEndPt = pos - (glm::vec3(body.forward()) * half);
                    Renderer::sendUniform4Safe((start + "DataA").c_str(), r.getAmbientIntensity(), r.getDiffuseIntensity(), r.getSpecularIntensity(), firstEndPt.x);
                    Renderer::sendUniform4Safe((start + "DataB").c_str(), firstEndPt.y, firstEndPt.z, r.getConstant(), r.getLinear());
                    Renderer::sendUniform4Safe((start + "DataC").c_str(), r.getExponent(), secndEndPt.x, secndEndPt.y, secndEndPt.z);
                    Renderer::sendUniform4Safe((start + "DataD").c_str(), r.color().x, r.color().y, r.color().z, static_cast<float>(lightType));
                    Renderer::sendUniform4Safe((start + "DataE").c_str(), r.rodLength(), 0.0f, static_cast<float>(r.getAttenuationModel()), 0.0f);
                    break;
                }default: {
                    break;
                }
            }
        }
        Skybox* skybox = scene.skybox();
        Renderer::sendUniform4Safe("ScreenData", epriv::Core::m_Engine->m_RenderManager._getGIPackedData(), Renderer::Settings::getGamma(), 0.0f, 0.0f);
        auto maxTextures = epriv::Core::m_Engine->m_RenderManager.OpenGLStateMachine.getMaxTextureUnits() - 1;
        if (skybox && skybox->texture()->numAddresses() >= 3) {
            Renderer::sendTextureSafe("irradianceMap", skybox->texture()->address(1), maxTextures - 2, GL_TEXTURE_CUBE_MAP);
            Renderer::sendTextureSafe("prefilterMap", skybox->texture()->address(2), maxTextures - 1, GL_TEXTURE_CUBE_MAP);
            Renderer::sendTextureSafe("brdfLUT", *Texture::BRDF, maxTextures);
        }else{
            Renderer::sendTextureSafe("irradianceMap", Texture::Black->address(0), maxTextures - 2, GL_TEXTURE_2D);
            Renderer::sendTextureSafe("prefilterMap", Texture::Black->address(0), maxTextures - 1, GL_TEXTURE_2D);
            Renderer::sendTextureSafe("brdfLUT", *Texture::BRDF, maxTextures);
        }
    }
    Renderer::sendUniform1Safe("AnimationPlaying", 0);
    glm::mat4 modelMatrix = parentModel * i.modelMatrix();

    glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(modelMatrix)));

    Renderer::sendUniformMatrix4Safe("Model", modelMatrix);
    Renderer::sendUniformMatrix3Safe("NormalMatrix", normalMatrix);
};
void ShipModelInstanceUnbindFunctor::operator()(EngineResource* r) const {
    //auto& i = *static_cast<ModelInstance*>(r);
};



struct ShipLogicFunctor final {void operator()(ComponentLogic& _component, const double& dt) const {
    Ship& ship = *static_cast<Ship*>(_component.getUserPointer());
    Map& map = static_cast<Map&>(ship.entity().scene());

    for (auto& shipSystem : ship.m_ShipSystems) {
        if (shipSystem.second) { //some ships wont have all the systems (cloaking device, etc)
            shipSystem.second->update(dt);
        }
    }

    if (ship.IsPlayer()) {
        #pragma region PlayerFlightControls
        auto* mytarget = ship.getTarget();
        if (!Engine::paused()) {
            if (ship.m_IsWarping && ship.m_WarpFactor > 0) {
                auto speed = ship.getWarpSpeedVector3() * static_cast<decimal>(dt);
                for (auto& pod : epriv::InternalScenePublicInterface::GetEntities(map)) {
                    Entity e = map.getEntity(pod);
                    const EntityDataRequest dataRequest(e);
                    auto* cam = e.getComponent<ComponentCamera>(dataRequest);
                    //TODO: parent->child relationship
                    if (e != ship.m_Entity && !cam && (e != map.getHUD().getSensorDisplay().radarRingEntity() && e != map.getHUD().getSensorDisplay().radarCameraEntity())) {
                        auto _otherBody = e.getComponent<ComponentBody>(dataRequest);
                        if (_otherBody) {
                            auto& otherBody = *_otherBody;
                            otherBody.setPosition(otherBody.position() + speed);
                        }
                    }
                }
                for (auto& particle : epriv::InternalScenePublicInterface::GetParticles(map)) {
                    particle.setPosition(particle.position() + glm::vec3(speed));
                }
            }
        }
        #pragma endregion

        #pragma region PlayerCameraControls
        auto& camera = *ship.m_PlayerCamera;
        const auto& cameraState = camera.getState();
        const auto* target = camera.getTarget();
        if (Engine::isKeyDownOnce(KeyboardKey::F1)) {
            camera.setState(CameraState::Cockpit);
        }else if (Engine::isKeyDownOnce(KeyboardKey::F3)) {
            camera.setState(CameraState::Orbit);
        }else if (Engine::isKeyDownOnce(KeyboardKey::F2)) {
            camera.setState(CameraState::FollowTarget);
        }
        #pragma endregion

        if (Engine::isKeyDownOnce(KeyboardKey::T) && map.name() != "Menu") {
            Entity scan = camera.getObjectInCenterRay(ship.m_Entity);
            if (!scan.null()) {
                auto* componentName = scan.getComponent<ComponentName>();
                if (componentName) {
                    EntityWrapper* scannedTarget = nullptr;
                    for (auto& obj : map.m_Objects) {
                        auto* componentName1 = obj->getComponent<ComponentName>();
                        if (componentName1 && componentName1->name() == componentName->name()) {
                            scannedTarget = obj;
                            break;
                        }
                    }
                    ship.setTarget(scannedTarget, true);
                }
            }
        }
        for (auto& shipSystem : ship.m_ShipSystems) {
            if (shipSystem.second) {
                shipSystem.second->render();
            }
        }
    }

    if (ship.m_AI) {
        auto& ai = *ship.m_AI;
        auto* fire_at_will = ai.getFireAtWill();
        if (ship.IsPlayer() && fire_at_will) {
            if (Engine::isKeyDownOnce(KeyboardKey::G)) {
                fire_at_will->toggle();
            }
        }

        ai.update(dt);
    }

    for (auto& decal : ship.m_DamageDecals) {
        auto& _decal = *decal;
        auto& shipBody = *ship.getComponent<ComponentBody>();
        const auto shipRotation = shipBody.rotation();
        _decal.setPosition(shipBody.position() + (shipRotation * _decal.initialPosition()));
        _decal.setRotation(shipRotation * _decal.initialRotation());
        _decal.update(dt);
        if (!_decal.active()) {
            removeFromVector(ship.m_DamageDecals, decal); //might be dangerous
        }
    }
}};

//TODO: move to the hull system?
struct HullCollisionFunctor final { void operator()(ComponentBody& owner, const glm::vec3& ownerHit, ComponentBody& other, const glm::vec3& otherHit, const glm::vec3& normal) const {
    auto ownerShipVoid = owner.getUserPointer1();
    if (ownerShipVoid) {
        auto otherShipVoid = other.getUserPointer1();
        if (otherShipVoid && ownerShipVoid != otherShipVoid) { //redundant?
            if (owner.getCollisionGroup() == CollisionFilter::_Custom_4 && other.getCollisionGroup() == CollisionFilter::_Custom_4) { //hull on hull only
                auto* ownerShip = static_cast<Ship*>(ownerShipVoid);
                auto* otherShip = static_cast<Ship*>(otherShipVoid);

                if (ownerShip->IsPlayer() /*|| otherShip->IsPlayer()*/) {

                    auto* ownerHull = static_cast<ShipSystemHull*>(ownerShip->getShipSystem(ShipSystemType::Hull));
                    auto* otherHull = static_cast<ShipSystemHull*>(otherShip->getShipSystem(ShipSystemType::Hull));
                    if (ownerHull && otherHull) {
                        const auto ownerMass = owner.mass() * 3000.0f;
                        const auto otherMass = other.mass() * 3000.0f;
                        const auto massTotal = ownerMass + otherMass;

                        const auto ownerLocal = ownerHit - glm::vec3(owner.position());
                        const auto otherLocal = otherHit - glm::vec3(other.position());

                        const auto owner_linear_velocity = ownerShip->getLinearVelocity();
                        const auto other_linear_velocity = otherShip->getLinearVelocity();

                        const auto ownerMomentum = ownerMass * glm::vec3(owner_linear_velocity);
                        const auto otherMomentum = otherMass * glm::vec3(other_linear_velocity);
                        const auto totalMomentum = ownerMomentum + otherMomentum;

                        const auto damageTotal1 = (ownerMass / massTotal) * totalMomentum;
                        const auto damageTotal2 = (otherMass / massTotal) * totalMomentum;

                        const auto damageRadiusOwner = 4.0f;
                        const auto damageRadiusOther = 4.0f;

                        PacketCollisionEvent pOut;
                        pOut.PacketType = PacketType::Client_To_Server_Collision_Event;
                        pOut.damage1 = glm::length(damageTotal2);
                        pOut.damage2 = glm::length(damageTotal1);
                        pOut.data =        ownerShip->getMapKey();
                        pOut.data += "," + otherShip->getMapKey();
                        
                        const auto owner_angular_velocity = owner.getAngularVelocity();
                        const auto other_angular_velocity = other.getAngularVelocity();

                        Math::Float16From32(&pOut.ax1, owner_angular_velocity.x);
                        Math::Float16From32(&pOut.ay1, owner_angular_velocity.y);
                        Math::Float16From32(&pOut.az1, owner_angular_velocity.z);

                        Math::Float16From32(&pOut.ax2, other_angular_velocity.x);
                        Math::Float16From32(&pOut.ay2, other_angular_velocity.y);
                        Math::Float16From32(&pOut.az2, other_angular_velocity.z);


                        Math::Float16From32(&pOut.lx1, owner_linear_velocity.x);
                        Math::Float16From32(&pOut.ly1, owner_linear_velocity.y);
                        Math::Float16From32(&pOut.lz1, owner_linear_velocity.z);

                        Math::Float16From32(&pOut.lx2, other_linear_velocity.x);
                        Math::Float16From32(&pOut.ly2, other_linear_velocity.y);
                        Math::Float16From32(&pOut.lz2, other_linear_velocity.z);

                        ownerShip->m_Client.send(pOut);

                        ownerHull->receiveCollisionVisual(normal, ownerLocal, damageRadiusOwner);
                        otherHull->receiveCollisionVisual(normal, otherLocal, damageRadiusOther);
                    }
                }
            }
        }
    }
}};

Ship::Ship(Team& team, Client& client, const string& shipClass, Map& map, const AIType::Type ai_type, const string& name, const glm_vec3 pos, const glm_vec3 scl, CollisionType::Type collisionType, const glm::vec3 aimPosDefault, const glm::vec3 camOffsetDefault):EntityWrapper(map),m_Client(client),m_Team(team){
    m_WarpFactor          = 0;
    m_AI                  = new AI(ai_type);
    m_ShipClass           = shipClass;
    m_IsWarping           = false;
    m_PlayerCamera        = nullptr;
    m_MouseFactor         = glm::dvec2(0.0);
    m_AimPositionDefaults.push_back(aimPosDefault);
    m_CameraOffsetDefault = camOffsetDefault;

    auto& shipInfo = Ships::Database[shipClass];
    auto& modelComponent     = *addComponent<ComponentModel>(shipInfo.MeshHandles[0], shipInfo.MaterialHandles[0], ResourceManifest::ShipShaderProgramDeferred, RenderStage::GeometryOpaque);
    auto& body               = *addComponent<ComponentBody>(collisionType);
    auto& nameComponent      = *addComponent<ComponentName>(name);
    auto& logicComponent     = *addComponent<ComponentLogic>(ShipLogicFunctor(), this);

    setModel(shipInfo.MeshHandles[0]);

    body.setDamping(static_cast<decimal>(0.01), static_cast<decimal>(0.2));
    body.getBtBody().setActivationState(DISABLE_DEACTIVATION);//this might be dangerous...
    body.setPosition(pos);
    body.setScale(scl);

    //the body is using a convex hull for ship to ship ramming
    body.setCollisionGroup(CollisionFilter::_Custom_4); //i belong to ramming hull group (group 4)
    body.setCollisionMask(CollisionFilter::_Custom_4); //i should only collide with other ramming hulls only
    body.setCollisionFunctor(HullCollisionFunctor());

	if (IsPlayer()) {
		m_PlayerCamera = static_cast<GameCamera*>(map.getActiveCamera());
	}
    body.setUserPointer1(this);
    body.setUserPointer2(&client);
    
    m_MapKey = name;
    uint c = 1;
    while (map.getShips().count(m_MapKey)) {
        m_MapKey = name + "_" + to_string(c);
        ++c;
    }

    map.m_Objects.push_back(this);
    map.getShips().emplace(m_MapKey, this);
    if (ai_type == AIType::Player_Other || ai_type == AIType::Player_You) {
        map.getShipsPlayerControlled().emplace(m_MapKey, this);
    }else{
        map.getShipsNPCControlled().emplace(m_MapKey, this);
    }

    registerEvent(EventType::WindowResized);

    //derived classes need to add their own ship systems
    modelComponent.setCustomBindFunctor(ShipModelInstanceBindFunctor());
    modelComponent.setCustomUnbindFunctor(ShipModelInstanceUnbindFunctor());
}
Ship::~Ship(){
    unregisterEvent(EventType::WindowResized);
    SAFE_DELETE(m_AI);
    SAFE_DELETE_VECTOR(m_DamageDecals);
	SAFE_DELETE_MAP(m_ShipSystems);
}
const string& Ship::getMapKey() const {
    return m_MapKey;
}

void Ship::addHullTargetPoints(vector<glm::vec3>& points) {
    for (auto& pt : points) {
        m_AimPositionDefaults.push_back(pt);
    }
}
const glm::vec3 Ship::getAimPositionDefault() {
    if (m_AimPositionDefaults.size() == 0) {
        return getPosition();
    }
    auto& body = *getComponent<ComponentBody>();
    return body.position() + Math::rotate_vec3(body.rotation(), m_AimPositionDefaults[0]);
}
const glm::vec3 Ship::getAimPositionRandom() {
    if (m_AimPositionDefaults.size() == 0) {
        return getPosition();
    }
    auto& body = *getComponent<ComponentBody>();
    if (m_AimPositionDefaults.size() == 1) {
        return body.position() + Math::rotate_vec3(body.rotation(), m_AimPositionDefaults[0]);
    }
    const auto randIndex = Helper::GetRandomIntFromTo(0, static_cast<int>(m_AimPositionDefaults.size()) - 1);
    return body.position() + Math::rotate_vec3(body.rotation(), m_AimPositionDefaults[randIndex]);
}
const glm::vec3 Ship::getAimPositionDefaultLocal() {
    if (m_AimPositionDefaults.size() == 0) {
        return glm::vec3(0.0f);
    }
    auto& body = *getComponent<ComponentBody>();
    if (m_AimPositionDefaults.size() == 1) {
        return Math::rotate_vec3(body.rotation(), m_AimPositionDefaults[0]);
    }
    return Math::rotate_vec3(body.rotation(), m_AimPositionDefaults[0]);
}
const uint Ship::getAimPositionRandomLocalIndex() {
    if (m_AimPositionDefaults.size() == 1) {
        return 0;
    }
    const auto randIndex = Helper::GetRandomIntFromTo(0, static_cast<int>(m_AimPositionDefaults.size()) - 1);
    return randIndex;
}
const glm::vec3 Ship::getAimPositionRandomLocal() {
    return getAimPositionLocal(getAimPositionRandomLocalIndex());
}
const glm::vec3 Ship::getAimPositionLocal(const uint index) {
    if (m_AimPositionDefaults.size() == 0) {
        return glm::vec3(0.0f);
    }
    auto& body = *getComponent<ComponentBody>();
    if (m_AimPositionDefaults.size() == 1) {
        return Math::rotate_vec3(body.rotation(), m_AimPositionDefaults[0]);
    }
    return Math::rotate_vec3(body.rotation(), m_AimPositionDefaults[index]);
}
void Ship::destroy() {
    for (auto& system : m_ShipSystems) {
        if (system.second) {
            system.second->destroy();
        }
    }
    EntityWrapper::destroy();
    SAFE_DELETE_VECTOR(m_DamageDecals);
}
AI* Ship::getAI() {
    return m_AI;
}
const AIType::Type Ship::getAIType() const {
    if(m_AI)
        return m_AI->getType();
    return AIType::AI_None;
}
const glm_vec3 Ship::getWarpSpeedVector3() {
    if (m_IsWarping && m_WarpFactor > 0) {
        auto& body = *getComponent<ComponentBody>();
        const auto speed = (m_WarpFactor * 1.0f / 0.46f) * 2.0f;
        return (body.forward() * glm::pow(static_cast<decimal>(speed), static_cast<decimal>(15.0))) / glm::log2(static_cast<decimal>(body.mass()) + static_cast<decimal>(0.5));
    }
    return glm_vec3(static_cast<decimal>(0.0));
}
const Team& Ship::getTeam() const {
    return m_Team;
}
const string Ship::getName() {
    return getComponent<ComponentName>()->name();
}
const glm_vec3 Ship::getPosition() {
    return getComponent<ComponentBody>()->position();
}
const glm_quat Ship::getRotation() {
    return getComponent<ComponentBody>()->rotation();
}
const glm_vec3 Ship::getPosition(const EntityDataRequest& dataRequest) {
    return getComponent<ComponentBody>(dataRequest)->position();
}
const glm_quat Ship::getRotation(const EntityDataRequest& dataRequest) {
    return getComponent<ComponentBody>(dataRequest)->rotation();
}
void Ship::updateAntiCloakScanFromPacket(const PacketMessage& packet) {
    auto* sensors = static_cast<ShipSystemSensors*>(m_ShipSystems[ShipSystemType::Sensors]);
    if (sensors) {
        auto info = Helper::SeparateStringByCharacter(packet.data, ',');
        sensors->toggleAntiCloakScan(false);
        sensors->m_AntiCloakScanActive = (info[0] == "1") ? true : false;
        sensors->m_AntiCloakScanTimer = packet.r;
    }
}
void Ship::updateProjectileImpact(const PacketProjectileImpact& packet) {
    glm::vec3 normal;
    float rad, time;
    Math::Float32From16(&rad, packet.radius);
    Math::Float32From16(&time, packet.time);
    Math::Float32From16(&normal.x, packet.normalX);
    Math::Float32From16(&normal.y, packet.normalY);
    Math::Float32From16(&normal.z, packet.normalZ);
    Map& map = static_cast<Map&>(entity().scene());
    WeaponProjectile* proj = nullptr;

    if (packet.PacketType == PacketType::Server_To_Client_Projectile_Cannon_Impact) {
        proj = map.getCannonProjectile(packet.projectile_index);
    }else if (packet.PacketType == PacketType::Server_To_Client_Projectile_Torpedo_Impact) {
        proj = map.getTorpedoProjectile(packet.projectile_index);
    }
    if (packet.shields) {
        auto* shields = static_cast<ShipSystemShields*>(getShipSystem(ShipSystemType::Shields));
        if (shields) {
            const glm::vec3 local = glm::vec3(packet.impactX, packet.impactY, packet.impactZ);
            shields->receiveHit(normal, local, rad, time, packet.damage, packet.shield_side);
        }
    }else{
        auto* hull = static_cast<ShipSystemHull*>(getShipSystem(ShipSystemType::Hull));
        if (hull) {
            hull->receiveHit(normal, glm::vec3(packet.impactX, packet.impactY, packet.impactZ), rad, time, packet.damage);
        }
    }
    if (proj) {
        proj->destroy();
    }
}
void Ship::updatePhysicsFromPacket(const PacketPhysicsUpdate& packet, Map& map, vector<string>& info) {
    const unsigned int size = stoi(info[4]);
    Anchor* closest = map.getRootAnchor();
    for (unsigned int i = 5; i < 5 + size; ++i) {
        auto& children = closest->getChildren();
        if (!children.count(info[i])) {
            return;
        }
        closest = children.at(info[i]);
    }
    const auto nearestAnchorPos = closest->getPosition();
    const auto x = packet.px + nearestAnchorPos.x;
    const auto y = packet.py + nearestAnchorPos.y;
    const auto z = packet.pz + nearestAnchorPos.z;

    auto& body = *getComponent<ComponentBody>();
    btRigidBody& bulletBody = *const_cast<btRigidBody*>(&body.getBtBody());
    bulletBody.activate(true);//this is needed for when objects are far apart, should probably find a way to better do this
    btTransform centerOfMass;

    float qw, ax, ay, az, lx, ly, lz;

    auto quat_xyz = Math::unpack3NormalsFrom32Int(packet.qXYZ);

    Math::Float32From16(&qw, packet.qw);
    Math::Float32From16(&lx, packet.lx);  Math::Float32From16(&ly, packet.ly);  Math::Float32From16(&lz, packet.lz);
    Math::Float32From16(&ax, packet.ax);  Math::Float32From16(&ay, packet.ay);  Math::Float32From16(&az, packet.az);

    const btQuaternion rot(quat_xyz.x, quat_xyz.y, quat_xyz.z, qw);

    const btVector3 pos(static_cast<btScalar>(x), static_cast<btScalar>(y), static_cast<btScalar>(z));
    centerOfMass.setOrigin(pos);
    centerOfMass.setRotation(rot);
    bulletBody.getMotionState()->setWorldTransform(centerOfMass);
    bulletBody.setCenterOfMassTransform(centerOfMass);

    body.clearAllForces();
    body.setAngularVelocity(ax, ay, az, false);
    body.setLinearVelocity(static_cast<decimal>(lx - (packet.wx * WARP_PHYSICS_MODIFIER)), static_cast<decimal>(ly - (packet.wy * WARP_PHYSICS_MODIFIER)), static_cast<decimal>(lz - (packet.wz * WARP_PHYSICS_MODIFIER)), false);
    //body.setGoal(x, y, z, PHYSICS_PACKET_TIMER_LIMIT);
}
const bool Ship::canSeeCloak(Ship* otherShip) {
    if (IsPlayer() || otherShip->isAlly(*this)) {
        return true;
    }
    auto* sensors = static_cast<ShipSystemSensors*>(m_ShipSystems[ShipSystemType::Sensors]);
    if (sensors) {
        if (sensors->isShipDetectedByAntiCloak(otherShip)) {
            return true;
        }
    }
    return false;
}
void Ship::updateCloakVisuals(const float& r, const float& g, const float& b, const float& alpha, ComponentModel& model) {
    for (auto& decal : m_DamageDecals) {
        if (decal && decal->active()) {
            auto& decal_model = *decal->getComponent<ComponentModel>();
            for (unsigned int i = 0; i < decal_model.getNumModels(); ++i) {
                auto& instance = decal_model.getModel(i);
                auto& color = instance.color();
                instance.setColor(color.r, color.g, color.b, alpha);
            }
        }
    }
    if (r >= 0.0f && g >= 0.0f && b >= 0.0f) {
        for (size_t i = 0; i < model.getNumModels(); ++i) {
            auto& instance = model.getModel(i);
            instance.setColor(r, g, b, alpha);
        }
    }
}
void Ship::updateCloakVisuals(const float& alpha, ComponentModel& model) {
    updateCloakVisuals(-1.0f, -1.0f, -1.0f, alpha, model);
}
void Ship::updateCloakFromPacket(const PacketCloakUpdate& packet) {
    if (!m_ShipSystems[ShipSystemType::CloakingDevice])
        return;
    ShipSystemCloakingDevice& cloak = *static_cast<ShipSystemCloakingDevice*>(m_ShipSystems[ShipSystemType::CloakingDevice]);
    Math::Float32From16(&cloak.m_CloakTimer, packet.cloakTimer);
    auto& model = *getComponent<ComponentModel>();

    Map& map = static_cast<Map&>(entity().scene());
    if (!canSeeCloak(map.getPlayer())) {
        if (cloak.m_CloakTimer < 0.0) {
            cloak.m_CloakTimer = 0.0f; 
        }
    }
    cloak.m_Active = packet.cloakActive;

    if (packet.justTurnedOn || packet.justTurnedOff) {
        if (packet.justTurnedOn)
            Ship::cloak(false);
        if (packet.justTurnedOff)
            Ship::decloak(false);
    }else{
        auto* shields = static_cast<ShipSystemShields*>(m_ShipSystems[ShipSystemType::Shields]);
        if (shields) {
            if (cloak.m_Active) {
                shields->turnOffShields();
            }
        }
        if (cloak.m_CloakTimer < 1.0f && cloak.m_CloakTimer >= 0.0f) {
            for (unsigned int i = 0; i < model.getNumModels(); ++i) {
                model.setModelShaderProgram(ResourceManifest::ShipShaderProgramForward, i, RenderStage::ForwardTransparentTrianglesSorted);
            }
            updateCloakVisuals(1, 1, 1, glm::abs(cloak.m_CloakTimer), model);
        }else if(cloak.m_CloakTimer < 0.0f){
            for (unsigned int i = 0; i < model.getNumModels(); ++i) {
                model.setModelShaderProgram(ResourceManifest::ShipShaderProgramForward, i, RenderStage::ForwardTransparentTrianglesSorted);
            }
            updateCloakVisuals(0.369f, 0.912f, 1, glm::abs(cloak.m_CloakTimer), model);
        }else{
            for (unsigned int i = 0; i < model.getNumModels(); ++i) {
                model.setModelShaderProgram(ResourceManifest::ShipShaderProgramDeferred, i, RenderStage::GeometryOpaque);
            }
            updateCloakVisuals(1, 1, 1, glm::abs(cloak.m_CloakTimer), model);
        }
    }
}
void Ship::updateHealthFromPacket(const PacketHealthUpdate& packet) {
    auto* shields = static_cast<ShipSystemShields*>(m_ShipSystems[ShipSystemType::Shields]);
    auto* hull    = static_cast<ShipSystemHull*>(m_ShipSystems[ShipSystemType::Hull]);
    if (shields) {

        shields->m_HealthPointsCurrent[0] = packet.currentShieldsHealthF;
        shields->m_HealthPointsCurrent[1] = packet.currentShieldsHealthA;
        shields->m_HealthPointsCurrent[2] = packet.currentShieldsHealthP;
        shields->m_HealthPointsCurrent[3] = packet.currentShieldsHealthS;
        shields->m_HealthPointsCurrent[4] = packet.currentShieldsHealthD;
        shields->m_HealthPointsCurrent[5] = packet.currentShieldsHealthV;


        if (packet.flags & PacketHealthUpdate::PacketHealthFlags::ShieldsActive) {
        }
        if (packet.flags & PacketHealthUpdate::PacketHealthFlags::ShieldsInstalled) {
        }
        if (packet.flags & PacketHealthUpdate::PacketHealthFlags::ShieldsTurnedOn) {
            shields->m_ShieldsAreUp = packet.flags;
        }
    }
    if (hull) {
        hull->m_HealthPointsCurrent = packet.currentHullHealth;
    }
}
const float Ship::updateShipDimensions() {
    auto& rigidBodyComponent = *getComponent<ComponentBody>();
    auto& modelComponent = *getComponent<ComponentModel>();

    const auto& boundingBox = modelComponent.boundingBox();
    const auto volume = boundingBox.x * boundingBox.y * boundingBox.z;
    auto mass = (volume * 0.4f) + 1.0f;
    rigidBodyComponent.setMass(mass);
    return mass;
}
void Ship::setModel(Handle& modelHandle) {
    auto& rigidBodyComponent = *getComponent<ComponentBody>();
    auto& modelComponent     = *getComponent<ComponentModel>();
    modelComponent.setModelMesh(modelHandle, 0);
    const auto& boundingBox = modelComponent.boundingBox();
    const auto volume = boundingBox.x * boundingBox.y * boundingBox.z;
    rigidBodyComponent.setMass((volume * 0.4f) + 1.0f);
}
void Ship::translateWarp(const double& amount, const double& dt){
    const auto amountToAdd = amount * (1.0 / 0.5);
    if((amount > 0.0 && m_WarpFactor + amount < 1.07) || (amount < 0.0 && m_WarpFactor > 0.0f)){
        m_WarpFactor += static_cast<float>(amountToAdd * dt);
    }
}
bool Ship::cloak(const bool sendPacket) {
    if (m_ShipSystems[ShipSystemType::CloakingDevice]) {
        ShipSystemCloakingDevice& cloak = *static_cast<ShipSystemCloakingDevice*>(m_ShipSystems[ShipSystemType::CloakingDevice]);
        auto& model = *getComponent<ComponentModel>();
        return cloak.cloak(model, sendPacket);
    }
    return false;
}
bool Ship::decloak(const bool sendPacket) {
    if (m_ShipSystems[ShipSystemType::CloakingDevice]) {
        ShipSystemCloakingDevice& cloak = *static_cast<ShipSystemCloakingDevice*>(m_ShipSystems[ShipSystemType::CloakingDevice]);
        auto& model = *getComponent<ComponentModel>();
        return cloak.decloak(model, sendPacket);
    }
    return false;
}
void Ship::toggleWarp() {
    m_IsWarping = !m_IsWarping;
    m_WarpFactor = 0;
    auto& rigidBodyComponent = *getComponent<ComponentBody>();
    rigidBodyComponent.clearLinearForces();
}
const string& Ship::getClass() const { 
    return m_ShipClass; 
}
GameCamera* Ship::getPlayerCamera() { 
    return m_PlayerCamera; 
}
const bool Ship::IsPlayer() const {
    return (m_AI && m_AI->getType() == AIType::Player_You) ? true : false;
}
const bool Ship::IsWarping() const {
    return m_IsWarping; 
}
Entity& Ship::entity() { 
    return m_Entity; 
}
ShipSystem* Ship::getShipSystem(const uint type) { 
    return m_ShipSystems[type]; 
}
const glm_vec3 Ship::getLinearVelocity() {
    if (m_IsWarping && IsPlayer()) {
        return -(getWarpSpeedVector3() * static_cast<decimal>(WARP_PHYSICS_MODIFIER));
    }
    return getComponent<ComponentBody>()->getLinearVelocity();
}
EntityWrapper* Ship::getTarget() { 
    auto* sensors = static_cast<ShipSystemSensors*>(m_ShipSystems[ShipSystemType::Sensors]);
    if(sensors)
        return sensors->m_Target;
    return nullptr;
}
void Ship::setTarget(const string& target, const bool sendPacket) {
    auto* sensors = static_cast<ShipSystemSensors*>(m_ShipSystems[ShipSystemType::Sensors]);
    if (sensors) {
        sensors->setTarget(target, sendPacket);
    }
    auto* camera = getPlayerCamera();
    if (camera) {
        camera->setTarget(target);
    }
}
void Ship::setTarget(EntityWrapper* target, const bool sendPacket) {
    auto* sensors = static_cast<ShipSystemSensors*>(m_ShipSystems[ShipSystemType::Sensors]);
    if (sensors) {
        sensors->setTarget(target, sendPacket);
    }
    auto* camera = getPlayerCamera();
    if (camera) {
        camera->setTarget(target);
    }
}
const glm_vec3& Ship::forward() {
    return getComponent<ComponentBody>()->forward();
}
const glm_vec3& Ship::right() {
    return getComponent<ComponentBody>()->right();
}
const glm_vec3& Ship::up() {
    return getComponent<ComponentBody>()->up();
}
const bool Ship::isCloaked() {
    if (m_ShipSystems[ShipSystemType::CloakingDevice]) {
        ShipSystemCloakingDevice& cloak = *static_cast<ShipSystemCloakingDevice*>(m_ShipSystems[ShipSystemType::CloakingDevice]);
        return (cloak.m_Active || cloak.getCloakTimer() < 1.0f) ? true : false;
    }
    return false;
}
const bool Ship::isFullyCloaked() {
    if (m_ShipSystems[ShipSystemType::CloakingDevice]) {
        ShipSystemCloakingDevice& cloak = *static_cast<ShipSystemCloakingDevice*>(m_ShipSystems[ShipSystemType::CloakingDevice]);
        return (cloak.m_Active && cloak.m_CloakTimer <= 0.0) ? true : false;
    }
    return false;
}
const bool Ship::isAlly(Ship& other) {
    return m_Team.isAllyTeam(other.getTeam());
}
const bool Ship::isEnemy(Ship& other) {
    return m_Team.isEnemyTeam(other.getTeam());
}
const bool Ship::isNeutral(Ship& other) {
    return m_Team.isNeutralTeam(other.getTeam());
}

void Ship::onEvent(const Event& e){
    if (e.type == EventType::WindowResized) {
        for (auto& system : m_ShipSystems) {
            if (system.second) {
                system.second->onResize(e.eventWindowResized.width, e.eventWindowResized.height);
            }
        }
    }
}

PrimaryWeaponBeam& Ship::getPrimaryWeaponBeam(const uint index) {
    auto& weapons = *static_cast<ShipSystemWeapons*>(getShipSystem(ShipSystemType::Weapons));
    return *weapons.m_PrimaryWeaponsBeams[index];
}
PrimaryWeaponCannon& Ship::getPrimaryWeaponCannon(const uint index) {
    auto& weapons = *static_cast<ShipSystemWeapons*>(getShipSystem(ShipSystemType::Weapons));
    return *weapons.m_PrimaryWeaponsCannons[index];
}
SecondaryWeaponTorpedo& Ship::getSecondaryWeaponTorpedo(const uint index) {
    auto& weapons = *static_cast<ShipSystemWeapons*>(getShipSystem(ShipSystemType::Weapons));
    return *weapons.m_SecondaryWeaponsTorpedos[index];
}
void Ship::update(const double& dt) {

}