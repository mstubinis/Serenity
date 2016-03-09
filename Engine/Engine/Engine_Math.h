#ifndef ENGINE_MATH_H
#define ENGINE_MATH_H

#include "Engine_Renderer.h"
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <bullet/btBulletCollisionCommon.h>
#include <bullet/btBulletDynamicsCommon.h>

namespace glm{

	#ifdef ENGINE_PRECISION_NORMAL
	typedef float nType;
	typedef glm::detail::tvec3<float> v3;
	typedef glm::detail::tmat4x4<float> m4;
	#endif
	#ifdef ENGINE_PRECISION_HIGH
	typedef double nType;
	typedef glm::detail::tvec3<double> v3;
	typedef glm::detail::tmat4x4<double> m4;
	#endif
	#ifdef ENGINE_PRECISION_VERY_HIGH
	typedef long double nType;
	typedef glm::detail::tvec3<long double> v3;
	typedef glm::detail::tmat4x4<long double> m4;
	#endif

	typedef glm::detail::tvec3<float> v3_f;
	typedef glm::detail::tmat4x4<float> m4_f;
	typedef glm::detail::tvec3<double> v3_d;
	typedef glm::detail::tmat4x4<double> m4_d;
	typedef glm::detail::tvec3<long double> v3_ld;
	typedef glm::detail::tmat4x4<long double> m4_ld;
};

namespace Engine{
	namespace Math{
		static glm::v3 getForward(glm::quat& q){
			return glm::normalize(glm::v3(2*(q.x*q.z+q.w*q.y),2*(q.y*q.x-q.w*q.x),1-2*(q.x*q.x+q.y*q.y)));
		}
		static glm::v3 getRight(glm::quat& q){
			return glm::normalize(glm::v3(1-2*(q.y*q.y+q.z*q.z),2*(q.x*q.y+q.w*q.z),2*(q.x*q.z-q.w*q.y)));
		}
		static glm::v3 getUp(glm::quat& q){
			return glm::normalize(glm::v3(2*(q.x*q.y-q.w*q.z),1-2*(q.x*q.x+q.z*q.z),2*(q.y*q.z+q.w*q.x)));
		}
		static glm::v3 getColumnVector(const btRigidBody* b, unsigned int column){
			btTransform t;
			b->getMotionState()->getWorldTransform(t);
			btVector3 v = t.getBasis().getColumn(column);
			return glm::v3(v.x(),v.y(),v.z());
		}
		static glm::v3 getForward(const btRigidBody* b){ return getColumnVector(b,2); }
		static glm::v3 getRight(const btRigidBody* b){ return getColumnVector(b,0); }
		static glm::v3 getUp(const btRigidBody* b){ return getColumnVector(b,1); }

		static float getAngleBetweenTwoVectors(glm::vec3 a, glm::vec3 b, bool degrees = true){
			if(degrees == true)
				return glm::acos(glm::dot(a,b)) * 57.2958f;
			return glm::acos(glm::dot(a,b));
		}
		static void alignTo(glm::quat& o, glm::vec3 direction,float speed=0, bool overTime=false){
			glm::quat original(o);

			glm::vec3 forward = glm::normalize(glm::vec3(getForward(o)));
			direction = -glm::normalize(direction);

			glm::vec3 xaxis = glm::normalize(glm::cross(glm::vec3(0,1,0), direction));
			glm::vec3 yaxis = glm::normalize(glm::cross(direction, xaxis));

			glm::mat3 rot;
			rot[0][0] = static_cast<float>(xaxis.x);
			rot[1][0] = static_cast<float>(yaxis.x);
			rot[2][0] = static_cast<float>(direction.x);
			rot[0][1] = static_cast<float>(xaxis.y);
			rot[1][1] = static_cast<float>(yaxis.y);
			rot[2][1] = static_cast<float>(direction.y);
			rot[0][2] = static_cast<float>(xaxis.z);
			rot[1][2] = static_cast<float>(yaxis.z);
			rot[2][2] = static_cast<float>(direction.z);
			o = glm::quat_cast(rot);
			if(speed != 0){
				speed *= Resources::dt();
				if(overTime){
					o = glm::lerp(original,o,speed*2.5f);
				}
				else{
					o = glm::lerp(original,o,speed);
				}
			}
		}
	};
};
#endif