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

Camera::Camera(std::string n, float angle, float aspectRatio, float near, float far,Scene* scene):ObjectBasic(glm::vec3(0),glm::vec3(1),n,scene,false){//create a perspective camera
    m_Angle = angle;
    m_AspectRatio = aspectRatio;
    m_Near = near;
    m_Far = far;
    m_Scene = scene;
    m_Type = CAMERA_TYPE_PERSPECTIVE;
    setPerspectiveProjection();
    lookAt(getPosition(),getPosition() + getForward(), getUp());

    Resources::Detail::ResourceManagement::_addToContainer(Resources::Detail::ResourceManagement::m_Cameras,name(),boost::shared_ptr<Camera>(this));
}
Camera::Camera(std::string n, float left, float right, float bottom, float top, float near, float far,Scene* scene):ObjectBasic(glm::vec3(0),glm::vec3(1),n,scene,false){//create an orthographic camera
    m_Angle = 45.0f;
    m_AspectRatio = 1.0f;
    m_Near = near;
    m_Far = far;
    m_Scene = scene;
    m_Type = CAMERA_TYPE_ORTHOGRAPHIC;
    setOrthoProjection(left,right,bottom,top);
    lookAt(getPosition(),getPosition() + getForward(), getUp());

    Resources::Detail::ResourceManagement::_addToContainer(Resources::Detail::ResourceManagement::m_Cameras,name(),boost::shared_ptr<Camera>(this));
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

    for(unsigned int i = 0; i < 6; i++){
        glm::vec3 normal(m_Planes[i].x, m_Planes[i].y, m_Planes[i].z);
        m_Planes[i] = -m_Planes[i] / glm::length(normal);
    }
}
void Camera::resize(unsigned int width, unsigned int height){
    if(m_Type == CAMERA_TYPE_PERSPECTIVE){setAspectRatio(float(width)/float(height));}
    else{setOrthoProjection(0,float(width),0,float(height));}
}
Camera::~Camera()
{ 
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