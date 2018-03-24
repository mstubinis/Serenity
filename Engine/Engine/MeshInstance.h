#pragma once
#ifndef ENGINE_MESH_INSTANCE_H
#define ENGINE_MESH_INSTANCE_H

#include "Engine_Math.h"
#include "BindableResource.h"

class Mesh;
class Material;
struct Handle;
class Entity;

namespace Engine{
	namespace epriv{
		struct DefaultMeshInstanceBindFunctor;
		struct DefaultMeshInstanceUnbindFunctor;
	};
};

class MeshInstanceAnimation final{
	friend struct Engine::epriv::DefaultMeshInstanceBindFunctor;
	friend struct Engine::epriv::DefaultMeshInstanceUnbindFunctor;
    private:
		class impl; std::unique_ptr<impl> m_i;
    public:
        MeshInstanceAnimation(Mesh*,const std::string& animName,float startTime,float endTime,uint requestedLoops = 1);
        ~MeshInstanceAnimation();
};
class MeshInstance final: public BindableResource{
	friend struct Engine::epriv::DefaultMeshInstanceBindFunctor;
	friend struct Engine::epriv::DefaultMeshInstanceUnbindFunctor;
    private:
        class impl; std::unique_ptr<impl> m_i;
    public:
        MeshInstance(const std::string& parentName,Mesh*,Material*,glm::vec3& = glm::vec3(0),glm::quat& = glm::quat(),glm::vec3& = glm::vec3(1.0));
        MeshInstance(const std::string& parentName,Handle mesh,Handle mat,glm::vec3& = glm::vec3(0),glm::quat& = glm::quat(),glm::vec3& = glm::vec3(1.0));
        MeshInstance(Entity*,Mesh*,Material*,glm::vec3& = glm::vec3(0),glm::quat& = glm::quat(),glm::vec3& = glm::vec3(1.0));
        MeshInstance(Entity*,Handle mesh,Handle mat,glm::vec3& = glm::vec3(0),glm::quat& = glm::quat(),glm::vec3& = glm::vec3(1.0));
        ~MeshInstance();

        Mesh* mesh();
		Object* parent();
		Entity* entity();
		glm::vec4& color();
		glm::vec3& godRaysColor();
        Material* material();
		glm::mat4& model();
        glm::vec3& position();
        glm::quat& orientation();
        glm::vec3& getScale();

        std::vector<MeshInstanceAnimation*>& animationQueue();
        void playAnimation(const std::string& animName,float startTime);
        void playAnimation(const std::string& animName,float startTime,float endTime,uint requestedLoops);

        void setMesh(Handle& meshHandle);                void setMesh(Mesh*);
        void setMaterial(Handle& materialHandle);        void setMaterial(Material*);

        void setPosition(float x,float y,float z);       void setPosition(glm::vec3&);
        void setOrientation(glm::quat&);                 void setOrientation(float x,float y,float z);
        void setScale(float x,float y,float z);          void setScale(glm::vec3&);

        void translate(float x,float y,float z);         void translate(glm::vec3&);
        void rotate(float pitch,float yaw,float roll);   void rotate(glm::vec3&);
        void scale(float x,float y,float z);             void scale(glm::vec3&);

        void update(float dt);
        void render();
};
#endif