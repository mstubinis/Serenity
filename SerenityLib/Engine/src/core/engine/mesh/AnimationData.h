#pragma once
#ifndef ENGINE_MESH_ANIMATION_DATA_H
#define ENGINE_MESH_ANIMATION_DATA_H

class  Mesh;
namespace Engine::priv {
    class  MeshSkeleton;
};

#include <core/engine/mesh/AnimationIncludes.h>
#include <unordered_map>
#include <glm/vec3.hpp>

namespace Engine::priv {
    class AnimationData final {
        friend class Engine::priv::MeshSkeleton;
        friend class Mesh;
        private:
            MeshSkeleton*                                      m_MeshSkeleton;
            double                                             m_TicksPerSecond;
            double                                             m_DurationInTicks;
            std::unordered_map<std::string, AnimationChannel>  m_KeyframeData;

            void ReadNodeHeirarchy(const std::string& animationName, const float time, const BoneNode* node, const glm::mat4& ParentTransform, std::vector<glm::mat4>& Transforms);
            void BoneTransform(const std::string& animationName, const float TimeInSeconds, std::vector<glm::mat4>& Transforms);
            void CalcInterpolatedPosition(glm::vec3& Out, const float AnimationTime, const AnimationChannel& node);
            void CalcInterpolatedRotation(aiQuaternion& Out, const float AnimationTime, const AnimationChannel& node);
            void CalcInterpolatedScaling(glm::vec3& Out, const float AnimationTime, const AnimationChannel& node);
            const size_t FindPosition(const float AnimationTime, const AnimationChannel& node) const;
            const size_t FindRotation(const float AnimationTime, const AnimationChannel& node) const;
            const size_t FindScaling(const float AnimationTime, const AnimationChannel& node) const;
        public:
            AnimationData() = delete;
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