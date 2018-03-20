#include "Components.h"
#include "Engine_Resources.h"
#include "Engine_Renderer.h"
#include "Engine_BuiltInResources.h"
#include "Engine.h"
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

unordered_map<std::type_index,uint> epriv::ComponentTypeRegistry::m_Map;
epriv::ObjectPool<ComponentBaseClass>* epriv::ComponentManager::m_ComponentPool;
unordered_map<std::type_index, vector<ComponentBaseClass*>> epriv::ComponentManager::m_ComponentVectors;

epriv::ComponentManager* componentManager = nullptr;

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
	MeshMaterialPair(Handle& mesh,Handle& material){
		_mesh = Resources::getMesh(mesh); _material = Resources::getMaterial(material); _modelMatrix = glm::mat4(1.0f);
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
		ComponentTypeRegistry       m_TypeRegistry;
		vector<Entity*>             m_EntitiesToBeDestroyed;

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

		void _init(const char* name, uint& w, uint& h,epriv::ComponentManager* super){
			super->m_ComponentPool = new ObjectPool<ComponentBaseClass>(epriv::MAX_NUM_ENTITIES * ComponentType::_TOTAL);
			super->m_EntityPool = new ObjectPool<Entity>(epriv::MAX_NUM_ENTITIES);

			m_TypeRegistry = ComponentTypeRegistry();
			m_TypeRegistry.emplace<ComponentBodyBaseClass>();
			m_TypeRegistry.emplace<ComponentModel>();
			m_TypeRegistry.emplace<ComponentCamera>();

			m_TypeRegistry.emplaceVector<ComponentBasicBody>();
			m_TypeRegistry.emplaceVector<ComponentRigidBody>();
			m_TypeRegistry.emplaceVector<ComponentModel>();
			m_TypeRegistry.emplaceVector<ComponentCamera>();
		}
		void _postInit(const char* name, uint& w, uint& h){
			componentManager = epriv::Core::m_Engine->m_ComponentManager;
		}
		void _destruct(epriv::ComponentManager* super){
			SAFE_DELETE(super->m_ComponentPool);
			SAFE_DELETE(super->m_EntityPool);
		}
		void _performTransformation(Entity* parent,glm::vec3& position,glm::quat& rotation,glm::vec3& scale,glm::mat4& modelMatrix){
			if(parent == nullptr){
				modelMatrix = glm::mat4(1.0f);
			}
			else{
				ComponentBodyBaseClass* baseBody = parent->getComponent<ComponentBodyBaseClass>();
				modelMatrix = baseBody->modelMatrix();
			}
			glm::mat4 translationMat = glm::translate(position);
			glm::mat4 rotationMat = glm::mat4_cast(rotation);
			glm::mat4 scaleMat = glm::scale(scale);
			modelMatrix = translationMat * rotationMat * scaleMat * modelMatrix;
		}
		void _updateComponentBaseBodies(const float& dt){
			vector<ComponentBaseClass*>& v = ComponentManager::m_ComponentVectors[std::type_index(typeid(ComponentBasicBody))];
			for(auto c:v){
				ComponentBasicBody& b = *((ComponentBasicBody*)c);
				_performTransformation(c->m_Owner->parent(),b._position,b._rotation,b._scale,b._modelMatrix);
			}
		}
		void _updateComponentRigidBodies(const float& dt){
			vector<ComponentBaseClass*>& v = ComponentManager::m_ComponentVectors[std::type_index(typeid(ComponentRigidBody))];
			for(auto c:v){
				ComponentRigidBody& b = *((ComponentRigidBody*)c);
			}
		}
		void _updateComponentModels(const float& dt){
			vector<ComponentBaseClass*>& v = ComponentManager::m_ComponentVectors[std::type_index(typeid(ComponentModel))];
			for(auto c:v){
				ComponentModel& modelC = *((ComponentModel*)c);
				for(auto model:modelC.models){
					if(model._mesh != nullptr){
						_performTransformation(c->m_Owner->parent(),model._position,model._rotation,model._scale,model._modelMatrix);
					}
				}
			}
		}
		void _updateComponentCameras(const float& dt){
			vector<ComponentBaseClass*>& v = ComponentManager::m_ComponentVectors[std::type_index(typeid(ComponentCamera))];
			for(auto c:v){
				ComponentCamera& cam = *((ComponentCamera*)c);
				_defaultUpdateCameraComponent(dt,cam);
				cam.update(dt);
			}
		}
		void _defaultUpdateCameraComponent(const float& dt,ComponentCamera& cameraComponent){
			//update view frustrum
			glm::mat4 vp = cameraComponent._projectionMatrix * cameraComponent._viewMatrix;
			glm::vec4 rowX = glm::row(vp, 0);
			glm::vec4 rowY = glm::row(vp, 1);
			glm::vec4 rowZ = glm::row(vp, 2);
			glm::vec4 rowW = glm::row(vp, 3);

			cameraComponent._planes[0] = glm::normalize(rowW + rowX);
			cameraComponent._planes[1] = glm::normalize(rowW - rowX);
			cameraComponent._planes[2] = glm::normalize(rowW + rowY);
			cameraComponent._planes[3] = glm::normalize(rowW - rowY);
			cameraComponent._planes[4] = glm::normalize(rowW + rowZ);
			cameraComponent._planes[5] = glm::normalize(rowW - rowZ);

			for(uint i = 0; i < 6; ++i){
				glm::vec3 normal(cameraComponent._planes[i].x, cameraComponent._planes[i].y, cameraComponent._planes[i].z);
				cameraComponent._planes[i] = -cameraComponent._planes[i] / glm::length(normal);
			}
		}
		void _updateCurrentScene(const float& dt){
			Scene* currentScene = Resources::getCurrentScene();




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


			for(auto entityID:currentScene->m_Entities){
				Entity* e = componentManager->_getEntity(entityID);
				if(e){//should not need this...
				    e->update(dt);
				}
			}



			if(currentScene->m_Skybox != nullptr) currentScene->m_Skybox->update();
		}
		void _destroyQueuedEntities(epriv::ComponentManager* super){
			for(auto e:m_EntitiesToBeDestroyed){
				super->_deleteEntityImmediately(e);
			}
			vector_clear(m_EntitiesToBeDestroyed);
		}
		void _update(const float& dt,epriv::ComponentManager* super){
			_updateCurrentScene(dt);
			
			_updateComponentBaseBodies(dt);
	        _updateComponentRigidBodies(dt);

			_updateComponentModels(dt);
			_updateComponentCameras(dt);

			_destroyQueuedEntities(super);
		}
		void _render(){
			
		}
};
epriv::ComponentManager::ComponentManager(const char* name, uint w, uint h):m_i(new impl){ m_i->_init(name,w,h,this); }
epriv::ComponentManager::~ComponentManager(){ m_i->_destruct(this); }

void epriv::ComponentManager::_init(const char* name, uint w, uint h){ m_i->_postInit(name,w,h); }
void epriv::ComponentManager::_update(float& dt){ m_i->_update(dt,this); }
void epriv::ComponentManager::_resize(uint width,uint height){
	for(auto camera:ComponentManager::m_ComponentVectors[std::type_index(typeid(ComponentCamera))]){ 
		ComponentCamera& cam = *((ComponentCamera*)camera);
		cam.resize(width,height); 
	}
}
void epriv::ComponentManager::_deleteEntityImmediately(Entity* e){
	//obviously try to improve this performance wise
	ComponentBasicBody* basicBody = e->getComponent<ComponentBasicBody>();
	ComponentRigidBody* rigidBody = e->getComponent<ComponentRigidBody>();
	ComponentModel* model = e->getComponent<ComponentModel>();
	ComponentCamera* cam = e->getComponent<ComponentCamera>();
	if(basicBody){
		removeFromVector(ComponentManager::m_ComponentVectors[std::type_index(typeid(ComponentBasicBody))],basicBody);
		//removeFromVector(m_i->m_CurrentSceneComponentBasicBodies,basicBody);
	}
	if(rigidBody){
		removeFromVector(ComponentManager::m_ComponentVectors[std::type_index(typeid(ComponentRigidBody))],rigidBody);
		//removeFromVector(m_i->m_CurrentSceneComponentRigidBodies,rigidBody);
	}
	if(model){
		removeFromVector(ComponentManager::m_ComponentVectors[std::type_index(typeid(ComponentModel))],model);
		//removeFromVector(m_i->m_CurrentSceneComponentModels,model);
	}
	if(cam){
		removeFromVector(ComponentManager::m_ComponentVectors[std::type_index(typeid(ComponentCamera))],cam);
		//removeFromVector(m_i->m_CurrentSceneComponentCameras,cam);
	}
	for(uint i = 0; i < ComponentType::_TOTAL; ++i){
		m_ComponentPool->remove(e->m_Components[i]);
	}
	m_EntityPool->remove(e->m_ID);
}
void epriv::ComponentManager::_addEntityToBeDestroyed(uint id){ Entity* e; m_EntityPool->getAsFast(id,e); _addEntityToBeDestroyed(e); }
void epriv::ComponentManager::_addEntityToBeDestroyed(Entity* e){
	for(auto entity:m_i->m_EntitiesToBeDestroyed){ if(entity->m_ID == e->m_ID){ return; } }
	m_i->m_EntitiesToBeDestroyed.push_back(e);
}

Entity* epriv::ComponentManager::_getEntity(uint id){
	Entity* e; m_EntityPool->getAsFast(id,e); return e;
}
ComponentBaseClass* epriv::ComponentManager::_getComponent(uint index){
	ComponentBaseClass* c; m_ComponentPool->getAsFast(index,c); return c;
}
void epriv::ComponentManager::_removeComponent(uint componentID){
	ComponentBaseClass* component = nullptr;
	m_ComponentPool->getAsFast(componentID,component);
    epriv::ComponentManager::_removeComponent(component);
}
void epriv::ComponentManager::_removeComponent(ComponentBaseClass* component){
	if(component){
		ComponentBasicBody* basicBody = dynamic_cast<ComponentBasicBody*>(component);
		ComponentRigidBody* rigidBody = dynamic_cast<ComponentRigidBody*>(component);
		ComponentModel* model = dynamic_cast<ComponentModel*>(component);
		ComponentCamera* cam = dynamic_cast<ComponentCamera*>(component);
		if(basicBody){
			removeFromVector(ComponentManager::m_ComponentVectors[std::type_index(typeid(ComponentBasicBody))],basicBody);
		}
		if(rigidBody){
			removeFromVector(ComponentManager::m_ComponentVectors[std::type_index(typeid(ComponentRigidBody))],rigidBody);
		}
		if(model){
			removeFromVector(ComponentManager::m_ComponentVectors[std::type_index(typeid(ComponentModel))],model);
		}
		if(cam){
			removeFromVector(ComponentManager::m_ComponentVectors[std::type_index(typeid(ComponentCamera))],cam);
		}
	}
}

ComponentBaseClass::ComponentBaseClass(){}
ComponentBaseClass::~ComponentBaseClass(){}

epriv::ComponentBodyBaseClass::ComponentBodyBaseClass(epriv::ComponentBodyType::Type type){ _type = type; }
epriv::ComponentBodyBaseClass::~ComponentBodyBaseClass(){}
epriv::ComponentBodyType::Type epriv::ComponentBodyBaseClass::getBodyType(){ return _type; }


#pragma region BasicBody

ComponentBasicBody::ComponentBasicBody():ComponentBaseClass(),epriv::ComponentBodyBaseClass(epriv::ComponentBodyType::BasicBody){
	_position = glm::vec3(0.0f);
	_scale = glm::vec3(1.0f);
	_rotation = glm::quat();
	_modelMatrix = glm::mat4(1.0f);
	Engine::Math::recalculateForwardRightUp(_rotation,_forward,_right,_up);
}
ComponentBasicBody::~ComponentBasicBody(){
}
void ComponentBasicBody::alignTo(glm::vec3 direction,float speed){
    Engine::Math::alignTo(_rotation,direction,speed);
    Engine::Math::recalculateForwardRightUp(_rotation,_forward,_right,_up);
}
glm::vec3 ComponentBasicBody::position(){ return glm::vec3(_modelMatrix[3][0],_modelMatrix[3][1],_modelMatrix[3][2]); }
glm::vec3 ComponentBasicBody::forward(){ return _forward; }
glm::vec3 ComponentBasicBody::right(){ return _right; }
glm::vec3 ComponentBasicBody::up(){ return _up; }
glm::mat4 ComponentBasicBody::modelMatrix(){ return _modelMatrix; }
glm::quat ComponentBasicBody::rotation(){ return _rotation; }
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

#pragma endregion

#pragma region Camera

ComponentCamera::ComponentCamera():ComponentBaseClass(){
	_eye = glm::vec3(0.0f); _up = glm::vec3(0.0f,1.0f,0.0f);
	_angle = glm::radians(60.0f); _aspectRatio = Resources::getWindowSize().x/(float)Resources::getWindowSize().y;
	_nearPlane = 0.1f; _farPlane = 3000.0f;
	_projectionMatrix = glm::perspective(_angle,_aspectRatio,_nearPlane,_farPlane);
	_viewMatrix = glm::lookAt(glm::vec3(0.0f),glm::vec3(0.0f,0.0f,-1.0f),glm::vec3(0.0f,1.0f,0.0f));
	_type = Type::Perspective;
}
ComponentCamera::ComponentCamera(float angle,float aspectRatio,float nearPlane,float farPlane):ComponentBaseClass(){
	_eye = glm::vec3(0.0f); _up = glm::vec3(0.0f,1.0f,0.0f);
	_angle = glm::radians(angle); _aspectRatio = aspectRatio; _nearPlane = nearPlane; _farPlane = farPlane;
	_projectionMatrix = glm::perspective(_angle,_aspectRatio,_nearPlane,_farPlane);
	_viewMatrix = glm::lookAt(glm::vec3(0.0f),glm::vec3(0.0f,0.0f,-1.0f),glm::vec3(0.0f,1.0f,0.0f));
	_type = Type::Perspective;
}
ComponentCamera::ComponentCamera(float left,float right,float bottom,float top,float nearPlane,float farPlane):ComponentBaseClass(){
	_eye = glm::vec3(0.0f); _up = glm::vec3(0.0f,1.0f,0.0f);
	_left = left; _right = right; _bottom = bottom; _top = top; _nearPlane = nearPlane; _farPlane = farPlane;
	_projectionMatrix = glm::ortho(_left,_right,_bottom,_top,_nearPlane,_farPlane);
	_viewMatrix = glm::lookAt(glm::vec3(0.0f),glm::vec3(0.0f,0.0f,-1.0f),glm::vec3(0.0f,1.0f,0.0f));
	_type = Type::Orthographic;
}
ComponentCamera::~ComponentCamera(){
}
void ComponentCamera::update(const float& dt){
}
void ComponentCamera::resize(uint width, uint height){
    if(_type == Type::Perspective){
		_projectionMatrix = glm::perspective(_angle,_aspectRatio,_nearPlane,_farPlane);
	}
    else{
		_projectionMatrix = glm::ortho(_left,_right,_bottom,_top,_nearPlane,_farPlane);
	}
}
bool ComponentCamera::sphereIntersectTest(glm::vec3 position,float radius){
    if(radius <= 0) return false;
    for (uint i = 0; i < 6; ++i){
        if (_planes[i].x * position.x + _planes[i].y * position.y + _planes[i].z * position.z + _planes[i].w - radius > 0) return false;
    }
    return true;
}
void ComponentCamera::lookAt(glm::vec3 eye,glm::vec3 forward,glm::vec3 up){
	_eye = eye;
	_up = up;
	_viewMatrix = glm::lookAt(_eye,forward,_up);
}
glm::mat4 ComponentCamera::getViewProjectionInverse(){ return glm::inverse(_projectionMatrix * _viewMatrix); }
glm::mat4 ComponentCamera::getProjection(){ return _projectionMatrix; }
glm::mat4 ComponentCamera::getProjectionInverse(){ return glm::inverse(_projectionMatrix); }
glm::mat4 ComponentCamera::getView(){ return _viewMatrix; }
glm::mat4 ComponentCamera::getViewInverse(){ return glm::inverse(_viewMatrix); }
glm::mat4 ComponentCamera::getViewProjection(){ return _projectionMatrix * _viewMatrix; }
glm::vec3 ComponentCamera::getViewVector(){ return glm::vec3(_viewMatrix[0][2],_viewMatrix[1][2],_viewMatrix[2][2]); }

#pragma endregion

#pragma region Model

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


ComponentModel::ComponentModel(Handle& meshHandle,Handle& materialHandle):ComponentBaseClass(),m_i(new impl){
	if(!meshHandle.null() && !materialHandle.null()){
		models.push_back( epriv::MeshMaterialPair(meshHandle,materialHandle) );
	}
	m_i->calculateRadius(this);
}
ComponentModel::ComponentModel(Mesh* mesh,Material* material):ComponentBaseClass(),m_i(new impl){
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
	epriv::ComponentBodyBaseClass* baseBody = m_Owner->getComponent<epriv::ComponentBodyBaseClass>();
	epriv::ComponentBodyType::Type type = baseBody->getBodyType();
	if(type == epriv::ComponentBodyType::BasicBody){
		return Engine::Math::rayIntersectSphere(  ((ComponentBasicBody*)baseBody)->position(),_radius,camera->_eye,camera->getViewVector()  );
	}
	else if(type == epriv::ComponentBodyType::RigidBody){
		return Engine::Math::rayIntersectSphere(  ((ComponentRigidBody*)baseBody)->position(),_radius,camera->_eye,camera->getViewVector()  );
	}
}

#pragma endregion

#pragma region RigidBody

ComponentRigidBody::ComponentRigidBody(Collision* collision):ComponentBaseClass(),epriv::ComponentBodyBaseClass(epriv::ComponentBodyType::RigidBody){
	_rigidBody = nullptr;
	_motionState = nullptr;

	setCollision(collision);


	//motion state/////////////////////////////////////
    glm::mat4 modelMatrix = glm::mat4(1.0f);
    btTransform tr; tr.setFromOpenGLMatrix(glm::value_ptr(modelMatrix));
    _motionState = new btDefaultMotionState(tr);
	///////////////////////////////////////////////////
    btRigidBody::btRigidBodyConstructionInfo CI(_mass,_motionState,_collision->getCollisionShape(),*(_collision->getInertia()));
    _rigidBody = new btRigidBody(CI);

    _rigidBody->setSleepingThresholds(0.015f,0.015f);
    _rigidBody->setFriction(0.3f);
    _rigidBody->setDamping(0.1f,0.4f);//this makes the objects slowly slow down in space, like air friction
	_rigidBody->setUserPointer(this);

	
	setMass(1.0f);

    Physics::addRigidBody(_rigidBody);
}
ComponentRigidBody::~ComponentRigidBody(){
    Physics::removeRigidBody(_rigidBody);
    SAFE_DELETE(_rigidBody);
    SAFE_DELETE(_motionState);
}
void ComponentRigidBody::setCollision(Collision* collision){
	_collision = collision;
	if(_collision == nullptr){
        _collision = new Collision(new btEmptyShape(),CollisionType::None,0.0f);
	}
	_collision->getCollisionShape()->setUserPointer(this);
}


void ComponentRigidBody::translate(glm::vec3& translation,bool local){ ComponentRigidBody::translate(translation.x,translation.y,translation.z,local); }
void ComponentRigidBody::translate(float x,float y,float z,bool local){
    _rigidBody->activate();
    btVector3 vec = btVector3(x,y,z);
    if(local){
		btQuaternion q = _rigidBody->getWorldTransform().getRotation().normalize();
        vec = vec.rotate(q.getAxis(),q.getAngle());
	}
    setPosition(position() + glm::vec3(vec.x(),vec.y(),vec.z()));
}
void ComponentRigidBody::rotate(glm::vec3& rotation,bool local){ ComponentRigidBody::rotate(rotation.x,rotation.y,rotation.z,local); }
void ComponentRigidBody::rotate(float pitch,float yaw,float roll,bool local){
    if(abs(pitch) < Object::m_RotationThreshold && abs(yaw) < Object::m_RotationThreshold && abs(roll) < Object::m_RotationThreshold)
        return;

	btQuaternion quat = _rigidBody->getWorldTransform().getRotation().normalize();
	glm::quat glmquat = glm::quat(quat.w(),quat.x(),quat.y(),quat.z());

    if(abs(pitch) >= Object::m_RotationThreshold) glmquat = glmquat * (glm::angleAxis(-pitch, glm::vec3(1,0,0)));   //pitch
    if(abs(yaw) >= Object::m_RotationThreshold) glmquat = glmquat * (glm::angleAxis(-yaw, glm::vec3(0,1,0)));   //yaw
    if(abs(roll) >= Object::m_RotationThreshold) glmquat = glmquat * (glm::angleAxis(roll,  glm::vec3(0,0,1)));   //roll

	quat = btQuaternion(glmquat.x,glmquat.y,glmquat.z,glmquat.w);
	_rigidBody->getWorldTransform().setRotation(quat);
}
void ComponentRigidBody::scale(glm::vec3& amount){ ComponentRigidBody::scale(amount.x,amount.y,amount.z); }
void ComponentRigidBody::scale(float x,float y,float z){
     btVector3 localScale = _collision->getCollisionShape()->getLocalScaling();
     _collision->getCollisionShape()->setLocalScaling(btVector3(localScale.x()+x,localScale.y()+y,localScale.z()+z));
	 ComponentModel* models = m_Owner->getComponent<ComponentModel>();
	 if(models){
		 models->m_i->calculateRadius(models);
	 }
}

void ComponentRigidBody::setPosition(glm::vec3& newPosition){ ComponentRigidBody::setPosition(newPosition.x,newPosition.y,newPosition.z); }
void ComponentRigidBody::setPosition(float x,float y,float z){
    btTransform tr; tr.setOrigin(btVector3(x,y,z));
    tr.setRotation(_rigidBody->getOrientation());
    _motionState->setWorldTransform(tr);
    if(_collision->getCollisionType() == CollisionType::TriangleShapeStatic){
		//there must be a better way to do this
        Physics::removeRigidBody(_rigidBody);
        SAFE_DELETE(_rigidBody);
        btRigidBody::btRigidBodyConstructionInfo CI(0,_motionState,_collision->getCollisionShape(),*_collision->getInertia());
        _rigidBody = new btRigidBody(CI);
        _rigidBody->setUserPointer(this);
        Physics::addRigidBody(_rigidBody);
    }
    _rigidBody->setWorldTransform(tr);
    _rigidBody->setCenterOfMassTransform(tr);
}
void ComponentRigidBody::setRotation(glm::quat& newRotation){ ComponentRigidBody::setRotation(newRotation.x,newRotation.y,newRotation.z,newRotation.w); }
void ComponentRigidBody::setRotation(float x,float y,float z,float w){
    btQuaternion quat(x,y,z,w);
    quat = quat.normalize();

    btTransform tr; tr.setOrigin(_rigidBody->getWorldTransform().getOrigin());
    tr.setRotation(quat);

    _rigidBody->setWorldTransform(tr);
    _rigidBody->setCenterOfMassTransform(tr);
	_motionState->setWorldTransform(tr);

    clearAngularForces();
}
void ComponentRigidBody::setScale(glm::vec3& newScale){ ComponentRigidBody::setScale(newScale.x,newScale.y,newScale.z); }
void ComponentRigidBody::setScale(float x,float y,float z){
    _collision->getCollisionShape()->setLocalScaling(btVector3(x,y,z));
	 ComponentModel* models = m_Owner->getComponent<ComponentModel>();
	 if(models){
		 models->m_i->calculateRadius(models);
	 }
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
	if(_collision != nullptr){
		btVector3 localScale = _collision->getCollisionShape()->getLocalScaling();
		m = glm::scale(m,glm::vec3(localScale.x(),localScale.y(),localScale.z()));
	}
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


#pragma endregion



Entity::Entity(){
	m_ParentID = m_ID = std::numeric_limits<uint>::max();
	m_Components = new uint[ComponentType::_TOTAL];
	for(uint i = 0; i < ComponentType::_TOTAL; ++i){
		m_Components[i] = std::numeric_limits<uint>::max();
	}
}
Entity::~Entity(){
	m_ParentID = m_ID = std::numeric_limits<uint>::max();
	delete[] m_Components;
}
void Entity::destroy(bool immediate){
	if(!immediate){
		//add to the deletion queue
		componentManager->_addEntityToBeDestroyed(m_ID);
	}
	else{
		//delete immediately
		componentManager->_deleteEntityImmediately(this);
	}
}
Entity* Entity::parent(){
	if(m_ParentID == std::numeric_limits<uint>::max())
		return nullptr;
	return componentManager->_getEntity(m_ParentID);
}
void Entity::addChild(Entity* child){
	child->m_ParentID = this->m_ID;
}
/*
epriv::ComponentBodyBaseClass* Entity::getComponent(epriv::ComponentBodyBaseClass* component){
	return (epriv::ComponentBodyBaseClass*)(componentManager->_getComponent(m_Components[ComponentType::Body]));
}
ComponentBasicBody* Entity::getComponent(ComponentBasicBody* component){
	return static_cast<ComponentBasicBody*>(componentManager->_getComponent(m_Components[ComponentType::Body])); //might have to be dynamic cast
}
ComponentModel* Entity::getComponent(ComponentModel* component){
	return (ComponentModel*)componentManager->_getComponent(m_Components[ComponentType::Model]);
}
ComponentRigidBody* Entity::getComponent(ComponentRigidBody* component){
	return static_cast<ComponentRigidBody*>(componentManager->_getComponent(m_Components[ComponentType::Body])); //might have to be dynamic cast
}
ComponentCamera* Entity::getComponent(ComponentCamera* component){
	return (ComponentCamera*)componentManager->_getComponent(m_Components[ComponentType::Camera]);
}
*/