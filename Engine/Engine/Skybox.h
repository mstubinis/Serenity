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
        SkyboxEmpty(Scene* = nullptr);
        virtual ~SkyboxEmpty();
        virtual void update(){}
        virtual void draw(){}
};
class Skybox: public SkyboxEmpty{
    protected:
        Texture* m_Texture;
        static GLuint m_Buffer;
		static GLuint m_VAO;
        static std::vector<glm::vec3> m_Vertices;
    public:
        Skybox(std::string* files, Scene* = nullptr);
        Skybox(std::string file, Scene* = nullptr);
        virtual ~Skybox();
        virtual void update(){}
        virtual void draw(){}
        Texture* texture(){ return m_Texture; }
        static void initMesh();
        static void bindMesh();
};
#endif