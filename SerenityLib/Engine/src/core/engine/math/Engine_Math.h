#pragma once
#ifndef ENGINE_ENGINE_MATH_H
#define ENGINE_ENGINE_MATH_H

#include <core/engine/utils/Utils.h>

#include <assimp/Importer.hpp>
#include <GL/glew.h>
#include <SFML/OpenGL.hpp>
#include <SFML/Graphics/Color.hpp>

class btVector3;
class btRigidBody;
class btQuaternion;
class Camera;

namespace Engine{
namespace Math{

    const glm_vec3 rotate_vec3(const glm_quat& rotation, const glm_vec3& vec);

    const glm::vec3 polynomial_interpolate_linear(std::vector<glm::vec3>& points, const float time);
    const glm::vec3 polynomial_interpolate_cubic(std::vector<glm::vec3>& points, const float time);

    void extractViewFrustumPlanesHartmannGribbs(const glm::mat4& inViewProjection,glm::vec4* outPlanes);

    void Float32From16(float*     out, const uint16_t in);
    void Float16From32(uint16_t*  out, const float    in);

    void Float32From16(float*    out, const uint16_t* in, const uint arraySize);
    void Float16From32(uint16_t* out, const float*    in, const uint arraySize);

    glm::vec2 rotate2DPoint(const glm::vec2& point, const float angle, const glm::vec2& origin = glm::vec2(0.0f, 0.0f));

    const glm_quat btToGLMQuat(const btQuaternion& q);
    btQuaternion glmToBTQuat(const glm_quat& q);

    const glm_vec3 btVectorToGLM(const btVector3&);
    btVector3 btVectorFromGLM(const glm_vec3&);

    glm::vec3 assimpToGLMVec3(const aiVector3D&);
    glm::mat4 assimpToGLMMat4(const aiMatrix4x4&);
    glm::mat3 assimpToGLMMat3(const aiMatrix3x3&);

    glm::vec3 getScreenCoordinates(const glm::vec3& position, Camera&, const bool clampToEdge = true);
    glm::vec3 getScreenCoordinates(const glm::vec3& position, Camera&, const glm::vec4& viewport, const bool clampToEdge = true);
    glm::vec3 getScreenCoordinates(const glm::vec3& position, Camera&, const glm::mat4& view, const glm::mat4& projection, const glm::vec4& viewport, const bool clampToEdge = true);

    glm::vec3 midpoint(const glm::vec3& a, const glm::vec3& b);

    glm::vec3 direction(const glm::vec3& eye, const glm::vec3& target);

    void translate(const btRigidBody&,btVector3&,bool local);
    void rotate(glm_quat& orientation, const decimal& pitch, const decimal& yaw, const decimal& roll);
    void rotate(glm::quat& orientation, const float& pitch, const float& yaw, const float& roll);
    void setRotation(glm_quat& orientation, const decimal& pitch, const decimal& yaw, const decimal& roll);
    void setRotation(glm::quat& orientation, const float& pitch, const float& yaw, const float& roll);
    void setFinalModelMatrix(glm_mat4& modelMatrix, const glm_vec3& position,const glm_quat& rotation, const glm_vec3& scale);
    void setFinalModelMatrix(glm::mat4& modelMatrix, const glm::vec3& position, const glm::quat& rotation, const glm::vec3& scale);

    uchar pack2NibblesIntoChar(const float x, const float y);
    glm::vec2 unpack2NibblesFromChar(const uchar);
    float pack2NibblesIntoCharBasic(const float x, const float y);
    glm::vec2 unpack2NibblesFromCharBasic(const float);

    const std::uint32_t pack3NormalsInto32Int(const float x, const float y, const float z);
    const glm::vec3     unpack3NormalsFrom32Int(const std::uint32_t&);
    const std::uint32_t pack3NormalsInto32Int(const glm::vec3&);

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

    void recalculateForwardRightUp(const glm_quat&,glm_vec3& forward,glm_vec3& right,glm_vec3& up);
    void recalculateForwardRightUp(const btRigidBody&,glm_vec3& forward,glm_vec3& right,glm_vec3& up);

    glm::vec3 getForward(const glm_quat& q);
    glm::vec3 getRight(const glm_quat& q);
    glm::vec3 getUp(const glm_quat& q);
    glm::vec3 getColumnVector(const btRigidBody& b, const uint& column);
    glm::vec3 getForward(const btRigidBody& b);
    glm::vec3 getRight(const btRigidBody& b);
    glm::vec3 getUp(const btRigidBody& b);

    float getAngleBetweenTwoVectors(const glm::vec3& a, const glm::vec3& b, bool degrees = true);
    void alignTo(glm_quat& o, const glm_vec3& direction);

    void setColor(glm::vec3& color, float r, float g, float b);
    void setColor(glm::vec4& color, float r, float g, float b, float a);

    bool isPointWithinCone(const glm::vec3& conePos,const glm::vec3& coneVector, const glm::vec3& point,const float fovRadians);
    bool isPointWithinCone(const glm::vec3& conePos,const glm::vec3& coneVector, const glm::vec3& point,const float fovRadians,const float fovDistance);

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
