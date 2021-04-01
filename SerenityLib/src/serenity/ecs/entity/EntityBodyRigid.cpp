#include <serenity/ecs/entity/EntityBodyRigid.h>
#include <serenity/ecs/components/ComponentBody.h>
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

glm_quat EntityBodyRigid::getRotation() const {
    return getComponent<ComponentBody>()->getRotation();
}
glm_quat EntityBodyRigid::getLocalRotation() const {
    return getRotation();
}
glm_quat EntityBodyRigid::getWorldRotation() const {
    return getComponent<ComponentBody>()->getWorldRotation();
}
glm_vec3 EntityBodyRigid::getScale() const {
    return getComponent<ComponentBody>()->getScale();
}
glm_vec3 EntityBodyRigid::getPosition() const {
    return getComponent<ComponentBody>()->getPosition();
}
glm_vec3 EntityBodyRigid::getLocalPosition() const {
    return getComponent<ComponentBody>()->getLocalPosition();
}
glm_vec3 EntityBodyRigid::getWorldPosition() const {
    return getComponent<ComponentBody>()->getWorldPosition();
}
const glm_vec3& EntityBodyRigid::forward() const {
    return getComponent<ComponentBody>()->forward();
}
const glm_vec3& EntityBodyRigid::right() const {
    return getComponent<ComponentBody>()->right();
}
const glm_vec3& EntityBodyRigid::up() const {
    return getComponent<ComponentBody>()->up();
}


glm_vec3 EntityBodyRigid::getLinearVelocity() const {
    return getComponent<ComponentBodyRigid>()->getLinearVelocity();
}
glm_vec3 EntityBodyRigid::getAngularVelocity() const {
    return getComponent<ComponentBodyRigid>()->getAngularVelocity();
}


void EntityBodyRigid::translate(const glm_vec3& translation, bool local) {
    getComponent<ComponentBody>()->translate(translation, local);
}
void EntityBodyRigid::translate(decimal x, decimal y, decimal z, bool local) {
    getComponent<ComponentBody>()->translate(x, y, z, local);
}
void EntityBodyRigid::translate(decimal t, bool local) {
    getComponent<ComponentBody>()->translate(t, local);
}


void EntityBodyRigid::rotate(const glm_vec3& rotation, bool local) {
    getComponent<ComponentBody>()->rotate(rotation, local);
}
void EntityBodyRigid::rotate(decimal pitch_radians, decimal yaw_radians, decimal roll_radians, bool local) {
    getComponent<ComponentBody>()->rotate(pitch_radians, yaw_radians, roll_radians, local);
}


void EntityBodyRigid::scale(const glm_vec3& amount) {
    getComponent<ComponentBody>()->scale(amount);
}
void EntityBodyRigid::scale(decimal x, decimal y, decimal z) {
    getComponent<ComponentBody>()->scale(x, y, z);
}
void EntityBodyRigid::scale(decimal s) {
    getComponent<ComponentBody>()->scale(s);
}

void EntityBodyRigid::setPosition(const glm_vec3& newPosition) {
    getComponent<ComponentBody>()->setPosition(newPosition);
}
void EntityBodyRigid::setPosition(decimal x, decimal y, decimal z) {
    getComponent<ComponentBody>()->setPosition(x, y, z);
}
void EntityBodyRigid::setPosition(decimal p) {
    getComponent<ComponentBody>()->setPosition(p);
}

void EntityBodyRigid::setRotation(const glm_quat& newRotation) {
    getComponent<ComponentBody>()->setRotation(newRotation);
}
void EntityBodyRigid::setRotation(decimal quat_x, decimal quat_y, decimal quat_z, decimal quat_w) {
    getComponent<ComponentBody>()->setRotation(quat_x, quat_y, quat_z, quat_w);
}

void EntityBodyRigid::setScale(const glm_vec3& newScale) {
    getComponent<ComponentBody>()->setScale(newScale);
}
void EntityBodyRigid::setScale(decimal x, decimal y, decimal z) {
    getComponent<ComponentBody>()->setScale(x, y, z);
}
void EntityBodyRigid::setScale(decimal s) {
    getComponent<ComponentBody>()->setScale(s);
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
