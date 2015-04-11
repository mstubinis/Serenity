#include "Camera.h"
#include "Engine_Resources.h"
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

Camera::Camera(float angleVal, float aspectRatioVal, float _near, float _far): Object("","",glm::vec3(0,0,0),glm::vec3(1,1,1),"Camera"){//create a perspective camera
	m_Angle = angleVal;
	m_AspectRatio = aspectRatioVal;
	m_ClipStart = _near;
	m_ClipEnd = _far;

	m_Type = CAMERA_TYPE_PERSPECTIVE;

	Set_Perspective_Projection();
	LookAt(Position(),Position() + Forward(), Up());
}
Camera::Camera(float leftVal, float rightVal, float bottomVal, float topVal, float _near, float _far): Object("","",glm::vec3(0,0,0),glm::vec3(1,1,1),"Camera"){//create an orthographic camera
	m_Angle = 45.0f;
	m_AspectRatio = 1.0f;
	m_ClipStart = _near;
	m_ClipEnd = _far;

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
	Set_Perspective_Projection();
}
void Camera::LookAt(const glm::vec3& target){ m_View = glm::lookAt(m_Position,target,Up()); }
void Camera::LookAt(const glm::vec3& target,const glm::vec3& up){ m_View = glm::lookAt(m_Position,target,up); }
void Camera::LookAt(const glm::vec3& eye,const glm::vec3& target,const glm::vec3& up){ m_View = glm::lookAt(eye,target,up); }
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