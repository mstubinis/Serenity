#include "PhaserBeam.h"
#include "../map/Map.h"
#include "../ResourceManifest.h"
#include "../Ship.h"
#include "../Helper.h"
#include "../networking/Packet.h"

#include <ecs/Components.h>
#include <core/engine/math/Engine_Math.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_access.hpp>
#include <glm/gtx/norm.hpp>
#include <core/engine/sounds/Engine_Sounds.h>
#include <core/engine/mesh/Mesh.h>
#include <core/engine/lights/Lights.h>

#include <core/engine/Engine.h>
#include <core/engine/materials/Material.h>
#include <core/engine/scene/Camera.h>

#include "../ships/shipSystems/ShipSystemShields.h"
#include "../ships/shipSystems/ShipSystemHull.h"

#include <core/engine/renderer/Decal.h>

#include "../particles/Sparks.h"
#include <core/engine/renderer/particles/ParticleEmitter.h>

using namespace Engine;
using namespace std;

struct PhaserBeamCollisionFunctor final { void operator()(CollisionCallbackEventData& data) const {
    auto phaserShipVoid = data.ownerBody.getUserPointer1();
    auto otherShipVoid = data.otherBody.getUserPointer1();
    if (otherShipVoid && phaserShipVoid) {
        if (otherShipVoid != phaserShipVoid) {//dont hit ourselves!
            Ship* otherShip = static_cast<Ship*>(otherShipVoid);
            if (otherShip) {
                auto& weapon = *static_cast<PhaserBeam*>(data.ownerBody.getUserPointer2());
                auto& source = static_cast<Ship*>(phaserShipVoid)->getMapKey();
                if (weapon.firingTime > 0.0f) {
                    auto& otherRotation = otherShip->getRotation();
                    Ship* sourceShip = static_cast<Ship*>(phaserShipVoid);
                    auto* shields = static_cast<ShipSystemShields*>(otherShip->getShipSystem(ShipSystemType::Shields));
                    auto* hull = static_cast<ShipSystemHull*>(otherShip->getShipSystem(ShipSystemType::Hull));
                    auto modelSpacePosition = glm::vec3((glm_vec3(data.otherHit) - data.otherBody.position()) * otherRotation);
                    auto finalDamage = static_cast<float>(Resources::dt()) * weapon.damage;

                    glm::vec3 localNormal = data.normalOnB * glm::quat(otherRotation);

                    if (shields && data.otherBody.getUserPointer() == shields) {
                        const uint shieldSide = static_cast<uint>(shields->getImpactSide(modelSpacePosition));
                        if (shields->getHealthCurrent(shieldSide) > 0) {
                            if (weapon.firingTimeShieldGraphicPing > 0.2f) {
                                shields->receiveHit(source, localNormal, modelSpacePosition, weapon.impactRadius, weapon.impactTime, finalDamage, shieldSide, true);
                                weapon.firingTimeShieldGraphicPing = 0.0f;
                            }else{
                                shields->receiveHit(source, localNormal, modelSpacePosition, weapon.impactRadius, weapon.impactTime, finalDamage, shieldSide, false);
                            }
                            return;
                        }
                    }
                    if (hull && data.otherBody.getUserPointer() == hull) {
                        if (weapon.firingTimeShieldGraphicPing > 1.0f) {
                            hull->receiveHit(source, localNormal, modelSpacePosition, weapon.impactRadius, finalDamage, data.otherModelInstanceIndex, true, true);

                            Map& map = static_cast<Map&>(otherShip->entity().scene());
                            ParticleEmitter emitter_(*Sparks::Spray, map, 0.1f, otherShip);
                            glm_quat q = glm_quat(1.0, 0.0, 0.0, 0.0);
                            Engine::Math::alignTo(q, -localNormal);
                            emitter_.setPosition(data.otherHit);
                            emitter_.setRotation(q);
                            auto* emitter = map.addParticleEmitter(emitter_);
                            if (emitter) {
                                otherShip->m_EmittersDestruction.push_back(std::make_tuple(emitter, data.otherModelInstanceIndex, modelSpacePosition, q));
                            }


                            weapon.firingTimeShieldGraphicPing = 0.0f;
                        }else{
                            hull->receiveHit(source, localNormal, modelSpacePosition, weapon.impactRadius, finalDamage, data.otherModelInstanceIndex, false, false);
                        }
                    }
                }
            }
        }
    }
}};

struct PhaserBeamInstanceBindFunctor { void operator()(EngineResource* r) const {
    //glDepthMask(GL_TRUE);
    auto& i = *static_cast<ModelInstance*>(r);
    Entity& parent = i.parent();
    auto& body = *parent.getComponent<ComponentBody>();
    PhaserBeam& beam = *static_cast<PhaserBeam*>(i.getUserPointer());

    glm::mat4 parentModel = body.modelMatrixRendering();
    glm::mat4 model = parentModel * i.modelMatrix();
    glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(model)));

    Renderer::sendUniform4Safe("Object_Color", i.color());
    Renderer::sendUniform3Safe("Gods_Rays_Color", i.godRaysColor());
    Renderer::sendUniform1Safe("AnimationPlaying", 0);
    Renderer::sendUniformMatrix4Safe("Model", model);
    Renderer::sendUniformMatrix3Safe("NormalMatrix", normalMatrix);

    auto& mesh = *(Mesh*)ResourceManifest::PhaserBeamMesh.get();
    mesh.modifyVertices(0, beam.modPts, MeshModifyFlags::Default);
    mesh.modifyVertices(1, beam.modUvs, MeshModifyFlags::Default | MeshModifyFlags::UploadToGPU);
}};
struct PhaserBeamInstanceUnbindFunctor { void operator()(EngineResource* r) const {
    //glDepthMask(GL_FALSE);
}};

PhaserBeam::PhaserBeam(Ship& ship, Map& map, const glm_vec3& position, const glm_vec3& forward, const float& arc, vector<glm::vec3>& windupPts, const float& damage, const float& _chargeTimerSpeed, const float& _firingTime, const float& _impactRadius, const float& _impactTime, const float& _volume, const uint& _maxCharges,const float& _rechargeTimePerRound, const unsigned int& _modelIndex, const float& endpointExtraScale, const float& beamSizeExtraScale, const float& RangeInKM, const float& BeamLaunchSpeed) : PrimaryWeaponBeam(WeaponType::PhaserBeam, ship, map, position, forward, arc, damage, _impactRadius, _impactTime, _volume, windupPts, _maxCharges, _rechargeTimePerRound, _chargeTimerSpeed, _firingTime, _modelIndex, endpointExtraScale, beamSizeExtraScale, RangeInKM, BeamLaunchSpeed){
    firstWindupGraphic = map.createEntity();
    secondWindupGraphic = map.createEntity();
    const auto photonOrange = glm::vec4(1.0f, 0.55f, 0.0f, 1.0f);
    const auto photonYellow = glm::vec4(1.0f, 0.85f, 0.0f, 1.0f);


    auto* model = beamGraphic.addComponent<ComponentModel>(ResourceManifest::PhaserBeamMesh, ResourceManifest::PhaserBeamMaterial, ShaderProgram::Forward, RenderStage::ForwardParticles);
    auto& beamModelInstance = model->getModel(0);
    beamModelInstance.hide();
    beamModelInstance.setScale(BEAM_SIZE_DEFAULT * additionalBeamSizeScale);

    auto& firstWindupBody = *firstWindupGraphic.addComponent<ComponentBody>();
    auto& secondWindupBody = *secondWindupGraphic.addComponent<ComponentBody>();
    auto& firstWindupModel = *firstWindupGraphic.addComponent<ComponentModel>(Mesh::Plane, (Material*)(ResourceManifest::TorpedoGlowMaterial).get(), ShaderProgram::Forward, RenderStage::ForwardParticles);
    auto& secondWindupModel = *secondWindupGraphic.addComponent<ComponentModel>(Mesh::Plane, (Material*)(ResourceManifest::TorpedoGlowMaterial).get(), ShaderProgram::Forward, RenderStage::ForwardParticles);

    auto& firstModel = firstWindupModel.getModel();
    auto& secondModel = secondWindupModel.getModel();

    firstModel.setScale(BEAM_SIZE_DEFAULT_END_POINT * additionalEndPointScale);
    secondModel.setScale(BEAM_SIZE_DEFAULT_END_POINT * additionalEndPointScale);

    firstModel.setColor(photonOrange);
    secondModel.setColor(photonOrange);

    auto& shipBody = *ship.getComponent<ComponentBody>();
    const glm::vec3 finalPosition = shipBody.position() + Math::rotate_vec3(shipBody.rotation(), position);

    firstWindupLight = new PointLight(finalPosition, &map);
    firstWindupLight->setColor(photonOrange);
    firstWindupLight->setAttenuation(LightRange::_20);
    firstWindupLight->deactivate();

    secondWindupLight = new PointLight(finalPosition, &map);
    secondWindupLight->setColor(photonOrange);
    secondWindupLight->setAttenuation(LightRange::_20);
    secondWindupLight->deactivate();

    beamModelInstance.setUserPointer(this);
    //beamModelInstance.setColor(photonOrange);
    beamModelInstance.setCustomBindFunctor(PhaserBeamInstanceBindFunctor());
    beamModelInstance.setCustomUnbindFunctor(PhaserBeamInstanceUnbindFunctor());

    beamLight->setColor(photonOrange);

    auto& beamEndBody = *beamEndPointGraphic.getComponent<ComponentBody>();
    auto& beamEndModel = *beamEndPointGraphic.getComponent<ComponentModel>();
    auto& beamModelEnd = beamEndModel.getModel(0);
    beamModelEnd.setColor(photonOrange);

    beamEndBody.setUserPointer(this);
    beamEndBody.setUserPointer1(&ship);
    beamEndBody.setUserPointer2(this);
    beamEndBody.setPosition(99999999999.9f);
    beamEndBody.setCollisionFunctor(PhaserBeamCollisionFunctor());
}
PhaserBeam::~PhaserBeam() {
    firstWindupGraphic.destroy();
    secondWindupGraphic.destroy();
    firstWindupLight->destroy();
    secondWindupLight->destroy();
    SAFE_DELETE(firstWindupLight);
    SAFE_DELETE(secondWindupLight);
}
const bool PhaserBeam::fire(const double& dt, const glm_vec3& chosen_target_pt) {
    auto res2 = isInArc(target, arc);
    targetCoordinates = chosen_target_pt;
    if (res2) {
        auto& targetBody = *target->getComponent<ComponentBody>();
        auto& shipBody = *ship.getComponent<ComponentBody>();
        const auto shipRotation = ship.getRotation();
        const auto shipPosition = ship.getPosition();
        const auto launcherPosition = shipPosition + (shipRotation * position);
        const auto distSquared = glm::distance2(launcherPosition, targetBody.position());

        if (distSquared < rangeInKMSquared) {
            const auto res = PrimaryWeaponBeam::fire(dt, chosen_target_pt);
            if (res) {
                return forceFire(dt);
            }
        }
    }
    return false;
}
const bool PhaserBeam::forceFire(const double& dt) {
    if (state == BeamWeaponState::Off) {
        auto& shipBody = *ship.getComponent<ComponentBody>();
        auto shipMatrix = shipBody.modelMatrix();
        shipMatrix = glm::translate(shipMatrix, position);
        const glm::vec3 finalPosition = glm::vec3(shipMatrix[3][0], shipMatrix[3][1], shipMatrix[3][2]);
        soundEffect = Engine::Sound::playEffect(ResourceManifest::SoundPhaserBeam);
        if (soundEffect) {
            soundEffect->setVolume(volume);
            soundEffect->setPosition(finalPosition);
            soundEffect->setAttenuation(0.6f);
        }
        state = BeamWeaponState::JustStarted;
        return true;
    }
    return false;
}
void PhaserBeam::update(const double& dt) {
    if (state == BeamWeaponState::JustStarted) {
        internal_update_initial_firing(dt);
    }else if (state == BeamWeaponState::WindingUp) {
        internal_update_winding_up(dt);
    }else if (state == BeamWeaponState::Firing) {  
        internal_update_firing(dt);
    }else if (state == BeamWeaponState::JustTurnedOff) {
        internal_update_ending(dt);
    }
    PrimaryWeaponBeam::update(dt);
}