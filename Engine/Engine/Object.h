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
class Object{
	private:
		glm::vec3 _Forward(); 
		glm::vec3 _Right(); 
		glm::vec3 _Up();
	protected:
		bool m_Changed;

		std::string m_Name;
		glm::mat4 m_WorldMatrix, m_Model;
		glm::quat m_Orientation;
		glm::vec3 m_Position, m_Scale, m_Color, m_Forward, m_Right, m_Up;
		float m_Radius;
		glm::vec3 m_BoundingBoxRadius;

		Object* m_Parent;
		std::vector<Object*> m_Children;

		Mesh* m_Mesh;
		Material* m_Material;

		virtual void m_Calculate_Radius();

	public:
		Object(
				std::string = "",
				std::string = "",
			    glm::vec3 = glm::vec3(0,0,0),   //Position
			    glm::vec3 = glm::vec3(1,1,1),   //Scale
			    std::string = "Object",         //Object
				bool addToResources = true      //Add this to the resource generic object pool (Don't do for lights)
			  );
		~Object();

		void Flag_As_Changed();

		float Distance(Object*);

		virtual void Translate(float,float,float,bool local=true); virtual void Translate(glm::vec3&,bool local=true);
		virtual void Rotate(float,float,float); virtual void Rotate(glm::vec3&);
		virtual void Scale(float,float,float); virtual void Scale(glm::vec3&);
		virtual void Pitch(float); void Yaw(float); virtual void Roll(float);

		virtual void Set_Position(float,float,float); virtual void Set_Position(glm::vec3);
		virtual void Set_Scale(float,float,float); virtual void Set_Scale(glm::vec3&);
		virtual void Set_Color(float,float,float); virtual void Set_Color(glm::vec3&);

		void Add_Child(Object*);

		virtual void Update(float);
		virtual void Render(Mesh*, Material*,bool=false);
		virtual void Render(bool=false);

		#pragma region Getters
		const glm::quat& Orientation(){ return m_Orientation; }
		const glm::vec3  Position() const{ return glm::vec3(m_Model[3][0],m_Model[3][1],m_Model[3][2]); }
		const glm::vec3& Scale() const{ return m_Scale; }
		const glm::vec3& Forward() const{ return m_Forward; }
		const glm::vec3& Right() const{ return m_Right; }
		const glm::vec3& Up() const{ return m_Up; }
		virtual const glm::vec3& RadiusBox() const{ return m_BoundingBoxRadius; }
		virtual const float Radius() const { return m_Radius; }
		const glm::vec3& Color() const{ return m_Color; }
		const glm::mat4& Model() const{ return m_Model; }
		const glm::mat4& World() const{ return m_WorldMatrix; }
		const std::string Name() const{ return m_Name; }
		const Object* Parent() const{ return m_Parent; }
		const std::vector<Object*> Children() const{ return m_Children; }
		#pragma endregion

		#pragma region Setters
		void Set_Mesh(Mesh*);
		void Set_Material(Material*);
		#pragma endregion
};
#endif