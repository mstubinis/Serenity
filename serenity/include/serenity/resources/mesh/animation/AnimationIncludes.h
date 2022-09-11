#pragma once
#ifndef ENGINE_MESH_ANIMATION_INCLUDES_H
#define ENGINE_MESH_ANIMATION_INCLUDES_H

struct aiNodeAnim;
struct aiNode;

#include <assimp/scene.h>

#include <serenity/dependencies/glm.h>
#include <serenity/utils/Utils.h>
#include <serenity/math/Engine_Math.h>

namespace Engine::priv {
    struct NodeSnapshot final {
        glm::vec3 position;
        Engine::quat32 rotation;
        glm::vec3 scale;
    };
    struct MeshSkeletonNode final {
        glm::mat4 Transform = glm::mat4{ 1.0f };
        bool      IsBone    = false;

        MeshSkeletonNode() = delete;
        MeshSkeletonNode(glm::mat4&& transform_)
            : Transform{ std::move(transform_) }
        {}
        MeshSkeletonNode(const aiNode& ainode)
            : Transform { Engine::Math::toGLM(ainode.mTransformation) }
        {}
    };
    struct Vector3Key final {
        glm::vec3 value = glm::vec3{ 0.0f };
        float     time  = 0.0f;
        Vector3Key() = default;
        Vector3Key(float time_, const glm::vec3& value_) 
            : value{ value_ }
            , time{ time_ }
        {}
    };
    struct QuatKey final {
        glm::quat  value = glm::quat{ 1.0f, 0.0f, 0.0f, 0.0f };
        float      time  = 0.0f;
        QuatKey() = default;
        QuatKey(float time_, const glm::quat& value_)
            : value{ value_ }
            , time{ time_ }
        {}
    };
    struct AnimationChannel final {
        std::vector<QuatKey>     RotationKeys;
        std::vector<Vector3Key>  PositionKeys;
        std::vector<Vector3Key>  ScalingKeys;

        AnimationChannel() = default;
        AnimationChannel(const aiNodeAnim&);
        AnimationChannel(std::vector<Vector3Key>&& positionKeys, std::vector<QuatKey>&& rotationKeys, std::vector<Vector3Key>&& scalingKeys);

        inline bool empty() const noexcept { return RotationKeys.empty() && PositionKeys.empty() && ScalingKeys.empty(); }
    };
};

#endif