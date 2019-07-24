#pragma once
#ifndef ENGINE_SKYBOX_H
#define ENGINE_SKYBOX_H

#include <string>
#include <core/engine/events/Engine_EventObject.h>
#include <core/engine/Engine_Utils.h>

typedef unsigned int     GLuint;

class Texture;
class Mesh;
namespace Engine {
namespace epriv {
    class SkyboxImplInterface;
};
};

class Skybox: public EventObserver{
    friend class Engine::epriv::SkyboxImplInterface;
    private:
        Texture*  m_Texture;
    public:
        Skybox(const std::string* files);
        Skybox(const std::string& file);
        virtual ~Skybox();

        virtual void update();
        virtual void draw();

        Texture* texture();

        static void bindMesh();

        void onEvent(const Event& e);
};
#endif