#pragma once
#ifndef ENGINE_RENDER_GRAPH_INCLUDE_GUARD
#define ENGINE_RENDER_GRAPH_INCLUDE_GUARD

class  ShaderProgram;
class  Camera;
class  Scene;
class  Material;
class  Mesh;
class  ModelInstance;
struct Entity;
class  Viewport;
namespace Engine::priv {
    class  RenderGraph;
    class  Renderer;
    struct InternalScenePublicInterface;
};

#include <core/engine/utils/Utils.h>
#include <core/engine/renderer/RendererIncludes.h>

namespace Engine::priv {
    class InstanceNode final : public Engine::NonCopyable {
        friend class  RenderGraph;
        friend struct InternalScenePublicInterface;
        private:
            ModelInstance*   instance = nullptr;

            InstanceNode() = delete;
        public:
            InstanceNode(ModelInstance& modelInstance);
            ~InstanceNode();

            InstanceNode(InstanceNode&& other) noexcept;
            InstanceNode& operator=(InstanceNode&& other) noexcept;
    };
    class MeshNode final : public Engine::NonCopyable {
        friend class  RenderGraph;
        friend struct InternalScenePublicInterface;
        private:
            Mesh*                        mesh = nullptr;
            std::vector<InstanceNode*>   instanceNodes;

            MeshNode() = delete;
        public:
            MeshNode(Mesh& mesh);
            ~MeshNode();

            MeshNode(MeshNode&& other) noexcept;
            MeshNode& operator=(MeshNode&& other) noexcept;
    };
    class MaterialNode final : public Engine::NonCopyable{
        friend class  RenderGraph;
        friend struct InternalScenePublicInterface;
        private:
            Material*                material = nullptr;
            std::vector<MeshNode>    meshNodes;

            MaterialNode() = delete;
        public:
            MaterialNode(Material& material);
            ~MaterialNode();

            MaterialNode(MaterialNode&& other) noexcept;
            MaterialNode& operator=(MaterialNode&& other) noexcept;
    };
    class RenderGraph final : public Engine::NonCopyable {
        friend class  Scene;
        friend struct Engine::priv::InternalScenePublicInterface;
        private:
            ShaderProgram*               m_ShaderProgram = nullptr;
            std::vector<MaterialNode>    m_MaterialNodes;
            std::vector<InstanceNode*>   m_InstancesTotal;

            RenderGraph() = delete;

            void addModelInstanceToPipeline(ModelInstance& modelInstance);
            void removeModelInstanceFromPipeline(ModelInstance& modelInstance);
        public:
            RenderGraph(ShaderProgram&);
            ~RenderGraph();

            RenderGraph(RenderGraph&& other) noexcept;
            RenderGraph& operator=(RenderGraph&& other) noexcept;

            bool remove_material_node(MaterialNode& materialNode);
            bool remove_mesh_node(MaterialNode& materialNode, MeshNode& meshNode);
            bool remove_instance_node(MeshNode& meshNode, InstanceNode& instanceNode);

            void clean(Entity entity);
            void sort(Camera& camera, SortingMode::Mode sortingMode);
            void sort_cheap(Camera& camera, SortingMode::Mode sortingMode);

            void sort_bruteforce(Camera& camera, SortingMode::Mode sortingMode);
            void sort_cheap_bruteforce(Camera& camera, SortingMode::Mode sortingMode);

            void render(Engine::priv::Renderer& renderer, Viewport& viewport, Camera& camera, bool useDefaultShaders = true, SortingMode::Mode sortingMode = SortingMode::None);
            void render_bruteforce(Engine::priv::Renderer& renderer, Viewport& viewport, Camera& camera, bool useDefaultShaders = true, SortingMode::Mode sortingMode = SortingMode::None);
            void validate_model_instances_for_rendering(Viewport& viewport, Camera& camera);

    };
};
#endif