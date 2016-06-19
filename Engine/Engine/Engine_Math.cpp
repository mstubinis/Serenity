#include "Engine_Math.h"
#include "Engine_Resources.h"
#include "Camera.h"

#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <bullet/btBulletCollisionCommon.h>
#include <bullet/btBulletDynamicsCommon.h>

using namespace Engine;

glm::vec3 Math::getScreenCoordinates(glm::vec3 objPos,bool clampToEdge){
    glm::vec2 windowSize = glm::vec2(Resources::getWindowSize().x,Resources::getWindowSize().y);
    glm::mat4 MV = Resources::getActiveCamera()->getView();
    glm::vec4 viewport = glm::vec4(0,0,windowSize.x,windowSize.y);
    glm::vec3 screen = glm::project(objPos,MV,Resources::getActiveCamera()->getProjection(),viewport);

    //check if point is behind
    float dot = glm::dot(Resources::getActiveCamera()->getViewVector(),objPos-glm::vec3(Resources::getActiveCamera()->getPosition()));

    float resX = float(screen.x);
    float resY = float(windowSize.y-screen.y);

    unsigned int inBounds = 1;

    if(clampToEdge){
        if(screen.x < 0){ resX = 0; inBounds = 0; }
        else if(screen.x > windowSize.x){ resX = windowSize.x; inBounds = 0; }
        if(resY < 0){ resY = 0; inBounds = 0; }
        else if(resY > windowSize.y){ resY = windowSize.y; inBounds = 0; }
    }

    if(dot < 0.0f){
        return glm::vec3(resX,resY,inBounds);
    }
    inBounds = 0;
    float fX = windowSize.x - screen.x;
    float fY = windowSize.y - resY;
    
    if(fX < windowSize.x/2){ if(clampToEdge) fX = 0; else fX = -9999999999.0f; }
    else if(fX > windowSize.x/2){ if(clampToEdge) fX = windowSize.x; else fX = -9999999999.0f; }
    if(fY < windowSize.y/2){ if(clampToEdge) fY = 0; else fY = -9999999999.0f; }
    else if(fY > windowSize.y/2){ if(clampToEdge) fY = windowSize.y; else fY = -9999999999.0f; }

    return glm::vec3(fX,fY,inBounds);
}
glm::vec3 Math::midpoint(glm::vec3 a, glm::vec3 b){ 
	return glm::vec3((a.x+b.x)/2.f,(a.y+b.y)/2.f,(a.z+b.z)/2.f); 
}
glm::vec3 Math::midpoint(glm::v3 a, glm::v3 b){ 
	return glm::vec3(float((a.x+b.x)/2),float((a.y+b.y)/2),float((a.z+b.z)/2)); 
}
glm::vec3 Math::direction(glm::v3 eye,glm::v3 target){ 
	return glm::normalize(glm::vec3(eye)-glm::vec3(target)); 
}
glm::vec3 Math::direction(glm::vec3 eye,glm::vec3 target){ 
	return glm::normalize(eye-target); 
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
glm::v3 Math::getForward(const btRigidBody* b){ 
	return Math::getColumnVector(b,2); 
}
glm::v3 Math::getRight(const btRigidBody* b){ 
	return Math::getColumnVector(b,0); 
}
glm::v3 Math::getUp(const btRigidBody* b){ 
	return Math::getColumnVector(b,1); 
}

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
void Math::setColor(glm::vec3& color,float r, float g, float b){
    if(r > 1) r = r / 255.0f;
    if(g > 1) g = g / 255.0f;
    if(b > 1) b = b / 255.0f;
    color.x = r; color.y = g; color.z = b;
}
void Math::setColor(glm::vec4& color,float r, float g, float b,float a){
    if(r > 1) r = r / 255.0f;
    if(g > 1) g = g / 255.0f;
    if(b > 1) b = b / 255.0f;
    if(a > 1) a = a / 255.0f;
    color.x = r; color.y = g; color.z = b; color.w = a; 
}