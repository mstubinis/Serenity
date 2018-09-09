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
#include "Engine_Time.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_access.hpp>

#include "Engine_Physics.h"
#include <bullet/btBulletCollisionCommon.h>
#include <bullet/btBulletDynamicsCommon.h>

using namespace Engine;
using namespace std;

boost::unordered_map<boost_type_index,uint>               epriv::ComponentTypeRegistry::m_MapComponentTypesToSlot;
boost::unordered_map<uint, vector<ComponentBaseClass*>>   epriv::ComponentManager::m_ComponentVectors;
boost::unordered_map<uint, vector<ComponentBaseClass*>>   epriv::ComponentManager::m_ComponentVectorsScene;
epriv::ObjectPool<ComponentBaseClass>*                    epriv::ComponentManager::m_ComponentPool;

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
            for(auto meshInstance:super->models){
                auto& pair = *meshInstance;
                glm::mat4& m = pair.model();
                glm::vec3 localPosition = glm::vec3(m[3][0],m[3][1],m[3][2]);
                float length = glm::length(localPosition) + pair.mesh()->getRadius() * Engine::Math::Max(pair.getScale());
                if(length > maxLength){
                    maxLength = length;
                }
            }
            super->_radius = maxLength;
            if(super->m_Owner){
                auto* body = super->m_Owner->getComponent<ComponentBody>();
                if(body){
                    super->_radius *= Engine::Math::Max(body->getScale());
                }
            }
            return super->_radius;
        }
};
class epriv::ComponentCameraSystem::impl final{
    public:

        void _init(){
        }
        void _destruct(){

        }
        static void _defaultUpdateCameraComponent(vector<ComponentBaseClass*>& vec,const float& dt){
            for(auto c:vec){
                auto& cmpnt = *(ComponentCamera*)c;
                cmpnt.update(dt);//custom camera user code
                Math::extractViewFrustumPlanesHartmannGribbs(cmpnt._projectionMatrix * cmpnt._viewMatrix,cmpnt._planes);//update view frustrum 
            }
        }
        void _update(const float& dt){
            uint slot = componentManager->getIndividualComponentTypeSlot<ComponentCamera>();
            auto& v = ComponentManager::m_ComponentVectorsScene.at(slot);
            auto split = epriv::threading::splitVector(v);
            for(auto vec:split){
                epriv::threading::addJob(_defaultUpdateCameraComponent,vec,dt);
            }
            epriv::threading::waitForAll();

            //command queue here
        }
};
epriv::ComponentCameraSystem::ComponentCameraSystem():m_i(new impl){ m_i->_init(); }
epriv::ComponentCameraSystem::~ComponentCameraSystem(){ m_i->_destruct(); }
void epriv::ComponentCameraSystem::update(const float& dt){ m_i->_update(dt); }

class epriv::ComponentManager::impl final{
    public:	
        ComponentTypeRegistry       m_TypeRegistry;
        vector<Entity*>             m_EntitiesToBeDestroyed;
        bool                        m_Paused;
        ComponentCameraSystem*      m_ComponentCameraSystem;
        void _init(const char* name, uint& w, uint& h,epriv::ComponentManager* super){
            super->m_ComponentPool = new ObjectPool<ComponentBaseClass>(epriv::MAX_NUM_ENTITIES * ComponentType::_TOTAL);
            super->m_EntityPool    = new ObjectPool<Entity>(epriv::MAX_NUM_ENTITIES);

            m_ComponentCameraSystem = new ComponentCameraSystem();

            m_Paused = false;

            m_TypeRegistry.emplace<ComponentBody>();
            m_TypeRegistry.emplace<ComponentModel>();
            m_TypeRegistry.emplace<ComponentCamera>();
        }
        void _postInit(const char* name, uint& w, uint& h){
        }
        void _destruct(epriv::ComponentManager* super){
            SAFE_DELETE(super->m_ComponentPool);
            SAFE_DELETE(super->m_EntityPool);

            SAFE_DELETE(m_ComponentCameraSystem);
        }
        void _performTransformation(Entity* parent,glm::vec3& position,glm::quat& rotation,glm::vec3& scale,glm::mat4& modelMatrix){
            if(!parent){
                modelMatrix = glm::mat4(1.0f);
            }
            else{
                auto* body = parent->getComponent<ComponentBody>();
                modelMatrix = body->modelMatrix();
            }
            glm::mat4 translationMat = glm::translate(position);
            glm::mat4 rotationMat = glm::mat4_cast(rotation);
            glm::mat4 scaleMat = glm::scale(scale);
            modelMatrix = translationMat * rotationMat * scaleMat * modelMatrix;
        }
        static void _updateBodiesJob(vector<ComponentBaseClass*>& vec){
            for(uint j = 0; j < vec.size(); ++j){
                auto& b = *(ComponentBody*)vec.at(j);
                if(b._physics){
                    Engine::Math::recalculateForwardRightUp(b.data.p.rigidBody,b._forward,b._right,b._up);
                }
                else{
                    componentManager->m_i->_performTransformation(b.m_Owner->parent(),*b.data.n.position,*b.data.n.rotation,*b.data.n.scale,*b.data.n.modelMatrix);
                }
            }
        }
        void _updateComponentBodies(const float& dt){
            //basic bodies
            uint slot = componentManager->getIndividualComponentTypeSlot<ComponentBody>();
            vector<ComponentBaseClass*>& v = ComponentManager::m_ComponentVectorsScene.at(slot);
            vector<vector<ComponentBaseClass*>> split = epriv::threading::splitVector(v);

            for(auto vec:split){
                epriv::threading::addJob(_updateBodiesJob,vec);
            }
            epriv::threading::waitForAll();
        }
        void _calculateRenderCheck(ComponentModel& m,Camera* camera){
            auto& body = *(m.m_Owner->getComponent<ComponentBody>());
            auto pos = body.position();
            uint sphereTest = camera->sphereIntersectTest(pos,m._radius);
            if(!m.visible() || sphereTest == 0 || camera->getDistance(pos) > m._radius * 1100.0f){ //1100 is the visibility threshold
                m.m_i->m_PassedRenderCheck = false;
                return;
            }
            m.m_i->m_PassedRenderCheck = true;
        }
        static void _updateModelComponentsJob(vector<ComponentBaseClass*>& vec,Camera* camera){
            for(uint j = 0; j < vec.size(); ++j){
                auto& model = *(ComponentModel*)vec.at(j);
                for(uint i = 0; i < model.models.size(); ++i){
                    auto& pair = *model.models.at(i);
                    if(pair.mesh()){
                        componentManager->m_i->_performTransformation(model.m_Owner->parent(),pair.position(),pair.orientation(),pair.getScale(),pair.model());
                        componentManager->m_i->_calculateRenderCheck(model,camera);
                    }
                }
            }
        }
        void _updateComponentModels(const float& dt){
            auto* camera = Resources::getCurrentScene()->getActiveCamera();
            uint slot = componentManager->getIndividualComponentTypeSlot<ComponentModel>();
            auto& v = ComponentManager::m_ComponentVectorsScene.at(slot);
            auto split = epriv::threading::splitVector(v);
            for(auto vec:split){
                epriv::threading::addJob(_updateModelComponentsJob,vec,camera);
            }
            epriv::threading::waitForAll();
        }
        void _updateCurrentScene(const float& dt){
            auto* currentScene = Resources::getCurrentScene();
            for(auto entityID:currentScene->m_Entities){
                Entity* e = componentManager->_getEntity(entityID);
                //if(e){//should not need this...
                    e->update(dt);
                //}
            }
            if(currentScene->m_Skybox) currentScene->m_Skybox->update();
        }
        void _destroyQueuedEntities(epriv::ComponentManager* super){
            for(auto e:m_EntitiesToBeDestroyed){
                super->_deleteEntityImmediately(e);
            }
            vector_clear(m_EntitiesToBeDestroyed);
        }
        void _updatePhysicsEngine(const float& dt){
            epriv::Core::m_Engine->m_TimeManager->stop_clock();

            //It's important that timeStep is always less than maxSubSteps * fixedTimeStep, otherwise you are losing time.
            //dt < maxSubSteps * fixedTimeStep
            float minStep = 0.0166666f; // == 0.0166666 at 1 fps
            uint maxSubSteps = 0;
            while(true){
                ++maxSubSteps; if(dt < (maxSubSteps * minStep)) break;
            }
            epriv::Core::m_Engine->m_PhysicsManager->_update(dt,maxSubSteps,minStep);
            epriv::Core::m_Engine->m_TimeManager->calculate_physics();
        }
        void _update(const float& dt,epriv::ComponentManager* super){
            _updateCurrentScene(dt); //take player input and perform player actions
            _updatePhysicsEngine(dt);
            if(!m_Paused){
                _updateComponentBodies(dt);
            }
            m_ComponentCameraSystem->update(dt); //update frustum planes
            if(!m_Paused){
                _updateComponentModels(dt); //transform model matrices and perform render check
            }
            _destroyQueuedEntities(super);
        }
};
epriv::ComponentTypeRegistry::ComponentTypeRegistry(){
    m_NextIndex = 0;
}
epriv::ComponentTypeRegistry::~ComponentTypeRegistry(){
    m_MapComponentTypesToSlot.clear();
    m_NextIndex = 0;
}


epriv::ComponentManager::ComponentManager(const char* name, uint w, uint h):m_i(new impl){ m_i->_init(name,w,h,this); componentManager = this; }
epriv::ComponentManager::~ComponentManager(){ m_i->_destruct(this); }

void epriv::ComponentManager::_init(const char* name, uint w, uint h){ m_i->_postInit(name,w,h); }

void epriv::ComponentManager::_pause(bool b){ m_i->m_Paused = b; }
void epriv::ComponentManager::_unpause(){ m_i->m_Paused = false; }
void epriv::ComponentManager::_update(const float& dt){ m_i->_update(dt,this); }
void epriv::ComponentManager::_resize(uint width,uint height){
    uint slot = componentManager->getIndividualComponentTypeSlot<ComponentCamera>();
    for(auto camera:ComponentManager::m_ComponentVectors.at(slot)){ 
        auto& cam = *(ComponentCamera*)camera;
        cam.resize(width,height); 
    }
}
void epriv::ComponentManager::_deleteEntityImmediately(Entity* e){
    //obviously try to improve this performance wise
    removeFromVector(e->scene()->entities(),e->id());
    for(uint i = 0; i < ComponentType::_TOTAL; ++i){
        const uint& componentID = e->m_Components.at(i);
        if(componentID != 0){
            ComponentBaseClass* component = nullptr;
            m_ComponentPool->get(componentID,component);
            componentManager->_removeComponent(component);
            m_ComponentPool->remove(componentID);
            e->m_Components.at(i) = 0;
        }
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
        vector_clear(type.second);
        vector_clear(m_ComponentVectorsScene.at(type.first));
    }
    for(auto entityID:newScene->entities()){
        Entity* e = newScene->getEntity(entityID);
        for(uint index = 0; index < ComponentType::_TOTAL; ++index){
            uint componentID = e->m_Components.at(index);
            if(componentID != 0){
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
ComponentCamera::~ComponentCamera(){}
void ComponentCamera::update(const float& dt){}
void ComponentCamera::resize(uint width, uint height){	
    if(_type == Type::Perspective){
        _aspectRatio = width/(float)height;
    }
    epriv::ComponentInternalFunctionality::rebuildProjectionMatrix(*this);
}
uint ComponentCamera::pointIntersectTest(glm::vec3& position){
    for(ushort i=0; i < 6; ++i){
        float d = _planes[i].x * position.x + _planes[i].y * position.y + _planes[i].z * position.z + _planes[i].w;
        if (d > 0.0f) return 0; //outside
    }
    return 1;//inside
}
uint ComponentCamera::sphereIntersectTest(glm::vec3& position,float radius){
    uint res = 1; //inside the viewing frustum
    if(radius <= 0.0f) return 0;
    for (ushort i = 0; i < 6; ++i){
        float d = _planes[i].x * position.x + _planes[i].y * position.y + _planes[i].z * position.z + _planes[i].w;
        if (d > radius * 2.0f) return 0; //outside the viewing frustrum
        else if (d > 0.0f) res = 2; //intersecting the viewing plane
    }
    return res;
}
void ComponentCamera::lookAt(glm::vec3 eye,glm::vec3 forward,glm::vec3 up){
    _eye = eye;
    _up = up;
    _viewMatrix = glm::lookAt(_eye,forward,_up);
    _viewMatrixNoTranslation = glm::lookAt(glm::vec3(0.0f),forward - _eye,_up);
}
glm::mat4 ComponentCamera::getProjection(){ return _projectionMatrix; }
glm::mat4 ComponentCamera::getProjectionInverse(){ return glm::inverse(_projectionMatrix); }
glm::mat4 ComponentCamera::getView(){ return _viewMatrixNoTranslation; }
glm::mat4 ComponentCamera::getViewInverse(){ return glm::inverse(_viewMatrixNoTranslation); }
glm::mat4 ComponentCamera::getViewProjection(){ return _projectionMatrix * _viewMatrixNoTranslation; }
glm::mat4 ComponentCamera::getViewProjectionInverse(){ return glm::inverse(_projectionMatrix * _viewMatrixNoTranslation); }
glm::vec3 ComponentCamera::getViewVector(){ return glm::vec3(_viewMatrix[0][2],_viewMatrix[1][2],_viewMatrix[2][2]); }
glm::vec3 ComponentCamera::getViewVectorNoTranslation(){ return glm::vec3(_viewMatrixNoTranslation[0][2],_viewMatrixNoTranslation[1][2],_viewMatrixNoTranslation[2][2]); }
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
uint ComponentModel::addModel(Handle& mesh, Handle& mat){ return ComponentModel::addModel((Mesh*)mesh.get(),(Material*)mat.get()); }
uint ComponentModel::addModel(Mesh* mesh,Material* material){
    models.push_back( new MeshInstance(m_Owner,mesh,material) );
    m_i->calculateRadius(this);
    return models.size() - 1;
}

void ComponentModel::setModel(Handle& mesh,Handle& mat,uint index){ ComponentModel::setModel((Mesh*)mesh.get(),(Material*)mat.get(), index); }
void ComponentModel::setModel(Mesh* mesh,Material* material,uint index){
    MeshInstance& pair = *(models.at(index));
    pair.setMesh(mesh);
    pair.setMaterial(material);
    m_i->calculateRadius(this);
}

void ComponentModel::setModelMesh(Mesh* mesh,uint index){
    auto& pair = *(models.at(index));
    pair.setMesh(mesh);
    m_i->calculateRadius(this);
}
void ComponentModel::setModelMesh(Handle& mesh, uint index){ ComponentModel::setModelMesh((Mesh*)mesh.get(),index); }
void ComponentModel::setModelMaterial(Material* material,uint index){
    auto& pair = *(models.at(index));
    pair.setMaterial(material);
}
void ComponentModel::setModelMaterial(Handle& mat,uint index){ ComponentModel::setModelMaterial((Material*)mat.get(),index); }
bool ComponentModel::rayIntersectSphere(ComponentCamera* camera){
    auto* body = m_Owner->getComponent<ComponentBody>();
    return Math::rayIntersectSphere(body->position(),_radius,camera->_eye,camera->getViewVector());
}

#pragma endregion

#pragma region Body

//basic body constructor
ComponentBody::ComponentBody():ComponentBaseClass(){
    data.n = NormalData();
    _physics = false;
    data.n.position = new glm::vec3(0.0f);
    data.n.scale = new glm::vec3(1.0f);
    data.n.rotation = new glm::quat(1.0f,0.0f,0.0f,0.0f);
    data.n.modelMatrix = new glm::mat4(1.0f);
    Math::recalculateForwardRightUp(*data.n.rotation,_forward,_right,_up);
}
//rigid body constructor
ComponentBody::ComponentBody(Collision* _collision,Entity* owner,glm::vec3 _scale):ComponentBaseClass(){
    data.p = PhysicsData();	
    _physics = true;
    _forward = glm::vec3(0.0f,0.0f,-1.0f);  _right = glm::vec3(1.0f,0.0f,0.0f);  _up = glm::vec3(0.0f,1.0f,0.0f);
    m_Owner = owner;

    data.p.motionState = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1)));
    float _mass = 1.0f;
    data.p.mass = _mass;

    setCollision(_collision, false, _scale);

    setMass(_mass);
    btDefaultMotionState* _motionState = data.p.motionState;
    btCollisionShape* _shape = data.p.collision->getShape();
    const btVector3& _inertia = data.p.collision->getInertia();

    btRigidBody::btRigidBodyConstructionInfo CI(_mass,_motionState, _shape,_inertia);
    data.p.rigidBody = new btRigidBody(CI);
    data.p.rigidBody->setSleepingThresholds(0.015f,0.015f);
    data.p.rigidBody->setFriction(0.3f);
    data.p.rigidBody->setDamping(0.1f,0.4f);//this makes the objects slowly slow down in space, like air friction 
    data.p.rigidBody->setMassProps(_mass,_inertia);
    data.p.rigidBody->updateInertiaTensor();
    data.p.rigidBody->setUserPointer(this);

    if(m_Owner->scene() == Resources::getCurrentScene())
        Physics::addRigidBody(data.p.rigidBody);
}
ComponentBody::~ComponentBody(){
    if(_physics){
        Physics::removeRigidBody(data.p.rigidBody);
        SAFE_DELETE(data.p.rigidBody);
        SAFE_DELETE(data.p.motionState);
    }else{
        SAFE_DELETE(data.n.position);
        SAFE_DELETE(data.n.scale);
        SAFE_DELETE(data.n.rotation);
        SAFE_DELETE(data.n.modelMatrix);
    }
}
void ComponentBody::alignTo(glm::vec3 direction,float speed){
    if(_physics){
        //recheck this
        btTransform tr;
        data.p.rigidBody->getMotionState()->getWorldTransform(tr);
        //Math::alignTo(Math::btToGLMQuat(tr.getRotation()),direction,speed);
        Math::recalculateForwardRightUp(data.p.rigidBody,_forward,_right,_up);
    }else{
        Math::alignTo(*data.n.rotation,direction,speed);
        Math::recalculateForwardRightUp(*data.n.rotation,_forward,_right,_up);
    }
}
void ComponentBody::setCollision(Collision* collision,bool emptyCollision,glm::vec3 _scale){
    btCollisionShape* newShape = nullptr;
    Collision* meshCollision = nullptr;
    if (!collision) {
        if (!emptyCollision) {
            auto* modelComponent = m_Owner->getComponent<ComponentModel>();
            btCompoundShape* compoundShape = new btCompoundShape();
            for (auto meshInstance : modelComponent->models) {
                auto& mesh = *meshInstance->mesh();
                btTransform t; t.setFromOpenGLMatrix(glm::value_ptr(meshInstance->model()));
                meshCollision = mesh.getCollision();		
                if (meshCollision) {
                    if (meshCollision->getType() == CollisionType::ConvexHull) {
                        btConvexHullShape* cast = static_cast<btConvexHullShape*>(meshCollision->getShape());
                        newShape = new btUniformScalingShape(cast, 1.0f);
                        newShape->setLocalScaling(Math::btVectorFromGLM(_scale));
                    }else if (meshCollision->getType() == CollisionType::TriangleShapeStatic){
                        btBvhTriangleMeshShape* cast = static_cast<btBvhTriangleMeshShape*>(meshCollision->getShape());
                        newShape = new btScaledBvhTriangleMeshShape(cast, Math::btVectorFromGLM(_scale));
                    }else{
                        newShape = meshCollision->getShape();
                    }
                }
                if (newShape) {
                    newShape->setMargin(0.001f);
                    compoundShape->addChildShape(t, newShape);
                    compoundShape->recalculateLocalAabb();
                }
            }
            data.p.collision = new Collision(compoundShape, CollisionType::Compound, data.p.mass); //double check this mass value?
        }else{
            data.p.collision = new Collision(new btEmptyShape(), CollisionType::None, 0.0f);
        }
    }else{
        data.p.collision = collision;
    }
    data.p.collision->getShape()->setUserPointer(this);
}
void ComponentBody::translate(glm::vec3 translation,bool local){ ComponentBody::translate(translation.x,translation.y,translation.z,local); }
void ComponentBody::translate(float x,float y,float z,bool local){
    if(_physics){
        data.p.rigidBody->activate();
        btVector3 v(x,y,z);
        Math::translate(data.p.rigidBody,v,local);
        setPosition(  position() + Engine::Math::btVectorToGLM(v)  );
    }else{
        glm::vec3& _position = *data.n.position;
        _position.x += x; _position.y += y; _position.z += z;
        glm::vec3 offset(x,y,z);
        if(local){
            offset = *(data.n.rotation) * offset;
        }
        setPosition(_position + offset);
    }
}
void ComponentBody::rotate(glm::vec3 rotation,bool local){ ComponentBody::rotate(rotation.x,rotation.y,rotation.z,local); }
void ComponentBody::rotate(float pitch,float yaw,float roll,bool local){
    if(_physics){
        btQuaternion quat = data.p.rigidBody->getWorldTransform().getRotation().normalize();
        glm::quat glmquat(quat.w(),quat.x(),quat.y(),quat.z());

        if(abs(pitch) >= 0.001f) glmquat = glmquat * (glm::angleAxis(-pitch, glm::vec3(1,0,0)));
        if(abs(yaw) >= 0.001f)   glmquat = glmquat * (glm::angleAxis(-yaw,   glm::vec3(0,1,0)));
        if(abs(roll) >= 0.001f)  glmquat = glmquat * (glm::angleAxis(roll,   glm::vec3(0,0,1)));

        quat = btQuaternion(glmquat.x,glmquat.y,glmquat.z,glmquat.w);
        data.p.rigidBody->getWorldTransform().setRotation(quat);
    }else{
        glm::quat& _rotation = *data.n.rotation;
        if (abs(pitch) >= 0.001f) _rotation = _rotation * (glm::angleAxis(-pitch, glm::vec3(1, 0, 0)));
        if (abs(yaw) >= 0.001f)   _rotation = _rotation * (glm::angleAxis(-yaw, glm::vec3(0, 1, 0)));
        if (abs(roll) >= 0.001f)  _rotation = _rotation * (glm::angleAxis(roll, glm::vec3(0, 0, 1)));
        Math::recalculateForwardRightUp(_rotation,_forward,_right,_up);
    }
}
void ComponentBody::scale(glm::vec3 amount){ ComponentBody::scale(amount.x,amount.y,amount.z); }
void ComponentBody::scale(float x,float y,float z){
    if(_physics){
        if(data.p.collision){
            if(data.p.collision->getType() == CollisionType::Compound){
                btCompoundShape* cast = ((btCompoundShape*)(data.p.collision->getShape()));
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
        glm::vec3& _scale = *data.n.scale;
        _scale.x += x; _scale.y += y; _scale.z += z;
    }
    if(m_Owner){
        auto* models = m_Owner->getComponent<ComponentModel>();
        if(models){
            models->m_i->calculateRadius(models);
        }
    }
}
void ComponentBody::setPosition(glm::vec3 newPosition){ ComponentBody::setPosition(newPosition.x,newPosition.y,newPosition.z); }
void ComponentBody::setPosition(float x,float y,float z){
    if(_physics){
        btTransform tr;
        tr.setOrigin(btVector3(x,y,z));
        tr.setRotation(data.p.rigidBody->getOrientation()); 
        if(data.p.collision->getType() == CollisionType::TriangleShapeStatic){
            Physics::removeRigidBody(data.p.rigidBody);
            SAFE_DELETE(data.p.rigidBody);
        }
        data.p.motionState->setWorldTransform(tr);
        if(data.p.collision->getType() == CollisionType::TriangleShapeStatic){
            btRigidBody::btRigidBodyConstructionInfo ci(data.p.mass,data.p.motionState,data.p.collision->getShape(),data.p.collision->getInertia());
            data.p.rigidBody = new btRigidBody(ci);
            data.p.rigidBody->setUserPointer(this);
            Physics::addRigidBody(data.p.rigidBody);
        }
        data.p.rigidBody->setMotionState(data.p.motionState); //is this needed?
        data.p.rigidBody->setWorldTransform(tr);
        data.p.rigidBody->setCenterOfMassTransform(tr);
    }else{
        glm::vec3& _position = *data.n.position;
        glm::mat4& _matrix = *data.n.modelMatrix;
        _position.x = x; _position.y = y; _position.z = z;
        _matrix[3][0] = x;
        _matrix[3][1] = y;
        _matrix[3][2] = z;
    }
}
void ComponentBody::setRotation(glm::quat newRotation){ ComponentBody::setRotation(newRotation.x,newRotation.y,newRotation.z,newRotation.w); }
void ComponentBody::setRotation(float x,float y,float z,float w){
    if(_physics){
        btQuaternion quat(x,y,z,w);
        quat = quat.normalize();

        btTransform tr; tr.setOrigin(data.p.rigidBody->getWorldTransform().getOrigin());
        tr.setRotation(quat);

        data.p.rigidBody->setWorldTransform(tr);
        data.p.rigidBody->setCenterOfMassTransform(tr);
        data.p.motionState->setWorldTransform(tr);

        Math::recalculateForwardRightUp(data.p.rigidBody,_forward,_right,_up);

        clearAngularForces();
    }else{
        glm::quat newRotation(w,x,y,z);
        newRotation = glm::normalize(newRotation);
        glm::quat& _rotation = *data.n.rotation;
        _rotation = newRotation;
        Math::recalculateForwardRightUp(_rotation,_forward,_right,_up);
    }
}
void ComponentBody::setScale(glm::vec3 newScale){ ComponentBody::setScale(newScale.x,newScale.y,newScale.z); }
void ComponentBody::setScale(float x,float y,float z){
    if(_physics){
        if(data.p.collision){
            if (data.p.collision->getType() == CollisionType::Compound) {
                btCompoundShape* cast = dynamic_cast<btCompoundShape*>(data.p.collision->getShape());
                if (cast) {
                    int numChildren = cast->getNumChildShapes();
                    if (numChildren > 0) {
                        for (int i = 0; i < numChildren; ++i) {
                            btCollisionShape* shape = cast->getChildShape(i);
                            btUniformScalingShape* convexHull = dynamic_cast<btUniformScalingShape*>(shape);
                            if (convexHull) {
                                convexHull->setLocalScaling(btVector3(x, y, z));
                                continue;
                            }
                            btScaledBvhTriangleMeshShape* triHull = dynamic_cast<btScaledBvhTriangleMeshShape*>(shape);
                            if (triHull) {
                                triHull->setLocalScaling(btVector3(x, y, z));
                            }
                        }
                    }
                }
            }
        }
    }else{
        glm::vec3& _scale = *data.n.scale;
        _scale.x = x; _scale.y = y; _scale.z = z;
    }
    if(m_Owner){
        auto* models = m_Owner->getComponent<ComponentModel>();
        if(models){
            models->m_i->calculateRadius(models);
        }
    }
}  
const btRigidBody* ComponentBody::getBody() const{ return data.p.rigidBody; }
glm::vec3 ComponentBody::position(){ //theres prob a better way to do this
    if(_physics){
        glm::mat4 m(1.0f);
        btTransform tr;  data.p.rigidBody->getMotionState()->getWorldTransform(tr);
        tr.getOpenGLMatrix(glm::value_ptr(m));
        return glm::vec3(m[3][0],m[3][1],m[3][2]);
    }
    glm::mat4& _matrix = *data.n.modelMatrix;
    return glm::vec3(_matrix[3][0],_matrix[3][1],_matrix[3][2]);
}
glm::vec3 ComponentBody::getScreenCoordinates(){ return Math::getScreenCoordinates(position(),false); }
glm::vec3 ComponentBody::getScale(){
    if(_physics){
        if(data.p.collision){
            if(data.p.collision->getType() == CollisionType::Compound){
                btCompoundShape* cast = dynamic_cast<btCompoundShape*>(data.p.collision->getShape());
                if (cast) {
                    int numChildren = cast->getNumChildShapes();
                    if (numChildren > 0) {
                        for (int i = 0; i < numChildren; ++i) {
                            btCollisionShape* shape = cast->getChildShape(i);
                            btUniformScalingShape* convexHull = dynamic_cast<btUniformScalingShape*>(shape);
                            if (convexHull) {
                                return Math::btVectorToGLM(const_cast<btVector3&>(convexHull->getLocalScaling()));
                            }
                            btScaledBvhTriangleMeshShape* triHull = dynamic_cast<btScaledBvhTriangleMeshShape*>(shape);
                            if (triHull) {
                                return Math::btVectorToGLM(const_cast<btVector3&>(triHull->getLocalScaling()));
                            }
                        }
                    }
                }
            }
        }
        return glm::vec3(1.0f);
    }
    return *data.n.scale; 
}
glm::quat ComponentBody::rotation(){
    if(_physics){
        return Engine::Math::btToGLMQuat(data.p.rigidBody->getWorldTransform().getRotation());
    }
    return *data.n.rotation;
}
glm::vec3 ComponentBody::forward(){ return _forward; }
glm::vec3 ComponentBody::right(){ return _right; }
glm::vec3 ComponentBody::up(){ return _up; }
glm::vec3 ComponentBody::getLinearVelocity(){ btVector3 v = data.p.rigidBody->getLinearVelocity(); return Engine::Math::btVectorToGLM(v); }
glm::vec3 ComponentBody::getAngularVelocity(){ btVector3 v = data.p.rigidBody->getAngularVelocity(); return Engine::Math::btVectorToGLM(v); }
float ComponentBody::mass(){ return data.p.mass; }
glm::mat4 ComponentBody::modelMatrix(){ //theres prob a better way to do this
    if(_physics){
        glm::mat4 m(1.0f);
        btTransform tr;  data.p.rigidBody->getMotionState()->getWorldTransform(tr);
        tr.getOpenGLMatrix(glm::value_ptr(m));
        if(data.p.collision){
            m = glm::scale(m,getScale());
        }
        return m;
    }
    return *data.n.modelMatrix;
}
void ComponentBody::setDamping(float linear,float angular){ data.p.rigidBody->setDamping(linear,angular); }
void ComponentBody::setDynamic(bool dynamic){
    if(dynamic){
        Physics::removeRigidBody(data.p.rigidBody);
        data.p.rigidBody->setCollisionFlags(btCollisionObject::CF_ANISOTROPIC_FRICTION_DISABLED);
        Physics::addRigidBody(data.p.rigidBody);
        data.p.rigidBody->activate();
    }else{
        Physics::removeRigidBody(data.p.rigidBody);
        data.p.rigidBody->setCollisionFlags(btCollisionObject::CF_STATIC_OBJECT);
        ComponentBody::clearAllForces();
        Physics::addRigidBody(data.p.rigidBody);
        data.p.rigidBody->activate();
    }
}
void ComponentBody::setLinearVelocity(float x,float y,float z,bool local){
    data.p.rigidBody->activate();
    btVector3 v(x,y,z);
    Math::translate(data.p.rigidBody,v,local);
    data.p.rigidBody->setLinearVelocity(v); 
}
void ComponentBody::setLinearVelocity(glm::vec3 velocity,bool local){ ComponentBody::setLinearVelocity(velocity.x,velocity.y,velocity.z,local); }
void ComponentBody::setAngularVelocity(float x,float y,float z,bool local){
    data.p.rigidBody->activate();
    btVector3 v(x,y,z);
    Math::translate(data.p.rigidBody,v,local);
    data.p.rigidBody->setAngularVelocity(v); 
}
void ComponentBody::setAngularVelocity(glm::vec3 velocity,bool local){ ComponentBody::setAngularVelocity(velocity.x,velocity.y,velocity.z,local); }
void ComponentBody::applyForce(float x,float y,float z,bool local){
    data.p.rigidBody->activate();
    btVector3 v(x,y,z);
    Math::translate(data.p.rigidBody,v,local);
    data.p.rigidBody->applyCentralForce(v); 
}
void ComponentBody::applyForce(glm::vec3 force,glm::vec3 origin,bool local){
    data.p.rigidBody->activate();
    btVector3 v(force.x,force.y,force.z);
    if(local){
        btQuaternion q = data.p.rigidBody->getWorldTransform().getRotation().normalize();
        v = v.rotate(q.getAxis(),q.getAngle());
    }
    data.p.rigidBody->applyForce(v,btVector3(origin.x,origin.y,origin.z)); 
}
void ComponentBody::applyImpulse(float x,float y,float z,bool local){
    data.p.rigidBody->activate();
    btVector3 v(x,y,z);
    Math::translate(data.p.rigidBody,v,local);
    data.p.rigidBody->applyCentralImpulse(v);
}
void ComponentBody::applyImpulse(glm::vec3 impulse,glm::vec3 origin,bool local){
    data.p.rigidBody->activate();
    btVector3 v(impulse.x,impulse.y,impulse.z);
    if(local){
        btQuaternion q = data.p.rigidBody->getWorldTransform().getRotation().normalize();
        v = v.rotate(q.getAxis(),q.getAngle());
    }
    data.p.rigidBody->applyImpulse(v,btVector3(origin.x,origin.y,origin.z));
}
void ComponentBody::applyTorque(float x,float y,float z,bool local){
    data.p.rigidBody->activate();
    btVector3 t(x,y,z);
    if(local){
        t = data.p.rigidBody->getInvInertiaTensorWorld().inverse() * (data.p.rigidBody->getWorldTransform().getBasis() * t);
    }
    data.p.rigidBody->applyTorque(t);
}
void ComponentBody::applyTorque(glm::vec3 torque,bool local){ ComponentBody::applyTorque(torque.x,torque.y,torque.z,local); }
void ComponentBody::applyTorqueImpulse(float x,float y,float z,bool local){
    data.p.rigidBody->activate();
    btVector3 t(x,y,z);
    if(local){
        t = data.p.rigidBody->getInvInertiaTensorWorld().inverse() * (data.p.rigidBody->getWorldTransform().getBasis() * t);
    }
    data.p.rigidBody->applyTorqueImpulse(t);
}
void ComponentBody::applyTorqueImpulse(glm::vec3 torqueImpulse,bool local){ ComponentBody::applyTorqueImpulse(torqueImpulse.x,torqueImpulse.y,torqueImpulse.z,local); }
void ComponentBody::clearLinearForces(){
    data.p.rigidBody->setActivationState(0);
    data.p.rigidBody->activate();
    btVector3 v(0,0,0);
    data.p.rigidBody->setLinearVelocity(v); 
}
void ComponentBody::clearAngularForces(){
    data.p.rigidBody->setActivationState(0);
    data.p.rigidBody->activate();
    btVector3 v(0,0,0);
    data.p.rigidBody->setAngularVelocity(v); 
}
void ComponentBody::clearAllForces(){
    data.p.rigidBody->setActivationState(0);
    data.p.rigidBody->activate();
    btVector3 v(0,0,0);
    data.p.rigidBody->setLinearVelocity(v); 
    data.p.rigidBody->setAngularVelocity(v); 
}
void ComponentBody::setMass(float mass){
    data.p.mass = mass;
    if(data.p.collision){
        data.p.collision->setMass(data.p.mass);
        if(data.p.rigidBody){
            data.p.rigidBody->setMassProps(data.p.mass,data.p.collision->getInertia());
        }
    }
}
#pragma endregion

Entity::Entity(){
    m_Scene = nullptr;
    m_ID = m_ParentID = 0;
    m_Components.resize(ComponentType::_TOTAL, 0);
}
Entity::~Entity(){
    m_ID = m_ParentID = 0;
    m_Scene = nullptr;
}
const uint Entity::id() const { return m_ID; }
Scene* Entity::scene(){ return m_Scene; }
void Entity::destroy(bool immediate){
    if(!immediate){
        componentManager->_addEntityToBeDestroyed(m_ID); //add to the deletion queue
    }else{
        componentManager->_deleteEntityImmediately(this); //delete immediately
    }
}
Entity* Entity::parent(){
    if(m_ParentID == 0)
        return nullptr;
    return componentManager->_getEntity(m_ParentID);
}
void Entity::addChild(Entity* child){
    child->m_ParentID = m_ID;
}
