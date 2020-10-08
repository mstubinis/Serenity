#pragma once
#ifndef ENGINE_MESH_LOADING_H
#define ENGINE_MESH_LOADING_H

struct VertexData;
struct aiScene;
struct aiNode;
struct MeshCPUData;

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
            static void        LoadPopulateGlobalNodes(const aiScene& scene, MeshInfoNode* root, MeshInfoNode* parent, MeshInfoNode* node, aiNode* ai_node, MeshRequest& meshRequest);
            
            //static void        FinalizeData(Handle meshHandle, MeshImportedData& data, float threshold);
            static void        FinalizeData(MeshCPUData& cpuData, MeshImportedData& data, float threshold);

            static VertexData* LoadFrom_OBJCC(const std::string& filename);
            static void        SaveTo_OBJCC(VertexData& data, std::string filename);

            static bool        GetSimilarVertexIndex(glm::vec3& in_pos, glm::vec2& in_uv, glm::vec3& in_norm, std::vector<glm::vec3>& pts, std::vector<glm::vec2>& uvs, std::vector<glm::vec3>& norms, uint32_t& result, float threshold);
            static void        CalculateTBNAssimp(MeshImportedData& data);
    };
};


#endif