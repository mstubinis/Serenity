
#include <serenity/resources/mesh/animation/AnimationData.h>
#include <serenity/math/Engine_Math.h>
#include <serenity/resources/mesh/Mesh.h>
#include <serenity/resources/mesh/animation/Skeleton.h>
#include <stack>

using namespace Engine::priv;

AnimationData::AnimationData( MeshNodeData& nodeData, Engine::priv::MeshSkeleton& skeleton, float ticksPerSecond, float durationInTicks)
    : m_NodeData        { &nodeData }
    , m_Skeleton        { &skeleton }
    , m_TicksPerSecond  { ticksPerSecond != 0.0f ? ticksPerSecond : 25.0f }
    , m_DurationInTicks { durationInTicks }
{
}
AnimationData::AnimationData( MeshNodeData& nodeData, Engine::priv::MeshSkeleton& skeleton, const aiAnimation& assimpAnim, MeshNodeData& filledNodes)
    : AnimationData{ nodeData, skeleton, (float)assimpAnim.mTicksPerSecond, (float)assimpAnim.mDuration }
{
    Engine::unordered_string_map<std::string, AnimationChannel> hashedChannels;
    for (uint32_t channelIdx = 0; channelIdx < assimpAnim.mNumChannels; ++channelIdx) {
        const aiNodeAnim& aiAnimChannel = *assimpAnim.mChannels[channelIdx];
        hashedChannels.emplace(std::piecewise_construct, std::forward_as_tuple(aiAnimChannel.mNodeName.C_Str()), std::forward_as_tuple(aiAnimChannel));
    }

    //sort into member data structure
    m_Channels.resize(filledNodes.m_Nodes.size());
    for (uint32_t i = 0; i < m_Channels.size(); ++i) {
        auto& nodeName = filledNodes.m_Nodes[i].Name;
        if (hashedChannels.contains(nodeName)) {
            m_Channels[i] = std::move(hashedChannels.at(nodeName));
        }
    }
}
void AnimationData::ComputeTransforms(float TimeInSeconds, std::vector<glm::mat4>& Xforms, std::array<uint16_t, 3>& keyframeIndices) {
    float TimeInTicks   = TimeInSeconds * m_TicksPerSecond;
    float AnimationTime = std::fmod(TimeInTicks, m_DurationInTicks);
    //uint16_t BoneIndex = 0;
    for (uint32_t i = 1; i < m_NodeData->m_Nodes.size(); ++i) {
        const auto parentIdx    = m_NodeData->m_NodeHeirarchy[i];
        const auto& currNode    = m_NodeData->m_Nodes[i];
        auto& channel           = m_Channels[i];
        glm::mat4 NodeTransform = currNode.Transform;
        if (!channel.empty()) {
            glm::vec3 s         = CalcInterpolatedScaling(AnimationTime, channel.ScalingKeys, keyframeIndices[2]);
            aiQuaternion q      = CalcInterpolatedRotation(AnimationTime, channel.RotationKeys, keyframeIndices[1]);
            glm::vec3 t         = CalcInterpolatedPosition(AnimationTime, channel.PositionKeys, keyframeIndices[0]);
            glm::mat4 rotation  = Math::assimpToGLMMat3(q.GetMatrix());
            NodeTransform       = glm::translate(t) * rotation * glm::scale(s);
        }
        m_NodeData->m_NodeTransforms[i] = m_NodeData->m_NodeTransforms[parentIdx - 1] * NodeTransform;
        if (m_Skeleton->hasBone(currNode.Name)) {
            uint16_t BoneIndex = m_Skeleton->m_BoneMapping.at(currNode.Name);
            BoneInfo& boneInfo = m_Skeleton->m_BoneInfo[BoneIndex];
            glm::mat4& Final   = boneInfo.FinalTransform;
            Final              = m_Skeleton->m_GlobalInverseTransform * m_NodeData->m_NodeTransforms[i] * boneInfo.BoneOffset;
            //this line allows for animation combinations. only works when additional animations start off in their resting places...
            //Final              = Xforms[BoneIndex] * Final;
            //Xforms[BoneIndex]  = Final;
            //++BoneIndex;
        }

    }
    //this is wierd... TODO: see if this can be removed by doing this step above
    for (auto i = 0U; i < m_Skeleton->numBones(); ++i) {
        Xforms[i] = m_Skeleton->m_BoneInfo[i].FinalTransform;
    }
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
glm::vec3 AnimationData::CalcInterpolatedPosition(float AnimTime, const std::vector<Engine::priv::Vector3Key>& positions, uint16_t& LastKeyFrm) {
    return internal_interpolate_vec3(AnimTime, positions, [this, &positions, AnimTime, &LastKeyFrm]() {
        return FindPositionIdx(AnimTime, positions, LastKeyFrm);
    });
}
aiQuaternion AnimationData::CalcInterpolatedRotation(float AnimTime, const std::vector<Engine::priv::QuatKey>& rotations, uint16_t& LastKeyFrm) {
    if (rotations.size() == 1) {
        return rotations[0].value;
    }
    size_t Index     = FindRotationIdx(AnimTime, rotations, LastKeyFrm);
    float DeltaTime  = rotations[Index + 1].time - rotations[Index].time;
    float Factor     = AnimTime - rotations[Index].time / DeltaTime;
    aiQuaternion out;
    aiQuaternion::Interpolate(out, rotations[Index].value, rotations[Index + 1].value, Factor);
    out = out.Normalize();
    return out;
}
glm::vec3 AnimationData::CalcInterpolatedScaling(float AnimTime, const std::vector<Engine::priv::Vector3Key>& scales, uint16_t& LastKeyFrm) {
    return internal_interpolate_vec3(AnimTime, scales, [this, &scales, AnimTime, &LastKeyFrm]() {
        return FindScalingIdx(AnimTime, scales, LastKeyFrm);
    });
}
