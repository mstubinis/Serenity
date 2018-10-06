#pragma once
#ifndef ENGINE_OBJECT_COMPONENTS_H
#define ENGINE_OBJECT_COMPONENTS_H

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include "core/engine/Engine_EventObject.h"
#include "core/engine/Engine_ObjectPool.h"

#include <boost/type_index.hpp>
#include <boost/unordered_map.hpp>

#include <boost/function.hpp>
#include <boost/bind.hpp>

#include <Bullet/LinearMath/btDefaultMotionState.h>
#include <Bullet/BulletDynamics/Dynamics/btRigidBody.h>

typedef unsigned short ushort;

struct Handle;
class OLD_Entity;
class Scene;
class Camera;
class ShaderP;
class Mesh;
class Material;
class MeshInstance;
class Collision;
class btRigidBody;
struct btDefaultMotionState;
class btVector3;

class OLD_ComponentBaseClass;
class OLD_ComponentBody;
class OLD_ComponentModel;
class OLD_ComponentCamera;
class OLD_Components;

typedef boost::typeindex::type_index   boost_type_index;

template <typename T> const boost_type_index type_id() { return boost_type_index(boost::typeindex::type_id<T>()); }
template <typename T> const boost_type_index type_id(T* component) { return boost_type_index(boost::typeindex::type_id_runtime(*component)); }

namespace Engine{
    namespace epriv{
        struct InternalComponentPublicInterface final {
            friend class ::OLD_ComponentCamera;
            friend class ::Camera;
            static glm::mat4 GetViewNoTranslation(const Camera&);
            static glm::mat4 GetViewInverseNoTranslation(const Camera&);
            static glm::mat4 GetViewProjectionNoTranslation(const Camera&);
            static glm::mat4 GetViewProjectionInverseNoTranslation(const Camera&);
            static glm::vec3 GetViewVectorNoTranslation(const Camera&);
        };
        class OLD_ComponentInternalFunctionality;
        struct MeshMaterialPair;
        class OLD_ComponentTypeRegistry;
        class OLD_ComponentManager;
        class OLD_ComponentCameraSystem;
        class OLD_ComponentModelSystem;
        class OLD_ComponentBodySystem;
    };
};
struct OLD_ComponentType{enum Type{
    Body,
    Model,
    Camera, //Can contain: ComponentCamera, ComponentGameCamera
_TOTAL,};};

class OLD_Components final {
    friend class ::Engine::epriv::OLD_ComponentManager;
    public:
        static OLD_ComponentBaseClass* GetComponent(uint id);
        static OLD_Entity* GetEntity(uint id);
        template <typename T> static uint getSlot() {
            return Engine::epriv::OLD_ComponentTypeRegistry::m_Registry.at(type_id<T>());
        }
        template <typename T> static uint getSlot(T* component) {
            return Engine::epriv::OLD_ComponentTypeRegistry::m_Registry.at(type_id(component));
        }
};


class OLD_ComponentBaseClass{
    friend class ::Engine::epriv::OLD_ComponentManager;
    friend class ::OLD_Entity;
    protected:
        uint m_Owner;
    public:
        BOOST_TYPE_INDEX_REGISTER_CLASS
        OLD_ComponentBaseClass() { m_Owner = 0; }
        OLD_Entity* owner() { return OLD_Components::GetEntity(m_Owner); }
        virtual ~OLD_ComponentBaseClass() {}
};
namespace Engine{

    template<class Base,class Derived> void registerComponent(){
        const boost_type_index baseType    = type_id<Base>();
        const boost_type_index derivedType = type_id<Derived>();
        auto& map = epriv::OLD_ComponentTypeRegistry::m_Registry;
        if(!map.count(derivedType)){
            const uint& baseClassSlot = map.at(baseType);
            map.emplace(derivedType,baseClassSlot);
        }
    }

    namespace epriv{
        const uint MAX_NUM_ENTITIES = 131072;
        class OLD_ComponentManager final{
            friend class ::OLD_Entity;
            friend class ::Scene;
            friend class ::OLD_Components;
            friend class ::Engine::epriv::OLD_ComponentTypeRegistry;
            friend class ::Engine::epriv::OLD_ComponentCameraSystem;
            friend class ::Engine::epriv::OLD_ComponentModelSystem;
            friend class ::Engine::epriv::OLD_ComponentBodySystem;
            private:
                class impl; std::unique_ptr<impl> m_i;
                ObjectPool<OLD_Entity>*                                                 m_EntityPool;
                static ObjectPool<OLD_ComponentBaseClass>*                              m_ComponentPool;
                static boost::unordered_map<uint, std::vector<OLD_ComponentBaseClass*>> m_ComponentVectors;
                static boost::unordered_map<uint, std::vector<OLD_ComponentBaseClass*>> m_ComponentVectorsScene;
            public:
                OLD_ComponentManager(const char* name, uint w, uint h);
                ~OLD_ComponentManager();

                static void onComponentAddedToEntity(OLD_Entity&);
                static void onEntityAddedToScene(Scene&, OLD_Entity&);
                static void onSceneSwap(Scene*, Scene*, OLD_Entity&);

                void _pause(bool=true);
                void _unpause();
                void _update(const float& dt);
                void _resize(uint width,uint height);

                void _sceneSwap(Scene* oldScene, Scene* newScene);
                void _deleteEntityImmediately(OLD_Entity&);
                void _addEntityToBeDestroyed(uint id);
                void _addEntityToBeDestroyed(OLD_Entity&);

                void _removeComponent(uint componentID);
                void _removeComponent(OLD_ComponentBaseClass* component);
        };
        class OLD_ComponentTypeRegistry final{
            friend class ::OLD_Entity;
            friend class ::Engine::epriv::OLD_ComponentManager;
            private:
                uint m_NextIndex;
            public:
                static boost::unordered_map<boost_type_index,uint> m_Registry;

                static uint slot(const boost_type_index& _index) { return m_Registry.at(_index); }

                OLD_ComponentTypeRegistry() { m_NextIndex = 0; }
                ~OLD_ComponentTypeRegistry() { m_Registry.clear(); m_NextIndex = 0; }

                template<class T> void emplace(){
                    m_Registry.emplace(type_id<T>(),m_NextIndex);
                    OLD_ComponentManager::m_ComponentVectors     .emplace(m_NextIndex,std::vector<OLD_ComponentBaseClass*>());
                    OLD_ComponentManager::m_ComponentVectorsScene.emplace(m_NextIndex,std::vector<OLD_ComponentBaseClass*>());
                    ++m_NextIndex;
                }
                template<class T,class V> void emplace(){
                    m_Registry.emplace(type_id<T>(),m_NextIndex);
                    m_Registry.emplace(type_id<V>(),m_NextIndex);
                    OLD_ComponentManager::m_ComponentVectors     .emplace(m_NextIndex,std::vector<OLD_ComponentBaseClass*>());
                    OLD_ComponentManager::m_ComponentVectorsScene.emplace(m_NextIndex,std::vector<OLD_ComponentBaseClass*>());
                    ++m_NextIndex;
                }
                template<class T,class V,class W> void emplace(){
                    m_Registry.emplace(type_id<T>(),m_NextIndex);
                    m_Registry.emplace(type_id<V>(),m_NextIndex);
                    m_Registry.emplace(type_id<W>(),m_NextIndex);
                    OLD_ComponentManager::m_ComponentVectors     .emplace(m_NextIndex,std::vector<OLD_ComponentBaseClass*>());
                    OLD_ComponentManager::m_ComponentVectorsScene.emplace(m_NextIndex,std::vector<OLD_ComponentBaseClass*>());
                    ++m_NextIndex;
                }
        };
        class OLD_ComponentSystemBaseClass{
            friend class ::Engine::epriv::OLD_ComponentManager;
            public:
                OLD_ComponentSystemBaseClass(){}
                virtual ~OLD_ComponentSystemBaseClass(){}
                virtual void update(const float& dt){}
                virtual void onSceneSwap(Scene* oldScene, Scene* newScene, OLD_ComponentBaseClass*, OLD_Entity&) {}
                virtual void onEntityAddedToScene(Scene&, OLD_ComponentBaseClass*, OLD_Entity&) {}
                virtual void onComponentAddedToEntity(OLD_ComponentBaseClass*, OLD_Entity&) {}
        };
        class OLD_ComponentModelSystem final: public OLD_ComponentSystemBaseClass {
            friend class ::Engine::epriv::OLD_ComponentManager;
            private:
                class impl; std::unique_ptr<impl> m_i;
            public:
                OLD_ComponentModelSystem();
                ~OLD_ComponentModelSystem();
                void update(const float& dt);
                void onSceneSwap(Scene* oldScene, Scene* newScene, OLD_ComponentBaseClass*, OLD_Entity&);
                void onEntityAddedToScene(Scene&, OLD_ComponentBaseClass*, OLD_Entity&);
                void onComponentAddedToEntity(OLD_ComponentBaseClass*, OLD_Entity&);
        };
        class OLD_ComponentCameraSystem final : public OLD_ComponentSystemBaseClass {
            friend class ::Engine::epriv::OLD_ComponentManager;
            private:
                class impl; std::unique_ptr<impl> m_i;
            public:
                OLD_ComponentCameraSystem();
                ~OLD_ComponentCameraSystem();
                void update(const float& dt);
                void onSceneSwap(Scene* oldScene, Scene* newScene, OLD_ComponentBaseClass*, OLD_Entity&);
                void onEntityAddedToScene(Scene&, OLD_ComponentBaseClass*, OLD_Entity&);
                void onComponentAddedToEntity(OLD_ComponentBaseClass*, OLD_Entity&);
        };
        class OLD_ComponentBodySystem final : public OLD_ComponentSystemBaseClass {
            friend class ::Engine::epriv::OLD_ComponentManager;
            private:
                class impl; std::unique_ptr<impl> m_i;
            public:
                OLD_ComponentBodySystem();
                ~OLD_ComponentBodySystem();
                void update(const float& dt);
                void onSceneSwap(Scene* oldScene, Scene* newScene, OLD_ComponentBaseClass*, OLD_Entity&);
                void onEntityAddedToScene(Scene&, OLD_ComponentBaseClass*, OLD_Entity&);
                void onComponentAddedToEntity(OLD_ComponentBaseClass*, OLD_Entity&);
        };
    };
};

class OLD_ComponentModel: public OLD_ComponentBaseClass{
    friend class  Engine::epriv::OLD_ComponentManager;
    friend class  Engine::epriv::OLD_ComponentModelSystem;
    friend class  Engine::epriv::OLD_ComponentInternalFunctionality;
    friend struct Engine::epriv::InternalComponentPublicInterface;
    friend class ::OLD_ComponentBody;
    private:
        std::vector<MeshInstance*> models;
        float _radius;
        glm::vec3 _radiusBox;
    public:
        BOOST_TYPE_INDEX_REGISTER_CLASS
        OLD_ComponentModel(Handle& meshHandle, Handle& materialHandle, OLD_Entity*, ShaderP* = 0,RenderStage::Stage = RenderStage::GeometryOpaque);
        OLD_ComponentModel(Mesh*, Handle& materialHandle, OLD_Entity*, ShaderP* = 0, RenderStage::Stage = RenderStage::GeometryOpaque);
        OLD_ComponentModel(Handle& meshHandle, Material*, OLD_Entity*, ShaderP* = 0, RenderStage::Stage = RenderStage::GeometryOpaque);
        OLD_ComponentModel(Mesh*, Material*, OLD_Entity*, ShaderP* = 0, RenderStage::Stage = RenderStage::GeometryOpaque);

        OLD_ComponentModel(Handle& meshHandle, Handle& materialHandle, OLD_Entity*, Handle& shaderPHandle, RenderStage::Stage = RenderStage::GeometryOpaque);
        OLD_ComponentModel(Mesh*, Handle& materialHandle, OLD_Entity*, Handle& shaderPHandle, RenderStage::Stage = RenderStage::GeometryOpaque);
        OLD_ComponentModel(Handle& meshHandle, Material*, OLD_Entity*, Handle& shaderPHandle, RenderStage::Stage = RenderStage::GeometryOpaque);
        OLD_ComponentModel(Mesh*, Material*, OLD_Entity*, Handle& shaderPHandle, RenderStage::Stage = RenderStage::GeometryOpaque);

        ~OLD_ComponentModel();

        uint getNumModels();
        float radius();
        glm::vec3 boundingBox();
        void show();
        void hide();

        MeshInstance* getModel(uint index = 0);

        uint addModel(Handle& meshHandle, Handle& materialHandle, ShaderP* = 0, RenderStage::Stage = RenderStage::GeometryOpaque);
        uint addModel(Mesh*,Material*, ShaderP* = 0, RenderStage::Stage = RenderStage::GeometryOpaque);

        void setModel(Handle& meshHandle,Handle& materialHandle,uint index, ShaderP* = 0, RenderStage::Stage = RenderStage::GeometryOpaque);
        void setModel(Mesh*,Material*,uint index, ShaderP* = 0, RenderStage::Stage = RenderStage::GeometryOpaque);

        void setModelMesh(Mesh*,uint index, RenderStage::Stage = RenderStage::GeometryOpaque);
        void setModelMesh(Handle& meshHandle, uint index, RenderStage::Stage = RenderStage::GeometryOpaque);
        
        void setModelMaterial(Material*,uint index, RenderStage::Stage = RenderStage::GeometryOpaque);
        void setModelMaterial(Handle& materialHandle,uint index, RenderStage::Stage = RenderStage::GeometryOpaque);

        void setModelShaderProgram(ShaderP*, uint index, RenderStage::Stage = RenderStage::GeometryOpaque);
        void setModelShaderProgram(Handle& materialHandle, uint index, RenderStage::Stage = RenderStage::GeometryOpaque);

        bool rayIntersectSphere(OLD_ComponentCamera& camera);

        template<class T> void setCustomBindFunctor  (T& functor,uint index = 0){ models[index]->setCustomBindFunctor(functor); }
        template<class T> void setCustomUnbindFunctor(T& functor,uint index = 0){ models[index]->setCustomUnbindFunctor(functor); }
};

class OLD_ComponentBody: public OLD_ComponentBaseClass{
    friend class  Engine::epriv::OLD_ComponentManager;
    friend class  Engine::epriv::OLD_ComponentBodySystem;
    friend struct Engine::epriv::InternalComponentPublicInterface;
    friend class ::OLD_ComponentModel;
    private:
        struct PhysicsData{
            Collision* collision;
            btRigidBody rigidBody;
            btDefaultMotionState motionState;
            float mass;
            PhysicsData():rigidBody(0,0,0){
                collision = 0; mass = 0;
            }
        };
        struct NormalData{
            glm::vec3 scale;
            glm::vec3 position;
            glm::quat rotation;
            glm::mat4 modelMatrix;
            NormalData() {
            }
        };
        union{
            NormalData* n;
            PhysicsData* p;
        } data;
        bool _physics;
        glm::vec3 _forward, _right, _up;
    public:
        BOOST_TYPE_INDEX_REGISTER_CLASS
        OLD_ComponentBody();
        OLD_ComponentBody(CollisionType::Type);
        ~OLD_ComponentBody();

        void alignTo(glm::vec3 direction,float speed);

        void translate(glm::vec3 translation,bool local = true);   void translate(float x,float y,float z,bool local = true);
        void rotate(glm::vec3 rotation,bool local = true);         void rotate(float pitch,float yaw,float roll,bool local = true);
        void scale(glm::vec3 amount);                              void scale(float x,float y,float z);

        void setPosition(glm::vec3 newPosition);                   void setPosition(float x,float y,float z);
        void setRotation(glm::quat newRotation);                   void setRotation(float x,float y,float z,float w);
        void setScale(glm::vec3 newScale);                         void setScale(float x,float y,float z);

        float mass();
        glm::vec3 getScreenCoordinates();
        glm::quat rotation();
        glm::vec3 getScale();
        glm::vec3 position();
        glm::vec3 forward();
        glm::vec3 right();
        glm::vec3 up();
        glm::vec3 getLinearVelocity();
        glm::vec3 getAngularVelocity();
        glm::mat4 modelMatrix();
        btRigidBody& getBody();

        void setCollision(CollisionType::Type,float mass);
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

class OLD_ComponentCamera: public OLD_ComponentBaseClass{
    friend class  Engine::epriv::OLD_ComponentManager;
    friend class  Engine::epriv::OLD_ComponentCameraSystem;
    friend class  Engine::epriv::OLD_ComponentInternalFunctionality;
    friend struct Engine::epriv::InternalComponentPublicInterface;
    friend class ::OLD_ComponentModel;
    friend class ::Camera;
    private:
        enum Type{ Perspective, Orthographic, };
        Type _type;
        glm::vec3 _eye,_up;
        glm::mat4 _viewMatrix, _viewMatrixNoTranslation, _projectionMatrix;
        glm::vec4 _planes[6];
        float _nearPlane,  _farPlane,  _bottom,  _top;
        union{ float _angle;        float _left;  };
        union{ float _aspectRatio;  float _right; };
    public:
        BOOST_TYPE_INDEX_REGISTER_CLASS
        OLD_ComponentCamera();
        OLD_ComponentCamera(float angle,float aspectRatio,float nearPlane,float farPlane);
        OLD_ComponentCamera(float left,float right,float bottom,float top,float nearPlane,float farPlane);
        ~OLD_ComponentCamera();

        virtual void update(const float& dt);
        void resize(uint width,uint height);
        void lookAt(glm::vec3 eye,glm::vec3 forward,glm::vec3 up);
        
        glm::mat4 getProjection();
        glm::mat4 getProjectionInverse();
        glm::mat4 getView();
        glm::mat4 getViewInverse();
        glm::mat4 getViewProjection();
        glm::mat4 getViewProjectionInverse();
        glm::vec3 getViewVector();

        float getAngle();    void setAngle(float);
        float getAspect();   void setAspect(float);
        float getNear();     void setNear(float);
        float getFar();      void setFar(float);

        uint pointIntersectTest(glm::vec3& objectPosition);
        uint sphereIntersectTest(glm::vec3& objectPosition,float objectRadius);
};

class OLD_Entity: public EventObserver{
    friend class ::Scene;
    friend class ::Engine::epriv::OLD_ComponentManager;
    private:
        Scene* m_Scene;
        uint ID;
        std::vector<uint> m_Components;
    public:
        OLD_Entity();
        virtual ~OLD_Entity();

        const uint id() const;
        Scene* scene();
        virtual void update(const float& dt){}

        void destroy();

        template<class T> T* getComponent(){
            const uint& slot = Engine::epriv::OLD_ComponentTypeRegistry::slot(type_id<T>());
            const uint& componentID = m_Components.at(slot);
            if(componentID == 0){
                return nullptr;
            }
            return Engine::epriv::OLD_ComponentManager::m_ComponentPool->getAsFast<T>(componentID);
        }
        template<class T> void addComponent(T* component){
            const uint& slot = Engine::epriv::OLD_ComponentTypeRegistry::slot(type_id<T>());
            uint& componentID = m_Components.at(slot);
            if(componentID != 0) return;
            uint generatedID = Engine::epriv::OLD_ComponentManager::m_ComponentPool->add(component);
            Engine::epriv::OLD_ComponentManager::m_ComponentVectors.at(slot).push_back(component);
            component->m_Owner = ID;
            componentID = generatedID;

            Engine::epriv::OLD_ComponentManager::onComponentAddedToEntity(*this);
            if (m_Scene) {
                Engine::epriv::OLD_ComponentManager::m_ComponentVectorsScene.at(slot).push_back(component);
            }
        }
        template<class T> void removeComponent(T* component){
            const uint& slot = Engine::epriv::OLD_ComponentTypeRegistry::slot(type_id<T>());
            uint& componentID = m_Components.at(slot);
            if(componentID == 0) return;
            component->m_Owner = 0;
            removeFromVector(Engine::epriv::OLD_ComponentManager::m_ComponentVectors.at(slot),component);
            removeFromVector(Engine::epriv::OLD_ComponentManager::m_ComponentVectorsScene.at(slot),component);
            Engine::epriv::OLD_ComponentManager::m_ComponentPool->remove(componentID);
            componentID = 0;
        }
};
#endif