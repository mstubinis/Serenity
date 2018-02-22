#include "Engine.h"
#include "Scene.h"
#include "Camera.h"
#include "ObjectDisplay.h"
#include "ObjectDynamic.h"
#include "Engine_Resources.h"

#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_access.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <boost/make_shared.hpp>

using namespace Engine;
using namespace boost;
using namespace std;

Camera::Camera(string n, float angle, float aspectRatio, float _near, float _far,Scene* scene):ObjectBasic(glm::vec3(0),glm::vec3(1),n,scene,false){//create a perspective camera
    m_Angle = angle;
    m_AspectRatio = aspectRatio;
    m_Near = _near;
    m_Far = _far;
    m_Scene = scene;
    m_Type = CameraType::Perspective;
    setPerspectiveProjection();
    lookAt(getPosition(),getPosition() + getForward(), getUp());

	Engine::impl::Core::m_Engine->m_ResourceManager->_addCamera(this);
	scene->cameras().emplace(name(),this);
}
Camera::Camera(string n, float left, float right, float bottom, float top, float _near, float _far,Scene* scene):ObjectBasic(glm::vec3(0),glm::vec3(1),n,scene,false){//create an orthographic camera
    m_Angle = 45.0f;
    m_AspectRatio = 1.0f;
    m_Near = _near;
    m_Far = _far;
    m_Scene = scene;
    m_Type = CameraType::Orthographic;
    setOrthoProjection(left,right,bottom,top);
    lookAt(getPosition(),getPosition() + getForward(), getUp());

    Engine::impl::Core::m_Engine->m_ResourceManager->_addCamera(this);
	scene->cameras().emplace(name(),this);
}
void Camera::_constructFrustrum(){
    glm::mat4 vp = m_Projection * m_View;
    glm::vec4 rowX = glm::row(vp, 0);
    glm::vec4 rowY = glm::row(vp, 1);
    glm::vec4 rowZ = glm::row(vp, 2);
    glm::vec4 rowW = glm::row(vp, 3);

    m_Planes[0] = glm::normalize(rowW + rowX);
    m_Planes[1] = glm::normalize(rowW - rowX);
    m_Planes[2] = glm::normalize(rowW + rowY);
    m_Planes[3] = glm::normalize(rowW - rowY);
    m_Planes[4] = glm::normalize(rowW + rowZ);
    m_Planes[5] = glm::normalize(rowW - rowZ);

    for(uint i = 0; i < 6; i++){
        glm::vec3 normal(m_Planes[i].x, m_Planes[i].y, m_Planes[i].z);
        m_Planes[i] = -m_Planes[i] / glm::length(normal);
    }
}
void Camera::resize(uint width,uint height){
    if(m_Type == CameraType::Perspective){setAspectRatio(float(width)/float(height));}
    else{setOrthoProjection(0,float(width),0,float(height));}
}
Camera::~Camera()
{ 
}
void Camera::setPerspectiveProjection(float angle,float aspectRatio,float _near,float _far){
	m_Angle = angle;
	m_AspectRatio = aspectRatio;
	m_Near = _near;
	m_Far = _far;
	m_Projection = glm::perspective(angle,aspectRatio,_near,_far);
}
void Camera::setPerspectiveProjection(){m_Projection = glm::perspective(m_Angle,m_AspectRatio,m_Near,m_Far);}
void Camera::setOrthoProjection(float l, float r, float b, float t){m_Projection = glm::ortho(l,r,b,t,m_Near,m_Far);}
void Camera::setAspectRatio(float ratio){ 
    m_AspectRatio = ratio;
    setPerspectiveProjection();
}
void Camera::lookAt(glm::vec3 target){ Camera::lookAt(getPosition(),target,getUp()); }
void Camera::lookAt(glm::vec3 target,glm::vec3 up){ Camera::lookAt(getPosition(),target,up); }
void Camera::lookAt(glm::vec3 eye,glm::vec3 target,glm::vec3 up){
    ObjectBasic::update(Resources::dt());
    setPosition(eye); 
    m_View = glm::lookAt(eye,target,up);
    m_Orientation = glm::conjugate(glm::quat_cast(m_View));
    m_Forward = -glm::normalize(eye-target);
    m_Up = glm::normalize(up);
    m_Right = glm::normalize(glm::cross(m_Forward,m_Up));
    _constructFrustrum();
}
void Camera::lookAt(Object* target, bool targetUp){
    glm::vec3 u; if(!targetUp) u = getUp(); else u = target->getUp();
    Camera::lookAt((getPosition(),target->getPosition(),u));
}
void Camera::update(float dt){
    lookAt(getPosition(),getPosition() + getForward(), getUp());
}
bool Camera::sphereIntersectTest(Object* obj){return sphereIntersectTest(obj->getPosition(),obj->getRadius());}
bool Camera::sphereIntersectTest(glm::vec3 pos, float radius){
    if(radius <= 0) return false;
    for (unsigned int i = 0; i < 6; i++){
        float dist = m_Planes[i].x * pos.x + m_Planes[i].y * pos.y + m_Planes[i].z * pos.z + m_Planes[i].w - radius;
        if (dist > 0) return false;
    }
    return true;
}
bool Camera::rayIntersectSphere(Object* obj){return obj->rayIntersectSphere(getPosition(),getViewVector());}
glm::mat4 Camera::getProjection(){ return m_Projection; }
glm::mat4 Camera::getView(){ return m_View; }
glm::mat4 Camera::getViewProjection(){ return m_Projection * m_View; }
glm::vec3 Camera::getViewVector(){ return glm::vec3(m_View[0][2],m_View[1][2],m_View[2][2]); }
const float Camera::getAngle() const { return m_Angle; }
const float Camera::getAspectRatio() const { return m_AspectRatio; }
const float Camera::getNear() const { return m_Near; }
const float Camera::getFar() const { return m_Far; }