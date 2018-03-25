#include "Engine.h"
#include "Scene.h"
#include "Camera.h"
#include "ObjectDisplay.h"
#include "Engine_Resources.h"

#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_access.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <boost/make_shared.hpp>

using namespace Engine;
using namespace boost;
using namespace std;

class Camera::impl final{
    public:
		void _baseInit(Scene* scene, Camera* super){
			scene->addEntity(super);

			super->m_BasicBody = new ComponentBasicBody();
			super->m_Camera->lookAt(super->m_BasicBody->position(),super->m_BasicBody->position() + super->m_BasicBody->forward(),super->m_BasicBody->up());
			if(scene == nullptr)
				scene = Resources::getCurrentScene();

			super->addComponent(super->m_BasicBody);
			super->addComponent(super->m_Camera);	
		}
		void _init(float& angle, float& aspectRatio, float& _near, float& _far,Scene* scene,Camera* super){
			super->m_Camera = new ComponentCamera(angle,aspectRatio,_near,_far);
			_baseInit(scene,super);
		}
		void _init(float& left, float& right, float& bottom, float& top, float& _near, float& _far,Scene* scene,Camera* super){		
			super->m_Camera = new ComponentCamera(left,right,bottom,top,_near,_far);
            _baseInit(scene,super);
		}
};

Camera::Camera(float angle, float aspectRatio, float _near, float _far,Scene* scene):Entity(),m_i(new impl){//create a perspective camera
	m_i->_init(angle,aspectRatio,_near,_far,scene,this);
}
Camera::Camera(float left, float right, float bottom, float top, float _near, float _far,Scene* scene):Entity(),m_i(new impl){//create an orthographic camera
	m_i->_init(left,right,bottom,top,_near,_far,scene,this);
}
Camera::~Camera(){ 
}
glm::vec3 Camera::getPosition(){ return m_BasicBody->position(); }
glm::quat Camera::getOrientation(){ return glm::conjugate(glm::quat_cast(m_Camera->getView()));; }
float Camera::getNear(){ return m_Camera->_nearPlane; }
float Camera::getFar(){ return m_Camera->_farPlane; }
glm::mat4 Camera::getViewProjectionInverse(){ return m_Camera->getViewProjectionInverse(); }
glm::mat4 Camera::getProjection(){ return m_Camera->getProjection(); }
glm::mat4 Camera::getView(){ return m_Camera->getView(); }
glm::mat4 Camera::getViewInverse(){ return m_Camera->getViewInverse(); }
glm::mat4 Camera::getProjectionInverse(){ return m_Camera->getProjectionInverse(); }
glm::mat4 Camera::getViewProjection(){ return m_Camera->getViewProjection(); }
glm::vec3 Camera::getViewVector(){ return m_Camera->getViewVector(); }
glm::vec3 Camera::forward(){ return m_BasicBody->forward(); }
glm::vec3 Camera::right(){ return m_BasicBody->right(); }
glm::vec3 Camera::up(){ return m_BasicBody->up(); }
float Camera::getDistance(Object* obj){
	return glm::distance(obj->getPosition(),getPosition());
}
float Camera::getDistance(glm::vec3 objPos){
	return glm::distance(objPos,getPosition());
}
bool Camera::sphereIntersectTest(Object* obj){
	return m_Camera->sphereIntersectTest(obj->getPosition(),obj->getRadius());
}
bool Camera::sphereIntersectTest(glm::vec3 pos, float radius){
	return m_Camera->sphereIntersectTest(pos,radius);
}
bool Camera::rayIntersectSphere(Object* obj){
	return obj->rayIntersectSphere(m_BasicBody->position(),m_Camera->getViewVector());
}
bool Camera::rayIntersectSphere(Entity* entity){
	epriv::ComponentBodyBaseClass& body = *(entity->getComponent<epriv::ComponentBodyBaseClass>());
	ComponentModel& model = *(entity->getComponent<ComponentModel>());
	return Engine::Math::rayIntersectSphere(body.position(),model.radius(),m_BasicBody->position(),m_Camera->getViewVector());
}