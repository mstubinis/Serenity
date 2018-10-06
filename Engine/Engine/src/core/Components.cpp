//#include "core/Components.h"
/*
#include "core/engine/Engine_ThreadManager.h"
#include "core/engine/Engine_Time.h"
#include "core/engine/Engine_Resources.h"
#include "core/engine/Engine_Renderer.h"
#include "core/engine/Engine_Physics.h"
*/
#include "core/engine/Engine.h"
#include "core/engine/Engine_Math.h"
#include "core/Mesh.h"
#include "core/Scene.h"
#include "core/Camera.h"
#include "core/Skybox.h"
#include "core/MeshInstance.h"

#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <bullet/btBulletCollisionCommon.h>
#include <bullet/btBulletDynamicsCommon.h>

#include <iostream>

using namespace Engine;
using namespace std;

boost::unordered_map<boost_type_index,uint>                   epriv::OLD_ComponentTypeRegistry::m_Registry;
boost::unordered_map<uint, vector<OLD_ComponentBaseClass*>>   epriv::OLD_ComponentManager::m_ComponentVectors;
boost::unordered_map<uint, vector<OLD_ComponentBaseClass*>>   epriv::OLD_ComponentManager::m_ComponentVectorsScene;
epriv::ObjectPool<OLD_ComponentBaseClass>*                    epriv::OLD_ComponentManager::m_ComponentPool;

epriv::OLD_ComponentManager* componentManager = nullptr;

class epriv::OLD_ComponentInternalFunctionality final{
    public:
        static void RebuildProjectionMatrix(OLD_ComponentCamera& cam){
            if(cam._type == OLD_ComponentCamera::Type::Perspective){
                cam._projectionMatrix = glm::perspective(cam._angle,cam._aspectRatio,cam._nearPlane,cam._farPlane);
            }else{
                cam._projectionMatrix = glm::ortho(cam._left,cam._right,cam._bottom,cam._top,cam._nearPlane,cam._farPlane);
            }
        }
        static float CalculateRadius(OLD_ComponentModel& super) {
            float maxLength = 0;
            glm::vec3 boundingBox = glm::vec3(0.0f);
            for (auto meshInstance : super.models) {
                auto& pair = *meshInstance;
                glm::mat4& m = pair.model();
                glm::vec3 localPosition = glm::vec3(m[3][0], m[3][1], m[3][2]);
                float length = glm::length(localPosition) + pair.mesh()->getRadius() * Engine::Math::Max(pair.getScale());
                glm::vec3 box = localPosition + pair.mesh()->getRadiusBox() * Engine::Math::Max(pair.getScale());
                if (length > maxLength) { 
                    maxLength = length; 
                }
                if (box.x > boundingBox.x || box.y > boundingBox.y || box.z > boundingBox.z) {
                    boundingBox = box;
                }
            }
            super._radius = maxLength;
            super._radiusBox = boundingBox;
            if (super.m_Owner != 0) {
                auto* body = super.owner()->getComponent<OLD_ComponentBody>();
                if (body) { 
                    float _bodyScale = Engine::Math::Max(body->getScale());
                    super._radius *= _bodyScale;
                    super._radiusBox *= _bodyScale;
                }
            }
            return super._radius;
        }
};



class epriv::OLD_ComponentManager::impl final{
    public:	
        OLD_ComponentTypeRegistry              m_TypeRegistry;
        vector<OLD_Entity*>                    m_EntitiesToBeDestroyed;
        bool                                   m_Paused;
        vector<OLD_ComponentSystemBaseClass*>  m_Systems;
        void _init(const char* name, uint& w, uint& h, OLD_ComponentManager& super){
            m_Paused = false;
            super.m_ComponentPool = new ObjectPool<OLD_ComponentBaseClass>(epriv::MAX_NUM_ENTITIES * OLD_ComponentType::_TOTAL);
            super.m_EntityPool    = new ObjectPool<OLD_Entity>(epriv::MAX_NUM_ENTITIES);

            m_Systems.resize(OLD_ComponentType::_TOTAL, nullptr);
            m_Systems[0] = new OLD_ComponentBodySystem();
            m_Systems[1] = new OLD_ComponentModelSystem();
            m_Systems[2] = new OLD_ComponentCameraSystem();

            m_TypeRegistry.emplace<OLD_ComponentBody>();
            m_TypeRegistry.emplace<OLD_ComponentModel>();
            m_TypeRegistry.emplace<OLD_ComponentCamera>();
        }
        void _destruct(OLD_ComponentManager& super){
            SAFE_DELETE(super.m_ComponentPool);
            SAFE_DELETE(super.m_EntityPool);
            SAFE_DELETE_VECTOR(m_Systems);
        }
        static void _performTransformation(OLD_Entity* parent,glm::vec3& position,glm::quat& rotation,glm::vec3& scale,glm::mat4& modelMatrix){
            if(!parent){
                modelMatrix = glm::mat4(1.0f);
            }else{
                auto* body = parent->getComponent<OLD_ComponentBody>();
                modelMatrix = body->modelMatrix();
            }
            modelMatrix = glm::translate(position) * glm::mat4_cast(rotation) * glm::scale(scale) * modelMatrix;
        }
        void _updateCurrentScene(const float& dt){
            auto& currentScene = *Resources::getCurrentScene();
            for(auto entityID:InternalScenePublicInterface::OLD_GetEntities(currentScene)){
                OLD_Entity& e = *OLD_Components::GetEntity(entityID);
                e.update(dt);
            }
            if(currentScene.skybox()) currentScene.skybox()->update();
        }
        void _destroyQueuedEntities(OLD_ComponentManager& super){
            for(auto e:m_EntitiesToBeDestroyed){
                super._deleteEntityImmediately(*e);
            }
            vector_clear(m_EntitiesToBeDestroyed);
        }
        void _updatePhysicsEngine(const float& dt){
            Core::m_Engine->m_TimeManager.stop_clock();
            //It's important that timeStep is always less than maxSubSteps * fixedTimeStep, otherwise you are losing time. dt < maxSubSteps * fixedTimeStep
            float minStep = 0.0166666f; // == 0.0166666 at 1 fps
            uint maxSubSteps = 0;
            while(true){
                ++maxSubSteps; if(dt < (maxSubSteps * minStep)) break;
            }
            Core::m_Engine->m_PhysicsManager._update(dt,maxSubSteps,minStep);
            Core::m_Engine->m_TimeManager.calculate_physics();
        }
        void _update(const float& dt, OLD_ComponentManager& super){
            _updateCurrentScene(dt); //take player input and perform player actions
            _updatePhysicsEngine(dt);
            if(!m_Paused){
                m_Systems[OLD_ComponentType::Body]->update(dt);
            }
            m_Systems[OLD_ComponentType::Camera]->update(dt); //update frustum planes
            if(!m_Paused){
                m_Systems[OLD_ComponentType::Model]->update(dt); //transform model matrices and perform render check
            }
            _destroyQueuedEntities(super);
        }
};

epriv::OLD_ComponentManager::OLD_ComponentManager(const char* name, uint w, uint h):m_i(new impl){ m_i->_init(name,w,h,*this); componentManager = this; }
epriv::OLD_ComponentManager::~OLD_ComponentManager(){ m_i->_destruct(*this); }
void epriv::OLD_ComponentManager::_pause(bool b){ m_i->m_Paused = b; }
void epriv::OLD_ComponentManager::_unpause(){ m_i->m_Paused = false; }
void epriv::OLD_ComponentManager::_update(const float& dt){ m_i->_update(dt,*this); }
void epriv::OLD_ComponentManager::_resize(uint width,uint height){
    uint slot = OLD_Components::getSlot<OLD_ComponentCamera>();
    for(auto camera: OLD_ComponentManager::m_ComponentVectors.at(slot)){
        auto& cam = *(OLD_ComponentCamera*)camera;
        cam.resize(width,height);
    }
}
void epriv::OLD_ComponentManager::_deleteEntityImmediately(OLD_Entity& entity){
    //obviously try to improve this performance wise
    removeFromVector(epriv::InternalScenePublicInterface::OLD_GetEntities(*entity.scene()), entity.ID);
    for(uint i = 0; i < OLD_ComponentType::_TOTAL; ++i){
        uint& componentID = entity.m_Components[i];
        if(componentID != 0){
            OLD_ComponentBaseClass* component = OLD_Components::GetComponent(componentID);
            componentManager->_removeComponent(component);
            m_ComponentPool->remove(componentID);
            componentID = 0;
        }
    }
    m_EntityPool->remove(entity.ID);
}
void epriv::OLD_ComponentManager::_addEntityToBeDestroyed(uint id){ _addEntityToBeDestroyed(*m_EntityPool->getAsFast<OLD_Entity>(id)); }
void epriv::OLD_ComponentManager::_addEntityToBeDestroyed(OLD_Entity& entity){
    for(auto destroyed:m_i->m_EntitiesToBeDestroyed){ 
        if(destroyed->ID == entity.ID){
            return; 
        } 
    }
    m_i->m_EntitiesToBeDestroyed.push_back(&entity);
}
void epriv::OLD_ComponentManager::_sceneSwap(Scene* oldScene, Scene* newScene){
    //TODO: add method to handle each component type on scene swap (like remove/add rigid body from physics world)
    for(auto iterator:m_ComponentVectorsScene){
        vector_clear(m_ComponentVectorsScene.at(iterator.first));
    }
    //cleanup scene specific data for components belonging to the old scene
    //if (oldScene) {
        //for (auto entityID : InternalScenePublicInterface::GetEntities(oldScene)) {
            //Entity* e = oldScene->getEntity(entityID);
            //epriv::ComponentManager::onSceneSwap(oldScene, newScene,e);
        //}
    //}
    //transfers the newScene's components into the vector used for scenes
    for(auto entityID:InternalScenePublicInterface::OLD_GetEntities(*newScene)){
        OLD_Entity& e = *newScene->OLD_getEntity(entityID);
        for(uint index = 0; index < OLD_ComponentType::_TOTAL; ++index){
            uint componentID = e.m_Components[index];
            if(componentID != 0){
                OLD_ComponentBaseClass* component = OLD_Components::GetComponent(componentID);
                if(component){
                    uint slot = OLD_Components::getSlot(component);
                    m_ComponentVectorsScene.at(slot).push_back(component);
                }
            }
        }
        epriv::OLD_ComponentManager::onSceneSwap(oldScene,newScene, e);
    }
}
void epriv::OLD_ComponentManager::_removeComponent(uint componentID){
    OLD_ComponentBaseClass* component = OLD_Components::GetComponent(componentID);
    epriv::OLD_ComponentManager::_removeComponent(component);
}
void epriv::OLD_ComponentManager::_removeComponent(OLD_ComponentBaseClass* component){
    if(component){
        uint slot = OLD_Components::getSlot(component);
        removeFromVector(m_ComponentVectors.at(slot),component);
        removeFromVector(m_ComponentVectorsScene.at(slot),component);
    }
}
void epriv::OLD_ComponentManager::onEntityAddedToScene(Scene& scene, OLD_Entity& entity) {
    entity.m_Scene = &scene;
    for (uint i = 0; i < entity.m_Components.size(); ++i) {
        const uint& componentID = entity.m_Components.at(i);
        if (componentID != 0){
            OLD_ComponentBaseClass* component = OLD_Components::GetComponent(entity.m_Components[i]);
            componentManager->m_i->m_Systems[i]->onEntityAddedToScene(scene, component, entity);
        }
    }
}
void epriv::OLD_ComponentManager::onSceneSwap(Scene* oldScene,Scene* newScene, OLD_Entity& entity) {
    for (uint i = 0; i < entity.m_Components.size(); ++i) {
        const uint& componentID = entity.m_Components.at(i);
        if (componentID != 0) {
            OLD_ComponentBaseClass* component = OLD_Components::GetComponent(entity.m_Components[i]);
            componentManager->m_i->m_Systems[i]->onSceneSwap(oldScene,newScene, component, entity);
        }
    }
}
void epriv::OLD_ComponentManager::onComponentAddedToEntity(OLD_Entity& entity) {
    for (uint i = 0; i < entity.m_Components.size(); ++i) {
        const uint& componentID = entity.m_Components.at(i);
        if (componentID != 0) {
            OLD_ComponentBaseClass* component = OLD_Components::GetComponent(entity.m_Components[i]);
            componentManager->m_i->m_Systems[i]->onComponentAddedToEntity(component, entity);
        }
    }
}

#pragma region ComponentSystems

class epriv::OLD_ComponentCameraSystem::impl final {
    public:
        static void _defaultUpdate(vector<OLD_ComponentBaseClass*>& vec, const float& dt) {
            for (auto camera : vec) {
                auto& cmpnt = *(OLD_ComponentCamera*)camera;
                cmpnt.update(dt);//custom camera user code
                Math::extractViewFrustumPlanesHartmannGribbs(cmpnt._projectionMatrix * cmpnt._viewMatrix, cmpnt._planes);//update view frustrum 
            }
        }
        void _update(const float& dt) {
            uint slot = OLD_Components::getSlot<OLD_ComponentCamera>();
            auto& v = OLD_ComponentManager::m_ComponentVectorsScene.at(slot);
            auto split = epriv::threading::splitVector(v);
            for (auto vec : split) {
                epriv::threading::addJob(_defaultUpdate, vec, dt);
            }
            epriv::threading::waitForAll();
        }
        void _onComponentAddedToEntity(OLD_ComponentBaseClass* component, OLD_Entity& _entity) {
            OLD_ComponentCamera& componentCamera = *(OLD_ComponentCamera*)component;
        }
        void _onEntityAddedToScene(Scene& scene, OLD_ComponentBaseClass* component, OLD_Entity& _entity) {
            OLD_ComponentCamera& componentCamera = *(OLD_ComponentCamera*)component;
        }
        void _onSceneSwap(Scene* oldScene,Scene* newScene, OLD_ComponentBaseClass* component, OLD_Entity& _entity) {
            OLD_ComponentCamera& componentCamera = *(OLD_ComponentCamera*)component;
        }
};
epriv::OLD_ComponentCameraSystem::OLD_ComponentCameraSystem() :OLD_ComponentSystemBaseClass(),m_i(new impl) { }
epriv::OLD_ComponentCameraSystem::~OLD_ComponentCameraSystem() { }
void epriv::OLD_ComponentCameraSystem::update(const float& dt) { m_i->_update(dt); }
void epriv::OLD_ComponentCameraSystem::onSceneSwap(Scene* o, Scene* s, OLD_ComponentBaseClass* c, OLD_Entity& e) { m_i->_onSceneSwap(o,s, c, e); }
void epriv::OLD_ComponentCameraSystem::onEntityAddedToScene(Scene& s, OLD_ComponentBaseClass* c, OLD_Entity& e) { m_i->_onEntityAddedToScene(s,c,e); }
void epriv::OLD_ComponentCameraSystem::onComponentAddedToEntity(OLD_ComponentBaseClass* c, OLD_Entity& e) { m_i->_onComponentAddedToEntity(c, e); }
class epriv::OLD_ComponentModelSystem::impl final {
    public:
        static void _calculateRenderCheck(OLD_ComponentModel& m, Camera* camera) {
            auto& body = *(m.owner()->getComponent<OLD_ComponentBody>());
            for (auto meshInstance : m.models) {
                auto& _meshInstance = *meshInstance;
                auto pos = body.position() + _meshInstance.position();
                                                                   //per mesh instance radius instead?
                uint sphereTest = camera->sphereIntersectTest(pos, m._radius);                //per mesh instance radius instead?
                if (!_meshInstance.visible() || sphereTest == 0 || camera->getDistance(pos) > m._radius * 1100.0f) {
                    _meshInstance.setPassedRenderCheck(false);
                    continue;
                }
                _meshInstance.setPassedRenderCheck(true);
            }
        }
        static void _defaultUpdate(vector<OLD_ComponentBaseClass*>& vec, Camera* camera) {
            for (uint j = 0; j < vec.size(); ++j) {
                auto& model = *(OLD_ComponentModel*)vec[j];
                for (uint i = 0; i < model.models.size(); ++i) {
                    auto& pair = *model.models[i];
                    if (pair.mesh()) {
                        //TODO: implement parent->child relationship...?
                        componentManager->m_i->_performTransformation(nullptr, pair.position(), pair.orientation(), pair.getScale(), pair.model());
                        _calculateRenderCheck(model, camera);
                    }
                }
            }
        }
        void _update(const float& dt) {
            auto* camera = Resources::getCurrentScene()->getActiveCamera();
            uint slot = OLD_Components::getSlot<OLD_ComponentModel>();
            auto& v = OLD_ComponentManager::m_ComponentVectorsScene.at(slot);
            auto split = epriv::threading::splitVector(v);
            for (auto vec : split) {
                epriv::threading::addJob(_defaultUpdate, vec, camera);
            }
            epriv::threading::waitForAll();
        }
        void _onComponentAddedToEntity(OLD_ComponentBaseClass* component, OLD_Entity& _entity) {
            OLD_ComponentModel& componentModel = *(OLD_ComponentModel*)component;
            OLD_ComponentInternalFunctionality::CalculateRadius(componentModel);
        }
        void _onEntityAddedToScene(Scene& scene, OLD_ComponentBaseClass* component, OLD_Entity& _entity) {
            OLD_ComponentModel& componentModel = *(OLD_ComponentModel*)component;
            for (auto _meshInstance : componentModel.models) {
                InternalScenePublicInterface::AddMeshInstanceToPipeline(scene, *_meshInstance, _meshInstance->stage());
            }
        }
        void _onSceneSwap(Scene* oldScene, Scene* newScene, OLD_ComponentBaseClass* component, OLD_Entity& _entity) {
            _onEntityAddedToScene(*newScene, component, _entity);
        }
};
epriv::OLD_ComponentModelSystem::OLD_ComponentModelSystem() :OLD_ComponentSystemBaseClass(),m_i(new impl) { }
epriv::OLD_ComponentModelSystem::~OLD_ComponentModelSystem() { }
void epriv::OLD_ComponentModelSystem::update(const float& dt) { m_i->_update(dt); }
void epriv::OLD_ComponentModelSystem::onSceneSwap(Scene* o, Scene* s, OLD_ComponentBaseClass* c, OLD_Entity& e) { m_i->_onSceneSwap(o, s, c, e); }
void epriv::OLD_ComponentModelSystem::onComponentAddedToEntity(OLD_ComponentBaseClass* c, OLD_Entity& e) { m_i->_onComponentAddedToEntity(c, e); }
void epriv::OLD_ComponentModelSystem::onEntityAddedToScene(Scene& s, OLD_ComponentBaseClass* c, OLD_Entity& e) { m_i->_onEntityAddedToScene(s,c, e); }
class epriv::OLD_ComponentBodySystem::impl final {
    public:
        static void _defaultUpdate(vector<OLD_ComponentBaseClass*>& vec) {
            for (uint j = 0; j < vec.size(); ++j) {
                auto& b = *(OLD_ComponentBody*)vec[j];
                if (b._physics) {
                    auto& physicsData = *b.data.p;
                    Engine::Math::recalculateForwardRightUp(physicsData.rigidBody, b._forward, b._right, b._up);
                }else{
                    auto& normalData = *b.data.n;
                    componentManager->m_i->_performTransformation(nullptr, normalData.position, normalData.rotation, normalData.scale, normalData.modelMatrix);
                }
            }
        }
        void _update(const float& dt) {
            const uint slot = OLD_Components::getSlot<OLD_ComponentBody>();
            vector<OLD_ComponentBaseClass*>& v = OLD_ComponentManager::m_ComponentVectorsScene.at(slot);
            auto split = epriv::threading::splitVector(v);
            for (auto vec : split) {
                epriv::threading::addJob(_defaultUpdate, vec);
            }
            epriv::threading::waitForAll();
        }
        void _onComponentAddedToEntity(OLD_ComponentBaseClass* component, OLD_Entity& _entity) {
            OLD_ComponentBody& componentBody = *(OLD_ComponentBody*)component;
            if (componentBody._physics) {
                auto* _collision = componentBody.data.p->collision;
                componentBody.setCollision((CollisionType::Type)_collision->getType(), componentBody.data.p->mass);
            }
        }
        void _onEntityAddedToScene(Scene& scene, OLD_ComponentBaseClass* component, OLD_Entity& _entity) {
            OLD_ComponentBody& componentBody = *(OLD_ComponentBody*)component;
            if (componentBody._physics) {
                auto& physicsData = *componentBody.data.p;
                if (&scene == Resources::getCurrentScene()) {
                    Physics::addRigidBody(physicsData.rigidBody);
                }else{
                    Physics::removeRigidBody(physicsData.rigidBody);
                }
            }       
        }
        void _onSceneSwap(Scene* oldScene, Scene* newScene, OLD_ComponentBaseClass* component, OLD_Entity& _entity) {
            _onEntityAddedToScene(*newScene, component, _entity);
        }
};
epriv::OLD_ComponentBodySystem::OLD_ComponentBodySystem() :OLD_ComponentSystemBaseClass(),m_i(new impl) { }
epriv::OLD_ComponentBodySystem::~OLD_ComponentBodySystem() { }
void epriv::OLD_ComponentBodySystem::update(const float& dt) { m_i->_update(dt); }
void epriv::OLD_ComponentBodySystem::onSceneSwap(Scene* o, Scene* s, OLD_ComponentBaseClass* c, OLD_Entity& e) { m_i->_onSceneSwap(o, s, c, e); }
void epriv::OLD_ComponentBodySystem::onEntityAddedToScene(Scene& s, OLD_ComponentBaseClass* c, OLD_Entity& e) { m_i->_onEntityAddedToScene(s,c,e); }
void epriv::OLD_ComponentBodySystem::onComponentAddedToEntity(OLD_ComponentBaseClass* c, OLD_Entity& e) { m_i->_onComponentAddedToEntity(c, e); }

#pragma endregion

#pragma region Camera

OLD_ComponentCamera::OLD_ComponentCamera(){
    _eye = glm::vec3(0.0f); _up = glm::vec3(0.0f,1.0f,0.0f);
    _angle = glm::radians(60.0f); _aspectRatio = Resources::getWindowSize().x/(float)Resources::getWindowSize().y;
    _nearPlane = 0.1f; _farPlane = 5000.0f;
    _projectionMatrix = glm::perspective(_angle,_aspectRatio,_nearPlane,_farPlane);
    _viewMatrix = glm::lookAt(glm::vec3(0.0f),glm::vec3(0.0f,0.0f,-1.0f),glm::vec3(0.0f,1.0f,0.0f));
    _type = Type::Perspective;
}
OLD_ComponentCamera::OLD_ComponentCamera(float angle,float aspectRatio,float nearPlane,float farPlane){
    _eye = glm::vec3(0.0f); _up = glm::vec3(0.0f,1.0f,0.0f);
    _angle = glm::radians(angle); _aspectRatio = aspectRatio; _nearPlane = nearPlane; _farPlane = farPlane;
    _projectionMatrix = glm::perspective(_angle,_aspectRatio,_nearPlane,_farPlane);
    _viewMatrix = glm::lookAt(glm::vec3(0.0f),glm::vec3(0.0f,0.0f,-1.0f),glm::vec3(0.0f,1.0f,0.0f));
    _type = Type::Perspective;
}
OLD_ComponentCamera::OLD_ComponentCamera(float left,float right,float bottom,float top,float nearPlane,float farPlane){
    _eye = glm::vec3(0.0f); _up = glm::vec3(0.0f,1.0f,0.0f);
    _left = left; _right = right; _bottom = bottom; _top = top; _nearPlane = nearPlane; _farPlane = farPlane;
    _projectionMatrix = glm::ortho(_left,_right,_bottom,_top,_nearPlane,_farPlane);
    _viewMatrix = glm::lookAt(glm::vec3(0.0f),glm::vec3(0.0f,0.0f,-1.0f),glm::vec3(0.0f,1.0f,0.0f));
    _type = Type::Orthographic;
}
OLD_ComponentCamera::~OLD_ComponentCamera(){}
void OLD_ComponentCamera::update(const float& dt){}
void OLD_ComponentCamera::resize(uint width, uint height){
    if(_type == Type::Perspective){
        _aspectRatio = width/(float)height;
    }
    epriv::OLD_ComponentInternalFunctionality::RebuildProjectionMatrix(*this);
}
uint OLD_ComponentCamera::pointIntersectTest(glm::vec3& position){
    for(ushort i=0; i < 6; ++i){
        float d = _planes[i].x * position.x + _planes[i].y * position.y + _planes[i].z * position.z + _planes[i].w;
        if (d > 0.0f) return 0; //outside
    }
    return 1;//inside
}
uint OLD_ComponentCamera::sphereIntersectTest(glm::vec3& position,float radius){
    uint res = 1; //inside the viewing frustum
    if(radius <= 0.0f) return 0;
    for (ushort i = 0; i < 6; ++i){
        float d = _planes[i].x * position.x + _planes[i].y * position.y + _planes[i].z * position.z + _planes[i].w;
        if (d > radius * 2.0f) return 0; //outside the viewing frustrum
        else if (d > 0.0f) res = 2; //intersecting the viewing plane
    }
    return res;
}
void OLD_ComponentCamera::lookAt(glm::vec3 eye,glm::vec3 center,glm::vec3 up){
    _eye = eye;
    _up = up;
    _viewMatrix              = glm::lookAt(_eye, center,_up);
    _viewMatrixNoTranslation = glm::lookAt(glm::vec3(0.0f), center - _eye,_up);
}
glm::mat4 OLD_ComponentCamera::getProjection(){ return _projectionMatrix; }
glm::mat4 OLD_ComponentCamera::getProjectionInverse(){ return glm::inverse(_projectionMatrix); }
glm::mat4 OLD_ComponentCamera::getView(){ return _viewMatrix; }
glm::mat4 OLD_ComponentCamera::getViewInverse(){ return glm::inverse(_viewMatrix); }
glm::mat4 OLD_ComponentCamera::getViewProjection(){ return _projectionMatrix * _viewMatrix; }
glm::mat4 OLD_ComponentCamera::getViewProjectionInverse(){ return glm::inverse(_projectionMatrix * _viewMatrix); }
glm::vec3 OLD_ComponentCamera::getViewVector(){ return glm::vec3(_viewMatrix[0][2],_viewMatrix[1][2],_viewMatrix[2][2]); }
float OLD_ComponentCamera::getAngle(){ return _angle; }
float OLD_ComponentCamera::getAspect(){ return _aspectRatio; }
float OLD_ComponentCamera::getNear(){ return _nearPlane; }
float OLD_ComponentCamera::getFar(){ return _farPlane; }
void OLD_ComponentCamera::setAngle(float a){ _angle = a; epriv::OLD_ComponentInternalFunctionality::RebuildProjectionMatrix(*this); }
void OLD_ComponentCamera::setAspect(float a){ _aspectRatio = a; epriv::OLD_ComponentInternalFunctionality::RebuildProjectionMatrix(*this); }
void OLD_ComponentCamera::setNear(float n){ _nearPlane = n; epriv::OLD_ComponentInternalFunctionality::RebuildProjectionMatrix(*this); }
void OLD_ComponentCamera::setFar(float f){ _farPlane = f; epriv::OLD_ComponentInternalFunctionality::RebuildProjectionMatrix(*this); }



glm::mat4 epriv::InternalComponentPublicInterface::GetViewNoTranslation(const Camera& c) {
    return c.m_Camera->_viewMatrixNoTranslation;
}
glm::mat4 epriv::InternalComponentPublicInterface::GetViewInverseNoTranslation(const Camera& c) {
    return glm::inverse(c.m_Camera->_viewMatrixNoTranslation);
}
glm::mat4 epriv::InternalComponentPublicInterface::GetViewProjectionNoTranslation(const Camera& c) {
    auto& component = *c.m_Camera;
    return component._projectionMatrix * component._viewMatrixNoTranslation;
}
glm::mat4 epriv::InternalComponentPublicInterface::GetViewProjectionInverseNoTranslation(const Camera& c) {
    auto& component = *c.m_Camera;
    return glm::inverse(component._projectionMatrix * component._viewMatrixNoTranslation);
}
glm::vec3 epriv::InternalComponentPublicInterface::GetViewVectorNoTranslation(const Camera& c) {
    auto& matrix = c.m_Camera->_viewMatrixNoTranslation;
    return glm::vec3(matrix[0][2], matrix[1][2], matrix[2][2]);
}



#pragma endregion

#pragma region Model

OLD_ComponentModel::OLD_ComponentModel(Handle& mesh,Handle& mat, OLD_Entity* _e, ShaderP* _prog, RenderStage::Stage _stage){
    m_Owner = _e->id(); if (!mesh.null()) addModel(mesh, mat, _prog, _stage);
}
OLD_ComponentModel::OLD_ComponentModel(Mesh* mesh,Handle& mat, OLD_Entity* _e, ShaderP* _prog, RenderStage::Stage _stage){
    m_Owner = _e->id(); if(mesh) addModel(mesh, (Material*)mat.get(), _prog, _stage);
}
OLD_ComponentModel::OLD_ComponentModel(Handle& mesh,Material* mat, OLD_Entity* _e, ShaderP* _prog, RenderStage::Stage _stage){
    m_Owner = _e->id(); if(!mesh.null()) addModel((Mesh*)mesh.get(), mat, _prog, _stage);
}
OLD_ComponentModel::OLD_ComponentModel(Mesh* mesh,Material* mat, OLD_Entity* _e, ShaderP* _prog, RenderStage::Stage _stage){
    m_Owner = _e->id(); if(mesh) addModel(mesh, mat, _prog, _stage);
}
OLD_ComponentModel::OLD_ComponentModel(Handle& mesh, Handle& mat, OLD_Entity* _e, Handle& _prog, RenderStage::Stage _stage) {
    m_Owner = _e->id(); if (!mesh.null()) addModel(mesh, mat, (ShaderP*)_prog.get(), _stage);
}
OLD_ComponentModel::OLD_ComponentModel(Mesh* mesh, Handle& mat, OLD_Entity* _e, Handle& _prog, RenderStage::Stage _stage) {
    m_Owner = _e->id(); if (mesh) addModel(mesh, (Material*)mat.get(), (ShaderP*)_prog.get(), _stage);
}
OLD_ComponentModel::OLD_ComponentModel(Handle& mesh, Material* mat, OLD_Entity* _e, Handle& _prog, RenderStage::Stage _stage) {
    m_Owner = _e->id(); if (!mesh.null()) addModel((Mesh*)mesh.get(), mat, (ShaderP*)_prog.get(), _stage);
}
OLD_ComponentModel::OLD_ComponentModel(Mesh* mesh, Material* mat, OLD_Entity* _e, Handle& _prog, RenderStage::Stage _stage) {
    m_Owner = _e->id(); if (mesh) addModel(mesh, mat, (ShaderP*)_prog.get(), _stage);
}
OLD_ComponentModel::~OLD_ComponentModel(){
    SAFE_DELETE_VECTOR(models);
}
uint OLD_ComponentModel::getNumModels() { return models.size(); }
MeshInstance* OLD_ComponentModel::getModel(uint index){ return models[index]; }
void OLD_ComponentModel::show() { for (auto model : models) model->show(); }
void OLD_ComponentModel::hide() { for (auto model : models) model->hide(); }
float OLD_ComponentModel::radius(){ return _radius; }
glm::vec3 OLD_ComponentModel::boundingBox() { return _radiusBox; }
uint OLD_ComponentModel::addModel(Handle& mesh, Handle& mat, ShaderP* shaderProgram, RenderStage::Stage _stage){ return OLD_ComponentModel::addModel((Mesh*)mesh.get(),(Material*)mat.get(), shaderProgram, _stage); }
uint OLD_ComponentModel::addModel(Mesh* mesh, Material* material, ShaderP* shaderProgram, RenderStage::Stage _stage) {
    MeshInstance* instance = new MeshInstance(*owner(), mesh, material, shaderProgram);
    models.push_back(instance);  
    if (m_Owner != 0) {
        auto* _scene = owner()->scene();
        if (_scene) {
            instance->setStage(_stage);
            epriv::InternalScenePublicInterface::AddMeshInstanceToPipeline(*_scene, *instance, _stage);
        }
    }
    epriv::OLD_ComponentInternalFunctionality::CalculateRadius(*this);
    return models.size() - 1;
}
void OLD_ComponentModel::setModel(Handle& mesh,Handle& mat,uint index, ShaderP* shaderProgram, RenderStage::Stage _stage){ OLD_ComponentModel::setModel((Mesh*)mesh.get(),(Material*)mat.get(), index, shaderProgram, _stage); }
void OLD_ComponentModel::setModel(Mesh* mesh,Material* material,uint index, ShaderP* shaderProgram, RenderStage::Stage _stage){
    auto& instance = *models[index];
    if (m_Owner != 0) {
        auto* _scene = owner()->scene();
        if (_scene) {
            epriv::InternalScenePublicInterface::RemoveMeshInstanceFromPipeline(*_scene, instance, instance.stage());
            instance.setShaderProgram(shaderProgram);
            instance.setMesh(mesh);
            instance.setMaterial(material);
            instance.setStage(_stage);
            epriv::InternalScenePublicInterface::AddMeshInstanceToPipeline(*_scene, instance, _stage);
        }
    }else{
        instance.setShaderProgram(shaderProgram);
        instance.setMesh(mesh);
        instance.setMaterial(material);
        instance.setStage(_stage);
    }
    epriv::OLD_ComponentInternalFunctionality::CalculateRadius(*this);
}
void OLD_ComponentModel::setModelShaderProgram(ShaderP* shaderProgram, uint index, RenderStage::Stage _stage) {
    auto& instance = *models[index];
    if (m_Owner != 0) {
        auto* _scene = owner()->scene();
        if (_scene) {
            epriv::InternalScenePublicInterface::RemoveMeshInstanceFromPipeline(*_scene, instance, instance.stage());
            instance.setShaderProgram(shaderProgram);
            instance.setStage(_stage);
            epriv::InternalScenePublicInterface::AddMeshInstanceToPipeline(*_scene, instance, _stage);
        }
    }else{
        instance.setShaderProgram(shaderProgram);
        instance.setStage(_stage);
    }
    epriv::OLD_ComponentInternalFunctionality::CalculateRadius(*this);
}
void OLD_ComponentModel::setModelShaderProgram(Handle& shaderPHandle, uint index, RenderStage::Stage _stage) { OLD_ComponentModel::setModelShaderProgram((ShaderP*)shaderPHandle.get(),index, _stage); }
void OLD_ComponentModel::setModelMesh(Mesh* mesh,uint index, RenderStage::Stage _stage){
    auto& instance = *models[index];
    if (m_Owner != 0) {
        auto* _scene = owner()->scene();
        if (_scene) {
            epriv::InternalScenePublicInterface::RemoveMeshInstanceFromPipeline(*_scene, instance, instance.stage());
            instance.setMesh(mesh);
            instance.setStage(_stage);
            epriv::InternalScenePublicInterface::AddMeshInstanceToPipeline(*_scene, instance, _stage);
        }
    }else{
        instance.setMesh(mesh);
        instance.setStage(_stage);
    }
    epriv::OLD_ComponentInternalFunctionality::CalculateRadius(*this);
}
void OLD_ComponentModel::setModelMesh(Handle& mesh, uint index, RenderStage::Stage _stage){ OLD_ComponentModel::setModelMesh((Mesh*)mesh.get(),index, _stage); }
void OLD_ComponentModel::setModelMaterial(Material* material,uint index, RenderStage::Stage _stage){
    auto& instance = *models[index];
    if (m_Owner != 0) {
        auto* _scene = owner()->scene();
        if (_scene) {
            epriv::InternalScenePublicInterface::RemoveMeshInstanceFromPipeline(*_scene, instance, instance.stage());
            instance.setMaterial(material);
            instance.setStage(_stage);
            epriv::InternalScenePublicInterface::AddMeshInstanceToPipeline(*_scene, instance, _stage);
        }
    }else{
        instance.setMaterial(material);
        instance.setStage(_stage);
    }
}
void OLD_ComponentModel::setModelMaterial(Handle& mat,uint index, RenderStage::Stage _stage){ OLD_ComponentModel::setModelMaterial((Material*)mat.get(),index, _stage); }
bool OLD_ComponentModel::rayIntersectSphere(OLD_ComponentCamera& camera){
    auto& body = *owner()->getComponent<OLD_ComponentBody>();
    return Math::rayIntersectSphere(body.position(),_radius,camera._eye,camera.getViewVector());
}

#pragma endregion

#pragma region Body

OLD_ComponentBody::OLD_ComponentBody(){
    data.n = new NormalData();
    _physics = false;
    auto& normalData = *data.n;
    normalData.position =  glm::vec3(0.0f);
    normalData.scale =  glm::vec3(1.0f);
    normalData.rotation =  glm::quat(1.0f,0.0f,0.0f,0.0f);
    normalData.modelMatrix =  glm::mat4(1.0f);
    Math::recalculateForwardRightUp(normalData.rotation,_forward,_right,_up);
}
OLD_ComponentBody::OLD_ComponentBody(CollisionType::Type _collisionType){
    data.p = new PhysicsData();	
    _physics = true;
    auto& physicsData = *data.p;
    _forward = glm::vec3(0,0,-1);  _right = glm::vec3(1,0,0);  _up = glm::vec3(0,1,0);

    physicsData.motionState = btDefaultMotionState(btTransform(btQuaternion(0,0,0,1)));
    float _mass = 1.0f;
    physicsData.mass = _mass;

    setCollision(_collisionType, _mass);

    setMass(_mass);
    btDefaultMotionState* _motionState = &physicsData.motionState;
    btCollisionShape* _shape = physicsData.collision->getShape();
    const btVector3& _inertia = physicsData.collision->getInertia();

    btRigidBody::btRigidBodyConstructionInfo CI(_mass,_motionState, _shape,_inertia);
    physicsData.rigidBody = btRigidBody(CI);
    auto& rigidBody = physicsData.rigidBody;
    rigidBody.setSleepingThresholds(0.015f,0.015f);
    rigidBody.setFriction(0.3f);
    rigidBody.setDamping(0.1f,0.4f);//air friction 
    rigidBody.setMassProps(_mass,_inertia);
    rigidBody.updateInertiaTensor();
    rigidBody.setUserPointer(this);
}
OLD_ComponentBody::~OLD_ComponentBody(){
    if(_physics){
        auto& physicsData = *data.p;
        Physics::removeRigidBody(physicsData.rigidBody);
        //SAFE_DELETE(physicsData.rigidBody);
        SAFE_DELETE(physicsData.collision);
        //SAFE_DELETE(physicsData.motionState);
        SAFE_DELETE(data.p);
    }else{
        auto& normalData = *data.n;
        //SAFE_DELETE(normalData.position);
        //SAFE_DELETE(normalData.scale);
        //SAFE_DELETE(normalData.rotation);
        //SAFE_DELETE(normalData.modelMatrix);
        SAFE_DELETE(data.n);
    }
}
void OLD_ComponentBody::alignTo(glm::vec3 direction,float speed){
    if(_physics){
        auto& physicsData = *data.p;
        //recheck this
        btTransform tr;
        physicsData.rigidBody.getMotionState()->getWorldTransform(tr);
        //Math::alignTo(Math::btToGLMQuat(tr.getRotation()),direction,speed);
        Math::recalculateForwardRightUp(physicsData.rigidBody,_forward,_right,_up);
    }else{
        auto& normalData = *data.n;
        Math::alignTo(normalData.rotation,direction,speed);
        Math::recalculateForwardRightUp(normalData.rotation,_forward,_right,_up);
    }
}
void OLD_ComponentBody::setCollision(CollisionType::Type _type,float _mass){
    auto& physicsData = *data.p;
    SAFE_DELETE(physicsData.collision);
    OLD_ComponentModel* modelComponent = nullptr;
    if (m_Owner != 0) {
        modelComponent = owner()->getComponent<OLD_ComponentModel>();
    }
    if (modelComponent) {
        if (_type == CollisionType::Compound) {
            physicsData.collision = new Collision(modelComponent, _mass);
        }else {
            physicsData.collision = new Collision(_type, modelComponent->getModel()->mesh(), _mass);
        }
    }else{
        physicsData.collision = new Collision(_type, nullptr, _mass);
    }
    physicsData.collision->getShape()->setUserPointer(this);
    Physics::removeRigidBody(physicsData.rigidBody);
    physicsData.rigidBody.setCollisionShape(physicsData.collision->getShape());
    physicsData.rigidBody.setMassProps(physicsData.mass, physicsData.collision->getInertia());
    physicsData.rigidBody.updateInertiaTensor();
    Physics::addRigidBody(physicsData.rigidBody);
}
void OLD_ComponentBody::translate(glm::vec3 translation,bool local){ OLD_ComponentBody::translate(translation.x,translation.y,translation.z,local); }
void OLD_ComponentBody::translate(float x,float y,float z,bool local){
    if(_physics){
        auto& physicsData = *data.p;
        physicsData.rigidBody.activate();
        btVector3 v(x,y,z);
        Math::translate(physicsData.rigidBody,v,local);
        setPosition(  position() + Engine::Math::btVectorToGLM(v)  );
    }else{
        auto& normalData = *data.n;
        glm::vec3& _position = normalData.position;
        _position.x += x; _position.y += y; _position.z += z;
        glm::vec3 offset(x,y,z);
        if(local){
            offset = (normalData.rotation) * offset;
        }
        setPosition(_position + offset);
    }
}
void OLD_ComponentBody::rotate(glm::vec3 rotation,bool local){ OLD_ComponentBody::rotate(rotation.x,rotation.y,rotation.z,local); }
void OLD_ComponentBody::rotate(float pitch,float yaw,float roll,bool local){
    if(_physics){
        auto& physicsData = *data.p;
        btQuaternion quat = physicsData.rigidBody.getWorldTransform().getRotation().normalize();
        glm::quat glmquat(quat.w(),quat.x(),quat.y(),quat.z());

        if(abs(pitch) >= 0.001f) glmquat = glmquat * (glm::angleAxis(-pitch, glm::vec3(1,0,0)));
        if(abs(yaw) >= 0.001f)   glmquat = glmquat * (glm::angleAxis(-yaw,   glm::vec3(0,1,0)));
        if(abs(roll) >= 0.001f)  glmquat = glmquat * (glm::angleAxis(roll,   glm::vec3(0,0,1)));

        quat = btQuaternion(glmquat.x,glmquat.y,glmquat.z,glmquat.w);
        physicsData.rigidBody.getWorldTransform().setRotation(quat);
    }else{
        auto& normalData = *data.n;
        glm::quat& _rotation = normalData.rotation;
        if (abs(pitch) >= 0.001f) _rotation = _rotation * (glm::angleAxis(-pitch, glm::vec3(1, 0, 0)));
        if (abs(yaw) >= 0.001f)   _rotation = _rotation * (glm::angleAxis(-yaw, glm::vec3(0, 1, 0)));
        if (abs(roll) >= 0.001f)  _rotation = _rotation * (glm::angleAxis(roll, glm::vec3(0, 0, 1)));
        Math::recalculateForwardRightUp(_rotation,_forward,_right,_up);
    }
}
void OLD_ComponentBody::scale(glm::vec3 amount){ OLD_ComponentBody::scale(amount.x,amount.y,amount.z); }
void OLD_ComponentBody::scale(float x,float y,float z){
    if(_physics){
        auto& physicsData = *data.p;
        if(physicsData.collision){
            if(physicsData.collision->getType() == CollisionType::Compound){
                btCompoundShape* cast = ((btCompoundShape*)(physicsData.collision->getShape()));
                for(int i = 0; i < cast->getNumChildShapes(); ++i){
                    btCollisionShape* shape = cast->getChildShape(i);
                    btUniformScalingShape* convexHull = dynamic_cast<btUniformScalingShape*>(shape);
                    if(convexHull){
                        convexHull->setLocalScaling(convexHull->getLocalScaling() + btVector3(x,y,z));
                        continue;
                    }
                    btScaledBvhTriangleMeshShape* triHull = dynamic_cast<btScaledBvhTriangleMeshShape*>(shape);
                    if(triHull){
                        triHull->setLocalScaling(triHull->getLocalScaling() + btVector3(x,y,z));
                    }
                }
            }
        }
    }else{
        auto& normalData = *data.n;
        glm::vec3& _scale = normalData.scale;
        _scale.x += x; _scale.y += y; _scale.z += z;
    }
    if(m_Owner != 0){
        auto* models = owner()->getComponent<OLD_ComponentModel>();
        if(models){
            epriv::OLD_ComponentInternalFunctionality::CalculateRadius(*models);
        }
    }
}
void OLD_ComponentBody::setPosition(glm::vec3 newPosition){ OLD_ComponentBody::setPosition(newPosition.x,newPosition.y,newPosition.z); }
void OLD_ComponentBody::setPosition(float x,float y,float z){
    if(_physics){
        auto& physicsData = *data.p;
        btTransform tr;
        tr.setOrigin(btVector3(x,y,z));
        tr.setRotation(physicsData.rigidBody.getOrientation());
        if(physicsData.collision->getType() == CollisionType::TriangleShapeStatic){
            Physics::removeRigidBody(physicsData.rigidBody);
            //SAFE_DELETE(physicsData.rigidBody);
        }
        physicsData.motionState.setWorldTransform(tr);
        if(physicsData.collision->getType() == CollisionType::TriangleShapeStatic){
            btRigidBody::btRigidBodyConstructionInfo ci(physicsData.mass, &physicsData.motionState, physicsData.collision->getShape(), physicsData.collision->getInertia());
            physicsData.rigidBody = btRigidBody(ci);
            physicsData.rigidBody.setUserPointer(this);
            Physics::addRigidBody(physicsData.rigidBody);
        }
        physicsData.rigidBody.setMotionState(&physicsData.motionState); //is this needed?
        physicsData.rigidBody.setWorldTransform(tr);
        physicsData.rigidBody.setCenterOfMassTransform(tr);
    }else{
        auto& normalData = *data.n;
        glm::vec3& _position = normalData.position;
        glm::mat4& _matrix = normalData.modelMatrix;
        _position.x = x; _position.y = y; _position.z = z;
        _matrix[3][0] = x;
        _matrix[3][1] = y;
        _matrix[3][2] = z;
    }
}
void OLD_ComponentBody::setRotation(glm::quat newRotation){ OLD_ComponentBody::setRotation(newRotation.x,newRotation.y,newRotation.z,newRotation.w); }
void OLD_ComponentBody::setRotation(float x,float y,float z,float w){
    if(_physics){
        auto& physicsData = *data.p;
        btQuaternion quat(x,y,z,w);
        quat = quat.normalize();

        btTransform tr; tr.setOrigin(physicsData.rigidBody.getWorldTransform().getOrigin());
        tr.setRotation(quat);

        physicsData.rigidBody.setWorldTransform(tr);
        physicsData.rigidBody.setCenterOfMassTransform(tr);
        physicsData.motionState.setWorldTransform(tr);

        Math::recalculateForwardRightUp(physicsData.rigidBody,_forward,_right,_up);

        clearAngularForces();
    }else{
        auto& normalData = *data.n;
        glm::quat newRotation(w,x,y,z);
        newRotation = glm::normalize(newRotation);
        glm::quat& _rotation = normalData.rotation;
        _rotation = newRotation;
        Math::recalculateForwardRightUp(_rotation,_forward,_right,_up);
    }
}
void OLD_ComponentBody::setScale(glm::vec3 newScale){ OLD_ComponentBody::setScale(newScale.x,newScale.y,newScale.z); }
void OLD_ComponentBody::setScale(float x,float y,float z){
    if(_physics){
        auto& physicsData = *data.p;
        if(physicsData.collision){
            if (physicsData.collision->getType() == CollisionType::Compound) {
                btCompoundShape* compoundShapeCast = dynamic_cast<btCompoundShape*>(physicsData.collision->getShape());
                if (compoundShapeCast) {
                    int numChildren = compoundShapeCast->getNumChildShapes();
                    if (numChildren > 0) {
                        for (int i = 0; i < numChildren; ++i) {
                            btCollisionShape* shape = compoundShapeCast->getChildShape(i);
                            btUniformScalingShape* convexHullCast = dynamic_cast<btUniformScalingShape*>(shape);
                            if (convexHullCast) {
                                convexHullCast->setLocalScaling(btVector3(x, y, z));
                                continue;
                            }
                            btScaledBvhTriangleMeshShape* triHullCast = dynamic_cast<btScaledBvhTriangleMeshShape*>(shape);
                            if (triHullCast) {
                                triHullCast->setLocalScaling(btVector3(x, y, z));
                            }
                        }
                    }
                }
            }
        }
    }else{
        auto& normalData = *data.n;
        glm::vec3& _scale = normalData.scale;
        _scale.x = x; _scale.y = y; _scale.z = z;
    }
    if(m_Owner != 0){
        auto* models = owner()->getComponent<OLD_ComponentModel>();
        if(models){
            epriv::OLD_ComponentInternalFunctionality::CalculateRadius(*models);
        }
    }
}  
btRigidBody& OLD_ComponentBody::getBody(){ return data.p->rigidBody; }
glm::vec3 OLD_ComponentBody::position(){ //theres prob a better way to do this
    if(_physics){
        glm::mat4 m(1.0f);
        btTransform tr;  data.p->rigidBody.getMotionState()->getWorldTransform(tr);
        tr.getOpenGLMatrix(glm::value_ptr(m));
        return glm::vec3(m[3][0],m[3][1],m[3][2]);
    }
    glm::mat4& _matrix = data.n->modelMatrix;
    return glm::vec3(_matrix[3][0],_matrix[3][1],_matrix[3][2]);
}
glm::vec3 OLD_ComponentBody::getScreenCoordinates(){ return Math::getScreenCoordinates(position(),false); }
glm::vec3 OLD_ComponentBody::getScale(){
    if(_physics){
        auto& physicsData = *data.p;
        if(physicsData.collision){
            if(physicsData.collision->getType() == CollisionType::Compound){
                btCompoundShape* compoundShapeCast = dynamic_cast<btCompoundShape*>(physicsData.collision->getShape());
                if (compoundShapeCast) {
                    int numChildren = compoundShapeCast->getNumChildShapes();
                    if (numChildren > 0) {
                        for (int i = 0; i < numChildren; ++i) {
                            btCollisionShape* shape = compoundShapeCast->getChildShape(i);
                            btUniformScalingShape* convexHullCast = dynamic_cast<btUniformScalingShape*>(shape);
                            if (convexHullCast) {
                                return Math::btVectorToGLM(const_cast<btVector3&>(convexHullCast->getLocalScaling()));
                            }
                            btScaledBvhTriangleMeshShape* triHullCast = dynamic_cast<btScaledBvhTriangleMeshShape*>(shape);
                            if (triHullCast) {
                                return Math::btVectorToGLM(const_cast<btVector3&>(triHullCast->getLocalScaling()));
                            }
                        }
                    }
                }
            }
        }
        return glm::vec3(1.0f);
    }
    return data.n->scale; 
}
glm::quat OLD_ComponentBody::rotation(){
    if(_physics){
        return Engine::Math::btToGLMQuat(data.p->rigidBody.getWorldTransform().getRotation());
    }
    return data.n->rotation;
}
glm::vec3 OLD_ComponentBody::forward(){ return _forward; }
glm::vec3 OLD_ComponentBody::right(){ return _right; }
glm::vec3 OLD_ComponentBody::up(){ return _up; }
glm::vec3 OLD_ComponentBody::getLinearVelocity(){ btVector3 v = data.p->rigidBody.getLinearVelocity(); return Engine::Math::btVectorToGLM(v); }
glm::vec3 OLD_ComponentBody::getAngularVelocity(){ btVector3 v = data.p->rigidBody.getAngularVelocity(); return Engine::Math::btVectorToGLM(v); }
float OLD_ComponentBody::mass(){ return data.p->mass; }
glm::mat4 OLD_ComponentBody::modelMatrix(){ //theres prob a better way to do this
    if(_physics){
        auto& physicsData = *data.p;
        glm::mat4 m(1.0f);
        btTransform tr;  physicsData.rigidBody.getMotionState()->getWorldTransform(tr);
        tr.getOpenGLMatrix(glm::value_ptr(m));
        if(physicsData.collision){
            m = glm::scale(m,getScale());
        }
        return m;
    }
    return data.n->modelMatrix;
}
void OLD_ComponentBody::setDamping(float linear,float angular){ data.p->rigidBody.setDamping(linear,angular); }
void OLD_ComponentBody::setDynamic(bool dynamic){
    if (_physics) {
        auto& physicsData = *data.p;
        if (dynamic) {
            Physics::removeRigidBody(physicsData.rigidBody);
            physicsData.rigidBody.setCollisionFlags(btCollisionObject::CF_ANISOTROPIC_FRICTION_DISABLED);
            Physics::addRigidBody(physicsData.rigidBody);
            physicsData.rigidBody.activate();
        }else {
            Physics::removeRigidBody(physicsData.rigidBody);
            physicsData.rigidBody.setCollisionFlags(btCollisionObject::CF_STATIC_OBJECT);
            OLD_ComponentBody::clearAllForces();
            Physics::addRigidBody(physicsData.rigidBody);
            physicsData.rigidBody.activate();
        }
    }
}
void OLD_ComponentBody::setLinearVelocity(float x,float y,float z,bool local){
    if (_physics) {
        auto& physicsData = *data.p;
        physicsData.rigidBody.activate();
        btVector3 v(x, y, z);
        Math::translate(physicsData.rigidBody, v, local);
        physicsData.rigidBody.setLinearVelocity(v);
    }
}
void OLD_ComponentBody::setLinearVelocity(glm::vec3 velocity,bool local){ OLD_ComponentBody::setLinearVelocity(velocity.x,velocity.y,velocity.z,local); }
void OLD_ComponentBody::setAngularVelocity(float x,float y,float z,bool local){
    if (_physics) {
        auto& physicsData = *data.p;
        physicsData.rigidBody.activate();
        btVector3 v(x, y, z);
        Math::translate(physicsData.rigidBody, v, local);
        physicsData.rigidBody.setAngularVelocity(v);
    }
}
void OLD_ComponentBody::setAngularVelocity(glm::vec3 velocity,bool local){ OLD_ComponentBody::setAngularVelocity(velocity.x,velocity.y,velocity.z,local); }
void OLD_ComponentBody::applyForce(float x,float y,float z,bool local){
    if (_physics) {
        auto& physicsData = *data.p;
        physicsData.rigidBody.activate();
        btVector3 v(x, y, z);
        Math::translate(physicsData.rigidBody, v, local);
        physicsData.rigidBody.applyCentralForce(v);
    }
}
void OLD_ComponentBody::applyForce(glm::vec3 force,glm::vec3 origin,bool local){
    if (_physics) {
        auto& physicsData = *data.p;
        physicsData.rigidBody.activate();
        btVector3 v(force.x, force.y, force.z);
        if (local) {
            btQuaternion q = physicsData.rigidBody.getWorldTransform().getRotation().normalize();
            v = v.rotate(q.getAxis(), q.getAngle());
        }
        physicsData.rigidBody.applyForce(v, btVector3(origin.x, origin.y, origin.z));
    }
}
void OLD_ComponentBody::applyImpulse(float x,float y,float z,bool local){
    if (_physics) {
        auto& physicsData = *data.p;
        physicsData.rigidBody.activate();
        btVector3 v(x, y, z);
        Math::translate(physicsData.rigidBody, v, local);
        physicsData.rigidBody.applyCentralImpulse(v);
    }
}
void OLD_ComponentBody::applyImpulse(glm::vec3 impulse,glm::vec3 origin,bool local){
    if (_physics) {
        auto& physicsData = *data.p;
        physicsData.rigidBody.activate();
        btVector3 v(impulse.x, impulse.y, impulse.z);
        if (local) {
            btQuaternion q = physicsData.rigidBody.getWorldTransform().getRotation().normalize();
            v = v.rotate(q.getAxis(), q.getAngle());
        }
        physicsData.rigidBody.applyImpulse(v, btVector3(origin.x, origin.y, origin.z));
    }
}
void OLD_ComponentBody::applyTorque(float x,float y,float z,bool local){
    if (_physics) {
        auto& physicsData = *data.p;
        physicsData.rigidBody.activate();
        btVector3 t(x, y, z);
        if (local) {
            t = physicsData.rigidBody.getInvInertiaTensorWorld().inverse() * (physicsData.rigidBody.getWorldTransform().getBasis() * t);
        }
        physicsData.rigidBody.applyTorque(t);
    }
}
void OLD_ComponentBody::applyTorque(glm::vec3 torque,bool local){ OLD_ComponentBody::applyTorque(torque.x,torque.y,torque.z,local); }
void OLD_ComponentBody::applyTorqueImpulse(float x,float y,float z,bool local){
    if (_physics) {
        auto& physicsData = *data.p;
        physicsData.rigidBody.activate();
        btVector3 t(x, y, z);
        if (local) {
            t = physicsData.rigidBody.getInvInertiaTensorWorld().inverse() * (physicsData.rigidBody.getWorldTransform().getBasis() * t);
        }
        physicsData.rigidBody.applyTorqueImpulse(t);
    }
}
void OLD_ComponentBody::applyTorqueImpulse(glm::vec3 torqueImpulse,bool local){ OLD_ComponentBody::applyTorqueImpulse(torqueImpulse.x,torqueImpulse.y,torqueImpulse.z,local); }
void OLD_ComponentBody::clearLinearForces(){
    if (_physics) {
        auto& physicsData = *data.p;
        physicsData.rigidBody.setActivationState(0);
        physicsData.rigidBody.activate();
        physicsData.rigidBody.setLinearVelocity(btVector3(0, 0, 0));
    }
}
void OLD_ComponentBody::clearAngularForces(){
    if (_physics) {
        auto& physicsData = *data.p;
        physicsData.rigidBody.setActivationState(0);
        physicsData.rigidBody.activate();
        physicsData.rigidBody.setAngularVelocity(btVector3(0, 0, 0));
    }
}
void OLD_ComponentBody::clearAllForces(){
    if (_physics) {
        auto& physicsData = *data.p;
        btVector3 v(0, 0, 0);
        physicsData.rigidBody.setActivationState(0);
        physicsData.rigidBody.activate();
        physicsData.rigidBody.setLinearVelocity(v);
        physicsData.rigidBody.setAngularVelocity(v);
    }
}
void OLD_ComponentBody::setMass(float mass){
    if (_physics) {
        auto& physicsData = *data.p;
        physicsData.mass = mass;
        if (physicsData.collision) {
            physicsData.collision->setMass(physicsData.mass);
            //if (physicsData.rigidBody) {
                physicsData.rigidBody.setMassProps(physicsData.mass, physicsData.collision->getInertia());
            //}
        }
    }
}
#pragma endregion



#pragma region Entity

OLD_Entity::OLD_Entity(){
    m_Scene = 0;
    ID = 0;
    m_Components.resize(OLD_ComponentType::_TOTAL, 0);
}
OLD_Entity::~OLD_Entity(){
    ID = 0;
    m_Scene = 0;
}
const uint OLD_Entity::id() const { return ID; }
Scene* OLD_Entity::scene(){ return m_Scene; }
void OLD_Entity::destroy(){
    componentManager->_addEntityToBeDestroyed(ID);  //add to the deletion queue
}

#pragma endregion

OLD_ComponentBaseClass* OLD_Components::GetComponent(uint index) { return componentManager->m_ComponentPool->getAsFast<OLD_ComponentBaseClass>(index); }
OLD_Entity* OLD_Components::GetEntity(uint id) { return componentManager->m_EntityPool->getAsFast<OLD_Entity>(id); }