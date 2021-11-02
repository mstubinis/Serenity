#pragma once
#ifndef ENGINE_RENDER_GRAPH_H
#define ENGINE_RENDER_GRAPH_H

class  Camera;
class  Scene;
class  Material;
class  Mesh;
class  ModelInstance;
class  Entity;
class  Viewport;
namespace Engine::priv {
    class  RenderGraph;
    class  RenderGraphContainer;
    class  RenderModule;
    class  MeshNode;
    class  MaterialNode;
    struct PublicScene;
};

#include <serenity/renderer/RendererIncludes.h>
#include <serenity/system/Macros.h>
#include <serenity/resources/Handle.h>
#include <vector>
#include <array>

namespace Engine::priv {
    class MeshNode final {
        friend class  RenderGraph;
        friend struct PublicScene;
        private:
            Handle                       meshHandle;
            std::vector<ModelInstance*>  instanceNodes;

            MeshNode() = delete;
        public:
            MeshNode(Handle mesh_)
                : meshHandle{ mesh_ }
            {
                assert(!meshHandle.null());
            }
            
            inline bool operator<(const MeshNode& other) const noexcept { return meshHandle < other.meshHandle; }
            inline bool operator>(const MeshNode& other) const noexcept { return meshHandle > other.meshHandle; }
            inline bool operator<=(const MeshNode& other) const noexcept { return meshHandle <= other.meshHandle; }
            inline bool operator>=(const MeshNode& other) const noexcept { return meshHandle >= other.meshHandle; }
            
    };
    class MaterialNode final {
        friend class  RenderGraph;
        friend struct PublicScene;
        private:
            Handle                 materialHandle;
            std::vector<MeshNode>  meshNodes;

            MaterialNode() = delete;
        public:
            MaterialNode(Handle material_)
                : materialHandle{ material_ }
            {
                assert(!materialHandle.null());
            }
            
            inline bool operator<(const MaterialNode& other) const noexcept { return materialHandle < other.materialHandle; }
            inline bool operator>(const MaterialNode& other) const noexcept { return materialHandle > other.materialHandle; }
            inline bool operator<=(const MaterialNode& other) const noexcept { return materialHandle <= other.materialHandle; }
            inline bool operator>=(const MaterialNode& other) const noexcept { return materialHandle >= other.materialHandle; }
            
    };
    class RenderGraph final {
        friend class  Scene;
        friend class  RenderGraphContainer;
        friend struct Engine::priv::PublicScene;
        private:
            std::vector<MaterialNode>     m_MaterialNodes;
            std::vector<ModelInstance*>   m_InstancesTotal;
            const Handle                  m_ShaderProgram;

            RenderGraph() = delete;

            void internal_sort_impl(Camera*, SortingMode, std::vector<ModelInstance*>&);
            void internal_sort_cheap_impl(Camera*, SortingMode, std::vector<ModelInstance*>&);
            void internal_scan_nodes(const ModelInstance&, Engine::priv::MaterialNode*&, Engine::priv::MeshNode*&, ModelInstance*&);
            void internal_addModelInstanceToPipeline(ModelInstance&);
            void internal_removeModelInstanceFromPipeline(ModelInstance&);
        public:
            RenderGraph(const Handle shaderProgram)
                : m_ShaderProgram{ shaderProgram }
            {}

            RenderGraph(const RenderGraph&)                = delete;
            RenderGraph& operator=(const RenderGraph&)     = delete;
            RenderGraph(RenderGraph&&) noexcept            = default;
            RenderGraph& operator=(RenderGraph&&) noexcept = default;

            template<class FUNC> void iterateMaterials(FUNC&& func) {
                for (auto& materialNode : m_MaterialNodes) {
                    func(*materialNode.materialHandle.get<Material>());
                }
            }

            bool remove_material_node(const MaterialNode*);
            bool remove_mesh_node(MaterialNode*, const MeshNode*);
            bool remove_instance_node(MeshNode*, const ModelInstance*);

            void clean(Entity);

            //accurate sort that takes object radius into account
            void sort(Camera*, SortingMode);
            void sort_bruteforce(Camera*, SortingMode);

            //not as accurate, only takes position into account but its cheaper to compute
            void sort_cheap(Camera*, SortingMode);
            void sort_cheap_bruteforce(Camera*, SortingMode);

            void render(Engine::priv::RenderModule&, Camera*, bool useDefaultShaders = true, SortingMode = SortingMode::None);
            void render_bruteforce(Engine::priv::RenderModule&, Camera*, bool useDefaultShaders = true, SortingMode = SortingMode::None);

            void render_shadow_map(Engine::priv::RenderModule&, Camera*);
            void render_bruteforce_shadow_map(Engine::priv::RenderModule&, Camera*);

            void validate_model_instances_for_rendering(Camera*, Viewport*);
            void validate_model_instances_for_rendering(const glm::mat4& viewProj, Viewport*);
    };

    class RenderGraphContainer final {
        using ContainerType = std::array<std::vector<Engine::priv::RenderGraph>, (size_t)RenderStage::_TOTAL>;
        private:
            ContainerType m_RenderGraphs;
        public:
            bool hasItemsToRender() const noexcept { 
                for (const auto& stage : m_RenderGraphs) {
                    for (const auto& graph : stage) {
                        if (graph.m_InstancesTotal.size() > 0) {
                            return true;
                        }
                    }
                }
                return false;
            }
            inline std::vector<Engine::priv::RenderGraph>& operator[](const size_t idx) noexcept { return m_RenderGraphs[idx]; }

            BUILD_BEGIN_END_ITR_CLASS_MEMBERS(ContainerType, m_RenderGraphs)
    };
};
#endif