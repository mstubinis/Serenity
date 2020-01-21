#include <core/engine/mesh/Skeleton.h>
#include <core/engine/mesh/MeshImportedData.h>
#include <core/engine/math/Engine_Math.h>

using namespace Engine;
using namespace std;

priv::AnimationData::AnimationData(const MeshSkeleton& meshSkeleton, const aiAnimation& assimpAnimation) {
    m_MeshSkeleton = const_cast<MeshSkeleton*>(&meshSkeleton);
    m_TicksPerSecond = assimpAnimation.mTicksPerSecond;
    m_DurationInTicks = assimpAnimation.mDuration;
    for (unsigned int c = 0; c < assimpAnimation.mNumChannels; ++c) {
        const aiNodeAnim& aiAnimNode = *assimpAnimation.mChannels[c];
        if (!m_KeyframeData.count(aiAnimNode.mNodeName.data)) {
            AnimationChannel animChannel;
            for (unsigned int b = 0; b < aiAnimNode.mNumPositionKeys; ++b) {
                animChannel.PositionKeys.emplace_back(aiAnimNode.mPositionKeys[b].mTime, Math::assimpToGLMVec3(aiAnimNode.mPositionKeys[b].mValue));
            }
            for (unsigned int b = 0; b < aiAnimNode.mNumRotationKeys; ++b) {
                animChannel.RotationKeys.emplace_back(aiAnimNode.mRotationKeys[b].mTime, aiAnimNode.mRotationKeys[b].mValue);
            }
            for (unsigned int b = 0; b < aiAnimNode.mNumScalingKeys; ++b) {
                animChannel.ScalingKeys.emplace_back(aiAnimNode.mScalingKeys[b].mTime, Math::assimpToGLMVec3(aiAnimNode.mScalingKeys[b].mValue));
            }
            m_KeyframeData.emplace(aiAnimNode.mNodeName.data, std::move(animChannel));
        }
    }
}
priv::AnimationData::~AnimationData() {
    m_KeyframeData.clear();
}
void priv::AnimationData::ReadNodeHeirarchy(const string& animationName, float time, const BoneNode* node, glm::mat4& ParentTransform, vector<glm::mat4>& Transforms) {
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
    auto& skeleton = *m_MeshSkeleton;
    if (skeleton.m_BoneMapping.count(BoneName)) {
        unsigned int BoneIndex(skeleton.m_BoneMapping.at(BoneName));
        BoneInfo& boneInfo = skeleton.m_BoneInfo[BoneIndex];
        glm::mat4& Final = boneInfo.FinalTransform;
        Final = skeleton.m_GlobalInverseTransform * Transform * boneInfo.BoneOffset;
        //this line allows for animation combinations. only works when additional animations start off in their resting places...
        Final = Transforms[BoneIndex] * Final;
    }
    for (size_t i = 0; i < node->Children.size(); ++i) {
        ReadNodeHeirarchy(animationName, time, node->Children[i], Transform, Transforms);
    }
}
void priv::AnimationData::BoneTransform(const string& animationName, float TimeInSeconds, vector<glm::mat4>& Transforms) {
    float TicksPerSecond = float(m_TicksPerSecond != 0 ? m_TicksPerSecond : 25.0f);
    float TimeInTicks(TimeInSeconds * TicksPerSecond);
    float AnimationTime(float(fmod(TimeInTicks, m_DurationInTicks)));
    glm::mat4 ParentIdentity(1.0f);
    auto& skeleton = *m_MeshSkeleton;
    ReadNodeHeirarchy(animationName, AnimationTime, skeleton.m_RootNode, ParentIdentity, Transforms);
    for (unsigned int i = 0; i < skeleton.m_NumBones; ++i) {
        Transforms[i] = skeleton.m_BoneInfo[i].FinalTransform;
    }
}
void priv::AnimationData::CalcInterpolatedPosition(glm::vec3& Out, float AnimationTime, const AnimationChannel& node) {
    if (node.PositionKeys.size() == 1) {
        Out = node.PositionKeys[0].value; return;
    }
    size_t PositionIndex(FindPosition(AnimationTime, node));
    size_t NextIndex(PositionIndex + 1);
    float DeltaTime((float)(node.PositionKeys[NextIndex].time - node.PositionKeys[PositionIndex].time));
    float Factor((AnimationTime - (float)node.PositionKeys[PositionIndex].time) / DeltaTime);
    glm::vec3 Start(node.PositionKeys[PositionIndex].value);
    glm::vec3 End(node.PositionKeys[NextIndex].value);
    Out = Start + Factor * (End - Start);
}
void priv::AnimationData::CalcInterpolatedRotation(aiQuaternion& Out, float AnimationTime, const AnimationChannel& node) {
    if (node.RotationKeys.size() == 1) {
        Out = node.RotationKeys[0].value; return;
    }
    size_t RotationIndex(FindRotation(AnimationTime, node));
    size_t NextIndex(RotationIndex + 1);
    float DeltaTime((float)(node.RotationKeys[NextIndex].time - node.RotationKeys[RotationIndex].time));
    float Factor((AnimationTime - (float)node.RotationKeys[RotationIndex].time) / DeltaTime);
    const aiQuaternion& StartRotationQ = node.RotationKeys[RotationIndex].value;
    const aiQuaternion& EndRotationQ = node.RotationKeys[NextIndex].value;
    aiQuaternion::Interpolate(Out, StartRotationQ, EndRotationQ, Factor);
    Out = Out.Normalize();
}
void priv::AnimationData::CalcInterpolatedScaling(glm::vec3& Out, float AnimationTime, const AnimationChannel& node) {
    if (node.ScalingKeys.size() == 1) {
        Out = node.ScalingKeys[0].value; return;
    }
    size_t ScalingIndex(FindScaling(AnimationTime, node));
    size_t NextIndex(ScalingIndex + 1);
    float DeltaTime((float)(node.ScalingKeys[NextIndex].time - node.ScalingKeys[ScalingIndex].time));
    float Factor((AnimationTime - (float)node.ScalingKeys[ScalingIndex].time) / DeltaTime);
    glm::vec3 Start(node.ScalingKeys[ScalingIndex].value);
    glm::vec3 End(node.ScalingKeys[NextIndex].value);
    Out = Start + Factor * (End - Start);
}
const size_t priv::AnimationData::FindPosition(const float AnimationTime, const AnimationChannel& node) {
    for (size_t i = 0; i < node.PositionKeys.size() - 1; ++i) { if (AnimationTime < (float)node.PositionKeys[i + 1].time) { return i; } }return 0;
}
const size_t priv::AnimationData::FindRotation(const float AnimationTime, const AnimationChannel& node) {
    for (size_t i = 0; i < node.RotationKeys.size() - 1; ++i) { if (AnimationTime < (float)node.RotationKeys[i + 1].time) { return i; } }return 0;
}
const size_t priv::AnimationData::FindScaling(const float AnimationTime, const AnimationChannel& node) {
    for (size_t i = 0; i < node.ScalingKeys.size() - 1; ++i) { if (AnimationTime < (float)node.ScalingKeys[i + 1].time) { return i; } }return 0;
}
const float priv::AnimationData::duration() {
    float TicksPerSecond(float(m_TicksPerSecond != 0 ? m_TicksPerSecond : 25.0f)); return float(float(m_DurationInTicks) / TicksPerSecond);
}












priv::MeshSkeleton::MeshSkeleton() {
    m_RootNode = nullptr;
    clear();
}
priv::MeshSkeleton::MeshSkeleton(const MeshImportedData& data) {
    m_RootNode = nullptr;
    fill(data);
}
priv::MeshSkeleton::~MeshSkeleton() {
    clear();
    cleanup();
}
const unsigned int& priv::MeshSkeleton::numBones() {
    return m_NumBones; 
}
void priv::MeshSkeleton::fill(const MeshImportedData& data) {
    for (auto& _b : data.m_Bones) {
        const VertexBoneData& b = _b.second;
        m_BoneIDs.push_back(glm::vec4(b.IDs[0], b.IDs[1], b.IDs[2], b.IDs[3]));
        m_BoneWeights.push_back(glm::vec4(b.Weights[0], b.Weights[1], b.Weights[2], b.Weights[3]));
    }
}
void priv::MeshSkeleton::populateCleanupMap(BoneNode* node, unordered_map<string, BoneNode*>& _map) {
    if (!_map.count(node->Name)) 
        _map.emplace(node->Name, node);
    for (auto& child : node->Children) {
        populateCleanupMap(child, _map);
    }
}
void priv::MeshSkeleton::cleanup() {
    unordered_map<string, BoneNode*> nodes;
    populateCleanupMap(m_RootNode, nodes);
    SAFE_DELETE_MAP(nodes);
}
void priv::MeshSkeleton::clear() {
    m_NumBones = 0;
    m_BoneMapping.clear();
}







