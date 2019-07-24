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
        struct DefaultMeshInstanceBindFunctor;
        struct DefaultMeshInstanceUnbindFunctor;
        struct MeshInstanceAnimation;
    };
};
class MeshInstance final: public BindableResource{
    friend struct Engine::epriv::DefaultMeshInstanceBindFunctor;
    friend struct Engine::epriv::DefaultMeshInstanceUnbindFunctor;
    friend class  ComponentModel;
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

        void internalInit(Mesh* mesh, Material* mat, Entity& parent, ShaderP* program);
        void internalUpdateModelMatrix();
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
        void* getUserPointer() const;
        void setUserPointer(void* t);
        Entity& parent();

        const glm::vec4& color() const;
        const glm::vec3& godRaysColor() const;
        const glm::mat4& modelMatrix() const;
        const glm::vec3& position() const;
        const glm::quat& orientation() const;
        const glm::vec3& getScale() const;

        const bool& visible() const;
        const bool& passedRenderCheck() const;
        void setPassedRenderCheck(const bool&);
        void show();
        void hide();

        const RenderStage::Stage& stage() const;
        void setStage(const RenderStage::Stage& stage);

        void playAnimation(const std::string& animName, const float& startTime, const float& endTime = -1.0f, const uint& requestedLoops = 1);

        void setColor(const float& r, const float& g, const float& b, const float& a = 1.0f);
        void setColor(const glm::vec4& color);
        void setColor(const glm::vec3& color);

        void setGodRaysColor(const float& r, const float& g, const float& b);
        void setGodRaysColor(const glm::vec3& color);

        void setShaderProgram(const Handle& shaderPHandle, ComponentModel&);
        void setShaderProgram(ShaderP*, ComponentModel&);

        void setMesh(const Handle& meshHandle, ComponentModel&);
        void setMesh(Mesh*, ComponentModel&);

        void setMaterial(const Handle& materialHandle, ComponentModel&);
        void setMaterial(Material*, ComponentModel&);

        void setPosition(const float& x, const float& y, const float& z);             void setPosition(const glm::vec3& position);
        void setOrientation(const glm::quat& orientation);                            void setOrientation(const float& x, const float& y, const float& z);
        void setScale(const float& x, const float& y, const float& z);                void setScale(const glm::vec3& scale);

        void translate(const float& x, const float& y, const float& z);               void translate(const glm::vec3& translation);
        void rotate(const float& pitch, const float& yaw, const float& roll);         void rotate(const glm::vec3& rotation);
        void scale(const float& x, const float& y, const float& z);                   void scale(const glm::vec3& scale);
};

#endif