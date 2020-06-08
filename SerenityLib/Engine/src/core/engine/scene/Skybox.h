#pragma once
#ifndef ENGINE_SKYBOX_H
#define ENGINE_SKYBOX_H

class Texture;
class Mesh;
namespace Engine::priv {
    class SkyboxImplInterface;
};

#include <core/engine/events/Observer.h>
#include <core/engine/utils/Utils.h>

class Skybox: public Observer{
    friend class Engine::priv::SkyboxImplInterface;
    private:
        Texture*  m_Texture = nullptr;
    public:
        static void bindMesh();
    public:
        Skybox(const std::string* files);
        Skybox(const std::string& file);
        virtual ~Skybox();

        virtual void update();

        Texture* texture() const;

        void onEvent(const Event& e);
};
#endif