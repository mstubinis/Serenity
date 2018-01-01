#pragma once
#ifndef ENGINE_SKYBOX_H
#define ENGINE_SKYBOX_H

#include <string>
#include <vector>
#include <glm/glm.hpp>

typedef unsigned int GLuint;
typedef unsigned int uint;

class Scene;
class Texture;
class Mesh;

class ISkybox{
    public:
        virtual ~ISkybox(){}
        virtual void update() = 0;
        virtual void draw() = 0;
        virtual Texture* texture() = 0;
};
class SkyboxEmpty: public ISkybox{
    public:
        SkyboxEmpty(std::string name,Scene* = nullptr);
        virtual ~SkyboxEmpty();
        virtual void update(){}
        virtual void draw(){}
};
class Skybox: public SkyboxEmpty{
    protected:
        Texture* m_Texture;
        static GLuint m_Buffer;
        static std::vector<glm::vec3> m_Vertices;
    public:
        Skybox(std::string name,Scene* = nullptr);
        virtual ~Skybox();
        Texture* texture(){ return m_Texture; }
        virtual void update();
        virtual void draw();
		static void initMesh();
		static void bindMesh();
};
#endif