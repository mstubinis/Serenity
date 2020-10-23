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
    class  RenderModule;
    struct InternalModelInstancePublicInterface final {
        static bool IsViewportValid(const ModelInstance&, const Viewport&);
    };
};

#include <core/engine/events/Event.h>
#include <ecs/Entity.h>

#include <core/engine/model/ModelInstanceIncludes.h>
#include <core/engine/model/ModelInstanceAnimation.h>
#include <core/engine/scene/ViewportIncludes.h>
#include <core/engine/events/Observer.h>
#include <core/engine/renderer/RendererIncludes.h>
#include <core/engine/resources/Handle.h>

class ModelInstance final : public Engine::UserPointer, public Observer {
    friend struct Engine::priv::DefaultModelInstanceBindFunctor;
    friend struct Engine::priv::DefaultModelInstanceUnbindFunctor;
    friend struct Engine::priv::ComponentModel_UpdateFunction;
    friend class  Engine::priv::RenderModule;
    friend class  ComponentModel;
    friend class  Collision;

    using bind_function   = void(*)(ModelInstance*, const Engine::priv::RenderModule*);
    using unbind_function = void(*)(ModelInstance*, const Engine::priv::RenderModule*);

    private:
        static decimal                                       m_GlobalDistanceFactor;
        static unsigned int                                  m_ViewportFlagDefault;
    private:
        bind_function                                        m_CustomBindFunctor   = [](ModelInstance*, const Engine::priv::RenderModule*) {};
        unbind_function                                      m_CustomUnbindFunctor = [](ModelInstance*, const Engine::priv::RenderModule*) {};

        ModelDrawingMode                                     m_DrawingMode         = ModelDrawingMode::Triangles;
        Engine::Flag<unsigned int>                           m_ViewportFlag;     //determine what viewports this can be seen in
        Engine::priv::ModelInstanceAnimationVector           m_AnimationVector;
        Entity                                               m_Parent              = Entity();
        Handle                                               m_ShaderProgramHandle = Handle{};
        Handle                                               m_MeshHandle          = Handle{};
        Handle                                               m_MaterialHandle      = Handle{};
        RenderStage                                          m_Stage               = RenderStage::GeometryOpaque;
        glm::vec3                                            m_Position            = glm::vec3(0.0f, 0.0f, 0.0f);
        glm::vec3                                            m_Scale               = glm::vec3(1.0f, 1.0f, 1.0f);
        Engine::color_vector_4                               m_GodRaysColor        = Engine::color_vector_4(0_uc);
        glm::quat                                            m_Orientation         = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
        glm::mat4                                            m_ModelMatrix         = glm::mat4(1.0f);
        Engine::color_vector_4                               m_Color               = Engine::color_vector_4(255_uc);
        bool                                                 m_PassedRenderCheck   = false;
        bool                                                 m_Visible             = true;
        bool                                                 m_ForceRender         = false;
        float                                                m_Radius              = 0.0f;
        size_t                                               m_Index               = 0U;

        float internal_calculate_radius();
        void internal_init(Handle mesh, Handle mat, Handle program);
        void internal_update_model_matrix(bool recalcRadius = true);

        inline void bind(const Engine::priv::RenderModule& renderer) noexcept { m_CustomBindFunctor(this, &renderer); }
        inline void unbind(const Engine::priv::RenderModule& renderer) noexcept { m_CustomUnbindFunctor(this, &renderer); }

        ModelInstance() = delete;
    public:
        static void setDefaultViewportFlag(uint32_t flag) noexcept { m_ViewportFlagDefault = flag; }
        static void setDefaultViewportFlag(ViewportFlag::Flag flag) noexcept { m_ViewportFlagDefault = flag; }
    public:
        ModelInstance(Entity, Handle mesh, Handle material, Handle shaderProgram = Handle{});

        ModelInstance(const ModelInstance& other)                = delete;
        ModelInstance& operator=(const ModelInstance& other)     = delete;
        ModelInstance(ModelInstance&& other) noexcept;
        ModelInstance& operator=(ModelInstance&& other) noexcept;

        ~ModelInstance();

        inline bool operator==(const ModelInstance& other) { return (m_Index == other.m_Index && m_Parent == other.m_Parent); }
        inline bool operator!=(const ModelInstance& other) { return !operator==(other); }
        inline bool operator==(ModelInstance& other) { return (m_Index == other.m_Index && m_Parent == other.m_Parent); }
        inline bool operator!=(ModelInstance& other) { return !operator==(other); }

        void onEvent(const Event& e) override;

        static inline void setGlobalDistanceFactor(decimal factor) noexcept { m_GlobalDistanceFactor = factor; }
        static inline CONSTEXPR decimal getGlobalDistanceFactor() noexcept { return m_GlobalDistanceFactor; }

        inline void setCustomBindFunctor(bind_function&& functor) noexcept { m_CustomBindFunctor   = std::move(functor); }
        inline void setCustomUnbindFunctor(unbind_function&& functor) noexcept { m_CustomUnbindFunctor = std::move(functor); }
        inline void setCustomBindFunctor(const bind_function& functor) noexcept { m_CustomBindFunctor = functor; }
        inline void setCustomUnbindFunctor(const unbind_function& functor) noexcept { m_CustomUnbindFunctor = functor; }

        inline void setViewportFlag(uint32_t flag) noexcept { m_ViewportFlag = flag; }
        inline void addViewportFlag(uint32_t flag) noexcept { m_ViewportFlag.add(flag); }
        inline void removeViewportFlag(uint32_t flag) noexcept { m_ViewportFlag.remove(flag); }
        inline void setViewportFlag(ViewportFlag::Flag flag) noexcept { m_ViewportFlag = flag; }
        inline void addViewportFlag(ViewportFlag::Flag flag) noexcept { m_ViewportFlag.add(flag); }
        inline void removeViewportFlag(ViewportFlag::Flag flag) noexcept { m_ViewportFlag.remove(flag); }
        inline uint32_t getViewportFlags() const noexcept { return m_ViewportFlag.get(); }

        inline const Engine::priv::ModelInstanceAnimationVector& getRunningAnimations() const noexcept { return m_AnimationVector; }

        inline float radius() const noexcept { return m_Radius; }
        inline size_t index() const noexcept { return m_Index; }
        inline ModelDrawingMode getDrawingMode() const noexcept { return m_DrawingMode; }
        inline void setDrawingMode(ModelDrawingMode drawMode) noexcept { m_DrawingMode = drawMode; }
        inline void forceRender(bool forced = true) noexcept { m_ForceRender = forced; }
        inline bool isForceRendered() const noexcept { return m_ForceRender; }
        inline Entity parent() const noexcept { return m_Parent; }
        inline const Engine::color_vector_4& color() const noexcept { return m_Color; }
        inline const Engine::color_vector_4& godRaysColor() const noexcept { return m_GodRaysColor; }
        inline const glm::mat4& modelMatrix() const noexcept { return m_ModelMatrix; }
        inline const glm::vec3& getScale() const noexcept { return m_Scale; }
        inline const glm::vec3& position() const noexcept { return m_Position; }
        inline const glm::quat& orientation() const noexcept { return m_Orientation; }
        inline Handle shaderProgram() const noexcept { return m_ShaderProgramHandle; }
        inline Handle mesh() const noexcept { return m_MeshHandle; }
        inline Handle material() const noexcept { return m_MaterialHandle; }
        inline RenderStage stage() const noexcept { return m_Stage; }
        inline void show(bool shown = true) noexcept { m_Visible = shown; }
        inline void hide() noexcept { m_Visible = false; }
        inline bool visible() const noexcept { return m_Visible; }
        inline bool passedRenderCheck() const noexcept { return m_PassedRenderCheck; }
        inline void setPassedRenderCheck(bool passed) noexcept { m_PassedRenderCheck = passed; }


        void setStage(RenderStage stage, ComponentModel& componentModel);

        void playAnimation(const std::string& animName, float startTime, float endTime = -1.0f, uint32_t requestedLoops = 1);

        inline void setColor(float r, float g, float b, float a = 1.0f) noexcept { m_Color = Engine::color_vector_4(r, g, b, a); }
        inline void setColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255_uc) noexcept { m_Color = Engine::color_vector_4(r, g, b, a); }
        inline void setColor(const glm::vec4& color) noexcept { setColor(color.r, color.g, color.b, color.a); }
        inline void setColor(const glm::vec3& color) noexcept { setColor(color.r, color.g, color.b, 1.0f); }

        inline void setGodRaysColor(float r, float g, float b) noexcept { m_GodRaysColor = Engine::color_vector_4(r, g, b, m_GodRaysColor.a()); }
        inline void setGodRaysColor(const glm::vec3& color) noexcept { setGodRaysColor(color.r, color.g, color.b); }

        void setShaderProgram(Handle shaderPHandle, ComponentModel&);

        void setMesh(Handle meshHandle, ComponentModel&);

        void setMaterial(Handle materialHandle, ComponentModel&);

        void setPosition(float x, float y, float z);

        void setOrientation(const glm::quat& orientation);
        void setOrientation(float x, float y, float z);

        void setScale(float x, float y, float z);

        void translate(float x, float y, float z);

        void rotate(float pitch, float yaw, float roll);

        inline void setScale(float scale) noexcept { setScale(scale, scale, scale); }
        inline void scale(float x, float y, float z) noexcept { setScale(x + m_Scale.x, y + m_Scale.y, z + m_Scale.z); }
        inline void setPosition(const glm::vec3& v) noexcept { setPosition(v.x, v.y, v.z); }
        inline void setScale(const glm::vec3& v) noexcept { setScale(v.x, v.y, v.z); }
        inline void translate(const glm::vec3& v) noexcept { translate(v.x, v.y, v.z); }
        inline void rotate(const glm::vec3& v) noexcept { rotate(v.x, v.y, v.z); }
        inline void scale(const glm::vec3& v) noexcept { scale(v.x, v.y, v.z); }
};

#endif