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
    None = 0,
    LoadConvexHull = 1 << 0,
    LoadTriangleMesh = 1 << 1,
};};

constexpr MeshCollisionLoadingFlag::Flag MESH_COLLISION_FACTORY_DEFAULT_LOAD_FLAG 
    = (MeshCollisionLoadingFlag::Flag)(MeshCollisionLoadingFlag::LoadConvexHull | MeshCollisionLoadingFlag::LoadTriangleMesh);



namespace Engine::priv {
    struct VertexSmoothingData final {
        glm::vec3  normal = glm::vec3(0.0f);
        size_t     index  = 0U;
    };
    struct VertexSmoothingGroup final {
        std::vector<VertexSmoothingData>   data;
        glm::vec3                          smoothedNormal = glm::vec3(0.0f);
    };
    struct Vertex final {
        glm::vec3 position = glm::vec3(0.0f);
        glm::vec2 uv       = glm::vec2(0.0f);
        glm::vec3 normal   = glm::vec3(0.0f);
        glm::vec3 binormal = glm::vec3(0.0f);
        glm::vec3 tangent  = glm::vec3(0.0f);
    };
    struct Triangle final {
        glm::vec3 position1 = glm::vec3(0.0f);
        glm::vec3 position2 = glm::vec3(0.0f);
        glm::vec3 position3 = glm::vec3(0.0f);
        glm::vec3 midpoint  = glm::vec3(0.0f);
        uint32_t index1     = 0;
        uint32_t index2     = 0;
        uint32_t index3     = 0;

        [[nodiscard]] float getArea() const noexcept {
            auto crossProduct = glm::cross(position2 - position1, position3 - position1);
            return 0.5f * glm::sqrt(
                (crossProduct.x*crossProduct.x) + 
                (crossProduct.y*crossProduct.y) + 
                (crossProduct.z*crossProduct.z)
            );
        }

        [[nodiscard]] glm::vec3 getRandomPoint() const noexcept {
            float A = (float)rand() / (float)RAND_MAX;
            float B = (float)rand() / (float)RAND_MAX;
            if (A + B >= 1.0f) {
                A = 1.0f - A;
                B = 1.0f - B;
            }
            return position1 + A * (position2 - position1) + B * (position3 - position1);
        }

    };
    struct VertexBoneData final {
        std::array<float, MESH_BONES_NUM_BONES_PER_VERTEX>   IDs;
        std::array<float, MESH_BONES_NUM_BONES_PER_VERTEX>   Weights;

        VertexBoneData() {
            IDs.fill(0.0f);
            Weights.fill(0.0f);
        }
        VertexBoneData(uint32_t BoneID, float Weight)
            : VertexBoneData{}
        {
            AddBoneData(BoneID, Weight);
        }

        void AddBoneData(uint32_t BoneID, float Weight) noexcept {
            for (size_t i = 0; i < IDs.size(); ++i) {
                if (Weights[i] == 0.0f) {
                    IDs[i]     = (float)BoneID;
                    Weights[i] = Weight;
                    return;
                }
            }
        }
    };
};


#endif