#pragma once
#ifndef ENGINE_MESH_ANIMATION_DATA_H
#define ENGINE_MESH_ANIMATION_DATA_H

class  Mesh;
struct MeshCPUData;
struct MeshNodeData;
class  SMSH_File;
struct MeshRequest;
struct MeshRequestPart;
namespace Engine::priv {
    class  MeshSkeleton;
    class  ModelInstanceAnimation;
    class  ModelInstanceAnimationContainer;
};
#include <serenity/resources/mesh/animation/AnimationIncludes.h>
#include <serenity/utils/Utils.h>
#include <unordered_map>
#include <string>
#include <functional>
#include <array>

namespace Engine::priv {
    class AnimationData final {
        friend class  Engine::priv::MeshSkeleton;
        friend class  Engine::priv::ModelInstanceAnimation;
        friend class  Engine::priv::ModelInstanceAnimationContainer;
        friend class  ::Mesh;
        friend struct ::MeshCPUData;
        friend class  ::SMSH_File;
        private:
            std::vector<AnimationChannel>  m_Channels; //channels per node. Often times alot of these channels will be empty as not all nodes contain animation data
            float                          m_TicksPerSecond  = 0.0f;
            float                          m_DurationInTicks = 0.0f;

            void ComputeTransformsSnapshot(
                float time,
                const std::vector<AnimationChannel>& snapshot,
                std::vector<glm::mat4>& Transforms,
                const MeshNodeData&,
#ifndef ENGINE_ANIMATIONS_NO_GLOBAL_INVERSE_TRANSFORM
                const glm::mat4& globalInverseTransform,
#endif
                std::vector<glm::mat4>& boneOffsets,
                std::vector<glm::mat4>& tempTransforms,
                std::vector<NodeSnapshot>& nodeTransforms,
                float blendingDuration
            );
            void ComputeTransforms(
                float time, 
                std::vector<glm::mat4>& Transforms, 
                const MeshNodeData&, 
#ifndef ENGINE_ANIMATIONS_NO_GLOBAL_INVERSE_TRANSFORM
                const glm::mat4& globalInverseTransform, 
#endif
                std::vector<glm::mat4>& boneOffsets, 
                std::vector<glm::mat4>& tempTransforms, 
                std::vector<NodeSnapshot>& nodeTransforms,
                std::vector<std::array<uint16_t, 3>>& currentKeyframes
            );

            AnimationData() = delete;
            AnimationData(float ticksPerSecond, float durationInTicks);
        public:
            AnimationData(const aiAnimation&, MeshRequestPart&);

            [[nodiscard]] inline constexpr float duration() const noexcept { return m_DurationInTicks / m_TicksPerSecond; }
            [[nodiscard]] inline constexpr float durationInTicks() const noexcept { return m_DurationInTicks; }
            [[nodiscard]] inline constexpr float ticksPerSecond() const noexcept { return m_TicksPerSecond; }
        };
};

#endif