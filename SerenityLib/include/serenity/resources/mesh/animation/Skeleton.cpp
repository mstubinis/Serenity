#include <serenity/resources/mesh/animation/Skeleton.h>
#include <serenity/resources/mesh/MeshRequest.h>
#include <serenity/resources/mesh/MeshImportedData.h>
#include <serenity/math/Engine_Math.h>

Engine::priv::MeshSkeleton::MeshSkeleton(const aiMesh& assimpMesh, const aiScene& assimpScene, MeshRequest& request, MeshNodeData* nodeData, Engine::priv::MeshImportedData& data) {
    //build general information
    m_GlobalInverseTransform = Engine::Math::assimpToGLMMat4(assimpScene.mRootNode->mTransformation.Inverse());
    
    //build bone information
    auto hashedBones = Engine::create_and_reserve<Engine::unordered_string_map<std::string, uint16_t>>(assimpMesh.mNumBones);
    m_BoneInfo.resize(assimpMesh.mNumBones);
    uint16_t BoneIndex = 0;
    for (uint32_t k = 0; k < assimpMesh.mNumBones; ++k) {
        auto boneName      = std::string{ assimpMesh.mBones[k]->mName.C_Str() };
        auto& assimpBone   = *assimpMesh.mBones[k];
        hashedBones.emplace(boneName, BoneIndex);
        ++BoneIndex;
    }
    int32_t k = 0;
    data.m_Bones.resize(data.m_Points.size());
    for (uint32_t i = 0; i < request.m_NodeData.m_Nodes.size(); ++i) {
        auto& node           = request.m_NodeData.m_Nodes[i];
        const auto& nodeName = request.m_NodeStrVector[i];
        if (hashedBones.contains(nodeName)) {
            node.IsBone = true;
            auto& assimpBone = *assimpMesh.mBones[hashedBones.at(nodeName)];
            setBoneOffsetMatrix(k, Engine::Math::assimpToGLMMat4(assimpBone.mOffsetMatrix));
            for (uint32_t j = 0; j < assimpBone.mNumWeights; ++j) {
                data.addBone(assimpBone.mWeights[j].mVertexId, k, assimpBone.mWeights[j].mWeight);
            }
            ++k;
        }
    }

    //build animation information
    if (assimpScene.mAnimations && assimpScene.mNumAnimations > 0) {
        m_AnimationData.reserve(assimpScene.mNumAnimations);
        for (auto k = 0U; k < assimpScene.mNumAnimations; ++k) {
            const auto& Ai_Animation = *assimpScene.mAnimations[k];
            std::string key          = Ai_Animation.mName.C_Str();
            if (key.empty()) {
                key = "Animation " + std::to_string(numAnimations());
            }
            addAnimation(key, *nodeData, Ai_Animation, request);
        }
    }
}