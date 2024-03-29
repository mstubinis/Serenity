#pragma once
#ifndef ENGINE_SKYBOX_H
#define ENGINE_SKYBOX_H

class Mesh;
namespace Engine::priv {
    class SkyboxImplInterface;
};

#include <serenity/events/Observer.h>
#include <serenity/utils/Utils.h>
#include <serenity/resources/Handle.h>

class Skybox: public Observer {
    friend class Engine::priv::SkyboxImplInterface;
    private:
        Handle  m_TextureCubemap = Handle{};
    public:
        static void bindMesh();
    public:
        Skybox(const std::array<std::string_view, 6>& files);
        Skybox(std::string_view file);
        virtual ~Skybox();

        virtual void update() {}

        [[nodiscard]] inline constexpr Handle cubemap() const noexcept { return m_TextureCubemap; }

        void onEvent(const Event&);
};
#endif