#include "Camera.h"
#include "Engine_Resources.h"
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_access.hpp>
#include <glm/gtc/type_ptr.hpp>

Camera::Camera(float angleVal, float aspectRatioVal, float _near, float _far): Object("","",glm::vec3(0,0,0),glm::vec3(1,1,1),"Camera"){//create a perspective camera
	m_Angle = angleVal;
	m_AspectRatio = aspectRatioVal;
	m_Near = _near;
	m_Far = _far;

	m_Type = CAMERA_TYPE_PERSPECTIVE;

	Set_Perspective_Projection();
	LookAt(Position(),Position() + Forward(), Up());
}
Camera::Camera(float leftVal, float rightVal, float bottomVal, float topVal, float _near, float _far): Object("","",glm::vec3(0,0,0),glm::vec3(1,1,1),"Camera"){//create an orthographic camera
	m_Angle = 45.0f;
	m_AspectRatio = 1.0f;
	m_Near = _near;
	m_Far = _far;

	m_Type = CAMERA_TYPE_ORTHOGRAPHIC;

	Set_Ortho_Projection(leftVal,rightVal,bottomVal,topVal);
	LookAt(Position(),Position() + Forward(), Up());
}
void Camera::_ConstructFrustrum(){
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
bool Camera::SphereIntersectTest(Object* sphere){
	glm::vec3 pos = sphere->Position();
	float rad = sphere->Radius();
	for (unsigned int i = 0; i < 6; i++){
		float dist = m_Planes[i].x * pos.x + m_Planes[i].y * pos.y + m_Planes[i].z * pos.z + m_Planes[i].w - sphere->Radius();
		if (dist > 0) 
			return false;
	}
	return true;
}
Camera::~Camera()
{ 
}
void Camera::Set_Perspective_Projection(){ 
	m_Projection = glm::perspective(m_Angle,m_AspectRatio,m_Near,m_Far); 
	m_Changed = true;
}
void Camera::Set_Ortho_Projection(float left, float right, float bottom, float top){
	m_Projection = glm::ortho(left,right,bottom,top,m_Near,m_Far);
	m_Changed = true;
}
void Camera::Set_Aspect_Ratio(float ratio){ 
	m_AspectRatio = ratio;
	Set_Perspective_Projection();
}
void Camera::LookAt(const glm::vec3& target){ 
	m_View = glm::lookAt(m_Position,target,Up());
	m_Changed = true;
}
void Camera::LookAt(const glm::vec3& target,const glm::vec3& up){ 
	m_View = glm::lookAt(m_Position,target,up); 
	m_Changed = true;
}
void Camera::LookAt(const glm::vec3& eye,const glm::vec3& target,const glm::vec3& up){ 
	m_View = glm::lookAt(eye,target,up); 
	m_Changed = true;
}
void Camera::LookAt(Object* target, bool targetUp){
	if(!targetUp) m_View = glm::lookAt(m_Position,target->Position(),Up());
	else m_View = glm::lookAt(m_Position,target->Position(),target->Up());
	m_Changed = true;
}
glm::mat4 Camera::Calculate_Projection(glm::mat4& modelMatrix){ return m_Projection * m_View * modelMatrix; }
glm::mat4 Camera::Calculate_ModelView(glm::mat4& modelMatrix){ return m_View * modelMatrix; }
glm::mat4 Camera::Calculate_ViewProjInverted(){ return glm::inverse(m_Projection * m_View); }
void Camera::Update(float dt){
	if(m_Changed){
		_ConstructFrustrum();
	}
	Object::Update(dt);
	glm::vec3 pos = Position();
	LookAt(pos,pos - Forward(), Up());
}
void Camera::Render(Mesh* mesh, Material* material,bool debug){}
void Camera::Render(bool debug){ Render(nullptr,nullptr,debug); }