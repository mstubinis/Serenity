#include <serenity/resources/mesh/animation/Skeleton.h>
#include <serenity/resources/mesh/MeshRequest.h>
#include <serenity/resources/mesh/MeshImportedData.h>
#include <serenity/math/Engine_Math.h>

Engine::priv::MeshSkeleton::MeshSkeleton(const aiMesh& assimpMesh, const aiScene& assimpScene, MeshRequest& request, MeshNodeData* nodeData, Engine::priv::MeshImportedData& data) {
    //build general information
    m_GlobalInverseTransform = Engine::Math::assimpToGLMMat4(assimpScene.mRootNode->mTransformation.Inverse());
    
    //build bone information
    Engine::unordered_string_map<std::string, uint16_t> hashedBones;
    m_BoneInfo.resize(assimpMesh.mNumBones);
    hashedBones.reserve(assimpMesh.mNumBones);
    uint16_t BoneIndex = 0;
    for (uint32_t k = 0; k < assimpMesh.mNumBones; ++k) {
        auto boneName      = std::string(assimpMesh.mBones[k]->mName.C_Str());
        auto& assimpBone   = *assimpMesh.mBones[k];
        m_BoneMapping.insert(boneName);
        hashedBones.emplace(boneName, BoneIndex);
        ++BoneIndex;
    }
    int k = 0;
    for (const auto& node : request.m_NodeData.m_Nodes) {
        if (hashedBones.contains(node.Name)) {
            int s = hashedBones.at(node.Name);
            auto& assimpBone = *assimpMesh.mBones[s];
            setBoneOffsetMatrix(k, Engine::Math::assimpToGLMMat4(assimpBone.mOffsetMatrix));
            for (uint32_t j = 0; j < assimpBone.mNumWeights; ++j) {
                data.addBone(assimpBone.mWeights[j].mVertexId, k, assimpBone.mWeights[j].mWeight);
            }
            ++k;
        }
    }

    //build animation information
    if (assimpScene.mAnimations && assimpScene.mNumAnimations > 0) {
        for (auto k = 0U; k < assimpScene.mNumAnimations; ++k) {
            const auto& Ai_Animation = *assimpScene.mAnimations[k];
            std::string key          = Ai_Animation.mName.C_Str();
            if (key.empty()) {
                key = "Animation " + std::to_string(numAnimations());
            }
            addAnimation(key, *nodeData, Ai_Animation, request.m_NodeData);
        }
    }
}