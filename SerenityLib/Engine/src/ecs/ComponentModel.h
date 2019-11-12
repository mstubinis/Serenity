#pragma once
#ifndef ENGINE_ECS_COMPONENT_MODEL_H
#define ENGINE_ECS_COMPONENT_MODEL_H

#include <ecs/ComponentBaseClass.h>
#include <ecs/ECSSystemConstructorInfo.h>
#include <core/engine/model/ModelInstance.h>

struct Handle;
class  ShaderProgram;
class  Mesh;
class  Material;
class  ComponentModel;
class  ComponentCamera;
namespace Engine {
    namespace epriv {
        struct ComponentModel_UpdateFunction;
        struct ComponentModel_EntityAddedToSceneFunction;
        struct ComponentModel_ComponentAddedToEntityFunction;
        struct ComponentModel_SceneEnteredFunction;
        struct ComponentModel_SceneLeftFunction;
        struct ComponentModel_Functions final {
            static float CalculateRadius(ComponentModel& super);
        };
    };
};

class ComponentModel: public ComponentBaseClass {
    friend struct Engine::epriv::ComponentModel_UpdateFunction;
    friend struct Engine::epriv::ComponentModel_EntityAddedToSceneFunction;
    friend struct Engine::epriv::ComponentModel_ComponentAddedToEntityFunction;
    friend struct Engine::epriv::ComponentModel_SceneEnteredFunction;
    friend struct Engine::epriv::ComponentModel_SceneLeftFunction;
    friend struct Engine::epriv::ComponentModel_Functions;
    friend class  ComponentCamera;
    private:
        std::vector<ModelInstance*>  m_ModelInstances;
        float                        m_Radius;
        glm::vec3                    m_RadiusBox;
    public:
        BOOST_TYPE_INDEX_REGISTER_CLASS
        explicit ComponentModel(const Entity&, Handle& meshHandle, Handle& materialHandle, ShaderProgram* = 0, const RenderStage::Stage& = RenderStage::GeometryOpaque);
        explicit ComponentModel(const Entity&, Mesh*, Handle& materialHandle, ShaderProgram* = 0, const RenderStage::Stage& = RenderStage::GeometryOpaque);
        explicit ComponentModel(const Entity&, Handle& meshHandle, Material*, ShaderProgram* = 0, const RenderStage::Stage& = RenderStage::GeometryOpaque);
        explicit ComponentModel(const Entity&, Mesh*, Material*, ShaderProgram* = 0, const RenderStage::Stage& = RenderStage::GeometryOpaque);

        explicit ComponentModel(const Entity&, Handle& meshHandle, Handle& materialHandle, Handle& shaderPHandle, const RenderStage::Stage& = RenderStage::GeometryOpaque);
        explicit ComponentModel(const Entity&, Mesh*, Handle& materialHandle, Handle& shaderPHandle, const RenderStage::Stage& = RenderStage::GeometryOpaque);
        explicit ComponentModel(const Entity&, Handle& meshHandle, Material*, Handle& shaderPHandle, const RenderStage::Stage& = RenderStage::GeometryOpaque);
        explicit ComponentModel(const Entity&, Mesh*, Material*, Handle& shaderPHandle, const RenderStage::Stage& = RenderStage::GeometryOpaque);

        ComponentModel(const ComponentModel& other)                = delete;
        ComponentModel& operator=(const ComponentModel& other)     = delete;
        ComponentModel(ComponentModel&& other) noexcept            = default;
        ComponentModel& operator=(ComponentModel&& other) noexcept = default;

        ~ComponentModel();

        const size_t getNumModels() const;
        const float& radius() const;
        const glm::vec3& boundingBox() const;
        void show();
        void hide();

        ModelInstance& getModel(const size_t& index = 0);
        ModelInstance& addModel(Handle& meshHandle, Handle& materialHandle, ShaderProgram* = 0, const RenderStage::Stage& = RenderStage::GeometryOpaque);
        ModelInstance& addModel(Mesh*,              Material*,              ShaderProgram* = 0, const RenderStage::Stage& = RenderStage::GeometryOpaque);
        ModelInstance& addModel(Handle& meshHandle, Handle& materialHandle, Handle& shaderProgram, const RenderStage::Stage & = RenderStage::GeometryOpaque);
        ModelInstance& addModel(Mesh*,              Material*,              Handle& shaderProgram, const RenderStage::Stage & = RenderStage::GeometryOpaque);


        void setStage(const RenderStage::Stage& stage, const uint& index = 0);

        void setModel(Handle& meshHandle, Handle& materialHandle, const uint& index, ShaderProgram* = 0, const RenderStage::Stage& = RenderStage::GeometryOpaque);
        void setModel(Mesh*, Material*, const uint& index, ShaderProgram* = 0, const RenderStage::Stage& = RenderStage::GeometryOpaque);

        void setModelMesh(Mesh*, const uint& index, const RenderStage::Stage& = RenderStage::GeometryOpaque);
        void setModelMesh(Handle& meshHandle, const uint& index, const RenderStage::Stage& = RenderStage::GeometryOpaque);

        void setModelMaterial(Material*, const uint& index, const RenderStage::Stage& = RenderStage::GeometryOpaque);
        void setModelMaterial(Handle& materialHandle, const uint& index, const RenderStage::Stage& = RenderStage::GeometryOpaque);

        void setModelShaderProgram(ShaderProgram*, const uint& index, const RenderStage::Stage& = RenderStage::GeometryOpaque);
        void setModelShaderProgram(Handle& materialHandle, const uint& index, const RenderStage::Stage& = RenderStage::GeometryOpaque);

        const bool rayIntersectSphere(const ComponentCamera& camera);

        template<class T> void setCustomBindFunctor(const T& functor, const uint& index = 0) { 
            m_ModelInstances[index]->setCustomBindFunctor(functor);
        }
        template<class T> void setCustomUnbindFunctor(const T& functor, const uint& index = 0) {
            m_ModelInstances[index]->setCustomUnbindFunctor(functor);
        }
};

class ComponentModel_System_CI : public Engine::epriv::ECSSystemCI {
    public:
        ComponentModel_System_CI();
        ~ComponentModel_System_CI() = default;
};

#endif