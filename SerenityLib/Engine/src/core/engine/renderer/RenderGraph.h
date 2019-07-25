#pragma once
#ifndef ENGINE_RENDER_GRAPH_INCLUDE_GUARD
#define ENGINE_RENDER_GRAPH_INCLUDE_GUARD

#include <core/engine/Engine_Utils.h>

class  ShaderP;
class  Camera;
class  Scene;
class  Material;
class  Mesh;
class  ModelInstance;

namespace Engine {
    namespace epriv {
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
        class RenderPipeline final {
            friend class ::Scene;
            private:
                ShaderP&                    shaderProgram;
                std::vector<MaterialNode*>  materialNodes;
            public:
                RenderPipeline(ShaderP&);
                ~RenderPipeline();

                void sort(Camera& c);
                void sort_cheap(Camera& c);
                void render(Camera& c);
        };
    };
};
#endif