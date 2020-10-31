#pragma once
#ifndef ENGINE_ECS_COMPONENT_MODEL_H
#define ENGINE_ECS_COMPONENT_MODEL_H

class  Handle;
class  ShaderProgram;
class  Mesh;
class  Material;
class  ComponentModel;
class  ComponentCamera;
class  ModelInstanceHandle;
namespace Engine::priv {
    struct ComponentModel_UpdateFunction;
    struct ComponentModel_Functions final {
        static float CalculateRadius(ComponentModel&);
        static size_t GetTotalVertexCount(ComponentModel&);
        static void RegisterDeferredMeshLoaded(ComponentModel&, Mesh*);
    };
};

#include <ecs/Entity.h>
#include <ecs/ECSSystemConstructorInfo.h>
#include <core/engine/model/ModelInstance.h>
#include <core/engine/model/ModelInstanceHandle.h>

using ModelInstanceVector = std::vector<std::unique_ptr<ModelInstance>>;

class ComponentModel: public Observer {
    friend struct Engine::priv::ComponentModel_UpdateFunction;
    friend struct Engine::priv::ComponentModel_Functions;
    friend class  ComponentCamera;
    private:
        ModelInstanceVector          m_ModelInstances;
        glm::vec3                    m_RadiusBox = glm::vec3(0.0f);
        float                        m_Radius = 0.0f;
        Entity                       m_Owner;

        ComponentModel() = delete;
    public:
        ComponentModel(Entity, Handle meshHandle, Handle materialHandle, Handle = Handle{}, RenderStage = RenderStage::GeometryOpaque);
       
        ComponentModel(const ComponentModel& other)                = delete;
        ComponentModel& operator=(const ComponentModel& other)     = delete;
        ComponentModel(ComponentModel&& other) noexcept;
        ComponentModel& operator=(ComponentModel&& other) noexcept;

        inline CONSTEXPR Entity getOwner() const noexcept { return m_Owner; }

        void onEvent(const Event&) override;

        void setViewportFlag(uint32_t flag) noexcept;
        void addViewportFlag(uint32_t flag) noexcept;
        inline void setViewportFlag(ViewportFlag::Flag flag) noexcept { setViewportFlag((uint32_t)flag); }
        inline void addViewportFlag(ViewportFlag::Flag flag) noexcept { addViewportFlag((uint32_t)flag); }

        inline size_t getNumModels() const noexcept { return m_ModelInstances.size(); }
        inline CONSTEXPR float radius() const noexcept { return m_Radius; }
        inline CONSTEXPR const glm::vec3& boundingBox() const noexcept { return m_RadiusBox; }
        void show(bool shown = true) noexcept;
        inline void hide() noexcept { show(false); }

        inline ModelInstance& getModel(size_t index = 0) noexcept { return *m_ModelInstances[index].get(); }
        ModelInstanceHandle addModel(Handle meshHandle, Handle materialHandle, Handle = Handle{}, RenderStage = RenderStage::GeometryOpaque);
        inline void removeModel(size_t index) noexcept { m_ModelInstances.erase(m_ModelInstances.begin() + index); }

        void setStage(RenderStage stage, size_t index = 0);
        void setModel(Handle meshHandle, Handle materialHandle, size_t index, Handle = Handle{}, RenderStage = RenderStage::GeometryOpaque);
        void setModelMesh(Handle meshHandle, size_t index, RenderStage = RenderStage::GeometryOpaque);
        void setModelMaterial(Handle materialHandle, size_t index, RenderStage = RenderStage::GeometryOpaque);
        void setModelShaderProgram(Handle shaderProgram, size_t index, RenderStage = RenderStage::GeometryOpaque);

        bool rayIntersectSphere(const ComponentCamera& camera) const noexcept;

        void setUserPointer(void* UserPointer) noexcept;

        inline void setCustomBindFunctor(ModelInstance::bind_function&& functor, size_t index = 0) noexcept {
            m_ModelInstances[index]->setCustomBindFunctor(std::move(functor));
        }
        inline void setCustomUnbindFunctor(ModelInstance::unbind_function&& functor, size_t index = 0) noexcept {
            m_ModelInstances[index]->setCustomUnbindFunctor(std::move(functor));
        }
        inline void setCustomBindFunctor(const ModelInstance::bind_function& functor, size_t index = 0) noexcept {
            m_ModelInstances[index]->setCustomBindFunctor(functor);
        }
        inline void setCustomUnbindFunctor(const ModelInstance::unbind_function& functor, size_t index = 0) noexcept {
            m_ModelInstances[index]->setCustomUnbindFunctor(functor);
        }

        inline ModelInstance& operator[](size_t index) { return *m_ModelInstances[index].get(); }
        inline const ModelInstance& operator[](size_t index) const { return *m_ModelInstances[index].get(); }

        inline ModelInstanceVector::iterator begin() noexcept { return m_ModelInstances.begin(); }
        inline ModelInstanceVector::iterator end() noexcept { return m_ModelInstances.end(); }
        inline ModelInstanceVector::const_iterator begin() const noexcept { return m_ModelInstances.begin(); }
        inline ModelInstanceVector::const_iterator end() const noexcept { return m_ModelInstances.end(); }
};

class ComponentModel_System_CI : public Engine::priv::ECSSystemCI {
    public:
        ComponentModel_System_CI();
        ~ComponentModel_System_CI() = default;
};

#endif