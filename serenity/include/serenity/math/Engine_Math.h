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

namespace glm {

}

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
    void extractViewFrustumPlanesHartmannGribbs(Camera&);

    [[nodiscard]] inline float Float32From16(const uint16_t in) noexcept { return glm::unpackHalf1x16(in); }
    [[nodiscard]] inline uint16_t Float16From32(const float in) noexcept { return glm::packHalf1x16(in); }

    [[nodiscard]] inline glm::vec3 Float32From16(const glm::u16vec3 in) noexcept { return glm::vec3{ Float32From16(in.x), Float32From16(in.y), Float32From16(in.z) }; }
    [[nodiscard]] inline glm::u16vec3 Float16From32(const glm::vec3 in) noexcept { return glm::u16vec3{ Float16From32(in.x), Float16From32(in.y), Float16From32(in.z) }; }

    template<class T>
    [[nodiscard]] inline float ToFloat(const T in) noexcept { 
        if constexpr (std::is_same_v<T, uint16_t> || sizeof(uint16_t) == sizeof(T)) {
            return float(glm::unpackHalf1x16(in));
        } else {
            return float(in);
        }
    }
    template<class T>
    [[nodiscard]] inline glm::vec2 ToFloatVec2(const T in) noexcept {
        if constexpr (std::is_same_v<T, glm::u16vec2> || sizeof(glm::u16vec2) == sizeof(T)) {
            return glm::vec2{ glm::unpackHalf1x16(in[0]), glm::unpackHalf1x16(in[1]) };
        } else {
            return glm::vec2{ in };
        }
    }
    template<class T>
    [[nodiscard]] inline glm::vec3 ToFloatVec3(const T in) noexcept {
        if constexpr (std::is_same_v<T, glm::u16vec3> || sizeof(glm::u16vec3) == sizeof(T)) {
            return glm::vec3{ glm::unpackHalf1x16(in[0]), glm::unpackHalf1x16(in[1]), glm::unpackHalf1x16(in[2]) };
        } else {
            return glm::vec3{ in };
        }
    }

    template<class T = uint16_t>
    [[nodiscard]] inline T PackFloat(const float in) noexcept {
        if constexpr (std::is_same_v<T, uint16_t> || sizeof(uint16_t) == sizeof(T)) {
            return glm::packHalf1x16(in);
        } else {
            return in;
        }
    }
    template<class T = glm::u16vec2>
    [[nodiscard]] inline T PackFloatVec2(const glm::vec2& in) noexcept {
        if constexpr (std::is_same_v<T, glm::u16vec2> || sizeof(glm::u16vec2) == sizeof(T)) {
            return T{ glm::packHalf1x16(in[0]), glm::packHalf1x16(in[1]) };
        } else {
            return in;
        }
    }
    template<class T = glm::u16vec3>
    [[nodiscard]] inline T PackFloatVec3(const glm::vec3& in) noexcept {
        if constexpr (std::is_same_v<T, glm::u16vec3> || sizeof(glm::u16vec3) == sizeof(T)) {
            return T{ glm::packHalf1x16(in[0]), glm::packHalf1x16(in[1]), glm::packHalf1x16(in[2]) };
        } else {
            return in;
        }
    }

    [[nodiscard]] glm::vec2 rotate2DPoint(const glm::vec2& point, float angle, const glm::vec2& origin = glm::vec2{ 0.0f, 0.0f });

    [[nodiscard]] glm::quat toGLM(const btQuaternion&);
    [[nodiscard]] glm_vec3 toGLM(const btVector3&);
    [[nodiscard]] glm::vec3 toGLM(const aiVector3D&);
    [[nodiscard]] glm::mat4 toGLM(const aiMatrix4x4&);
    [[nodiscard]] glm::mat3 toGLM(const aiMatrix3x3&);
    [[nodiscard]] glm::quat toGLM(const aiQuaternion&);

    [[nodiscard]] btQuaternion toBT(const glm::quat&);
#if defined(ENGINE_HIGH_PRECISION)
    [[nodiscard]] btVector3 toBT(const glm_vec3&);
#endif
    [[nodiscard]] btVector3 toBT(const glm::vec3&);

    [[nodiscard]] bool rect_fully_contained(const glm::vec4& bigger, const glm::vec4& smaller) noexcept;
    [[nodiscard]] glm::vec4 rect_union(const glm::vec4& bigger, const glm::vec4& smaller) noexcept;
    [[nodiscard]] glm::vec3 getScreenCoordinates(const glm::vec3& position, const Camera&, bool clampToEdge = true);
    [[nodiscard]] glm::vec3 getScreenCoordinates(const glm::vec3& position, const Camera&, const glm::vec4& viewport, bool clampToEdge = true);
    [[nodiscard]] glm::vec3 getScreenCoordinates(const glm::vec3& position, const Camera&, const glm::mat4& view, const glm::mat4& projection, const glm::vec4& viewport, bool clampToEdge = true);

    [[nodiscard]] glm::vec3 midpoint(const glm::vec3& a, const glm::vec3& b);
    [[nodiscard]] glm::vec3 direction(const glm::vec3& eye, const glm::vec3& target);

    void translate(const btRigidBody&, btVector3&, bool local) noexcept;
    void rotate(glm::quat& orientation, float pitch, float yaw, float roll, bool local) noexcept;
    void rotatePitch(glm::quat& orientation, float pitch, bool local) noexcept;
    void rotateYaw(glm::quat& orientation, float yaw, bool local) noexcept;
    void rotateRoll(glm::quat& orientation, float roll, bool local) noexcept;
    void setRotation(glm::quat& orientation, float pitch, float yaw, float roll) noexcept;
#ifdef ENGINE_HIGH_PRECISION
    void setFinalModelMatrix(glm_mat4& modelMatrix, const glm_vec3& position, const glm::quat& rotation, const glm::vec3& scale) noexcept;
#endif
    void setFinalModelMatrix(glm::mat4& modelMatrix, const glm::vec3& position, const glm::quat& rotation, const glm::vec3& scale) noexcept;

    template<class MATRIX, class VAL_TYPE>
    void setMatrixPosition(MATRIX& matrix, VAL_TYPE x, VAL_TYPE y, VAL_TYPE z) {
        matrix[3][0] = x;
        matrix[3][1] = y;
        matrix[3][2] = z;
    }
    template<class MATRIX, class VEC_TYPE>
    void setMatrixPosition(MATRIX& matrix, const VEC_TYPE& position) {
        matrix[3][0] = position[0];
        matrix[3][1] = position[1];
        matrix[3][2] = position[2];
    }


    [[nodiscard]] glm::vec3 getMatrixPosition(const glm::mat4& matrix);
#if defined(ENGINE_HIGH_PRECISION)
    [[nodiscard]] glm_vec3 getMatrixPosition(const glm_mat4& matrix);
#endif

    template<class MATRIX> void removeMatrixPosition(MATRIX& matrix) noexcept {
        matrix[3][0] = 0.0f;
        matrix[3][1] = 0.0f;
        matrix[3][2] = 0.0f;
    }
    //removes the scaling portion of the input matrix, and returns what the scaling was before the operation
    template<class MATRIX, class VECTOR3> VECTOR3 removeMatrixScale(MATRIX& matrix) noexcept {
        auto scaleX = glm::length(VECTOR3(matrix[0][0], matrix[0][1], matrix[0][2]));
        auto scaleY = glm::length(VECTOR3(matrix[1][0], matrix[1][1], matrix[1][2]));
        auto scaleZ = glm::length(VECTOR3(matrix[2][0], matrix[2][1], matrix[2][2]));
        matrix = glm::scale(matrix, VECTOR3(1.0f / scaleX, 1.0f / scaleY, 1.0f / scaleZ));
        return VECTOR3(scaleX, scaleY, scaleZ);
    }

    template<class QUAT> [[nodiscard]] inline glm::vec3 getForward(const QUAT& q) noexcept { return glm::normalize(q * glm::vec3{ 0.0f, 0.0f, -1.0f }); }
    template<class QUAT> [[nodiscard]] inline glm::vec3 getRight(const QUAT& q) noexcept { return glm::normalize(q * glm::vec3{ 1.0f, 0.0f, 0.0f }); }
    template<class QUAT> [[nodiscard]] inline glm::vec3 getUp(const QUAT& q) noexcept { return glm::normalize(q * glm::vec3{ 0.0f, 1.0f, 0.0f }); }


    [[nodiscard]] glm::vec3 getColumnVector(const btRigidBody&, uint32_t column);
    [[nodiscard]] glm::vec3 getForward(const btRigidBody&);
    [[nodiscard]] glm::vec3 getRight(const btRigidBody&);
    [[nodiscard]] glm::vec3 getUp(const btRigidBody&);

    template<class F, class R, class U> void recalculateForwardRightUp(const glm::quat& quat, F& f, R& r, U& u) noexcept {
        f = getForward(quat);
        r = getRight(quat);
        u = getUp(quat);
    }
    template<class F, class R, class U> void recalculateForwardRightUp(const btRigidBody& BTRigidBody, F& f, R& r, U& u) noexcept {
        f = getForward(BTRigidBody);
        r = getRight(BTRigidBody);
        u = getUp(BTRigidBody);
    }

    [[nodiscard]] float getAngleBetweenTwoVectors(const glm::vec3& a, const glm::vec3& b, bool degrees = true);


    [[nodiscard]] glm::quat alignTo(float x, float y, float z) noexcept;
    template<class VECTOR3> [[nodiscard]] inline glm::quat alignTo(const VECTOR3& direction) noexcept { 
        return alignTo(static_cast<float>(direction.x), static_cast<float>(direction.y), static_cast<float>(direction.z)); 
    }

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
    [[nodiscard]] std::array<uint8_t, 4> PaintersAlgorithm(const std::array<uint8_t, 4>& paintColor, const std::array<uint8_t, 4>& canvasColor);

    [[nodiscard]] bool rayIntersectSphere(const glm::vec3& position, float radius, const glm::vec3& A, const glm::vec3& rayVector);
};
#endif
