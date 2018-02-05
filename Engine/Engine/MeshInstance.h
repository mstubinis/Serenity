#pragma once
#ifndef ENGINE_MESH_INSTANCE_H
#define ENGINE_MESH_INSTANCE_H

#include "Engine_Math.h"
#include "BindableResource.h"

class Mesh;
class Material;
class MeshInstanceAnimation final{
	friend struct DefaultMeshInstanceBindFunctor;
	friend struct DefaultMeshInstanceUnbindFunctor;
    private:
		class impl; std::unique_ptr<impl> m_i;
    public:
        MeshInstanceAnimation(Mesh*,const std::string& animName,float startTime,float endTime,uint requestedLoops = 1);
        ~MeshInstanceAnimation();
};
class MeshInstance final: public BindableResource{
    private:
        class impl; std::unique_ptr<impl> m_i;
    public:
        MeshInstance(const std::string& parentName,Mesh*,Material*,glm::vec3& = glm::vec3(0),glm::quat& = glm::quat(),glm::vec3& = glm::vec3(1.0));
        MeshInstance(const std::string& parentName,std::string mesh,std::string mat,glm::vec3& = glm::vec3(0),glm::quat& = glm::quat(),glm::vec3& = glm::vec3(1.0));
        ~MeshInstance();

        Mesh* mesh();
        Material* material();
        glm::mat4& model();
        glm::vec3& position();
        glm::quat& orientation();
        glm::vec3& getScale();
        Object* parent();

        std::vector<MeshInstanceAnimation*>& animationQueue();
        void playAnimation(const std::string& animName,float startTime);
        void playAnimation(const std::string& animName,float startTime,float endTime,uint requestedLoops);

        void setOrientation(glm::quat&);
        void setOrientation(float x,float y,float z);

        void setMesh(const std::string&);
        void setMesh(Mesh*);
        void setMaterial(const std::string&);
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