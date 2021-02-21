#pragma once
#ifndef ENGINE_MESH_ANIMATION_DATA_H
#define ENGINE_MESH_ANIMATION_DATA_H

class  Mesh;
struct MeshCPUData;
struct MeshNodeData;
class  SMSH_File;
struct MeshRequest;
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
            float                          m_TicksPerSecond  = 0.0f;
            float                          m_DurationInTicks = 0.0f;

            //O(1) with high probability
            template<typename T> [[nodiscard]] uint16_t internal_find_keyframe_idx(float AnimTime, const T& KeyFrms, uint16_t& CurrentKeyFrame) const noexcept {
                for (uint16_t i = CurrentKeyFrame; i < KeyFrms.size() - 1; ++i) {
                    if (AnimTime >= KeyFrms[i].time && AnimTime < KeyFrms[i + 1].time) {
                        CurrentKeyFrame = i;
                        return i;
                    }
                }
                CurrentKeyFrame = 0;
                return 0;
            }
            [[nodiscard]] glm::vec3 internal_interpolate_vec3(float AnimTime, const std::vector<Vector3Key>& keys, std::function<size_t()>&& FindKeyFrmIdx);

            void                      ComputeTransforms(float time, std::vector<glm::mat4>& Transforms, MeshSkeleton&, MeshNodeData&);
            [[nodiscard]] glm::vec3   CalcInterpolatedPosition(float AnimTime, const std::vector<Vector3Key>&, uint16_t& CurrentKeyFrame);
            [[nodiscard]] glm::quat   CalcInterpolatedRotation(float AnimTime, const std::vector<QuatKey>&, uint16_t& CurrentKeyFrame);
            [[nodiscard]] glm::vec3   CalcInterpolatedScaling(float AnimTime, const std::vector<Vector3Key>&, uint16_t& CurrentKeyFrame);
            [[nodiscard]] inline int16_t FindPositionIdx(float AnimTime, const std::vector<Vector3Key>& positions, uint16_t& CurrentKeyFrame) const noexcept {
                return internal_find_keyframe_idx(AnimTime, positions, CurrentKeyFrame);
            }
            [[nodiscard]] inline int16_t FindRotationIdx(float AnimTime, const std::vector<QuatKey>& rotations, uint16_t& CurrentKeyFrame) const noexcept {
                return internal_find_keyframe_idx(AnimTime, rotations, CurrentKeyFrame);
            }
            [[nodiscard]] inline int16_t FindScalingIdx(float AnimTime, const std::vector<Vector3Key>& scales, uint16_t& CurrentKeyFrame) const noexcept {
                return internal_find_keyframe_idx(AnimTime, scales, CurrentKeyFrame);
            }

            AnimationData() = delete;
            AnimationData( float ticksPerSecond, float durationInTicks);
        public:
            AnimationData( const aiAnimation&, MeshRequest&);

            [[nodiscard]] inline constexpr float duration() const noexcept { return m_DurationInTicks / m_TicksPerSecond; }
            [[nodiscard]] inline constexpr float durationInTicks() const noexcept { return m_DurationInTicks; }
            [[nodiscard]] inline constexpr float ticksPerSecond() const noexcept { return m_TicksPerSecond; }
        };
};

#endif