#pragma once
#ifndef ENGINE_INCLUDES_H
#define ENGINE_INCLUDES_H

class GameCore;
namespace Engine::priv {
    class PhysicsModule;
    class EditorCore;
    class RenderModule;
    class ResourceManager;
}

#include <serenity/types/ViewPointer.h>

namespace Engine {
    [[nodiscard]] Engine::view_ptr<GameCore> getGameCore() noexcept;
    [[nodiscard]] Engine::priv::ResourceManager& getResourceManager() noexcept;
    [[nodiscard]] Engine::priv::PhysicsModule& getPhysicsManager() noexcept;
    [[nodiscard]] Engine::priv::EditorCore& getEditor() noexcept;
    [[nodiscard]] Engine::priv::RenderModule& getRenderer() noexcept;
}

#endif