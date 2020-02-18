#include <core/engine/mesh/Skeleton.h>
#include <core/engine/mesh/AnimationIncludes.h>
#include <core/engine/mesh/MeshImportedData.h>
#include <core/engine/math/Engine_Math.h>

using namespace Engine::priv;
using namespace std;

MeshSkeleton::MeshSkeleton() {
    m_RootNode = nullptr;
    clear();
}
MeshSkeleton::~MeshSkeleton() {

}
const unsigned int& MeshSkeleton::numBones() const {
    return m_NumBones; 
}
//void MeshSkeleton::fill(const MeshImportedData& meshImportedData) {
//    for (auto& vertexBoneData : meshImportedData.m_Bones) {
//        const VertexBoneData& b = vertexBoneData.second;
//        m_BoneIDs.push_back(glm::vec4(b.IDs[0], b.IDs[1], b.IDs[2], b.IDs[3]));
//        m_BoneWeights.push_back(glm::vec4(b.Weights[0], b.Weights[1], b.Weights[2], b.Weights[3]));
//    }
//}
void MeshSkeleton::populateCleanupMap(BoneNode* boneNode, unordered_map<string, BoneNode*>& boneNodeMap) {
    if (!boneNodeMap.count(boneNode->Name)) {
        boneNodeMap.emplace(boneNode->Name, boneNode);
    }
    for (auto& child : boneNode->Children) {
        populateCleanupMap(child, boneNodeMap);
    }
}
void MeshSkeleton::cleanup() {
    unordered_map<string, BoneNode*> nodes;
    populateCleanupMap(m_RootNode, nodes);
    SAFE_DELETE_MAP(nodes);
}
void MeshSkeleton::clear() {
    m_NumBones = 0;
    m_BoneMapping.clear();
}