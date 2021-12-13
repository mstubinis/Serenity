#pragma once
#ifndef ENGINE_DEPENDENCIES_GLM_H
#define ENGINE_DEPENDENCIES_GLM_H

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/mat3x3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtc/matrix_access.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/integer.hpp>

#ifndef ENGINE_HIGH_PRECISION
    using decimal  = float;
    using glm_vec2 = glm::fvec2;
    using glm_vec3 = glm::fvec3;
    using glm_vec4 = glm::fvec4;
    using glm_quat = glm::fquat;
    using glm_mat4 = glm::fmat4;
    using glm_mat3 = glm::fmat3;
    using glm_mat2 = glm::fmat2;
#else
    #ifndef BT_USE_DOUBLE_PRECISION
        #define BT_USE_DOUBLE_PRECISION
    #endif
    using decimal  = double;
    using glm_vec2 = glm::dvec2;
    using glm_vec3 = glm::dvec3;
    using glm_vec4 = glm::dvec4;
    using glm_quat = glm::dquat;
    using glm_mat4 = glm::dmat4;
    using glm_mat3 = glm::dmat3;
    using glm_mat2 = glm::dmat2;

    inline constexpr glm_vec3 operator* (const glm_vec3& lhs, const glm::vec3& rhs) noexcept { return lhs * glm_vec3{ rhs }; }
    inline constexpr glm_vec3 operator* (const glm::vec3& lhs, const glm_vec3& rhs) noexcept { return glm_vec3{ lhs } *rhs; }
    inline constexpr glm_vec3 operator+ (const glm_vec3& lhs, const glm::vec3& rhs) noexcept { return lhs + glm_vec3{ rhs }; }
    inline constexpr glm_vec3 operator+ (const glm::vec3& lhs, const glm_vec3& rhs) noexcept { return glm_vec3{ lhs } + rhs; }
    inline constexpr glm_vec3 operator- (const glm_vec3& lhs, const glm::vec3& rhs) noexcept { return lhs - glm_vec3{ rhs }; }
    inline constexpr glm_vec3 operator- (const glm::vec3& lhs, const glm_vec3& rhs) noexcept { return glm_vec3{ lhs } - rhs; }
    inline constexpr glm_vec3 operator/ (const glm_vec3& lhs, const glm::vec3& rhs) noexcept { return lhs / glm_vec3{ rhs }; }
    inline constexpr glm_vec3 operator/ (const glm::vec3& lhs, const glm_vec3& rhs) noexcept { return glm_vec3{ lhs } / rhs; }

    inline constexpr glm_quat operator* (const glm_quat& lhs, const glm::quat& rhs) noexcept { return lhs * glm_quat{ rhs }; }
    inline constexpr glm_quat operator* (const glm::quat& lhs, const glm_quat& rhs) noexcept { return glm_quat{ lhs } *rhs; }
    inline constexpr glm_quat operator+ (const glm_quat& lhs, const glm::quat& rhs) noexcept { return lhs + glm_quat{ rhs }; }
    inline constexpr glm_quat operator+ (const glm::quat& lhs, const glm_quat& rhs) noexcept { return glm_quat{ lhs } + rhs; }
    inline constexpr glm_quat operator- (const glm_quat& lhs, const glm::quat& rhs) noexcept { return lhs - glm_quat{ rhs }; }
    inline constexpr glm_quat operator- (const glm::quat& lhs, const glm_quat& rhs) noexcept { return glm_quat{ lhs } - rhs; }


    inline constexpr glm::vec3 operator* (const glm::vec3& lhs, const glm_quat& rhs) noexcept { return lhs * glm::quat{ rhs }; }
    inline constexpr glm_vec3 operator* (const glm_vec3& lhs, const glm::quat& rhs) noexcept { return lhs * glm_quat{ rhs }; }

    inline constexpr glm::vec3 operator* (const glm_quat& lhs, const glm::vec3& rhs) noexcept { return glm::quat{ lhs } * rhs; }
    inline constexpr glm_vec3 operator* (const glm::quat& lhs, const glm_vec3& rhs) noexcept { return glm_quat{ lhs } * rhs; }

#endif

#include <LinearMath/btVector3.h>

inline constexpr glm::vec3 operator* (const glm::vec3& lhs, const btVector3& rhs) { return lhs * glm::vec3{ rhs.x(), rhs.y(), rhs.z() }; }
inline btVector3 operator* (const btVector3& lhs, const glm::vec3& rhs) { return lhs * btVector3(rhs.x, rhs.y, rhs.z); }

inline constexpr glm::vec3 operator/ (const glm::vec3& lhs, const btVector3& rhs) { return lhs / glm::vec3{ rhs.x(), rhs.y(), rhs.z() }; }
inline btVector3 operator/ (const btVector3& lhs, const glm::vec3& rhs) { return lhs / btVector3(rhs.x, rhs.y, rhs.z); }

inline constexpr glm::vec3 operator+ (const glm::vec3& lhs, const btVector3& rhs) { return lhs + glm::vec3{ rhs.x(), rhs.y(), rhs.z() }; }
inline btVector3 operator+ (const btVector3& lhs, const glm::vec3& rhs) { return lhs + btVector3(rhs.x, rhs.y, rhs.z); }

inline constexpr glm::vec3 operator- (const glm::vec3& lhs, const btVector3& rhs) { return lhs - glm::vec3{ rhs.x(), rhs.y(), rhs.z() }; }
inline btVector3 operator- (const btVector3& lhs, const glm::vec3& rhs) { return lhs - btVector3(rhs.x, rhs.y, rhs.z); }

#endif