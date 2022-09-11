#include <serenity/resources/mesh/animation/Skeleton.h>
#include <serenity/resources/mesh/MeshRequest.h>
#include <serenity/resources/mesh/MeshImportedData.h>
#include <serenity/math/Engine_Math.h>

Engine::priv::MeshSkeleton::MeshSkeleton(const aiMesh& assimpMesh, const aiScene& assimpScene, MeshRequestPart& part, Engine::priv::MeshImportedData& meshImportedData) 
    : m_GlobalInverseTransform{ Engine::Math::toGLM(assimpScene.mRootNode->mTransformation.Inverse()) }
    , m_BoneOffsets{ assimpMesh.mNumBones, glm::mat4(1.0f) }
{
    //build bone information
    auto hashedBones = Engine::create_and_reserve<Engine::unordered_string_map<std::string, uint16_t>>(assimpMesh.mNumBones);

    for (uint32_t boneIdx = 0; boneIdx != assimpMesh.mNumBones; ++boneIdx) {
        hashedBones.emplace( std::piecewise_construct,  std::forward_as_tuple(assimpMesh.mBones[boneIdx]->mName.C_Str()), std::forward_as_tuple(boneIdx) );
    }


    int32_t boneIndex = 0;
    meshImportedData.m_Bones.resize(meshImportedData.m_Points.size());
    for (size_t nodeIdx = 0; nodeIdx != part.nodesData.m_Nodes.size(); ++nodeIdx) {
        auto node = part.nodesData.m_Nodes[nodeIdx];
        const auto& nodeName = part.nodesNames[nodeIdx];
        if (hashedBones.contains(nodeName)) {
            part.nodesData.m_Nodes[nodeIdx].IsBone = true;
            auto& assimpBone = *assimpMesh.mBones[hashedBones.at(nodeName)];
            setBoneOffsetMatrix(boneIndex, Engine::Math::toGLM(assimpBone.mOffsetMatrix));
            for (unsigned int j = 0; j != assimpBone.mNumWeights; ++j) {
                meshImportedData.addBone(assimpBone.mWeights[j].mVertexId, boneIndex, assimpBone.mWeights[j].mWeight);
            }
            ++boneIndex;
        }
    }

    //build animation information
    if (assimpScene.mAnimations && assimpScene.mNumAnimations > 0) {
        m_AnimationMapping.reserve(assimpScene.mNumAnimations);
        m_AnimationData.reserve(assimpScene.mNumAnimations);
        for (unsigned int animationIdx = 0; animationIdx != assimpScene.mNumAnimations; ++animationIdx) {
            const auto& Ai_Animation = *assimpScene.mAnimations[animationIdx];
            std::string key          = Ai_Animation.mName.C_Str();
            if (key.empty()) {
                key = "Animation " + std::to_string(m_AnimationData.size());
            }
            const auto animIndex = addAnimation(key, Ai_Animation, part);
        }
    }
}