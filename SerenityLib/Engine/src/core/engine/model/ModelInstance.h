#pragma once
#ifndef ENGINE_MESH_INSTANCE_H
#define ENGINE_MESH_INSTANCE_H

class  Handle;
class  ShaderProgram;
class  Material;
class  Mesh;
class  ComponentModel;
class  Viewport;
class  ModelInstance;
class  Collision;
namespace Engine::priv {
    struct DefaultModelInstanceBindFunctor;
    struct DefaultModelInstanceUnbindFunctor;
    struct ComponentModel_UpdateFunction;
    class  ModelInstanceAnimation;
    class  Renderer;
    struct InternalModelInstancePublicInterface final {
        static const bool IsViewportValid(const ModelInstance&, const Viewport&);
    };
};

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/gtc/quaternion.hpp>
#include <core/engine/events/Engine_EventObject.h>
#include <ecs/Entity.h>

#include <core/engine/model/ModelInstanceIncludes.h>
#include <core/engine/model/ModelInstanceAnimation.h>
#include <core/engine/scene/ViewportIncludes.h>
#include <functional>


class ModelInstance final : public Engine::UserPointer {
    friend struct Engine::priv::DefaultModelInstanceBindFunctor;
    friend struct Engine::priv::DefaultModelInstanceUnbindFunctor;
    friend struct Engine::priv::ComponentModel_UpdateFunction;
    friend class  Engine::priv::Renderer;
    friend class  ComponentModel;
    friend class  Collision;
    private:
        static decimal                                       m_GlobalDistanceFactor;
        static unsigned int                                  m_ViewportFlagDefault;
    private:
        std::function<void(ModelInstance*, const Engine::priv::Renderer*)>  m_CustomBindFunctor   = [](ModelInstance*, const Engine::priv::Renderer*) {};
        std::function<void(ModelInstance*, const Engine::priv::Renderer*)>  m_CustomUnbindFunctor = [](ModelInstance*, const Engine::priv::Renderer*) {};

        ModelDrawingMode::Mode                               m_DrawingMode       = ModelDrawingMode::Triangles;
        Engine::Flag<unsigned int>                           m_ViewportFlag;     //determine what viewports this can be seen in
        Engine::priv::ModelInstanceAnimationVector           m_AnimationVector;
        Entity                                               m_Parent            = Entity();
        ShaderProgram*                                       m_ShaderProgram     = nullptr;
        Mesh*                                                m_Mesh              = nullptr;
        Material*                                            m_Material          = nullptr;
        RenderStage::Stage                                   m_Stage             = RenderStage::GeometryOpaque;
        glm::vec3                                            m_Position          = glm::vec3(0.0f, 0.0f, 0.0f);
        glm::vec3                                            m_Scale             = glm::vec3(1.0f, 1.0f, 1.0f);
        Engine::color_vector_4                               m_GodRaysColor      = Engine::color_vector_4(0_uc);
        glm::quat                                            m_Orientation       = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
        glm::mat4                                            m_ModelMatrix       = glm::mat4(1.0f);
        Engine::color_vector_4                               m_Color             = Engine::color_vector_4(255_uc);
        bool                                                 m_PassedRenderCheck = false;
        bool                                                 m_Visible           = true;
        bool                                                 m_ForceRender       = false;
        size_t                                               m_Index             = 0U;

        void internal_init(Mesh* mesh, Material* mat, ShaderProgram* program);
        void internal_update_model_matrix();

        void bind(const Engine::priv::Renderer& renderer);
        void unbind(const Engine::priv::Renderer& renderer);

        ModelInstance() = delete;
    public:
        ModelInstance(Entity, Mesh*,       Material*,  ShaderProgram* = 0);
        ModelInstance(Entity, Handle mesh, Handle mat, ShaderProgram* = 0);
        ModelInstance(Entity, Mesh*,       Handle mat, ShaderProgram* = 0);
        ModelInstance(Entity, Handle mesh, Material*,  ShaderProgram* = 0);

        ModelInstance(const ModelInstance& other)                = delete;
        ModelInstance& operator=(const ModelInstance& other)     = delete;
        ModelInstance(ModelInstance&& other) noexcept;
        ModelInstance& operator=(ModelInstance&& other) noexcept;

        ~ModelInstance();

        static void setGlobalDistanceFactor(decimal factor);
        static decimal getGlobalDistanceFactor();

        template<typename T> void setCustomBindFunctor(const T& functor) {
            m_CustomBindFunctor   = std::bind<void>(functor, std::placeholders::_1, std::placeholders::_2);
        }
        template<typename T> void setCustomUnbindFunctor(const T& functor) {
            m_CustomUnbindFunctor = std::bind<void>(functor, std::placeholders::_1, std::placeholders::_2);
        }
        static void setDefaultViewportFlag(const unsigned int flag);
        static void setDefaultViewportFlag(const ViewportFlag::Flag flag);

        size_t index() const;
        ModelDrawingMode::Mode getDrawingMode() const;
        void setDrawingMode(const ModelDrawingMode::Mode);

        void setViewportFlag(const unsigned int flag);
        void addViewportFlag(const unsigned int flag);
        void removeViewportFlag(const unsigned int flag);
        void setViewportFlag(const ViewportFlag::Flag flag);
        void addViewportFlag(const ViewportFlag::Flag flag);
        void removeViewportFlag(const ViewportFlag::Flag flag);

        unsigned int getViewportFlags() const;

        void forceRender(const bool forced = true);
        bool isForceRendered() const;

        ShaderProgram* shaderProgram() const;
        Mesh* mesh() const;
        Material* material() const;
        Entity parent() const;

        const Engine::color_vector_4& color() const;
        const Engine::color_vector_4& godRaysColor() const;
        const glm::mat4& modelMatrix() const;
        const glm::vec3& position() const;
        const glm::quat& orientation() const;
        const glm::vec3& getScale() const;

        bool visible() const;
        bool passedRenderCheck() const;
        void setPassedRenderCheck(const bool);
        void show();
        void hide();

        RenderStage::Stage stage() const;
        //void setStage(const RenderStage::Stage& stage);
        void setStage(const RenderStage::Stage stage, ComponentModel& componentModel);

        void playAnimation(const std::string& animName, const float startTime, const float endTime = -1.0f, const unsigned int requestedLoops = 1);

        void setColor(const float r, const float g, const float b, const float a = 1.0f);
        void setColor(const unsigned char r, const unsigned char g, const unsigned char b, const unsigned char a = 255);
        void setColor(const glm::vec4& color);
        void setColor(const glm::vec3& color);

        void setGodRaysColor(const float r, const float g, const float b);
        void setGodRaysColor(const glm::vec3& color);

        void setShaderProgram(Handle shaderPHandle, ComponentModel&);
        void setShaderProgram(ShaderProgram*, ComponentModel&);

        void setMesh(Handle meshHandle, ComponentModel&);
        void setMesh(Mesh*, ComponentModel&);

        void setMaterial(Handle materialHandle, ComponentModel&);
        void setMaterial(Material*, ComponentModel&);

        void setPosition(const float x, const float y, const float z);
        void setPosition(const glm::vec3& position);

        void setOrientation(const glm::quat& orientation);
        void setOrientation(const float x, const float y, const float z);

        void setScale(const float scale);

        void setScale(const float x, const float y, const float z);
        void setScale(const glm::vec3& scale);

        void translate(const float x, const float y, const float z);
        void translate(const glm::vec3& translation);

        void rotate(const float pitch, const float yaw, const float roll);
        void rotate(const glm::vec3& rotation);

        void scale(const float x, const float y, const float z);
        void scale(const glm::vec3& scale);
};

#endif