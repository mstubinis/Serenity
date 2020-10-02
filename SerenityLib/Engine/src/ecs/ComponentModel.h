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
        static float CalculateRadius(ComponentModel& super);
        static void RegisterDeferredMeshLoaded(ComponentModel& super, Mesh* mesh);
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
        Entity                       m_Owner;
        ModelInstanceVector          m_ModelInstances;
        float                        m_Radius          = 0.0f;
        glm::vec3                    m_RadiusBox       = glm::vec3(0.0f);

        ComponentModel() = delete;
    public:
        //BOOST_TYPE_INDEX_REGISTER_CLASS
        ComponentModel(Entity, Handle meshHandle, Handle materialHandle, ShaderProgram* = 0, RenderStage = RenderStage::GeometryOpaque);
        ComponentModel(Entity, Mesh*,             Handle materialHandle, ShaderProgram* = 0, RenderStage = RenderStage::GeometryOpaque);
        ComponentModel(Entity, Handle meshHandle, Material*,             ShaderProgram* = 0, RenderStage = RenderStage::GeometryOpaque);
        ComponentModel(Entity, Mesh*,             Material*,             ShaderProgram* = 0, RenderStage = RenderStage::GeometryOpaque);

        ComponentModel(Entity, Handle meshHandle, Handle materialHandle, Handle shaderPHandle, RenderStage = RenderStage::GeometryOpaque);
        ComponentModel(Entity, Mesh*,             Handle materialHandle, Handle shaderPHandle, RenderStage = RenderStage::GeometryOpaque);
        ComponentModel(Entity, Handle meshHandle, Material*,             Handle shaderPHandle, RenderStage = RenderStage::GeometryOpaque);
        ComponentModel(Entity, Mesh*,             Material*,             Handle shaderPHandle, RenderStage = RenderStage::GeometryOpaque);

        ComponentModel(const ComponentModel& other)                = delete;
        ComponentModel& operator=(const ComponentModel& other)     = delete;
        ComponentModel(ComponentModel&& other) noexcept;
        ComponentModel& operator=(ComponentModel&& other) noexcept;

        inline CONSTEXPR Entity getOwner() const noexcept { return m_Owner; }

        void onEvent(const Event& e) override;

        void setViewportFlag(unsigned int flag);
        void addViewportFlag(unsigned int flag);
        void setViewportFlag(ViewportFlag::Flag flag);
        void addViewportFlag(ViewportFlag::Flag flag);

        inline size_t getNumModels() const noexcept { return m_ModelInstances.size(); }
        inline CONSTEXPR float radius() const noexcept { return m_Radius; }
        inline CONSTEXPR const glm::vec3& boundingBox() const noexcept { return m_RadiusBox; }
        void show(bool shown = true) noexcept;
        inline void hide() noexcept { show(false); }

        inline ModelInstance& getModel(size_t index = 0) noexcept { return *m_ModelInstances[index].get(); }
        ModelInstanceHandle addModel(Handle meshHandle, Handle materialHandle, ShaderProgram* = 0,   RenderStage = RenderStage::GeometryOpaque);
        ModelInstanceHandle addModel(Mesh*,             Material*,             ShaderProgram* = 0,   RenderStage = RenderStage::GeometryOpaque);
        ModelInstanceHandle addModel(Handle meshHandle, Handle materialHandle, Handle shaderProgram, RenderStage = RenderStage::GeometryOpaque);
        ModelInstanceHandle addModel(Mesh*,             Material*,             Handle shaderProgram, RenderStage = RenderStage::GeometryOpaque);

        inline void removeModel(size_t index) noexcept { m_ModelInstances.erase(m_ModelInstances.begin() + index); }

        void setStage(RenderStage stage, size_t index = 0);

        void setModel(Handle meshHandle, Handle materialHandle, size_t index, ShaderProgram* = 0, RenderStage = RenderStage::GeometryOpaque);
        void setModel(Mesh*,             Material*,             size_t index, ShaderProgram* = 0, RenderStage = RenderStage::GeometryOpaque);

        void setModelMesh(Mesh*,             size_t index, RenderStage = RenderStage::GeometryOpaque);
        void setModelMesh(Handle meshHandle, size_t index, RenderStage = RenderStage::GeometryOpaque);

        void setModelMaterial(Material*,             size_t index, RenderStage = RenderStage::GeometryOpaque);
        void setModelMaterial(Handle materialHandle, size_t index, RenderStage = RenderStage::GeometryOpaque);

        void setModelShaderProgram(ShaderProgram*,        size_t index, RenderStage = RenderStage::GeometryOpaque);
        void setModelShaderProgram(Handle materialHandle, size_t index, RenderStage = RenderStage::GeometryOpaque);

        bool rayIntersectSphere(const ComponentCamera& camera) const;

        void setUserPointer(void* UserPointer);

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