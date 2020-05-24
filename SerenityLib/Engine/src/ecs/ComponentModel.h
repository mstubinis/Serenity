#pragma once
#ifndef ENGINE_ECS_COMPONENT_MODEL_H
#define ENGINE_ECS_COMPONENT_MODEL_H

class  Handle;
class  ShaderProgram;
class  Mesh;
class  Material;
class  ComponentModel;
class  ComponentCamera;
namespace Engine::priv {
    struct ComponentModel_UpdateFunction;
    struct ComponentModel_EntityAddedToSceneFunction;
    struct ComponentModel_ComponentAddedToEntityFunction;
    struct ComponentModel_ComponentRemovedFromEntityFunction;
    struct ComponentModel_SceneEnteredFunction;
    struct ComponentModel_SceneLeftFunction;
    struct ComponentModel_Functions final {
        static float CalculateRadius(ComponentModel& super);
        static void RegisterDeferredMeshLoaded(ComponentModel& super, Mesh* mesh);
    };
};

#include <ecs/Entity.h>
#include <ecs/ECSSystemConstructorInfo.h>
#include <core/engine/model/ModelInstance.h>

typedef std::vector<ModelInstance*> ModelInstanceVector;

class ComponentModel: public EventObserver {
    friend struct Engine::priv::ComponentModel_UpdateFunction;
    friend struct Engine::priv::ComponentModel_EntityAddedToSceneFunction;
    friend struct Engine::priv::ComponentModel_ComponentAddedToEntityFunction;
    friend struct Engine::priv::ComponentModel_ComponentRemovedFromEntityFunction;
    friend struct Engine::priv::ComponentModel_SceneEnteredFunction;
    friend struct Engine::priv::ComponentModel_SceneLeftFunction;
    friend struct Engine::priv::ComponentModel_Functions;
    friend class  ComponentCamera;
    private:
        Entity m_Owner;

        ModelInstanceVector          m_ModelInstances;
        float                        m_Radius;
        glm::vec3                    m_RadiusBox;

        ComponentModel() = delete;
    public:
        BOOST_TYPE_INDEX_REGISTER_CLASS
        explicit ComponentModel(const Entity, const Handle meshHandle, const Handle materialHandle, ShaderProgram* = 0,         const RenderStage::Stage = RenderStage::GeometryOpaque);
        explicit ComponentModel(const Entity, Mesh*,                   const Handle materialHandle, ShaderProgram* = 0,         const RenderStage::Stage = RenderStage::GeometryOpaque);
        explicit ComponentModel(const Entity, const Handle meshHandle, Material*,                   ShaderProgram* = 0,         const RenderStage::Stage = RenderStage::GeometryOpaque);
        explicit ComponentModel(const Entity, Mesh*,                   Material*,                   ShaderProgram* = 0,         const RenderStage::Stage = RenderStage::GeometryOpaque);

        explicit ComponentModel(const Entity, const Handle meshHandle, const Handle materialHandle, const Handle shaderPHandle, const RenderStage::Stage = RenderStage::GeometryOpaque);
        explicit ComponentModel(const Entity, Mesh*,                   const Handle materialHandle, const Handle shaderPHandle, const RenderStage::Stage = RenderStage::GeometryOpaque);
        explicit ComponentModel(const Entity, const Handle meshHandle, Material*,                   const Handle shaderPHandle, const RenderStage::Stage = RenderStage::GeometryOpaque);
        explicit ComponentModel(const Entity, Mesh*,                   Material*,                   const Handle shaderPHandle, const RenderStage::Stage = RenderStage::GeometryOpaque);

        ComponentModel(const ComponentModel& other)                = delete;
        ComponentModel& operator=(const ComponentModel& other)     = delete;
        ComponentModel(ComponentModel&& other) noexcept;
        ComponentModel& operator=(ComponentModel&& other) noexcept;

        ~ComponentModel();

        Entity getOwner() const;

        void onEvent(const Event& event_) override;

        void setViewportFlag(const unsigned int flag);
        void addViewportFlag(const unsigned int flag);
        void setViewportFlag(const ViewportFlag::Flag flag);
        void addViewportFlag(const ViewportFlag::Flag flag);

        size_t getNumModels() const;
        float radius() const;
        const glm::vec3& boundingBox() const;
        void show();
        void hide();

        ModelInstance& getModel(const size_t& index = 0) const;
        void removeModel(const size_t& index);
        ModelInstance& addModel(const Handle meshHandle, const Handle materialHandle, ShaderProgram* = 0,         const RenderStage::Stage = RenderStage::GeometryOpaque);
        ModelInstance& addModel(Mesh*,                   Material*,                   ShaderProgram* = 0,         const RenderStage::Stage = RenderStage::GeometryOpaque);
        ModelInstance& addModel(const Handle meshHandle, const Handle materialHandle, const Handle shaderProgram, const RenderStage::Stage = RenderStage::GeometryOpaque);
        ModelInstance& addModel(Mesh*,                   Material*,                   const Handle shaderProgram, const RenderStage::Stage = RenderStage::GeometryOpaque);


        void setStage(const RenderStage::Stage& stage, const size_t& index = 0);

        void setModel(const Handle meshHandle, const Handle materialHandle, const size_t& index, ShaderProgram* = 0, const RenderStage::Stage = RenderStage::GeometryOpaque);
        void setModel(Mesh*,                   Material*,                   const size_t& index, ShaderProgram* = 0, const RenderStage::Stage = RenderStage::GeometryOpaque);

        void setModelMesh(Mesh*,                   const size_t& index, const RenderStage::Stage = RenderStage::GeometryOpaque);
        void setModelMesh(const Handle meshHandle, const size_t& index, const RenderStage::Stage = RenderStage::GeometryOpaque);

        void setModelMaterial(Material*,                   const size_t& index, const RenderStage::Stage = RenderStage::GeometryOpaque);
        void setModelMaterial(const Handle materialHandle, const size_t& index, const RenderStage::Stage = RenderStage::GeometryOpaque);

        void setModelShaderProgram(ShaderProgram*,              const size_t& index, const RenderStage::Stage = RenderStage::GeometryOpaque);
        void setModelShaderProgram(const Handle materialHandle, const size_t& index, const RenderStage::Stage = RenderStage::GeometryOpaque);

        bool rayIntersectSphere(const ComponentCamera& camera) const;

        void setUserPointer(void* UserPointer);

        template<typename T> void setCustomBindFunctor(const T& functor, const size_t& index = 0) {
            m_ModelInstances[index]->setCustomBindFunctor(functor);
        }
        template<typename T> void setCustomUnbindFunctor(const T& functor, const size_t& index = 0) {
            m_ModelInstances[index]->setCustomUnbindFunctor(functor);
        }

        ModelInstance& operator[](const size_t index) {
            return *m_ModelInstances[index];
        }
        const ModelInstance& operator[](const size_t index) const {
            return *m_ModelInstances[index];
        }

        ModelInstanceVector::iterator begin() {
            return m_ModelInstances.begin();
        }
        ModelInstanceVector::iterator end() {
            return m_ModelInstances.end();
        }
        ModelInstanceVector::const_iterator begin() const {
            return m_ModelInstances.begin();
        }
        ModelInstanceVector::const_iterator end() const {
            return m_ModelInstances.end();
        }
};

class ComponentModel_System_CI : public Engine::priv::ECSSystemCI {
    public:
        ComponentModel_System_CI();
        ~ComponentModel_System_CI() = default;
};

#endif