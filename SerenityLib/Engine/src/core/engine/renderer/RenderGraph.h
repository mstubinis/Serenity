#pragma once
#ifndef ENGINE_RENDER_GRAPH_INCLUDE_GUARD
#define ENGINE_RENDER_GRAPH_INCLUDE_GUARD

#include <core/engine/Engine_Utils.h>

class  Mesh;
class  Material;
class  MeshInstance;
class  ShaderP;
class  Camera;
class  Scene;

namespace Engine {
    namespace epriv {
        struct InstanceNode {
            MeshInstance* instance;
            InstanceNode(MeshInstance& i) :instance(&i) {}
        };
        struct MeshNode {
            Mesh* mesh;
            std::vector<InstanceNode*> instanceNodes;
            MeshNode(Mesh& m) :mesh(&m) {}
            ~MeshNode() {
                SAFE_DELETE_VECTOR(instanceNodes);
            }
        };
        struct MaterialNode {
            Material* material;
            std::vector<MeshNode*> meshNodes;
            MaterialNode(Material& m) :material(&m) {}
            ~MaterialNode() {
                SAFE_DELETE_VECTOR(meshNodes);
            }
        };
        class RenderPipeline final {
            friend class ::Scene;
            private:
                ShaderP& shaderProgram;
                std::vector<MaterialNode*> materialNodes;
            public:
                RenderPipeline(ShaderP&);
                ~RenderPipeline();

                void sort(Camera& c);
                void sort_cheap(Camera& c);
                void render();
        };
    };
};
#endif