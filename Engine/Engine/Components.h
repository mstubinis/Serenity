#pragma once
#ifndef ENGINE_OBJECT_COMPONENTS_H
#define ENGINE_OBJECT_COMPONENTS_H

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <memory>
#include <vector>
#include "Engine_EventObject.h"
#include "Engine_ObjectPool.h"

#include <boost/type_index.hpp>
#include <unordered_map>
#include <boost/unordered_map.hpp>
#include <limits>

#include <boost/function.hpp>
#include <boost/bind.hpp>

typedef unsigned int uint;
typedef unsigned short ushort;

class Entity;
class Scene;
class Camera;
class Mesh;
class Material;
class Collision;
class btRigidBody;
struct btDefaultMotionState;
class btVector3;
struct Handle;
class MeshInstance;

class ComponentBaseClass;
class ComponentBasicBody;
class ComponentRigidBody;
class ComponentModel;
class ComponentCamera;

typedef boost::typeindex::type_index boost_type_index;

namespace Engine{
    namespace epriv{
		class ComponentInternalFunctionality;
        struct MeshMaterialPair;
        class ComponentTypeRegistry;
        class ComponentManager;
        class ComponentBodyBaseClass;
    };
};
class ComponentType{public:enum Type{
    Body, //Can contain: ComponentRigidBody, ComponentBasicBody, maybe more...
    Model,
    Camera, //Can contain: ComponentCamera, ComponentGameCamera
_TOTAL,};};

class ComponentBaseClass{
    friend class ::Engine::epriv::ComponentManager;
    friend class ::Entity;
    protected:
        Entity* m_Owner; //eventually make this an entity ID instead?
    public:
        BOOST_TYPE_INDEX_REGISTER_CLASS
        ComponentBaseClass();
        virtual ~ComponentBaseClass();
};
namespace Engine{

    template<typename Base,typename Derived> void registerComponent(){
        boost_type_index baseType    = boost_type_index(boost::typeindex::type_id<Base>());
        boost_type_index derivedType = boost_type_index(boost::typeindex::type_id<Derived>());

        if(!epriv::ComponentTypeRegistry::m_MapScene.count(derivedType)){
            uint baseClassSlot = epriv::ComponentTypeRegistry::m_MapScene.at(baseType);
            epriv::ComponentTypeRegistry::m_MapScene.emplace(derivedType,baseClassSlot);
        }
        if(!epriv::ComponentTypeRegistry::m_Map.count(derivedType)){
            uint baseClassSlot = epriv::ComponentTypeRegistry::m_Map.at(baseType);
            epriv::ComponentTypeRegistry::m_Map.emplace(derivedType,baseClassSlot);
        }
    }

    namespace epriv{
        const uint MAX_NUM_ENTITIES = 32768;

        class ComponentManager final{
            friend class ::Entity;
            friend class ::Scene;
            friend class ::Engine::epriv::ComponentTypeRegistry;
            private:
                class impl;
                EntityPool<Entity>*                    m_EntityPool;
                static EntityPool<ComponentBaseClass>* m_ComponentPool;
                static std::unordered_map<uint, std::vector<ComponentBaseClass*>> m_ComponentVectors;
                static std::unordered_map<uint, std::vector<ComponentBaseClass*>> m_ComponentVectorsScene;

                template<typename T> uint getIndividualComponentTypeSlot(){
                    boost_type_index typeIndex = boost_type_index(boost::typeindex::type_id<T>());
                    return Engine::epriv::ComponentTypeRegistry::m_MapScene.at(typeIndex);
                }
                template<typename T> uint getIndividualComponentTypeSlot(T* component){
                    boost_type_index typeIndex = boost_type_index(boost::typeindex::type_id_runtime(*component));
                    return Engine::epriv::ComponentTypeRegistry::m_MapScene.at(typeIndex);
                }
            public:
                std::unique_ptr<impl> m_i;

                ComponentManager(const char* name, uint w, uint h);
                ~ComponentManager();

                void _init(const char* name, uint w, uint h);
				void _pause(bool=true);
				void _unpause();
                void _update(const float& dt);
                void _render();
                void _resize(uint width,uint height);

                void _sceneSwap(Scene* oldScene, Scene* newScene);
                void _deleteEntityImmediately(Entity*);
                void _addEntityToBeDestroyed(uint id);
                void _addEntityToBeDestroyed(Entity*);
                Entity* _getEntity(uint id);

                ComponentBaseClass* _getComponent(uint index);
                void _removeComponent(uint componentID);
                void _removeComponent(ComponentBaseClass* component);
        };
        class ComponentTypeRegistry final{
            friend class ::Entity;
            friend class ::Engine::epriv::ComponentManager;
            private:
                uint m_NextIndex;
                uint m_NextIndexScene;
            public:
                static boost::unordered_map<boost_type_index,uint> m_Map;
                static boost::unordered_map<boost_type_index,uint> m_MapScene;

                ComponentTypeRegistry(){
                    m_NextIndex = 0;
                    m_NextIndexScene = 0;
                }
                ~ComponentTypeRegistry(){
                    m_Map.clear();
                    m_MapScene.clear();
                    m_NextIndex = 0;
                    m_NextIndexScene = 0;
                }
                template<typename T> void emplace(){
                    m_Map.emplace(boost_type_index(boost::typeindex::type_id<T>()),m_NextIndex);
                    ++m_NextIndex;
                }
                template<typename T,typename V> void emplace(){
                    m_Map.emplace(boost_type_index(boost::typeindex::type_id<T>()),m_NextIndex);
                    m_Map.emplace(boost_type_index(boost::typeindex::type_id<V>()),m_NextIndex);
                    ++m_NextIndex;
                }
                template<typename T,typename V,typename W> void emplace(){
                    m_Map.emplace(boost_type_index(boost::typeindex::type_id<T>()),m_NextIndex);
                    m_Map.emplace(boost_type_index(boost::typeindex::type_id<V>()),m_NextIndex);
                    m_Map.emplace(boost_type_index(boost::typeindex::type_id<W>()),m_NextIndex);
                    ++m_NextIndex;
                }
                template<typename T,typename V,typename W,typename X> void emplace(){
                    m_Map.emplace(boost_type_index(boost::typeindex::type_id<T>()),m_NextIndex);
                    m_Map.emplace(boost_type_index(boost::typeindex::type_id<V>()),m_NextIndex);
                    m_Map.emplace(boost_type_index(boost::typeindex::type_id<W>()),m_NextIndex);
                    m_Map.emplace(boost_type_index(boost::typeindex::type_id<X>()),m_NextIndex);
                    ++m_NextIndex;
                }
                template<typename T,typename V,typename W,typename X,typename Y> void emplace(){
                    m_Map.emplace(boost_type_index(boost::typeindex::type_id<T>()),m_NextIndex);
                    m_Map.emplace(boost_type_index(boost::typeindex::type_id<V>()),m_NextIndex);
                    m_Map.emplace(boost_type_index(boost::typeindex::type_id<W>()),m_NextIndex);
                    m_Map.emplace(boost_type_index(boost::typeindex::type_id<X>()),m_NextIndex);
                    m_Map.emplace(boost_type_index(boost::typeindex::type_id<Y>()),m_NextIndex);
                    ++m_NextIndex;
                }
                template<typename T,typename V,typename W,typename X,typename Y,typename Z> void emplace(){
                    m_Map.emplace(boost_type_index(boost::typeindex::type_id<T>()),m_NextIndex);
                    m_Map.emplace(boost_type_index(boost::typeindex::type_id<V>()),m_NextIndex);
                    m_Map.emplace(boost_type_index(boost::typeindex::type_id<W>()),m_NextIndex);
                    m_Map.emplace(boost_type_index(boost::typeindex::type_id<X>()),m_NextIndex);
                    m_Map.emplace(boost_type_index(boost::typeindex::type_id<Y>()),m_NextIndex);
                    m_Map.emplace(boost_type_index(boost::typeindex::type_id<Z>()),m_NextIndex);
                    ++m_NextIndex;
                }
                void _emplaceSceneFinal(){
                    std::vector<ComponentBaseClass*> v;
                    ComponentManager::m_ComponentVectors.emplace(m_NextIndexScene,v);
                    ComponentManager::m_ComponentVectorsScene.emplace(m_NextIndexScene,v);
                    ++m_NextIndexScene;
                }
                
                template<typename T> void emplaceVector(){
                    m_MapScene.emplace(boost_type_index(boost::typeindex::type_id<T>()),m_NextIndexScene);

                   _emplaceSceneFinal();
                }
                template<typename T,typename U> void emplaceVector(){
                    m_MapScene.emplace(boost_type_index(boost::typeindex::type_id<T>()),m_NextIndexScene);
                    m_MapScene.emplace(boost_type_index(boost::typeindex::type_id<U>()),m_NextIndexScene);

                   _emplaceSceneFinal();
                }
                template<typename T,typename U,typename V> void emplaceVector(){
                    m_MapScene.emplace(boost_type_index(boost::typeindex::type_id<T>()),m_NextIndexScene);
                    m_MapScene.emplace(boost_type_index(boost::typeindex::type_id<U>()),m_NextIndexScene);
                    m_MapScene.emplace(boost_type_index(boost::typeindex::type_id<V>()),m_NextIndexScene);

                   _emplaceSceneFinal();
                }
                template<typename T,typename U,typename V,typename W> void emplaceVector(){
                    m_MapScene.emplace(boost_type_index(boost::typeindex::type_id<T>()),m_NextIndexScene);
                    m_MapScene.emplace(boost_type_index(boost::typeindex::type_id<U>()),m_NextIndexScene);
                    m_MapScene.emplace(boost_type_index(boost::typeindex::type_id<V>()),m_NextIndexScene);
                    m_MapScene.emplace(boost_type_index(boost::typeindex::type_id<W>()),m_NextIndexScene);

                   _emplaceSceneFinal();
                }
                template<typename T,typename U,typename V,typename W,typename X> void emplaceVector(){
                    m_MapScene.emplace(boost_type_index(boost::typeindex::type_id<T>()),m_NextIndexScene);
                    m_MapScene.emplace(boost_type_index(boost::typeindex::type_id<U>()),m_NextIndexScene);
                    m_MapScene.emplace(boost_type_index(boost::typeindex::type_id<V>()),m_NextIndexScene);
                    m_MapScene.emplace(boost_type_index(boost::typeindex::type_id<W>()),m_NextIndexScene);
                    m_MapScene.emplace(boost_type_index(boost::typeindex::type_id<X>()),m_NextIndexScene);

                   _emplaceSceneFinal();
                }
                template<typename T,typename U,typename V,typename W,typename X,typename Y> void emplaceVector(){
                    m_MapScene.emplace(boost_type_index(boost::typeindex::type_id<T>()),m_NextIndexScene);
                    m_MapScene.emplace(boost_type_index(boost::typeindex::type_id<U>()),m_NextIndexScene);
                    m_MapScene.emplace(boost_type_index(boost::typeindex::type_id<V>()),m_NextIndexScene);
                    m_MapScene.emplace(boost_type_index(boost::typeindex::type_id<W>()),m_NextIndexScene);
                    m_MapScene.emplace(boost_type_index(boost::typeindex::type_id<X>()),m_NextIndexScene);
                    m_MapScene.emplace(boost_type_index(boost::typeindex::type_id<Y>()),m_NextIndexScene);

                   _emplaceSceneFinal();
                }
        };
        class ComponentBodyType{public:enum Type{
            BasicBody,
            RigidBody,

        _TOTAL,};};
        class ComponentBodyBaseClass: public ComponentBaseClass{
            protected:
                ComponentBodyType::Type _type;
            public:
                BOOST_TYPE_INDEX_REGISTER_CLASS
                ComponentBodyBaseClass(ComponentBodyType::Type);
                virtual ~ComponentBodyBaseClass();
                ComponentBodyType::Type getBodyType();
                virtual glm::quat rotation(){ return glm::quat(); }
                virtual glm::vec3 getScale(){ return glm::vec3(1.0f); }
				virtual glm::vec3 getScreenCoordinates(){ return glm::vec3(-9999.0f,-9999.0f,0.0f); }
                virtual glm::vec3 position(){ return glm::vec3(0.0f); }
                virtual glm::mat4 modelMatrix(){ return glm::mat4(1.0f); }
                virtual glm::vec3 forward(){ return glm::vec3(0.0f,0.0f,-1.0f); }
                virtual glm::vec3 right(){ return glm::vec3(1.0f,0.0f,0.0f); }
                virtual glm::vec3 up(){ return glm::vec3(0.0f,1.0f,0.0f); }
                virtual void setPosition(float x,float y,float z){}
                virtual void setPosition(glm::vec3& pos){}
        };
    };
};

class ComponentModel: public ComponentBaseClass{
    friend class ::Engine::epriv::ComponentManager;
    friend class ::ComponentRigidBody;
    friend class ::ComponentBasicBody;
    private:
        std::vector<MeshInstance*> models;
        float _radius;
        class impl; std::unique_ptr<impl> m_i;
    public:
        BOOST_TYPE_INDEX_REGISTER_CLASS
        ComponentModel(Handle& meshHandle,Handle& materialHandle,Entity*);
        ComponentModel(Mesh*,Handle& materialHandle,Entity*);
        ComponentModel(Handle& meshHandle,Material*,Entity*);
        ComponentModel(Mesh*,Material*,Entity*);
        ~ComponentModel();

        float radius();
        bool passedRenderCheck();
        bool visible();
        void show();
        void hide();

        MeshInstance* getModel(uint index = 0);

        uint addModel(Handle& meshHandle, Handle& materialHandle);
        uint addModel(Mesh*,Material*);

        void setModel(Handle& meshHandle,Handle& materialHandle,uint index);
        void setModel(Mesh*,Material*,uint index);

        void setModelMesh(Mesh*,uint index);
        void setModelMesh(Handle& meshHandle, uint index);
        
        void setModelMaterial(Material*,uint index);
        void setModelMaterial(Handle& materialHandle,uint index);

        bool rayIntersectSphere(ComponentCamera* camera);

        template<class T> void setCustomBindFunctor  (T& functor,uint index = 0){ models.at(index)->setCustomBindFunctor(functor); }
        template<class T> void setCustomUnbindFunctor(T& functor,uint index = 0){ models.at(index)->setCustomUnbindFunctor(functor); }
};

class ComponentBasicBody: public Engine::epriv::ComponentBodyBaseClass{
    friend class ::Engine::epriv::ComponentManager;
    friend class ::ComponentModel;
    friend class ::Camera;
    private:
        glm::mat4 _modelMatrix;
        glm::vec3 _position, _scale, _forward, _right, _up;
        glm::quat _rotation;
    public:
        BOOST_TYPE_INDEX_REGISTER_CLASS
        ComponentBasicBody();
        ~ComponentBasicBody();

		glm::vec3 getScreenCoordinates();
        glm::vec3 position();
        glm::vec3 getScale();
        glm::vec3 forward();
        glm::vec3 right();
        glm::vec3 up();
        glm::mat4 modelMatrix();
        glm::quat rotation();

        void alignTo(glm::vec3 direction,float speed = 0);

        void translate(glm::vec3& translation,bool local = true);    void translate(float x,float y,float z,bool local = true);
        void rotate(glm::vec3& rotation);                            void rotate(float pitch,float yaw,float roll);
        void scale(glm::vec3& amount);                               void scale(float x,float y,float z);

        void setPosition(glm::vec3& newPosition);                    void setPosition(float x,float y,float z);
        void setRotation(glm::quat& newRotation);                    void setRotation(float x,float y,float z,float w);
        void setScale(glm::vec3& newScale);                          void setScale(float x,float y,float z);
};

class ComponentRigidBody: public Engine::epriv::ComponentBodyBaseClass{
    friend class ::Engine::epriv::ComponentManager;
    private:
        Collision* _collision;
        btRigidBody* _rigidBody;
        btDefaultMotionState* _motionState;
        glm::vec3 _forward, _right, _up;
        float _mass;
    public:
        BOOST_TYPE_INDEX_REGISTER_CLASS
        ComponentRigidBody(Collision* = nullptr,Entity* owner = nullptr);
        ~ComponentRigidBody();

        void translate(glm::vec3& translation,bool local = true);   void translate(float x,float y,float z,bool local = true);
        void rotate(glm::vec3& rotation,bool local = true);         void rotate(float pitch,float yaw,float roll,bool local = true);
        void scale(glm::vec3& amount);                              void scale(float x,float y,float z);

        void setPosition(glm::vec3& newPosition);                   void setPosition(float x,float y,float z);
        void setRotation(glm::quat& newRotation);                   void setRotation(float x,float y,float z,float w);
        void setScale(glm::vec3& newScale);                         void setScale(float x,float y,float z);

        float mass();
		glm::vec3 getScreenCoordinates();
        glm::quat rotation();
        glm::vec3 position();
        glm::vec3 forward();
        glm::vec3 right();
        glm::vec3 up();
        glm::vec3 getLinearVelocity();
        glm::vec3 getAngularVelocity();
        glm::mat4 modelMatrix();
        const btRigidBody* getBody() const;

        void setCollision(Collision*,bool emptyCollision = false);
        void setDamping(float linear,float angular);

        void setDynamic(bool dynamic);
        void setMass(float mass);

        void clearLinearForces();
        void clearAngularForces();
        void clearAllForces();

        void setLinearVelocity(float x,float y,float z,bool local = true);   void setLinearVelocity(glm::vec3 velocity,bool local = true);
        void setAngularVelocity(float x,float y,float z,bool local = true);  void setAngularVelocity(glm::vec3 velocity,bool local = true);
        void applyForce(float x,float y,float z,bool local=true);            void applyForce(glm::vec3 force,glm::vec3 origin = glm::vec3(0),bool local=true);
        void applyImpulse(float x,float y,float z,bool local=true);          void applyImpulse(glm::vec3 impulse,glm::vec3 origin = glm::vec3(0),bool local=true);
        void applyTorque(float x,float y,float z,bool local=true);           void applyTorque(glm::vec3 torque,bool local=true);
        void applyTorqueImpulse(float x,float y,float z,bool local=true);    void applyTorqueImpulse(glm::vec3 torqueImpulse,bool local=true);
};

class ComponentCamera: public ComponentBaseClass{
    friend class ::Engine::epriv::ComponentManager;
	friend class ::Engine::epriv::ComponentInternalFunctionality;
    friend class ::ComponentModel;
    friend class ::Camera;
    private:
        enum Type{ Perspective, Orthographic, };
        Type _type;
        glm::vec3 _eye,_up;
        glm::mat4 _viewMatrix,  _projectionMatrix;
        glm::vec4 _planes[6];
        float _nearPlane,  _farPlane,  _bottom,  _top;
        union{ float _angle;        float _left;  };
        union{ float _aspectRatio;  float _right; };
    public:
        BOOST_TYPE_INDEX_REGISTER_CLASS
        ComponentCamera();
        ComponentCamera(float angle,float aspectRatio,float nearPlane,float farPlane);
        ComponentCamera(float left,float right,float bottom,float top,float nearPlane,float farPlane);
        ~ComponentCamera();

        virtual void update(const float& dt);
        void resize(uint width,uint height);
        void lookAt(glm::vec3 eye,glm::vec3 forward,glm::vec3 up);

        glm::mat4 getViewProjectionInverse();
        glm::mat4 getProjection();
        glm::mat4 getView();
        glm::mat4 getViewInverse();
        glm::mat4 getProjectionInverse();
        glm::mat4 getViewProjection();
        glm::vec3 getViewVector();

		float getAngle();    void setAngle(float);
		float getAspect();   void setAspect(float);
		float getNear();     void setNear(float);
		float getFar();      void setFar(float);

        bool sphereIntersectTest(glm::vec3 position,float radius);
};

class Entity{
    friend class ::Scene;
    friend class ::Engine::epriv::ComponentManager;
    private:
        Scene* m_Scene;
        uint m_ParentID, m_ID;
        uint* m_Components;
    public:
        Entity();
        virtual ~Entity();

        uint id();
        Scene* scene();
        virtual void registerEvent(EventType::Type type){}
        virtual void unregisterEvent(EventType::Type type){}
        virtual void update(const float& dt){}
        virtual void onEvent(const Event& e){}

        void destroy(bool immediate = false); //completely eradicate from memory. by default it its eradicated at the end of the frame before rendering logic, but can be overrided to be deleted immediately after the call
        Entity* parent();
        void addChild(Entity* child);

        template<typename T> T* getComponent(){
            boost_type_index typeIndex = boost_type_index(boost::typeindex::type_id<T>());
            uint slot = Engine::epriv::ComponentTypeRegistry::m_Map.at(typeIndex);
            const uint& componentID = m_Components[slot];
            if(componentID == std::numeric_limits<uint>::max()){
                return nullptr;
            }
            T* c = nullptr;
            Engine::epriv::ComponentManager::m_ComponentPool->getAsFast(componentID,c);
            return c;
        }
        template<typename T> void addComponent(T* component){
            boost_type_index typeIndex = boost_type_index(boost::typeindex::type_id<T>());
            uint type = Engine::epriv::ComponentTypeRegistry::m_Map.at(typeIndex);
            uint& componentID = m_Components[type];
            if(componentID != std::numeric_limits<uint>::max()) return;
            uint generatedID = Engine::epriv::ComponentManager::m_ComponentPool->add(component);
            uint sceneSlot = Engine::epriv::ComponentTypeRegistry::m_MapScene.at(typeIndex);
            std::vector<ComponentBaseClass*>& v = Engine::epriv::ComponentManager::m_ComponentVectors.at(sceneSlot);			
            v.push_back(component);
            if(m_Scene && m_Scene == Resources::getCurrentScene()){
                std::vector<ComponentBaseClass*>& vScene = Engine::epriv::ComponentManager::m_ComponentVectorsScene.at(sceneSlot);
                vScene.push_back(component);
            }
            component->m_Owner = this;
            componentID = generatedID;
        }
        template<typename T> void removeComponent(T* component){
            boost_type_index typeIndex = boost_type_index(boost::typeindex::type_id<T>());
            uint type = Engine::epriv::ComponentTypeRegistry::m_Map.at(typeIndex);
            uint& componentID = m_Components[type];
            if(componentID == std::numeric_limits<uint>::max()) return;
            component->m_Owner = nullptr;
            uint sceneSlot = Engine::epriv::ComponentTypeRegistry::m_MapScene.at(typeIndex);
            std::vector<ComponentBaseClass*>& v = Engine::epriv::ComponentManager::m_ComponentVectors.at(sceneSlot);
            std::vector<ComponentBaseClass*>& vScene = Engine::epriv::ComponentManager::m_ComponentVectorsScene.at(sceneSlot);
            removeFromVector(v,component);
            removeFromVector(vScene,component);
            Engine::epriv::ComponentManager::m_ComponentPool->remove(componentID);
            componentID = std::numeric_limits<uint>::max();
        }
};
#endif