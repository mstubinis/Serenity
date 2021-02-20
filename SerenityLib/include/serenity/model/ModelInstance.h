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
    struct ComponentModel_UpdateFunction;
    class  ModelInstanceAnimation;
    class  RenderModule;
    struct PublicModelInstance final {
        [[nodiscard]] static bool IsViewportValid(const ModelInstance&, const Viewport&);
    };
};

#include <serenity/events/Event.h>
#include <serenity/ecs/Entity.h>

#include <serenity/model/ModelInstanceIncludes.h>
#include <serenity/model/ModelInstanceAnimation.h>
#include <serenity/scene/ViewportIncludes.h>
#include <serenity/events/Observer.h>
#include <serenity/renderer/RendererIncludes.h>
#include <serenity/resources/Handle.h>
#include <serenity/types/ColorVector.h>
#include <serenity/dependencies/glm.h>
#include <serenity/types/Types.h>

class ModelInstance final : public Observer {
    friend struct Engine::priv::ComponentModel_UpdateFunction;
    friend class  Engine::priv::RenderModule;
    friend class  ComponentModel;
    friend class  Collision;

    using BindFunc   = void(*)(ModelInstance*, const Engine::priv::RenderModule*);
    using UnbindFunc = void(*)(ModelInstance*, const Engine::priv::RenderModule*);

    private:
        static decimal                                    m_GlobalDistanceFactor;
        static uint32_t                                   m_ViewportFlagDefault;
    private:
        glm::mat4                                         m_ModelMatrix         = glm::mat4(1.0f);
        glm::quat                                         m_Orientation         = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
        glm::vec3                                         m_Position            = glm::vec3(0.0f, 0.0f, 0.0f);
        glm::vec3                                         m_Scale               = glm::vec3(1.0f, 1.0f, 1.0f);
        BindFunc                                          m_CustomBindFunctor   = [](ModelInstance*, const Engine::priv::RenderModule*) {};
        UnbindFunc                                        m_CustomUnbindFunctor = [](ModelInstance*, const Engine::priv::RenderModule*) {};

        ModelDrawingMode                                  m_DrawingMode         = ModelDrawingMode::Triangles;
        Engine::Flag<uint32_t>                            m_ViewportFlag;     //determine what viewports this can be seen in
        Engine::priv::ModelInstanceAnimationContainer     m_Animations;
        Entity                                            m_Parent              = Entity();
        Handle                                            m_ShaderProgramHandle = Handle{};
        Handle                                            m_MeshHandle          = Handle{};
        Handle                                            m_MaterialHandle      = Handle{};
        RenderStage                                       m_Stage               = RenderStage::GeometryOpaque;
        Engine::color_vector_4                            m_GodRaysColor        = Engine::color_vector_4(0_uc);
        Engine::color_vector_4                            m_Color               = Engine::color_vector_4(255_uc);
        void*                                             m_UserPointer         = nullptr;
        float                                             m_Radius              = 0.0f;
        uint32_t                                          m_Index               = 0U;
        bool                                              m_PassedRenderCheck   = false;
        bool                                              m_Visible             = true;
        bool                                              m_ForceRender         = false;

        float internal_calculate_radius();
        void internal_init(Handle mesh, Handle mat, Handle program);
        void internal_update_model_matrix(bool recalcRadius = true);

        inline void bind(const Engine::priv::RenderModule& renderer) noexcept { m_CustomBindFunctor(this, &renderer); }
        inline void unbind(const Engine::priv::RenderModule& renderer) noexcept { m_CustomUnbindFunctor(this, &renderer); }

        ModelInstance() = delete;
    public:
        static inline void setDefaultViewportFlag(uint32_t flag) noexcept { m_ViewportFlagDefault = flag; }
        static inline void setDefaultViewportFlag(ViewportFlag::Flag flag) noexcept { m_ViewportFlagDefault = flag; }
    public:
        ModelInstance(Entity, Handle mesh, Handle material, Handle shaderProgram = Handle{});

        ModelInstance(const ModelInstance&)                = delete;
        ModelInstance& operator=(const ModelInstance&)     = delete;
        ModelInstance(ModelInstance&&) noexcept;
        ModelInstance& operator=(ModelInstance&&) noexcept;

        ~ModelInstance();

        void onEvent(const Event& e) override;

        static inline void setGlobalDistanceFactor(decimal factor) noexcept { m_GlobalDistanceFactor = factor; }
        static inline constexpr decimal getGlobalDistanceFactor() noexcept { return m_GlobalDistanceFactor; }

        inline void setUserPointer(void* userPtr) noexcept { m_UserPointer = userPtr; }
        inline void* getUserPointer() const noexcept { return m_UserPointer; }

        inline void setCustomBindFunctor(BindFunc&& functor) noexcept { m_CustomBindFunctor   = std::move(functor); }
        inline void setCustomUnbindFunctor(UnbindFunc&& functor) noexcept { m_CustomUnbindFunctor = std::move(functor); }
        inline void setCustomBindFunctor(const BindFunc& functor) noexcept { m_CustomBindFunctor = functor; }
        inline void setCustomUnbindFunctor(const UnbindFunc& functor) noexcept { m_CustomUnbindFunctor = functor; }

        inline void setViewportFlag(uint32_t flag) noexcept { m_ViewportFlag = flag; }
        inline void addViewportFlag(uint32_t flag) noexcept { m_ViewportFlag.add(flag); }
        inline void removeViewportFlag(uint32_t flag) noexcept { m_ViewportFlag.remove(flag); }
        inline void setViewportFlag(ViewportFlag::Flag flag) noexcept { m_ViewportFlag = flag; }
        inline void addViewportFlag(ViewportFlag::Flag flag) noexcept { m_ViewportFlag.add(flag); }
        inline void removeViewportFlag(ViewportFlag::Flag flag) noexcept { m_ViewportFlag.remove(flag); }
        [[nodiscard]] inline uint32_t getViewportFlags() const noexcept { return m_ViewportFlag.get(); }

        [[nodiscard]] inline const Engine::priv::ModelInstanceAnimationContainer& getRunningAnimations() const noexcept { return m_Animations; }

        [[nodiscard]] inline float radius() const noexcept { return m_Radius; }
        [[nodiscard]] inline uint32_t index() const noexcept { return m_Index; }
        [[nodiscard]] inline ModelDrawingMode getDrawingMode() const noexcept { return m_DrawingMode; }
        inline void setDrawingMode(ModelDrawingMode drawMode) noexcept { m_DrawingMode = drawMode; }
        inline void forceRender(bool forced = true) noexcept { m_ForceRender = forced; }
        [[nodiscard]] inline bool isForceRendered() const noexcept { return m_ForceRender; }
        [[nodiscard]] inline Entity parent() const noexcept { return m_Parent; }
        [[nodiscard]] inline const Engine::color_vector_4& color() const noexcept { return m_Color; }
        [[nodiscard]] inline const Engine::color_vector_4& godRaysColor() const noexcept { return m_GodRaysColor; }
        [[nodiscard]] inline const glm::mat4& modelMatrix() const noexcept { return m_ModelMatrix; }
        [[nodiscard]] inline const glm::vec3& getScale() const noexcept { return m_Scale; }
        [[nodiscard]] inline const glm::vec3& position() const noexcept { return m_Position; }
        [[nodiscard]] inline const glm::quat& orientation() const noexcept { return m_Orientation; }
        [[nodiscard]] inline Handle shaderProgram() const noexcept { return m_ShaderProgramHandle; }
        [[nodiscard]] inline Handle mesh() const noexcept { return m_MeshHandle; }
        [[nodiscard]] inline Handle material() const noexcept { return m_MaterialHandle; }
        [[nodiscard]] inline RenderStage stage() const noexcept { return m_Stage; }
        inline void show(bool shown = true) noexcept { m_Visible = shown; }
        inline void hide() noexcept { m_Visible = false; }
        [[nodiscard]] inline bool visible() const noexcept { return m_Visible; }
        [[nodiscard]] inline bool passedRenderCheck() const noexcept { return m_PassedRenderCheck; }
        inline void setPassedRenderCheck(bool passed) noexcept { m_PassedRenderCheck = passed; }


        void setStage(RenderStage stage, ComponentModel& componentModel);

        void playAnimation(std::string_view animName, float startTime, float endTime = -1.0f, uint32_t requestedLoops = 1);

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