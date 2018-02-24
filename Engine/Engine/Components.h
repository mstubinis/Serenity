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



		class ComponentBaseClass{
			friend class ::Engine::epriv::ComponentManager;
		    protected:
				Object* m_Owner;
		    public:
				ComponentBaseClass(Object* = nullptr);
				virtual ~ComponentBaseClass();

				void setOwner(Object*);
		};

	};
};

class ComponentModel: public Engine::epriv::ComponentBaseClass{
	friend class ::Engine::epriv::ComponentManager;
    private:
		std::vector<Engine::epriv::MeshMaterialPair> models;
    public:
		ComponentModel(Mesh*,Material*,Object* = nullptr);
		~ComponentModel();
};


class ComponentTransform: public Engine::epriv::ComponentBaseClass{
	friend class ::Engine::epriv::ComponentManager;
    private:
		glm::mat4 modelMatrix;
		glm::vec3 position, scale;
		glm::quat rotation;
    public:
		ComponentTransform(Object* = nullptr);
		~ComponentTransform();
};

#endif