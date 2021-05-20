#pragma once
#ifndef ENGINE_RENDERER_CULLING_SPHERE_INTERSECT_TEST_H
#define ENGINE_RENDERER_CULLING_SPHERE_INTERSECT_TEST_H

class Camera;
class ComponentCamera;

#include <serenity/dependencies/glm.h>
#include <array>

namespace Engine::priv::Culling {

    class SphereIntersectTest final {
        public:
            //returns 0 if outside the frustum, 1 if inside the frustum, and 2 if intersecting the frustum
            static int sphereIntersectTest(const glm_vec3& position, const float radius, const std::array<glm::vec4, 6>& frustumPlanes);
            //returns 0 if outside the frustum, 1 if inside the frustum, and 2 if intersecting the frustum
            static int sphereIntersectTest(const glm_vec3& position, const float radius, const Camera&);
            //returns 0 if outside the frustum, 1 if inside the frustum, and 2 if intersecting the frustum
            static int sphereIntersectTest(const glm_vec3& position, const float radius, const ComponentCamera&);
    };
    //returns 0 if outside the frustum, 1 if inside the frustum, and 2 if intersecting the frustum
    int sphereIntersectTest(const glm_vec3& position, const float radius, const std::array<glm::vec4, 6>& frustumPlanes);
    //returns 0 if outside the frustum, 1 if inside the frustum, and 2 if intersecting the frustum
    int sphereIntersectTest(const glm_vec3& position, const float radius, const Camera&);
    //returns 0 if outside the frustum, 1 if inside the frustum, and 2 if intersecting the frustum
    int sphereIntersectTest(const glm_vec3& position, const float radius, const ComponentCamera&);
    //returns 0 if outside the frustum, 1 if inside the frustum, and 2 if intersecting the frustum
    int sphereIntersectTest(const glm_vec3& position, const float radius, const Camera* const);
}

#endif