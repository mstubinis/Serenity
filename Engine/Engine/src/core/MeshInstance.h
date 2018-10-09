#pragma once
#ifndef ENGINE_MESH_INSTANCE_H_INCLUDE_GUARD
#define ENGINE_MESH_INSTANCE_H_INCLUDE_GUARD

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/gtc/quaternion.hpp>
#include "core/engine/BindableResource.h"
#include "core/engine/Engine_EventObject.h"

struct Handle;
class OLD_Entity;
struct Entity;
class ShaderP;
class Material;
class Mesh;
class MeshInstance;
namespace Engine{
    namespace epriv{
        struct DefaultMeshInstanceBindFunctor { void operator()(EngineResource* r) const; };
        struct DefaultMeshInstanceUnbindFunctor { void operator()(EngineResource* r) const; };
        struct MeshInstanceAnimation;
    };
};
class MeshInstance final: public BindableResource{
    friend struct Engine::epriv::DefaultMeshInstanceBindFunctor;
    friend struct Engine::epriv::DefaultMeshInstanceUnbindFunctor;
    private:
        std::vector<Engine::epriv::MeshInstanceAnimation*> m_AnimationQueue;
        OLD_Entity* m_Entity;
        ShaderP* m_ShaderProgram;
        Mesh* m_Mesh;
        Material* m_Material;
        RenderStage::Stage m_Stage;
        glm::vec3 m_Position, m_Scale, m_GodRaysColor;
        glm::quat m_Orientation;
        glm::mat4 m_Model;
        glm::vec4 m_Color;
        bool m_PassedRenderCheck, m_Visible;

        void _init(Mesh* mesh, Material* mat, OLD_Entity& entity, ShaderP* program);
        void _init(Mesh* mesh, Material* mat, Entity& entity, ShaderP* program);
        void _updateModelMatrix();
    public:
        MeshInstance(OLD_Entity&, Mesh*,       Material*,  ShaderP* = 0);
        MeshInstance(OLD_Entity&, Handle mesh, Handle mat, ShaderP* = 0);
        MeshInstance(OLD_Entity&, Mesh*,       Handle mat, ShaderP* = 0);
        MeshInstance(OLD_Entity&, Handle mesh, Material*,  ShaderP* = 0);
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
        OLD_Entity* parent();
        glm::vec4& color();
        glm::vec3& godRaysColor();
        glm::mat4& model();
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

        void setColor(float r,float g,float b,float a = 1.0f);
        void setColor(glm::vec4 color);

        void setGodRaysColor(float r,float g,float b);
        void setGodRaysColor(glm::vec3 color);

        void setShaderProgram(const Handle& shaderPHandle);    void setShaderProgram(ShaderP*);
        void setMesh(const Handle& meshHandle);                void setMesh(Mesh*);
        void setMaterial(const Handle& materialHandle);        void setMaterial(Material*);

        void setPosition(float x,float y,float z);             void setPosition(glm::vec3);
        void setOrientation(glm::quat);                        void setOrientation(float x,float y,float z);
        void setScale(float x,float y,float z);                void setScale(glm::vec3);

        void translate(float x,float y,float z);               void translate(glm::vec3);
        void rotate(float pitch,float yaw,float roll);         void rotate(glm::vec3);
        void scale(float x,float y,float z);                   void scale(glm::vec3);
};
#endif
