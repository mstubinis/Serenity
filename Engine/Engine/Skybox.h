#pragma once
#ifndef ENGINE_SKYBOX_H
#define ENGINE_SKYBOX_H

#include <string>
#include <vector>
#include <memory>
#include <glm/glm.hpp>
#include "Engine_EventObject.h"

typedef unsigned int GLuint;
typedef unsigned int uint;

class Scene;
class Texture;
class Mesh;

namespace Engine {
    namespace epriv {
        class SkyboxImplInterface;
    };
};

class ISkybox{public:
    virtual ~ISkybox(){}
    virtual void update() = 0;
    virtual void draw() = 0;
    virtual Texture* texture() = 0;
};
class SkyboxEmpty: public ISkybox{public:
    SkyboxEmpty(Scene* = nullptr);
    virtual ~SkyboxEmpty();
	virtual void update();
	virtual void draw();
};
class Skybox: public SkyboxEmpty, public EventObserver{
    friend class Engine::epriv::SkyboxImplInterface;
    private:
        class impl; std::unique_ptr<impl> m_i;
    public:
        Skybox(std::string* files, Scene* = nullptr);
        Skybox(std::string file, Scene* = nullptr);
        virtual ~Skybox();
		virtual void update();
		virtual void draw();
        Texture* texture();
        static void bindMesh();

        void onEvent(const Event& e);
};
#endif