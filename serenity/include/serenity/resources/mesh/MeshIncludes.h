#pragma once
#ifndef ENGINE_MESH_INCLUDES_H
#define ENGINE_MESH_INCLUDES_H

#include <serenity/utils/Utils.h>
#include <GL/glew.h>
#include <SFML/OpenGL.hpp>
#include <serenity/dependencies/glm.h>
#include <serenity/system/TypeDefs.h>
#include <array>

constexpr uint32_t MESH_BONES_NUM_BONES_PER_VERTEX = 4U;
constexpr uint32_t MESH_BONES_NUM_MAX_INSTANCES    = 65536U;
constexpr float    MESH_DEFAULT_THRESHOLD          = 0.0005f;

struct MeshModifyFlags { enum Flag {
    None                 = 1 << 0,
    Orphan               = 1 << 1,
    UploadToGPU          = 1 << 2,
    RecalculateTriangles = 1 << 3,
};};
struct MeshCollisionLoadingFlag final { enum Flag : uint8_t {
    None             = 0,
    LoadConvexHull   = 1 << 0,
    LoadTriangleMesh = 1 << 1,
};};

constexpr MeshCollisionLoadingFlag::Flag MESH_COLLISION_FACTORY_DEFAULT_LOAD_FLAG 
    = (MeshCollisionLoadingFlag::Flag)(MeshCollisionLoadingFlag::LoadConvexHull | MeshCollisionLoadingFlag::LoadTriangleMesh);

namespace Engine::priv {
    struct VertexSmoothingData final {
        glm::vec3  normal = glm::vec3{ 0.0f };
        size_t     index  = 0U;
    };
    struct VertexSmoothingGroup final {
        std::vector<VertexSmoothingData>   data;
        glm::vec3                          smoothedNormal = glm::vec3{ 0.0f };
    };
    struct Vertex final {
        glm::vec3 position = glm::vec3{ 0.0f };
        glm::vec2 uv       = glm::vec2{ 0.0f };
        glm::vec3 normal   = glm::vec3{ 0.0f };
        glm::vec3 binormal = glm::vec3{ 0.0f };
        glm::vec3 tangent  = glm::vec3{ 0.0f };
    };
    class Triangle {
        private:
            std::array<glm::vec3, 3> positions = { glm::vec3{ 0.0f }, glm::vec3{ 0.0f }, glm::vec3{ 0.0f } };

            glm::vec3 midpoint = glm::vec3{ 0.0f };

            std::array<uint32_t, 3> indices = { 0, 0, 0 };
        public:

            inline void recalcMidpoint() noexcept { midpoint = (positions[0] / 3.0f) + (positions[1] / 3.0f) + (positions[2] / 3.0f); }

            inline void setPosition(size_t i, const glm::vec3& pos) noexcept { positions[i] = pos; }

            void setPositions(const glm::vec3& pos1, const glm::vec3& pos2, const glm::vec3& pos3) noexcept {
                setPosition(0, pos1);
                setPosition(1, pos2);
                setPosition(2, pos3);
            }
            inline void setIndex(size_t i, uint32_t idx) noexcept { indices[i] = idx; }
            void setIndices(uint32_t idx1, uint32_t idx2, uint32_t idx3) noexcept {
                setIndex(0, idx1);
                setIndex(1, idx2);
                setIndex(2, idx3);
            }


            [[nodiscard]] inline const glm::vec3& getMidpoint() const noexcept { return midpoint; }
            [[nodiscard]] inline const glm::vec3& getPosition(size_t i) const noexcept { return positions[i]; }
            [[nodiscard]] inline uint32_t getIndex(size_t i) const noexcept { return indices[i]; }

            [[nodiscard]] float getArea() const noexcept {
                const auto crossProduct = glm::cross(positions[1] - positions[0], positions[2] - positions[0]);
                return 0.5f * glm::sqrt(
                    (crossProduct.x * crossProduct.x) + 
                    (crossProduct.y * crossProduct.y) + 
                    (crossProduct.z * crossProduct.z)
                );
            }

            [[nodiscard]] glm::vec3 getRandomPoint() const noexcept {
                float A = float(rand()) / float(RAND_MAX);
                float B = float(rand()) / float(RAND_MAX);
                if (A + B >= 1.0f) {
                    A = 1.0f - A;
                    B = 1.0f - B;
                }
                return positions[0] + A * (positions[1] - positions[0]) + B * (positions[2] - positions[0]);
            }

    };
    struct VertexBoneData final {
        std::array<float, MESH_BONES_NUM_BONES_PER_VERTEX> IDs;
        std::array<float, MESH_BONES_NUM_BONES_PER_VERTEX> Weights;

        VertexBoneData() {
            IDs.fill(0.0f);
            Weights.fill(0.0f);
        }
        bool AddBoneData(uint32_t BoneID, float Weight) noexcept {
            for (size_t i = 0; i != IDs.size(); ++i) {
                if (Weights[i] == 0.0f) {
                    IDs[i]     = float(BoneID);
                    Weights[i] = Weight;
                    return true;
                }
            }
            return false;
        }
    };
};

#endif