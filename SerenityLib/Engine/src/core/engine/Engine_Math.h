#pragma once
#ifndef ENGINE_ENGINE_MATH_H
#define ENGINE_ENGINE_MATH_H

#include <glm/fwd.hpp>
#include <glm/gtc/quaternion.hpp>
#include <core/engine/Engine_Utils.h>

#include <vector>
#include <assimp/Importer.hpp>
#include <GL/glew.h>
#include <SFML/OpenGL.hpp>
#include <SFML/Graphics/Color.hpp>

class btVector3;
class btRigidBody;
class btQuaternion;
class Camera;
typedef unsigned short  ushort;
typedef unsigned char   uchar;

namespace Engine{
namespace Math{
    void extractViewFrustumPlanesHartmannGribbs(const glm::mat4& inViewProjection,glm::vec4* outPlanes);

    void Float32From16(float*     out, const uint16_t in);
    void Float16From32(uint16_t*  out, const float    in);

    void Float32From16(float*    out, const uint16_t* in, const uint arraySize);
    void Float16From32(uint16_t* out, const float*    in, const uint arraySize);

    glm::vec2 rotate2DPoint(const glm::vec2& point, const float angle, const glm::vec2& origin = glm::vec2(0.0f, 0.0f));

    glm::quat btToGLMQuat(const btQuaternion& q);
    btQuaternion glmToBTQuat(const glm::quat& q);

    glm::vec3 btVectorToGLM(const btVector3&);
    btVector3 btVectorFromGLM(const glm::vec3&);

    glm::vec3 assimpToGLMVec3(const aiVector3D&);
    glm::mat4 assimpToGLMMat4(const aiMatrix4x4&);
    glm::mat3 assimpToGLMMat3(const aiMatrix3x3&);

    glm::vec3 getScreenCoordinates(const glm::vec3& position, const bool clampToEdge = true);
    glm::vec3 getScreenCoordinates(const glm::vec3& position, Camera&, const bool clampToEdge = true);

    glm::vec3 midpoint(const glm::vec3& a, const glm::vec3& b);

    glm::vec3 direction(const glm::vec3& eye, const glm::vec3& target);


    void lookAtToQuat(glm::quat& o, const glm::vec3& eye, const glm::vec3& target, const glm::vec3& up);

    void translate(const btRigidBody&,btVector3&,bool local);

    uchar pack2NibblesIntoChar(const float x, const float y);
    glm::vec2 unpack2NibblesFromChar(const uchar);
    float pack2NibblesIntoCharBasic(const float x, const float y);
    glm::vec2 unpack2NibblesFromCharBasic(const float);

    GLuint pack3NormalsInto32Int(const float x, const float y, const float z);
    GLuint pack3NormalsInto32Int(const glm::vec3&);

    float pack3FloatsInto1Float(float,float,float);
    float pack3FloatsInto1Float(const glm::vec3&);
    glm::vec3 unpack3FloatsInto1Float(float i);

    float pack3FloatsInto1FloatUnsigned(float,float,float);
    float pack3FloatsInto1FloatUnsigned(const glm::vec3&);
    glm::vec3 unpack3FloatsInto1FloatUnsigned(float i);
        
    float pack2FloatsInto1Float(const float, const float);
    float pack2FloatsInto1Float(const glm::vec2&);
    glm::vec2 unpack2FloatsInto1Float(const float i);

    float remainder(float,float);

    void removeMatrixPosition(glm::mat4&);

    void recalculateForwardRightUp(const glm::quat&,glm::vec3& forward,glm::vec3& right,glm::vec3& up);
    void recalculateForwardRightUp(const btRigidBody&,glm::vec3& forward,glm::vec3& right,glm::vec3& up);

    glm::vec3 getForward(const glm::quat& q);
    glm::vec3 getRight(const glm::quat& q);
    glm::vec3 getUp(const glm::quat& q);
    glm::vec3 getColumnVector(const btRigidBody& b, const uint& column);
    glm::vec3 getForward(const btRigidBody& b);
    glm::vec3 getRight(const btRigidBody& b);
    glm::vec3 getUp(const btRigidBody& b);

    float getAngleBetweenTwoVectors(const glm::vec3& a, const glm::vec3& b, bool degrees = true);
    void alignTo(glm::quat& o, const glm::vec3& direction,float speed=0);

    void setColor(glm::vec3& color, float r, float g, float b);
    void setColor(glm::vec4& color, float r, float g, float b, float a);

    bool isPointWithinCone(const glm::vec3& conePos,const glm::vec3& coneVector,glm::vec3& point,const float fovRadians);
    bool isPointWithinCone(const glm::vec3& conePos,const glm::vec3& coneVector,glm::vec3& point,const float fovRadians,const float fovDistance);

    float toRadians(const float degrees);
    float toDegrees(const float radians);
    float toRadians(const double degrees);
    float toDegrees(const double radians);

    float Max(const glm::vec2&);
    float Max(const glm::vec3&);
    float Max(const glm::vec4&);

    float Max(const float, const float);
    float Max(const float, const float, const float);
    float Max(const float, const float, const float, const float);

    uint Max(const uint, const uint);
    uint Max(const uint, const uint, const uint);
    uint Max(const uint, const uint, const uint, const uint);

    float fade(const float t);
    double fade(const double t);

    float lerp(const float t, const float a, const float b);
    double lerp(const double t, const double a, const double b);

    float grad(const int hash, const float x, const float y, const float z);
    double grad(const int hash, const double x, const double y, const double z);

    glm::vec4 PaintersAlgorithm(const glm::vec4& paintColor, const glm::vec4& canvasColor);
    sf::Color PaintersAlgorithm(const sf::Color& paintColor, const sf::Color& canvasColor);

    bool rayIntersectSphere(const glm::vec3& position, const float radius, const glm::vec3& A, const glm::vec3& rayVector);
};
};
#endif
