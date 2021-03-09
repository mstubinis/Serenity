
#include <serenity/resources/mesh/animation/AnimationData.h>
#include <serenity/math/Engine_Math.h>
#include <serenity/resources/mesh/Mesh.h>
#include <serenity/resources/mesh/MeshRequest.h>
#include <serenity/resources/mesh/animation/Skeleton.h>
#include <stack>

using namespace Engine::priv;

AnimationData::AnimationData( float ticksPerSecond, float durationInTicks)
    : m_TicksPerSecond  { ticksPerSecond != 0.0f ? ticksPerSecond : 25.0f }
    , m_DurationInTicks { durationInTicks }
{}
AnimationData::AnimationData( const aiAnimation& assimpAnim, MeshRequest& request)
    : AnimationData{ (float)assimpAnim.mTicksPerSecond, (float)assimpAnim.mDuration }
{
    Engine::unordered_string_map<std::string, AnimationChannel> hashedChannels;
    for (uint32_t channelIdx = 0; channelIdx < assimpAnim.mNumChannels; ++channelIdx) {
        const aiNodeAnim& aiAnimChannel = *assimpAnim.mChannels[channelIdx];
        hashedChannels.emplace(std::piecewise_construct, std::forward_as_tuple(aiAnimChannel.mNodeName.C_Str()), std::forward_as_tuple(aiAnimChannel));
    }

    //sort into member data structure
    m_Channels.resize(request.m_NodeData.m_Nodes.size());
    for (uint32_t i = 0; i < m_Channels.size(); ++i) {
        auto& nodeName = request.m_NodeStrVector[i];
        if (hashedChannels.contains(nodeName)) {
            m_Channels[i] = std::move(hashedChannels.at(nodeName));
        }
    }
}
void AnimationData::ComputeTransforms(float TimeInSeconds, std::vector<glm::mat4>& Xforms, MeshSkeleton& skeleton, MeshNodeData& nodeData) {
    const float TimeInTicks   = TimeInSeconds * m_TicksPerSecond;
    const float AnimationTime = std::fmod(TimeInTicks, m_DurationInTicks);
    uint16_t BoneIndex = 0;
    for (uint32_t i = 1; i < nodeData.m_Nodes.size(); ++i) {
        const auto parentIdx    = nodeData.m_NodeHeirarchy[i];
        const auto& currNode    = nodeData.m_Nodes[i];
        auto& channel           = m_Channels[i];
        glm::mat4 NodeTransform = currNode.Transform;
        if (!channel.empty()) {
            glm::vec3 s         = CalcInterpolatedScaling(AnimationTime, channel.ScalingKeys, channel.CurrentKeyframes[2]);
            glm::quat q         = CalcInterpolatedRotation(AnimationTime, channel.RotationKeys, channel.CurrentKeyframes[1]);
            glm::vec3 t         = CalcInterpolatedPosition(AnimationTime, channel.PositionKeys, channel.CurrentKeyframes[0]);
            NodeTransform       = glm::translate(t) * glm::mat4_cast(q) * glm::scale(s);
        }
        nodeData.m_NodeTransforms[i] = nodeData.m_NodeTransforms[parentIdx - 1] * NodeTransform;
        if (currNode.IsBone) {
            BoneInfo& boneInfo  = skeleton.m_BoneInfo[BoneIndex];
            glm::mat4& Final    = boneInfo.FinalTransform;
            Final               = skeleton.m_GlobalInverseTransform * nodeData.m_NodeTransforms[i] * boneInfo.BoneOffset;
            Final               = Xforms[BoneIndex] * Final; //this line allows for animation combinations. only works when additional animations start off in their resting places...
            Xforms[BoneIndex]   = Final;
            ++BoneIndex;
        }

    }
    //this is wierd... TODO: see if this can be removed by doing this step above
    //for (auto i = 0U; i < skeleton.numBones(); ++i) {
    //    Xforms[i] = skeleton.m_BoneInfo[i].FinalTransform;
    //}
}
glm::vec3 AnimationData::internal_interpolate_vec3(float AnimTime, const std::vector<Engine::priv::Vector3Key>& keys, std::function<size_t()>&& FindKeyFrmIdx) {
    if (keys.size() == 1) {
        return keys[0].value;
    }
    size_t Index     = FindKeyFrmIdx();
    float DeltaTime  = keys[Index + 1].time - keys[Index].time;
    float Factor     = AnimTime - keys[Index].time / DeltaTime;
    return keys[Index].value + Factor * (keys[Index + 1].value - keys[Index].value);
}
glm::vec3 AnimationData::CalcInterpolatedPosition(float AnimTime, const std::vector<Engine::priv::Vector3Key>& positions, uint16_t& CurrentKeyFrame) {
    return internal_interpolate_vec3(AnimTime, positions, [this, &positions, AnimTime, &CurrentKeyFrame]() {
        return FindPositionIdx(AnimTime, positions, CurrentKeyFrame);
    });
}
glm::quat AnimationData::CalcInterpolatedRotation(float AnimTime, const std::vector<Engine::priv::QuatKey>& rotations, uint16_t& CurrentKeyFrame) {
    if (rotations.size() == 1) {
        return rotations[0].value;
    }
    size_t Index     = FindRotationIdx(AnimTime, rotations, CurrentKeyFrame);
    float DeltaTime  = rotations[Index + 1].time - rotations[Index].time;
    float Factor     = AnimTime - rotations[Index].time / DeltaTime;
    return glm::normalize(glm::slerp(rotations[Index].value, rotations[Index + 1].value, Factor));
}
glm::vec3 AnimationData::CalcInterpolatedScaling(float AnimTime, const std::vector<Engine::priv::Vector3Key>& scales, uint16_t& CurrentKeyFrame) {
    return internal_interpolate_vec3(AnimTime, scales, [this, &scales, AnimTime, &CurrentKeyFrame]() {
        return FindScalingIdx(AnimTime, scales, CurrentKeyFrame);
    });
}
