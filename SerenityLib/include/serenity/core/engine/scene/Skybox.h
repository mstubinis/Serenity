#pragma once
#ifndef ENGINE_SKYBOX_H
#define ENGINE_SKYBOX_H

class Mesh;
namespace Engine::priv {
    class SkyboxImplInterface;
};

#include <serenity/core/engine/events/Observer.h>
#include <serenity/core/engine/utils/Utils.h>
#include <serenity/core/engine/resources/Handle.h>

class Skybox: public Observer{
    friend class Engine::priv::SkyboxImplInterface;
    private:
        Handle  m_Texture = Handle{};
    public:
        static void bindMesh();
    public:
        Skybox(const std::array<std::string, 6>& files);
        Skybox(const std::string& file);
        virtual ~Skybox();

        virtual void update() {}

        inline constexpr Handle texture() const noexcept { return m_Texture; }

        void onEvent(const Event& e);
};
#endif