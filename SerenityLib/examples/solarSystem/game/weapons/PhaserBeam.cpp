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

#include "../ships/shipSystems/ShipSystemShields.h"
#include "../ships/shipSystems/ShipSystemHull.h"

#include <core/engine/renderer/Decal.h>

using namespace Engine;
using namespace std;

struct PhaserBeamCollisionFunctor final { void operator()(ComponentBody& owner, const glm::vec3& ownerHit, ComponentBody& other, const glm::vec3& otherHit, const glm::vec3& normal) const {
    auto phaserShipVoid = owner.getUserPointer1();
    auto otherShipVoid = other.getUserPointer1();
    if (otherShipVoid && phaserShipVoid) {
        if (otherShipVoid != phaserShipVoid) {//dont hit ourselves!
            Ship* otherShip = static_cast<Ship*>(otherShipVoid);
            if (otherShip) {
                auto& weapon = *static_cast<PhaserBeam*>(owner.getUserPointer2());
                if (weapon.firingTime > 0.0f) {
                    Ship* sourceShip = static_cast<Ship*>(phaserShipVoid);
                    auto* shields = static_cast<ShipSystemShields*>(otherShip->getShipSystem(ShipSystemType::Shields));
                    auto* hull = static_cast<ShipSystemHull*>(otherShip->getShipSystem(ShipSystemType::Hull));
                    auto local = otherHit - glm::vec3(other.position());
                    auto finalDamage = static_cast<float>(Resources::dt()) * weapon.damage;
                    if (shields && other.getUserPointer() == shields) {
                        const uint shieldSide = static_cast<uint>(shields->getImpactSide(local));
                        if (shields->getHealthCurrent(shieldSide) > 0) {
                            if (weapon.firingTimeShieldGraphicPing > 0.2f) {
                                shields->receiveHit(normal, local, weapon.impactRadius, weapon.impactTime, finalDamage, shieldSide, true);
                                weapon.firingTimeShieldGraphicPing = 0.0f;
                            }else{
                                shields->receiveHit(normal, local, weapon.impactRadius, weapon.impactTime, finalDamage, shieldSide, false);
                            }
                            return;
                        }
                    }
                    if (hull && other.getUserPointer() == hull) {
                        if (weapon.firingTimeShieldGraphicPing > 1.0f) {
                            hull->receiveHit(normal, local, weapon.impactRadius, weapon.impactTime, finalDamage, true, true);
                            weapon.firingTimeShieldGraphicPing = 0.0f;
                        }else{
                            hull->receiveHit(normal, local, weapon.impactRadius, weapon.impactTime, finalDamage, false, false);
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

PhaserBeam::PhaserBeam(Ship& ship, Map& map, const glm_vec3& position, const glm_vec3& forward, const float& arc, vector<glm::vec3>& windupPts, const float& damage, const float& _chargeTimerSpeed, const float& _firingTime, const float& _impactRadius, const float& _impactTime, const float& _volume, const uint& _maxCharges,const float& _rechargeTimePerRound, const unsigned int& _modelIndex, const float& endpointExtraScale, const float& beamSizeExtraScale, const float& RangeInKM) : PrimaryWeaponBeam(WeaponType::PhaserBeam, ship, map, position, forward, arc, damage, _impactRadius, _impactTime, _volume, windupPts, _maxCharges, _rechargeTimePerRound, _chargeTimerSpeed, _firingTime, _modelIndex, endpointExtraScale, beamSizeExtraScale, RangeInKM){
    firstWindupGraphic = map.createEntity();
    secondWindupGraphic = map.createEntity();

    auto* model = beamGraphic.addComponent<ComponentModel>(ResourceManifest::PhaserBeamMesh, ResourceManifest::PhaserBeamMaterial, ShaderProgram::Forward, RenderStage::ForwardParticles);
    auto& beamModel1 = model->getModel(0);
    beamModel1.hide();
    beamModel1.setScale(0.095f * additionalBeamSizeScale);

    auto& firstWindupBody = *firstWindupGraphic.addComponent<ComponentBody>();
    auto& secondWindupBody = *secondWindupGraphic.addComponent<ComponentBody>();
    auto& firstWindupModel = *firstWindupGraphic.addComponent<ComponentModel>(Mesh::Plane, (Material*)(ResourceManifest::TorpedoGlowMaterial).get(), ShaderProgram::Forward, RenderStage::ForwardParticles);
    auto& secondWindupModel = *secondWindupGraphic.addComponent<ComponentModel>(Mesh::Plane, (Material*)(ResourceManifest::TorpedoGlowMaterial).get(), ShaderProgram::Forward, RenderStage::ForwardParticles);

    auto& firstModel = firstWindupModel.getModel();
    auto& secondModel = secondWindupModel.getModel();

    firstModel.setScale(0.095f * additionalEndPointScale);
    secondModel.setScale(0.095f * additionalEndPointScale);

    const auto photonOrange = glm::vec4(1.0f, 0.55f, 0.0f, 1.0f);
    const auto photonYellow = glm::vec4(1.0f, 0.85f, 0.0f, 1.0f);

    firstModel.setColor(photonOrange);
    secondModel.setColor(photonOrange);

    auto& shipBody = *ship.getComponent<ComponentBody>();
    const glm::vec3 finalPosition = shipBody.position() + Math::rotate_vec3(shipBody.rotation(), position);

    firstWindupLight = new PointLight(finalPosition, &map);
    firstWindupLight->setColor(photonOrange);
    firstWindupLight->setAttenuation(LightRange::_7);
    firstWindupLight->deactivate();

    secondWindupLight = new PointLight(finalPosition, &map);
    secondWindupLight->setColor(photonOrange);
    secondWindupLight->setAttenuation(LightRange::_7);
    secondWindupLight->deactivate();

    auto& beamModel = *beamGraphic.getComponent<ComponentModel>();
    auto& beamModelOne = beamModel.getModel();
    beamModelOne.setUserPointer(this);
    //beamModelOne.setColor(photonOrange);
    beamModelOne.setCustomBindFunctor(PhaserBeamInstanceBindFunctor());
    beamModelOne.setCustomUnbindFunctor(PhaserBeamInstanceUnbindFunctor());

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
            soundEffect->setAttenuation(0.8f);
        }
        state = BeamWeaponState::JustStarted;
        return true;
    }
    return false;
}
void PhaserBeam::update(const double& dt) {
    const auto fdt = static_cast<float>(dt);
    glm::vec3 firstWindupPos;
    glm::vec3 secondWindupPos;
    auto& firstWindupBody = *firstWindupGraphic.getComponent<ComponentBody>();
    auto& secondWindupBody = *secondWindupGraphic.getComponent<ComponentBody>();
    auto& firstWindupLightBody = *firstWindupLight->getComponent<ComponentBody>();
    auto& secondWindupLightBody = *secondWindupLight->getComponent<ComponentBody>();

    auto& firstWindupModel = *firstWindupGraphic.getComponent<ComponentModel>();
    auto& secondWindupModel = *secondWindupGraphic.getComponent<ComponentModel>();
    auto& beamLightBody = *beamLight->getComponent<ComponentBody>();

    auto& beamEndBody = *beamEndPointGraphic.getComponent<ComponentBody>();
    auto& beamEndModel = *beamEndPointGraphic.getComponent<ComponentModel>();
    auto& body = *beamGraphic.getComponent<ComponentBody>();

    auto& beamModel = *beamGraphic.getComponent<ComponentModel>();
    auto& beamModelOne = beamModel.getModel(0);

    const auto factor = 235.0f;

    if (state == BeamWeaponState::JustStarted) {
        #pragma region JustStarted
        firstWindupModel.show();
        secondWindupModel.show();
        firstWindupLight->activate();
        secondWindupLight->activate();
        beamLight->activate();
        state = BeamWeaponState::WindingUp;
        #pragma endregion
    }
    else if (state == BeamWeaponState::WindingUp) {
        #pragma region WindingUp
        auto& cam = *firstWindupBody.getOwner().scene().getActiveCamera();
        auto camRotation = cam.getOrientation();

        firstWindupModel.getModel().setOrientation(camRotation);
        secondWindupModel.getModel().setOrientation(camRotation);
        beamEndModel.getModel().setOrientation(camRotation);

        const auto chargeSpeedModifier = fdt * chargeTimerSpeed;
        chargeTimer += chargeSpeedModifier;

        //place the windups properly
        const auto shipRotation = ship.getRotation();
        const auto shipPosition = ship.getPosition();
        const auto launcherPosition = shipPosition + Math::rotate_vec3(shipRotation, position);
        if (soundEffect) {
            soundEffect->setPosition(launcherPosition);
        }
        if (windupPoints.size() == 1) {
            firstWindupPos = secondWindupPos = (shipPosition + Math::rotate_vec3(shipRotation, windupPoints[0]));
        }
        else {
            const auto halfCharge = chargeTimer * 0.5f;
            firstWindupPos = shipPosition + Math::rotate_vec3(shipRotation, Math::polynomial_interpolate_cubic(windupPoints, halfCharge));
            secondWindupPos = shipPosition + Math::rotate_vec3(shipRotation, Math::polynomial_interpolate_cubic(windupPoints, 1.0f - halfCharge));
        }
        firstWindupBody.setPosition(firstWindupPos);
        secondWindupBody.setPosition(secondWindupPos);
        firstWindupLightBody.setPosition(firstWindupPos);
        secondWindupLightBody.setPosition(secondWindupPos);
        if (chargeTimer >= 1.0f) {
            state = BeamWeaponState::Firing;
            chargeTimer = 1.0f;
            --numRounds;
            return;
        }
        #pragma endregion
    }
    else if (state == BeamWeaponState::Firing) {
        #pragma region Firing
        auto& targetBody = *target->getComponent<ComponentBody>();
        const glm::vec3 tgt = glm::vec3(targetBody.position()) + targetCoordinates;

        beamEndModel.show();
        beamModelOne.show();
        beamModelOne.forceRender(true);

        const auto shipRotation = ship.getRotation();
        const auto shipPosition = ship.getPosition();
        if (windupPoints.size() == 1) {
            firstWindupPos = secondWindupPos = (shipPosition + Math::rotate_vec3(shipRotation, windupPoints[0]));
        }else{
            const auto halfCharge = chargeTimer * 0.5f;
            firstWindupPos = shipPosition + Math::rotate_vec3(shipRotation, Math::polynomial_interpolate_cubic(windupPoints, halfCharge));
            secondWindupPos = shipPosition + Math::rotate_vec3(shipRotation, Math::polynomial_interpolate_cubic(windupPoints, 1.0f - halfCharge));
        }

        //bring the windups closer to the target at the end
        auto end = (firingTimeMax - firingTime);
        //end - big to zero
        //dir from target point to startPos
        const auto time = firingTime * factor;
        const auto time2 = end * factor;

        auto startPos = firstWindupPos;
        const auto startPos2 = firstWindupPos;
        auto BigDirTargetToStart = tgt - startPos;
        auto lenTarToStart = glm::length(BigDirTargetToStart);
        auto dirTargetToStart = glm::normalize(BigDirTargetToStart);

        dirTargetToStart *= glm::min(time2, lenTarToStart);
        startPos = tgt - dirTargetToStart;

        body.setPosition(startPos);
        firstWindupBody.setPosition(startPos);
        secondWindupBody.setPosition(startPos);
        firstWindupLightBody.setPosition(startPos);
        secondWindupLightBody.setPosition(startPos);

        glm_quat q;

        auto bigDir = startPos2 - tgt;
        auto len = glm::length(bigDir);
        auto dir = glm::normalize(bigDir);
        Math::alignTo(q, -dir);
        beamModelOne.setOrientation(q);

        dir *= glm::min(time, len);
        glm::vec3 realTargetPos = startPos2 - dir;

        auto* shipShields = static_cast<ShipSystemShields*>(ship.getShipSystem(ShipSystemType::Shields));
        auto* shipHull = static_cast<ShipSystemHull*>(ship.getShipSystem(ShipSystemType::Hull));

        vector<Entity> ignored;
        ignored.reserve(3);
        ignored.push_back(shipShields->getEntity());
        ignored.push_back(shipHull->getEntity());
        ignored.push_back(ship.entity());

        unsigned short group = CollisionFilter::_Custom_2;
        unsigned short mask = -1;
        mask = mask & ~CollisionFilter::_Custom_4; //do not ray cast against the convex hull (custom 4)
        mask = mask & ~CollisionFilter::_Custom_5; //do not ray cast against the shields if they are down
        mask = mask & ~CollisionFilter::_Custom_2; //and ignore other weapons!

        firingTime += fdt;
        firingTimeShieldGraphicPing += fdt;

        auto rayCastPoints = Physics::rayCast(startPos, tgt, ignored, group, mask);
        Engine::RayCastResult* closest = nullptr;

        auto lambda_get_closest = [&](vector<Engine::RayCastResult>& vec, float& min_dist, uint& closest_index, Engine::RayCastResult*& closest_, const glm::vec3& startPos_) {
            for (uint i = 0; i < vec.size(); ++i) {
                const auto dist = glm::distance2(vec[i].hitPosition, startPos_);
                if (dist < min_dist) {
                    min_dist = dist;
                    closest_ = &const_cast<Engine::RayCastResult&>(vec[i]);
                    closest_index = i;
                }
            }
        };

        //get closest 2 points
        float minDist = 9999999999999.0f;
        uint closestIndex = 0;
        lambda_get_closest(rayCastPoints, minDist, closestIndex, closest, startPos);
        if (!closest || closest->hitNormal == glm::vec3(0.0f)) {
            state = BeamWeaponState::CoolingDown;
            return;
        }
        Ship* targetShip = dynamic_cast<Ship*>(target);
        if (targetShip) {
            auto* targetShields = static_cast<ShipSystemShields*>(targetShip->getShipSystem(ShipSystemType::Shields));
            const auto side = targetShields->getImpactSide(closest->hitPosition - glm::vec3(targetShip->getPosition()));
            if (targetShields->getHealthCurrent(side) <= 0.0f) {
                rayCastPoints.erase(rayCastPoints.begin() + closestIndex);
            }
            minDist = 9999999999999.0f;
            closestIndex = 0;
            lambda_get_closest(rayCastPoints, minDist, closestIndex, closest, startPos);
        }
        const glm::vec3 midpt = Math::midpoint(startPos, realTargetPos);

        beamLightBody.setPosition(midpt);
        beamLightBody.setRotation(q);
        glm::vec3 finPos;
        if (time >= len) {
            finPos = closest->hitPosition;
        }
        else {
            finPos = realTargetPos;
        }
        beamEndBody.setPosition(finPos);
        const auto len2 = glm::length(finPos - startPos);
        beamLight->setRodLength(len2);
        modifyBeamMesh(beamModel, len2);
        auto x = (-1.0f * (glm::length(finPos - startPos) / factor)) + firingTimeMax;
        if ((firingTime + fdt) >= x) {
            state = BeamWeaponState::CoolingDown;
            return;
        }
        if (!isInArc(target, arc + 5.0f)) {
            firingTime = x;
            state = BeamWeaponState::CoolingDown;
        }
        #pragma endregion
    }
    else if (state == BeamWeaponState::CoolingDown) {
        #pragma region CoolingDown
        auto& targetBody = *target->getComponent<ComponentBody>();
        const auto tgt = glm::vec3(targetBody.position()) + targetCoordinates;

        firstWindupModel.hide();
        secondWindupModel.hide();
        const auto inf = glm::vec3(9999999999999.9f);
        firstWindupBody.setPosition(inf);
        secondWindupBody.setPosition(inf);

        beamEndModel.show();
        beamModelOne.show();
        beamModelOne.forceRender(true);

        const auto shipRotation = ship.getRotation();
        const auto shipPosition = ship.getPosition();
        if (windupPoints.size() == 1) {
            firstWindupPos = secondWindupPos = (shipPosition + Math::rotate_vec3(shipRotation, windupPoints[0]));
        }else{
            const auto halfCharge = chargeTimer * 0.5f;
            firstWindupPos = shipPosition + Math::rotate_vec3(shipRotation, Engine::Math::polynomial_interpolate_cubic(windupPoints, halfCharge));
            secondWindupPos = shipPosition + Math::rotate_vec3(shipRotation, Engine::Math::polynomial_interpolate_cubic(windupPoints, 1.0f - halfCharge));
        }
        //bring the windups closer to the target at the end
        auto end = (firingTimeMax - firingTime);
        //end - big to zero
        //dir from target point to startPos
        const auto time = firingTime * factor;
        const auto time2 = end * factor;

        auto startPos = firstWindupPos;
        const auto startPos2 = firstWindupPos;
        auto BigDirTargetToStart = tgt - startPos;
        auto lenTarToStart = glm::length(BigDirTargetToStart);
        auto TargetToStart = glm::normalize(BigDirTargetToStart) * glm::min(time2, lenTarToStart);
        startPos = tgt - TargetToStart;

        body.setPosition(startPos);
        firstWindupLightBody.setPosition(startPos);
        secondWindupLightBody.setPosition(startPos);

        glm_quat q;
        auto bigDir = startPos2 - tgt;
        auto len = glm::length(bigDir);
        auto dir = glm::normalize(bigDir);
        Math::alignTo(q, -dir);
        beamModelOne.setOrientation(q);
        dir *= glm::min(time, len);
        glm::vec3 realTargetPos = startPos2 - dir;

        auto* shipShields = static_cast<ShipSystemShields*>(ship.getShipSystem(ShipSystemType::Shields));
        auto* shipHull = static_cast<ShipSystemHull*>(ship.getShipSystem(ShipSystemType::Hull));

        vector<Entity> ignored;
        ignored.reserve(3);
        ignored.push_back(shipShields->getEntity());
        ignored.push_back(shipHull->getEntity());
        ignored.push_back(ship.entity());

        unsigned short group = CollisionFilter::_Custom_2;
        unsigned short mask = -1;
        //custom 1 is shields
        mask = mask & ~CollisionFilter::_Custom_4; //do not ray cast against the convex hull (custom 4)
        mask = mask & ~CollisionFilter::_Custom_5; //do not ray cast against the shields if they are down
        mask = mask & ~CollisionFilter::_Custom_2; //and ignore other weapons!


        firingTime += fdt;
        firingTimeShieldGraphicPing += fdt;

        auto rayCastPoints = Physics::rayCast(startPos, tgt, ignored, group, mask);
        Engine::RayCastResult* closest = nullptr;

        auto lambda_get_closest = [&](vector<Engine::RayCastResult>& vec, float& min_dist, uint& closest_index, Engine::RayCastResult*& closest_, const glm::vec3& startPos_) {
            for (uint i = 0; i < vec.size(); ++i) {
                const auto dist = glm::distance2(vec[i].hitPosition, startPos_);
                if (dist < min_dist) {
                    min_dist = dist;
                    closest_ = &const_cast<Engine::RayCastResult&>(vec[i]);
                    closest_index = i;
                }
            }
        };

        //get closest 2 points
        float minDist = 9999999999999.0f;
        uint closestIndex = 0;
        lambda_get_closest(rayCastPoints, minDist, closestIndex, closest, startPos);
        if (!closest || closest->hitNormal == glm::vec3(0.0f)) {
            state = BeamWeaponState::CoolingDown;
            return;
        }
        Ship* targetShip = dynamic_cast<Ship*>(target);
        if (targetShip) {
            auto* targetShields = static_cast<ShipSystemShields*>(targetShip->getShipSystem(ShipSystemType::Shields));
            const auto side = targetShields->getImpactSide(closest->hitPosition - glm::vec3(targetShip->getPosition()));
            if (targetShields->getHealthCurrent(side) <= 0.0f) {
                rayCastPoints.erase(rayCastPoints.begin() + closestIndex);
            }
            minDist = 9999999999999.0f;
            closestIndex = 0;
            lambda_get_closest(rayCastPoints, minDist, closestIndex, closest, startPos);
        }
        const glm::vec3 midpt = Math::midpoint(startPos, realTargetPos);

        beamLightBody.setPosition(midpt);
        beamLightBody.setRotation(q);
        glm::vec3 finPos;
        if (time >= len) {
            finPos = closest->hitPosition;
        }else{
            finPos = realTargetPos;
        }
        beamEndBody.setPosition(finPos);
        const auto len2 = glm::length(finPos - startPos);
        beamLight->setRodLength(len2);
        modifyBeamMesh(beamModel, len2);
        if (firingTime >= firingTimeMax) {
            state = BeamWeaponState::JustTurnedOff;
            return;
        }
        #pragma endregion
    }
    else if (state == BeamWeaponState::JustTurnedOff) {
        #pragma region JustTurnedOff
        firingTime = 0.0f;
        chargeTimer = 0.0f;
        firstWindupModel.hide();
        secondWindupModel.hide();
        beamModel.hide();
        beamEndModel.hide();

        firstWindupLight->deactivate();
        secondWindupLight->deactivate();
        beamLight->deactivate();
        if (soundEffect) {
            soundEffect->stop();
        }
        state = BeamWeaponState::Off;
        return;
        #pragma endregion
    }
    PrimaryWeaponBeam::update(dt);
}