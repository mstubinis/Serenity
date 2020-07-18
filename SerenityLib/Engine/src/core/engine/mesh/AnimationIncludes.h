#pragma once
#ifndef ENGINE_MESH_ANIMATION_INCLUDES_H
#define ENGINE_MESH_ANIMATION_INCLUDES_H

#include <assimp/scene.h>

namespace Engine::priv {
    struct BoneInfo final {
        glm::mat4   BoneOffset     = glm::mat4(0.0f);
        glm::mat4   FinalTransform = glm::mat4(1.0f);
    };
    struct MeshInfoNode final {
        std::string                 Name      = "";
        MeshInfoNode*               Parent    = nullptr;
        glm::mat4                   Transform = glm::mat4(1.0f);
        std::vector<MeshInfoNode*>  Children;

        MeshInfoNode() = delete;
        MeshInfoNode(std::string&& name_, glm::mat4&& transform_) {
            Name      = name_;
            Transform = transform_;
        }
        ~MeshInfoNode() {
        }
    };
    struct AnimationKeyBaseClass {
        float     time;
        AnimationKeyBaseClass() = default;
        AnimationKeyBaseClass(float time_) {
            time = time_;
        }
    };
    struct Vector3Key final : public AnimationKeyBaseClass {
        glm::vec3  value;
        Vector3Key() = default;
        Vector3Key(float time_, const glm::vec3& value_) : AnimationKeyBaseClass(time_){
            value = value_;
        }
    };
    struct QuatKey final : public AnimationKeyBaseClass {
        aiQuaternion  value;
        QuatKey() = default;
        QuatKey(float time_, const aiQuaternion& value_) : AnimationKeyBaseClass(time_){
            value = value_;
        }
    };
    struct AnimationChannel final {
        std::vector<Vector3Key>  PositionKeys;
        std::vector<QuatKey>     RotationKeys;
        std::vector<Vector3Key>  ScalingKeys;
    };
};

using MeshNodeMap = std::unordered_map<std::string, Engine::priv::MeshInfoNode*>;

#endif