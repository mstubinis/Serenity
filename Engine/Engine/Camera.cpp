#include "Camera.h"
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_access.hpp>
#include <glm/gtc/type_ptr.hpp>

Camera::Camera(float angleVal, float aspectRatioVal, float _near, float _far,Scene* scene): Object("","",glm::vec3(0,0,0),glm::vec3(1,1,1),"Camera",true,scene){//create a perspective camera
	m_Angle = angleVal;
	m_AspectRatio = aspectRatioVal;
	m_Near = _near;
	m_Far = _far;

	m_Type = CAMERA_TYPE_PERSPECTIVE;

	setPerspectiveProjection();
	lookAt(getPosition(),getPosition() + getForward(), getUp());
}
Camera::Camera(float leftVal, float rightVal, float bottomVal, float topVal, float _near, float _far,Scene* scene): Object("","",glm::vec3(0,0,0),glm::vec3(1,1,1),"Camera",true,scene){//create an orthographic camera
	m_Angle = 45.0f;
	m_AspectRatio = 1.0f;
	m_Near = _near;
	m_Far = _far;

	m_Type = CAMERA_TYPE_ORTHOGRAPHIC;

	setOrthoProjection(leftVal,rightVal,bottomVal,topVal);
	lookAt(getPosition(),getPosition() + getForward(), getUp());
}
void Camera::_constructFrustrum(){
	glm::mat4 matrix = m_Projection * m_View;

	glm::vec4 rowX = glm::row(matrix, 0);
	glm::vec4 rowY = glm::row(matrix, 1);
	glm::vec4 rowZ = glm::row(matrix, 2);
	glm::vec4 rowW = glm::row(matrix, 3);

	m_Planes[0] = glm::normalize(rowW + rowX);
	m_Planes[1] = glm::normalize(rowW - rowX);
	m_Planes[2] = glm::normalize(rowW + rowY);
	m_Planes[3] = glm::normalize(rowW - rowY);
	m_Planes[4] = glm::normalize(rowW + rowZ);
	m_Planes[5] = glm::normalize(rowW - rowZ);

	for(unsigned int i = 0; i < 6; i++){
		glm::vec3 normal(m_Planes[i].x, m_Planes[i].y, m_Planes[i].z);
		float length = glm::length(normal);
		m_Planes[i] = -m_Planes[i] / length;
	}
}
bool Camera::sphereIntersectTest(Object* sphere){
	glm::vec3 pos = sphere->getPosition();
	float rad = sphere->getRadius();
	for (unsigned int i = 0; i < 6; i++){
		float dist = m_Planes[i].x * pos.x + m_Planes[i].y * pos.y + m_Planes[i].z * pos.z + m_Planes[i].w - sphere->getRadius();
		if (dist > 0){
			return false;
		}
	}
	return true;
}
Camera::~Camera()
{ 
}
void Camera::setPerspectiveProjection(){ 
	m_Projection = glm::perspective(m_Angle,m_AspectRatio,m_Near,m_Far); 
	flagAsChanged();
}
void Camera::setOrthoProjection(float left, float right, float bottom, float top){
	m_Projection = glm::ortho(left,right,bottom,top,m_Near,m_Far);
	flagAsChanged();
}
void Camera::setAspectRatio(float ratio){ 
	m_AspectRatio = ratio;
	setPerspectiveProjection();
}
void Camera::lookAt(const glm::vec3& target){ 
	m_View = glm::lookAt(m_Position,target,getUp());
	flagAsChanged();
}
void Camera::lookAt(const glm::vec3& target,const glm::vec3& up){ 
	m_View = glm::lookAt(m_Position,target,up); 
	flagAsChanged();
}
void Camera::lookAt(const glm::vec3& eye,const glm::vec3& target,const glm::vec3& up){ 
	m_View = glm::lookAt(eye,target,up); 
	flagAsChanged();
}
void Camera::lookAt(Object* target, bool targetUp){
	if(!targetUp) m_View = glm::lookAt(m_Position,target->getPosition(),getUp());
	else m_View = glm::lookAt(m_Position,target->getPosition(),target->getUp());
	flagAsChanged();
}
glm::mat4 Camera::calculateProjection(glm::mat4& modelMatrix){ return m_Projection * m_View * modelMatrix; }
glm::mat4 Camera::calculateModelView(glm::mat4& modelMatrix){ return m_View * modelMatrix; }
glm::mat4 Camera::calculateViewProjInverted(){ return glm::inverse(m_Projection * m_View); }
void Camera::update(float dt){
	if(m_Changed){
		_constructFrustrum();
	}
	Object::update(dt);
	glm::vec3 pos = getPosition();
	lookAt(pos,pos - getForward(), getUp());
}
void Camera::render(Mesh* mesh, Material* material,bool debug){}
void Camera::render(bool debug){ render(nullptr,nullptr,debug); }