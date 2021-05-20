#pragma once
#ifndef ENGINE_RENDERER_CULLING_H
#define ENGINE_RENDERER_CULLING_H

class Scene;
class Viewport;
class Camera;
class ModelInstance;

#include <serenity/dependencies/glm.h>
#include <vector>

namespace Engine::priv::Culling {
    void cull(Camera*, Viewport*, const std::vector<ModelInstance*>&);
}

#endif