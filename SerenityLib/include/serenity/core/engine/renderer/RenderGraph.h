#pragma once
#ifndef ENGINE_RENDER_GRAPH_H
#define ENGINE_RENDER_GRAPH_H

class  ShaderProgram;
class  Camera;
class  Scene;
class  Material;
class  Mesh;
class  ModelInstance;
class  ModelInstanceHandle;
struct Entity;
class  Viewport;
namespace Engine::priv {
    class  RenderGraph;
    class  RenderModule;
    struct InternalScenePublicInterface;
};

#include <serenity/core/engine/utils/Utils.h>
#include <serenity/core/engine/renderer/RendererIncludes.h>
#include <serenity/core/engine/model/ModelInstanceHandle.h>
#include <serenity/core/engine/resources/Handle.h>

namespace Engine::priv {
    class MeshNode final {
        friend class  RenderGraph;
        friend struct InternalScenePublicInterface;
        private:
            Handle                       mesh = Handle{};
            std::vector<ModelInstance*>  instanceNodes;

            MeshNode() = delete;
        public:
            MeshNode(Handle mesh_)
                : mesh{ mesh_ }
            {}
    };
    class MaterialNode final {
        friend class  RenderGraph;
        friend struct InternalScenePublicInterface;
        private:
            Handle                 material = Handle{};
            std::vector<MeshNode>  meshNodes;

            MaterialNode() = delete;
        public:
            MaterialNode(Handle material_)
                : material{ material_ }
            {}
    };
    class RenderGraph final {
        friend class  Scene;
        friend struct Engine::priv::InternalScenePublicInterface;
        private:
            Handle                        m_ShaderProgram = Handle{};
            std::vector<MaterialNode>     m_MaterialNodes;
            std::vector<ModelInstance*>   m_InstancesTotal;

            RenderGraph() = delete;

            void addModelInstanceToPipeline(ModelInstance& modelInstance, ComponentModel&);
            void removeModelInstanceFromPipeline(ModelInstance& modelInstance);
        public:
            RenderGraph(Handle shaderProgram)
                : m_ShaderProgram{ shaderProgram }
            {}

            RenderGraph(const RenderGraph& other) = delete;
            RenderGraph& operator=(const RenderGraph& other) = delete;
            RenderGraph(RenderGraph&& other) noexcept = default;
            RenderGraph& operator=(RenderGraph&& other) noexcept = default;

            bool remove_material_node(MaterialNode& materialNode);
            bool remove_mesh_node(MaterialNode& materialNode, MeshNode& meshNode);
            bool remove_instance_node(MeshNode& meshNode, ModelInstance& instanceNode);

            void clean(Entity entity);
            void sort(Camera& camera, SortingMode sortingMode);
            void sort_cheap(Camera& camera, SortingMode sortingMode);

            void sort_bruteforce(Camera& camera, SortingMode sortingMode);
            void sort_cheap_bruteforce(Camera& camera, SortingMode sortingMode);

            void render(Engine::priv::RenderModule& renderer, Viewport& viewport, Camera& camera, bool useDefaultShaders = true, SortingMode sortingMode = SortingMode::None);
            void render_bruteforce(Engine::priv::RenderModule& renderer, Viewport& viewport, Camera& camera, bool useDefaultShaders = true, SortingMode sortingMode = SortingMode::None);
            void validate_model_instances_for_rendering(Viewport& viewport, Camera& camera);

    };
};
#endif