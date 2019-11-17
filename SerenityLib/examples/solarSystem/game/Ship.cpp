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
#include <core/engine/renderer/particles/Particle.h>
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

#include <core/engine/renderer/particles/ParticleEmitter.h>
#include <core/engine/renderer/particles/ParticleEmissionProperties.h>
#include "particles/Fire.h"
#include "particles/Sparks.h"

using namespace Engine;
using namespace std;


void ShipModelInstanceBindFunctor::operator()(EngineResource* r) const {
    auto& i = *static_cast<ModelInstance*>(r);
    Entity& parent = i.parent();
    const auto& stage = i.stage();
    auto& scene = parent.scene();
    Camera& cam = *scene.getActiveCamera();
    glm::vec3 camPos = cam.getPosition();
    auto& body = *(parent.getComponent<ComponentBody>());
    Ship& ship = *static_cast<Ship*>(i.getUserPointer());
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
    Renderer::sendUniform1("OfflineGlowFactor", ship.m_OfflineGlowFactor);
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
    switch (ship.m_State) {
        case ShipState::Nominal: {
            
            if (ship.IsPlayer()) {
                ship.internal_update_player_you_logic(dt, map);
            }
            if (ship.m_AI) {
                ship.internal_update_ai(dt, map);
            }
            auto* hull = static_cast<ShipSystemHull*>(ship.getShipSystem(ShipSystemType::Hull));
            if (hull) {
                auto hull_percent = hull->getHealthPercent();
                if (hull_percent < 0.15f) {
                    ship.m_OfflineGlowFactorTimer += static_cast<float>(dt);
                    if (ship.m_OfflineGlowFactorTimer > 0.2f) {
                        const auto rand = Helper::GetRandomFloatFromTo(0.0f, 100.0f);
                        if (rand < 9.0f) {
                            ship.m_OfflineGlowFactor = 0.0f;
                        }else{
                            ship.m_OfflineGlowFactor = 1.0f;
                        }
                        ship.m_OfflineGlowFactorTimer = 0.0f;
                    }
                }

            }
            break;
        }case ShipState::JustFlaggedForDestruction: {
            ship.internal_update_just_flagged_for_destruction(dt, map);
            break;
        }case ShipState::UndergoingDestruction: {
            ship.internal_update_undergoing_destruction(dt, map);
            break;
        }case ShipState::JustFlaggedAsFullyDestroyed: {
            ship.internal_update_just_destroyed_fully(dt, map);
            break;
        }case ShipState::Destroyed: {
            break;
        }case ShipState::JustFlaggedToRespawn: {
            ship.internal_update_just_flagged_for_respawn(dt, map);
            break;
        }case ShipState::UndergoingRespawning: {
            ship.internal_update_undergoing_respawning(dt, map);
            break;
        }case ShipState::WaitingForServerToRespawnMe: {
            break;
        }default: {
            break;
        }
    }
    for (auto it2 = ship.m_EmittersDestruction.begin(); it2 != ship.m_EmittersDestruction.end();){
        auto& tuple = (*it2);
        auto active = std::get<0>(tuple)->isActive();
        if (!active){
            it2 = ship.m_EmittersDestruction.erase(it2);
        }else{
            ++it2;
        }
    }
    for (auto& shipSystem : ship.m_ShipSystems) {
        if (shipSystem.second) { //some ships wont have all the systems (cloaking device, etc)
            shipSystem.second->update(dt);
        }
    }
    ship.internal_update_damage_emitters(dt, map);
    ship.internal_update_decals(dt, map);
}};

//TODO: move to the hull system?
struct HullCollisionFunctor final { void operator()(CollisionCallbackEventData& data) const {
    auto ownerShipVoid = data.ownerBody.getUserPointer1();
    if (ownerShipVoid) {
        auto otherShipVoid = data.otherBody.getUserPointer1();
        if (otherShipVoid && ownerShipVoid != otherShipVoid) { //redundant?
            if (data.ownerBody.getCollisionGroup() == CollisionFilter::_Custom_4 && data.otherBody.getCollisionGroup() == CollisionFilter::_Custom_4) { //hull on hull only
                auto* ownerShip = static_cast<Ship*>(ownerShipVoid);
                auto* otherShip = static_cast<Ship*>(otherShipVoid);

                if (ownerShip->IsPlayer() /*|| otherShip->IsPlayer()*/) {

                    auto* ownerHull = static_cast<ShipSystemHull*>(ownerShip->getShipSystem(ShipSystemType::Hull));
                    auto* otherHull = static_cast<ShipSystemHull*>(otherShip->getShipSystem(ShipSystemType::Hull));
                    if (ownerHull && otherHull) {
                        const auto ownerMass = data.ownerBody.mass() * 3000.0f;
                        const auto otherMass = data.otherBody.mass() * 3000.0f;
                        const auto massTotal = ownerMass + otherMass;

                        const auto ownerLocal = data.ownerHit - glm::vec3(data.ownerBody.position());
                        const auto otherLocal = data.otherHit - glm::vec3(data.otherBody.position());

                        const auto owner_linear_velocity = glm::vec3(ownerShip->getLinearVelocity());
                        const auto other_linear_velocity = glm::vec3(otherShip->getLinearVelocity());

                        const auto ownerMomentum = ownerMass * owner_linear_velocity;
                        const auto otherMomentum = otherMass * other_linear_velocity;
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
                        
                        const auto owner_angular_velocity = glm::vec3(data.ownerBody.getAngularVelocity());
                        const auto other_angular_velocity = glm::vec3(data.otherBody.getAngularVelocity());

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

                        ownerHull->receiveCollisionVisual(data.normalOnB, ownerLocal, damageRadiusOwner, data.ownerModelInstanceIndex);
                        otherHull->receiveCollisionVisual(data.normalOnB, otherLocal, damageRadiusOther, data.otherModelInstanceIndex);
                    }
                }
            }
        }
    }
}};

Ship::Ship(Team& team, Client& client, const string& shipClass, Map& map, const AIType::Type ai_type, const string& name, const glm_vec3 pos, const glm_vec3 scl, CollisionType::Type collisionType, const glm::vec3 aimPosDefault, const glm::vec3 camOffsetDefault):EntityWrapper(map),m_Client(client),m_Team(team){
    m_WarpFactor              = 0;
    m_State                   = ShipState::Nominal;
    m_DestructionTimerCurrent = 0.0;
    m_DestructionTimerDecalTimer = 0.0;
    m_DestructionTimerDecalTimerMax = 0.5;
    m_RespawnTimer            = 0.0;
    m_RespawnTimerMax         = 120.0;
    m_OfflineGlowFactor       = 1.0f;
    m_OfflineGlowFactorTimer  = 0.0f;

    m_AI                      = new AI(ai_type);
    m_ShipClass               = shipClass;
    m_IsWarping               = false;
    m_PlayerCamera            = nullptr;
    m_MouseFactor             = glm::dvec2(0.0);
    m_CameraOffsetDefault     = camOffsetDefault;
    m_AimPositionDefaults.push_back(aimPosDefault);

    auto& shipInfo            = Ships::Database[shipClass];
    auto& modelComponent      = *addComponent<ComponentModel>(shipInfo.MeshHandles[0], shipInfo.MaterialHandles[0], ResourceManifest::ShipShaderProgramDeferred, RenderStage::GeometryOpaque);
    auto& bodyComponent       = *addComponent<ComponentBody>(collisionType);
    auto& nameComponent       = *addComponent<ComponentName>(name);
    auto& logicComponent      = *addComponent<ComponentLogic>(ShipLogicFunctor(), this);

    setModel(shipInfo.MeshHandles[0]);

    bodyComponent.setDamping(static_cast<decimal>(0.01), static_cast<decimal>(0.2));
    bodyComponent.getBtBody().setActivationState(DISABLE_DEACTIVATION);//this might be dangerous...
    bodyComponent.setPosition(pos);
    bodyComponent.setScale(scl);

    //the body is using a convex hull for ship to ship ramming
    bodyComponent.setCollisionGroup(CollisionFilter::_Custom_4); //i belong to ramming hull group (group 4)
    bodyComponent.setCollisionMask(CollisionFilter::_Custom_4); //i should only collide with other ramming hulls only
    bodyComponent.setCollisionFunctor(HullCollisionFunctor());

	if (IsPlayer()) {
		m_PlayerCamera = static_cast<GameCamera*>(map.getActiveCamera());
	}
    bodyComponent.setUserPointer1(this);
    bodyComponent.setUserPointer2(&client);
    
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
    modelComponent.setUserPointer(this);

    internal_calculate_ship_destruction_time_max(modelComponent);
}
Ship::~Ship(){
    unregisterEvent(EventType::WindowResized);
    SAFE_DELETE(m_AI);
    for (auto& tuple : m_DamageDecals) {
        SAFE_DELETE(std::get<0>(tuple));
    }
    m_DamageDecals.clear();
	SAFE_DELETE_MAP(m_ShipSystems);
}
void Ship::respawn(const glm_vec3& newPosition, const string& nearest_spawn_anchor, Map& map) {
    auto& bodyComponent = *getComponent<ComponentBody>();
    auto& modelComponent = *getComponent<ComponentModel>();

    auto* shields = static_cast<ShipSystemShields*>(getShipSystem(ShipSystemType::Shields));
    auto* hull = static_cast<ShipSystemHull*>(getShipSystem(ShipSystemType::Hull));
    if (shields) {
        shields->restoreToFull();
        shields->reset_all_impact_points();
        shields->turnOnShields();
        shields->getEntity().getComponent<ComponentBody>()->addPhysicsToWorld();
        bodyComponent.addPhysicsToWorld();
    }
    if (hull) {
        hull->restoreToFull();
        hull->getEntity().getComponent<ComponentBody>()->addPhysicsToWorld();

    }
    auto* playerCamera = getPlayerCamera();
    if (IsPlayer() && playerCamera) {
        playerCamera->setState(CameraState::Cockpit);
    }
    m_OfflineGlowFactorTimer = 0.0f;
    m_OfflineGlowFactor = 1.0f;

    modelComponent.show();
    auto& anchorPos = map.getSpawnAnchor(nearest_spawn_anchor)->getPosition();
    bodyComponent.setPosition(anchorPos + newPosition);
    auto vec = glm::normalize(newPosition);
    glm_quat q = glm_quat(1.0, 0.0, 0.0, 0.0);
    Engine::Math::alignTo(q, vec);
    bodyComponent.setRotation(q);
    setState(ShipState::Nominal);
}

void Ship::internal_update_undergoing_respawning(const double& dt, Map& map) {
    m_RespawnTimer += dt;
    if (m_RespawnTimer >= m_RespawnTimerMax) {
        setState(ShipState::WaitingForServerToRespawnMe);
        m_RespawnTimer = 0.0;
    }
}
void Ship::internal_update_just_flagged_for_respawn(const double& dt, Map& map) {
    setState(ShipState::UndergoingRespawning);
}
void Ship::internal_update_just_flagged_for_destruction(const double& dt, Map& map) {
    setDamping(0.0, 0.0);
    m_DestructionTimerCurrent = 0.0;

    auto* sensors = static_cast<ShipSystemSensors*>(getShipSystem(ShipSystemType::Sensors));
    auto* cloak = static_cast<ShipSystemCloakingDevice*>(getShipSystem(ShipSystemType::CloakingDevice));
    if (sensors) {
        sensors->disableAntiCloakScan(true);
    }
    if (cloak) {
        cloak->forceCloakOff(true);
    }
    setTarget(nullptr, true);
    setState(ShipState::UndergoingDestruction);
}
void Ship::internal_update_just_destroyed_fully(const double& dt, Map& map) {
    auto& bodyComponent = *getComponent<ComponentBody>();
    auto& modelComponent = *getComponent<ComponentModel>();

    auto* shields = static_cast<ShipSystemShields*>(getShipSystem(ShipSystemType::Shields));
    auto* hull = static_cast<ShipSystemHull*>(getShipSystem(ShipSystemType::Hull));
    if (shields) {
        shields->reset_all_impact_points();
        shields->turnOffShields();
        shields->getEntity().getComponent<ComponentBody>()->removePhysicsFromWorld();
    }
    if (hull) {
        hull->getEntity().getComponent<ComponentBody>()->removePhysicsFromWorld();
        
    }
    bodyComponent.clearAllForces();
    bodyComponent.removePhysicsFromWorld();
    for (auto& ptr : m_EmittersDestruction) {
        auto& emitter = *std::get<0>(ptr);
        emitter.deactivate();
    }
    //TODO: notify server if its the player or an npc ship (might need to rethink this)
    if (getAIType() != AIType::Player_Other) {
        PacketMessage pOut;
        pOut.PacketType = PacketType::Client_To_Server_Ship_Was_Just_Destroyed;
        pOut.name = getMapKey();
        pOut.data = getClass(); //[0]

        Anchor* finalAnchor = map.getRootAnchor();
        const auto& list = map.getClosestAnchor();
        for (auto& closest : list) {
            finalAnchor = finalAnchor->getChildren().at(closest);
        }
        pOut.data += "," + to_string(list.size()); //[1]
        for (auto& closest : list)
            pOut.data += "," + closest;
        const auto nearestAnchorPos = glm::vec3(finalAnchor->getPosition());
        pOut.r = nearestAnchorPos.x;
        pOut.g = nearestAnchorPos.y;
        pOut.b = nearestAnchorPos.z;


        m_Client.send(pOut);
    }

    modelComponent.hide();
    for (auto& tuple : m_DamageDecals) {
        SAFE_DELETE(std::get<0>(tuple));
    }
    m_DamageDecals.clear();
    setState(ShipState::Destroyed);
}

struct Test final { void operator()(const double& dt, ParticleEmitter& emitter, ParticleEmissionProperties& properties, std::mutex& mutex_) {
    emitter.m_UserData.x += static_cast<float>(dt);
    if (emitter.m_UserData.x > emitter.m_UserData.y) {
        auto& map_ = static_cast<Map&>(emitter.entity().scene());
        mutex_.lock();
        ParticleEmitter emitter_3(*Fire::OutwardFireball, map_, 8.0, nullptr);
        auto x = map_.addParticleEmitter(emitter_3);
        mutex_.unlock();

        auto rand_rot_x = Helper::GetRandomFloatFromTo(-0.15f, 0.15f);
        auto rand_rot_y = Helper::GetRandomFloatFromTo(-0.15f, 0.15f);
        auto rand_rot_z = Helper::GetRandomFloatFromTo(-0.15f, 0.15f);
        glm_vec3 n = glm_vec3(rand_rot_x, rand_rot_y, rand_rot_z);

        emitter_3.setPosition(emitter.position());
        emitter_3.setScale(emitter.getScale());
        emitter_3.setRotation(emitter.rotation());
        emitter_3.rotate(n.x, n.y, n.z);
        emitter_3.setLinearVelocity( (emitter.linearVelocity() * 0.65)  * emitter.rotation());


        emitter.m_UserData.x = 0.0f;
        emitter.m_UserData.y = Helper::GetRandomFloatFromTo(1.1f, 1.4f);
    }
};};

void Ship::internal_update_undergoing_destruction(const double& dt, Map& map) {
    m_DestructionTimerCurrent += dt;
    m_DestructionTimerDecalTimer += dt;

    m_OfflineGlowFactorTimer += static_cast<float>(dt);
    if (m_OfflineGlowFactorTimer > 0.2f) {
        const auto rand = Helper::GetRandomFloatFromTo(0.0f, 100.0f);
        if (rand < ((m_DestructionTimerCurrent / m_DestructionTimerMax) * 100.0f) * 0.4f) {
            m_OfflineGlowFactor = 0.0f;
        }else{
            m_OfflineGlowFactor = 1.0f;
        }
        m_OfflineGlowFactorTimer = 0.0f;
    }
    if (m_DestructionTimerDecalTimer > m_DestructionTimerDecalTimerMax) {
        //apply random hull fire decal, particle effect, and small (or large explosion sound at various times)

        auto rand = Helper::GetRandomIntFromTo(0, 100);
        auto rand2 = Helper::GetRandomFloatFromTo(0.15f, 0.4f) + 0.3f;

        m_DestructionTimerDecalTimerMax = rand2;

        Handle randSmallSound;
        if (rand < 33) {
            randSmallSound = ResourceManifest::SoundExplosionSmall1;
        }else if (rand >= 33 && rand < 66) {
            randSmallSound = ResourceManifest::SoundExplosionSmall2;
        }else {
            randSmallSound = ResourceManifest::SoundExplosionSmall3;
        }

        auto rand3 = Helper::GetRandomFloatFromTo(0.75f, 1.0f);
        auto rand4 = Helper::GetRandomFloatFromTo(25.75f, 35.0f);

        auto& shipModelComponent = *getComponent<ComponentModel>();
        const auto modelIndex = Helper::GetRandomIntFromTo(0, static_cast<int>(shipModelComponent.getNumModels() - 1));
        auto& instance = shipModelComponent.getModel(modelIndex);
        auto& mesh = *instance.mesh();
        auto& verts = const_cast<VertexData&>(mesh.getVertexData()).getData<glm::vec3>(0);
        auto& norms = const_cast<VertexData&>(mesh.getVertexData()).getData<std::uint32_t>(2);
        const auto randVertexIndex = Helper::GetRandomIntFromTo(0, static_cast<int>(verts.size()) - 1);


        auto localNormal = glm::normalize(Math::unpack3NormalsFrom32Int(norms[randVertexIndex]));

        auto localPos    = verts[randVertexIndex];

        localPos = localPos + instance.position();
        auto* hull = static_cast<ShipSystemHull*>(getShipSystem(ShipSystemType::Hull));
        if (hull) {
            auto rand5 = Helper::GetRandomFloatFromTo(2.5f, 4.4f);
            hull->applyDamageDecal(localNormal, localPos, rand5, modelIndex, true);
        }

        auto* sound = Sound::playEffect(randSmallSound);
        if (sound) {
            sound->setPosition(getPosition());
            sound->setAttenuation(0.3f);
        }
        m_DestructionTimerDecalTimer = 0.0;
    }

    if (m_DestructionTimerCurrent >= m_DestructionTimerMax) {
        m_DestructionTimerCurrent = 0.0;
        m_DestructionTimerDecalTimer = 0.0;

        auto rand = Helper::GetRandomIntFromTo(0, 100);

        Handle randLargeSound;
        if (rand < 50) {
            randLargeSound = ResourceManifest::SoundExplosionLarge1;
        }else {
            randLargeSound = ResourceManifest::SoundExplosionLarge2;
        }
        auto* sound = Sound::playEffect(randLargeSound);
        if (sound) {
            sound->setPosition(getPosition());
            sound->setAttenuation(0.3f);
        }
        setState(ShipState::JustFlaggedAsFullyDestroyed);


        ParticleEmitter emitter_(*Sparks::ExplosionSparks, map, 0.01, this);
        glm::quat q = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
        emitter_.setPosition(glm::vec3(0.0f));

        auto* emitter = map.addParticleEmitter(emitter_);
        if (emitter) {
            m_EmittersDestruction.push_back(make_tuple(emitter, 0, glm::vec3(0.0f), q));
        }
        const auto shipLinV = getLinearVelocity();
        for (int i = 0; i < 8 + int(m_DestructionTimerMax); ++i) {
            auto rand_n_x = Helper::GetRandomFloatFromTo(-1.0f, 1.0f);
            auto rand_n_y = Helper::GetRandomFloatFromTo(-1.0f, 1.0f);
            auto rand_n_z = Helper::GetRandomFloatFromTo(-1.0f, 1.0f);
            auto norm = glm::normalize(glm::vec3(rand_n_x, rand_n_y, rand_n_z));

            auto factor = Helper::GetRandomFloatFromTo(0.45f, 0.55f);
            auto randScale = Helper::GetRandomFloatFromTo(1.0f, 1.6f);
            auto pos = getPosition();

            ParticleEmitter emitter_2(*Fire::OutwardFireballDebrisFire, map, 6.0, nullptr);
            glm_quat q = glm_quat(1.0, 0.0, 0.0, 0.0);
            Math::alignTo(q, -norm);
            emitter_2.setPosition(pos);
            emitter_2.setRotation(q);
            emitter_2.setScale(randScale, randScale, randScale);
            emitter_2.setLinearVelocity(shipLinV, false);
            auto newVel = glm_vec3(norm * factor);
            emitter_2.applyLinearVelocity(newVel, true);
            auto* emitter2 = map.addParticleEmitter(emitter_2);
            if (emitter2) {
                emitter2->m_UserData.y = 0.4f;
                emitter2->setUpdateFunctor(Test());
            }
        }
        //TODO: alert server?
    }
}
void Ship::internal_update_damage_emitters(const double& dt, Map& map) {
    for (auto& ptr : m_EmittersDestruction) {

        auto& emitter    = *std::get<0>(ptr);
        auto& modelIndex =  std::get<1>(ptr);
        auto& initialPos =  std::get<2>(ptr);
        auto& initialRot =  std::get<3>(ptr);

        auto& shipBody = *getComponent<ComponentBody>();
        auto& shipModel = *getComponent<ComponentModel>();
        const auto shipRotation = shipBody.rotation();

        if (modelIndex > 0) {
            auto& instance = shipModel.getModel(modelIndex);
            auto instancePos = glm_vec3(instance.position());
            auto instanceRot = instance.orientation();
            auto part1 = Math::rotate_vec3(instanceRot, initialPos);
            glm_vec3 localPos = instancePos + part1;

            emitter.setPosition(shipBody.position() + (shipRotation * localPos));
            emitter.setRotation(shipRotation * (glm_quat(instanceRot) * initialRot));

        }else{
            emitter.setPosition(shipBody.position() + (shipRotation * initialPos));
            emitter.setRotation(shipRotation * initialRot);
        }
    }
}
void Ship::internal_update_decals(const double& dt, Map& map) {

    auto& shipBody = *getComponent<ComponentBody>();
    auto& shipModel = *getComponent<ComponentModel>();
    const auto shipRotation = shipBody.rotation();
    for (auto& tuple : m_DamageDecals) {
        auto* decal_ptr = std::get<0>(tuple);
        auto& decal = *decal_ptr;
        auto& modelIndex = std::get<1>(tuple);

        if (modelIndex > 0) {      
            auto& instance = shipModel.getModel(modelIndex);
            auto instancePos = glm_vec3(instance.position());
            auto instanceRot = instance.orientation();
            auto part1 = Math::rotate_vec3(instanceRot, decal.initialPosition());
            glm_vec3 localPos = instancePos + part1;

            decal.setPosition(shipBody.position() + (shipRotation * localPos));
            decal.setRotation(shipRotation * (glm_quat(instanceRot) * decal.initialRotation()));     
        }else {
            decal.setPosition(shipBody.position() + (shipRotation * decal.initialPosition()));
            decal.setRotation(shipRotation * decal.initialRotation());
        }
        decal.update(dt);

        const auto rand = Helper::GetRandomIntFromTo(0, 13000);
        if (rand < 2) {
            ParticleEmitter emitter_(*Sparks::Spray, map, 3.0, this);
            auto* emitter = map.addParticleEmitter(emitter_);
            if (emitter) {
                m_EmittersDestruction.push_back(make_tuple(emitter, modelIndex, decal.initialPosition(), decal.initialRotation()));
            }
        }

        if (!decal.active()) {
            removeFromVector(m_DamageDecals, tuple); //might be dangerous
        }
    }
}
void Ship::internal_update_ai(const double& dt, Map& map) {
    auto& ai = *m_AI;
    if (IsPlayer()) {
        if (Engine::isKeyDownOnce(KeyboardKey::H)) {
            auto* fire_at_will = ai.getFireAtWill();
            if (fire_at_will) {
                fire_at_will->toggle();
            }
        }
    }
    ai.update(dt);
}
void Ship::internal_update_player_you_logic(const double& dt, Map& map) {
    #pragma region PlayerFlightControls
    auto* mytarget = getTarget();
    if (!Engine::paused()) {
        if (m_IsWarping && m_WarpFactor > 0) {
            auto speed = getWarpSpeedVector3() * static_cast<decimal>(dt);
            for (auto& pod : epriv::InternalScenePublicInterface::GetEntities(map)) {
                Entity e = map.getEntity(pod);
                const EntityDataRequest dataRequest(e);
                auto* cam = e.getComponent<ComponentCamera>(dataRequest);
                //TODO: parent->child relationship
                if (e != m_Entity && !cam && (e != map.getHUD().getSensorDisplay().radarRingEntity() && e != map.getHUD().getSensorDisplay().radarCameraEntity())) {
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


    auto& camera = *m_PlayerCamera;
    if (Engine::isKeyDownOnce(KeyboardKey::F1)) {
        camera.setState(CameraState::Cockpit);
    }else if (Engine::isKeyDownOnce(KeyboardKey::F3)) {
        camera.setState(CameraState::Orbit);
    }else if (Engine::isKeyDownOnce(KeyboardKey::F2)) {
        camera.setState(CameraState::FollowTarget);
    }
    //target whatever is in direct line of sight of the camera
    /*
    if (Engine::isKeyDownOnce(KeyboardKey::T) && map.name() != "Menu") {
        vector<Entity> exclusions{ m_Entity, static_cast<ShipSystemHull*>(getShipSystem(ShipSystemType::Hull))->getEntity(), camera.entity() };
        auto* shields = static_cast<ShipSystemShields*>(getShipSystem(ShipSystemType::Shields));
        if (shields)
            exclusions.push_back(shields->getEntity());
        Entity scan = camera.getObjectInCenterRay(exclusions);
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
                setTarget(scannedTarget, true);
            }
        }
    }
    */
    for (auto& shipSystem : m_ShipSystems) {
        if (shipSystem.second) {
            shipSystem.second->render();
        }
    }
}

void Ship::internal_calculate_ship_destruction_time_max(ComponentModel& model) {
    auto rad = model.radius();
    m_DestructionTimerMax = (rad * 5.0) + 2.5;
}

const string& Ship::getMapKey() const {
    return m_MapKey;
}

const bool Ship::setState(const ShipState::State& state) {
    if (m_State != state) {
        m_State = state;
        return true;
    }
    return false;
}
void Ship::setDamping(const decimal& linear, const decimal& angular) {
    auto& body = *getComponent<ComponentBody>();
    body.setDamping(linear, angular);
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
    for (auto& tuple : m_DamageDecals) {
        SAFE_DELETE(std::get<0>(tuple));
    }
    m_DamageDecals.clear();
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

    bool forceHull = false;
    if (packet.PacketType == PacketType::Server_To_Client_Projectile_Cannon_Impact) {
        proj = map.getCannonProjectile(packet.projectile_index);
    }else if (packet.PacketType == PacketType::Server_To_Client_Projectile_Torpedo_Impact) {
        proj = map.getTorpedoProjectile(packet.projectile_index);
        forceHull = true;
    }
    const glm::vec3 impactModelSpacePosition = glm::vec3(packet.impactX, packet.impactY, packet.impactZ);

    if (packet.shields) {
        auto* shields = static_cast<ShipSystemShields*>(getShipSystem(ShipSystemType::Shields));
        if (shields) {
            const glm::vec3 impactModelSpacePosition = glm::vec3(packet.impactX, packet.impactY, packet.impactZ);
            shields->receiveHit(normal, impactModelSpacePosition, rad, time, packet.damage, packet.shield_side, true);
        }
    }else{
        auto* hull = static_cast<ShipSystemHull*>(getShipSystem(ShipSystemType::Hull));
        if (hull) {
            hull->receiveHit(normal, impactModelSpacePosition, rad, packet.damage, static_cast<size_t>(packet.model_index), forceHull, true);
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
    for (auto& tuple : m_DamageDecals) {
        auto* decal = std::get<0>(tuple);
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

const bool Ship::isDestroyed() const {
    return (m_State == ShipState::JustFlaggedForDestruction ||
            m_State == ShipState::UndergoingDestruction ||
            m_State == ShipState::JustFlaggedAsFullyDestroyed ||
            m_State == ShipState::Destroyed ||
            m_State == ShipState::JustFlaggedToRespawn || 
            m_State == ShipState::UndergoingRespawning ||
            m_State == ShipState::WaitingForServerToRespawnMe
    ) ? true : false;
}
const bool Ship::isFullyDestroyed() const {
    return (m_State == ShipState::JustFlaggedAsFullyDestroyed ||
            m_State == ShipState::Destroyed ||
            m_State == ShipState::JustFlaggedToRespawn ||
            m_State == ShipState::UndergoingRespawning ||
            m_State == ShipState::WaitingForServerToRespawnMe
    ) ? true : false;
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
    if (IsPlayer() && Engine::isKeyDownOnce(KeyboardKey::Space)) {
        //foldWingsUp();
        //foldWingsDown();
        setState(ShipState::UndergoingDestruction);
    }
}