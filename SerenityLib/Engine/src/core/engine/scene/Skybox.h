#pragma once
#ifndef ENGINE_SKYBOX_H
#define ENGINE_SKYBOX_H

class Mesh;
namespace Engine::priv {
    class SkyboxImplInterface;
};

#include <core/engine/events/Observer.h>
#include <core/engine/utils/Utils.h>
#include <core/engine/resources/Handle.h>

class Skybox: public Observer{
    friend class Engine::priv::SkyboxImplInterface;
    private:
        Handle  m_Texture = Handle{};
    public:
        static void bindMesh();
    public:
        Skybox(const std::string* files);
        Skybox(const std::string& file);
        virtual ~Skybox();

        virtual void update() {}

        inline CONSTEXPR Handle texture() const noexcept { return m_Texture; }

        void onEvent(const Event& e);
};
#endif