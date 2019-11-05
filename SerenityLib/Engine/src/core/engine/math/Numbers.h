#pragma once
#ifndef ENGINE_MATH_NUMBERS_H
#define ENGINE_MATH_NUMBERS_H

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat3x3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/quaternion.hpp>

#ifndef ENGINE_HIGH_PRECISION
    typedef float       decimal;
    typedef glm::vec2   glm_vec2;
    typedef glm::vec3   glm_vec3;
    typedef glm::vec4   glm_vec4;
    typedef glm::quat   glm_quat;
    typedef glm::mat4   glm_mat3;
    typedef glm::mat4   glm_mat4;
#else
    //TODO: find a way to get bullet to use double precision without compiler / linker errors
    #define BT_USE_DOUBLE_PRECISION

    typedef double      decimal;
    typedef glm::dvec2  glm_vec2;
    typedef glm::dvec3  glm_vec3;
    typedef glm::dvec4  glm_vec4;
    typedef glm::dquat  glm_quat;
    typedef glm::dmat3  glm_mat3;
    typedef glm::dmat4  glm_mat4;
#endif

#endif