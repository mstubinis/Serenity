#pragma once
#ifndef ENGINE_ECS_COMPONENT_MODEL_H
#define ENGINE_ECS_COMPONENT_MODEL_H

#include "ComponentBaseClass.h"

/*

struct Handle;
class ShaderP;
class Mesh;
class Material;
class MeshInstance;

class ComponentCamera;

class ComponentModel : public ComponentBaseClass {
    private:
        std::vector<MeshInstance*> models;
        float _radius;
        glm::vec3 _radiusBox;
    public:
        BOOST_TYPE_INDEX_REGISTER_CLASS
        ComponentModel(Handle& meshHandle, Handle& materialHandle, Entity*, ShaderP* = 0, RenderStage::Stage = RenderStage::GeometryOpaque);
        ComponentModel(Mesh*, Handle& materialHandle, Entity*, ShaderP* = 0, RenderStage::Stage = RenderStage::GeometryOpaque);
        ComponentModel(Handle& meshHandle, Material*, Entity*, ShaderP* = 0, RenderStage::Stage = RenderStage::GeometryOpaque);
        ComponentModel(Mesh*, Material*, Entity*, ShaderP* = 0, RenderStage::Stage = RenderStage::GeometryOpaque);

        ComponentModel(Handle& meshHandle, Handle& materialHandle, Entity*, Handle& shaderPHandle, RenderStage::Stage = RenderStage::GeometryOpaque);
        ComponentModel(Mesh*, Handle& materialHandle, Entity*, Handle& shaderPHandle, RenderStage::Stage = RenderStage::GeometryOpaque);
        ComponentModel(Handle& meshHandle, Material*, Entity*, Handle& shaderPHandle, RenderStage::Stage = RenderStage::GeometryOpaque);
        ComponentModel(Mesh*, Material*, Entity*, Handle& shaderPHandle, RenderStage::Stage = RenderStage::GeometryOpaque);

        ~ComponentModel();

        uint getNumModels();
        float radius();
        glm::vec3 boundingBox();
        void show();
        void hide();

        MeshInstance* getModel(uint index = 0);

        uint addModel(Handle& meshHandle, Handle& materialHandle, ShaderP* = 0, RenderStage::Stage = RenderStage::GeometryOpaque);
        uint addModel(Mesh*, Material*, ShaderP* = 0, RenderStage::Stage = RenderStage::GeometryOpaque);

        void setModel(Handle& meshHandle, Handle& materialHandle, uint index, ShaderP* = 0, RenderStage::Stage = RenderStage::GeometryOpaque);
        void setModel(Mesh*, Material*, uint index, ShaderP* = 0, RenderStage::Stage = RenderStage::GeometryOpaque);

        void setModelMesh(Mesh*, uint index, RenderStage::Stage = RenderStage::GeometryOpaque);
        void setModelMesh(Handle& meshHandle, uint index, RenderStage::Stage = RenderStage::GeometryOpaque);

        void setModelMaterial(Material*, uint index, RenderStage::Stage = RenderStage::GeometryOpaque);
        void setModelMaterial(Handle& materialHandle, uint index, RenderStage::Stage = RenderStage::GeometryOpaque);

        void setModelShaderProgram(ShaderP*, uint index, RenderStage::Stage = RenderStage::GeometryOpaque);
        void setModelShaderProgram(Handle& materialHandle, uint index, RenderStage::Stage = RenderStage::GeometryOpaque);

        bool rayIntersectSphere(ComponentCamera& camera);

        template<class T> void setCustomBindFunctor(T& functor, uint index = 0) { models.at(index)->setCustomBindFunctor(functor); }
        template<class T> void setCustomUnbindFunctor(T& functor, uint index = 0) { models.at(index)->setCustomUnbindFunctor(functor); }
};

*/

#endif