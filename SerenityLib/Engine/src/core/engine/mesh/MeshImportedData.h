#pragma once
#ifndef ENGINE_MESH_IMPORTED_DATA_H
#define ENGINE_MESH_IMPORTED_DATA_H

#include <core/engine/mesh/MeshIncludes.h>

namespace Engine::priv {
    struct MeshImportedData final {
        std::map<uint32_t, VertexBoneData>      m_Bones;
        std::vector<glm::vec3>                  file_points;
        std::vector<glm::vec2>                  file_uvs;
        std::vector<glm::vec3>                  file_normals;
        std::vector<glm::vec3>                  points;
        std::vector<glm::vec2>                  uvs;
        std::vector<glm::vec3>                  normals;
        std::vector<glm::vec3>                  binormals;
        std::vector<glm::vec3>                  tangents;
        std::vector<uint32_t>                   indices;

        MeshImportedData() = default;
        MeshImportedData(const MeshImportedData& other)                = delete;
        MeshImportedData& operator=(const MeshImportedData& other)     = delete;
        MeshImportedData(MeshImportedData&& other) noexcept            = default;
        MeshImportedData& operator=(MeshImportedData&& other) noexcept = default;
        ~MeshImportedData() = default;
    };
};
#endif