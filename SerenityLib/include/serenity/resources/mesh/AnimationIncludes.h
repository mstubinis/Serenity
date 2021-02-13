#pragma once
#ifndef ENGINE_MESH_ANIMATION_INCLUDES_H
#define ENGINE_MESH_ANIMATION_INCLUDES_H

#include <assimp/scene.h>
#include <serenity/dependencies/glm.h>
#include <vector>
#include <string>
#include <unordered_map>
#include <memory>

namespace Engine::priv {
    struct BoneInfo final {
        glm::mat4   BoneOffset     = glm::mat4(0.0f);
        glm::mat4   FinalTransform = glm::mat4(1.0f);
    };
    struct MeshInfoNode final {
        std::vector<std::unique_ptr<MeshInfoNode>>  Children;
        glm::mat4                                   Transform = glm::mat4(1.0f);
        std::string                                 Name;
        MeshInfoNode*                               Parent    = nullptr;

        MeshInfoNode() = delete;
        MeshInfoNode(std::string&& name_, glm::mat4&& transform_) 
            : Name { std::move(name_) }
            , Transform { std::move(transform_) }
        {}
    };
    struct AnimationKeyBaseClass {
        float     time = 0.0f;

        AnimationKeyBaseClass() = default;
        AnimationKeyBaseClass(float time_) 
            : time{ time_ }
        {}
    };
    struct Vector3Key final : public AnimationKeyBaseClass {
        glm::vec3  value = glm::vec3(0.0f);

        Vector3Key() = default;
        Vector3Key(float time_, const glm::vec3& value_) 
            : AnimationKeyBaseClass{ time_ }
            , value{ value_ }
        {}
    };
    struct QuatKey final : public AnimationKeyBaseClass {
        aiQuaternion  value = aiQuaternion(1.0f, 0.0f, 0.0f, 0.0f);

        QuatKey() = default;
        QuatKey(float time_, const aiQuaternion& value_) 
            : AnimationKeyBaseClass{ time_ }
            , value{ value_ }
        {}
    };
    struct AnimationChannel final {
        std::vector<QuatKey>     RotationKeys;
        std::vector<Vector3Key>  PositionKeys;
        std::vector<Vector3Key>  ScalingKeys;
    };
};

using MeshNodeMap = std::unordered_map<std::string, Engine::priv::MeshInfoNode*>;

#endif