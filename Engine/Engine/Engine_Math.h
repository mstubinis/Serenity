#pragma once
#ifndef ENGINE_ENGINE_MATH_H
#define ENGINE_ENGINE_MATH_H

#include <glm/fwd.hpp>
#include <glm/gtc/quaternion.hpp>

#include <vector>

#include <assimp/Importer.hpp>

class btVector3;
class btRigidBody;
class btQuaternion;
class Object;
class ObjectDynamic;
typedef unsigned int uint;

namespace glm{
    //floats
    typedef glm::tquat<float> q_f;	
    typedef glm::tvec2<float> v2_f;
    typedef glm::tvec3<float> v3_f;
    typedef glm::tvec4<float> v4_f;
    typedef glm::tmat2x2<float> m2_f;
    typedef glm::tmat2x2<float> m2x2_f;
    typedef glm::tmat2x3<float> m2x3_f;
    typedef glm::tmat2x4<float> m2x4_f;
    typedef glm::tmat3x3<float> m3_f;
    typedef glm::tmat3x3<float> m3x3_f;
    typedef glm::tmat3x2<float> m3x2_f;
    typedef glm::tmat3x4<float> m3x4_f;
    typedef glm::tmat4x2<float> m4x2_f;
    typedef glm::tmat4x3<float> m4x3_f;
    typedef glm::tmat4x4<float> m4_f;
    typedef glm::tmat4x4<float> m4x4_f;
    //doubles
    typedef glm::tquat<double> q_d;
    typedef glm::tvec2<double> v2_d;
    typedef glm::tvec3<double> v3_d;
    typedef glm::tvec4<double> v4_d;
    typedef glm::tmat2x2<double> m2_d;
    typedef glm::tmat2x2<double> m2x2_d;
    typedef glm::tmat2x3<double> m2x3_d;
    typedef glm::tmat2x4<double> m2x4_d;
    typedef glm::tmat3x3<double> m3_d;
    typedef glm::tmat3x3<double> m3x3_d;
    typedef glm::tmat3x2<double> m3x2_d;
    typedef glm::tmat3x4<double> m3x4_d;
    typedef glm::tmat4x2<double> m4x2_d;
    typedef glm::tmat4x3<double> m4x3_d;
    typedef glm::tmat4x4<double> m4_d;
    typedef glm::tmat4x4<double> m4x4_d;
    //long doubles
    typedef glm::tquat<long double> q_ld;
    typedef glm::tvec2<long double> v2_ld;
    typedef glm::tvec3<long double> v3_ld;
    typedef glm::tvec4<long double> v4_ld;
    typedef glm::tmat2x2<long double> m2_ld;
    typedef glm::tmat2x2<long double> m2x2_ld;
    typedef glm::tmat2x3<long double> m2x3_ld;
    typedef glm::tmat2x4<long double> m2x4_ld;
    typedef glm::tmat3x3<long double> m3_ld;
    typedef glm::tmat3x3<long double> m3x3_ld;
    typedef glm::tmat3x2<long double> m3x2_ld;
    typedef glm::tmat3x4<long double> m3x4_ld;
    typedef glm::tmat4x2<long double> m4x2_ld;
    typedef glm::tmat4x3<long double> m4x3_ld;
    typedef glm::tmat4x4<long double> m4_ld;
    typedef glm::tmat4x4<long double> m4x4_ld;
	
    #ifdef ENGINE_PRECISION_NORMAL
        typedef float number;
        typedef number num;
        typedef q_f q;	
        typedef v2_f v2;
        typedef v3_f v3;
        typedef v4_f v4;
        typedef m2_f m2;
        typedef m2x2_f m2x2;
        typedef m2x3_f m2x3;
        typedef m2x4_f m2x4;
        typedef m3_f m3;
        typedef m3x3_f m3x3;
        typedef m3x2_f m3x2;
        typedef m3x4_f m3x4;
        typedef m4x2_f m4x2;
        typedef m4x3_f m4x3;
        typedef m4_f m4;
        typedef m4x4_f m4x4;
    #endif
    #ifdef ENGINE_PRECISION_HIGH
        typedef double number;
        typedef number num;
        typedef q_d q;
        typedef v2_d v2;
        typedef v3_d v3;
        typedef v4_d v4;
        typedef m2_d m2;
        typedef m2x2_d m2x2;
        typedef m2x3_d m2x3;
        typedef m2x4_d m2x4;
        typedef m3_d m3;
        typedef m3x3_d m3x3;
        typedef m3x2_d m3x2;
        typedef m3x4_d m3x4;
        typedef m4x2_d m4x2;
        typedef m4x3_d m4x3;
        typedef m4_d m4;
        typedef m4x4_d m4x4;
    #endif
    #ifdef ENGINE_PRECISION_VERY_HIGH
		/*
        typedef long double number;
        typedef number num;
        typedef q_ld q;
        typedef v2_ld v2;
        typedef v3_ld v3;
        typedef v4_ld v4;
        typedef m2_ld m2;
        typedef m2x2_ld m2x2;
        typedef m2x3_ld m2x3;
        typedef m2x4_ld m2x4;
        typedef m3_ld m3;
        typedef m3x3_ld m3x3;
        typedef m3x2_ld m3x2;
        typedef m3x4_ld m3x4;
        typedef m4x2_ld m4x2;
        typedef m4x3_ld m4x3;
        typedef m4_ld m4;
        typedef m4x4_ld m4x4;
		*/
    #endif
};

namespace Engine{
    namespace Math{

        glm::quat btToGLMQuat(btQuaternion& q);
        btQuaternion glmToBTQuat(glm::quat& q);

        glm::vec3 assimpToGLMVec3(aiVector3D&);
        glm::mat4 assimpToGLMMat4(aiMatrix4x4&);
        glm::mat3 assimpToGLMMat3(aiMatrix3x3&);

        glm::vec3 getScreenCoordinates(glm::vec3&,bool clampToEdge = true);

        glm::vec3 midpoint(glm::vec3&,glm::vec3&);

        glm::vec3 direction(glm::vec3& eye,glm::vec3& target);

        void lookAtToQuat(glm::quat& o,glm::vec3& eye, glm::vec3& target, glm::vec3& up);
	    
        void translate(ObjectDynamic*,btVector3&,bool local);

        float pack3FloatsInto1(float,float,float);
        float pack3FloatsInto1(glm::vec3&);
        glm::vec3 unpackFloatInto3(float);
        float pack3BytesInto1(unsigned char,unsigned char,unsigned char);

        glm::vec3 getForward(glm::quat& q);
        glm::vec3 getRight(glm::quat& q);
        glm::vec3 getUp(glm::quat& q);
        glm::vec3 getColumnVector(const btRigidBody* b, unsigned int column);
        glm::vec3 getForward(const btRigidBody* b);
        glm::vec3 getRight(const btRigidBody* b);
        glm::vec3 getUp(const btRigidBody* b);

        float getAngleBetweenTwoVectors(glm::vec3& a, glm::vec3& b, bool degrees = true);
        void alignTo(glm::quat& o,Object* origin, glm::vec3& direction,float speed=0);
        void alignToX(glm::quat& o,Object* origin,Object* target,float speed=0);
        void alignToY(glm::quat& o,Object* origin,Object* target,float speed=0);
        void alignToZ(glm::quat& o,Object* origin,Object* target,float speed=0);

        void setColor(glm::vec3& color,float r,float g,float b);
        void setColor(glm::vec4& color,float r,float g,float b,float a);

        bool isPointWithinCone(const glm::vec3& conePos,const glm::vec3& coneVector,glm::vec3& point,const float fovRadians);
        bool isPointWithinCone(const glm::vec3& conePos,const glm::vec3& coneVector,glm::vec3& point,const float fovRadians,const float fovDistance);

        float toRadians(float degrees);
        float toDegrees(float radians);
        float toRadians(double degrees);
        float toDegrees(double radians);

        float Max(glm::vec2&);
        float Max(glm::vec3&);
        float Max(glm::vec4&);

        float Max(float,float);
        float Max(float,float,float);
        float Max(float,float,float,float);

        float fade(float t);
        double fade(double t);

        float lerp(float t, float a, float b);
        double lerp(double t, double a, double b);

        float grad(int hash, float x, float y, float z);
        double grad(int hash, double x, double y, double z);

        glm::vec4 PaintersAlgorithm(glm::vec4& paintColor, glm::vec4& canvasColor);

        bool rayIntersectSphere(glm::vec3& position, float radius, glm::vec3& A, glm::vec3& rayVector);
    };
};
#endif
