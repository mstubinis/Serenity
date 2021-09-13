#pragma once
#ifndef ENGINE_RENDERER_CULLING_H
#define ENGINE_RENDERER_CULLING_H

class Scene;
class Viewport;
class Camera;
class ModelInstance;

#include <serenity/dependencies/glm.h>
#include <vector>
#include <array>

namespace Engine::priv::Culling {
    void cull(const Camera* const, const Viewport* const, const std::vector<ModelInstance*>&);
    void cull(const Camera* const, const std::vector<ModelInstance*>&);

    void cull(const glm::mat4& viewProjMatrix, const Viewport* const, const std::vector<ModelInstance*>&);
    void cull(const glm::mat4& viewMatrix, const glm::mat4& projMatrix, const Viewport* const, const std::vector<ModelInstance*>&);

    void cull(const glm::mat4& viewProjMatrix, const std::vector<ModelInstance*>&);
    void cull(const glm::mat4& viewMatrix, const glm::mat4& projMatrix, const std::vector<ModelInstance*>&);

    void cull(const Viewport* const, const std::vector<ModelInstance*>&);
    void cull(const std::vector<ModelInstance*>&);

    const std::array<glm::vec4, 6>& getFrustumPlanes(const Camera& camera);
    std::array<glm::vec4, 6> getFrustumPlanes(const glm::mat4& viewProjectionMatrix);
}

#endif