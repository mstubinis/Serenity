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
class  Entity;
class  Viewport;
namespace Engine::priv {
    class  RenderGraph;
    class  RenderGraphContainer;
    class  RenderModule;
    struct PublicScene;
};

#include <serenity/utils/Utils.h>
#include <serenity/renderer/RendererIncludes.h>
#include <serenity/model/ModelInstanceHandle.h>
#include <serenity/resources/Handle.h>
#include <array>

namespace Engine::priv {
    class MeshNode final {
        friend class  RenderGraph;
        friend struct PublicScene;
        private:
            Handle                       mesh;
            std::vector<ModelInstance*>  instanceNodes;

            MeshNode() = delete;
        public:
            MeshNode(Handle mesh_)
                : mesh{ mesh_ }
            {}
    };
    class MaterialNode final {
        friend class  RenderGraph;
        friend struct PublicScene;
        private:
            Handle                 material;
            std::vector<MeshNode>  meshNodes;

            MaterialNode() = delete;
        public:
            MaterialNode(Handle material_)
                : material{ material_ }
            {}
    };
    class RenderGraph final {
        friend class  Scene;
        friend class  RenderGraphContainer;
        friend struct Engine::priv::PublicScene;
        private:
            Handle                        m_ShaderProgram;
            std::vector<MaterialNode>     m_MaterialNodes;
            std::vector<ModelInstance*>   m_InstancesTotal;

            RenderGraph() = delete;

            void internal_addModelInstanceToPipeline(ModelInstance&);
            void internal_removeModelInstanceFromPipeline(ModelInstance&);
        public:
            RenderGraph(Handle shaderProgram)
                : m_ShaderProgram{ shaderProgram }
            {}

            RenderGraph(const RenderGraph&)                = delete;
            RenderGraph& operator=(const RenderGraph&)     = delete;
            RenderGraph(RenderGraph&&) noexcept            = default;
            RenderGraph& operator=(RenderGraph&&) noexcept = default;

            bool remove_material_node(MaterialNode&);
            bool remove_mesh_node(MaterialNode&, MeshNode&);
            bool remove_instance_node(MeshNode&, ModelInstance&);

            void clean(Entity entity);

            void sort(Camera*, SortingMode);
            void sort_cheap(Camera*, SortingMode);
            void sort_bruteforce(Camera*, SortingMode);
            void sort_cheap_bruteforce(Camera*, SortingMode);

            void render(Engine::priv::RenderModule&, Camera*, bool useDefaultShaders = true, SortingMode = SortingMode::None);
            void render_bruteforce(Engine::priv::RenderModule&, Camera*, bool useDefaultShaders = true, SortingMode = SortingMode::None);

            void render_shadow_map(Engine::priv::RenderModule&, Camera*);
            void render_bruteforce_shadow_map(Engine::priv::RenderModule&, Camera*);

            void validate_model_instances_for_rendering(Camera*, Viewport*);
    };

    class RenderGraphContainer final {
        using ContainerType = std::array<std::vector<Engine::priv::RenderGraph>, (size_t)RenderStage::_TOTAL>;
        private:
            ContainerType m_RenderGraphs;
        public:
            bool hasItemsToRender() const noexcept { 
                for (const auto& itr : m_RenderGraphs) {
                    for (const auto& graph : itr) {
                        if (graph.m_InstancesTotal.size() > 0) {
                            return true;
                        }
                    }
                }
                return false;
            }
            inline std::vector<Engine::priv::RenderGraph>& operator[](size_t idx) noexcept { return m_RenderGraphs[idx]; }

            inline ContainerType::iterator begin() noexcept { return m_RenderGraphs.begin(); }
            inline ContainerType::iterator end() noexcept { return m_RenderGraphs.end(); }
            inline ContainerType::const_iterator begin() const noexcept { return m_RenderGraphs.begin(); }
            inline ContainerType::const_iterator end() const noexcept { return m_RenderGraphs.end(); }
    };
};
#endif