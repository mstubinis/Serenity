#include <core/engine/mesh/AnimationData.h>
#include <core/engine/math/Engine_Math.h>
#include <core/engine/mesh/Skeleton.h>

using namespace Engine::priv;
using namespace std;

AnimationData::AnimationData(const MeshSkeleton& meshSkeleton, const aiAnimation& assimpAnimation) : m_MeshSkeleton(const_cast<MeshSkeleton&>(meshSkeleton)) {
    m_TicksPerSecond  = assimpAnimation.mTicksPerSecond;
    m_DurationInTicks = assimpAnimation.mDuration;
    for (unsigned int c = 0; c < assimpAnimation.mNumChannels; ++c) {
        const aiNodeAnim& aiAnimNode = *assimpAnimation.mChannels[c];
        if (!m_KeyframeData.count(aiAnimNode.mNodeName.data)) {
            AnimationChannel animation_channel;
            for (unsigned int b = 0; b < aiAnimNode.mNumPositionKeys; ++b) {
                animation_channel.PositionKeys.emplace_back(  aiAnimNode.mPositionKeys[b].mTime, Math::assimpToGLMVec3(aiAnimNode.mPositionKeys[b].mValue)  );
            }
            for (unsigned int b = 0; b < aiAnimNode.mNumRotationKeys; ++b) {
                animation_channel.RotationKeys.emplace_back(  aiAnimNode.mRotationKeys[b].mTime, aiAnimNode.mRotationKeys[b].mValue  );
            }
            for (unsigned int b = 0; b < aiAnimNode.mNumScalingKeys; ++b) {
                animation_channel.ScalingKeys.emplace_back(  aiAnimNode.mScalingKeys[b].mTime, Math::assimpToGLMVec3(aiAnimNode.mScalingKeys[b].mValue)  );
            }
            m_KeyframeData.emplace(aiAnimNode.mNodeName.data, std::move(animation_channel));
        }
    }
}
AnimationData::~AnimationData() {
    m_KeyframeData.clear();
}
void AnimationData::ReadNodeHeirarchy(const string& animationName, const float time, const BoneNode* node, const glm::mat4& ParentTransform, vector<glm::mat4>& Transforms) {
    string BoneName(node->Name);
    glm::mat4 NodeTransform(node->Transform);
    if (m_KeyframeData.count(BoneName)) {
        const auto keyframes(m_KeyframeData.at(BoneName));
        glm::vec3 s; CalcInterpolatedScaling(s, time, keyframes);
        aiQuaternion q; CalcInterpolatedRotation(q, time, keyframes);
        glm::mat4 rotation(Math::assimpToGLMMat3(q.GetMatrix()));
        glm::vec3 t; CalcInterpolatedPosition(t, time, keyframes);
        NodeTransform = glm::mat4(1.0f);
        NodeTransform = glm::translate(NodeTransform, t);
        NodeTransform *= rotation;
        NodeTransform = glm::scale(NodeTransform, s);
    }
    glm::mat4 Transform(ParentTransform * NodeTransform);
    if (m_MeshSkeleton.m_BoneMapping.count(BoneName)) {
        unsigned int BoneIndex(m_MeshSkeleton.m_BoneMapping.at(BoneName));
        BoneInfo& boneInfo   = m_MeshSkeleton.m_BoneInfo[BoneIndex];
        glm::mat4& Final     = boneInfo.FinalTransform;
        Final                = m_MeshSkeleton.m_GlobalInverseTransform * Transform * boneInfo.BoneOffset;
        //this line allows for animation combinations. only works when additional animations start off in their resting places...
        Final                = Transforms[BoneIndex] * Final;
    }
    for (size_t i = 0; i < node->Children.size(); ++i) {
        ReadNodeHeirarchy(animationName, time, node->Children[i], Transform, Transforms);
    }
}
void AnimationData::BoneTransform(const string& animationName, const float TimeInSeconds, vector<glm::mat4>& Transforms) {
    float TicksPerSecond = static_cast<float>(m_TicksPerSecond != 0.0 ? m_TicksPerSecond : 25.0);
    float TimeInTicks(TimeInSeconds * TicksPerSecond);
    float AnimationTime(static_cast<float>(fmod(TimeInTicks, m_DurationInTicks)));
    glm::mat4 ParentIdentity(1.0f);
    ReadNodeHeirarchy(animationName, AnimationTime, m_MeshSkeleton.m_RootNode, ParentIdentity, Transforms);
    for (unsigned int i = 0; i < m_MeshSkeleton.m_NumBones; ++i) {
        Transforms[i] = m_MeshSkeleton.m_BoneInfo[i].FinalTransform;
    }
}
void AnimationData::internal_interpolate_vec3(glm::vec3& Out, const float AnimationTime, const vector<Engine::priv::Vector3Key>& keys, function<size_t()> call) {
    if (keys.size() == 1) {
        Out = keys[0].value;
        return;
    }
    size_t CurrentIndex(call());
    size_t NextIndex(CurrentIndex + 1);
    float DeltaTime(static_cast<float>(keys[NextIndex].time - keys[CurrentIndex].time));
    float Factor(AnimationTime - static_cast<float>(keys[CurrentIndex].time) / DeltaTime);
    glm::vec3 Start(keys[CurrentIndex].value);
    glm::vec3 End(keys[NextIndex].value);
    Out = Start + Factor * (End - Start);
}
void AnimationData::CalcInterpolatedPosition(glm::vec3& Out, const float AnimationTime, const AnimationChannel& node) {
    auto lambda_find_position = [this, &node, AnimationTime]() {
        return FindPosition(AnimationTime, node);
    };
    return internal_interpolate_vec3(Out, AnimationTime, node.PositionKeys, lambda_find_position);
    /*
    if (node.PositionKeys.size() == 1) {
        Out = node.PositionKeys[0].value;
        return;
    }
    size_t CurrentIndex(FindPosition(AnimationTime, node));
    size_t NextIndex(CurrentIndex + 1);
    float DeltaTime(static_cast<float>(node.PositionKeys[NextIndex].time - node.PositionKeys[CurrentIndex].time));
    float Factor(AnimationTime - static_cast<float>(node.PositionKeys[CurrentIndex].time) / DeltaTime);
    glm::vec3 Start(node.PositionKeys[CurrentIndex].value);
    glm::vec3 End(node.PositionKeys[NextIndex].value);
    Out = Start + Factor * (End - Start);
    */
}
void AnimationData::CalcInterpolatedRotation(aiQuaternion& Out, const float AnimationTime, const AnimationChannel& node) {
    if (node.RotationKeys.size() == 1) {
        Out = node.RotationKeys[0].value;
        return;
    }
    size_t RotationIndex(FindRotation(AnimationTime, node));
    size_t NextIndex(RotationIndex + 1);
    float DeltaTime(static_cast<float>(node.RotationKeys[NextIndex].time - node.RotationKeys[RotationIndex].time));
    float Factor(AnimationTime - static_cast<float>(node.RotationKeys[RotationIndex].time) / DeltaTime);
    const aiQuaternion& StartRotationQ = node.RotationKeys[RotationIndex].value;
    const aiQuaternion& EndRotationQ   = node.RotationKeys[NextIndex].value;
    aiQuaternion::Interpolate(Out, StartRotationQ, EndRotationQ, Factor);
    Out = Out.Normalize();
}
void AnimationData::CalcInterpolatedScaling(glm::vec3& Out, const float AnimationTime, const AnimationChannel& node) {
    auto lambda_find_scaling = [this, &node, AnimationTime]() {
        return FindScaling(AnimationTime, node);
    };
    return internal_interpolate_vec3(Out, AnimationTime, node.ScalingKeys, lambda_find_scaling);
    /*
    if (node.ScalingKeys.size() == 1) {
        Out = node.ScalingKeys[0].value;
        return;
    }
    size_t CurrentIndex(FindScaling(AnimationTime, node));
    size_t NextIndex(CurrentIndex + 1);
    float DeltaTime(static_cast<float>(node.ScalingKeys[NextIndex].time - node.ScalingKeys[CurrentIndex].time));
    float Factor(AnimationTime - static_cast<float>(node.ScalingKeys[CurrentIndex].time) / DeltaTime);
    glm::vec3 Start(node.ScalingKeys[CurrentIndex].value);
    glm::vec3 End(node.ScalingKeys[NextIndex].value);
    Out = Start + Factor * (End - Start);
    */
}
const size_t AnimationData::FindPosition(const float AnimationTime, const AnimationChannel& node) const {
    return internal_find(AnimationTime, node, node.PositionKeys);
}
const size_t AnimationData::FindRotation(const float AnimationTime, const AnimationChannel& node) const {
    return internal_find(AnimationTime, node, node.RotationKeys);
}
const size_t AnimationData::FindScaling(const float AnimationTime, const AnimationChannel& node) const {
    return internal_find(AnimationTime, node, node.ScalingKeys);
}
const float AnimationData::duration() const {
    float TicksPerSecond(m_TicksPerSecond != 0.0 ? static_cast<float>(m_TicksPerSecond) : 25.0f);
    return static_cast<float>(m_DurationInTicks) / TicksPerSecond;
}
