#pragma once
#ifndef ENGINE_MESH_ANIMATION_DATA_H
#define ENGINE_MESH_ANIMATION_DATA_H

class  Mesh;
struct MeshCPUData;
struct MeshNodeData;
class  SMSH_File;
namespace Engine::priv {
    class  MeshSkeleton;
    class  ModelInstanceAnimation;
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
        friend class  Mesh;
        friend struct MeshCPUData;
        friend class  SMSH_File;
        private:
            std::vector<AnimationChannel>  m_Channels;
            MeshNodeData*                  m_NodeData        = nullptr;
            Engine::priv::MeshSkeleton*    m_Skeleton        = nullptr;
            float                          m_TicksPerSecond  = 0.0f;
            float                          m_DurationInTicks = 0.0f;

            template<typename T> [[nodiscard]] uint16_t internal_find_keyframe_idx(float AnimTime, const T& KeyFrms, uint16_t& LastKeyFrm) const noexcept {
                for (uint16_t i = LastKeyFrm; i < static_cast<uint16_t>(KeyFrms.size() - 1); ++i) {
                    if (AnimTime < KeyFrms[i + 1].time) {
                        LastKeyFrm = i;
                        return i;
                    }
                }
                LastKeyFrm = 0;
                return 0;
            }
            [[nodiscard]] glm::vec3 internal_interpolate_vec3(float AnimTime, const std::vector<Engine::priv::Vector3Key>& keys, std::function<size_t()>&& FindKeyFrmIdx);

            void                          ComputeTransforms(float time, std::vector<glm::mat4>& Transforms, std::array<uint16_t, 3>& keyframeIndices);
            [[nodiscard]] glm::vec3       CalcInterpolatedPosition(float AnimTime, const std::vector<Engine::priv::Vector3Key>&, uint16_t& LastKeyFrm);
            [[nodiscard]] aiQuaternion    CalcInterpolatedRotation(float AnimTime, const std::vector<Engine::priv::QuatKey>&, uint16_t& LastKeyFrm);
            [[nodiscard]] glm::vec3       CalcInterpolatedScaling(float AnimTime, const std::vector<Engine::priv::Vector3Key>&, uint16_t& LastKeyFrm);
            [[nodiscard]] inline uint32_t FindPositionIdx(float AnimTime, const std::vector<Engine::priv::Vector3Key>& positions, uint16_t& LastKeyFrm) const noexcept {
                return internal_find_keyframe_idx(AnimTime, positions, LastKeyFrm);
            }
            [[nodiscard]] inline uint32_t FindRotationIdx(float AnimTime, const std::vector<Engine::priv::QuatKey>& rotations, uint16_t& LastKeyFrm) const noexcept {
                return internal_find_keyframe_idx(AnimTime, rotations, LastKeyFrm);
            }
            [[nodiscard]] inline uint32_t FindScalingIdx(float AnimTime, const std::vector<Engine::priv::Vector3Key>& scales, uint16_t& LastKeyFrm) const noexcept {
                return internal_find_keyframe_idx(AnimTime, scales, LastKeyFrm);
            }

            AnimationData() = delete;
            AnimationData( MeshNodeData& nodeData, Engine::priv::MeshSkeleton&, float ticksPerSecond, float durationInTicks);
        public:
            AnimationData( MeshNodeData& nodeData, Engine::priv::MeshSkeleton&, const aiAnimation&, MeshNodeData& filledNodes);

            [[nodiscard]] inline constexpr float duration() const noexcept { return m_DurationInTicks / m_TicksPerSecond; }
            [[nodiscard]] inline constexpr float durationInTicks() const noexcept { return m_DurationInTicks; }
            [[nodiscard]] inline constexpr float ticksPerSecond() const noexcept { return m_TicksPerSecond; }
        };
};

#endif