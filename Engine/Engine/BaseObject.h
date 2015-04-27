#ifndef BASEOBJECT_H
#define BASEOBJECT_H

#include <string>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class BaseObject{
	private:
		void _calculateForward(); 
		void _calculateRight(); 
		void _calculateUp();
	protected:
		glm::vec3 m_Position;
		glm::quat m_Orientation;
		glm::mat4 m_Model;

		glm::vec3 m_Forward, m_Right, m_Up;

		BaseObject* m_Parent;
		std::vector<BaseObject*> m_Children;
		std::string m_Name;
	public:
		BaseObject(glm::vec3,std::string);
		~BaseObject();

		void flagAsChanged();
		void setName(std::string);

		float getDistance(BaseObject*);
		unsigned long long getDistanceLL(BaseObject*);
		glm::vec3 getScreenCoordinates();

		virtual void addChild(BaseObject*);
		virtual void update(float);

		virtual void translate(float,float,float,bool local=true); 
		virtual void translate(glm::vec3,bool local=true);
		virtual void rotate(float,float,float); 
		virtual void rotate(glm::vec3);

		virtual void pitch(float); 
		virtual void yaw(float); 
		virtual void roll(float);

		virtual void setPosition(float,float,float); 
		virtual void setPosition(glm::vec3);

		const glm::mat4& getModel() const{ return m_Model; }
		const glm::vec3& getForward() const{ return m_Forward; }
		const glm::vec3& getRight() const{ return m_Right; }
		const glm::vec3& getUp() const{ return m_Up; }
		const glm::vec3& getPosition() const{ return glm::vec3(m_Model[3][0],m_Model[3][1],m_Model[3][2]); }
		const BaseObject* getParent() const{ return m_Parent; }
		const std::vector<BaseObject*>& getChildren() const{ return m_Children; }
		const glm::quat& getOrientation(){ return m_Orientation; }
		const std::string& getName() const{ return m_Name; }
		
};
#endif