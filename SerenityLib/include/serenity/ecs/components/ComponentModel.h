#pragma once
#ifndef ENGINE_ECS_COMPONENT_MODEL_H
#define ENGINE_ECS_COMPONENT_MODEL_H

class  Handle;
class  ShaderProgram;
class  Mesh;
class  Material;
class  ComponentModel;
class  ComponentTransform;
class  ComponentCamera;
class  ModelInstanceHandle;
class  Camera;
namespace Engine::priv {
    class  EditorWindowScene;
}

#include <vector>
#include <serenity/ecs/ECS.h>
#include <serenity/dependencies/glm.h>

namespace Engine::priv {
    struct ComponentModel_Functions final {
        static std::pair<glm::vec3, float> CalculateBoundingBoxAndRadius(const std::vector<glm::vec3>& points);
        static float CalculateRadius(ComponentModel&);
        static std::pair<glm::vec3, float> CalculateRadi(const std::vector<Entity>&);
        static size_t GetTotalVertexCount(ComponentModel&);
        static void RegisterDeferredMeshLoaded(ComponentModel&, Mesh*);
    };
}

#include <serenity/model/ModelInstance.h>
#include <serenity/model/ModelInstanceHandle.h>
#include <string>

using ModelInstanceVector = std::vector<std::unique_ptr<ModelInstance>>;

class ComponentModel : public Observer, public ComponentBaseClass<ComponentModel> {
    friend struct Engine::priv::ComponentModel_Functions;
    friend class  ComponentCamera;
    friend class  Engine::priv::EditorWindowScene;
    private:
        ModelInstanceVector     m_ModelInstances;
        glm::vec3               m_RadiusBox       = glm::vec3{0.0f};
        float                   m_Radius          = 0.0f;
        Entity                  m_Owner;

        ComponentModel() = delete;
    public:
        ComponentModel(Entity, Handle mesh, Handle material, Handle shaderProgram = {}, RenderStage = RenderStage::GeometryOpaque);
        ComponentModel(Entity, const std::string& meshName, const std::string& materialName, const std::string& shaderProgramName = {}, RenderStage = RenderStage::GeometryOpaque);

        ComponentModel(const ComponentModel&)                = delete;
        ComponentModel& operator=(const ComponentModel&)     = delete;
        ComponentModel(ComponentModel&&) noexcept;
        ComponentModel& operator=(ComponentModel&&) noexcept;
        ~ComponentModel();

        [[nodiscard]] inline constexpr Entity getOwner() const noexcept { return m_Owner; }

        void onEvent(const Event&) override;

        void setViewportFlag(uint32_t flag) noexcept;
        void addViewportFlag(uint32_t flag) noexcept;
        inline void setViewportFlag(ViewportFlag flag) noexcept { setViewportFlag((uint32_t)flag); }
        inline void addViewportFlag(ViewportFlag flag) noexcept { addViewportFlag((uint32_t)flag); }

        [[nodiscard]] inline size_t getNumModels() const noexcept { return m_ModelInstances.size(); }
        [[nodiscard]] inline constexpr float getRadius() const noexcept { return m_Radius; }
        [[nodiscard]] inline constexpr const glm::vec3& getBoundingBox() const noexcept { return m_RadiusBox; }
        void show(bool shown = true) noexcept;
        inline void hide() noexcept { show(false); }

        [[nodiscard]] inline ModelInstance& getModel(size_t index = 0) noexcept { return *m_ModelInstances[index].get(); }
        [[nodiscard]] inline const ModelInstance& getModel(size_t index = 0) const noexcept { return *m_ModelInstances[index].get(); }
        ModelInstanceHandle addModel(Handle mesh, Handle material, Handle shaderProgram = {}, RenderStage = RenderStage::GeometryOpaque);
        inline void removeModel(size_t index) noexcept { m_ModelInstances.erase(m_ModelInstances.begin() + index); }

        void setStage(RenderStage stage, size_t index = 0);
        void setModel(Handle mesh, Handle material, size_t index, Handle = {}, RenderStage = RenderStage::GeometryOpaque);
        void setModelMesh(Handle mesh, size_t index, RenderStage = RenderStage::GeometryOpaque);
        void setModelMaterial(Handle material, size_t index, RenderStage = RenderStage::GeometryOpaque);
        void setModelShaderProgram(Handle shaderProgram, size_t index, RenderStage = RenderStage::GeometryOpaque);

        [[nodiscard]] bool rayIntersectSphere(const ComponentTransform& cameraTransform) const noexcept;
        [[nodiscard]] bool rayIntersectSphere(const Camera&) const noexcept;

        void setUserPointer(void* UserPointer) noexcept;

        inline void setCustomBindFunctor(ModelInstance::BindFunc&& func, size_t index = 0) noexcept {
            m_ModelInstances[index]->setCustomBindFunctor(std::move(func));
        }
        inline void setCustomUnbindFunctor(ModelInstance::UnbindFunc&& func, size_t index = 0) noexcept {
            m_ModelInstances[index]->setCustomUnbindFunctor(std::move(func));
        }
        inline void setCustomBindFunctor(const ModelInstance::BindFunc& func, size_t index = 0) noexcept {
            m_ModelInstances[index]->setCustomBindFunctor(func);
        }
        inline void setCustomUnbindFunctor(const ModelInstance::UnbindFunc& func, size_t index = 0) noexcept {
            m_ModelInstances[index]->setCustomUnbindFunctor(func);
        }

        void setShadowCaster(bool isShadowCaster) noexcept;

        inline ModelInstance& operator[](size_t index) { return *m_ModelInstances[index].get(); }
        inline const ModelInstance& operator[](size_t index) const { return *m_ModelInstances[index].get(); }

        inline ModelInstanceVector::iterator begin() noexcept { return m_ModelInstances.begin(); }
        inline ModelInstanceVector::iterator end() noexcept { return m_ModelInstances.end(); }
        inline ModelInstanceVector::const_iterator begin() const noexcept { return m_ModelInstances.begin(); }
        inline ModelInstanceVector::const_iterator end() const noexcept { return m_ModelInstances.end(); }
};

namespace Engine::priv {
    class ComponentModelLUABinder {
        private:
            Entity m_Owner;

            ComponentModelLUABinder() = delete;
        public:
            ComponentModelLUABinder(Entity owner)
                : m_Owner{ owner }
            {}
    };
}

#endif