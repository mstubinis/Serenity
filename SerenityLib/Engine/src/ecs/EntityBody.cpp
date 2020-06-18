#include <ecs/EntityBody.h>
#include <ecs/ComponentBody.h>

EntityBody::EntityBody(Scene& scene) : Entity(scene) {
}
EntityBody::EntityBody(unsigned int entityID, unsigned int sceneID, unsigned int versionID) : Entity(entityID, sceneID, versionID){
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
float EntityBody::mass(const EntityDataRequest& request) const {
    return getComponent<ComponentBody>(request)->mass();
}
glm_quat EntityBody::getRotation(const EntityDataRequest& request) const {
    return getComponent<ComponentBody>(request)->getRotation();
}
glm_vec3 EntityBody::getScale(const EntityDataRequest& request) const {
    return getComponent<ComponentBody>(request)->getScale();
}
glm_vec3 EntityBody::getPosition(const EntityDataRequest& request) const {
    return getComponent<ComponentBody>(request)->getPosition();
}
glm_vec3 EntityBody::getLocalPosition(const EntityDataRequest& request) const {
    return getComponent<ComponentBody>(request)->getLocalPosition();
}
glm::vec3 EntityBody::getPositionRender(const EntityDataRequest& request) const {
    return getComponent<ComponentBody>(request)->getPositionRender();
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
const glm_vec3& EntityBody::forward(const EntityDataRequest& request) const {
    return getComponent<ComponentBody>(request)->forward();
}
const glm_vec3& EntityBody::right(const EntityDataRequest& request) const {
    return getComponent<ComponentBody>(request)->right();
}
const glm_vec3& EntityBody::up(const EntityDataRequest& request) const {
    return getComponent<ComponentBody>(request)->up();
}



glm_vec3 EntityBody::getLinearVelocity() const {
    return getComponent<ComponentBody>()->getLinearVelocity();
}
glm_vec3 EntityBody::getAngularVelocity() const {
    return getComponent<ComponentBody>()->getAngularVelocity();
}
glm_vec3 EntityBody::getLinearVelocity(const EntityDataRequest& request) const {
    return getComponent<ComponentBody>(request)->getLinearVelocity();
}
glm_vec3 EntityBody::getAngularVelocity(const EntityDataRequest& request) const {
    return getComponent<ComponentBody>(request)->getAngularVelocity();
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
void EntityBody::translate(const EntityDataRequest& request, const glm_vec3& translation, const bool local) {
    getComponent<ComponentBody>(request)->translate(translation, local);
}
void EntityBody::translate(const EntityDataRequest& request, const decimal& x, const decimal& y, const decimal& z, const bool local) {
    getComponent<ComponentBody>(request)->translate(x, y, z, local);
}
void EntityBody::translate(const EntityDataRequest& request, const decimal& t, const bool local) {
    getComponent<ComponentBody>(request)->translate(t, local);
}


void EntityBody::rotate(const glm_vec3& rotation, const bool local) {
    getComponent<ComponentBody>()->translate(rotation, local);
}
void EntityBody::rotate(const decimal& pitch_radians, const decimal& yaw_radians, const decimal& roll_radians, const bool local) {
    getComponent<ComponentBody>()->translate(pitch_radians, yaw_radians, roll_radians, local);
}
void EntityBody::rotate(const EntityDataRequest& request, const glm_vec3& rotation, const bool local) {
    getComponent<ComponentBody>(request)->translate(rotation, local);
}
void EntityBody::rotate(const EntityDataRequest& request, const decimal& pitch_radians, const decimal& yaw_radians, const decimal& roll_radians, const bool local) {
    getComponent<ComponentBody>(request)->translate(pitch_radians, yaw_radians, roll_radians, local);
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
void EntityBody::scale(const EntityDataRequest& request, const glm_vec3& amount) {
    getComponent<ComponentBody>(request)->scale(amount);
}
void EntityBody::scale(const EntityDataRequest& request, const decimal& x, const decimal& y, const decimal& z) {
    getComponent<ComponentBody>(request)->scale(x, y, z);
}
void EntityBody::scale(const EntityDataRequest& request, const decimal& s) {
    getComponent<ComponentBody>(request)->scale(s);
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
void EntityBody::setPosition(const EntityDataRequest& request, const glm_vec3& newPosition) {
    getComponent<ComponentBody>(request)->setPosition(newPosition);
}
void EntityBody::setPosition(const EntityDataRequest& request, const decimal& x, const decimal& y, const decimal& z) {
    getComponent<ComponentBody>(request)->setPosition(x, y, z);
}
void EntityBody::setPosition(const EntityDataRequest& request, const decimal& p) {
    getComponent<ComponentBody>(request)->setPosition(p);
}




void EntityBody::setRotation(const glm_quat& newRotation) {
    getComponent<ComponentBody>()->setRotation(newRotation);
}
void EntityBody::setRotation(const decimal& quat_x, const decimal& quat_y, const decimal& quat_z, const decimal& quat_w) {
    getComponent<ComponentBody>()->setRotation(quat_x, quat_y, quat_z, quat_w);
}
void EntityBody::setRotation(const EntityDataRequest& request, const glm_quat& newRotation) {
    getComponent<ComponentBody>(request)->setRotation(newRotation);
}
void EntityBody::setRotation(const EntityDataRequest& request, const decimal& quat_x, const decimal& quat_y, const decimal& quat_z, const decimal& quat_w) {
    getComponent<ComponentBody>(request)->setRotation(quat_x, quat_y, quat_z, quat_w);
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
void EntityBody::setScale(const EntityDataRequest& request, const glm_vec3& newScale) {
    getComponent<ComponentBody>(request)->setScale(newScale);
}
void EntityBody::setScale(const EntityDataRequest& request, const decimal& x, const decimal& y, const decimal& z) {
    getComponent<ComponentBody>(request)->setScale(x, y, z);
}
void EntityBody::setScale(const EntityDataRequest& request, const decimal& s) {
    getComponent<ComponentBody>(request)->setScale(s);
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
void EntityBody::setLinearVelocity(const EntityDataRequest& request, const decimal& x, const decimal& y, const decimal& z, const bool local) {
    getComponent<ComponentBody>(request)->setLinearVelocity(x, y, z, local);
}
void EntityBody::setLinearVelocity(const EntityDataRequest& request, const glm_vec3& velocity, const bool local) {
    getComponent<ComponentBody>(request)->setLinearVelocity(velocity, local);
}



void EntityBody::setAngularVelocity(const decimal& x, const decimal& y, const decimal& z, const bool local) {
    getComponent<ComponentBody>()->setAngularVelocity(x, y, z, local);
}
void EntityBody::setAngularVelocity(const glm_vec3& velocity, const bool local) {
    getComponent<ComponentBody>()->setAngularVelocity(velocity, local);
}
void EntityBody::setAngularVelocity(const EntityDataRequest& request, const decimal& x, const decimal& y, const decimal& z, const bool local) {
    getComponent<ComponentBody>(request)->setAngularVelocity(x, y, z, local);
}
void EntityBody::setAngularVelocity(const EntityDataRequest& request, const glm_vec3& velocity, const bool local) {
    getComponent<ComponentBody>(request)->setAngularVelocity(velocity, local);
}



void EntityBody::applyForce(const decimal& x, const decimal& y, const decimal& z, const bool local) {
    getComponent<ComponentBody>()->applyForce(x, y, z, local);
}
void EntityBody::applyForce(const glm_vec3& force, const glm_vec3& origin, const bool local) {
    getComponent<ComponentBody>()->applyForce(force, origin, local);
}
void EntityBody::applyForce(const EntityDataRequest& request, const decimal& x, const decimal& y, const decimal& z, const bool local) {
    getComponent<ComponentBody>(request)->applyForce(x, y, z, local);
}
void EntityBody::applyForce(const EntityDataRequest& request, const glm_vec3& force, const glm_vec3& origin, const bool local) {
    getComponent<ComponentBody>(request)->applyForce(force, origin, local);
}



void EntityBody::applyImpulse(const decimal& x, const decimal& y, const decimal& z, const bool local) {
    getComponent<ComponentBody>()->applyImpulse(x, y, z, local);
}
void EntityBody::applyImpulse(const glm_vec3& impulse, const glm_vec3& origin, const bool local) {
    getComponent<ComponentBody>()->applyImpulse(impulse, origin, local);
}
void EntityBody::applyImpulse(const EntityDataRequest& request, const decimal& x, const decimal& y, const decimal& z, const bool local) {
    getComponent<ComponentBody>(request)->applyImpulse(x, y, z, local);
}
void EntityBody::applyImpulse(const EntityDataRequest& request, const glm_vec3& impulse, const glm_vec3& origin, const bool local) {
    getComponent<ComponentBody>(request)->applyImpulse(impulse, origin, local);
}



void EntityBody::applyTorque(const decimal& x, const decimal& y, const decimal& z, const bool local) {
    getComponent<ComponentBody>()->applyTorque(x, y, z, local);
}
void EntityBody::applyTorque(const glm_vec3& torque, const bool local) {
    getComponent<ComponentBody>()->applyTorque(torque, local);
}
void EntityBody::applyTorque(const EntityDataRequest& request, const decimal& x, const decimal& y, const decimal& z, const bool local) {
    getComponent<ComponentBody>(request)->applyTorque(x, y, z, local);
}
void EntityBody::applyTorque(const EntityDataRequest& request, const glm_vec3& torque, const bool local) {
    getComponent<ComponentBody>(request)->applyTorque(torque, local);
}



void EntityBody::applyTorqueImpulse(const decimal& x, const decimal& y, const decimal& z, const bool local) {
    getComponent<ComponentBody>()->applyTorqueImpulse(x, y, z, local);
}
void EntityBody::applyTorqueImpulse(const glm_vec3& torqueImpulse, const bool local) {
    getComponent<ComponentBody>()->applyTorqueImpulse(torqueImpulse, local);
}
void EntityBody::applyTorqueImpulse(const EntityDataRequest& request, const decimal& x, const decimal& y, const decimal& z, const bool local) {
    getComponent<ComponentBody>(request)->applyTorqueImpulse(x, y, z, local);
}
void EntityBody::applyTorqueImpulse(const EntityDataRequest& request, const glm_vec3& torqueImpulse, const bool local) {
    getComponent<ComponentBody>(request)->applyTorqueImpulse(torqueImpulse, local);
}