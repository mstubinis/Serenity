#pragma once
#ifndef ENGINE_ENGINE_MATH_H
#define ENGINE_ENGINE_MATH_H

class btVector3;
class btRigidBody;
class btQuaternion;
class Camera;

#include <assimp/Importer.hpp>
#include <GL/glew.h>
#include <SFML/OpenGL.hpp>
#include <SFML/Graphics/Color.hpp>
#include <serenity/dependencies/glm.h>
#include <glm/gtc/packing.hpp>
#include <glm/gtc/round.hpp>
#include <vector>

namespace Engine::Math {
    inline glm_vec3 rotate_vec3(const glm_quat& rotation, const glm_vec3& vec) noexcept { return rotation * vec; }

    [[nodiscard]] glm::vec3 polynomial_interpolate_linear(const std::vector<glm::vec3>& points, float time);
    [[nodiscard]] glm::vec3 polynomial_interpolate_cubic(const std::vector<glm::vec3>& points, float time);

    [[nodiscard]] bool IsSpecialFloat(float number) noexcept;
    [[nodiscard]] bool IsSpecialFloat(const glm::vec2& vec) noexcept;
    [[nodiscard]] bool IsSpecialFloat(const glm::vec3& vec) noexcept;

    [[nodiscard]] bool IsNear(float v1, float v2, float threshold) noexcept;
    [[nodiscard]] bool IsNear(glm::vec2& v1, glm::vec2& v2, float threshold) noexcept;
    [[nodiscard]] bool IsNear(glm::vec3& v1, glm::vec3& v2, float threshold) noexcept;

    void extractViewFrustumPlanesHartmannGribbs(const glm::mat4& inViewProjection, std::array<glm::vec4, 6>& outPlanes);

    inline void Float32From16(float* out, const uint16_t in) noexcept { *out = glm::unpackHalf1x16(in); }
    inline void Float16From32(uint16_t* out, const float in) noexcept { *out = glm::packHalf1x16(in); }
    [[nodiscard]] inline float Float32From16(const uint16_t in) noexcept { return glm::unpackHalf1x16(in); }
    [[nodiscard]] inline uint16_t Float16From32(const float in) noexcept { return glm::packHalf1x16(in); }

    void Float32From16(float*    out, const uint16_t* in, uint32_t arraySize) noexcept;
    void Float16From32(uint16_t* out, const float*    in, uint32_t arraySize) noexcept;

    [[nodiscard]] glm::vec2 rotate2DPoint(const glm::vec2& point, float angle, const glm::vec2& origin = glm::vec2(0.0f, 0.0f));

    [[nodiscard]] glm_quat toGLM(const btQuaternion& q);
    [[nodiscard]] glm_vec3 toGLM(const btVector3&);
    [[nodiscard]] glm::vec3 toGLM(const aiVector3D&);
    [[nodiscard]] glm::mat4 toGLM(const aiMatrix4x4&);
    [[nodiscard]] glm::mat3 toGLM(const aiMatrix3x3&);
    [[nodiscard]] glm::quat toGLM(const aiQuaternion&);

    [[nodiscard]] btQuaternion toBT(const glm_quat& q);
    [[nodiscard]] btVector3 toBT(const glm_vec3&);

    [[nodiscard]] bool rect_fully_contained(const glm::vec4& bigger, const glm::vec4& smaller) noexcept;
    [[nodiscard]] glm::vec4 rect_union(const glm::vec4& bigger, const glm::vec4& smaller) noexcept;
    [[nodiscard]] glm::vec3 getScreenCoordinates(const glm::vec3& position, const Camera&, bool clampToEdge = true);
    [[nodiscard]] glm::vec3 getScreenCoordinates(const glm::vec3& position, const Camera&, const glm::vec4& viewport, bool clampToEdge = true);
    [[nodiscard]] glm::vec3 getScreenCoordinates(const glm::vec3& position, const Camera&, const glm::mat4& view, const glm::mat4& projection, const glm::vec4& viewport, bool clampToEdge = true);

    [[nodiscard]] glm::vec3 midpoint(const glm::vec3& a, const glm::vec3& b);
    [[nodiscard]] glm::vec3 direction(const glm::vec3& eye, const glm::vec3& target);

    void translate(const btRigidBody&, btVector3&, bool local) noexcept;
    void rotate(glm_quat& orientation, decimal pitch, decimal yaw, decimal roll) noexcept;
    void rotate(glm::quat& orientation, float pitch, float yaw, float roll) noexcept;
    void setRotation(glm_quat& orientation, decimal pitch, decimal yaw, decimal roll) noexcept;
    void setRotation(glm::quat& orientation, float pitch, float yaw, float roll) noexcept;
    void setFinalModelMatrix(glm_mat4& modelMatrix, const glm_vec3& position,const glm_quat& rotation, const glm_vec3& scale) noexcept;
    void setFinalModelMatrix(glm::mat4& modelMatrix, const glm::vec3& position, const glm::quat& rotation, const glm::vec3& scale) noexcept;

    [[nodiscard]] glm::vec3 getMatrixPosition(const glm::mat4& matrix);
    [[nodiscard]] glm_vec3 getMatrixPosition(const glm_mat4& matrix);
    void removeMatrixPosition(glm::mat4&);

    void recalculateForwardRightUp(const glm_quat&, glm_vec3& forward, glm_vec3& right, glm_vec3& up);
    void recalculateForwardRightUp(const btRigidBody&, glm_vec3& forward, glm_vec3& right, glm_vec3& up);

    [[nodiscard]] glm::vec3 getForward(const glm_quat& q);
    [[nodiscard]] glm::vec3 getRight(const glm_quat& q);
    [[nodiscard]] glm::vec3 getUp(const glm_quat& q);
    [[nodiscard]] glm::vec3 getColumnVector(const btRigidBody& b, unsigned int column);
    [[nodiscard]] glm::vec3 getForward(const btRigidBody& b);
    [[nodiscard]] glm::vec3 getRight(const btRigidBody& b);
    [[nodiscard]] glm::vec3 getUp(const btRigidBody& b);

    [[nodiscard]] float getAngleBetweenTwoVectors(const glm::vec3& a, const glm::vec3& b, bool degrees = true);

    //void alignTo(glm_quat& o, const glm_vec3& direction) noexcept;
    //void alignTo(glm_quat& o, decimal x, decimal y, decimal z) noexcept;
    [[nodiscard]] glm_quat alignTo(decimal x, decimal y, decimal z) noexcept;
    [[nodiscard]] inline glm_quat alignTo(const glm_vec3& direction) noexcept { return alignTo(direction.x, direction.y, direction.z); }

    void setColor(glm::vec3& color, float r, float g, float b);
    void setColor(glm::vec4& color, float r, float g, float b, float a);

    [[nodiscard]] bool isPointWithinCone(const glm::vec3& conePos, const glm::vec3& coneVector, const glm::vec3& point, float fovRadians);
    [[nodiscard]] bool isPointWithinCone(const glm::vec3& conePos, const glm::vec3& coneVector, const glm::vec3& point, float fovRadians, float fovDistance);

    [[nodiscard]] inline float toRadians(float degrees) noexcept { return degrees * 0.0174533f; }
    [[nodiscard]] inline float toDegrees(float radians) noexcept { return radians * 57.2958f; }
    [[nodiscard]] inline float toRadians(double degrees) noexcept { return Math::toRadians((float)degrees); }
    [[nodiscard]] inline float toDegrees(double radians) noexcept { return Math::toDegrees((float)radians); }
    [[nodiscard]] inline float remainder(float x, float y) noexcept { return x - (glm::round(x / y) * y); }

    [[nodiscard]] inline float Max(const glm::vec2& v) noexcept { return std::max(v.x, v.y); }
    [[nodiscard]] inline float Max(const glm::vec3& v) noexcept { return std::max(v.x, std::max(v.y, v.z)); }
    [[nodiscard]] inline float Max(const glm::vec4& v) noexcept { return std::max(v.x, std::max(v.y, std::max(v.z, v.w))); }
    [[nodiscard]] inline float Max(float x, float y) noexcept { return std::max(x, y); }
    [[nodiscard]] inline float Max(float x, float y, float z) noexcept { return std::max(x, std::max(y, z)); }
    [[nodiscard]] inline float Max(float x, float y, float z, float w) noexcept { return std::max(x, std::max(y, std::max(z, w))); }

    [[nodiscard]] inline double Max(const glm::dvec2& v) noexcept { return std::max(v.x, v.y); }
    [[nodiscard]] inline double Max(const glm::dvec3& v) noexcept { return std::max(v.x, std::max(v.y, v.z)); }
    [[nodiscard]] inline double Max(const glm::dvec4& v) noexcept { return std::max(v.x, std::max(v.y, std::max(v.z, v.w))); }
    [[nodiscard]] inline double Max(double x, double y) noexcept { return std::max(x, y); }
    [[nodiscard]] inline double Max(double x, double y, double z) noexcept { return std::max(x, std::max(y, z)); }
    [[nodiscard]] inline double Max(double x, double y, double z, double w) noexcept { return std::max(x, std::max(y, std::max(z, w))); }

    [[nodiscard]] inline uint32_t Max(uint32_t x, uint32_t y) noexcept { return std::max(x, y); }
    [[nodiscard]] inline uint32_t Max(uint32_t x, uint32_t y, unsigned int z) noexcept { return std::max(x, std::max(y, z)); }
    [[nodiscard]] inline uint32_t Max(uint32_t x, uint32_t y, uint32_t z, uint32_t w) noexcept { return std::max(x, std::max(y, std::max(z, w))); }

    [[nodiscard]] float fade(float t);
    [[nodiscard]] double fade(double t);

    [[nodiscard]] float lerp(float t, float a, float b);
    [[nodiscard]] double lerp(double t, double a, double b);

    [[nodiscard]] float grad(int hash, float x, float y, float z);
    [[nodiscard]] double grad(int hash, double x, double y, double z);

    [[nodiscard]] glm::vec4 PaintersAlgorithm(const glm::vec4& paintColor, const glm::vec4& canvasColor);
    [[nodiscard]] sf::Color PaintersAlgorithm(const sf::Color& paintColor, const sf::Color& canvasColor);

    [[nodiscard]] bool rayIntersectSphere(const glm::vec3& position, float radius, const glm::vec3& A, const glm::vec3& rayVector);
};
#endif
