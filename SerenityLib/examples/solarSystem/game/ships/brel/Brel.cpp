#include "Brel.h"
#include "../../ResourceManifest.h"
#include "../shipSystems/ShipSystemWeapons.h"
#include "../../weapons/cannons/DisruptorCannon.h"
#include "../../weapons/torpedos/KlingonPhotonTorpedo.h"

#include <core/engine/mesh/Mesh.h>
#include <core/engine/math/Engine_Math.h>
#include <core/engine/Engine.h>
#include <core/engine/physics/Collision.h>
#include <BulletCollision/CollisionShapes/btCompoundShape.h>
#include <BulletDynamics/Dynamics/btRigidBody.h>

#include "../../ships/shipSystems/ShipSystemCloakingDevice.h"
#include "../../ships/shipSystems/ShipSystemMainThrusters.h"
#include "../../ships/shipSystems/ShipSystemPitchThrusters.h"
#include "../../ships/shipSystems/ShipSystemReactor.h"
#include "../../ships/shipSystems/ShipSystemRollThrusters.h"
#include "../../ships/shipSystems/ShipSystemSensors.h"
#include "../../ships/shipSystems/ShipSystemShields.h"
#include "../../ships/shipSystems/ShipSystemWarpDrive.h"
#include "../../ships/shipSystems/ShipSystemYawThrusters.h"
#include "../../ships/shipSystems/ShipSystemWeapons.h"
#include "../../ships/shipSystems/ShipSystemHull.h"
#include "../../ships/Ships.h"
#include "../../ai/AI.h"
#include "../Ships.h"

constexpr auto CLASS = "B'rel";

using namespace std;
using namespace Engine;

Brel::Brel(AIType::Type& ai_type, Team& team, Client& client, Map& map, const string& name, glm::vec3 position, glm::vec3 scale, CollisionType::Type collisionType)
:Ship(team,client, CLASS, map, ai_type, name, position, scale, collisionType, glm::vec3(0.0f,0.7f,0.7f)) {

    m_InitialCamera = glm::vec3(0.0f, 0.7f, 0.7f);
    const auto className = getClass();

    //add wings
    auto& model = *getComponent<ComponentModel>();

    model.getModel(0).setMesh(Ships::Database.at(className).MeshHandles[1], model); //brel head only

    auto& wing1 = model.addModel(ResourceManifest::BrelMeshWing, Ships::Database.at(className).MaterialHandles[0], ResourceManifest::ShipShaderProgramDeferred, RenderStage::GeometryOpaque);
    wing1.setPosition(0.232951f, 0.316462f, 0.08058f);
    auto& wing2 = model.addModel(ResourceManifest::BrelMeshWing2, Ships::Database.at(className).MaterialHandles[0], ResourceManifest::ShipShaderProgramDeferred, RenderStage::GeometryOpaque);
    wing2.setPosition(-0.232951f, 0.316462f, 0.08058f);

    model.setCustomBindFunctor(ShipModelInstanceBindFunctor(),0);
    model.setCustomUnbindFunctor(ShipModelInstanceUnbindFunctor(),0);
    model.setCustomBindFunctor(ShipModelInstanceBindFunctor(),1);
    model.setCustomUnbindFunctor(ShipModelInstanceUnbindFunctor(),1);
    model.setCustomBindFunctor(ShipModelInstanceBindFunctor(),2);
    model.setCustomUnbindFunctor(ShipModelInstanceUnbindFunctor(),2);
    model.setUserPointer(this);

    const auto mass = updateShipDimensions();

    auto& body = *getComponent<ComponentBody>();
    auto* c = new Collision(&body, model, mass, CollisionType::ConvexHull);
    body.setCollision(c);
    //the body is using a convex hull for ship to ship ramming

    body.rebuildRigidBody();

    body.setDamping(static_cast<decimal>(0.01), static_cast<decimal>(0.2));
    auto& btBody = const_cast<btRigidBody&>(body.getBtBody());
    btBody.setActivationState(DISABLE_DEACTIVATION);//this might be dangerous...
    body.setCollisionGroup(CollisionFilter::_Custom_4); //i belong to ramming hull group (group 4)
    body.setCollisionMask(CollisionFilter::_Custom_4); //i should only collide with other ramming hulls only

    //blender 3d to game 3d: switch y and z, then negate z
    auto& _this = *this;
    for (uint i = 0; i < ShipSystemType::_TOTAL; ++i) {
        ShipSystem* system = nullptr;
        if (i == 0)  system = new ShipSystemReactor(_this, 1000);
        else if (i == 1)  system = new ShipSystemPitchThrusters(_this, 1.22f);
        else if (i == 2)  system = new ShipSystemYawThrusters(_this, 1.22f);
        else if (i == 3)  system = new ShipSystemRollThrusters(_this, 1.2f);
        else if (i == 4)  system = new ShipSystemCloakingDevice(_this);
        else if (i == 5)  system = new ShipSystemShields(_this, map, 7700.0f, 7700.0f, 7700.0f, 7700.0f, 10700.0f, 10700.0f);
        else if (i == 6)  system = new ShipSystemMainThrusters(_this);
        else if (i == 7)  system = new ShipSystemWarpDrive(_this);
        else if (i == 8)  system = new ShipSystemSensors(_this, map);
        else if (i == 9)  system = new ShipSystemWeapons(_this);
        else if (i == 10)  system = new ShipSystemHull(_this, map, 15100.0f);
        m_ShipSystems.emplace(i, system);
    }
    internal_finialize_init(ai_type);

    //update shield size
    auto* shields = static_cast<ShipSystemShields*>(getShipSystem(ShipSystemType::Shields));
    auto shieldScale = ((Mesh*)Ships::Database.at(className).MeshHandles[0].get())->getRadiusBox() * SHIELD_SCALE_FACTOR;
    shieldScale *= shields->getAdditionalShieldSizeScale();


    auto& shieldsBody = *shields->getEntity().getComponent<ComponentBody>();
    shieldsBody.setScale(shieldScale);
    m_ShieldScale = shieldScale;

    auto& weapons = *static_cast<ShipSystemWeapons*>(getShipSystem(ShipSystemType::Weapons));

    //blender 3d to game 3d: switch y and z, then negate z
    auto* leftTop = new DisruptorCannon(_this, map, glm::vec3(-0.781865f, -0.638287f, -0.6665f), glm::vec3(-0.01f, 0, -1), 17.0f, 6, 500, 0.7f, 2.5f, 1.8f, 40.5f, 50.0f, 2);
    auto* rightTop = new DisruptorCannon(_this, map, glm::vec3(0.781865f, -0.638287f, -0.6665f), glm::vec3(0.01f, 0, -1), 17.0f, 6, 500, 0.7f, 2.5f, 1.8f, 40.5f, 50.0f, 1);

    weapons.addPrimaryWeaponCannon(*leftTop, true);
    weapons.addPrimaryWeaponCannon(*rightTop, true);

    auto* fwd_torp_1 = new KlingonPhotonTorpedo(_this, map, glm::vec3(0.0f, 0.148089f, -0.854614f), glm::vec3(0.0f, 0.0f, -1.0f), 35.0f, 1);
    auto* fwd_torp_2 = new KlingonPhotonTorpedo(_this, map, glm::vec3(0.0f, 0.148089f, -0.854614f), glm::vec3(0.0f, 0.0f, -1.0f), 35.0f, 1);
    auto* aft_torp = new KlingonPhotonTorpedo(_this, map, glm::vec3(0.0f, 0.115291f, 0.511922f), glm::vec3(0.0f, 0.0f, 1.0f), 35.0f, 1);

    weapons.addSecondaryWeaponTorpedo(*fwd_torp_1, true);
    weapons.addSecondaryWeaponTorpedo(*fwd_torp_2, true);
    weapons.addSecondaryWeaponTorpedo(*aft_torp);

    foldWingsUp();
    updateWingSpan(100000.0f, BrelWingSpanState::State::Up, 1);

    if (Ships::Database[CLASS].HullImpactPoints.size() == 0) {
        Ships::Database[CLASS].HullImpactPoints = {
            glm::vec3(0.0f, 0.311455f, 0.397761f),
            glm::vec3(0, 0.196357f, -0.839703f),
            glm::vec3(0, 0.196357f, -0.652894f),
            glm::vec3(0, 0.18686f, -0.523145f),
            glm::vec3(0, 0.215453f, -0.314285f),
            glm::vec3(0, 0.288782f, -0.096017f),
            glm::vec3(0, 0.288782f, 0.136811f),
            glm::vec3(0, 0.288782f, 0.345834f),
            glm::vec3(0, 0.288782f, 0.475843f),
            glm::vec3(0.207882f, 0.416252f, 0.265136f),
            glm::vec3(-0.207882f, 0.416252f, 0.265136f),
            glm::vec3(0.1024f, 0.331857f, 0.085601f),
            glm::vec3(-0.1024f, 0.331857f, 0.085601f),
            glm::vec3(0.0f, 0.105164f, 0.421337f),
            glm::vec3(0.0f, 0.158835f, 0.154595f),
        };
    }

    m_AI->installFireAtWill(ai_type, _this, map, *static_cast<ShipSystemSensors*>(m_ShipSystems[ShipSystemType::Sensors]), *static_cast<ShipSystemWeapons*>(m_ShipSystems[ShipSystemType::Weapons]));
    m_AI->installThreatTable(map);
}
Brel::~Brel() {

}
void Brel::foldWingsUp() {
    if (m_WingState == BrelWingSpanState::Down) {
        m_WingState = BrelWingSpanState::RotatingUp;
    }
}
void Brel::foldWingsDown() {
    if (m_WingState == BrelWingSpanState::Up) {
        m_WingState = BrelWingSpanState::RotatingDown;
    }
}
void Brel::updateWingSpan(const double& dt, const BrelWingSpanState::State end, const int mod) {
    auto& model = *getComponent<ComponentModel>();
    auto& body = *getComponent<ComponentBody>();

    auto& wing1 = model.getModel(1);
    auto& wing2 = model.getModel(2);
    const auto amount = min(static_cast<float>((0.4f * dt) * mod), BREL_WING_SPAN_MAX);
    m_WingRotation += amount;
    if (m_WingState == BrelWingSpanState::RotatingUp) {
        if (m_WingRotation >= BREL_WING_SPAN_MAX) {
            m_WingRotation = BREL_WING_SPAN_MAX;
            m_WingState = end;
        }
        wing1.rotate(0, 0, amount);
        wing2.rotate(0, 0, -amount);
    }
    if (m_WingState == BrelWingSpanState::RotatingDown) {
        if (m_WingRotation < 0.0f) {
            m_WingRotation = 0.0f;
            m_WingState = end;
        }
        wing1.rotate(0, 0, amount);
        wing2.rotate(0, 0, -amount);
    }

    btCompoundShape* compound = static_cast<btCompoundShape*>(body.getCollision()->getBtShape());
    auto& hullBody = *(static_cast<ShipSystemHull*>(getShipSystem(ShipSystemType::Hull)))->getEntity().getComponent<ComponentBody>();
    btCompoundShape* hullcompound = static_cast<btCompoundShape*>(hullBody.getCollision()->getBtShape());
    auto wing1Transform = btTransform(Math::glmToBTQuat(wing1.orientation()), Math::btVectorFromGLM(wing1.position()));
    auto wing2Transform = btTransform(Math::glmToBTQuat(wing2.orientation()), Math::btVectorFromGLM(wing2.position()));
    compound->updateChildTransform(1, wing1Transform);
    compound->updateChildTransform(2, wing2Transform);
    hullcompound->updateChildTransform(1, wing1Transform);
    hullcompound->updateChildTransform(2, wing2Transform);

    hullBody.setMass(hullBody.mass());

    auto* shields = static_cast<ShipSystemShields*>(getShipSystem(ShipSystemType::Shields));
    auto& shieldsModel = *shields->getEntity().getComponent<ComponentModel>();
    auto& shieldsBody  = *shields->getEntity().getComponent<ComponentBody>();

    glm_vec3 newLocalPos = glm_vec3(0, m_WingRotation * 0.5f, 0);
    m_CameraOffsetDefault = m_InitialCamera - (newLocalPos * static_cast<decimal>(0.5));

    auto& model1 = shieldsModel.getModel(0);
    model1.setPosition(newLocalPos);

    auto shieldScale = m_ShieldScale + glm_vec3(m_WingRotation * 0.3f, 0.0f, 0.0f);
    shieldsBody.setScale(shieldScale);
}
void Brel::update(const double& dt) {
    if (!isDestroyed()) {
        auto& sensors = *static_cast<ShipSystemSensors*>(getShipSystem(ShipSystemType::Sensors));
        const auto& closestEnemy = sensors.getClosestEnemyShip();
        if (closestEnemy.ship) {
            if (closestEnemy.distanceAway2 < static_cast<decimal>(150.0 * 150.0)) { //15km away
                foldWingsDown();
            }else{
                foldWingsUp();
            }
        }else{
            foldWingsUp();
        }
        //for testing only
        /*
        if (IsPlayer() && Engine::isKeyDownOnce(KeyboardKey::Space)) {
            foldWingsUp();
            foldWingsDown();
        }
        */
        if (m_WingState == BrelWingSpanState::RotatingUp) {
            updateWingSpan(dt, BrelWingSpanState::Up, 1);
        }
        if (m_WingState == BrelWingSpanState::RotatingDown) {
            updateWingSpan(dt, BrelWingSpanState::Down, -1);
        }
    }
    Ship::update(dt);
}
const Brel::BrelWingSpanState::State Brel::getWingState() const {
    return m_WingState;
}