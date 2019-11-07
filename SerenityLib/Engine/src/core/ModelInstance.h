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
class  ShaderProgram;
class  Material;
class  Mesh;
class  ComponentModel;
class  Viewport;
class  ModelInstance;
namespace Engine{
    namespace epriv{
        struct DefaultModelInstanceBindFunctor;
        struct DefaultModelInstanceUnbindFunctor;
        struct ModelInstanceAnimation;

        struct InternalModelInstancePublicInterface final {
            static const bool IsViewportValid(ModelInstance&, Viewport&);
        };
    };
};

struct ModelDrawingMode final { enum Mode {
    Triangles = GL_TRIANGLES,
    TriangleStrip = GL_TRIANGLE_STRIP,
    TriangleFan = GL_TRIANGLE_FAN,
    Points = GL_POINTS,
    LineStrip = GL_LINE_STRIP,
    LineLoop = GL_LINE_LOOP,
    Lines = GL_LINES,
    Patches = GL_PATCHES,

//GL_LINE_STRIP_ADJACENCY, GL_LINES_ADJACENCY, GL_TRIANGLE_STRIP_ADJACENCY and GL_TRIANGLES_ADJACENCY are available only if the GL version is 3.2 or greater.
    TriangleStripAdjacency = GL_TRIANGLE_STRIP_ADJACENCY,
    TrianglesAdjacency = GL_TRIANGLES_ADJACENCY,
    LineStripAdjacency = GL_LINE_STRIP_ADJACENCY,
    LinesAdjacency = GL_LINES_ADJACENCY,
};};

struct ViewportFlag final { enum Flag: unsigned int {
    None    = 0,
    _1 = 1 << 0,
    _2 = 1 << 1,
    _3 = 1 << 2,
    _4 = 1 << 3,
    _5 = 1 << 4,
    _6 = 1 << 5,
    _7 = 1 << 6,
    _8 = 1 << 7,
    _9 = 1 << 8,
    _10 = 1 << 9,
    _11 = 1 << 10,
    _12 = 1 << 11,
    _13 = 1 << 13,
    _14 = 1 << 14,
    _15 = 1 << 15,
    All     = 4294967295,
};};

class ModelInstance final: public BindableResource{
    friend struct Engine::epriv::DefaultModelInstanceBindFunctor;
    friend struct Engine::epriv::DefaultModelInstanceUnbindFunctor;
    friend class  ComponentModel;
    private:
        ModelDrawingMode::Mode                               m_DrawingMode;
        static unsigned int                                  m_ViewportFlagDefault;
        unsigned int                                         m_ViewportFlag; //determine what viewports this can be seen in
        void*                                                m_UserPointer;
        std::vector<Engine::epriv::ModelInstanceAnimation*>  m_AnimationQueue;
        Entity                                               m_Parent;
        ShaderProgram*                                       m_ShaderProgram;
        Mesh*                                                m_Mesh;
        Material*                                            m_Material;
        RenderStage::Stage                                   m_Stage;
        glm::vec3                                            m_Position, m_Scale, m_GodRaysColor;
        glm::quat                                            m_Orientation;
        glm::mat4                                            m_ModelMatrix;
        glm::vec4                                            m_Color;
        bool                                                 m_PassedRenderCheck;
        bool                                                 m_Visible;
        bool                                                 m_ForceRender;

        void internalInit(Mesh* mesh, Material* mat, ShaderProgram* program);
        void internalUpdateModelMatrix();
    public:
        ModelInstance(Entity&, Mesh*, Material*, ShaderProgram* = 0);
        ModelInstance(Entity&, Handle mesh, Handle mat, ShaderProgram* = 0);
        ModelInstance(Entity&, Mesh*, Handle mat, ShaderProgram* = 0);
        ModelInstance(Entity&, Handle mesh, Material*, ShaderProgram* = 0);

        ModelInstance(const ModelInstance& other)                = default;
        ModelInstance& operator=(const ModelInstance& other)     = default;
        ModelInstance(ModelInstance&& other) noexcept            = default;
        ModelInstance& operator=(ModelInstance&& other) noexcept = default;

        ~ModelInstance();

        static void setDefaultViewportFlag(const unsigned int flag);
        static void setDefaultViewportFlag(const ViewportFlag::Flag flag);

        const ModelDrawingMode::Mode& getDrawingMode() const;
        void setDrawingMode(const ModelDrawingMode::Mode&);

        void setViewportFlag(const unsigned int flag);
        void addViewportFlag(const unsigned int flag);
        void setViewportFlag(const ViewportFlag::Flag flag);
        void addViewportFlag(const ViewportFlag::Flag flag);
        const unsigned int getViewportFlags() const;

        void forceRender(const bool forced = true);
        const bool isForceRendered() const;

        ShaderProgram* shaderProgram();
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
        void setShaderProgram(ShaderProgram*, ComponentModel&);

        void setMesh(const Handle& meshHandle, ComponentModel&);
        void setMesh(Mesh*, ComponentModel&);

        void setMaterial(const Handle& materialHandle, ComponentModel&);
        void setMaterial(Material*, ComponentModel&);

        void setPosition(const float& x, const float& y, const float& z);
        void setPosition(const glm::vec3& position);

        void setOrientation(const glm::quat& orientation);
        void setOrientation(const float& x, const float& y, const float& z);

        void setScale(const float& scale);
        void setScale(const float& x, const float& y, const float& z);
        void setScale(const glm::vec3& scale);

        void translate(const float& x, const float& y, const float& z);
        void translate(const glm::vec3& translation);

        void rotate(const float& pitch, const float& yaw, const float& roll);
        void rotate(const glm::vec3& rotation);

        void scale(const float& x, const float& y, const float& z);
        void scale(const glm::vec3& scale);
};

#endif