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
class Camera;
class Object{
	private:
		glm::vec3 _calculateForward(); 
		glm::vec3 _calculateRight(); 
		glm::vec3 _calculateUp();
	protected:
		std::string m_Name;
		glm::mat4 m_Model;
		glm::quat m_Orientation;
		glm::vec3 m_Scale, m_Forward, m_Right, m_Up;
		glm::vec3 m_Position;

		Object* m_Parent;
		std::vector<Object*> m_Children;
	public:
		Object(
			    glm::vec3 = glm::vec3(0,0,0),   //Position
			    glm::vec3 = glm::vec3(1,1,1),   //Scale
			    std::string = "Object",         //Object
				bool isNotLight = true,
				Scene* = nullptr                //The scene to add the object to (default nullptr = the current scene)
			  );
		~Object();

		float getDistance(Object*);
		unsigned long long getDistanceLL(Object*);
		glm::vec3 getScreenCoordinates();

		virtual void translate(float,float,float,bool local=true); 
		virtual void translate(glm::vec3,bool local=true);
		virtual void rotate(float,float,float); 
		virtual void rotate(glm::vec3);
		virtual void scale(float,float,float); 
		virtual void scale(glm::vec3);

		virtual void setPosition(float,float,float); 
		virtual void setPosition(glm::vec3);
		virtual void setScale(float,float,float); 
		virtual void setScale(glm::vec3);

		void addChild(Object*);

		virtual void update(float);
		virtual void _updateMatrix();
		virtual void render(Mesh*,Material*,bool=false);
		virtual void render(bool=false);

		const glm::quat& getOrientation(){ return m_Orientation; }
		const glm::vec3 getPosition() const{ return glm::vec3(m_Model[3][0],m_Model[3][1],m_Model[3][2]); }
		const glm::vec3& getScale() const{ return m_Scale; }
		const glm::vec3& getForward() const{ return m_Forward; }
		const glm::vec3& getRight() const{ return m_Right; }
		const glm::vec3& getUp() const{ return m_Up; }
		const glm::mat4& getModel() const{ return m_Model; }
		const std::string getName() const{ return m_Name; }
		const Object* getParent() const{ return m_Parent; }
		const std::vector<Object*> getChildren() const{ return m_Children; }

		virtual void setName(std::string);

		virtual bool rayIntersectSphere(Camera*);
};
#endif