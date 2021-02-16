#pragma once
#ifndef ENGINE_MESH_ANIMATION_INCLUDES_H
#define ENGINE_MESH_ANIMATION_INCLUDES_H

#include <assimp/scene.h>
#include <serenity/dependencies/glm.h>
#include <serenity/utils/Utils.h>
#include <serenity/math/Engine_Math.h>
#include <vector>
#include <string>
#include <unordered_map>
#include <memory>

namespace Engine::priv {
    struct BoneInfo final {
        glm::mat4 BoneOffset     = glm::mat4{ 0.0f };
        glm::mat4 FinalTransform = glm::mat4{ 1.0f };
    };
    struct MeshInfoNode final {
        glm::mat4     Transform = glm::mat4{ 1.0f };
        std::string   Name;

        MeshInfoNode() = delete;
        MeshInfoNode(std::string&& name_, glm::mat4&& transform_) 
            : Name      { std::move(name_) }
            , Transform { std::move(transform_) }
        {}
        MeshInfoNode(const aiNode& ainode) 
            : Name      { ainode.mName.C_Str() }
            , Transform { Engine::Math::assimpToGLMMat4(ainode.mTransformation) }
        {}
    };
    struct Vector3Key final {
        glm::vec3 value = glm::vec3{ 0.0f };
        float     time  = 0.0f;
        Vector3Key() = default;
        Vector3Key(float time_, const glm::vec3& value_) 
            : time{ time_ }
            , value{ value_ }
        {}
    };
    struct QuatKey final {
        aiQuaternion  value = aiQuaternion{ 1.0f, 0.0f, 0.0f, 0.0f };
        float         time  = 0.0f;
        QuatKey() = default;
        QuatKey(float time_, const aiQuaternion& value_) 
            : time{ time_ }
            , value{ value_ }
        {}
    };
    struct AnimationChannel final {
        std::vector<QuatKey>     RotationKeys;
        std::vector<Vector3Key>  PositionKeys;
        std::vector<Vector3Key>  ScalingKeys;

        AnimationChannel() = default;
        AnimationChannel(const aiNodeAnim& aiAnimNode);

        inline bool empty() const noexcept { return RotationKeys.empty() && PositionKeys.empty() && ScalingKeys.empty(); }
    };
};

#endif