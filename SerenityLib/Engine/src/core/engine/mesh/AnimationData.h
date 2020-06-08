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
    class AnimationData final : public Engine::NonCopyable, public Engine::NonMoveable {
        friend class Engine::priv::MeshSkeleton;
        friend class Engine::priv::ModelInstanceAnimation;
        friend class Mesh;
        private:
            MeshSkeleton&                                      m_MeshSkeleton;
            float                                              m_TicksPerSecond;
            float                                              m_DurationInTicks;
            std::unordered_map<std::string, AnimationChannel>  m_KeyframeData;

            void internal_interpolate_vec3(glm::vec3& Out, float AnimationTime, const std::vector<Engine::priv::Vector3Key>& keys, std::function<size_t()> call);

            template<typename T> size_t internal_find(float AnimationTime, const AnimationChannel& node, const std::vector<T>& keys) const {
                for (size_t i = 0; i < keys.size() - 1; ++i) {
                    if (AnimationTime < keys[i + 1].time) {
                        return i;
                    }
                }
                return 0;
            }

            void ReadNodeHeirarchy(const std::string& animationName, float time, const BoneNode* node, const glm::mat4& ParentTransform, std::vector<glm::mat4>& Transforms);
            void BoneTransform(const std::string& animationName, float TimeInSeconds, std::vector<glm::mat4>& Transforms);
            void CalcInterpolatedPosition(glm::vec3& Out, float AnimationTime, const AnimationChannel& node);
            void CalcInterpolatedRotation(aiQuaternion& Out, float AnimationTime, const AnimationChannel& node);
            void CalcInterpolatedScaling(glm::vec3& Out, float AnimationTime, const AnimationChannel& node);
            size_t FindPosition(float AnimationTime, const AnimationChannel& node) const;
            size_t FindRotation(float AnimationTime, const AnimationChannel& node) const;
            size_t FindScaling(float AnimationTime, const AnimationChannel& node) const;

            AnimationData() = delete;
        public:
            AnimationData(Engine::priv::MeshSkeleton& skeleton, const aiAnimation& animation);
            ~AnimationData();

            float duration() const;
        };
};

#endif