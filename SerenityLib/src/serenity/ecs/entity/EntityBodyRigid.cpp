#include <serenity/ecs/entity/EntityBodyRigid.h>
#include <serenity/ecs/components/ComponentBodyRigid.h>

EntityBodyRigid::EntityBodyRigid(Scene& scene)
    : Entity{ scene }
{}
EntityBodyRigid::EntityBodyRigid(uint32_t entityID, uint32_t sceneID, uint32_t versionID)
    : Entity{ entityID, sceneID, versionID }
{}
EntityBodyRigid::EntityBodyRigid(const Entity& other)
    : Entity{ other }
{}

float EntityBodyRigid::mass() const {
    return getComponent<ComponentBodyRigid>()->mass();
}
glm_quat EntityBodyRigid::getRotation() const {
    return getComponent<ComponentBodyRigid>()->getRotation();
}
glm_vec3 EntityBodyRigid::getScale() const {
    return getComponent<ComponentBodyRigid>()->getScale();
}
glm_vec3 EntityBodyRigid::getPosition() const {
    return getComponent<ComponentBodyRigid>()->getPosition();
}
glm_vec3 EntityBodyRigid::getLocalPosition() const {
    return getComponent<ComponentBodyRigid>()->getLocalPosition();
}

const glm_vec3& EntityBodyRigid::forward() const {
    return getComponent<ComponentBodyRigid>()->forward();
}
const glm_vec3& EntityBodyRigid::right() const {
    return getComponent<ComponentBodyRigid>()->right();
}
const glm_vec3& EntityBodyRigid::up() const {
    return getComponent<ComponentBodyRigid>()->up();
}


glm_vec3 EntityBodyRigid::getLinearVelocity() const {
    return getComponent<ComponentBodyRigid>()->getLinearVelocity();
}
glm_vec3 EntityBodyRigid::getAngularVelocity() const {
    return getComponent<ComponentBodyRigid>()->getAngularVelocity();
}


void EntityBodyRigid::translate(const glm_vec3& translation, bool local) {
    getComponent<ComponentBodyRigid>()->translate(translation, local);
}
void EntityBodyRigid::translate(decimal x, decimal y, decimal z, bool local) {
    getComponent<ComponentBodyRigid>()->translate(x, y, z, local);
}
void EntityBodyRigid::translate(decimal t, bool local) {
    getComponent<ComponentBodyRigid>()->translate(t, local);
}


void EntityBodyRigid::rotate(const glm_vec3& rotation, bool local) {
    getComponent<ComponentBodyRigid>()->rotate(rotation, local);
}
void EntityBodyRigid::rotate(decimal pitch_radians, decimal yaw_radians, decimal roll_radians, bool local) {
    getComponent<ComponentBodyRigid>()->rotate(pitch_radians, yaw_radians, roll_radians, local);
}


void EntityBodyRigid::scale(const glm_vec3& amount) {
    getComponent<ComponentBodyRigid>()->scale(amount);
}
void EntityBodyRigid::scale(decimal x, decimal y, decimal z) {
    getComponent<ComponentBodyRigid>()->scale(x, y, z);
}
void EntityBodyRigid::scale(decimal s) {
    getComponent<ComponentBodyRigid>()->scale(s);
}

void EntityBodyRigid::setPosition(const glm_vec3& newPosition) {
    getComponent<ComponentBodyRigid>()->setPosition(newPosition);
}
void EntityBodyRigid::setPosition(decimal x, decimal y, decimal z) {
    getComponent<ComponentBodyRigid>()->setPosition(x, y, z);
}
void EntityBodyRigid::setPosition(decimal p) {
    getComponent<ComponentBodyRigid>()->setPosition(p);
}

void EntityBodyRigid::setRotation(const glm_quat& newRotation) {
    getComponent<ComponentBodyRigid>()->setRotation(newRotation);
}
void EntityBodyRigid::setRotation(decimal quat_x, decimal quat_y, decimal quat_z, decimal quat_w) {
    getComponent<ComponentBodyRigid>()->setRotation(quat_x, quat_y, quat_z, quat_w);
}

void EntityBodyRigid::setScale(const glm_vec3& newScale) {
    getComponent<ComponentBodyRigid>()->setScale(newScale);
}
void EntityBodyRigid::setScale(decimal x, decimal y, decimal z) {
    getComponent<ComponentBodyRigid>()->setScale(x, y, z);
}
void EntityBodyRigid::setScale(decimal s) {
    getComponent<ComponentBodyRigid>()->setScale(s);
}

void EntityBodyRigid::setDamping(decimal linear, decimal angular) {
    getComponent<ComponentBodyRigid>()->setDamping(linear, angular);
}

void EntityBodyRigid::setDynamic(bool dynamic) {
    getComponent<ComponentBodyRigid>()->setDynamic(dynamic);
}
void EntityBodyRigid::setMass(float mass) {
    getComponent<ComponentBodyRigid>()->setMass(mass);
}
void EntityBodyRigid::setGravity(decimal x, decimal y, decimal z) {
    getComponent<ComponentBodyRigid>()->setGravity(x, y, z);
}

void EntityBodyRigid::clearLinearForces() {
    getComponent<ComponentBodyRigid>()->clearLinearForces();
}
void EntityBodyRigid::clearAngularForces() {
    getComponent<ComponentBodyRigid>()->clearAngularForces();
}
void EntityBodyRigid::clearAllForces() {
    getComponent<ComponentBodyRigid>()->clearAllForces();
}



void EntityBodyRigid::setLinearVelocity(decimal x, decimal y, decimal z, bool local) {
    getComponent<ComponentBodyRigid>()->setLinearVelocity(x, y, z, local);
}
void EntityBodyRigid::setLinearVelocity(const glm_vec3& velocity, bool local) {
    getComponent<ComponentBodyRigid>()->setLinearVelocity(velocity, local);
}


void EntityBodyRigid::setAngularVelocity(decimal x, decimal y, decimal z, bool local) {
    getComponent<ComponentBodyRigid>()->setAngularVelocity(x, y, z, local);
}
void EntityBodyRigid::setAngularVelocity(const glm_vec3& velocity, bool local) {
    getComponent<ComponentBodyRigid>()->setAngularVelocity(velocity, local);
}


void EntityBodyRigid::applyForce(decimal x, decimal y, decimal z, bool local) {
    getComponent<ComponentBodyRigid>()->applyForce(x, y, z, local);
}
void EntityBodyRigid::applyForce(const glm_vec3& force, const glm_vec3& origin, bool local) {
    getComponent<ComponentBodyRigid>()->applyForce(force, origin, local);
}


void EntityBodyRigid::applyImpulse(decimal x, decimal y, decimal z, bool local) {
    getComponent<ComponentBodyRigid>()->applyImpulse(x, y, z, local);
}
void EntityBodyRigid::applyImpulse(const glm_vec3& impulse, const glm_vec3& origin, bool local) {
    getComponent<ComponentBodyRigid>()->applyImpulse(impulse, origin, local);
}


void EntityBodyRigid::applyTorque(decimal x, decimal y, decimal z, bool local) {
    getComponent<ComponentBodyRigid>()->applyTorque(x, y, z, local);
}
void EntityBodyRigid::applyTorque(const glm_vec3& torque, bool local) {
    getComponent<ComponentBodyRigid>()->applyTorque(torque, local);
}


void EntityBodyRigid::applyTorqueImpulse(decimal x, decimal y, decimal z, bool local) {
    getComponent<ComponentBodyRigid>()->applyTorqueImpulse(x, y, z, local);
}
void EntityBodyRigid::applyTorqueImpulse(const glm_vec3& torqueImpulse, bool local) {
    getComponent<ComponentBodyRigid>()->applyTorqueImpulse(torqueImpulse, local);
}
