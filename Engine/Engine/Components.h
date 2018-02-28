#pragma once
#ifndef ENGINE_OBJECT_COMPONENTS_H
#define ENGINE_OBJECT_COMPONENTS_H

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <memory>
#include <vector>

typedef unsigned int uint;

class Object;
class Mesh;
class Material;
struct Handle;

namespace Engine{
	namespace epriv{
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
		};
	};
};
class ComponentBaseClass{
	friend class ::Engine::epriv::ComponentManager;
	protected:
		Object* m_Owner; //eventually make this an entity ID instead?
	public:
		ComponentBaseClass(Object* = nullptr);
		virtual ~ComponentBaseClass();

		void setOwner(Object*);
};


class ComponentModel: public ComponentBaseClass{
	friend class ::Engine::epriv::ComponentManager;
    private:
		std::vector<Engine::epriv::MeshMaterialPair> models;
    public:
		ComponentModel(Mesh*,Material*,Object* = nullptr);
		~ComponentModel();

		uint addModel(Handle& meshHandle, Handle& materialHandle);
		uint addModel(Mesh*,Material*);

		void setModel(Handle& meshHandle,Handle& materialHandle,uint index);
		void setModel(Mesh*,Material*,uint index);

		void setModelMesh(Mesh*,uint index);
		void setModelMesh(Handle& meshHandle, uint index);
		
		void setModelMaterial(Material*,uint index);
		void setModelMaterial(Handle& materialHandle,uint index);
};


class ComponentTransform: public ComponentBaseClass{
	friend class ::Engine::epriv::ComponentManager;
    private:
		glm::mat4 _modelMatrix;
		glm::vec3 _position, _scale, _forward, _right, _up;
		glm::quat _rotation;
    public:
		ComponentTransform(Object* = nullptr);
		~ComponentTransform();

		void translate(glm::vec3& translation); void translate(float x,float y,float z);
		void rotate(glm::vec3& rotation); void rotate(float pitch,float yaw,float roll);
		void scale(glm::vec3& amount); void scale(float x,float y,float z);

		void setPosition(glm::vec3& newPosition); void setPosition(float x,float y,float z);
		void setRotation(glm::quat& newRotation); void setRotation(float x,float y,float z,float w);
		void setScale(glm::vec3& newScale); void setScale(float x,float y,float z);
};

#endif