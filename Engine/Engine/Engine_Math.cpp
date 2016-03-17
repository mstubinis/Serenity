#include "Engine_Math.h"

using namespace Engine;

glm::vec3 Math::midpoint(glm::vec3 a, glm::vec3 b){
	float _x = (a.x + b.x)/2.0f;
	float _y = (a.y + b.y)/2.0f;
	float _z = (a.z + b.z)/2.0f;
	return glm::vec3(_x,_y,_z);
}
glm::vec3 Math::midpoint(glm::v3 a, glm::v3 b){
	float _x = static_cast<float>((a.x + b.x)/2);
	float _y = static_cast<float>((a.y + b.y)/2);
	float _z = static_cast<float>((a.z + b.z)/2);
	return glm::vec3(_x,_y,_z);
}

glm::v3 Math::getForward(glm::quat& q){
	return glm::normalize(glm::v3(2*(q.x*q.z+q.w*q.y),2*(q.y*q.x-q.w*q.x),1-2*(q.x*q.x+q.y*q.y)));
}
glm::v3 Math::getRight(glm::quat& q){
	return glm::normalize(glm::v3(1-2*(q.y*q.y+q.z*q.z),2*(q.x*q.y+q.w*q.z),2*(q.x*q.z-q.w*q.y)));
}
glm::v3 Math::getUp(glm::quat& q){
	return glm::normalize(glm::v3(2*(q.x*q.y-q.w*q.z),1-2*(q.x*q.x+q.z*q.z),2*(q.y*q.z+q.w*q.x)));
}
glm::v3 Math::getColumnVector(const btRigidBody* b, unsigned int column){
	btTransform t;
	b->getMotionState()->getWorldTransform(t);
	btVector3 v = t.getBasis().getColumn(column);
	return glm::v3(v.x(),v.y(),v.z());
}
glm::v3 Math::getForward(const btRigidBody* b){ return Math::getColumnVector(b,2); }
glm::v3 Math::getRight(const btRigidBody* b){ return Math::getColumnVector(b,0); }
glm::v3 Math::getUp(const btRigidBody* b){ return Math::getColumnVector(b,1); }

float Math::getAngleBetweenTwoVectors(glm::vec3 a, glm::vec3 b, bool degrees){
	if(degrees == true)
		return glm::acos(glm::dot(a,b)) * 57.2958f;
	return glm::acos(glm::dot(a,b));
}

void Math::alignTo(glm::quat& o, glm::vec3 direction,float speed, bool overTime){
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