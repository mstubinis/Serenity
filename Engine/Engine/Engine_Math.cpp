#include "Engine_Math.h"
#include "Engine_Resources.h"
#include "Camera.h"

#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <bullet/btBulletCollisionCommon.h>
#include <bullet/btBulletDynamicsCommon.h>

using namespace Engine;

float Math::toRadians(float degrees){ return degrees * 0.0174533f; }
float Math::toDegrees(float radians){ return radians * 57.2958f; }
float Math::toRadians(double degrees){ return Math::toRadians(float(degrees)); }
float Math::toDegrees(double radians){ return Math::toDegrees(float(radians)); }

bool Math::isPointWithinCone(const glm::v3& conePos,const glm::v3& coneVector,glm::v3& point,const float fovRadians){
    // forced protection against NaN if vectors happen to be equal
    point.x += 0.01f;
    //
    glm::v3 differenceVector = glm::normalize(point - conePos);
    glm::num t = glm::dot(coneVector,differenceVector);
    return ( t >= glm::cos( fovRadians ) );
}
bool Math::isPointWithinCone(const glm::v3& conePos,const glm::v3& coneVector,glm::v3& point,const float fovRadians,const glm::num maxDistance){
    // forced protection against NaN if vectors happen to be equal
    point.x += 0.01f;
    //
    glm::v3 differenceVector = glm::normalize(point - conePos);
    glm::num t = glm::dot(coneVector,differenceVector);
    glm::num length = glm::length(point-conePos);
    if ( length > maxDistance ){ return false; }
    return ( t >= glm::cos( fovRadians ) );
}

glm::vec3 Math::getScreenCoordinates(glm::vec3& objPos,bool clampToEdge){
    glm::vec2 windowSize = glm::vec2(Resources::getWindowSize().x,Resources::getWindowSize().y);
    glm::vec4 viewport = glm::vec4(0,0,windowSize.x,windowSize.y);
    glm::vec3 screen = glm::project(objPos,Resources::getActiveCamera()->getView(),Resources::getActiveCamera()->getProjection(),viewport);

    //check if point is behind
    float dot = glm::dot(Resources::getActiveCamera()->getViewVector(),objPos-glm::vec3(Resources::getActiveCamera()->getPosition()));

    float resX = float(screen.x);
    float resY = float(screen.y);

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
    float fY = windowSize.y - screen.y;
    
    if(fX < windowSize.x/2){ if(clampToEdge) fX = 0; else fX = -9999999999.0f; }
    else if(fX > windowSize.x/2){ if(clampToEdge) fX = windowSize.x; else fX = -9999999999.0f; }
    if(fY < windowSize.y/2){ if(clampToEdge) fY = 0; else fY = -9999999999.0f; }
    else if(fY > windowSize.y/2){ if(clampToEdge) fY = windowSize.y; else fY = -9999999999.0f; }

    return glm::vec3(fX,fY,inBounds);
}
glm::vec3 Math::midpoint(glm::vec3& a, glm::vec3& b){ return glm::vec3((a.x+b.x)/2.f,(a.y+b.y)/2.f,(a.z+b.z)/2.f); }
glm::vec3 Math::midpoint(glm::v3& a, glm::v3& b){ return glm::vec3(float((a.x+b.x)/2),float((a.y+b.y)/2),float((a.z+b.z)/2)); }
glm::vec3 Math::direction(glm::v3& eye,glm::v3& target){ return glm::normalize(glm::vec3(eye)-glm::vec3(target)); }
glm::vec3 Math::direction(glm::vec3& eye,glm::vec3& target){ return glm::normalize(eye-target); }
glm::v3 Math::getForward(glm::quat& q){return glm::normalize(glm::v3(2*(q.x*q.z+q.w*q.y),2*(q.y*q.x-q.w*q.x),1-2*(q.x*q.x+q.y*q.y)));}
glm::v3 Math::getRight(glm::quat& q){return glm::normalize(glm::v3(1-2*(q.y*q.y+q.z*q.z),2*(q.x*q.y+q.w*q.z),2*(q.x*q.z-q.w*q.y)));}
glm::v3 Math::getUp(glm::quat& q){return glm::normalize(glm::v3(2*(q.x*q.y-q.w*q.z),1-2*(q.x*q.x+q.z*q.z),2*(q.y*q.z+q.w*q.x)));}
glm::v3 Math::getColumnVector(const btRigidBody* b, unsigned int column){
    btTransform t;
    b->getMotionState()->getWorldTransform(t);
    btVector3 v = t.getBasis().getColumn(column);
    return glm::v3(v.x(),v.y(),v.z());
}
glm::v3 Math::getForward(const btRigidBody* b){ return Math::getColumnVector(b,2); }
glm::v3 Math::getRight(const btRigidBody* b){ return Math::getColumnVector(b,0); }
glm::v3 Math::getUp(const btRigidBody* b){ return Math::getColumnVector(b,1); }

float Math::getAngleBetweenTwoVectors(glm::vec3& a, glm::vec3& b, bool degrees){
    // forced protection against NaN if a and b happen to be equal
    a.x += 0.01f;
    //
    float angle = glm::acos( glm::dot(a,b) / (glm::length(a)*glm::length(b)) );
    if(degrees) angle *= 57.2958f;
    return angle;
}

void Math::alignTo(glm::quat& o, glm::vec3& direction,float speed, bool overTime){
    glm::quat original(o);

    glm::vec3 forward = glm::normalize(glm::vec3(getForward(o)));
    direction = -glm::normalize(direction);

    glm::vec3 xaxis = glm::normalize(glm::cross(glm::vec3(0,1,0), direction));
    glm::vec3 yaxis = glm::normalize(glm::cross(direction, xaxis));

    glm::mat3 rot;
    rot[0][0] = float(xaxis.x);
    rot[1][0] = float(yaxis.x);
    rot[2][0] = float(direction.x);
    rot[0][1] = float(xaxis.y);
    rot[1][1] = float(yaxis.y);
    rot[2][1] = float(direction.y);
    rot[0][2] = float(xaxis.z);
    rot[1][2] = float(yaxis.z);
    rot[2][2] = float(direction.z);
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
float Math::fade(float t){ return t*t*t*(t*(t*6.0f-15.0f)+10.0f); }
double Math::fade(double t){ return t*t*t*(t*(t*6.0-15.0)+10.0); }
glm::num Math::fade(glm::num t){ return t*t*t*(t*(t*glm::num(6.0)-glm::num(15.0))+glm::num(10.0)); }
float Math::lerp(float t, float a, float b){return a + t * (b - a);}
double Math::lerp(double t, double a, double b){return a + t * (b - a);}
glm::num Math::lerp(glm::num t, glm::num a, glm::num b){return a + t * (b - a);}
float Math::grad(int hash, float x, float y, float z){
    int h = hash & 15;
    double u = h<8 ? x : y,v = h<4 ? y : h==12||h==14 ? x : z;
    return float(((h&1) == 0 ? u : -u) + ((h&2) == 0 ? v : -v));
}
double Math::grad(int hash, double x, double y, double z){
    int h = hash & 15;
    double u = h<8 ? x : y,v = h<4 ? y : h==12||h==14 ? x : z;
    return ((h&1) == 0 ? u : -u) + ((h&2) == 0 ? v : -v);
}
glm::num Math::grad(int hash, glm::num x, glm::num y, glm::num z){
    int h = hash & 15;
    double u = h<8 ? x : y,v = h<4 ? y : h==12||h==14 ? x : z;
    return glm::num(((h&1) == 0 ? u : -u) + ((h&2) == 0 ? v : -v));
}
glm::vec4 Math::PaintersAlgorithm(glm::vec4& bottomColor, glm::vec4& topColor){
	glm::vec4 ret(0);

	float _a = topColor.a + bottomColor.a * (1-topColor.a);
	ret.r = ((topColor.r*topColor.a + bottomColor.r*bottomColor.a * (1-topColor.a)) / _a);
    ret.g = ((topColor.g*topColor.a + bottomColor.g*bottomColor.a * (1-topColor.a)) / _a);
    ret.b = ((topColor.b*topColor.a + bottomColor.b*bottomColor.a * (1-topColor.a)) / _a);
	ret.a = _a;

	return ret;
}