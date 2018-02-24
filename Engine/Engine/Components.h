#pragma once
#ifndef ENGINE_OBJECT_COMPONENTS_H
#define ENGINE_OBJECT_COMPONENTS_H

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <memory>

typedef unsigned int uint;

class Object;

namespace Engine{
	namespace epriv{
		class ComponentManager final{
		    private:
				class impl;
		    public:
				std::unique_ptr<impl> m_i;

				ComponentManager(const char* name, uint w, uint h);
				~ComponentManager();

				void _init(const char* name, uint w, uint h);
				void _update(float&);
		};



		class ComponentBaseClass{
		    protected:
				Object* m_Owner;
		    public:
				ComponentBaseClass(Object* = nullptr);
				virtual ~ComponentBaseClass();

				void setOwner(Object*);

				virtual void update(float&);

		};
	};
};


class ComponentTransform: public Engine::epriv::ComponentBaseClass{
    private:
		glm::mat4 model;
		glm::vec3 position, scale;
		glm::quat rotation;
    public:
		ComponentTransform(Object* = nullptr);
		~ComponentTransform();

		void update(float&);
};


#endif