#pragma once
#ifndef ENGINE_RENDERER_CULLING_POINT_INTERSECT_TEST_H
#define ENGINE_RENDERER_CULLING_POINT_INTERSECT_TEST_H

class Camera;
class ComponentCamera;

#include <serenity/dependencies/glm.h>

namespace Engine::priv::Culling {
    //returns 0 if outside the frustum, 1 if inside the frustum
    int pointIntersectTest(const glm_vec3& position, const std::array<glm::vec4, 6>& frustumPlanes);
    //returns 0 if outside the frustum, 1 if inside the frustum
    int pointIntersectTest(const glm_vec3& position, const Camera&);
    //returns 0 if outside the frustum, 1 if inside the frustum
    int pointIntersectTest(const glm_vec3& position, const ComponentCamera&);
    //returns 0 if outside the frustum, 1 if inside the frustum
    int pointIntersectTest(const glm_vec3& position, const Camera* const);

    //returns 0 if outside the frustum, 1 if inside the frustum
    int pointIntersectTest(const glm_vec3& position, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix);
    //returns 0 if outside the frustum, 1 if inside the frustum
    int pointIntersectTest(const glm_vec3& position, const glm::mat4& viewProjectionMatrix);
}

#endif