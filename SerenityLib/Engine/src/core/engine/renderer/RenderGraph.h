#pragma once
#ifndef ENGINE_RENDER_GRAPH_INCLUDE_GUARD
#define ENGINE_RENDER_GRAPH_INCLUDE_GUARD

#include <core/engine/utils/Utils.h>
#include <core/engine/renderer/RendererIncludes.h>

class  ShaderProgram;
class  Camera;
class  Scene;
class  Material;
class  Mesh;
class  ModelInstance;
struct Entity;
class  Viewport;
namespace Engine {
    namespace epriv {
        struct InternalScenePublicInterface;
        struct InstanceNode final {
            ModelInstance* instance;
            InstanceNode(const ModelInstance& modelInstance) : instance(&const_cast<ModelInstance&>(modelInstance)) {
            }
        };
        struct MeshNode final {
            Mesh* mesh;
            std::vector<InstanceNode*> instanceNodes;
            MeshNode(const Mesh& mesh_) : mesh(&const_cast<Mesh&>(mesh_)) {
            }
            ~MeshNode() {
                SAFE_DELETE_VECTOR(instanceNodes);
            }
        };
        struct MaterialNode final {
            Material* material;
            std::vector<MeshNode*> meshNodes;
            MaterialNode(const Material& material_) : material(&const_cast<Material&>(material_)) {
            }
            ~MaterialNode() {
                SAFE_DELETE_VECTOR(meshNodes);
            }
        };
        class RenderGraph final {
            friend class  Scene;
            friend struct Engine::epriv::InternalScenePublicInterface;
            private:
                ShaderProgram&               shaderProgram;
                std::vector<MaterialNode*>   materialNodes;
                std::vector<InstanceNode*>   instancesTotal;
            public:
                RenderGraph(ShaderProgram&);
                ~RenderGraph();

                void clean(const uint entityData);
                void sort(Camera& camera, const SortingMode::Mode sortingMode);
                void sort_cheap(Camera& camera, const SortingMode::Mode sortingMode);

                void sort_bruteforce(Camera& camera, const SortingMode::Mode sortingMode);
                void sort_cheap_bruteforce(Camera& camera, const SortingMode::Mode sortingMode);

                void render(Viewport& viewport, Camera& camera, const bool useDefaultShaders = true, const SortingMode::Mode sortingMode = SortingMode::None);
                void render_bruteforce(Viewport& viewport, Camera& camera, const bool useDefaultShaders = true, const SortingMode::Mode sortingMode = SortingMode::None);
                void validate_model_instances_for_rendering(Viewport& viewport, Camera& camera);

        };
    };
};
#endif