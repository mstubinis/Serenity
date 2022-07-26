#pragma once
#ifndef ENGINE_RENDERER_CULLING_DISTANCE_TEST_H
#define ENGINE_RENDERER_CULLING_DISTANCE_TEST_H

class Camera;

#include <serenity/dependencies/glm.h>

namespace Engine::priv::Culling {
    //returns true if the object should be visible based on its distance from the camera
    bool distanceTest(const glm_vec3& objectPosition, const float objectRadius, const glm_vec3& cameraPosition);
    //returns true if the object should be visible based on its distance from the camera
    bool distanceTest(const glm_vec3& objectPosition, const float objectRadius, const Camera&);
    //returns true if the object should be visible based on its distance from the camera
    bool distanceTest(const glm_vec3& objectPosition, const float objectRadius, const Camera* const);
}

#endif