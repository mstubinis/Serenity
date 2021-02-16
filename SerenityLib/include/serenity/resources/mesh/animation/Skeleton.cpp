#include <serenity/resources/mesh/animation/Skeleton.h>
#include <serenity/resources/mesh/MeshRequest.h>
#include <serenity/resources/mesh/MeshImportedData.h>
#include <serenity/math/Engine_Math.h>

Engine::priv::MeshSkeleton::MeshSkeleton(const aiMesh& assimpMesh, const aiScene& assimpScene, MeshRequest& request, MeshNodeData* nodeData, Engine::priv::MeshImportedData& data) {
    //build general information
    m_GlobalInverseTransform = Engine::Math::assimpToGLMMat4(assimpScene.mRootNode->mTransformation.Inverse());
    
    //build bone information
    m_BoneInfo.resize(assimpMesh.mNumBones);

    for (uint32_t k = 0; k < assimpMesh.mNumBones; ++k) {
        auto boneName      = std::string(assimpMesh.mBones[k]->mName.C_Str());
        auto& assimpBone   = *assimpMesh.mBones[k];
        uint16_t BoneIndex = 0;

        if (!hasBone(boneName)) {
            BoneIndex = numBones();
            m_BoneInfo.emplace_back();
        }else{
            BoneIndex = getBoneIndex(boneName);
        }
        addBoneMapping(boneName, BoneIndex);



        setBoneOffsetMatrix(BoneIndex, Engine::Math::assimpToGLMMat4(assimpBone.mOffsetMatrix));
        for (uint32_t j = 0; j < assimpBone.mNumWeights; ++j) {
            data.addBone(assimpBone.mWeights[j].mVertexId, BoneIndex, assimpBone.mWeights[j].mWeight);
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