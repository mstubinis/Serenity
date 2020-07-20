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
    struct ComponentModel_Functions final {
        static float CalculateRadius(ComponentModel& super);
        static void RegisterDeferredMeshLoaded(ComponentModel& super, Mesh* mesh);
    };
};

#include <ecs/Entity.h>
#include <ecs/ECSSystemConstructorInfo.h>
#include <core/engine/model/ModelInstance.h>
//#include <boost/type_index.hpp>

using ModelInstanceVector = std::vector<ModelInstance*>;

class ComponentModel: public Observer {
    friend struct Engine::priv::ComponentModel_UpdateFunction;
    friend struct Engine::priv::ComponentModel_Functions;
    friend class  ComponentCamera;
    private:
        Entity m_Owner;

        ModelInstanceVector          m_ModelInstances;
        float                        m_Radius          = 0.0f;
        glm::vec3                    m_RadiusBox       = glm::vec3(0.0f);

        ComponentModel() = delete;
    public:
        //BOOST_TYPE_INDEX_REGISTER_CLASS
        explicit ComponentModel(Entity, Handle meshHandle, Handle materialHandle, ShaderProgram* = 0, RenderStage = RenderStage::GeometryOpaque);
        explicit ComponentModel(Entity, Mesh*,             Handle materialHandle, ShaderProgram* = 0, RenderStage = RenderStage::GeometryOpaque);
        explicit ComponentModel(Entity, Handle meshHandle, Material*,             ShaderProgram* = 0, RenderStage = RenderStage::GeometryOpaque);
        explicit ComponentModel(Entity, Mesh*,             Material*,             ShaderProgram* = 0, RenderStage = RenderStage::GeometryOpaque);

        explicit ComponentModel(Entity, Handle meshHandle, Handle materialHandle, Handle shaderPHandle, RenderStage = RenderStage::GeometryOpaque);
        explicit ComponentModel(Entity, Mesh*,             Handle materialHandle, Handle shaderPHandle, RenderStage = RenderStage::GeometryOpaque);
        explicit ComponentModel(Entity, Handle meshHandle, Material*,             Handle shaderPHandle, RenderStage = RenderStage::GeometryOpaque);
        explicit ComponentModel(Entity, Mesh*,             Material*,             Handle shaderPHandle, RenderStage = RenderStage::GeometryOpaque);

        ComponentModel(const ComponentModel& other)                = delete;
        ComponentModel& operator=(const ComponentModel& other)     = delete;
        ComponentModel(ComponentModel&& other) noexcept;
        ComponentModel& operator=(ComponentModel&& other) noexcept;

        ~ComponentModel();

        Entity getOwner() const;

        void onEvent(const Event& event_) override;

        void setViewportFlag(unsigned int flag);
        void addViewportFlag(unsigned int flag);
        void setViewportFlag(ViewportFlag::Flag flag);
        void addViewportFlag(ViewportFlag::Flag flag);

        size_t getNumModels() const;
        float radius() const;
        const glm::vec3& boundingBox() const;
        void show();
        void hide();

        ModelInstance& getModel(size_t index = 0) const;
        void removeModel(size_t index);
        ModelInstance& addModel(Handle meshHandle, Handle materialHandle, ShaderProgram* = 0,   RenderStage = RenderStage::GeometryOpaque);
        ModelInstance& addModel(Mesh*,             Material*,             ShaderProgram* = 0,   RenderStage = RenderStage::GeometryOpaque);
        ModelInstance& addModel(Handle meshHandle, Handle materialHandle, Handle shaderProgram, RenderStage = RenderStage::GeometryOpaque);
        ModelInstance& addModel(Mesh*,             Material*,             Handle shaderProgram, RenderStage = RenderStage::GeometryOpaque);


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

        template<typename T> void setCustomBindFunctor(const T& functor, size_t index = 0) {
            m_ModelInstances[index]->setCustomBindFunctor(functor);
        }
        template<typename T> void setCustomUnbindFunctor(const T& functor, size_t index = 0) {
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