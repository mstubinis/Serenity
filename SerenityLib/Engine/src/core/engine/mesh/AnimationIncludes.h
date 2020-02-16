#pragma once
#ifndef ENGINE_MESH_ANIMATION_INCLUDES_H
#define ENGINE_MESH_ANIMATION_INCLUDES_H

#include <string>
#include <vector>
#include <glm/mat4x4.hpp>
#include <assimp/scene.h>

namespace Engine::priv {
    struct BoneInfo final {
        glm::mat4   BoneOffset;
        glm::mat4   FinalTransform;
        BoneInfo() {
            BoneOffset     = glm::mat4(0.0f);
            FinalTransform = glm::mat4(1.0f);
        }
    };
    struct BoneNode final {
        std::string             Name;
        BoneNode*               Parent;
        std::vector<BoneNode*>  Children;
        glm::mat4               Transform;
        BoneNode() {
            Name = "";
            Parent = nullptr;
            Transform = glm::mat4(1.0f);
        }
    };
    struct Vector3Key final {
        glm::vec3  value;
        double     time;
        Vector3Key(const double time_, const glm::vec3& value_) {
            value = value_;
            time = time_;
        }
    };
    struct QuatKey final {
        aiQuaternion  value;
        double        time;
        QuatKey(const double time_, const aiQuaternion& value_) {
            value = value_;
            time = time_;
        }
    };
    struct AnimationChannel final {
        std::vector<Vector3Key>  PositionKeys;
        std::vector<QuatKey>     RotationKeys;
        std::vector<Vector3Key>  ScalingKeys;
    };
};

#endif