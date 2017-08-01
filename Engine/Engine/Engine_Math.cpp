#include "Engine_Math.h"
#include "Engine_Resources.h"
#include "Camera.h"
#include "ObjectDynamic.h"

#include <math.h>

#include <glm/gtc/quaternion.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <bullet/btBulletCollisionCommon.h>
#include <bullet/btBulletDynamicsCommon.h>

using namespace Engine;

glm::quat Math::btToGLMQuat(btQuaternion& q){ glm::quat glmQuat = glm::quat(q.getW(),q.getX(),q.getY(),q.getZ()); return glmQuat; }
btQuaternion Math::glmToBTQuat(glm::quat& q){ btQuaternion btQuat = btQuaternion(q.x,q.y,q.z,q.w); return btQuat; }
glm::vec3 Math::assimpToGLMVec3(aiVector3D& n){ glm::vec3 ret = glm::vec3(n.x,n.y,n.z); return ret; }
glm::mat4 Math::assimpToGLMMat4(aiMatrix4x4& n){ glm::mat4 ret = glm::mat4(n.a1,n.b1,n.c1,n.d1,n.a2,n.b2,n.c2,n.d2,n.a3,n.b3,n.c3,n.d3,n.a4,n.b4,n.c4,n.d4); return ret; }
glm::mat3 Math::assimpToGLMMat3(aiMatrix3x3& n){ glm::mat3 ret = glm::mat3(n.a1,n.b1,n.c1,n.a2,n.b2,n.c2,n.a3,n.b3,n.c3); return ret; }

float Math::toRadians(float degrees){ return degrees * 0.0174533f; }
float Math::toDegrees(float radians){ return radians * 57.2958f; }
float Math::toRadians(double degrees){ return Math::toRadians(float(degrees)); }
float Math::toDegrees(double radians){ return Math::toDegrees(float(radians)); }

bool Math::isPointWithinCone(const glm::vec3& conePos,const glm::vec3& coneVector,glm::vec3& point,const float fovRadians){
    // forced protection against NaN if vectors happen to be equal
    point.x += 0.0001f;
    //
    glm::vec3 differenceVector = glm::normalize(point - conePos);
    float t = glm::dot(coneVector,differenceVector);
    return ( t >= glm::cos( fovRadians ) );
}
bool Math::isPointWithinCone(const glm::vec3& conePos,const glm::vec3& coneVector,glm::vec3& point,const float fovRadians,const float maxDistance){
    // forced protection against NaN if vectors happen to be equal
    point.x += 0.0001f;
    //
    glm::vec3 differenceVector = glm::normalize(point - conePos);
    float t = glm::dot(coneVector,differenceVector);
    float length = glm::length(point-conePos);
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
float Math::Max(glm::vec2& v){ return glm::max(v.x,v.y); }
float Math::Max(glm::vec3& v){ return glm::max(v.x,glm::max(v.y,v.z)); }
float Math::Max(glm::vec4& v){ return glm::max(v.x,glm::max(v.y,glm::max(v.z,v.w))); }
float Math::Max(float x, float y){ return glm::max(x,y); }
float Math::Max(float x, float y, float z){ return glm::max(x,glm::max(y,z)); }
float Math::Max(float x, float y, float z, float w){ return glm::max(x,glm::max(y,glm::max(z,w))); }

float Math::pack4FloatsInto1Float(float r,float g,float b,float a){
	const float decode_x = 1.0f/255.0f;
	const float decode_y = 1.0f/65025.0f;
	const float decode_z = 1.0f/16581375.0f;
	return glm::dot( glm::vec4(r,g,b,a), glm::vec4(1.0f, decode_x, decode_y, decode_z) );
}
float Math::pack4FloatsInto1Float(glm::vec4& c){ return Math::pack4FloatsInto1Float(c.r,c.g,c.b,c.a); }

float Math::pack3FloatsInto1(float x,float y,float z){
    //Scale and bias
    unsigned char _x = (unsigned char)(x*255.0f);    
    unsigned char _y = (unsigned char)(y*255.0f);        
    unsigned char _z = (unsigned char)(z*255.0f);       
     
    unsigned int packedColor = (_x << 16) | (_y << 8) | _z;
    float packedFloat = (float) ( ((double)packedColor) / ((double) (1 << 24)) );  
    return packedFloat;
}
float Math::pack3FloatsInto1(glm::vec3& v){ return Math::pack3FloatsInto1(v.x,v.y,v.z); }
glm::vec3 Math::unpackFloatInto3(float f){
    float r = (float)fmod(f, 1.0f);
    float g = (float)fmod(f, 1.0f);
    float b = (float)fmod(f, 1.0f);

    //Unpack to the -1..1 range
    r = (r * 2.0f) - 1.0f;
    g = (g * 2.0f) - 1.0f;
    b = (b * 2.0f) - 1.0f;
    
    return glm::vec3(r,g,b);
}
float Math::pack3BytesInto1Float(unsigned char r,unsigned char g,unsigned char b){
    return float(r) + (float(g) * 256.0f) + (float(b) * 256.0f * 256.0f);
}



void Math::translate(ObjectDynamic* obj,btVector3& vec,bool local){
    if(local){
        btTransform t;
        btQuaternion q = obj->getRigidBody()->getWorldTransform().getRotation();
        q = q.normalize();
        vec = vec.rotate(q.getAxis(),q.getAngle());
    }
}
void Math::lookAtToQuat(glm::quat& o,glm::vec3& eye, glm::vec3& target, glm::vec3& up){
    glm::vec3 forward = eye - target;
 
    glm::vec3 vector = glm::normalize(forward);
    glm::vec3 vector2 = glm::normalize(glm::cross(vector,up));
    glm::vec3 vector3 = glm::cross(vector,vector2);
    float m00 = vector2.x;
    float m01 = vector2.y;
    float m02 = vector2.z;
    float m10 = vector3.x;
    float m11 = vector3.y;
    float m12 = vector3.z;
    float m20 = vector.x;
    float m21 = vector.y;
    float m22 = vector.z;
 
    double num8 = (m00 + m11) + m22;
    if (num8 > 0.0f){
        float num = float((double)glm::sqrt(num8 + 1.0));
        o.w = num * 0.5f;
        num = 0.5f / num;
        o.x = (m12 - m21) * num;
        o.y = (m20 - m02) * num;
        o.z = (m01 - m10) * num;
        return;
    }
    if ((m00 >= m11) && (m00 >= m22)){
        float num7 = float((double)glm::sqrt(((1.0 + m00) - m11) - m22));
        float num4 = 0.5f / num7;
        o.x = 0.5f * num7;
        o.y = (m01 + m10) * num4;
        o.z = (m02 + m20) * num4;
        o.w = (m12 - m21) * num4;
        return;
    }
    if (m11 > m22){
        float num6 = float((double)glm::sqrt(((1.0 + m11) - m00) - m22));
        float num3 = 0.5f / num6;
        o.x = (m10 + m01) * num3;
        o.y = 0.5f * num6;
        o.z = (m21 + m12) * num3;
        o.w = (m20 - m02) * num3;
        return;
    }
    float num5 = float((double)glm::sqrt(((1.0 + m22) - m00) - m11));
    float num2 = 0.5f / num5;
    o.x = (m20 + m02) * num2;
    o.y = (m21 + m12) * num2;
    o.z = 0.5f * num5;
    o.w = (m01 - m10) * num2;
}

glm::vec3 Math::midpoint(glm::vec3& a, glm::vec3& b){ return glm::vec3((a.x+b.x)/2.f,(a.y+b.y)/2.f,(a.z+b.z)/2.f); }
glm::vec3 Math::direction(glm::vec3& eye,glm::vec3& target){ return glm::normalize(eye-target); }
glm::vec3 Math::getForward(glm::quat& q){return glm::normalize(q * glm::vec3(0,0,-1));}
glm::vec3 Math::getRight(glm::quat& q){return glm::normalize(q * glm::vec3(1,0,0));}
glm::vec3 Math::getUp(glm::quat& q){return glm::normalize(q * glm::vec3(0,1,0));}
glm::vec3 Math::getColumnVector(const btRigidBody* b, unsigned int column){
    btTransform t;
    b->getMotionState()->getWorldTransform(t);
    btVector3 v = t.getBasis().getColumn(column);
	return glm::normalize(glm::vec3(v.x(),v.y(),v.z()));
}
glm::vec3 Math::getForward(const btRigidBody* b){ return Math::getColumnVector(b,2); }
glm::vec3 Math::getRight(const btRigidBody* b){ return Math::getColumnVector(b,0); }
glm::vec3 Math::getUp(const btRigidBody* b){ return Math::getColumnVector(b,1); }

float Math::getAngleBetweenTwoVectors(glm::vec3& a, glm::vec3& b, bool degrees){
    // forced protection against NaN if a and b happen to be equal
    a.x += 0.0001f;
    //
    float angle = glm::acos( glm::dot(a,b) / (glm::length(a)*glm::length(b)) );
    if(degrees) angle *= 57.2958f;
    return angle;
}

void Math::alignToX(glm::quat& o,Object* origin, Object* target,float speed){

}
void Math::alignToY(glm::quat& o,Object* origin, Object* target,float speed){

}
void Math::alignToZ(glm::quat& o,Object* origin, Object* target,float speed){

}
void Math::alignTo(glm::quat& o,Object* origin, glm::vec3& direction,float speed){
    glm::quat original(o);
    direction = glm::normalize(direction);
    glm::vec3 xaxis = glm::normalize(glm::cross(glm::vec3(0,1,0), direction));
    glm::vec3 yaxis = glm::normalize(glm::cross(direction, xaxis));
    glm::mat3 rot;
    rot[0][0] = float(xaxis.x);     rot[0][1] = float(xaxis.y);     rot[0][2] = float(xaxis.z);
    rot[1][0] = float(yaxis.x);     rot[1][1] = float(yaxis.y);     rot[1][2] = float(yaxis.z);
    rot[2][0] = float(direction.x); rot[2][1] = float(direction.y); rot[2][2] = float(direction.z);
    o = glm::quat_cast(rot);
    if(speed != 0){
        float angle = Math::getAngleBetweenTwoVectors(direction,glm::vec3(getForward(original)),true); // degrees
        speed *= 1.0f/angle;
        speed *= Resources::dt();
        o = glm::mix(original,o,speed*5);
    }
    o = glm::normalize(o);
}
void Math::setColor(glm::vec3& c,float r, float g, float b){
    if(r > 1) r = r / 255.0f;
    if(g > 1) g = g / 255.0f;
    if(b > 1) b = b / 255.0f;
    c.x = r; c.y = g; c.z = b;
}
void Math::setColor(glm::vec4& c,float r, float g, float b,float a){
    if(r > 1) r = r / 255.0f;
    if(g > 1) g = g / 255.0f;
    if(b > 1) b = b / 255.0f;
    if(a > 1) a = a / 255.0f;
    c.x = r; c.y = g; c.z = b; c.w = a; 
}
float Math::fade(float t){ return t*t*t*(t*(t*6.0f-15.0f)+10.0f); }
double Math::fade(double t){ return t*t*t*(t*(t*6.0-15.0)+10.0); }
float Math::lerp(float t, float a, float b){return a + t * (b - a);}
double Math::lerp(double t, double a, double b){return a + t * (b - a);}
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
glm::vec4 Math::PaintersAlgorithm(glm::vec4& p, glm::vec4& c){
    glm::vec4 ret(0);
    float a = p.a + c.a * (1-p.a);
    ret.r = ((p.r*p.a + c.r*c.a * (1-p.a)) / a);
    ret.g = ((p.g*p.a + c.g*c.a * (1-p.a)) / a);
    ret.b = ((p.b*p.a + c.b*c.a * (1-p.a)) / a);
    ret.a = a;
    return ret;
}
bool Math::rayIntersectSphere(glm::vec3& C, float r,glm::vec3& A, glm::vec3& rayVector){
    glm::vec3 _a = glm::vec3(A);
    glm::vec3 B = _a + rayVector;
    float dot = glm::dot(rayVector,C-_a); //check if point is behind
    if(dot >= 0)
        return false;
    float a = ((B.x-A.x)*(B.x-A.x))  +  ((B.y - A.y)*(B.y - A.y))  +  ((B.z - A.z)*(B.z - A.z));
    float b = 2* ((B.x - A.x)*(A.x - C.x)  +  (B.y - A.y)*(A.y - C.y)  +  (B.z - A.z)*(A.z-C.z));
    float c = (((A.x-C.x)*(A.x-C.x))  +  ((A.y - C.y)*(A.y - C.y))  +  ((A.z - C.z)*(A.z - C.z))) - (r*r);
    float d = (b*b) - (4*a*c);
    if(d < 0)
        return false;
    return true;
}
