#pragma once
#ifndef ENGINE_MESH_ANIMATION_DATA_H
#define ENGINE_MESH_ANIMATION_DATA_H

class  Mesh;
struct MeshCPUData;
class  SMSH_File;
namespace Engine::priv {
    class  MeshSkeleton;
    class  ModelInstanceAnimation;
};

#include <core/engine/mesh/AnimationIncludes.h>

namespace Engine::priv {
    class AnimationData final {
        friend class  Engine::priv::MeshSkeleton;
        friend class  Engine::priv::ModelInstanceAnimation;
        friend class  Mesh;
        friend class  SMSH_File;
        private:
            std::unordered_map<std::string, AnimationChannel>  m_KeyframeData;
            MeshCPUData*                                       m_MeshCPUData     = nullptr;
            float                                              m_TicksPerSecond  = 0.0f;
            float                                              m_DurationInTicks = 0.0f;

            glm::vec3 internal_interpolate_vec3(float AnimationTime, const std::vector<Engine::priv::Vector3Key>& keys, std::function<size_t()>&& call);

            template<typename T> size_t internal_find(float AnimationTime, const AnimationChannel& node, const std::vector<T>& keys) const {
                for (size_t i = 0; i < keys.size() - 1; ++i) {
                    if (AnimationTime < keys[i + 1].time) {
                        return i;
                    }
                }
                return 0;
            }

            void ReadNodeHeirarchy(const std::string& animationName, float time, const MeshInfoNode* node, const glm::mat4& ParentTransform, std::vector<glm::mat4>& Transforms);
            void BoneTransform(const std::string& animationName, float TimeInSeconds, std::vector<glm::mat4>& Transforms);
            glm::vec3 CalcInterpolatedPosition(float AnimationTime, const AnimationChannel& node);
            aiQuaternion CalcInterpolatedRotation(float AnimationTime, const AnimationChannel& node);
            glm::vec3 CalcInterpolatedScaling(float AnimationTime, const AnimationChannel& node);
            size_t FindPosition(float AnimationTime, const AnimationChannel& node) const;
            size_t FindRotation(float AnimationTime, const AnimationChannel& node) const;
            size_t FindScaling(float AnimationTime, const AnimationChannel& node) const;

            AnimationData() = delete;
            AnimationData(MeshCPUData& cpuData, float ticksPerSecond, float durationInTicks);
        public:
            AnimationData(MeshCPUData& cpuData, const aiAnimation& animation);

            CONSTEXPR float duration() const noexcept {
                float TicksPerSecond((m_TicksPerSecond != 0.0f) ? m_TicksPerSecond : 25.0f);
                return m_DurationInTicks / TicksPerSecond;
            }
            inline CONSTEXPR float durationInTicks() const noexcept { return m_DurationInTicks; }
            inline CONSTEXPR float ticksPerSecond() const noexcept { return m_TicksPerSecond; }
        };
};

#endif