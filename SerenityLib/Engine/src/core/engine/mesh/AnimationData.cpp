#include <core/engine/mesh/AnimationData.h>
#include <core/engine/math/Engine_Math.h>
#include <core/engine/mesh/Mesh.h>
#include <core/engine/mesh/Skeleton.h>

using namespace Engine::priv;
using namespace std;

AnimationData::AnimationData(Mesh& mesh, const aiAnimation& assimpAnimation) : m_Mesh(&mesh) {
    m_TicksPerSecond  = (float)assimpAnimation.mTicksPerSecond;
    m_DurationInTicks = (float)assimpAnimation.mDuration;
    for (unsigned int c = 0; c < assimpAnimation.mNumChannels; ++c) {
        const aiNodeAnim& aiAnimNode = *assimpAnimation.mChannels[c];
        if (!m_KeyframeData.count(aiAnimNode.mNodeName.data)) {
            AnimationChannel animation_channel;
            for (unsigned int b = 0; b < aiAnimNode.mNumPositionKeys; ++b) {
                animation_channel.PositionKeys.emplace_back(  float(aiAnimNode.mPositionKeys[b].mTime), Math::assimpToGLMVec3(aiAnimNode.mPositionKeys[b].mValue)  );
            }
            for (unsigned int b = 0; b < aiAnimNode.mNumRotationKeys; ++b) {
                animation_channel.RotationKeys.emplace_back(  float(aiAnimNode.mRotationKeys[b].mTime), aiAnimNode.mRotationKeys[b].mValue  );
            }
            for (unsigned int b = 0; b < aiAnimNode.mNumScalingKeys; ++b) {
                animation_channel.ScalingKeys.emplace_back(  float(aiAnimNode.mScalingKeys[b].mTime), Math::assimpToGLMVec3(aiAnimNode.mScalingKeys[b].mValue)  );
            }
            m_KeyframeData.emplace(aiAnimNode.mNodeName.data, std::move(animation_channel));
        }
    }
}
AnimationData::AnimationData(Mesh& mesh, float ticksPerSecond, float durationInTicks) : m_Mesh(&mesh) {
    m_TicksPerSecond  = ticksPerSecond;
    m_DurationInTicks = durationInTicks;
}
AnimationData::~AnimationData() {
    m_KeyframeData.clear();
}
void AnimationData::ReadNodeHeirarchy(const string& animationName, float time, const MeshInfoNode* node, const glm::mat4& ParentTransform, vector<glm::mat4>& Transforms) {
    glm::mat4 NodeTransform(node->Transform);
    if (m_KeyframeData.count(node->Name)) {
        AnimationChannel keyframes(m_KeyframeData.at(node->Name));
        glm::vec3 s; CalcInterpolatedScaling(s, time, keyframes);
        aiQuaternion q; CalcInterpolatedRotation(q, time, keyframes);
        glm::mat4 rotation(Math::assimpToGLMMat3(q.GetMatrix()));
        glm::vec3 t; CalcInterpolatedPosition(t, time, keyframes);
        NodeTransform  = glm::mat4(1.0f);
        NodeTransform  = glm::translate(NodeTransform, t);
        NodeTransform *= rotation;
        NodeTransform  = glm::scale(NodeTransform, s);
    }
    glm::mat4 Transform(ParentTransform * NodeTransform);
    if (m_Mesh->m_Skeleton->m_BoneMapping.count(node->Name)) {
        unsigned int BoneIndex(m_Mesh->m_Skeleton->m_BoneMapping.at(node->Name));
        BoneInfo& boneInfo   = m_Mesh->m_Skeleton->m_BoneInfo[BoneIndex];
        glm::mat4& Final     = boneInfo.FinalTransform;
        Final                = m_Mesh->m_Skeleton->m_GlobalInverseTransform * Transform * boneInfo.BoneOffset;
        //this line allows for animation combinations. only works when additional animations start off in their resting places...
        Final                = Transforms[BoneIndex] * Final;
    }
    for (size_t i = 0; i < node->Children.size(); ++i) {
        ReadNodeHeirarchy(animationName, time, node->Children[i], Transform, Transforms);
    }
}
void AnimationData::BoneTransform(const string& animationName, float TimeInSeconds, vector<glm::mat4>& Transforms) {
    float TicksPerSecond = (m_TicksPerSecond != 0.0) ? m_TicksPerSecond : 25.0;
    float TimeInTicks(TimeInSeconds * TicksPerSecond);
    float AnimationTime(fmod(TimeInTicks, m_DurationInTicks));
    glm::mat4 ParentIdentity(1.0f);
    ReadNodeHeirarchy(animationName, AnimationTime, m_Mesh->m_RootNode, ParentIdentity, Transforms);
    for (unsigned int i = 0; i < m_Mesh->m_Skeleton->m_NumBones; ++i) {
        Transforms[i] = m_Mesh->m_Skeleton->m_BoneInfo[i].FinalTransform;
    }
}
void AnimationData::internal_interpolate_vec3(glm::vec3& Out, float AnimationTime, const vector<Engine::priv::Vector3Key>& keys, function<size_t()> call) {
    if (keys.size() == 1) {
        Out = keys[0].value;
        return;
    }
    size_t CurrentIndex(call());
    size_t NextIndex(CurrentIndex + 1);
    float DeltaTime(keys[NextIndex].time - keys[CurrentIndex].time);
    float Factor(AnimationTime - keys[CurrentIndex].time / DeltaTime);
    glm::vec3 Start(keys[CurrentIndex].value);
    glm::vec3 End(keys[NextIndex].value);
    Out = Start + Factor * (End - Start);
}
void AnimationData::CalcInterpolatedPosition(glm::vec3& Out, float AnimationTime, const AnimationChannel& node) {
    auto lambda_find_position = [this, &node, AnimationTime]() {
        return FindPosition(AnimationTime, node);
    };
    return internal_interpolate_vec3(Out, AnimationTime, node.PositionKeys, lambda_find_position);
}
void AnimationData::CalcInterpolatedRotation(aiQuaternion& Out, float AnimationTime, const AnimationChannel& node) {
    if (node.RotationKeys.size() == 1) {
        Out = node.RotationKeys[0].value;
        return;
    }
    size_t RotationIndex(FindRotation(AnimationTime, node));
    size_t NextIndex(RotationIndex + 1);
    float DeltaTime(node.RotationKeys[NextIndex].time - node.RotationKeys[RotationIndex].time);
    float Factor(AnimationTime - node.RotationKeys[RotationIndex].time / DeltaTime);
    const aiQuaternion& StartRotationQ = node.RotationKeys[RotationIndex].value;
    const aiQuaternion& EndRotationQ   = node.RotationKeys[NextIndex].value;
    aiQuaternion::Interpolate(Out, StartRotationQ, EndRotationQ, Factor);
    Out = Out.Normalize();
}
void AnimationData::CalcInterpolatedScaling(glm::vec3& Out, float AnimationTime, const AnimationChannel& node) {
    auto lambda_find_scaling = [this, &node, AnimationTime]() {
        return FindScaling(AnimationTime, node);
    };
    return internal_interpolate_vec3(Out, AnimationTime, node.ScalingKeys, lambda_find_scaling); 
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
float AnimationData::duration() const {
    float TicksPerSecond( (m_TicksPerSecond != 0.0f) ? m_TicksPerSecond : 25.0f);
    return m_DurationInTicks / TicksPerSecond;
}
float AnimationData::durationInTicks() const {
    return m_DurationInTicks;
}
float AnimationData::ticksPerSecond() const {
    return m_TicksPerSecond;
}