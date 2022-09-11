#pragma once
#ifndef ENGINE_MESH_IMPORTED_DATA_H
#define ENGINE_MESH_IMPORTED_DATA_H

#include <serenity/resources/mesh/MeshIncludes.h>
#include <serenity/resources/mesh/MeshLoading.h>
#include <serenity/dependencies/glm.h>
#include <serenity/system/TypeDefs.h>
#include <map>
#include <vector>

namespace Engine::priv {
    class MeshImportedData final {
        private:
            void internal_reserve(uint32_t capacity) {
                m_Points.reserve(capacity);
                m_UVs.reserve(capacity);
                m_Normals.reserve(capacity);
                m_Binormals.reserve(capacity);
                m_Tangents.reserve(capacity);
            }
            void internal_build_vertices(const aiMesh& aimesh) {
                for (unsigned int j = 0; j < aimesh.mNumVertices; ++j) {
                    //pos
                    auto& pos = aimesh.mVertices[j];
                    m_Points.emplace_back(pos.x, pos.y, pos.z);
                    //uv
                    if (aimesh.mTextureCoords[0]) {
                        auto& uv = aimesh.mTextureCoords[0][j];
                        //this is to fix uv compression errors near the poles.
                        //if(uv.y <= 0.0001f){ uv.y = 0.001f; }
                        //if(uv.y >= 0.9999f){ uv.y = 0.999f; }
                        m_UVs.emplace_back(uv.x, uv.y);
                    } else {
                        m_UVs.emplace_back(0.0f, 0.0f);
                    }
                    if (aimesh.mNormals) {
                        auto& norm = aimesh.mNormals[j];
                        m_Normals.emplace_back(norm.x, norm.y, norm.z);
                    }
                    if (aimesh.mBitangents) {
                        //auto& binorm = aimesh.mBitangents[j];
                        //m_Binormals.emplace_back(binorm.x,binorm.y,binorm.z);
                    }
                    if (aimesh.mTangents) {
                        //auto& tang = aimesh.mTangents[j];
                        //m_Tangents.emplace_back(tang.x,tang.y,tang.z);
                    }
                }
            }
            void internal_build_indices(const aiMesh& aimesh) {
                m_Indices.reserve(aimesh.mNumFaces * 3);
                for (uint32_t j = 0; j < aimesh.mNumFaces; ++j) {
                    const auto& face = aimesh.mFaces[j];
                    m_Indices.emplace_back(face.mIndices[0]);
                    m_Indices.emplace_back(face.mIndices[1]);
                    m_Indices.emplace_back(face.mIndices[2]);
                }
            }
        public:
            std::vector<VertexBoneData> m_Bones;
            std::vector<glm::vec3>      m_FilePoints;
            std::vector<glm::vec2>      m_FileUVs;
            std::vector<glm::vec3>      m_FileNormals;
            std::vector<glm::vec3>      m_Points;
            std::vector<glm::vec2>      m_UVs;
            std::vector<glm::vec3>      m_Normals;
            std::vector<glm::vec3>      m_Binormals;
            std::vector<glm::vec3>      m_Tangents;
            std::vector<uint32_t>       m_Indices;

            MeshImportedData() = default;
            MeshImportedData(const MeshImportedData&)                = delete;
            MeshImportedData& operator=(const MeshImportedData&)     = delete;
            MeshImportedData(MeshImportedData&&) noexcept            = default;
            MeshImportedData& operator=(MeshImportedData&&) noexcept = default;

            MeshImportedData(uint32_t capacity) {
                internal_reserve(capacity);
            }
            MeshImportedData(const aiMesh& aimesh) {
                internal_reserve(aimesh.mNumVertices);
                internal_build_vertices(aimesh);
                internal_build_indices(aimesh);
            }
            bool addBone(uint32_t vertexID, uint32_t boneIndex, float boneWeight) {
                return m_Bones[vertexID].AddBoneData(boneIndex, boneWeight);
            }
            void triangulateIndices(const std::vector<std::vector<uint32_t>>& indices, uint8_t flags) {
                const size_t n = indices[0].size();
                if ((flags & MeshLoadingFlags::Points) && !m_FilePoints.empty()) {
                    for (size_t i = 0; i != n; ++i) {
                        m_Points.emplace_back(m_FilePoints[indices[0][i] - 1]);
                    }
                }
                if ((flags & MeshLoadingFlags::UVs) && !m_FileUVs.empty()) {
                    for (size_t i = 0; i != n; ++i) {
                        m_UVs.emplace_back(m_FileUVs[indices[1][i] - 1]);
                    }
                }
                if ((flags & MeshLoadingFlags::Normals) && !m_FileNormals.empty()) {
                    for (size_t i = 0; i != n; ++i) {
                        m_Normals.emplace_back(m_FileNormals[indices[2][i] - 1]);
                    }
                }
            }
    };
};
#endif