#pragma once
#ifndef ENGINE_MESH_INSTANCE_H
#define ENGINE_MESH_INSTANCE_H

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/gtc/quaternion.hpp>
#include <core/engine/BindableResource.h>
#include <core/engine/events/Engine_EventObject.h>
#include <ecs/Entity.h>

struct Handle;
class  ShaderP;
class  Material;
class  Mesh;
class  MeshInstance;
class  ComponentModel;
namespace Engine{
    namespace epriv{
        struct DefaultMeshInstanceBindFunctor { void operator()(EngineResource* r) const; };
        struct DefaultMeshInstanceUnbindFunctor { void operator()(EngineResource* r) const; };
        struct MeshInstanceAnimation;
    };
};
class MeshInstance final: public BindableResource{
    friend struct ::Engine::epriv::DefaultMeshInstanceBindFunctor;
    friend struct ::Engine::epriv::DefaultMeshInstanceUnbindFunctor;
    friend class  ::ComponentModel;
    private:
        void*                                                m_UserPointer;
        std::vector<Engine::epriv::MeshInstanceAnimation*>   m_AnimationQueue;
        Entity                                               m_Parent;
        ShaderP*                                             m_ShaderProgram;
        Mesh*                                                m_Mesh;
        Material*                                            m_Material;
        RenderStage::Stage                                   m_Stage;
        glm::vec3                                            m_Position, m_Scale, m_GodRaysColor;
        glm::quat                                            m_Orientation;
        glm::mat4                                            m_ModelMatrix;
        glm::vec4                                            m_Color;
        bool                                                 m_PassedRenderCheck;
        bool                                                 m_Visible;

        void _init(Mesh* mesh, Material* mat, Entity& parent, ShaderP* program);
        void _updateModelMatrix();
    public:
        MeshInstance(Entity&, Mesh*, Material*, ShaderP* = 0);
        MeshInstance(Entity&, Handle mesh, Handle mat, ShaderP* = 0);
        MeshInstance(Entity&, Mesh*, Handle mat, ShaderP* = 0);
        MeshInstance(Entity&, Handle mesh, Material*, ShaderP* = 0);

        MeshInstance(const MeshInstance& other) = default;
        MeshInstance& operator=(const MeshInstance& other) = default;
        MeshInstance(MeshInstance&& other) noexcept = default;
        MeshInstance& operator=(MeshInstance&& other) noexcept = default;

        ~MeshInstance();

        ShaderP* shaderProgram();
        Mesh* mesh();
        Material* material();
        void* getUserPointer() { return m_UserPointer; }
        template<typename T> void setUserPointer(T* t) { m_UserPointer = t; }
        Entity& parent() { return m_Parent; }
        glm::vec4& color();
        glm::vec3& godRaysColor();
        glm::mat4& modelMatrix();
        glm::vec3& position();
        glm::quat& orientation();
        glm::vec3& getScale();

        bool visible();
        bool passedRenderCheck();
        void setPassedRenderCheck(bool);
        void show();
        void hide();

        RenderStage::Stage stage();
        void setStage(RenderStage::Stage);

        void playAnimation(const std::string& animName,float startTime,float endTime = -1.0f, uint requestedLoops = 1);

        void setColor(float, float, float, float = 1.0f);
        void setColor(glm::vec4& color);
        void setColor(glm::vec3& color);

        void setGodRaysColor(float r,float g,float b);
        void setGodRaysColor(glm::vec3& color);

        void setShaderProgram(const Handle& shaderPHandle, ComponentModel&);
        void setShaderProgram(ShaderP*, ComponentModel&);

        void setMesh(const Handle& meshHandle, ComponentModel&);
        void setMesh(Mesh*, ComponentModel&);

        void setMaterial(const Handle& materialHandle, ComponentModel&);
        void setMaterial(Material*, ComponentModel&);

        void setPosition(float x,float y,float z);             void setPosition(glm::vec3&);
        void setOrientation(glm::quat&);                       void setOrientation(float x,float y,float z);
        void setScale(float x,float y,float z);                void setScale(glm::vec3&);

        void translate(float x,float y,float z);               void translate(glm::vec3&);
        void rotate(float pitch,float yaw,float roll);         void rotate(glm::vec3&);
        void scale(float x,float y,float z);                   void scale(glm::vec3&);
};

#endif