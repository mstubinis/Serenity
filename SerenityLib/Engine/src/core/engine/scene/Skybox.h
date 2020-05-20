#pragma once
#ifndef ENGINE_SKYBOX_H
#define ENGINE_SKYBOX_H

#include <core/engine/events/Engine_EventObject.h>
#include <core/engine/utils/Utils.h>

class Texture;
class Mesh;
namespace Engine::priv {
    class SkyboxImplInterface;
};
class Skybox: public EventObserver{
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