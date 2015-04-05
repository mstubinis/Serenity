#include "Camera.h"
#include "Engine_Resources.h"
#include <glm\gtx\transform.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>

Camera::Camera(float angleVal, float aspectRatioVal, float clipStartVal, float clipEndVal): Object("","",glm::vec3(0,0,0),glm::vec3(1,1,1),"Camera"){//create a perspective camera
	m_Angle = angleVal;
	m_AspectRatio = aspectRatioVal;
	m_ClipStart = clipStartVal;
	m_ClipEnd = clipEndVal;

	m_Type = CAMERA_TYPE_PERSPECTIVE;

	Set_Perspective_Projection();
	LookAt(Position(),Position() + Forward(), Up());
}
Camera::Camera(float leftVal, float rightVal, float bottomVal, float topVal, float clipStartVal, float clipEndVal): Object("","",glm::vec3(0,0,0),glm::vec3(1,1,1),"Camera"){//create an orthographic camera
	m_Angle = 45.0f;
	m_AspectRatio = 1.0f;
	m_ClipStart = clipStartVal;
	m_ClipEnd = clipEndVal;

	m_Type = CAMERA_TYPE_ORTHOGRAPHIC;

	Set_Ortho_Projection(leftVal,rightVal,bottomVal,topVal);
	LookAt(Position(),Position() + Forward(), Up());
}
Camera::~Camera(){ 
}
void Camera::Set_Perspective_Projection(){ m_Projection = glm::perspective(m_Angle,m_AspectRatio,m_ClipStart,m_ClipEnd); }
void Camera::Set_Ortho_Projection(float left, float right, float bottom, float top){
	m_Projection = glm::ortho(left,right,bottom,top,m_ClipStart,m_ClipEnd);
}
void Camera::Set_Aspect_Ratio(float ratio){ 
	m_AspectRatio = ratio;
	if(m_Type == CAMERA_TYPE_PERSPECTIVE)
		Set_Perspective_Projection();
}
void Camera::LookAt(glm::vec3& target){ m_View = glm::lookAt(m_Position,target,Up()); }
void Camera::LookAt(glm::vec3& target,glm::vec3& up){ m_View = glm::lookAt(m_Position,target,up); }
void Camera::LookAt(glm::vec3& eye,glm::vec3& target,glm::vec3& up){ m_View = glm::lookAt(eye,target,up); }
void Camera::LookAt(Object* target, bool targetUp){
	if(!targetUp) m_View = glm::lookAt(m_Position,target->Position(),Up());
	else m_View = glm::lookAt(m_Position,target->Position(),target->Up());
}
glm::mat4 Camera::Calculate_Projection(glm::mat4& modelMatrix){ return m_Projection * m_View * modelMatrix; }
glm::mat4 Camera::Calculate_ModelView(glm::mat4& modelMatrix){ return m_View * modelMatrix; }
glm::mat4 Camera::Calculate_ViewProjInverted(){ return glm::inverse(m_Projection * m_View); }
void Camera::Update(float dt){
	Object::Update(dt);
	glm::vec3 pos = Position();
	LookAt(pos,pos - Forward(), Up());
}
void Camera::Render(Mesh* mesh, Material* material,bool debug){}
void Camera::Render(bool debug){ Render(nullptr,nullptr,debug); }
glm::mat4 Camera::Projection() const { return m_Projection; }
glm::mat4 Camera::View() const { return m_View; }
CAMERA_TYPE Camera::Get_Type() const { return m_Type; }
float Camera::Get_Angle_Between(Object* other){
	glm::vec3 a = Forward();
	glm::vec3 b = glm::normalize(other->Position() - m_Position); //object to camera
	float dot_product = glm::dot(a,b);
	float length_of_a = glm::length(a);
	float length_of_b = glm::length(b);

	float cos_angle = dot_product/(length_of_a*length_of_b);
	float angle = acos(cos_angle) * (180/3.14159265359f);
	if(a == b)
		return 0;
	return angle;
}