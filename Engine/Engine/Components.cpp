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
#include "MeshInstance.h"
#include "Engine_ThreadManager.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_access.hpp>

#include "Engine_Physics.h"
#include <bullet/btBulletCollisionCommon.h>
#include <bullet/btBulletDynamicsCommon.h>

using namespace Engine;
using namespace std;

boost::unordered_map<boost_type_index,uint> epriv::ComponentTypeRegistry::m_Map;
boost::unordered_map<boost_type_index,uint> epriv::ComponentTypeRegistry::m_MapScene;
epriv::EntityPool<ComponentBaseClass>* epriv::ComponentManager::m_ComponentPool;
unordered_map<uint, vector<ComponentBaseClass*>> epriv::ComponentManager::m_ComponentVectors;
unordered_map<uint, vector<ComponentBaseClass*>> epriv::ComponentManager::m_ComponentVectorsScene;

epriv::ComponentManager* componentManager = nullptr;

class epriv::ComponentInternalFunctionality final{
    public:
		static void rebuildProjectionMatrix(ComponentCamera& cam){
			if(cam._type == ComponentCamera::Type::Perspective){
				cam._projectionMatrix = glm::perspective(cam._angle,cam._aspectRatio,cam._nearPlane,cam._farPlane);
			}
			else{
				cam._projectionMatrix = glm::ortho(cam._left,cam._right,cam._bottom,cam._top,cam._nearPlane,cam._farPlane);
			}
		}
};

class ComponentModel::impl final{
    public:
        bool m_PassedRenderCheck;
        bool m_Visible;
        void _init(Entity* owner,ComponentModel* super){
            m_PassedRenderCheck = false;
            m_Visible = true;
            super->m_Owner = owner;
        }

        float calculateRadius(ComponentModel* super){
            float maxLength = 0;
            for(auto model:super->models){
                MeshInstance& pair = *model;
                glm::mat4 m = pair.model();
                glm::vec3 localPosition = glm::vec3(m[3][0],m[3][1],m[3][2]);
                float length = glm::length(localPosition) + pair.mesh()->getRadius() * Engine::Math::Max(pair.getScale());
                if(length > maxLength){
                    maxLength = length;
                }
            }
            super->_radius = maxLength;
            if(super->m_Owner){
                epriv::ComponentBodyBaseClass* body = super->m_Owner->getComponent<epriv::ComponentBodyBaseClass>();
                if(body != nullptr){
                    super->_radius *= Engine::Math::Max(body->getScale());
                }
            }
            return super->_radius;
        }
};

class epriv::ComponentManager::impl final{
    public:	
        ComponentTypeRegistry       m_TypeRegistry;
        vector<Entity*>             m_EntitiesToBeDestroyed;
		bool                        m_Paused;
        void _init(const char* name, uint& w, uint& h,epriv::ComponentManager* super){
            super->m_ComponentPool = new EntityPool<ComponentBaseClass>(epriv::MAX_NUM_ENTITIES * ComponentType::_TOTAL);
            super->m_EntityPool = new EntityPool<Entity>(epriv::MAX_NUM_ENTITIES);
			m_Paused = false;
            m_TypeRegistry = ComponentTypeRegistry();


            m_TypeRegistry.emplace<epriv::ComponentBodyBaseClass,ComponentBasicBody,ComponentRigidBody>();
            m_TypeRegistry.emplace<ComponentModel>();
            m_TypeRegistry.emplace<ComponentCamera>();

            m_TypeRegistry.emplaceVector<ComponentBasicBody>();
            m_TypeRegistry.emplaceVector<ComponentRigidBody>();
            m_TypeRegistry.emplaceVector<ComponentModel>();
            m_TypeRegistry.emplaceVector<ComponentCamera>();
        }
        void _postInit(const char* name, uint& w, uint& h){
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
        static void _updateBaseBodiesJob(vector<ComponentBaseClass*>& vec){
            for(uint j = 0; j < vec.size(); ++j){
                ComponentBasicBody& b = *((ComponentBasicBody*)vec.at(j));
                Core::m_Engine->m_ComponentManager->m_i->_performTransformation(b.m_Owner->parent(),b._position,b._rotation,b._scale,b._modelMatrix);
            }
        }
        void _updateComponentBaseBodies(const float& dt){
            uint slot = componentManager->getIndividualComponentTypeSlot<ComponentBasicBody>();
            vector<ComponentBaseClass*>& v = ComponentManager::m_ComponentVectorsScene.at(slot);
            vector<vector<ComponentBaseClass*>>& split = epriv::threading::splitVector(v);

            for(auto vec:split){
                epriv::threading::addJob(_updateBaseBodiesJob,vec);
            }
            epriv::threading::waitForAll();
        }
        void _updateComponentRigidBodies(const float& dt){
            uint slot = componentManager->getIndividualComponentTypeSlot<ComponentRigidBody>();
            vector<ComponentBaseClass*>& v = ComponentManager::m_ComponentVectorsScene.at(slot);
            for(auto c:v){
                ComponentRigidBody& b = *((ComponentRigidBody*)c);
                Engine::Math::recalculateForwardRightUp(b._rigidBody,b._forward,b._right,b._up);
            }
        }
        void _calculateRenderCheck(ComponentModel& m,Camera* camera){
            epriv::ComponentBodyBaseClass& body = *(m.m_Owner->getComponent<epriv::ComponentBodyBaseClass>());
            glm::vec3& pos = body.position();
            if(!m.visible() || !camera->sphereIntersectTest(pos,m._radius) || camera->getDistance(pos) > m._radius * 1100.0f){ //1100 is the visibility threshold
                m.m_i->m_PassedRenderCheck = false;
                return;
            }
            m.m_i->m_PassedRenderCheck = true;
        }
        static void _updateModelComponentsJob(vector<ComponentBaseClass*>& vec,Camera* camera){
            for(uint j = 0; j < vec.size(); ++j){
                ComponentModel& modelComponent = *((ComponentModel*)vec.at(j));
                for(uint i = 0; i < modelComponent.models.size(); ++i){
                    MeshInstance& pair = *modelComponent.models.at(i);
                    if(pair.mesh()){
                        Core::m_Engine->m_ComponentManager->m_i->_performTransformation(modelComponent.m_Owner->parent(),pair.position(),pair.orientation(),pair.getScale(),pair.model());
                        Core::m_Engine->m_ComponentManager->m_i->_calculateRenderCheck(modelComponent,camera);
                    }
                }
            }
        }
        void _updateComponentModels(const float& dt){
            Camera* camera = Resources::getCurrentScene()->getActiveCamera();
            uint slot = componentManager->getIndividualComponentTypeSlot<ComponentModel>();
            vector<ComponentBaseClass*>& v = ComponentManager::m_ComponentVectorsScene.at(slot);
            vector<vector<ComponentBaseClass*>>& split = epriv::threading::splitVector(v);

            for(auto vec:split){
                epriv::threading::addJob(_updateModelComponentsJob,vec,camera);
            }
            epriv::threading::waitForAll();
        }
        void _updateComponentCameras(const float& dt){
            uint slot = componentManager->getIndividualComponentTypeSlot<ComponentCamera>();
            vector<ComponentBaseClass*>& v = ComponentManager::m_ComponentVectorsScene.at(slot);
            for(auto c:v){
                ComponentCamera& cam = *((ComponentCamera*)c);
                _defaultUpdateCameraComponent(dt,cam);
                cam.update(dt);
            }
        }
        void _defaultUpdateCameraComponent(const float& dt,ComponentCamera& cam){
            //update view frustrum
            glm::mat4 vp = cam._projectionMatrix * cam._viewMatrix;
			glm::vec4 rows[4];
			for(ushort i = 0; i < 4; ++i)
				rows[i] = glm::row(vp,i);
			for(ushort i = 0; i < 3; ++i){
				ushort index = i * 2;
                cam._planes[index  ] = glm::normalize(rows[3] + rows[i]);  //0,2,4
                cam._planes[index+1] = glm::normalize(rows[3] - rows[i]);  //1,3,5
			}
            for(ushort i = 0; i < 6; ++i){
                cam._planes[i] = -cam._planes[i] / glm::length(cam._planes[i]);
            }
        }
        void _updateCurrentScene(const float& dt){
            Scene* currentScene = Resources::getCurrentScene();
            Camera* active = currentScene->getActiveCamera();
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
			if(!m_Paused){	
				_updateComponentBaseBodies(dt);
				_updateComponentRigidBodies(dt);
				_updateComponentModels(dt);
			}
            _updateComponentCameras(dt);

            _destroyQueuedEntities(super);
        }
        void _render(){
            
        }
};
epriv::ComponentManager::ComponentManager(const char* name, uint w, uint h):m_i(new impl){ m_i->_init(name,w,h,this); componentManager = this; }
epriv::ComponentManager::~ComponentManager(){ m_i->_destruct(this); }

void epriv::ComponentManager::_init(const char* name, uint w, uint h){ m_i->_postInit(name,w,h); }

void epriv::ComponentManager::_pause(bool b){ m_i->m_Paused = b; }
void epriv::ComponentManager::_unpause(){ m_i->m_Paused = false; }
void epriv::ComponentManager::_update(const float& dt){ m_i->_update(dt,this); }
void epriv::ComponentManager::_resize(uint width,uint height){
    uint slot = componentManager->getIndividualComponentTypeSlot<ComponentCamera>();
    for(auto camera:ComponentManager::m_ComponentVectors.at(slot)){ 
        ComponentCamera& cam = *(ComponentCamera*)camera;
        cam.resize(width,height); 
    }
}
void epriv::ComponentManager::_deleteEntityImmediately(Entity* e){
    //obviously try to improve this performance wise
    for(uint i = 0; i < ComponentType::_TOTAL; ++i){
        ComponentBaseClass* base = nullptr;
        m_ComponentPool->get(e->m_Components[i],base);
        if(base){
            uint slot = componentManager->getIndividualComponentTypeSlot(base);
            removeFromVector(m_ComponentVectors.at(slot),base);
            removeFromVector(m_ComponentVectorsScene.at(slot),base);
        }
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
void epriv::ComponentManager::_sceneSwap(Scene* oldScene, Scene* newScene){

    //TODO: add method to handle each component type on scene swap (like remove/add rigid body from physics world)

    for(auto type:m_ComponentVectorsScene){
        vector<ComponentBaseClass*>& v = (type.second);
        vector_clear(v);
        vector<ComponentBaseClass*> n;
        m_ComponentVectorsScene.at(type.first) = n;
    }
    for(auto entityID:newScene->entities()){
        Entity* e = newScene->getEntity(entityID);
        for(uint index = 0; index < ComponentType::_TOTAL; ++index){
            uint componentID = e->m_Components[index];
            if(componentID != std::numeric_limits<uint>::max()){
                ComponentBaseClass* component = nullptr;
                m_ComponentPool->get(componentID,component);
                if(component){
                    uint slot = componentManager->getIndividualComponentTypeSlot(component);
                    //if(!m_ComponentVectorsScene.count(slot)){
                        
                    //}
                    //else{
                        m_ComponentVectorsScene.at(slot).push_back(component);
                    //}
                }
            }
        }
    }
}

Entity* epriv::ComponentManager::_getEntity(uint id){
    Entity* e = nullptr; m_EntityPool->getAsFast(id,e); return e;
}
ComponentBaseClass* epriv::ComponentManager::_getComponent(uint index){
    ComponentBaseClass* c = nullptr; m_ComponentPool->getAsFast(index,c); return c;
}
void epriv::ComponentManager::_removeComponent(uint componentID){
    ComponentBaseClass* component = nullptr;
    m_ComponentPool->getAsFast(componentID,component);
    epriv::ComponentManager::_removeComponent(component);
}
void epriv::ComponentManager::_removeComponent(ComponentBaseClass* component){
    if(component){
        uint slot = componentManager->getIndividualComponentTypeSlot(component);
        removeFromVector(m_ComponentVectors.at(slot),component);
        removeFromVector(m_ComponentVectorsScene.at(slot),component);
    }
}

ComponentBaseClass::ComponentBaseClass(){ m_Owner = nullptr; }
ComponentBaseClass::~ComponentBaseClass(){}

epriv::ComponentBodyBaseClass::ComponentBodyBaseClass(epriv::ComponentBodyType::Type type):ComponentBaseClass(){ _type = type; }
epriv::ComponentBodyBaseClass::~ComponentBodyBaseClass(){}
epriv::ComponentBodyType::Type epriv::ComponentBodyBaseClass::getBodyType(){ return _type; }


#pragma region BasicBody

ComponentBasicBody::ComponentBasicBody():epriv::ComponentBodyBaseClass(epriv::ComponentBodyType::BasicBody){
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
glm::vec3 ComponentBasicBody::getScreenCoordinates(){ return Engine::Math::getScreenCoordinates(position(),false); }
glm::vec3 ComponentBasicBody::getScale(){ return _scale; }
glm::vec3 ComponentBasicBody::forward(){ return _forward; }
glm::vec3 ComponentBasicBody::right(){ return _right; }
glm::vec3 ComponentBasicBody::up(){ return _up; }
glm::mat4 ComponentBasicBody::modelMatrix(){ return _modelMatrix; }
glm::quat ComponentBasicBody::rotation(){ return _rotation; }
void ComponentBasicBody::translate(glm::vec3& translation,bool local){ ComponentBasicBody::translate(translation.x,translation.y,translation.z,local); }
void ComponentBasicBody::translate(float x,float y,float z,bool local){
    _position.x += x; _position.y += y; _position.z += z;
    glm::vec3 offset = glm::vec3(x,y,z);
    if(local){
        offset = _rotation * offset;
    }
    setPosition(_position + offset);
}
void ComponentBasicBody::setPosition(glm::vec3& newPosition){ ComponentBasicBody::setPosition(newPosition.x,newPosition.y,newPosition.z); }
void ComponentBasicBody::setPosition(float x,float y,float z){
    _position.x = x; _position.y = y; _position.z = z;
    _modelMatrix[3][0] = x;
    _modelMatrix[3][1] = y;
    _modelMatrix[3][2] = z;
}
void ComponentBasicBody::rotate(glm::vec3& rotation){ ComponentBasicBody::rotate(rotation.x,rotation.y,rotation.z); }
void ComponentBasicBody::rotate(float pitch,float yaw,float roll){
    if(abs(pitch) >= 0.001f) _rotation = _rotation * (glm::angleAxis(-pitch, glm::vec3(1,0,0)));   //pitch
    if(abs(yaw) >= 0.001f)   _rotation = _rotation * (glm::angleAxis(-yaw,   glm::vec3(0,1,0)));   //yaw
    if(abs(roll) >= 0.001f)  _rotation = _rotation * (glm::angleAxis(roll,   glm::vec3(0,0,1)));   //roll
    Engine::Math::recalculateForwardRightUp(_rotation,_forward,_right,_up);
}
void ComponentBasicBody::scale(glm::vec3& amount){ ComponentBasicBody::scale(amount.x,amount.y,amount.z); }
void ComponentBasicBody::scale(float x,float y,float z){
    _scale.x += x; _scale.y += y; _scale.z += z;
    if(m_Owner){
        ComponentModel* models = m_Owner->getComponent<ComponentModel>();
        if(models){
            models->m_i->calculateRadius(models);
        }
    }
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
    if(m_Owner){
        ComponentModel* models = m_Owner->getComponent<ComponentModel>();
        if(models){
            models->m_i->calculateRadius(models);
        }
    }
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
        _aspectRatio = width/(float)height;
    }
	epriv::ComponentInternalFunctionality::rebuildProjectionMatrix(*this);
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

float ComponentCamera::getAngle(){ return _angle; }
float ComponentCamera::getAspect(){ return _aspectRatio; }
float ComponentCamera::getNear(){ return _nearPlane; }
float ComponentCamera::getFar(){ return _farPlane; }
void ComponentCamera::setAngle(float a){ _angle = a; epriv::ComponentInternalFunctionality::rebuildProjectionMatrix(*this); }
void ComponentCamera::setAspect(float a){ _aspectRatio = a; epriv::ComponentInternalFunctionality::rebuildProjectionMatrix(*this); }
void ComponentCamera::setNear(float n){ _nearPlane = n; epriv::ComponentInternalFunctionality::rebuildProjectionMatrix(*this); }
void ComponentCamera::setFar(float f){ _farPlane = f; epriv::ComponentInternalFunctionality::rebuildProjectionMatrix(*this); }

#pragma endregion

#pragma region Model

ComponentModel::ComponentModel(Handle& meshHandle,Handle& materialHandle,Entity* owner):ComponentBaseClass(),m_i(new impl){
    m_i->_init(owner,this);
    if(!meshHandle.null()){
        models.push_back( new MeshInstance(m_Owner,meshHandle,materialHandle) );
    }
    m_i->calculateRadius(this);
}
ComponentModel::ComponentModel(Mesh* mesh,Handle& materialHandle,Entity* owner):ComponentBaseClass(),m_i(new impl){
    m_i->_init(owner,this);
    if(mesh){
        models.push_back( new MeshInstance(m_Owner,mesh,materialHandle) );
    }
    m_i->calculateRadius(this);
}
ComponentModel::ComponentModel(Handle& meshHandle,Material* material,Entity* owner):ComponentBaseClass(),m_i(new impl){
    m_i->_init(owner,this);
    if(!meshHandle.null()){
        models.push_back( new MeshInstance(m_Owner,meshHandle,material) );
    }
    m_i->calculateRadius(this);
}
ComponentModel::ComponentModel(Mesh* mesh,Material* material,Entity* owner):ComponentBaseClass(),m_i(new impl){
    m_i->_init(owner,this);
    if(mesh){
        models.push_back( new MeshInstance(m_Owner,mesh,material) );
    }
    m_i->calculateRadius(this);
}
ComponentModel::~ComponentModel(){
    models.clear();
}
MeshInstance* ComponentModel::getModel(uint index){ return models.at(index); }
bool ComponentModel::passedRenderCheck(){ return m_i->m_PassedRenderCheck; }
bool ComponentModel::visible(){ return m_i->m_Visible; }
void ComponentModel::show(){ m_i->m_Visible = true; }
void ComponentModel::hide(){ m_i->m_Visible = false; }
float ComponentModel::radius(){ return _radius; }
uint ComponentModel::addModel(Handle& meshHandle, Handle& materialHandle){ return ComponentModel::addModel(Resources::getMesh(meshHandle),Resources::getMaterial(materialHandle)); }
uint ComponentModel::addModel(Mesh* mesh,Material* material){
    models.push_back( new MeshInstance(m_Owner,mesh,material) );
    m_i->calculateRadius(this);
    return models.size() - 1;
}

void ComponentModel::setModel(Handle& meshHandle,Handle& materialHandle,uint index){ ComponentModel::setModel(Resources::getMesh(meshHandle),Resources::getMaterial(materialHandle), index); }
void ComponentModel::setModel(Mesh* mesh,Material* material,uint index){
    MeshInstance& pair = *(models.at(index));
    pair.setMesh(mesh);
    pair.setMaterial(material);
    m_i->calculateRadius(this);
}

void ComponentModel::setModelMesh(Mesh* mesh,uint index){
    MeshInstance& pair = *(models.at(index));
    pair.setMesh(mesh);
    m_i->calculateRadius(this);
}
void ComponentModel::setModelMesh(Handle& meshHandle, uint index){ ComponentModel::setModelMesh(Resources::getMesh(meshHandle),index); }

void ComponentModel::setModelMaterial(Material* material,uint index){
    MeshInstance& pair = *(models.at(index));
    pair.setMaterial(material);
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
    return false;
}

#pragma endregion

#pragma region RigidBody

ComponentRigidBody::ComponentRigidBody(Collision* collision,Entity* owner):epriv::ComponentBodyBaseClass(epriv::ComponentBodyType::RigidBody){
    _forward = glm::vec3(0.0f,0.0f,-1.0f);  _right = glm::vec3(1.0f,0.0f,0.0f);  _up = glm::vec3(0.0f,1.0f,0.0f);
    m_Owner = owner;
    setCollision(collision,false);
    setScale(1.0f,1.0f,1.0f);

    //motion state/////////////////////////////////////
    glm::mat4 modelMatrix = glm::mat4(1.0f);
    modelMatrix *= glm::mat4_cast(glm::quat());
    modelMatrix = glm::scale(modelMatrix,glm::vec3(1.0f));
    btTransform tr; tr.setFromOpenGLMatrix(glm::value_ptr(modelMatrix));
    _motionState = new btDefaultMotionState(tr);
    ///////////////////////////////////////////////////
    setMass(1.0f);
    btRigidBody::btRigidBodyConstructionInfo CI(_mass,_motionState,_collision->getCollisionShape(),*(_collision->getInertia()));
    _rigidBody = new btRigidBody(CI);

    _rigidBody->setSleepingThresholds(0.015f,0.015f);
    _rigidBody->setFriction(0.3f);
    _rigidBody->setDamping(0.1f,0.4f);//this makes the objects slowly slow down in space, like air friction
    _rigidBody->setUserPointer(this);
    _rigidBody->setMassProps(  _mass, *(_collision->getInertia())   );

    if(m_Owner->scene() == Resources::getCurrentScene())
        Physics::addRigidBody(_rigidBody);
}
ComponentRigidBody::~ComponentRigidBody(){
    Physics::removeRigidBody(_rigidBody);
    SAFE_DELETE(_rigidBody);
    SAFE_DELETE(_motionState);
}
void ComponentRigidBody::setCollision(Collision* collision,bool emptyCollision){
    _collision = collision;
    if(_collision == nullptr){
        if(m_Owner && emptyCollision == false){
            ComponentModel* model = m_Owner->getComponent<ComponentModel>();
            if(model && model->models.size() > 0){
                btCompoundShape* shape = new btCompoundShape();
                for(auto m:model->models){
                    btTransform t; t.setFromOpenGLMatrix(glm::value_ptr(m->model()));
                    shape->addChildShape(t,m->mesh()->getCollision()->getCollisionShape());
                }
                _collision = new Collision(shape,CollisionType::Compound, _mass);
                _collision->getCollisionShape()->setUserPointer(this);
                return;
            }
        }
    }
    _collision = new Collision(new btEmptyShape(),CollisionType::None,_mass);
    _collision->getCollisionShape()->setUserPointer(this);
}
void ComponentRigidBody::translate(glm::vec3& translation,bool local){ ComponentRigidBody::translate(translation.x,translation.y,translation.z,local); }
void ComponentRigidBody::translate(float x,float y,float z,bool local){
    _rigidBody->activate();
    btVector3 vec = btVector3(x,y,z);
    Engine::Math::translate(_rigidBody,vec,local);
    setPosition(  position() + Engine::Math::btVectorToGLM(vec)  );
}
void ComponentRigidBody::rotate(glm::vec3& rotation,bool local){ ComponentRigidBody::rotate(rotation.x,rotation.y,rotation.z,local); }
void ComponentRigidBody::rotate(float pitch,float yaw,float roll,bool local){
    btQuaternion quat = _rigidBody->getWorldTransform().getRotation().normalize();
    glm::quat glmquat = glm::quat(quat.w(),quat.x(),quat.y(),quat.z());

    if(abs(pitch) >= 0.001f) glmquat = glmquat * (glm::angleAxis(-pitch, glm::vec3(1,0,0)));   //pitch
    if(abs(yaw) >= 0.001f)   glmquat = glmquat * (glm::angleAxis(-yaw,   glm::vec3(0,1,0)));   //yaw
    if(abs(roll) >= 0.001f)  glmquat = glmquat * (glm::angleAxis(roll,   glm::vec3(0,0,1)));   //roll

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
    if(_collision->getCollisionType() == CollisionType::TriangleShapeStatic){
        Physics::removeRigidBody(_rigidBody);
        SAFE_DELETE(_rigidBody);
    }
    _motionState->setWorldTransform(tr);
    if(_collision->getCollisionType() == CollisionType::TriangleShapeStatic){
        btRigidBody::btRigidBodyConstructionInfo ci(_mass,_motionState,_collision->getCollisionShape(),*_collision->getInertia()); //use _mass instead of 0?
        _rigidBody = new btRigidBody(ci);
        _rigidBody->setUserPointer(this);
        Physics::addRigidBody(_rigidBody);
    }
    _rigidBody->setMotionState(_motionState); //is this needed?
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

    Engine::Math::recalculateForwardRightUp(_rigidBody,_forward,_right,_up);

    clearAngularForces();
}
void ComponentRigidBody::setScale(glm::vec3& newScale){ ComponentRigidBody::setScale(newScale.x,newScale.y,newScale.z); }
void ComponentRigidBody::setScale(float x,float y,float z){
    _collision->getCollisionShape()->setLocalScaling(btVector3(x,y,z));
    if(m_Owner){
        ComponentModel* models = m_Owner->getComponent<ComponentModel>();
        if(models){
            models->m_i->calculateRadius(models);
        }
    }
}  
const btRigidBody* ComponentRigidBody::getBody() const{ return _rigidBody; }
glm::vec3 ComponentRigidBody::position(){ //theres prob a better way to do this
    glm::mat4 m(1.0f);
    btTransform tr;  _rigidBody->getMotionState()->getWorldTransform(tr);
    tr.getOpenGLMatrix(glm::value_ptr(m));
    return glm::vec3(m[3][0],m[3][1],m[3][2]);
}
glm::vec3 ComponentRigidBody::getScreenCoordinates(){ return Engine::Math::getScreenCoordinates(position(),false); }
glm::quat ComponentRigidBody::rotation(){ return Engine::Math::btToGLMQuat(_rigidBody->getWorldTransform().getRotation()); }
glm::vec3 ComponentRigidBody::forward(){ return _forward; }
glm::vec3 ComponentRigidBody::right(){ return _right; }
glm::vec3 ComponentRigidBody::up(){ return _up; }
glm::vec3 ComponentRigidBody::getLinearVelocity(){ btVector3 v = _rigidBody->getLinearVelocity(); return Engine::Math::btVectorToGLM(v); }
glm::vec3 ComponentRigidBody::getAngularVelocity(){ btVector3 v = _rigidBody->getAngularVelocity(); return Engine::Math::btVectorToGLM(v); }
float ComponentRigidBody::mass(){ return _mass; }
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
void ComponentRigidBody::setDamping(float linear,float angular){ _rigidBody->setDamping(linear,angular); }
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
    Engine::Math::translate(_rigidBody,vec,local);
    _rigidBody->setLinearVelocity(vec); 
}
void ComponentRigidBody::setLinearVelocity(glm::vec3 velocity,bool local){ ComponentRigidBody::setLinearVelocity(velocity.x,velocity.y,velocity.z,local); }
void ComponentRigidBody::setAngularVelocity(float x,float y,float z,bool local){
    _rigidBody->activate();
    btVector3 vec = btVector3(x,y,z);
    Engine::Math::translate(_rigidBody,vec,local);
    _rigidBody->setAngularVelocity(vec); 
}
void ComponentRigidBody::setAngularVelocity(glm::vec3 velocity,bool local){ ComponentRigidBody::setAngularVelocity(velocity.x,velocity.y,velocity.z,local); }
void ComponentRigidBody::applyForce(float x,float y,float z,bool local){
    _rigidBody->activate();
    btVector3 vec = btVector3(x,y,z);
    Engine::Math::translate(_rigidBody,vec,local);
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
    Engine::Math::translate(_rigidBody,vec,local);
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
        if(_rigidBody){
            _rigidBody->setMassProps(  _mass, *(_collision->getInertia())   );
        }
    }
}


#pragma endregion



Entity::Entity(){
    uint maxValue = std::numeric_limits<uint>::max();
    m_Scene = nullptr;
    m_ParentID = maxValue;
    m_ID = maxValue;
    m_Components = new uint[ComponentType::_TOTAL];
    for(uint i = 0; i < ComponentType::_TOTAL; ++i){
        m_Components[i] = maxValue;
    }
}
Entity::~Entity(){
    uint maxValue = std::numeric_limits<uint>::max();
    m_ParentID = maxValue;
    m_ID = maxValue;
    m_Scene = nullptr;
    delete[] m_Components;
}
uint Entity::id(){ return m_ID; }
Scene* Entity::scene(){ return m_Scene; }
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
    uint maxValue = std::numeric_limits<uint>::max();
    if(m_ParentID == maxValue)
        return nullptr;
    return componentManager->_getEntity(m_ParentID);
}
void Entity::addChild(Entity* child){
    child->m_ParentID = this->m_ID;
}
