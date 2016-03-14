#include "Camera.h"
#include "ObjectDisplay.h"
#include "ObjectDynamic.h"
#include "Engine_Resources.h"
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_access.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace Engine;

Camera::Camera(std::string name, float angleVal, float aspectRatioVal, float _near, float _far,Scene* scene):ObjectBasic(glm::v3(0),glm::vec3(1),"ZZZ" + name,scene){//create a perspective camera
	m_Angle = angleVal;
	m_AspectRatio = aspectRatioVal;
	m_Near = _near;
	m_Far = _far;

	m_Type = CAMERA_TYPE_PERSPECTIVE;

	setPerspectiveProjection();
	lookAt(getPosition(),getPosition() + glm::v3(getForward()), glm::v3(getUp()));

	Engine::Resources::Detail::ResourceManagement::m_Cameras[name] = this;
}
Camera::Camera(std::string name, float leftVal, float rightVal, float bottomVal, float topVal, float _near, float _far,Scene* scene):ObjectBasic(glm::v3(0),glm::vec3(1),"ZZZ" + name,scene){//create an orthographic camera
	m_Angle = 45.0f;
	m_AspectRatio = 1.0f;
	m_Near = _near;
	m_Far = _far;

	m_Type = CAMERA_TYPE_ORTHOGRAPHIC;

	setOrthoProjection(leftVal,rightVal,bottomVal,topVal);
	lookAt(getPosition(),getPosition() + glm::v3(getForward()), glm::v3(getUp()));

	Engine::Resources::Detail::ResourceManagement::m_Cameras[name] = this;
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
	if(m_Type == CAMERA_TYPE_PERSPECTIVE){
		float w = static_cast<float>(width);
		float h = static_cast<float>(height);
		setAspectRatio(w/h);
	}
	else{
		setOrthoProjection(0,(float)width,0,(float)height);
	}
}
Camera::~Camera()
{ 
}
void Camera::setPerspectiveProjection(){ 
	m_Projection = glm::perspective(m_Angle,m_AspectRatio,m_Near,m_Far); 
}
void Camera::setOrthoProjection(float left, float right, float bottom, float top){
	m_Projection = glm::ortho(left,right,bottom,top,m_Near,m_Far);
}
void Camera::setAspectRatio(float ratio){ 
	m_AspectRatio = ratio;
	setPerspectiveProjection();
}
void Camera::lookAt(glm::v3 target){ 
	m_View = glm::lookAt(getPosition(),target,glm::v3(getUp()));
}
void Camera::lookAt(glm::v3 target,glm::v3 up){ 
	m_View = glm::lookAt(getPosition(),target,up); 
}
void Camera::lookAt(glm::v3 eye,glm::v3 target,glm::v3 up){ 
	m_View = glm::lookAt(eye,target,up); 
}
void Camera::lookAt(Object* target, bool targetUp){
	if(!targetUp) m_View = glm::lookAt(getPosition(),target->getPosition(),glm::v3(getUp()));
	else m_View = glm::lookAt(getPosition(),target->getPosition(),glm::v3(target->getUp()));
}
glm::mat4 Camera::calculateProjection(glm::mat4 modelMatrix){ return m_Projection * m_View * modelMatrix; }
glm::mat4 Camera::calculateModelView(glm::mat4 modelMatrix){ return m_View * modelMatrix; }
glm::mat4 Camera::calculateViewProjInverted(){ return glm::inverse(m_Projection * m_View); }
void Camera::update(float dt){
	_constructFrustrum();
	ObjectBasic::update(dt);
}
bool Camera::sphereIntersectTest(Object* sphere){
	return sphereIntersectTest(sphere->getPosition(),sphere->getRadius());
}
bool Camera::sphereIntersectTest(glm::v3 pos, float radius){
	if(radius <= 0)
		return false;
	for (unsigned int i = 0; i < 6; i++){
		glm::nType dist = m_Planes[i].x * pos.x + m_Planes[i].y * pos.y + m_Planes[i].z * pos.z + m_Planes[i].w - radius;
		if (dist > 0){
			return false;
		}
	}
	return true;
}
bool Camera::rayIntersectSphere(Object* object){
	return object->rayIntersectSphere(getPosition(),getViewVector());
}