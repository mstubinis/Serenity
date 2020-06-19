#include <ecs/EntityBody.h>
#include <ecs/ComponentBody.h>

EntityBody::EntityBody(Scene& scene) : Entity(scene) {
}
EntityBody::EntityBody(std::uint32_t entityID, std::uint32_t sceneID, std::uint32_t versionID) : Entity(entityID, sceneID, versionID){
}
EntityBody::EntityBody(const Entity& other) {
    m_Data = other.m_Data;
}

float EntityBody::mass() const {
    return getComponent<ComponentBody>()->mass();
}
glm_quat EntityBody::getRotation() const {
    return getComponent<ComponentBody>()->getRotation();
}
glm_vec3 EntityBody::getScale() const {
    return getComponent<ComponentBody>()->getScale();
}
glm_vec3 EntityBody::getPosition() const {
    return getComponent<ComponentBody>()->getPosition();
}
glm_vec3 EntityBody::getLocalPosition() const {
    return getComponent<ComponentBody>()->getLocalPosition();
}
glm::vec3 EntityBody::getPositionRender() const {
    return getComponent<ComponentBody>()->getPositionRender();
}

const glm_vec3& EntityBody::forward() const {
    return getComponent<ComponentBody>()->forward();
}
const glm_vec3& EntityBody::right() const {
    return getComponent<ComponentBody>()->right();
}
const glm_vec3& EntityBody::up() const {
    return getComponent<ComponentBody>()->up();
}


glm_vec3 EntityBody::getLinearVelocity() const {
    return getComponent<ComponentBody>()->getLinearVelocity();
}
glm_vec3 EntityBody::getAngularVelocity() const {
    return getComponent<ComponentBody>()->getAngularVelocity();
}


void EntityBody::translate(const glm_vec3& translation, const bool local) {
    getComponent<ComponentBody>()->translate(translation, local);
}
void EntityBody::translate(const decimal& x, const decimal& y, const decimal& z, const bool local) {
    getComponent<ComponentBody>()->translate(x, y, z, local);
}
void EntityBody::translate(const decimal& t, const bool local) {
    getComponent<ComponentBody>()->translate(t, local);
}


void EntityBody::rotate(const glm_vec3& rotation, const bool local) {
    getComponent<ComponentBody>()->translate(rotation, local);
}
void EntityBody::rotate(const decimal& pitch_radians, const decimal& yaw_radians, const decimal& roll_radians, const bool local) {
    getComponent<ComponentBody>()->translate(pitch_radians, yaw_radians, roll_radians, local);
}


void EntityBody::scale(const glm_vec3& amount) {
    getComponent<ComponentBody>()->scale(amount);
}
void EntityBody::scale(const decimal& x, const decimal& y, const decimal& z) {
    getComponent<ComponentBody>()->scale(x, y, z);
}
void EntityBody::scale(const decimal& s) {
    getComponent<ComponentBody>()->scale(s);
}


void EntityBody::setPosition(const glm_vec3& newPosition) {
    getComponent<ComponentBody>()->setPosition(newPosition);
}
void EntityBody::setPosition(const decimal& x, const decimal& y, const decimal& z) {
    getComponent<ComponentBody>()->setPosition(x, y, z);
}
void EntityBody::setPosition(const decimal& p) {
    getComponent<ComponentBody>()->setPosition(p);
}




void EntityBody::setRotation(const glm_quat& newRotation) {
    getComponent<ComponentBody>()->setRotation(newRotation);
}
void EntityBody::setRotation(const decimal& quat_x, const decimal& quat_y, const decimal& quat_z, const decimal& quat_w) {
    getComponent<ComponentBody>()->setRotation(quat_x, quat_y, quat_z, quat_w);
}


void EntityBody::setScale(const glm_vec3& newScale) {
    getComponent<ComponentBody>()->setScale(newScale);
}
void EntityBody::setScale(const decimal& x, const decimal& y, const decimal& z) {
    getComponent<ComponentBody>()->setScale(x, y, z);
}
void EntityBody::setScale(const decimal& s) {
    getComponent<ComponentBody>()->setScale(s);
}



void EntityBody::setDamping(const decimal& linear, const decimal& angular) {
    getComponent<ComponentBody>()->setDamping(linear, angular);
}

void EntityBody::setDynamic(const bool dynamic) {
    getComponent<ComponentBody>()->setDynamic(dynamic);
}
void EntityBody::setMass(const float mass) {
    getComponent<ComponentBody>()->setMass(mass);
}
void EntityBody::setGravity(const decimal& x, const decimal& y, const decimal& z) {
    getComponent<ComponentBody>()->setGravity(x, y, z);
}

void EntityBody::clearLinearForces() {
    getComponent<ComponentBody>()->clearLinearForces();
}
void EntityBody::clearAngularForces() {
    getComponent<ComponentBody>()->clearAngularForces();
}
void EntityBody::clearAllForces() {
    getComponent<ComponentBody>()->clearAllForces();
}



void EntityBody::setLinearVelocity(const decimal& x, const decimal& y, const decimal& z, const bool local) {
    getComponent<ComponentBody>()->setLinearVelocity(x, y, z, local);
}
void EntityBody::setLinearVelocity(const glm_vec3& velocity, const bool local) {
    getComponent<ComponentBody>()->setLinearVelocity(velocity, local);
}


void EntityBody::setAngularVelocity(const decimal& x, const decimal& y, const decimal& z, const bool local) {
    getComponent<ComponentBody>()->setAngularVelocity(x, y, z, local);
}
void EntityBody::setAngularVelocity(const glm_vec3& velocity, const bool local) {
    getComponent<ComponentBody>()->setAngularVelocity(velocity, local);
}


void EntityBody::applyForce(const decimal& x, const decimal& y, const decimal& z, const bool local) {
    getComponent<ComponentBody>()->applyForce(x, y, z, local);
}
void EntityBody::applyForce(const glm_vec3& force, const glm_vec3& origin, const bool local) {
    getComponent<ComponentBody>()->applyForce(force, origin, local);
}


void EntityBody::applyImpulse(const decimal& x, const decimal& y, const decimal& z, const bool local) {
    getComponent<ComponentBody>()->applyImpulse(x, y, z, local);
}
void EntityBody::applyImpulse(const glm_vec3& impulse, const glm_vec3& origin, const bool local) {
    getComponent<ComponentBody>()->applyImpulse(impulse, origin, local);
}


void EntityBody::applyTorque(const decimal& x, const decimal& y, const decimal& z, const bool local) {
    getComponent<ComponentBody>()->applyTorque(x, y, z, local);
}
void EntityBody::applyTorque(const glm_vec3& torque, const bool local) {
    getComponent<ComponentBody>()->applyTorque(torque, local);
}


void EntityBody::applyTorqueImpulse(const decimal& x, const decimal& y, const decimal& z, const bool local) {
    getComponent<ComponentBody>()->applyTorqueImpulse(x, y, z, local);
}
void EntityBody::applyTorqueImpulse(const glm_vec3& torqueImpulse, const bool local) {
    getComponent<ComponentBody>()->applyTorqueImpulse(torqueImpulse, local);
}
