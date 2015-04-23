#ifndef OBJECT_H
#define OBJECT_H

#include "Engine_Renderer.h"
#include <string>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class Mesh;
class Material;
class Scene;
class Object{
	private:
		glm::vec3 _calculateForward(); 
		glm::vec3 _calculateRight(); 
		glm::vec3 _calculateUp();
	protected:
		bool m_Changed;

		std::string m_Name;
		glm::mat4 m_WorldMatrix;
		glm::mat4 m_Model;
		glm::quat m_Orientation;
		glm::vec3 m_Scale, m_Forward, m_Right, m_Up;
		glm::vec4 m_Color;
		glm::vec3 m_Position;
		float m_Radius;
		glm::vec3 m_BoundingBoxRadius;

		Object* m_Parent;
		std::vector<Object*> m_Children;

		Mesh* m_Mesh;
		Material* m_Material;

		virtual void calculateRadius();

	public:
		Object(
				std::string = "",
				std::string = "",
			    glm::vec3 = glm::vec3(0,0,0),   //Position
			    glm::vec3 = glm::vec3(1,1,1),   //Scale
			    std::string = "Object",         //Object
				bool isNotLight = true,
				Scene* = nullptr                //The scene to add the object to (default nullptr = the current scene)
			  );
		~Object();

		void flagAsChanged();

		float getDistance(Object*);
		unsigned long long getDistanceLL(Object*);
		glm::vec3 getScreenCoordinates();

		virtual void translate(float,float,float,bool local=true); 
		virtual void translate(glm::vec3,bool local=true);
		virtual void rotate(float,float,float); 
		virtual void rotate(glm::vec3);
		virtual void scale(float,float,float); 
		virtual void scale(glm::vec3);
		virtual void pitch(float); 
		virtual void yaw(float); 
		virtual void roll(float);

		virtual void setPosition(float,float,float); 
		virtual void setPosition(glm::vec3);
		virtual void setScale(float,float,float); 
		virtual void setScale(glm::vec3);
		virtual void setColor(float,float,float,float); 
		virtual void setColor(glm::vec4);

		void addChild(Object*);

		virtual void update(float);
		virtual void render(Mesh*, Material*,bool=false);
		virtual void render(bool=false);

		#pragma region Getters
		const bool hasChanged() const { return m_Changed; }
		const glm::quat& getOrientation(){ return m_Orientation; }
		const glm::vec3 getPosition() const{ return glm::vec3(m_Model[3][0],m_Model[3][1],m_Model[3][2]); }
		const glm::vec3& getScale() const{ return m_Scale; }
		const glm::vec3& getForward() const{ return m_Forward; }
		const glm::vec3& getRight() const{ return m_Right; }
		const glm::vec3& getUp() const{ return m_Up; }
		virtual const glm::vec3& getRadiusBox() const{ return m_BoundingBoxRadius; }
		virtual const float getRadius() const { return m_Radius; }
		const glm::vec4& getColor() const{ return m_Color; }
		const glm::mat4& getModel() const{ return m_Model; }
		const glm::mat4& getWorld() const{ return m_WorldMatrix; }
		const std::string getName() const{ return m_Name; }
		const Object* getParent() const{ return m_Parent; }
		const std::vector<Object*> getChildren() const{ return m_Children; }
		#pragma endregion

		#pragma region Setters
		void setMesh(Mesh*);
		void setMaterial(Material*);
		virtual void setName(std::string);
		#pragma endregion
};
#endif