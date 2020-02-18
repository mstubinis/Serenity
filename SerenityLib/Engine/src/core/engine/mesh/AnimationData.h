#pragma once
#ifndef ENGINE_MESH_ANIMATION_DATA_H
#define ENGINE_MESH_ANIMATION_DATA_H

class  Mesh;
namespace Engine::priv {
    class  MeshSkeleton;
    class  ModelInstanceAnimation;
};

#include <core/engine/mesh/AnimationIncludes.h>
#include <unordered_map>
#include <glm/vec3.hpp>
#include <functional>

namespace Engine::priv {
    class AnimationData final {
        friend class Engine::priv::MeshSkeleton;
        friend class Engine::priv::ModelInstanceAnimation;
        friend class Mesh;
        private:
            MeshSkeleton&                                      m_MeshSkeleton;
            double                                             m_TicksPerSecond;
            double                                             m_DurationInTicks;
            std::unordered_map<std::string, AnimationChannel>  m_KeyframeData;

            void internal_interpolate_vec3(glm::vec3& Out, const float AnimationTime, const std::vector<Engine::priv::Vector3Key>& keys, std::function<size_t()> call);

            template<typename T>
            const size_t internal_find(const float AnimationTime, const AnimationChannel& node, const std::vector<T>& keys) const {
                for (size_t i = 0; i < keys.size() - 1; ++i) {
                    if (AnimationTime < static_cast<float>(keys[i + 1].time)) {
                        return i;
                    }
                }
                return 0;
            }


            void ReadNodeHeirarchy(const std::string& animationName, const float time, const BoneNode* node, const glm::mat4& ParentTransform, std::vector<glm::mat4>& Transforms);
            void BoneTransform(const std::string& animationName, const float TimeInSeconds, std::vector<glm::mat4>& Transforms);
            void CalcInterpolatedPosition(glm::vec3& Out, const float AnimationTime, const AnimationChannel& node);
            void CalcInterpolatedRotation(aiQuaternion& Out, const float AnimationTime, const AnimationChannel& node);
            void CalcInterpolatedScaling(glm::vec3& Out, const float AnimationTime, const AnimationChannel& node);
            const size_t FindPosition(const float AnimationTime, const AnimationChannel& node) const;
            const size_t FindRotation(const float AnimationTime, const AnimationChannel& node) const;
            const size_t FindScaling(const float AnimationTime, const AnimationChannel& node) const;

            AnimationData() = delete;
        public:
            AnimationData(const Engine::priv::MeshSkeleton&, const aiAnimation&);

            AnimationData(const AnimationData&) = delete;
            AnimationData& operator=(const AnimationData&) = delete;
            AnimationData(AnimationData&&) noexcept = delete;
            AnimationData& operator=(AnimationData&& other) noexcept = delete;
            ~AnimationData();

            const float duration() const;
        };
};

#endif