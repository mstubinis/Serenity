#include <serenity/resources/mesh/animation/AnimationIncludes.h>
#include <serenity/math/Engine_Math.h>

Engine::priv::AnimationChannel::AnimationChannel(const aiNodeAnim& aiAnimNode) {
    PositionKeys.reserve(aiAnimNode.mNumPositionKeys);
    RotationKeys.reserve(aiAnimNode.mNumRotationKeys);
    ScalingKeys.reserve(aiAnimNode.mNumScalingKeys);
    for (auto i = 0U; i < aiAnimNode.mNumPositionKeys; ++i) {
        PositionKeys.emplace_back((float)aiAnimNode.mPositionKeys[i].mTime, Engine::Math::toGLM(aiAnimNode.mPositionKeys[i].mValue));
    }
    for (auto i = 0U; i < aiAnimNode.mNumRotationKeys; ++i) {
        RotationKeys.emplace_back((float)aiAnimNode.mRotationKeys[i].mTime, Engine::Math::toGLM(aiAnimNode.mRotationKeys[i].mValue));
    }
    for (auto i = 0U; i < aiAnimNode.mNumScalingKeys; ++i) {
        ScalingKeys.emplace_back((float)aiAnimNode.mScalingKeys[i].mTime, Engine::Math::toGLM(aiAnimNode.mScalingKeys[i].mValue));
    }
}