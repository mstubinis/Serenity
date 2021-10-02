#include <serenity/ecs/entity/EntityBody.h>
#include <serenity/ecs/components/ComponentTransform.h>

EntityBody::EntityBody(Scene& scene) 
    : Entity{ scene }
{}
EntityBody::EntityBody(EntityID entityID, EntityID sceneID, EntityID versionID)
    : Entity{ entityID, sceneID, versionID }
{}
EntityBody::EntityBody(const Entity& other) 
    : Entity{ other }
{}

glm::quat EntityBody::getRotation() const {
    return getComponent<ComponentTransform>()->getRotation();
}
glm::quat EntityBody::getLocalRotation() const {
    return getRotation();
}
glm::quat EntityBody::getWorldRotation() const {
    return getComponent<ComponentTransform>()->getWorldRotation();
}
glm::vec3 EntityBody::getScale() const {
    return getComponent<ComponentTransform>()->getScale();
}
glm_vec3 EntityBody::getPosition() const {
    return getComponent<ComponentTransform>()->getPosition();
}
glm_vec3 EntityBody::getWorldPosition() const {
    return getComponent<ComponentTransform>()->getWorldPosition();
}
glm_vec3 EntityBody::getLocalPosition() const {
    return getComponent<ComponentTransform>()->getLocalPosition();
}

const glm::vec3& EntityBody::getForward() const {
    return getComponent<ComponentTransform>()->getForward();
}
const glm::vec3& EntityBody::getRight() const {
    return getComponent<ComponentTransform>()->getRight();
}
const glm::vec3& EntityBody::getUp() const {
    return getComponent<ComponentTransform>()->getUp();
}

/*
glm_vec3 EntityBody::getLinearVelocity() const {
    return getComponent<ComponentTransform>()->getLinearVelocity();
}
*/
void EntityBody::translate(const glm_vec3& translation, bool local) {
    getComponent<ComponentTransform>()->translate(translation, local);
}
void EntityBody::translate(decimal x, decimal y, decimal z, bool local) {
    getComponent<ComponentTransform>()->translate(x, y, z, local);
}
void EntityBody::translate(decimal t, bool local) {
    getComponent<ComponentTransform>()->translate(t, local);
}


void EntityBody::rotate(const glm::vec3& rotation, bool local) {
    getComponent<ComponentTransform>()->rotate(rotation, local);
}
void EntityBody::rotate(float pitch_radians, float yaw_radians, float roll_radians, bool local) {
    getComponent<ComponentTransform>()->rotate(pitch_radians, yaw_radians, roll_radians, local);
}


void EntityBody::scale(const glm::vec3& amount) {
    getComponent<ComponentTransform>()->scale(amount);
}
void EntityBody::scale(float x, float y, float z) {
    getComponent<ComponentTransform>()->scale(x, y, z);
}
void EntityBody::scale(float s) {
    getComponent<ComponentTransform>()->scale(s);
}

void EntityBody::setPosition(const glm_vec3& newPosition) {
    getComponent<ComponentTransform>()->setPosition(newPosition);
}
void EntityBody::setPosition(decimal x, decimal y, decimal z) {
    getComponent<ComponentTransform>()->setPosition(x, y, z);
}
void EntityBody::setPosition(decimal p) {
    getComponent<ComponentTransform>()->setPosition(p);
}

void EntityBody::setRotation(const glm::quat& newRotation) {
    getComponent<ComponentTransform>()->setRotation(newRotation);
}
void EntityBody::setRotation(float quat_x, float quat_y, float quat_z, float quat_w) {
    getComponent<ComponentTransform>()->setRotation(quat_x, quat_y, quat_z, quat_w);
}

void EntityBody::setScale(const glm::vec3& newScale) {
    getComponent<ComponentTransform>()->setScale(newScale);
}
void EntityBody::setScale(float x, float y, float z) {
    getComponent<ComponentTransform>()->setScale(x, y, z);
}
void EntityBody::setScale(float s) {
    getComponent<ComponentTransform>()->setScale(s);
}
/*
void EntityBody::setLinearVelocity(decimal x, decimal y, decimal z, bool local) {
    getComponent<ComponentTransform>()->setLinearVelocity(x, y, z, local);
}
void EntityBody::setLinearVelocity(const glm_vec3& velocity, bool local) {
    getComponent<ComponentTransform>()->setLinearVelocity(velocity, local);
}
*/