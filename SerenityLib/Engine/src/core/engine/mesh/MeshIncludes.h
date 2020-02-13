#pragma once
#ifndef ENGINE_MESH_INCLUDES_INCLUDE_GUARD
#define ENGINE_MESH_INCLUDES_INCLUDE_GUARD

#include <core/engine/utils/Utils.h>
#include <GL/glew.h>
#include <SFML/OpenGL.hpp>
#include <glm/glm.hpp>

const unsigned int NUM_BONES_PER_VERTEX   = 4U;
const unsigned int NUM_MAX_INSTANCES      = 65536U;

struct MeshModifyFlags {enum Flag {
    Default              = 1 << 0,
    Orphan               = 1 << 1,
    UploadToGPU          = 1 << 2,
    RecalculateTriangles = 1 << 3,
};};

namespace Engine::priv {
    struct Vertex final {
        glm::vec3 position = glm::vec3(0.0f);
        glm::vec2 uv       = glm::vec2(0.0f);
        glm::vec3 normal   = glm::vec3(0.0f);
        glm::vec3 binormal = glm::vec3(0.0f);
        glm::vec3 tangent  = glm::vec3(0.0f);

        Vertex() {}
        ~Vertex() {}
        Vertex(const Vertex&)                      = delete;
        Vertex& operator=(const Vertex&)           = delete;
        Vertex(Vertex&& other) noexcept            = default;
        Vertex& operator=(Vertex&& other) noexcept = default;
    };
    struct Triangle final {
        glm::vec3 position1   = glm::vec3(0.0f);
        glm::vec3 position2   = glm::vec3(0.0f);
        glm::vec3 position3   = glm::vec3(0.0f);
        glm::vec3 midpoint    = glm::vec3(0.0f);
        unsigned short index1 = 0;
        unsigned short index2 = 0;
        unsigned short index3 = 0;

        Triangle() {}
        ~Triangle() {}
        Triangle(const Triangle&)                      = delete;
        Triangle& operator=(const Triangle&)           = delete;
        Triangle(Triangle&& other) noexcept            = default;
        Triangle& operator=(Triangle&& other) noexcept = default;

    };
    struct VertexBoneData final {
        float     IDs[NUM_BONES_PER_VERTEX];
        float Weights[NUM_BONES_PER_VERTEX];

        VertexBoneData() {
            for (unsigned int i = 0; i < NUM_BONES_PER_VERTEX; ++i) {
                IDs[i]     = 0.0f;
                Weights[i] = 0.0f;
            }
        }
        ~VertexBoneData(){}
        VertexBoneData(const VertexBoneData&)                      = delete;
        VertexBoneData& operator=(const VertexBoneData&)           = delete;
        VertexBoneData(VertexBoneData&& other) noexcept            = default;
        VertexBoneData& operator=(VertexBoneData&& other) noexcept = default;

        void AddBoneData(uint BoneID, float Weight) {
            for (unsigned int i = 0; i < NUM_BONES_PER_VERTEX; ++i) {
                if (Weights[i] == 0.0f) {
                    IDs[i] = float(BoneID);
                    Weights[i] = Weight;
                    return;
                }
            }
        }
    };
};


#endif