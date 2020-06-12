#pragma once
#ifndef ENGINE_MESH_LOADING_INCLUDE_GUARD
#define ENGINE_MESH_LOADING_INCLUDE_GUARD

struct VertexData;
struct aiScene;
struct aiNode;

#include <core/engine/utils/Utils.h>
#include <core/engine/mesh/Skeleton.h>
#include <core/engine/mesh/MeshRequest.h>

namespace Engine::priv {
    struct MeshLoadingFlags final {enum Flag {
        Points  = 1 << 0,
        UVs     = 1 << 1,
        Normals = 1 << 2,
        Faces   = 1 << 3,
        TBN     = 1 << 4,
        //= 1 << 5,
        //= 1 << 6,
        //= 1 << 7,
        //= 1 << 8,
        //= 1 << 9,
        //= 1 << 10,
        //= 1 << 11,
        //= 1 << 12,
        //= 1 << 13,
        //= 1 << 14,
        //= 1 << 15,
    };};
    class MeshLoader final {
        friend class  Mesh;
        friend struct MeshRequest;
        friend class  Engine::priv::MeshSkeleton;
        public:
            static void        LoadProcessNodeData(MeshRequest& meshRequest, const aiScene& scene, const aiNode& node, uint& count);

            static void        LoadPopulateGlobalNodes(const aiScene& scene, Engine::priv::MeshInfoNode* root, Engine::priv::MeshInfoNode* parent, Engine::priv::MeshInfoNode* node, aiNode* ai_node, MeshRequest& meshRequest);
            static void        FinalizeData(Mesh& mesh, MeshImportedData& data, float threshold);

            static VertexData* LoadFrom_OBJCC(std::string& filename);
            static void        SaveTo_OBJCC(VertexData& data, std::string filename);

            static bool        IsNear(float v1, float v2, float threshold);
            static bool        IsNear(glm::vec2& v1, glm::vec2& v2, float threshold);
            static bool        IsNear(glm::vec3& v1, glm::vec3& v2, float threshold);
            static bool        IsSpecialFloat(float number);
            static bool        IsSpecialFloat(const glm::vec2& vec);
            static bool        IsSpecialFloat(const glm::vec3& vec);
            static bool        GetSimilarVertexIndex(glm::vec3& in_pos, glm::vec2& in_uv, glm::vec3& in_norm, std::vector<glm::vec3>& pts, std::vector<glm::vec2>& uvs, std::vector<glm::vec3>& norms, unsigned int& result, float threshold);
            static void        CalculateTBNAssimp(MeshImportedData& data);
    };
};


#endif