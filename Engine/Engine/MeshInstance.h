#pragma once
#ifndef MESH_INSTANCE_H
#define MESH_INSTANCE_H

#include "Engine_Math.h"
#include "BindableResource.h"

class Mesh;
class Material;
class Object;

class MeshInstanceAnimation{
    friend class AnimationProcessor;
    private:
        uint currentLoops;
        uint requestedLoops;
        float currentTime;
        float startTime;
        float endTime;
        std::string animName;
        Mesh* mesh;
    public:
        MeshInstanceAnimation(Mesh*,std::string _animName,float _startTime,float _duration);
        MeshInstanceAnimation(Mesh*,std::string _animName,float _startTime,float _endTime,uint requestedLoops);
        ~MeshInstanceAnimation();
};

class MeshInstance final: public BindableResource{
    private:
        class impl;
        std::unique_ptr<impl> m_i;
    public:
        MeshInstance(std::string& parentName,Mesh*,Material*,glm::vec3& = glm::vec3(0),glm::quat& = glm::quat(),glm::vec3& = glm::vec3(1.0));
        MeshInstance(std::string& parentName,std::string mesh,std::string mat,glm::vec3& = glm::vec3(0),glm::quat& = glm::quat(),glm::vec3& = glm::vec3(1.0));
        ~MeshInstance();

        Mesh* mesh();
        Material* material();
        glm::mat4& model();
        glm::vec3& position();
        glm::quat& orientation();
        glm::vec3& getScale();
        Object* parent();

        std::vector<MeshInstanceAnimation>& animationQueue();
        void playAnimation(const std::string& animName,float startTime);
        void playAnimation(const std::string& animName,float startTime,float endTime,uint requestedLoops);

        void setOrientation(glm::quat&);
        void setOrientation(float x,float y,float z);

        void setMesh(std::string);
        void setMesh(Mesh*);
        void setMaterial(std::string);
        void setMaterial(Material*);

        void setPosition(float x,float y,float z);
        void setScale(float x,float y,float z);
        void setPosition(glm::vec3&);
        void setScale(glm::vec3&);

        void translate(float x,float y,float z);
        void rotate(float pitch,float yaw,float roll);
        void scale(float x,float y,float z);

        void translate(glm::vec3&);
        void rotate(glm::vec3&);
        void scale(glm::vec3&);

        void update(float dt);
        void render();
};
#endif