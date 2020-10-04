#pragma once
#ifndef ENGINE_MESH_INCLUDES_H
#define ENGINE_MESH_INCLUDES_H

#include <core/engine/utils/Utils.h>
#include <GL/glew.h>
#include <SFML/OpenGL.hpp>
#include <glm/glm.hpp>

constexpr unsigned int NUM_BONES_PER_VERTEX   = 4U;
constexpr unsigned int NUM_MAX_INSTANCES      = 65536U;

struct MeshModifyFlags {enum Flag {
    Default              = 1 << 0,
    Orphan               = 1 << 1,
    UploadToGPU          = 1 << 2,
    RecalculateTriangles = 1 << 3,
};};

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

        Vertex() = default;
        Vertex(const Vertex& other)                = delete;
        Vertex& operator=(const Vertex& other)     = delete;
        Vertex(Vertex&& other) noexcept            = default;
        Vertex& operator=(Vertex&& other) noexcept = default;
        ~Vertex() = default;
    };
    struct Triangle final {
        glm::vec3 position1   = glm::vec3(0.0f);
        glm::vec3 position2   = glm::vec3(0.0f);
        glm::vec3 position3   = glm::vec3(0.0f);
        glm::vec3 midpoint    = glm::vec3(0.0f);
        unsigned int index1   = 0;
        unsigned int index2   = 0;
        unsigned int index3   = 0;

        Triangle() = default;
        Triangle(const Triangle& other)                = delete;
        Triangle& operator=(const Triangle& other)     = delete;
        Triangle(Triangle&& other) noexcept            = default;
        Triangle& operator=(Triangle&& other) noexcept = default;
        ~Triangle() = default;

        glm::vec3 getRandomPoint() const noexcept {
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
        std::array<float, NUM_BONES_PER_VERTEX>   IDs;
        std::array<float, NUM_BONES_PER_VERTEX>   Weights;

        VertexBoneData() {
            IDs.fill(0.0f);
            Weights.fill(0.0f);
        }
        VertexBoneData(unsigned int BoneID, float Weight) : VertexBoneData(){
            AddBoneData(BoneID, Weight);
        }
        VertexBoneData(const VertexBoneData& other)                = delete;
        VertexBoneData& operator=(const VertexBoneData& other)     = delete;
        VertexBoneData(VertexBoneData&& other) noexcept            = default;
        VertexBoneData& operator=(VertexBoneData&& other) noexcept = default;
        ~VertexBoneData() = default;

        void AddBoneData(unsigned int BoneID, float Weight) {
            for (unsigned int i = 0; i < IDs.size(); ++i) {
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