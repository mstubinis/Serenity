#include "Brel.h"
#include "../../ResourceManifest.h"
#include "../shipSystems/ShipSystemWeapons.h"
#include "../../weapons/DisruptorCannon.h"
#include "../../weapons/KlingonPhotonTorpedo.h"

#include <core/engine/mesh/Mesh.h>
#include <core/engine/math/Engine_Math.h>
#include <BulletCollision/CollisionShapes/btCompoundShape.h>

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

using namespace std;
using namespace Engine;

Brel::Brel(Client& client, Map& map, bool player, const string& name, glm::vec3 position, glm::vec3 scale, CollisionType::Type collisionType)
:Ship(client, ResourceManifest::BrelMeshHead, ResourceManifest::BrelMaterial, "Brel", map,player, name, position, scale, collisionType, glm::vec3(0.0f, 0.311455f, 0.397761f), glm::vec3(0.0f,0.7f,0.7f)) {

    m_InitialCamera = glm::vec3(0.0f, 0.7f, 0.7f);

    //add wings
    auto& model = *getComponent<ComponentModel>();
    auto& wing1 = model.addModel(ResourceManifest::BrelMeshWing, ResourceManifest::BrelMaterial);
    wing1.setPosition(0.232951f, 0.316462f, 0.08058f);
    auto& wing2 = model.addModel(ResourceManifest::BrelMeshWing2, ResourceManifest::BrelMaterial);
    wing2.setPosition(-0.232951f, 0.316462f, 0.08058f);

    const auto mass = updateShipDimensions();

    auto& body = *getComponent<ComponentBody>();
    auto* c = new Collision(&body, model, mass, CollisionType::ConvexHull);
    body.setCollision(c);
    //the body is using a convex hull for ship to ship ramming

    body.rebuildRigidBody();

    const_cast<btRigidBody&>(body.getBtBody()).setDamping(static_cast<btScalar>(0.01), static_cast<btScalar>(0.2));
    body.getBtBody().setActivationState(DISABLE_DEACTIVATION);//this might be dangerous...

    //blender 3d to game 3d: switch y and z, then negate z
    auto& _this = *this;
    for (uint i = 0; i < ShipSystemType::_TOTAL; ++i) {
        ShipSystem* system = nullptr;
        if (i == 0)  system = new ShipSystemReactor(_this, 1000);
        else if (i == 1)  system = new ShipSystemPitchThrusters(_this);
        else if (i == 2)  system = new ShipSystemYawThrusters(_this);
        else if (i == 3)  system = new ShipSystemRollThrusters(_this);
        else if (i == 4)  system = new ShipSystemCloakingDevice(_this);
        else if (i == 5)  system = new ShipSystemShields(_this, map, 7700.0f, 7700.0f, 7700.0f, 7700.0f, 10700.0f, 10700.0f);
        else if (i == 6)  system = new ShipSystemMainThrusters(_this);
        else if (i == 7)  system = new ShipSystemWarpDrive(_this);
        else if (i == 8)  system = new ShipSystemSensors(_this, map);
        else if (i == 9)  system = new ShipSystemWeapons(_this);
        else if (i == 10)  system = new ShipSystemHull(_this, map, 7100.0f);
        m_ShipSystems.emplace(i, system);
    }
    //update shield size
    const auto shieldScale = ((Mesh*)ResourceManifest::BrelMesh.get())->getRadiusBox() * SHIELD_SCALE_FACTOR;
    auto* shields = static_cast<ShipSystemShields*>(getShipSystem(ShipSystemType::Shields));
    auto& shieldsBody = *shields->getEntity().getComponent<ComponentBody>();
    shieldsBody.setScale(shieldScale);
    m_ShieldScale = shieldScale;

    auto& weapons = *static_cast<ShipSystemWeapons*>(getShipSystem(ShipSystemType::Weapons));

    //blender 3d to game 3d: switch y and z, then negate z
    auto* leftTop = new DisruptorCannon(_this, map, glm::vec3(-0.781865f, -0.638287f, -0.6665f), glm::vec3(0.01f, 0, -1), 10.0f, 6, 250, 0.7f, 2.5f, 1.8f, 50.5f, 25.0f, 1);
    auto* leftBottom = new DisruptorCannon(_this, map, glm::vec3(-0.78429f, -0.65897f, -0.662792f), glm::vec3(0.01f, 0, -1), 10.0f, 6, 250, 0.7f, 2.5f, 1.8f, 50.5f, 25.0f, 1);
    auto* rightBottom = new DisruptorCannon(_this, map, glm::vec3(0.78429f, -0.65897f, -0.662792f), glm::vec3(-0.01f, 0, -1), 10.0f, 6, 250, 0.7f, 2.5f, 1.8f, 50.5f, 25.0f, 2);
    auto* rightTop = new DisruptorCannon(_this, map, glm::vec3(0.781865f, -0.638287f, -0.6665f), glm::vec3(-0.01f, 0, -1), 10.0f, 6, 250, 0.7f, 2.5f, 1.8f, 50.5f, 25.0f, 2);

    weapons.addPrimaryWeaponCannon(*leftTop);
    weapons.addPrimaryWeaponCannon(*leftBottom);
    weapons.addPrimaryWeaponCannon(*rightBottom);
    weapons.addPrimaryWeaponCannon(*rightTop);

    auto* fwd_torp = new KlingonPhotonTorpedo(_this, map, glm::vec3(0.0f, 0.148089f, -0.854614f), glm::vec3(0.0f, 0.0f, -1.0f), 35.0f, 2);
    auto* aft_torp = new KlingonPhotonTorpedo(_this, map, glm::vec3(0.0f, 0.115291f, 0.511922f), glm::vec3(0.0f, 0.0f, 1.0f), 35.0f, 1);

    weapons.addSecondaryWeaponTorpedo(*fwd_torp);
    weapons.addSecondaryWeaponTorpedo(*aft_torp);

    foldWingsUp();
    updateWingSpan(100000.0f, BrelWingSpanState::State::Up, 1);
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

    auto scl = m_ShieldScale + glm_vec3(m_WingRotation * 0.3f, 0.0f, 0.0f);
    shieldsBody.setScale(scl);
    model1.setScale(scl);
}
void Brel::update(const double& dt) {
    if (m_IsPlayer && Engine::isKeyDownOnce(KeyboardKey::Space)) {
        foldWingsUp();
        foldWingsDown();
    }
    if (m_WingState == BrelWingSpanState::RotatingUp) {
        updateWingSpan(dt, BrelWingSpanState::Up, 1);
    }
    if (m_WingState == BrelWingSpanState::RotatingDown) {
        updateWingSpan(dt, BrelWingSpanState::Down, -1);
    }
    Ship::update(dt);
}
const Brel::BrelWingSpanState::State Brel::getWingState() const {
    return m_WingState;
}