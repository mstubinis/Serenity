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

namespace glm{

	#ifdef ENGINE_PRECISION_NORMAL
	typedef float nType;
	typedef glm::detail::tvec3<float> v3;
	typedef glm::detail::tmat4x4<float> m4;
	#endif
	#ifdef ENGINE_PRECISION_HIGH
	typedef double nType;
	typedef glm::detail::tvec3<nType> v3;
	typedef glm::detail::tmat4x4<nType> m4;
	#endif
	#ifdef ENGINE_PRECISION_VERY_HIGH
	typedef long double nType;
	typedef glm::detail::tvec3<long double> v3;
	typedef glm::detail::tmat4x4<long double> m4;
	#endif

	typedef glm::detail::tvec3<float> v3_f;
	typedef glm::detail::tmat4x4<float> m4_f;
	typedef glm::detail::tvec3<nType> v3_d;
	typedef glm::detail::tmat4x4<nType> m4_d;
	typedef glm::detail::tvec3<long double> v3_ld;
	typedef glm::detail::tmat4x4<long double> m4_ld;
};

class Object{
	private:
		glm::v3 _calculateForward(); 
		glm::v3 _calculateRight(); 
		glm::v3 _calculateUp();
	protected:
		std::string m_Name;
		glm::v3 m_Position;
		glm::m4 m_Model;
		glm::quat m_Orientation;
		glm::vec3 m_Scale;
		glm::v3 m_Forward, m_Right, m_Up;
		glm::v3 _prevPosition;

		Object* m_Parent;
		float m_Radius;
		std::vector<Object*> m_Children;
	public:
		Object(
			    glm::v3 = glm::v3(0,0,0), //Position
			    glm::vec3 = glm::vec3(1,1,1),   //Scale
			    std::string = "Object",         //Object
				Scene* = nullptr                //The scene to add the object to (default nullptr = the current scene)
			  );
		virtual ~Object();

		float getDistance(Object*);
		unsigned long long getDistanceLL(Object*);

		glm::vec3 getScreenCoordinates();

		virtual void alignTo(glm::v3,float speed=0,bool overTime=false);

		virtual void translate(glm::nType,glm::nType,glm::nType,bool local=true); 
		virtual void translate(glm::v3,bool local=true);
		virtual void rotate(float,float,float,bool overTime = true); 
		virtual void rotate(glm::vec3, bool overTime = true);
		virtual void scale(float,float,float); 
		virtual void scale(glm::vec3);

		virtual void setPosition(glm::nType,glm::nType,glm::nType); 
		virtual void setPosition(glm::v3);
		virtual void setScale(float,float,float); 
		virtual void setScale(glm::vec3);

		void addChild(Object*);

		virtual void update(float);
		virtual void render(GLuint=0,bool=false);
		virtual void draw(GLuint shader,bool=false);

		virtual const float getRadius() const { return m_Radius; }

		const glm::quat& getOrientation(){ return m_Orientation; }
		const glm::v3 getPosition(){ return glm::v3(m_Model[3][0],m_Model[3][1],m_Model[3][2]); }
		const glm::vec3& getScale() const{ return m_Scale; }
		const glm::v3& getForward() const{ return m_Forward; }
		const glm::v3& getRight() const{ return m_Right; }
		const glm::v3& getUp() const{ return m_Up; }
		const glm::m4& getModel() const{ return m_Model; }
		const std::string& getName() const{ return m_Name; }
		const glm::v3 getMotionVector() { return getPosition() - _prevPosition; }
		Object* getParent() const{ return m_Parent; }
		const std::vector<Object*> getChildren() const{ return m_Children; }

		virtual void setName(std::string);

		virtual bool rayIntersectSphere(Camera*);
		virtual bool rayIntersectSphere(glm::v3 origin, glm::vec3 vector);
};
#endif