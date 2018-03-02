#pragma once
#ifndef ENGINE_OBJECT_COMPONENTS_H
#define ENGINE_OBJECT_COMPONENTS_H

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <memory>
#include <vector>

typedef unsigned int uint;

class Entity;
class Object;
class Mesh;
class Material;
class Collision;
class btRigidBody;
struct btDefaultMotionState;
class btVector3;
struct Handle;

class ComponentCamera;
class ComponentBaseClass;

namespace Engine{
	namespace epriv{
		const uint MAX_NUM_ENTITIES = 32768;

		struct MeshMaterialPair;
		class ComponentManager final{
		    private:
				class impl;
		    public:
				std::unique_ptr<impl> m_i;

				ComponentManager(const char* name, uint w, uint h);
				~ComponentManager();

				void _init(const char* name, uint w, uint h);
				void _update(float&);
				void _render();

				Handle _addComponent(ComponentBaseClass* component,uint type);
				ComponentBaseClass* _getComponent(uint index);
				void _removeComponent(uint index);
		};
	};
};

class ComponentType{public:enum Type{
	Transform,
	Model,
	Physics,
	Camera,
	Collision,

	_TOTAL,
};};

class ComponentBaseClass{
	friend class ::Engine::epriv::ComponentManager;
	protected:
		Entity* m_Owner; //eventually make this an entity ID instead?
	public:
		ComponentBaseClass(Entity* = nullptr);
		ComponentBaseClass(uint entityID);
		virtual ~ComponentBaseClass();

		void setOwner(Entity*);
		void setOwner(uint entityID);
};


class ComponentModel: public ComponentBaseClass{
	friend class ::Engine::epriv::ComponentManager;
    private:
		std::vector<Engine::epriv::MeshMaterialPair> models;
		float _radius;
    public:
		ComponentModel(Entity* owner,Mesh*,Material*);
		~ComponentModel();

		uint addModel(Handle& meshHandle, Handle& materialHandle);
		uint addModel(Mesh*,Material*);

		void setModel(Handle& meshHandle,Handle& materialHandle,uint index);
		void setModel(Mesh*,Material*,uint index);

		void setModelMesh(Mesh*,uint index);
		void setModelMesh(Handle& meshHandle, uint index);
		
		void setModelMaterial(Material*,uint index);
		void setModelMaterial(Handle& materialHandle,uint index);

		bool rayIntersectSphere(ComponentCamera*);
};


class ComponentTransform: public ComponentBaseClass{
	friend class ::Engine::epriv::ComponentManager;
    private:
		glm::mat4 _modelMatrix;
		glm::vec3 _position, _scale, _forward, _right, _up;
		glm::quat _rotation;
    public:
		ComponentTransform(Entity* owner);
		~ComponentTransform();

		void translate(glm::vec3& translation); void translate(float x,float y,float z);
		void rotate(glm::vec3& rotation); void rotate(float pitch,float yaw,float roll);
		void scale(glm::vec3& amount); void scale(float x,float y,float z);

		void setPosition(glm::vec3& newPosition); void setPosition(float x,float y,float z);
		void setRotation(glm::quat& newRotation); void setRotation(float x,float y,float z,float w);
		void setScale(glm::vec3& newScale); void setScale(float x,float y,float z);
};

class ComponentPhysics: public ComponentBaseClass{
	friend class ::Engine::epriv::ComponentManager;
    private:
		Collision* _collision;
		btRigidBody* _rigidBody;
		float _mass;
    public:
		ComponentPhysics(Entity* owner);
		~ComponentPhysics();

		void setDynamic(bool dynamic);
		void setMass(float mass);

		void setLinearVelocity(float x,float y,float z,bool local = true); void setLinearVelocity(glm::vec3 velocity,bool local = true);
		void setAngularVelocity(float x,float y,float z,bool local = true); void setAngularVelocity(glm::vec3 velocity,bool local = true);
};


class ComponentCamera: public ComponentBaseClass{
	friend class ::Engine::epriv::ComponentManager;
    private:
		glm::mat4 _viewMatrix,  _projectionMatrix;
		glm::vec4 _planes[6];
		float _nearPlane,  _farPlane,  _bottom,  _top;
		union{ float _angle;        float _left;  };
		union{ float _aspectRatio;  float _right; };
    public:
		ComponentCamera(Entity* owner);
		ComponentCamera(Entity* owner,float angle,float aspectRatio,float nearPlane,float farPlane);
		ComponentCamera(Entity* owner,float left,float right,float bottom,float top,float nearPlane,float farPlane);
		~ComponentCamera();

		void update();
		void lookAt(glm::vec3 eye,glm::vec3 forward,glm::vec3 up);
		glm::vec3 viewVector();
};


#endif