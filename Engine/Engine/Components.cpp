#include "Components.h"
#include "Engine_Resources.h"
#include "Engine_ObjectPool.h"
#include "Engine.h"
#include "Object.h"
#include "Mesh.h"
#include "Material.h"
#include "Scene.h"
#include "Camera.h"
#include "Skybox.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_access.hpp>

#include "Engine_Physics.h"
#include <bullet/btBulletCollisionCommon.h>
#include <bullet/btBulletDynamicsCommon.h>

using namespace Engine;
using namespace std;

struct epriv::MeshMaterialPair final{
	Mesh* _mesh;
	Material* _material;
	glm::mat4 _modelMatrix;
	glm::vec3 _position, _scale, _forward, _right, _up;
	glm::quat _rotation;
	MeshMaterialPair(Mesh* mesh,Material* material){
		_mesh = mesh; _material = material; _modelMatrix = glm::mat4(1.0f);
		_position = glm::vec3(0.0f); _rotation = glm::quat(); _scale = glm::vec3(1.0f);
		Engine::Math::recalculateForwardRightUp(_rotation,_forward,_right,_up);
	}

	void translate(glm::vec3& translation){ translate(translation.x,translation.y,translation.z); }
	void translate(float x,float y,float z){
		_position.x += x; _position.y += y; _position.z += z;
	}
	void setPosition(glm::vec3& newPosition){ setPosition(newPosition.x,newPosition.y,newPosition.z); }
	void setPosition(float x,float y,float z){
		_position.x = x; _position.y = y; _position.z = z;
	}
	void rotate(glm::vec3& rotation){ rotate(rotation.x,rotation.y,rotation.z); }
	void rotate(float pitch,float yaw,float roll){
		if(abs(pitch) < Object::m_RotationThreshold && abs(yaw) < Object::m_RotationThreshold && abs(roll) < Object::m_RotationThreshold)
			return;
		if(abs(pitch) >= Object::m_RotationThreshold) _rotation = _rotation * (glm::angleAxis(-pitch, glm::vec3(1,0,0)));   //pitch
		if(abs(yaw) >= Object::m_RotationThreshold) _rotation = _rotation * (glm::angleAxis(-yaw, glm::vec3(0,1,0)));   //yaw
		if(abs(roll) >= Object::m_RotationThreshold) _rotation = _rotation * (glm::angleAxis(roll,  glm::vec3(0,0,1)));   //roll
		Engine::Math::recalculateForwardRightUp(_rotation,_forward,_right,_up);
	}
	void scale(glm::vec3& amount){ scale(amount.x,amount.y,amount.z); }
	void scale(float x,float y,float z){
		_scale.x += x; _scale.y += y; _scale.z += z;
	}
	void setRotation(glm::quat& newRotation){
		newRotation = glm::normalize(newRotation);
		_rotation = newRotation;
		Engine::Math::recalculateForwardRightUp(_rotation,_forward,_right,_up);
	}
	void setRotation(float x,float y,float z,float w){ setRotation(glm::quat(w,x,y,z)); }
	void setScale(glm::vec3& newScale){ setScale(newScale.x,newScale.y,newScale.z); }
	void setScale(float x,float y,float z){
		_scale.x = x; _scale.y = y; _scale.z = z;
	}
};


class epriv::ComponentManager::impl final{
    public:

		ObjectPool<ComponentBaseClass>* m_ComponentPool;

		//  TOTAL                                                    //Current Scene Only
		vector<ComponentBasicBody*> m_ComponentBasicBodies;       vector<ComponentBasicBody*> m_CurrentSceneComponentBasicBodies;
		vector<ComponentRigidBody*> m_ComponentRigidBodies;       vector<ComponentRigidBody*> m_CurrentSceneComponentRigidBodies;
		vector<ComponentModel*>     m_ComponentModels;            vector<ComponentModel*>     m_CurrentSceneComponentModels;
		vector<ComponentCamera*>    m_ComponentCameras;           vector<ComponentCamera*>    m_CurrentSceneComponentCameras;

		//implement this workflow somehow with ComponentModels

		//list of shaders
		    //bind shader
		    //list of materials
		       //bind material
		       //list of meshes
		          //bind mesh
		          //list of objects using meshes
		              //bind object
		              //render object
		              //unbind object
		          //unbind mesh
		       //unbind material
		    //unbind shader

		void _init(const char* name, uint& w, uint& h){
			m_ComponentPool = new ObjectPool<ComponentBaseClass>(epriv::MAX_NUM_ENTITIES * ComponentType::_TOTAL);
		}
		void _postInit(const char* name, uint& w, uint& h){
		}
		void _destruct(){
			for(auto c:m_ComponentBasicBodies) SAFE_DELETE(c);
			for(auto c:m_ComponentRigidBodies) SAFE_DELETE(c);
			for(auto c:m_ComponentModels)      SAFE_DELETE(c);
			for(auto c:m_ComponentCameras)     SAFE_DELETE(c);

			SAFE_DELETE(m_ComponentPool);
		}

		void _performTransformation(Entity* parent,glm::vec3& position,glm::quat& rotation,glm::vec3& scale,glm::mat4& modelMatrix){
			if(parent == nullptr){
				modelMatrix = glm::mat4(1.0f);
			}
			else{
				ComponentBodyBaseClass* baseBody = nullptr; baseBody = parent->getComponent(baseBody);
				modelMatrix = baseBody->modelMatrix();
			}
			glm::mat4 translationMat = glm::translate(position);
			glm::mat4 rotationMat = glm::mat4_cast(rotation);
			glm::mat4 scaleMat = glm::scale(scale);
			modelMatrix = translationMat * rotationMat * scaleMat * modelMatrix;
		}
		void _updateComponentBaseBodies(float& dt){
			for(auto c:m_CurrentSceneComponentBasicBodies){
				_performTransformation(c->m_Owner->parent(),c->_position,c->_rotation,c->_scale,c->_modelMatrix);
			}
		}
		void _updateComponentRigidBodies(float& dt){
			for(auto c:m_CurrentSceneComponentRigidBodies){
			}
		}
		void _updateComponentModels(float& dt){
			for(auto c:m_CurrentSceneComponentModels){ 
				for(auto model:c->models){
					if(model._mesh != nullptr){
						_performTransformation(c->m_Owner->parent(),model._position,model._rotation,model._scale,model._modelMatrix);
					}
				}
			}
		}
		void _updateComponentCameras(float& dt){
			for(auto c:m_CurrentSceneComponentCameras){ 
			}
		}
		void _updateCurrentScene(float& dt){
			Scene* currentScene = Resources::getCurrentScene();
			for(auto entityID:currentScene->m_Entities){
				Entity* e = epriv::Core::m_Engine->m_ResourceManager->_getEntity(entityID);
				e->update(dt);
			}



			Camera* active = currentScene->getActiveCamera();
			for (auto it = currentScene->m_Objects.cbegin(); it != currentScene->m_Objects.cend();){
				Object* obj = it->second;
				if (obj->isDestroyed()){
					epriv::Core::m_Engine->m_ResourceManager->_remObject(obj->name());
					currentScene->m_Objects.erase(it++);
				}
				else{
					obj->checkRender(active); //consider batch culling using the thread pool
					obj->update(dt); 
					++it;
				}
			}
			for (auto it = currentScene->m_Cameras.cbegin(); it != currentScene->m_Cameras.cend();){
				Camera* cam = it->second;
				if (cam->isDestroyed()){
					epriv::Core::m_Engine->m_ResourceManager->_remCamera(cam->name());
				}
				else{
					cam->update(dt); 
					++it;
				}
			}
			if(currentScene->m_Skybox != nullptr) currentScene->m_Skybox->update();
		}
		void _update(float& dt){

			_updateCurrentScene(dt);

			_updateComponentBaseBodies(dt);
			_updateComponentRigidBodies(dt);
			_updateComponentModels(dt);
			_updateComponentCameras(dt);
		}
		void _render(){
			
		}
		Handle _addComponent(ComponentBaseClass* component,uint& type){
			return m_ComponentPool->add(component,type);
		}
};
epriv::ComponentManager::ComponentManager(const char* name, uint w, uint h):m_i(new impl){ m_i->_init(name,w,h); }
epriv::ComponentManager::~ComponentManager(){ m_i->_destruct(); }

void epriv::ComponentManager::_init(const char* name, uint w, uint h){ m_i->_postInit(name,w,h); }
void epriv::ComponentManager::_update(float& dt){ m_i->_update(dt); }
Handle epriv::ComponentManager::_addComponent(ComponentBaseClass* component,uint type){
	return m_i->_addComponent(component,type);
}
ComponentBaseClass* epriv::ComponentManager::_getComponent(uint index){
	ComponentBaseClass* c; m_i->m_ComponentPool->getAsFast(index,c); return c;
}
void epriv::ComponentManager::_removeComponent(uint index){
	m_i->m_ComponentPool->remove(index);
}


ComponentBaseClass::ComponentBaseClass(Entity* owner){
	m_Owner = owner;
}
ComponentBaseClass::ComponentBaseClass(uint entityID){
	m_Owner = epriv::Core::m_Engine->m_ResourceManager->_getEntity(entityID);
}
ComponentBaseClass::~ComponentBaseClass(){}
void ComponentBaseClass::setOwner(Entity* owner){
	m_Owner = owner;
}
void ComponentBaseClass::setOwner(uint entityID){
	m_Owner = epriv::Core::m_Engine->m_ResourceManager->_getEntity(entityID);
}




epriv::ComponentBodyBaseClass::ComponentBodyBaseClass(epriv::ComponentBodyType::Type type){ _type = type; }
epriv::ComponentBodyBaseClass::~ComponentBodyBaseClass(){}
epriv::ComponentBodyType::Type epriv::ComponentBodyBaseClass::getBodyType(){ return _type; }



ComponentBasicBody::ComponentBasicBody(Entity* owner):ComponentBaseClass(owner),epriv::ComponentBodyBaseClass(epriv::ComponentBodyType::BasicBody){
	_position = glm::vec3(0.0f);
	_scale = glm::vec3(1.0f);
	_rotation = glm::quat();
	_modelMatrix = glm::mat4(1.0f);
	Engine::Math::recalculateForwardRightUp(_rotation,_forward,_right,_up);
}
ComponentBasicBody::~ComponentBasicBody(){
}
glm::vec3 ComponentBasicBody::position(){ return glm::vec3(_modelMatrix[3][0],_modelMatrix[3][1],_modelMatrix[3][2]); }
glm::mat4 ComponentBasicBody::modelMatrix(){ return _modelMatrix; }
void ComponentBasicBody::translate(glm::vec3& translation){ ComponentBasicBody::translate(translation.x,translation.y,translation.z); }
void ComponentBasicBody::translate(float x,float y,float z){
	_position.x += x; _position.y += y; _position.z += z;
}
void ComponentBasicBody::setPosition(glm::vec3& newPosition){ ComponentBasicBody::setPosition(newPosition.x,newPosition.y,newPosition.z); }
void ComponentBasicBody::setPosition(float x,float y,float z){
	_position.x = x; _position.y = y; _position.z = z;
}
void ComponentBasicBody::rotate(glm::vec3& rotation){ ComponentBasicBody::rotate(rotation.x,rotation.y,rotation.z); }
void ComponentBasicBody::rotate(float pitch,float yaw,float roll){
    if(abs(pitch) < Object::m_RotationThreshold && abs(yaw) < Object::m_RotationThreshold && abs(roll) < Object::m_RotationThreshold)
        return;
    if(abs(pitch) >= Object::m_RotationThreshold) _rotation = _rotation * (glm::angleAxis(-pitch, glm::vec3(1,0,0)));   //pitch
    if(abs(yaw) >= Object::m_RotationThreshold) _rotation = _rotation * (glm::angleAxis(-yaw, glm::vec3(0,1,0)));   //yaw
    if(abs(roll) >= Object::m_RotationThreshold) _rotation = _rotation * (glm::angleAxis(roll,  glm::vec3(0,0,1)));   //roll
    Engine::Math::recalculateForwardRightUp(_rotation,_forward,_right,_up);
}
void ComponentBasicBody::scale(glm::vec3& amount){ ComponentBasicBody::scale(amount.x,amount.y,amount.z); }
void ComponentBasicBody::scale(float x,float y,float z){
	_scale.x += x; _scale.y += y; _scale.z += z;
}
void ComponentBasicBody::setRotation(glm::quat& newRotation){
    newRotation = glm::normalize(newRotation);
    _rotation = newRotation;
    Engine::Math::recalculateForwardRightUp(_rotation,_forward,_right,_up);
}
void ComponentBasicBody::setRotation(float x,float y,float z,float w){ ComponentBasicBody::setRotation(glm::quat(w,x,y,z)); }
void ComponentBasicBody::setScale(glm::vec3& newScale){ ComponentBasicBody::setScale(newScale.x,newScale.y,newScale.z); }
void ComponentBasicBody::setScale(float x,float y,float z){
	_scale.x = x; _scale.y = y; _scale.z = z;
}





ComponentCamera::ComponentCamera(Entity* owner){
	_eye = glm::vec3(0.0f); _up = glm::vec3(0.0f,1.0f,0.0f);
	_angle = glm::radians(60.0f); _aspectRatio = Resources::getWindowSize().x/(float)Resources::getWindowSize().y;
	_nearPlane = 0.1f; _farPlane = 3000.0f;
	_projectionMatrix = glm::perspective(_angle,_aspectRatio,_nearPlane,_farPlane);
	_viewMatrix = glm::lookAt(glm::vec3(0.0f),glm::vec3(0.0f,0.0f,-1.0f),glm::vec3(0.0f,1.0f,0.0f));
}
ComponentCamera::ComponentCamera(Entity* owner,float angle,float aspectRatio,float nearPlane,float farPlane){
	_eye = glm::vec3(0.0f); _up = glm::vec3(0.0f,1.0f,0.0f);
	_angle = glm::radians(angle); _aspectRatio = aspectRatio; _nearPlane = nearPlane; _farPlane = farPlane;
	_projectionMatrix = glm::perspective(_angle,_aspectRatio,_nearPlane,_farPlane);
	_viewMatrix = glm::lookAt(glm::vec3(0.0f),glm::vec3(0.0f,0.0f,-1.0f),glm::vec3(0.0f,1.0f,0.0f));
}
ComponentCamera::ComponentCamera(Entity* owner,float left,float right,float bottom,float top,float nearPlane,float farPlane){
	_eye = glm::vec3(0.0f); _up = glm::vec3(0.0f,1.0f,0.0f);
	_left = left; _right = right; _bottom = bottom; _top = top; _nearPlane = nearPlane; _farPlane = farPlane;
	_projectionMatrix = glm::ortho(_left,_right,_bottom,_top,_nearPlane,_farPlane);
	_viewMatrix = glm::lookAt(glm::vec3(0.0f),glm::vec3(0.0f,0.0f,-1.0f),glm::vec3(0.0f,1.0f,0.0f));
}
ComponentCamera::~ComponentCamera(){
}
void ComponentCamera::update(){
	//update view frustrum
    glm::mat4 vp = _projectionMatrix * _viewMatrix;
    glm::vec4 rowX = glm::row(vp, 0);
    glm::vec4 rowY = glm::row(vp, 1);
    glm::vec4 rowZ = glm::row(vp, 2);
    glm::vec4 rowW = glm::row(vp, 3);

    _planes[0] = glm::normalize(rowW + rowX);
    _planes[1] = glm::normalize(rowW - rowX);
    _planes[2] = glm::normalize(rowW + rowY);
    _planes[3] = glm::normalize(rowW - rowY);
    _planes[4] = glm::normalize(rowW + rowZ);
    _planes[5] = glm::normalize(rowW - rowZ);

    for(uint i = 0; i < 6; ++i){
        glm::vec3 normal(_planes[i].x, _planes[i].y, _planes[i].z);
        _planes[i] = -_planes[i] / glm::length(normal);
    }
}
void ComponentCamera::lookAt(glm::vec3 eye,glm::vec3 forward,glm::vec3 up){
	_eye = eye;
	_up = up;
	_viewMatrix = glm::lookAt(_eye,forward,_up);
	ComponentCamera::update();
}
glm::vec3 ComponentCamera::viewVector(){ return glm::vec3(_viewMatrix[0][2],_viewMatrix[1][2],_viewMatrix[2][2]); }



class ComponentModel::impl final{
    public:
		float calculateRadius(ComponentModel* super){
			float maxLength = 0;
			for(auto model:super->models){
				float length = 0;
				glm::mat4 m = model._modelMatrix;
				glm::vec3 localPosition = glm::vec3(m[3][0],m[3][1],m[3][2]);
				length = glm::length(localPosition) + model._mesh->getRadius() * Engine::Math::Max(model._scale);
				if(length > maxLength){
					maxLength = length;
				}
			}
			super->_radius = maxLength;
			return super->_radius;
		}
};


ComponentModel::ComponentModel(Entity* owner,Mesh* mesh,Material* material):ComponentBaseClass(owner),m_i(new impl){
	if(mesh && material){
		models.push_back( epriv::MeshMaterialPair(mesh,material) );
	}
	m_i->calculateRadius(this);
}
ComponentModel::~ComponentModel(){
	models.clear();
}
uint ComponentModel::addModel(Handle& meshHandle, Handle& materialHandle){ return ComponentModel::addModel(Resources::getMesh(meshHandle),Resources::getMaterial(materialHandle)); }
uint ComponentModel::addModel(Mesh* mesh,Material* material){
	models.push_back( epriv::MeshMaterialPair(mesh,material) );
	m_i->calculateRadius(this);
	return models.size();
}

void ComponentModel::setModel(Handle& meshHandle,Handle& materialHandle,uint index){ ComponentModel::setModel(Resources::getMesh(meshHandle),Resources::getMaterial(materialHandle), index); }
void ComponentModel::setModel(Mesh* mesh,Material* material,uint index){
	epriv::MeshMaterialPair& pair = models.at(index);
	pair._mesh = mesh;
	pair._material = material;
	m_i->calculateRadius(this);
}

void ComponentModel::setModelMesh(Mesh* mesh,uint index){
	epriv::MeshMaterialPair& pair = models.at(index);
	pair._mesh = mesh;
	m_i->calculateRadius(this);
}
void ComponentModel::setModelMesh(Handle& meshHandle, uint index){ ComponentModel::setModelMesh(Resources::getMesh(meshHandle),index); }
		
void ComponentModel::setModelMaterial(Material* material,uint index){
	epriv::MeshMaterialPair& pair = models.at(index);
	pair._material = material;
}
void ComponentModel::setModelMaterial(Handle& materialHandle,uint index){ ComponentModel::setModelMaterial(Resources::getMaterial(materialHandle),index); }
bool ComponentModel::rayIntersectSphere(ComponentCamera* camera){
	epriv::ComponentBodyBaseClass* baseBody = m_Owner->getComponent(baseBody);
	epriv::ComponentBodyType::Type type = baseBody->getBodyType();
	if(type == epriv::ComponentBodyType::BasicBody){
		return Engine::Math::rayIntersectSphere(  ((ComponentBasicBody*)baseBody)->position(),_radius,camera->_eye,camera->viewVector()  );
	}
	else if(type == epriv::ComponentBodyType::RigidBody){
		return Engine::Math::rayIntersectSphere(  ((ComponentRigidBody*)baseBody)->position(),_radius,camera->_eye,camera->viewVector()  );
	}
}



ComponentRigidBody::ComponentRigidBody(Entity* owner):ComponentBaseClass(owner),epriv::ComponentBodyBaseClass(epriv::ComponentBodyType::RigidBody){
	_collision = nullptr;
	_rigidBody = nullptr;
	_motionState = nullptr;
	_mass = 1.0f;
}
ComponentRigidBody::~ComponentRigidBody(){
    Physics::removeRigidBody(_rigidBody);
    SAFE_DELETE(_rigidBody);
    SAFE_DELETE(_motionState);
}
glm::vec3 ComponentRigidBody::position(){ //theres prob a better way to do this
    glm::mat4 m(1.0f);
    btTransform tr;  _rigidBody->getMotionState()->getWorldTransform(tr);
    tr.getOpenGLMatrix(glm::value_ptr(m));
    return glm::vec3(m[3][0],m[3][1],m[3][2]);
}
glm::mat4 ComponentRigidBody::modelMatrix(){ //theres prob a better way to do this
    glm::mat4 m(1.0f);
    btTransform tr;  _rigidBody->getMotionState()->getWorldTransform(tr);
    tr.getOpenGLMatrix(glm::value_ptr(m));
    //btVector3 localScale = m_Collision->getCollisionShape()->getLocalScaling();
    //m = glm::scale(m,glm::vec3(localScale.x(),localScale.y(),localScale.z()));
	return m;
}
void ComponentRigidBody::setDynamic(bool dynamic){
    if(dynamic){
        Physics::removeRigidBody(_rigidBody);
        _rigidBody->setCollisionFlags(btCollisionObject::CF_ANISOTROPIC_FRICTION_DISABLED);
        Physics::addRigidBody(_rigidBody);
        _rigidBody->activate();
    }
    else{
        Physics::removeRigidBody(_rigidBody);
        _rigidBody->setCollisionFlags(btCollisionObject::CF_STATIC_OBJECT);

		ComponentRigidBody::clearAllForces();

        Physics::addRigidBody(_rigidBody);
        _rigidBody->activate();
    }
}
void ComponentRigidBody::setLinearVelocity(float x,float y,float z,bool local){
    _rigidBody->activate();
    btVector3 vec = btVector3(x,y,z);
    if(local){
		btQuaternion q = _rigidBody->getWorldTransform().getRotation().normalize();
        vec = vec.rotate(q.getAxis(),q.getAngle());
	}
    _rigidBody->setLinearVelocity(vec); 
}
void ComponentRigidBody::setLinearVelocity(glm::vec3 velocity,bool local){ ComponentRigidBody::setLinearVelocity(velocity.x,velocity.y,velocity.z,local); }
void ComponentRigidBody::setAngularVelocity(float x,float y,float z,bool local){
    _rigidBody->activate();
    btVector3 vec = btVector3(x,y,z);
    if(local){
		btQuaternion q = _rigidBody->getWorldTransform().getRotation().normalize();
        vec = vec.rotate(q.getAxis(),q.getAngle());
	}
    _rigidBody->setAngularVelocity(vec); 
}
void ComponentRigidBody::setAngularVelocity(glm::vec3 velocity,bool local){ ComponentRigidBody::setAngularVelocity(velocity.x,velocity.y,velocity.z,local); }
void ComponentRigidBody::applyForce(float x,float y,float z,bool local){
    _rigidBody->activate();
    btVector3 vec = btVector3(x,y,z);
    if(local){
		btQuaternion q = _rigidBody->getWorldTransform().getRotation().normalize();
        vec = vec.rotate(q.getAxis(),q.getAngle());
	}
    _rigidBody->applyCentralForce(vec); 
}
void ComponentRigidBody::applyForce(glm::vec3 force,glm::vec3 origin,bool local){
    _rigidBody->activate();
    btVector3 vec = btVector3(force.x,force.y,force.z);
    if(local){
		btQuaternion q = _rigidBody->getWorldTransform().getRotation().normalize();
        vec = vec.rotate(q.getAxis(),q.getAngle());
	}
    _rigidBody->applyForce(vec,btVector3(origin.x,origin.y,origin.z)); 
}
void ComponentRigidBody::applyImpulse(float x,float y,float z,bool local){
    _rigidBody->activate();
    btVector3 vec = btVector3(x,y,z);
    if(local){
		btQuaternion q = _rigidBody->getWorldTransform().getRotation().normalize();
        vec = vec.rotate(q.getAxis(),q.getAngle());
	}
    _rigidBody->applyCentralImpulse(vec);
}
void ComponentRigidBody::applyImpulse(glm::vec3 impulse,glm::vec3 origin,bool local){
    _rigidBody->activate();
    btVector3 vec = btVector3(impulse.x,impulse.y,impulse.z);
    if(local){
		btQuaternion q = _rigidBody->getWorldTransform().getRotation().normalize();
        vec = vec.rotate(q.getAxis(),q.getAngle());
	}
    _rigidBody->applyImpulse(vec,btVector3(origin.x,origin.y,origin.z));
}
void ComponentRigidBody::applyTorque(float x,float y,float z,bool local){
    _rigidBody->activate();
    btVector3 t(x,y,z);
    if(local){
        t = _rigidBody->getInvInertiaTensorWorld().inverse() * (_rigidBody->getWorldTransform().getBasis() * t);
    }
    _rigidBody->applyTorque(t);
}
void ComponentRigidBody::applyTorque(glm::vec3 torque,bool local){ ComponentRigidBody::applyTorque(torque.x,torque.y,torque.z,local); }
void ComponentRigidBody::applyTorqueImpulse(float x,float y,float z,bool local){
    _rigidBody->activate();
    btVector3 t(x,y,z);
    if(local){
        t = _rigidBody->getInvInertiaTensorWorld().inverse() * (_rigidBody->getWorldTransform().getBasis() * t);
    }
    _rigidBody->applyTorqueImpulse(t);
}
void ComponentRigidBody::applyTorqueImpulse(glm::vec3 torqueImpulse,bool local){ ComponentRigidBody::applyTorqueImpulse(torqueImpulse.x,torqueImpulse.y,torqueImpulse.z,local); }
void ComponentRigidBody::clearLinearForces(){
    _rigidBody->setActivationState(0);
	_rigidBody->activate();
	btVector3 vec = btVector3(0,0,0);
	_rigidBody->setLinearVelocity(vec); 
}
void ComponentRigidBody::clearAngularForces(){
    _rigidBody->setActivationState(0);
	_rigidBody->activate();
    btVector3 vec = btVector3(0,0,0);
	_rigidBody->setAngularVelocity(vec); 
}
void ComponentRigidBody::clearAllForces(){
	_rigidBody->setActivationState(0);
	_rigidBody->activate();
	btVector3 vec = btVector3(0,0,0);
	_rigidBody->setLinearVelocity(vec); 
    _rigidBody->setAngularVelocity(vec); 
}
void ComponentRigidBody::setMass(float mass){
    _mass = mass;
	if(_collision){
		_collision->setMass(_mass);
		_rigidBody->setMassProps(_mass,*(_collision->getInertia()));
	}
}