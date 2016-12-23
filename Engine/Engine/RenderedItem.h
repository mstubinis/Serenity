#pragma once
#ifndef RENDERED_ITEM_H
#define RENDERED_ITEM_H

#include <memory>
#include "Engine_Math.h"
#include "Engine_ResourceBasic.h"

class Mesh;
class Material;

class RenderedItem final: public EngineResource{
	private:
		class impl;
		std::unique_ptr<impl> m_i;
	public:
		RenderedItem(boost::shared_ptr<std::string>& parentNamePtr,Mesh*,Material*,glm::vec3& = glm::vec3(0),glm::quat& = glm::quat(),glm::vec3& = glm::vec3(1.0));
		RenderedItem(boost::shared_ptr<std::string>& parentNamePtr,std::string mesh,std::string mat,glm::vec3& = glm::vec3(0),glm::quat& = glm::quat(),glm::vec3& = glm::vec3(1.0));
		~RenderedItem();

		Mesh* mesh();
		Material* material();
		glm::mat4& model();
		glm::vec3& position();
		glm::quat& orientation();
		glm::vec3& getScale();
		std::string& parent();
		boost::weak_ptr<std::string>& parentPtr();

		void bind();
		void unbind();

		void setOrientation(glm::quat&);
		void setOrientation(float x,float y,float z);

		void setMesh(std::string);
		void setMesh(Mesh*);
		void setMaterial(std::string);
		void setMaterial(Material*);

		void setPosition(float x,float y,float z);
		void setScale(float x,float y,float z);
		void setPosition(glm::vec3&);
		void setScale(glm::vec3&);

		void translate(float x,float y,float z);
		void rotate(float pitch,float yaw,float roll);
		void scale(float x,float y,float z);

		void translate(glm::vec3&);
		void rotate(glm::vec3&);
		void scale(glm::vec3&);

		void update(float dt);

		template<class T> void setCustomBindFunctor(T& functor);
		template<class T> void setCustomUnbindFunctor(T& functor);
};
#endif