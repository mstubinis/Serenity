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
    struct InternalScenePublicInterface;
};

#include <core/engine/utils/Utils.h>
#include <core/engine/renderer/RendererIncludes.h>

namespace Engine::priv {
    class InstanceNode final : public Engine::NonCopyable {
        friend class  RenderGraph;
        friend struct InternalScenePublicInterface;
        private:
            ModelInstance*   instance;

            InstanceNode() = delete;
        public:
            InstanceNode(const ModelInstance& modelInstance);
            ~InstanceNode();

            InstanceNode(InstanceNode&& other) noexcept;
            InstanceNode& operator=(InstanceNode&& other) noexcept;
    };
    class MeshNode final : public Engine::NonCopyable {
        friend class  RenderGraph;
        friend struct InternalScenePublicInterface;
        private:
            Mesh*                        mesh;
            std::vector<InstanceNode*>   instanceNodes;

            MeshNode() = delete;
        public:
            MeshNode(const Mesh& mesh);
            ~MeshNode();

            MeshNode(MeshNode&& other) noexcept;
            MeshNode& operator=(MeshNode&& other) noexcept;
    };
    class MaterialNode final : public Engine::NonCopyable{
        friend class  RenderGraph;
        friend struct InternalScenePublicInterface;
        private:
            Material*                material;
            std::vector<MeshNode>    meshNodes;

            MaterialNode() = delete;
        public:
            MaterialNode(const Material& material);
            ~MaterialNode();

            MaterialNode(MaterialNode&& other) noexcept;
            MaterialNode& operator=(MaterialNode&& other) noexcept;
    };
    class RenderGraph final : public Engine::NonCopyable {
        friend class  Scene;
        friend struct Engine::priv::InternalScenePublicInterface;
        private:
            ShaderProgram*               m_ShaderProgram;
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

            const bool remove_material_node(const MaterialNode& materialNode);
            const bool remove_mesh_node(MaterialNode& materialNode, const MeshNode& meshNode);
            const bool remove_instance_node(MeshNode& meshNode, const InstanceNode& instanceNode);

            void clean(const uint entityData);
            void sort(const Camera& camera, const SortingMode::Mode sortingMode);
            void sort_cheap(const Camera& camera, const SortingMode::Mode sortingMode);

            void sort_bruteforce(const Camera& camera, const SortingMode::Mode sortingMode);
            void sort_cheap_bruteforce(const Camera& camera, const SortingMode::Mode sortingMode);

            void render(const Viewport& viewport, const Camera& camera, const bool useDefaultShaders = true, const SortingMode::Mode sortingMode = SortingMode::None);
            void render_bruteforce(const Viewport& viewport, const Camera& camera, const bool useDefaultShaders = true, const SortingMode::Mode sortingMode = SortingMode::None);
            void validate_model_instances_for_rendering(const Viewport& viewport, const Camera& camera);

    };
};
#endif