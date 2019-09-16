#include "PlasmaBeam.h"
#include "../map/Map.h"
#include "../ResourceManifest.h"
#include "../Ship.h"
#include "../Helper.h"

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

struct PlasmaBeamCollisionFunctor final {
    void operator()(ComponentBody& owner, const glm::vec3& ownerHit, ComponentBody& other, const glm::vec3& otherHit, const glm::vec3& normal) const {
        auto phaserShipVoid = owner.getUserPointer1();
        auto otherShipVoid = other.getUserPointer1();
        if (otherShipVoid && phaserShipVoid) {
            if (otherShipVoid != phaserShipVoid) {//dont hit ourselves!
                Ship* otherShip = static_cast<Ship*>(otherShipVoid);
                if (otherShip) {
                    PlasmaBeam& Plasma = *static_cast<PlasmaBeam*>(owner.getUserPointer2());
                    if (Plasma.firingTime > 0.0f) {
                        Ship* sourceShip = static_cast<Ship*>(phaserShipVoid);
                        auto* shields = static_cast<ShipSystemShields*>(otherShip->getShipSystem(ShipSystemType::Shields));
                        auto* hull = static_cast<ShipSystemHull*>(otherShip->getShipSystem(ShipSystemType::Hull));
                        auto local = otherHit - other.position();

                        auto finalDamage = static_cast<float>(Resources::dt()) * Plasma.damage;

                        if (shields && shields->getHealthCurrent() > 0 && other.getUserPointer() == shields) {
                            if (Plasma.firingTimeShieldGraphicPing > 0.2f) {
                                shields->receiveHit(normal, local, Plasma.impactRadius, Plasma.impactTime, finalDamage, true);
                                Plasma.firingTimeShieldGraphicPing = 0.0f;
                            }else{
                                shields->receiveHit(normal, local, Plasma.impactRadius, Plasma.impactTime, finalDamage, false);
                            }
                            return;
                        }
                        if (hull && other.getUserPointer() == hull) {
                            if (Plasma.firingTimeShieldGraphicPing > 1.0f) {
                                hull->receiveHit(normal, local, Plasma.impactRadius, Plasma.impactTime, finalDamage, true, true);
                                Plasma.firingTimeShieldGraphicPing = 0.0f;
                            }else{
                                hull->receiveHit(normal, local, Plasma.impactRadius, Plasma.impactTime, finalDamage, false, false);
                            }
                        }
                    }
                }
            }
        }
    }
};

struct PlasmaBeamInstanceBindFunctor { void operator()(EngineResource* r) const {
    //glDepthMask(GL_TRUE);
    auto& i = *static_cast<ModelInstance*>(r);
    Entity& parent = i.parent();
    auto& body = *parent.getComponent<ComponentBody>();
    PlasmaBeam& beam = *static_cast<PlasmaBeam*>(i.getUserPointer());

    glm::mat4 parentModel = body.modelMatrix();
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
struct PlasmaBeamInstanceUnbindFunctor { void operator()(EngineResource* r) const {
    //glDepthMask(GL_FALSE);
}};

PlasmaBeam::PlasmaBeam(Ship& ship, Map& map, const glm::vec3& position, const glm::vec3& forward, const float& arc, vector<glm::vec3>& windupPts, const float& damage, const float& _chargeTimerSpeed, const float& _firingTime, const float& _impactRadius, const float& _impactTime, const float& _volume, const uint& _maxCharges, const float& _rechargeTimePerRound) : PrimaryWeaponBeam(ship, map, position, forward, arc, damage, _impactRadius, _impactTime, _volume, windupPts, _maxCharges, _rechargeTimePerRound, _chargeTimerSpeed, _firingTime), m_Map(map) {
    firstWindupGraphic = new EntityWrapper(map);
    secondWindupGraphic = new EntityWrapper(map);

    auto* model = beamGraphic->addComponent<ComponentModel>(ResourceManifest::PhaserBeamMesh, ResourceManifest::PlasmaBeamMaterial, ShaderProgram::Forward, RenderStage::ForwardParticles);
    auto& beamModel1 = model->getModel(0);
    beamModel1.hide();
    beamModel1.setScale(0.09f);

    auto& firstWindupBody = *firstWindupGraphic->addComponent<ComponentBody>();
    auto& secondWindupBody = *secondWindupGraphic->addComponent<ComponentBody>();
    auto& firstWindupModel = *firstWindupGraphic->addComponent<ComponentModel>(Mesh::Plane, (Material*)(ResourceManifest::TorpedoGlow2Material).get(), ShaderProgram::Forward, RenderStage::ForwardParticles);
    auto& secondWindupModel = *secondWindupGraphic->addComponent<ComponentModel>(Mesh::Plane, (Material*)(ResourceManifest::TorpedoGlow2Material).get(), ShaderProgram::Forward, RenderStage::ForwardParticles);

    auto& firstModel = firstWindupModel.getModel();
    auto& secondModel = secondWindupModel.getModel();

    firstModel.setScale(0.09f);
    secondModel.setScale(0.09f);

    const auto plasmaGreen = glm::vec4(0.0f, 0.93f, 0.6f, 1.0f);
    const auto plasmaTeal = glm::vec4(0.53f, 1.0f, 0.73f, 1.0f);

    firstModel.setColor(plasmaGreen);
    secondModel.setColor(plasmaGreen);

    auto& shipBody = *ship.getComponent<ComponentBody>();

    auto shipMatrix = shipBody.modelMatrix();
    shipMatrix = glm::translate(shipMatrix, position);
    const auto finalPosition = glm::vec3(shipMatrix[3][0], shipMatrix[3][1], shipMatrix[3][2]);

    firstWindupLight = new PointLight(finalPosition, &map);
    firstWindupLight->setColor(plasmaGreen);
    firstWindupLight->setAttenuation(LightRange::_7);
    firstWindupLight->deactivate();

    secondWindupLight = new PointLight(finalPosition, &map);
    secondWindupLight->setColor(plasmaGreen);
    secondWindupLight->setAttenuation(LightRange::_7);
    secondWindupLight->deactivate();

    auto& beamModel = *beamGraphic->getComponent<ComponentModel>();
    auto& beamModelOne = beamModel.getModel();
    beamModelOne.setUserPointer(this);
    //beamModelOne.setColor(plasmaGreen);
    beamModelOne.setCustomBindFunctor(PlasmaBeamInstanceBindFunctor());
    beamModelOne.setCustomUnbindFunctor(PlasmaBeamInstanceUnbindFunctor());

    beamLight->setColor(plasmaGreen);

    auto& beamEndBody = *beamEndPointGraphic->getComponent<ComponentBody>();
    auto& beamEndModel = *beamEndPointGraphic->getComponent<ComponentModel>();
    auto& beamModelEnd = beamEndModel.getModel(0);
    beamModelEnd.setColor(plasmaGreen);

    beamEndBody.setUserPointer(this);
    beamEndBody.setUserPointer1(&ship);
    beamEndBody.setUserPointer2(this);
    beamEndBody.setPosition(99999999999.9f);
    beamEndBody.setCollisionFunctor(PlasmaBeamCollisionFunctor());
}
PlasmaBeam::~PlasmaBeam() {
    firstWindupGraphic->destroy();
    secondWindupGraphic->destroy();
    firstWindupLight->destroy();
    secondWindupLight->destroy();
    SAFE_DELETE(firstWindupGraphic);
    SAFE_DELETE(secondWindupGraphic);
    SAFE_DELETE(firstWindupLight);
    SAFE_DELETE(secondWindupLight);
}
const bool PlasmaBeam::fire(const double& dt) {
    auto* target = ship.getTarget();
    auto res2 = isInArc(target, arc);
    if (res2) {
        auto& targetBody = *target->getComponent<ComponentBody>();
        auto& shipBody = *ship.getComponent<ComponentBody>();
        const auto shipRotation = ship.getRotation();
        const auto shipPosition = ship.getPosition();
        const auto launcherPosition = shipPosition + (shipRotation * position);
        const auto distSquared = glm::distance2(launcherPosition, targetBody.position());

        if (distSquared < 100 * 100) { //100 * 100 should be 10 KM
            const auto res = PrimaryWeaponBeam::fire(dt);
            if (res) {
                forceFire(dt);
                return true;
            }
        }
    }
    return false;
}
void PlasmaBeam::forceFire(const double& dt) {
    //move the two end flares towards the middle using the interpolation
    if (!isFiring) {

        auto& shipBody = *ship.getComponent<ComponentBody>();
        auto shipMatrix = shipBody.modelMatrix();
        shipMatrix = glm::translate(shipMatrix, position);
        const glm::vec3 finalPosition = glm::vec3(shipMatrix[3][0], shipMatrix[3][1], shipMatrix[3][2]);
        soundEffect = Engine::Sound::playEffect(ResourceManifest::SoundPlasmaBeam);
        if (soundEffect) {
            soundEffect->setVolume(volume);
            soundEffect->setPosition(finalPosition);
            soundEffect->setAttenuation(0.1f);
        }
        isFiring = true;
    }
}
void PlasmaBeam::update(const double& dt) {
    if (isFiring) {
        auto& firstWindupBody = *firstWindupGraphic->getComponent<ComponentBody>();
        auto& secondWindupBody = *secondWindupGraphic->getComponent<ComponentBody>();
        auto& firstWindupLightBody = *firstWindupLight->getComponent<ComponentBody>();
        auto& secondWindupLightBody = *secondWindupLight->getComponent<ComponentBody>();

        auto& firstWindupModel = *firstWindupGraphic->getComponent<ComponentModel>();
        auto& secondWindupModel = *secondWindupGraphic->getComponent<ComponentModel>();
        auto& beamModel = *beamGraphic->getComponent<ComponentModel>();
        auto& beamLightBody = *beamLight->getComponent<ComponentBody>();

        auto& beamEndBody = *beamEndPointGraphic->getComponent<ComponentBody>();
        auto& beamEndModel = *beamEndPointGraphic->getComponent<ComponentModel>();

        firstWindupModel.show();
        secondWindupModel.show();
        firstWindupLight->activate();
        secondWindupLight->activate();
        beamLight->activate();

        auto& cam = *firstWindupBody.getOwner().scene().getActiveCamera();
        auto camRotation = cam.getOrientation();

        firstWindupModel.getModel().setOrientation(camRotation);
        secondWindupModel.getModel().setOrientation(camRotation);
        beamEndModel.getModel().setOrientation(camRotation);

        const auto fdt = static_cast<float>(dt);
        const auto chargeSpeedModifier = fdt * chargeTimerSpeed;
        chargeTimer += chargeSpeedModifier;
        chargeTimer = glm::min(chargeTimer, 1.0f);

        //place the windups properly
        const auto shipRotation = ship.getRotation();
        const auto shipPosition = ship.getPosition();
        const auto launcherPosition = shipPosition + (shipRotation * position);
        if (soundEffect) {
            soundEffect->setPosition(launcherPosition);
        }

        glm::vec3 firstWindupPos;
        glm::vec3 secondWindupPos;
        if (windupPoints.size() == 1) {
            firstWindupPos = secondWindupPos = shipPosition + (shipRotation * windupPoints[0]);
        }
        else {
            const auto halfCharge = chargeTimer * 0.5f;
            firstWindupPos = shipPosition + (shipRotation * Engine::Math::polynomial_interpolate_cubic(windupPoints, halfCharge));
            secondWindupPos = shipPosition + (shipRotation * Engine::Math::polynomial_interpolate_cubic(windupPoints, 1.0f - halfCharge));
        }
        firstWindupBody.setPosition(firstWindupPos);
        secondWindupBody.setPosition(secondWindupPos);
        firstWindupLightBody.setPosition(firstWindupPos);
        secondWindupLightBody.setPosition(secondWindupPos);

        auto cancel = [&](PlasmaBeam& beam, ComponentModel& firstWindup, ComponentModel& secondWindup, PointLight& firstLight, PointLight& secondLight, RodLight& beamLight, ComponentModel& beamModel, ComponentModel& beamEndMdl) {
            beam.isFiring = false;
            beam.isFiringWeapon = false;
            beam.firingTime = 0.0f;
            beam.chargeTimer = 0.0f;
            firstWindup.hide();
            secondWindup.hide();
            beamModel.hide();
            beamEndMdl.hide();

            firstLight.deactivate();
            secondLight.deactivate();
            beamLight.deactivate();
            if (soundEffect) {
                soundEffect->stop();
            }
        };
        auto* target = ship.getTarget();
        if (chargeTimer >= 1.0f) {
            if (!isFiringWeapon) {
                isFiringWeapon = true;
                --numRounds;
            }
            firingTime += fdt;
            firingTimeShieldGraphicPing += fdt;

            auto& body = *beamGraphic->getComponent<ComponentBody>();
            auto& beamModelOne = beamModel.getModel(0);
            beamEndModel.show();
            beamModelOne.show();
            body.setPosition(firstWindupPos);
            glm::quat q;
            const auto targetPos = target->getComponent<ComponentBody>()->position();
            const auto dir = glm::normalize(firstWindupPos - targetPos);
            Math::alignTo(q, -dir);
            beamModelOne.setOrientation(q);
            beamModelOne.forceRender(true);

            auto* shipShields = static_cast<ShipSystemShields*>(ship.getShipSystem(ShipSystemType::Shields));
            auto* shipHull = static_cast<ShipSystemHull*>(ship.getShipSystem(ShipSystemType::Hull));

            vector<Entity> ignored;
            ignored.reserve(3);
            ignored.push_back(shipShields->getEntity()->entity());
            ignored.push_back(shipHull->getEntity()->entity());
            ignored.push_back(ship.entity());


            unsigned short group = CollisionFilter::_Custom_2;
            unsigned short mask = -1;
            //custom 1 is shields
            //do not ray cast against the convex hull (custom 4)
            mask = mask & ~CollisionFilter::_Custom_4;
            //do not ray cast against the shields if they are down
            mask = mask & ~CollisionFilter::_Custom_5;
            //and ignore other weapons!
            mask = mask & ~CollisionFilter::_Custom_2;

            auto rayCastPoints = Physics::rayCast(firstWindupPos, targetPos, ignored, group, mask);
            if (rayCastPoints.size() == 0) {
                cancel(*this, firstWindupModel, secondWindupModel, *firstWindupLight, *secondWindupLight, *beamLight, beamModel, beamEndModel);
                return;
            }
            //get the closest point
            float distNearest = 999999999999.0f;
            glm::vec3 chosenPosition = glm::vec3(99999999999999.0f);
            for (auto& pt : rayCastPoints) {
                auto chkDist = glm::distance2(pt.hitPosition, firstWindupPos);
                if (chkDist < distNearest) {
                    distNearest = chkDist;
                    chosenPosition = pt.hitPosition;
                }
            }
            //we have closest point
            auto len2 = glm::length(chosenPosition - launcherPosition);

            beamLight->setRodLength(len2);

            const glm::vec3 midpt = Math::midpoint(firstWindupPos, chosenPosition);

            beamLightBody.setPosition(midpt);
            beamLightBody.setRotation(q);

            beamEndBody.setPosition(chosenPosition);

            modifyBeamMesh(beamModel, len2);

            if (firingTime >= firingTimeMax) {
                cancel(*this, firstWindupModel, secondWindupModel, *firstWindupLight, *secondWindupLight, *beamLight, beamModel, beamEndModel);
            }
        }
        if (!isInArc(target, arc + 5.0f)) {
            cancel(*this, firstWindupModel, secondWindupModel, *firstWindupLight, *secondWindupLight, *beamLight, beamModel, beamEndModel);
        }
    }
    PrimaryWeaponBeam::update(dt);
}