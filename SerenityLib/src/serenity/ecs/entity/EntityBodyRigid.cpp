#include <serenity/ecs/entity/EntityBodyRigid.h>
#include <serenity/ecs/components/ComponentTransform.h>
#include <serenity/ecs/components/ComponentRigidBody.h>

EntityBodyRigid::EntityBodyRigid(Scene& scene)
    : Entity{ scene }
{}
EntityBodyRigid::EntityBodyRigid(uint32_t entityID, uint32_t sceneID, uint32_t versionID)
    : Entity{ entityID, sceneID, versionID }
{}
EntityBodyRigid::EntityBodyRigid(const Entity& other)
    : Entity{ other }
{}

glm::quat EntityBodyRigid::getRotation() const {
    return getComponent<ComponentTransform>()->getRotation();
}
glm::quat EntityBodyRigid::getLocalRotation() const {
    return getRotation();
}
glm::quat EntityBodyRigid::getWorldRotation() const {
    return getComponent<ComponentTransform>()->getWorldRotation();
}
glm::vec3 EntityBodyRigid::getScale() const {
    return getComponent<ComponentTransform>()->getScale();
}
glm_vec3 EntityBodyRigid::getPosition() const {
    return getComponent<ComponentTransform>()->getPosition();
}
glm_vec3 EntityBodyRigid::getLocalPosition() const {
    return getComponent<ComponentTransform>()->getLocalPosition();
}
glm_vec3 EntityBodyRigid::getWorldPosition() const {
    return getComponent<ComponentTransform>()->getWorldPosition();
}
const glm::vec3& EntityBodyRigid::getForward() const {
    return getComponent<ComponentTransform>()->getForward();
}
const glm::vec3& EntityBodyRigid::getRight() const {
    return getComponent<ComponentTransform>()->getRight();
}
const glm::vec3& EntityBodyRigid::getUp() const {
    return getComponent<ComponentTransform>()->getUp();
}


glm_vec3 EntityBodyRigid::getLinearVelocity() const {
    return getComponent<ComponentRigidBody>()->getLinearVelocity();
}
glm_vec3 EntityBodyRigid::getAngularVelocity() const {
    return getComponent<ComponentRigidBody>()->getAngularVelocity();
}


void EntityBodyRigid::translate(const glm_vec3& translation, bool local) {
    getComponent<ComponentTransform>()->translate(translation, local);
}
void EntityBodyRigid::translate(decimal x, decimal y, decimal z, bool local) {
    getComponent<ComponentTransform>()->translate(x, y, z, local);
}
void EntityBodyRigid::translate(decimal t, bool local) {
    getComponent<ComponentTransform>()->translate(t, local);
}


void EntityBodyRigid::rotate(const glm::vec3& rotation, bool local) {
    getComponent<ComponentTransform>()->rotate(rotation, local);
}
void EntityBodyRigid::rotate(float pitch_radians, float yaw_radians, float roll_radians, bool local) {
    getComponent<ComponentTransform>()->rotate(pitch_radians, yaw_radians, roll_radians, local);
}


void EntityBodyRigid::scale(const glm::vec3& amount) {
    getComponent<ComponentTransform>()->scale(amount);
}
void EntityBodyRigid::scale(float x, float y, float z) {
    getComponent<ComponentTransform>()->scale(x, y, z);
}
void EntityBodyRigid::scale(float s) {
    getComponent<ComponentTransform>()->scale(s);
}

void EntityBodyRigid::setPosition(const glm_vec3& newPosition) {
    getComponent<ComponentTransform>()->setPosition(newPosition);
}
void EntityBodyRigid::setPosition(decimal x, decimal y, decimal z) {
    getComponent<ComponentTransform>()->setPosition(x, y, z);
}
void EntityBodyRigid::setPosition(decimal p) {
    getComponent<ComponentTransform>()->setPosition(p);
}

void EntityBodyRigid::setRotation(const glm::quat& newRotation) {
    getComponent<ComponentTransform>()->setRotation(newRotation);
}
void EntityBodyRigid::setRotation(float quat_x, float quat_y, float quat_z, float quat_w) {
    getComponent<ComponentTransform>()->setRotation(quat_x, quat_y, quat_z, quat_w);
}

void EntityBodyRigid::setScale(const glm::vec3& newScale) {
    getComponent<ComponentTransform>()->setScale(newScale);
}
void EntityBodyRigid::setScale(float x, float y, float z) {
    getComponent<ComponentTransform>()->setScale(x, y, z);
}
void EntityBodyRigid::setScale(float s) {
    getComponent<ComponentTransform>()->setScale(s);
}

void EntityBodyRigid::setDamping(decimal linear, decimal angular) {
    getComponent<ComponentRigidBody>()->setDamping(linear, angular);
}

void EntityBodyRigid::setDynamic(bool dynamic) {
    getComponent<ComponentRigidBody>()->setDynamic(dynamic);
}
void EntityBodyRigid::setMass(float mass) {
    getComponent<ComponentRigidBody>()->setMass(mass);
}
void EntityBodyRigid::setGravity(decimal x, decimal y, decimal z) {
    getComponent<ComponentRigidBody>()->setGravity(x, y, z);
}

void EntityBodyRigid::clearLinearForces() {
    getComponent<ComponentRigidBody>()->clearLinearForces();
}
void EntityBodyRigid::clearAngularForces() {
    getComponent<ComponentRigidBody>()->clearAngularForces();
}
void EntityBodyRigid::clearAllForces() {
    getComponent<ComponentRigidBody>()->clearAllForces();
}



void EntityBodyRigid::setLinearVelocity(decimal x, decimal y, decimal z, bool local) {
    getComponent<ComponentRigidBody>()->setLinearVelocity(x, y, z, local);
}
void EntityBodyRigid::setLinearVelocity(const glm_vec3& velocity, bool local) {
    getComponent<ComponentRigidBody>()->setLinearVelocity(velocity, local);
}


void EntityBodyRigid::setAngularVelocity(decimal x, decimal y, decimal z, bool local) {
    getComponent<ComponentRigidBody>()->setAngularVelocity(x, y, z, local);
}
void EntityBodyRigid::setAngularVelocity(const glm_vec3& velocity, bool local) {
    getComponent<ComponentRigidBody>()->setAngularVelocity(velocity, local);
}


void EntityBodyRigid::applyForce(decimal x, decimal y, decimal z, bool local) {
    getComponent<ComponentRigidBody>()->applyForce(x, y, z, local);
}
void EntityBodyRigid::applyForce(const glm_vec3& force, const glm_vec3& origin, bool local) {
    getComponent<ComponentRigidBody>()->applyForce(force, origin, local);
}


void EntityBodyRigid::applyImpulse(decimal x, decimal y, decimal z, bool local) {
    getComponent<ComponentRigidBody>()->applyImpulse(x, y, z, local);
}
void EntityBodyRigid::applyImpulse(const glm_vec3& impulse, const glm_vec3& origin, bool local) {
    getComponent<ComponentRigidBody>()->applyImpulse(impulse, origin, local);
}


void EntityBodyRigid::applyTorque(decimal x, decimal y, decimal z, bool local) {
    getComponent<ComponentRigidBody>()->applyTorque(x, y, z, local);
}
void EntityBodyRigid::applyTorque(const glm_vec3& torque, bool local) {
    getComponent<ComponentRigidBody>()->applyTorque(torque, local);
}


void EntityBodyRigid::applyTorqueImpulse(decimal x, decimal y, decimal z, bool local) {
    getComponent<ComponentRigidBody>()->applyTorqueImpulse(x, y, z, local);
}
void EntityBodyRigid::applyTorqueImpulse(const glm_vec3& torqueImpulse, bool local) {
    getComponent<ComponentRigidBody>()->applyTorqueImpulse(torqueImpulse, local);
}
