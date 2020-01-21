#include <core/engine/scene/Camera.h>
#include <core/engine/scene/Scene.h>
#include <core/engine/math/Engine_Math.h>
#include <core/engine/resources/Engine_Resources.h>

#include <glm/gtx/norm.hpp>

using namespace Engine;
using namespace boost;
using namespace std;


Camera::Camera(const float angle, const float aspectRatio, const float _near, const float _far,Scene* scene) : EntityWrapper(*scene){//create a perspective camera
    if (!scene)
        scene = Resources::getCurrentScene();
    auto& cam   = *m_Entity.addComponent<ComponentCamera>(angle, aspectRatio, _near, _far);
    auto& logic = *m_Entity.addComponent<ComponentLogic2>();
    auto& body  = *m_Entity.addComponent<ComponentBody>();

    cam.lookAt(body.position(), body.position() + body.forward(), body.up());
    logic.setUserPointer(this);

    priv::InternalScenePublicInterface::GetCameras(*scene).push_back(this);
}
Camera::Camera(const float left, const float right, const float bottom, const float top, const float _near, const float _far,Scene* scene) : EntityWrapper(*scene){//create an orthographic camera
    if (!scene)
        scene = Resources::getCurrentScene();
    auto& cam   = *m_Entity.addComponent<ComponentCamera>(left, right, bottom, top, _near, _far);
    auto& logic = *m_Entity.addComponent<ComponentLogic2>();
    auto& body  = *m_Entity.addComponent<ComponentBody>();

    cam.lookAt(body.position(), body.position() + body.forward(), body.up());
    logic.setUserPointer(this);

    priv::InternalScenePublicInterface::GetCameras(*scene).push_back(this);
}
Camera::~Camera(){ 
}
void Camera::lookAt(const glm_vec3& eye, const glm_vec3& center, const glm_vec3& up) {
    m_Entity.getComponent<ComponentCamera>()->lookAt(eye, center, up);
}
const glm_vec3 Camera::getPosition(){ 
    return m_Entity.getComponent<ComponentBody>()->position(); 
}
const glm::quat Camera::getOrientation(){
    return glm::conjugate(glm::quat_cast(m_Entity.getComponent<ComponentCamera>()->getView())); 
}
const float& Camera::getAngle(){ 
    return m_Entity.getComponent<ComponentCamera>()->m_Angle; 
}
const float& Camera::getAspect(){ 
    return m_Entity.getComponent<ComponentCamera>()->m_AspectRatio; 
}
const float& Camera::getNear(){ 
    return m_Entity.getComponent<ComponentCamera>()->m_NearPlane; 
}
const float& Camera::getFar(){ 
    return m_Entity.getComponent<ComponentCamera>()->m_FarPlane; 
}
void Camera::setAngle(const float _Angle){
    m_Entity.getComponent<ComponentCamera>()->setAngle(_Angle); 
}
void Camera::setAspect(const float _Aspect){
    m_Entity.getComponent<ComponentCamera>()->setAspect(_Aspect); 
}
void Camera::setNear(const float _near){
    m_Entity.getComponent<ComponentCamera>()->setNear(_near); 
}
void Camera::setFar(const float _far){
    m_Entity.getComponent<ComponentCamera>()->setFar(_far); 
}
const glm::mat4 Camera::getViewProjectionInverse(){ 
    return m_Entity.getComponent<ComponentCamera>()->getViewProjectionInverse(); 
}
const glm::mat4 Camera::getProjection(){ 
    return m_Entity.getComponent<ComponentCamera>()->getProjection(); 
}
const glm::mat4 Camera::getView(){ 
    return m_Entity.getComponent<ComponentCamera>()->getView(); 
}
const glm::mat4 Camera::getViewInverse(){ 
    return m_Entity.getComponent<ComponentCamera>()->getViewInverse(); 
}
const glm::mat4 Camera::getProjectionInverse(){ 
    return m_Entity.getComponent<ComponentCamera>()->getProjectionInverse(); 
}
const glm::mat4 Camera::getViewProjection(){ 
    return m_Entity.getComponent<ComponentCamera>()->getViewProjection();
}
const glm::vec3 Camera::getViewVector(){ 
    return m_Entity.getComponent<ComponentCamera>()->getViewVector(); 
}
const glm_vec3 Camera::forward(){ 
    return m_Entity.getComponent<ComponentCamera>()->forward(); 
}
const glm_vec3 Camera::right(){ 
    return m_Entity.getComponent<ComponentCamera>()->right(); 
}
const glm_vec3 Camera::up(){ 
    return m_Entity.getComponent<ComponentCamera>()->up(); 
}
const decimal Camera::getDistance(Entity& e){
    auto& b = *e.getComponent<ComponentBody>();
    return glm::distance(b.position(),getPosition());
}
const decimal Camera::getDistance(const glm_vec3& objPos){
    return glm::distance(objPos,getPosition()); 
}
const decimal Camera::getDistanceSquared(Entity& e) {
    auto& b = *e.getComponent<ComponentBody>();
    return glm::distance2(b.position(), getPosition());
}
const decimal Camera::getDistanceSquared(const glm_vec3& objPos) {
    return glm::distance2(objPos, getPosition()); 
}
const decimal Camera::getDistanceSquared(Entity& e, const glm_vec3& this_pos) {
    auto& b = *e.getComponent<ComponentBody>();
    return glm::distance2(b.position(), this_pos);
}
const decimal Camera::getDistanceSquared(const glm_vec3& objPos, const glm_vec3& this_pos) {
    return glm::distance2(objPos, this_pos);
}


const unsigned int Camera::sphereIntersectTest(const glm_vec3& pos, const float radius) {
    return m_Entity.getComponent<ComponentCamera>()->sphereIntersectTest(pos,radius); 
}
const unsigned int Camera::pointIntersectTest(const glm_vec3& pos) {
    return m_Entity.getComponent<ComponentCamera>()->pointIntersectTest(pos); 
}
const bool Camera::rayIntersectSphere(Entity& entity){
    auto* body        = entity.getComponent<ComponentBody>();
    auto* model       = entity.getComponent<ComponentModel>();
    auto& cameraBody  = *m_Entity.getComponent<ComponentBody>();
    auto& thisCamera  = *m_Entity.getComponent<ComponentCamera>();
    auto radius       = 0.0f;
    if(model) 
        radius = model->radius();
    if (!body)
        return false;
    return Math::rayIntersectSphere(body->position(), radius, cameraBody.position(), thisCamera.getViewVector());
}

