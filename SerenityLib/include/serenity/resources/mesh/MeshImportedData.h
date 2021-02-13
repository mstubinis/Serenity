#pragma once
#ifndef ENGINE_MESH_IMPORTED_DATA_H
#define ENGINE_MESH_IMPORTED_DATA_H

#include <serenity/resources/mesh/MeshIncludes.h>
#include <serenity/dependencies/glm.h>
#include <serenity/system/TypeDefs.h>
#include <map>
#include <vector>

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

        void triangulateIndices(const std::vector<std::vector<uint32_t>>& indices, uint8_t flags) {
            for (size_t i = 0; i < indices[0].size(); ++i) {
                if ((flags & MeshLoadingFlags::Points) && file_points.size() > 0) {
                    points.emplace_back(file_points[indices[0][i] - 1]);
                }
                if ((flags & MeshLoadingFlags::UVs) && file_uvs.size() > 0) {
                    uvs.emplace_back(file_uvs[indices[1][i] - 1]);
                }
                if ((flags & MeshLoadingFlags::Normals) && file_normals.size() > 0) {
                    normals.emplace_back(file_normals[indices[2][i] - 1]);
                }
            }
        }
    };
};
#endif