
#include <serenity/resources/mesh/animation/AnimationData.h>
#include <serenity/math/Engine_Math.h>
#include <serenity/resources/mesh/Mesh.h>
#include <serenity/resources/mesh/MeshRequest.h>
#include <serenity/resources/mesh/animation/Skeleton.h>
#include <serenity/math/MathCompression.h>

#include <glm/gtx/matrix_interpolation.hpp>

namespace {
    //O(1) with high probability
    [[nodiscard]] uint16_t internal_find_key_idx(float AnimTime, const auto& KeyFrms, uint16_t& CurrentKeyFrame) noexcept {
        for (uint16_t i = CurrentKeyFrame; i < KeyFrms.size() - 1; ++i) {
            if (AnimTime >= KeyFrms[i].time && AnimTime < KeyFrms[i + 1].time) {
                CurrentKeyFrame = i;
                return i;
            }
        }
        CurrentKeyFrame = 0;
        return 0;
    }
    template<class FUNC>
    auto internal_interpolate(const float AnimTime, const auto& keys, uint16_t& CurrentKeyFrame, FUNC&& func) {
        if (keys.size() == 1) {
            return keys[0].value;
        }
        const size_t Index              = internal_find_key_idx(AnimTime, keys, CurrentKeyFrame);
        const float Timeframe           = keys[Index + 1].time - keys[Index].time;
        const float percentThroughFrame = AnimTime - keys[Index].time / Timeframe;
        const auto& x                   = keys[Index].value;
        const auto& y                   = keys[Index + 1].value;
        //return x + Factor * (y - x);
        return func(x, y, percentThroughFrame);
    }


    template<class FUNC>
    auto internal_interpolate(const float AnimTime, FUNC&& func, const auto& value0, const auto& value1, float duration) {
        const float percentThroughFrame = AnimTime / duration;
        return func(value0, value1, percentThroughFrame);
    }


    [[nodiscard]] inline glm::vec3 i_mix(const glm::vec3& a, const glm::vec3& b, float factor) noexcept {
        return glm::mix(a, b, factor);
    }
    [[nodiscard]] inline glm::quat i_slerp(const glm::quat& a, const glm::quat& b, float factor) noexcept {
        return glm::slerp(a, b, factor);
    }

    glm::vec3 CalcInterpolatedPosition(float AnimTime, const std::vector<Engine::priv::Vector3Key>& positions, uint16_t& CurrentKeyFrame) {
        return internal_interpolate<glm::vec3(&)(const glm::vec3&, const glm::vec3&, float)>(AnimTime, positions, CurrentKeyFrame, glm::mix);
    }
    glm::quat CalcInterpolatedRotation(float AnimTime, const std::vector<Engine::priv::QuatKey>& rotations, uint16_t& CurrentKeyFrame) {
        return glm::normalize(internal_interpolate(AnimTime, rotations, CurrentKeyFrame, glm::slerp<float, glm::packed_highp>));
    }
    glm::vec3 CalcInterpolatedScaling(float AnimTime, const std::vector<Engine::priv::Vector3Key>& scales, uint16_t& CurrentKeyFrame) {
        return internal_interpolate<glm::vec3(&)(const glm::vec3&, const glm::vec3&, float)>(AnimTime, scales, CurrentKeyFrame, glm::mix);
    }
}


namespace Engine::priv {
    AnimationData::AnimationData(float ticksPerSecond, float durationInTicks)
        : m_TicksPerSecond{ ticksPerSecond != 0.0f ? ticksPerSecond : 25.0f }
        , m_DurationInTicks{ durationInTicks }
    {
    }
    AnimationData::AnimationData(const aiAnimation& assimpAnim, MeshRequestPart& meshRequestPart)
        : AnimationData{ float(assimpAnim.mTicksPerSecond), float(assimpAnim.mDuration) }
    {
        Engine::unordered_string_map<std::string, AnimationChannel> hashedChannels;
        for (uint32_t channelIdx = 0; channelIdx != assimpAnim.mNumChannels; ++channelIdx) {
            const aiNodeAnim& aiAnimChannel = *assimpAnim.mChannels[channelIdx];
            hashedChannels.emplace(std::piecewise_construct, std::forward_as_tuple(aiAnimChannel.mNodeName.C_Str()), std::forward_as_tuple(aiAnimChannel));
        }

        //sort into member data structure
        m_Channels.resize(meshRequestPart.nodesData.m_Nodes.size());
        for (uint32_t nodeIdx = 0; nodeIdx != m_Channels.size(); ++nodeIdx) {
            auto& nodeName = meshRequestPart.nodesNames[nodeIdx];
            auto itr       = hashedChannels.find(nodeName);
            if (itr != hashedChannels.end()) {
                m_Channels[nodeIdx] = std::move(itr->second);
            }
        }
    }
    void AnimationData::ComputeTransformsSnapshot(
        float TimeInSeconds, 
        const std::vector<AnimationChannel>& snapshots, 
        std::vector<glm::mat4>& boneVertexTransforms,
        const MeshNodeData& nodeData, 
#ifndef ENGINE_ANIMATIONS_NO_GLOBAL_INVERSE_TRANSFORM
        const glm::mat4& globalInverseTransform, 
#endif
        std::vector<glm::mat4>& bonesOffsets, 
        std::vector<glm::mat4>& nodeLocalTransforms, 
        std::vector<NodeSnapshot>& nodeSnapshots, 
        float blendingDuration
    ) {
        uint16_t BoneIndex = 0;
        for (uint32_t nodeIdx = 1; nodeIdx != nodeData.m_Nodes.size(); ++nodeIdx) {
            const auto parentIdx    = nodeData.m_NodeHeirarchy[nodeIdx];
            const auto& currNode    = nodeData.m_Nodes[nodeIdx];
            const auto& channel     = m_Channels[nodeIdx];
            const auto& snapshot    = snapshots[nodeIdx];
            glm::mat4 NodeTransform = currNode.Transform;
            if (!channel.empty()) {
                nodeSnapshots[nodeIdx].position = internal_interpolate(
                    TimeInSeconds, i_mix, snapshot.PositionKeys[0].value, channel.PositionKeys[0].value, blendingDuration
                );
                glm::quat rot                   = internal_interpolate(
                    TimeInSeconds, i_slerp, snapshot.RotationKeys[0].value, channel.RotationKeys[0].value, blendingDuration
                );
                nodeSnapshots[nodeIdx].rotation = rot;
                nodeSnapshots[nodeIdx].scale    = internal_interpolate(
                    TimeInSeconds, i_mix, snapshot.ScalingKeys[0].value, channel.ScalingKeys[0].value, blendingDuration
                );
                NodeTransform = glm::translate(nodeSnapshots[nodeIdx].position)
                              * glm::mat4_cast(glm::normalize(rot))
                              * glm::scale(nodeSnapshots[nodeIdx].scale);
            }
            nodeLocalTransforms[nodeIdx] = nodeLocalTransforms[parentIdx - 1] * NodeTransform;
            if (currNode.IsBone) {
                boneVertexTransforms[BoneIndex] =
                    boneVertexTransforms[BoneIndex] *
#ifndef ENGINE_ANIMATIONS_NO_GLOBAL_INVERSE_TRANSFORM
                    globalInverseTransform *
#endif
                    nodeLocalTransforms[nodeIdx] *
                    bonesOffsets[BoneIndex]
                ;
                ++BoneIndex;
            }
        }
    }
    void AnimationData::ComputeTransforms(
        float TimeInSeconds, 
        std::vector<glm::mat4>& boneVertexTransforms, 
        const MeshNodeData& nodeData, 
#ifndef ENGINE_ANIMATIONS_NO_GLOBAL_INVERSE_TRANSFORM
        const glm::mat4& globalInverseTransform, 
#endif
        std::vector<glm::mat4>& bonesOffsets, 
        std::vector<glm::mat4>& nodeLocalTransforms, 
        std::vector<NodeSnapshot>& nodeSnapshots, 
        std::vector<std::array<uint16_t, 3>>& currentKeyframes
    ) {
        const float TimeInTicks   = TimeInSeconds * m_TicksPerSecond;
        const float AnimationTime = std::fmod(TimeInTicks, m_DurationInTicks);
        uint16_t BoneIndex        = 0;
        for (uint32_t nodeIdx = 1; nodeIdx != nodeData.m_Nodes.size(); ++nodeIdx) {
            const auto parentIdx    = nodeData.m_NodeHeirarchy[nodeIdx];
            const auto& currNode    = nodeData.m_Nodes[nodeIdx];
            const auto& channel     = m_Channels[nodeIdx];
            glm::mat4 NodeTransform = currNode.Transform;
            if (!channel.empty()) {
                nodeSnapshots[nodeIdx].position    = CalcInterpolatedPosition(AnimationTime, channel.PositionKeys, currentKeyframes[nodeIdx][0]);
                glm::quat rot                      = CalcInterpolatedRotation(AnimationTime, channel.RotationKeys, currentKeyframes[nodeIdx][1]);
                nodeSnapshots[nodeIdx].rotation    = rot;
                nodeSnapshots[nodeIdx].scale       = CalcInterpolatedScaling(AnimationTime, channel.ScalingKeys, currentKeyframes[nodeIdx][2]);
                
                NodeTransform = glm::translate(nodeSnapshots[nodeIdx].position) 
                              * glm::mat4_cast(rot)
                              * glm::scale(nodeSnapshots[nodeIdx].scale);
            }
            nodeLocalTransforms[nodeIdx] = nodeLocalTransforms[parentIdx - 1] * NodeTransform;    
            if (currNode.IsBone) {
                boneVertexTransforms[BoneIndex] = 
                    boneVertexTransforms[BoneIndex] *
#ifndef ENGINE_ANIMATIONS_NO_GLOBAL_INVERSE_TRANSFORM
                    globalInverseTransform *
#endif
                    nodeLocalTransforms[nodeIdx] *
                    bonesOffsets[BoneIndex]
                ;
                ++BoneIndex;
            }
        }
    }
}