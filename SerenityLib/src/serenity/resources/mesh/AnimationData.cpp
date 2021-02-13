
#include <serenity/resources/mesh/AnimationData.h>
#include <serenity/math/Engine_Math.h>
#include <serenity/resources/mesh/Mesh.h>
#include <serenity/resources/mesh/Skeleton.h>

using namespace Engine::priv;

AnimationData::AnimationData(MeshCPUData& cpuData, float ticksPerSecond, float durationInTicks)
    : m_MeshCPUData     { &cpuData }
    , m_TicksPerSecond  { ticksPerSecond }
    , m_DurationInTicks { durationInTicks }
{}
AnimationData::AnimationData(MeshCPUData& cpuData, const aiAnimation& assimpAnimation)
    : AnimationData{ cpuData, (float)assimpAnimation.mTicksPerSecond, (float)assimpAnimation.mDuration }
{
    for (auto c = 0U; c < assimpAnimation.mNumChannels; ++c) {
        const aiNodeAnim& aiAnimNode = *assimpAnimation.mChannels[c];
        if (!m_KeyframeData.contains(aiAnimNode.mNodeName.data)) {
            AnimationChannel animation_channel;
            for (auto b = 0U; b < aiAnimNode.mNumPositionKeys; ++b) {
                animation_channel.PositionKeys.emplace_back((float)aiAnimNode.mPositionKeys[b].mTime, Math::assimpToGLMVec3(aiAnimNode.mPositionKeys[b].mValue));
            }
            for (auto b = 0U; b < aiAnimNode.mNumRotationKeys; ++b) {
                animation_channel.RotationKeys.emplace_back((float)aiAnimNode.mRotationKeys[b].mTime, aiAnimNode.mRotationKeys[b].mValue);
            }
            for (auto b = 0U; b < aiAnimNode.mNumScalingKeys; ++b) {
                animation_channel.ScalingKeys.emplace_back((float)aiAnimNode.mScalingKeys[b].mTime, Math::assimpToGLMVec3(aiAnimNode.mScalingKeys[b].mValue));
            }
            m_KeyframeData.emplace(
                std::piecewise_construct,
                std::forward_as_tuple(aiAnimNode.mNodeName.data), 
                std::forward_as_tuple(std::move(animation_channel))
            );
        }
    }
}
void AnimationData::ReadNodeHeirarchy(const std::string& animationName, float time, const MeshInfoNode* node, const glm::mat4& ParentTransform, std::vector<glm::mat4>& Transforms) {
    glm::mat4 NodeTransform{ node->Transform };
    if (m_KeyframeData.contains(node->Name)) {
        AnimationChannel keyframes{ m_KeyframeData.at(node->Name) };
        glm::vec3 s    = CalcInterpolatedScaling(time, keyframes);
        aiQuaternion q = CalcInterpolatedRotation(time, keyframes);
        glm::vec3 t    = CalcInterpolatedPosition(time, keyframes);
        glm::mat4 rotation{ Math::assimpToGLMMat3(q.GetMatrix()) };
        NodeTransform  = glm::mat4{ 1.0f };
        NodeTransform  = glm::translate(NodeTransform, t);
        NodeTransform *= rotation;
        NodeTransform  = glm::scale(NodeTransform, s);
    }
    glm::mat4 Transform{ ParentTransform * NodeTransform };
    if (m_MeshCPUData->m_Skeleton->m_BoneMapping.contains(node->Name)) {
        unsigned int BoneIndex{ m_MeshCPUData->m_Skeleton->m_BoneMapping.at(node->Name) };
        BoneInfo& boneInfo   = m_MeshCPUData->m_Skeleton->m_BoneInfo[BoneIndex];
        glm::mat4& Final     = boneInfo.FinalTransform;
        Final                = m_MeshCPUData->m_Skeleton->m_GlobalInverseTransform * Transform * boneInfo.BoneOffset;
        //this line allows for animation combinations. only works when additional animations start off in their resting places...
        Final                = Transforms[BoneIndex] * Final;
    }
    for (size_t i = 0; i < node->Children.size(); ++i) {
        ReadNodeHeirarchy(animationName, time, node->Children[i].get(), Transform, Transforms);
    }
}
void AnimationData::BoneTransform(const std::string& animationName, float TimeInSeconds, std::vector<glm::mat4>& Transforms) {
    float TicksPerSecond = (m_TicksPerSecond != 0.0f) ? m_TicksPerSecond : 25.0f;
    float TimeInTicks{ TimeInSeconds * TicksPerSecond };
    float AnimationTime{ std::fmod(TimeInTicks, m_DurationInTicks) };
    glm::mat4 ParentIdentity{ 1.0f };
    ReadNodeHeirarchy(animationName, AnimationTime, m_MeshCPUData->m_RootNode, ParentIdentity, Transforms);
    for (auto i = 0U; i < m_MeshCPUData->m_Skeleton->m_NumBones; ++i) {
        Transforms[i] = m_MeshCPUData->m_Skeleton->m_BoneInfo[i].FinalTransform;
    }
}
glm::vec3 AnimationData::internal_interpolate_vec3(float AnimationTime, const std::vector<Engine::priv::Vector3Key>& keys, std::function<size_t()>&& call) {
    if (keys.size() == 1) {
        return keys[0].value;
    }
    size_t CurrentIndex{ call() };
    size_t NextIndex{ CurrentIndex + 1 };
    float DeltaTime{ keys[NextIndex].time - keys[CurrentIndex].time };
    float Factor{ AnimationTime - keys[CurrentIndex].time / DeltaTime };
    glm::vec3 Start{ keys[CurrentIndex].value };
    glm::vec3 End{ keys[NextIndex].value };
    return Start + Factor * (End - Start);
}
glm::vec3 AnimationData::CalcInterpolatedPosition(float AnimationTime, const AnimationChannel& node) {
    return internal_interpolate_vec3(AnimationTime, node.PositionKeys, [this, &node, AnimationTime]() { return FindPosition(AnimationTime, node); });
}
aiQuaternion AnimationData::CalcInterpolatedRotation(float AnimationTime, const AnimationChannel& node) {
    if (node.RotationKeys.size() == 1) {
        return node.RotationKeys[0].value;
    }
    size_t RotationIndex{ FindRotation(AnimationTime, node) };
    size_t NextIndex{ RotationIndex + 1 };
    float DeltaTime{ node.RotationKeys[NextIndex].time - node.RotationKeys[RotationIndex].time };
    float Factor{ AnimationTime - node.RotationKeys[RotationIndex].time / DeltaTime };
    const aiQuaternion& StartRotationQ = node.RotationKeys[RotationIndex].value;
    const aiQuaternion& EndRotationQ   = node.RotationKeys[NextIndex].value;
    aiQuaternion out;
    out.Interpolate(out, StartRotationQ, EndRotationQ, Factor);
    out = out.Normalize();
    return out;
}
glm::vec3 AnimationData::CalcInterpolatedScaling(float AnimationTime, const AnimationChannel& node) {
    return internal_interpolate_vec3(AnimationTime, node.ScalingKeys, [this, &node, AnimationTime]() { return FindScaling(AnimationTime, node); });
}
size_t AnimationData::FindPosition(float AnimationTime, const AnimationChannel& node) const {
    return internal_find(AnimationTime, node, node.PositionKeys);
}
size_t AnimationData::FindRotation(float AnimationTime, const AnimationChannel& node) const {
    return internal_find(AnimationTime, node, node.RotationKeys);
}
size_t AnimationData::FindScaling(float AnimationTime, const AnimationChannel& node) const {
    return internal_find(AnimationTime, node, node.ScalingKeys);
}
