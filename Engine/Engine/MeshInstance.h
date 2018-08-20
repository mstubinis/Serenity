#pragma once
#ifndef ENGINE_MESH_INSTANCE_H
#define ENGINE_MESH_INSTANCE_H

#include "Engine_Math.h"
#include "BindableResource.h"

struct Handle;

class Entity;
class Material;
class Mesh;
class MeshInstanceAnimation;
class MeshInstance;

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
        MeshInstanceAnimation(const MeshInstanceAnimation&); // non construction-copyable
        MeshInstanceAnimation& operator=(const MeshInstanceAnimation&); // non copyable
    public:
        MeshInstanceAnimation(Mesh*,const std::string& animName,float startTime,float endTime,uint requestedLoops = 1);
        ~MeshInstanceAnimation();
};
class MeshInstance final: public BindableResource{
    friend struct Engine::epriv::DefaultMeshInstanceBindFunctor;
    friend struct Engine::epriv::DefaultMeshInstanceUnbindFunctor;
    private:
        class impl; std::unique_ptr<impl> m_i;
        MeshInstance(const MeshInstance&); // non construction-copyable
        MeshInstance& operator=(const MeshInstance&); // non copyable
    public:
        MeshInstance(Entity*,Mesh*,Material*,glm::vec3& = glm::vec3(0.0f),glm::quat& = glm::quat(),glm::vec3& = glm::vec3(1.0f));
        MeshInstance(Entity*,Handle mesh,Handle mat,glm::vec3& = glm::vec3(0.0f),glm::quat& = glm::quat(),glm::vec3& = glm::vec3(1.0f));
        MeshInstance(Entity*,Mesh*,Handle mat,glm::vec3& = glm::vec3(0.0f),glm::quat& = glm::quat(),glm::vec3& = glm::vec3(1.0f));
        MeshInstance(Entity*,Handle mesh,Material*,glm::vec3& = glm::vec3(0.0f),glm::quat& = glm::quat(),glm::vec3& = glm::vec3(1.0f));
        ~MeshInstance();

        Mesh* mesh();
        Material* material();
        Entity* parent();
        glm::vec4& color();
        glm::vec3& godRaysColor();
        glm::mat4& model();
        glm::vec3& position();
        glm::quat& orientation();
        glm::vec3& getScale();

        std::vector<MeshInstanceAnimation*>& animationQueue();
        void playAnimation(const std::string& animName,float startTime);
        void playAnimation(const std::string& animName,float startTime,float endTime,uint requestedLoops);

        void setColor(float r,float g,float b,float a = 1.0f);
        void setColor(glm::vec4& color);

        void setGodRaysColor(float r,float g,float b);
        void setGodRaysColor(glm::vec3& color);

        void setMesh(Handle& meshHandle);                void setMesh(Mesh*);
        void setMaterial(Handle& materialHandle);        void setMaterial(Material*);

        void setPosition(float x,float y,float z);       void setPosition(glm::vec3&);
        void setOrientation(glm::quat&);                 void setOrientation(float x,float y,float z);
        void setScale(float x,float y,float z);          void setScale(glm::vec3&);

        void translate(float x,float y,float z);         void translate(glm::vec3&);
        void rotate(float pitch,float yaw,float roll);   void rotate(glm::vec3&);
        void scale(float x,float y,float z);             void scale(glm::vec3&);

        void update(const float& dt);
        void render();
};
#endif
