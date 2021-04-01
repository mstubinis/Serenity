#include <serenity/ecs/entity/EntityBody.h>
#include <serenity/ecs/components/ComponentBody.h>

EntityBody::EntityBody(Scene& scene) 
    : Entity{ scene }
{}
EntityBody::EntityBody(uint32_t entityID, uint32_t sceneID, uint32_t versionID) 
    : Entity{ entityID, sceneID, versionID }
{}
EntityBody::EntityBody(const Entity& other) 
    : Entity{ other }
{}

glm_quat EntityBody::getRotation() const {
    return getComponent<ComponentBody>()->getRotation();
}
glm_quat EntityBody::getLocalRotation() const {
    return getRotation();
}
glm_quat EntityBody::getWorldRotation() const {
    return getComponent<ComponentBody>()->getWorldRotation();
}
glm_vec3 EntityBody::getScale() const {
    return getComponent<ComponentBody>()->getScale();
}
glm_vec3 EntityBody::getPosition() const {
    return getComponent<ComponentBody>()->getPosition();
}
glm_vec3 EntityBody::getWorldPosition() const {
    return getComponent<ComponentBody>()->getWorldPosition();
}
glm_vec3 EntityBody::getLocalPosition() const {
    return getComponent<ComponentBody>()->getLocalPosition();
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

/*
glm_vec3 EntityBody::getLinearVelocity() const {
    return getComponent<ComponentBody>()->getLinearVelocity();
}
*/
void EntityBody::translate(const glm_vec3& translation, bool local) {
    getComponent<ComponentBody>()->translate(translation, local);
}
void EntityBody::translate(decimal x, decimal y, decimal z, bool local) {
    getComponent<ComponentBody>()->translate(x, y, z, local);
}
void EntityBody::translate(decimal t, bool local) {
    getComponent<ComponentBody>()->translate(t, local);
}


void EntityBody::rotate(const glm_vec3& rotation, bool local) {
    getComponent<ComponentBody>()->rotate(rotation, local);
}
void EntityBody::rotate(decimal pitch_radians, decimal yaw_radians, decimal roll_radians, bool local) {
    getComponent<ComponentBody>()->rotate(pitch_radians, yaw_radians, roll_radians, local);
}


void EntityBody::scale(const glm_vec3& amount) {
    getComponent<ComponentBody>()->scale(amount);
}
void EntityBody::scale(decimal x, decimal y, decimal z) {
    getComponent<ComponentBody>()->scale(x, y, z);
}
void EntityBody::scale(decimal s) {
    getComponent<ComponentBody>()->scale(s);
}

void EntityBody::setPosition(const glm_vec3& newPosition) {
    getComponent<ComponentBody>()->setPosition(newPosition);
}
void EntityBody::setPosition(decimal x, decimal y, decimal z) {
    getComponent<ComponentBody>()->setPosition(x, y, z);
}
void EntityBody::setPosition(decimal p) {
    getComponent<ComponentBody>()->setPosition(p);
}

void EntityBody::setRotation(const glm_quat& newRotation) {
    getComponent<ComponentBody>()->setRotation(newRotation);
}
void EntityBody::setRotation(decimal quat_x, decimal quat_y, decimal quat_z, decimal quat_w) {
    getComponent<ComponentBody>()->setRotation(quat_x, quat_y, quat_z, quat_w);
}

void EntityBody::setScale(const glm_vec3& newScale) {
    getComponent<ComponentBody>()->setScale(newScale);
}
void EntityBody::setScale(decimal x, decimal y, decimal z) {
    getComponent<ComponentBody>()->setScale(x, y, z);
}
void EntityBody::setScale(decimal s) {
    getComponent<ComponentBody>()->setScale(s);
}
/*
void EntityBody::setLinearVelocity(decimal x, decimal y, decimal z, bool local) {
    getComponent<ComponentBody>()->setLinearVelocity(x, y, z, local);
}
void EntityBody::setLinearVelocity(const glm_vec3& velocity, bool local) {
    getComponent<ComponentBody>()->setLinearVelocity(velocity, local);
}
*/