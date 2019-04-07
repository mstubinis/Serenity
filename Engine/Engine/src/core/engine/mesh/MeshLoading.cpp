#include <core/engine/mesh/MeshLoading.h>
#include <core/engine/mesh/MeshImportedData.h>
#include <core/engine/mesh/VertexData.h>
#include <core/engine/mesh/Mesh.h>
#include <core/engine/Engine_Math.h>

#include <boost/math/special_functions/fpclassify.hpp>

#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

#include <boost/filesystem.hpp>
#include <boost/iostreams/device/mapped_file.hpp>
#include <iostream>
#include <fstream>

using namespace Engine;
using namespace std;
namespace boostm = boost::math;

void epriv::MeshLoader::LoadInternal(MeshSkeleton* skeleton, MeshImportedData& data, const string& file) {
    Assimp::Importer importer;
    const aiScene* AssimpScene = importer.ReadFile(file, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
    if (!AssimpScene || AssimpScene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !AssimpScene->mRootNode) {
        return;
    }
    //animation stuff
    aiMatrix4x4 m = AssimpScene->mRootNode->mTransformation; // node->mTransformation?
    m.Inverse();
    unordered_map<string, BoneNode*> map;
    if (AssimpScene->mAnimations && AssimpScene->mNumAnimations > 0 && !skeleton) {
        skeleton = new MeshSkeleton();
        skeleton->m_GlobalInverseTransform = Math::assimpToGLMMat4(m);
    }
    MeshLoader::LoadProcessNode(skeleton, data, *AssimpScene, *AssimpScene->mRootNode, map);
    if (skeleton) {
        skeleton->fill(data);
    }
}

void epriv::MeshLoader::LoadPopulateGlobalNodes(const aiNode& node, BoneNodeMap& _map) {
    if (!_map.count(node.mName.data)) {
        BoneNode* bone_node = new BoneNode();
        bone_node->Name = node.mName.data;
        bone_node->Transform = Math::assimpToGLMMat4(const_cast<aiMatrix4x4&>(node.mTransformation));
        _map.emplace(node.mName.data, bone_node);
    }
    for (uint i = 0; i < node.mNumChildren; ++i) {
        MeshLoader::LoadPopulateGlobalNodes(*node.mChildren[i], _map);
    }
}

void epriv::MeshLoader::LoadProcessNode(MeshSkeleton* _skeleton, MeshImportedData& data, const aiScene& scene, const aiNode& node, BoneNodeMap& _map) {
    auto& root = *(scene.mRootNode);

    if (_skeleton && &node == &root) {
        MeshLoader::LoadPopulateGlobalNodes(root, _map);
    }
    for (uint i = 0; i < node.mNumMeshes; ++i) {
        const aiMesh& aimesh = *scene.mMeshes[node.mMeshes[i]];
        #pragma region vertices

        data.points.reserve(aimesh.mNumVertices);
        if (aimesh.mTextureCoords[0]) { data.uvs.reserve(aimesh.mNumVertices); }
        if (aimesh.mNormals) {          data.normals.reserve(aimesh.mNumVertices); }
        if (aimesh.mTangents) {         data.tangents.reserve(aimesh.mNumVertices); }
        if (aimesh.mBitangents) {       data.binormals.reserve(aimesh.mNumVertices); }
        for (uint j = 0; j < aimesh.mNumVertices; ++j) {
            //pos
            auto& pos = aimesh.mVertices[j];
            data.points.emplace_back(pos.x, pos.y, pos.z);
            //uv
            if (aimesh.mTextureCoords[0]) {
                //this is to prevent uv compression from beign f-ed up at the poles.
                auto& uv = aimesh.mTextureCoords[0][j];
                //if(uv.y <= 0.0001f){ uv.y = 0.001f; }
                //if(uv.y >= 0.9999f){ uv.y = 0.999f; }
                data.uvs.emplace_back(uv.x, uv.y);
            }else{
                data.uvs.emplace_back(0.0f, 0.0f);
            }
            if (aimesh.mNormals) {
                auto& norm = aimesh.mNormals[j];
                data.normals.emplace_back(norm.x, norm.y, norm.z);
            }
            if (aimesh.mTangents) {
                auto& tang = aimesh.mTangents[j];
                //data.tangents.emplace_back(tang.x,tang.y,tang.z);
            }
            if (aimesh.mBitangents) {
                auto& binorm = aimesh.mBitangents[j];
                //data.binormals.emplace_back(binorm.x,binorm.y,binorm.z);
            }
        }
        #pragma endregion
        // Process indices
        #pragma region indices
        data.indices.reserve(aimesh.mNumFaces * 3);
        for (uint j = 0; j < aimesh.mNumFaces; ++j) {
            const auto& face = aimesh.mFaces[j];

            const auto& index0 = face.mIndices[0];
            const auto& index1 = face.mIndices[1];
            const auto& index2 = face.mIndices[2];

            data.indices.emplace_back(index0);
            data.indices.emplace_back(index1);
            data.indices.emplace_back(index2);
        }
        #pragma endregion
        //bones
        #pragma region Skeleton
        if (aimesh.mNumBones > 0) {
            auto& skeleton = *_skeleton;
            #pragma region IndividualBones
            //build bone information
            for (uint k = 0; k < aimesh.mNumBones; ++k) {
                auto& boneNode = *(_map.at(aimesh.mBones[k]->mName.data));
                auto& assimpBone = *aimesh.mBones[k];
                uint BoneIndex(0);
                if (!skeleton.m_BoneMapping.count(boneNode.Name)) {
                    BoneIndex = skeleton.m_NumBones;
                    ++skeleton.m_NumBones;
                    skeleton.m_BoneInfo.emplace_back();
                }else{
                    BoneIndex = skeleton.m_BoneMapping.at(boneNode.Name);
                }
                skeleton.m_BoneMapping.emplace(boneNode.Name, BoneIndex);
                skeleton.m_BoneInfo[BoneIndex].BoneOffset = Math::assimpToGLMMat4(assimpBone.mOffsetMatrix);
                for (uint j = 0; j < assimpBone.mNumWeights; ++j) {
                    uint VertexID = assimpBone.mWeights[j].mVertexId;
                    float Weight = assimpBone.mWeights[j].mWeight;
                    epriv::VertexBoneData d;
                    d.AddBoneData(BoneIndex, Weight);
                    data.m_Bones.emplace(VertexID, d);
                }
            }
            //build skeleton parent child relationship
            for (auto& node : _map) {
                const auto& assimpNode = root.FindNode(node.first.c_str());
                auto iter = assimpNode;
                while (iter != 0 && iter->mParent != 0) {
                    if (_map.count(iter->mParent->mName.data)) {
                        node.second->Parent = _map.at(iter->mParent->mName.data);
                        node.second->Parent->Children.push_back(node.second);
                        break; //might need to double check this
                    }
                    iter = iter->mParent;
                }
            }
            //and finalize the root node
            if (!skeleton.m_RootNode) {
                for (auto& node : _map) {
                    if (!node.second->Parent) {
                        skeleton.m_RootNode = node.second;
                        break;
                    }
                }
            }
            #pragma endregion

            #pragma region Animations
            if (scene.mAnimations && scene.mNumAnimations > 0) {
                for (uint k = 0; k < scene.mNumAnimations; ++k) {
                    const aiAnimation& anim = *scene.mAnimations[k];
                    string key(anim.mName.C_Str());
                    if (key == "") {
                        key = "Animation " + to_string(skeleton.m_AnimationData.size());
                    }
                    if (!skeleton.m_AnimationData.count(key)) {
                        skeleton.m_AnimationData.emplace(
                            std::piecewise_construct,
                            std::forward_as_tuple(key),
                            std::forward_as_tuple(skeleton, anim)
                        );
                    }
                }
            }
            #pragma endregion
        }
        #pragma endregion
        MeshLoader::CalculateTBNAssimp(data);
    }
    for (uint i = 0; i < node.mNumChildren; ++i) {
        MeshLoader::LoadProcessNode(_skeleton, data, scene, *node.mChildren[i], _map);
    }
}

void epriv::MeshLoader::LoadProcessNode(vector<MeshRequestPart>& _parts, const aiScene& scene, const aiNode& node, BoneNodeMap& _map) {
    auto& root = *(scene.mRootNode);

    for (uint i = 0; i < node.mNumMeshes; ++i) {
        const aiMesh& aimesh = *scene.mMeshes[node.mMeshes[i]];


        MeshRequestPart part = MeshRequestPart();

        epriv::MeshImportedData data;

        #pragma region vertices
        if (aimesh.mVertices) { data.points.reserve(aimesh.mNumVertices); }
        if (aimesh.mTextureCoords[0]) { data.uvs.reserve(aimesh.mNumVertices); }
        if (aimesh.mNormals) { data.normals.reserve(aimesh.mNumVertices); }
        if (aimesh.mBitangents) { data.binormals.reserve(aimesh.mNumVertices); }
        if (aimesh.mTangents) { data.tangents.reserve(aimesh.mNumVertices); }
        for (uint j = 0; j < aimesh.mNumVertices; ++j) {
            //pos
            auto& pos = aimesh.mVertices[j];
            data.points.emplace_back(pos.x, pos.y, pos.z);
            //uv
            if (aimesh.mTextureCoords[0]) {
                //this is to prevent uv compression from beign f-ed up at the poles.
                auto& uv = aimesh.mTextureCoords[0][j];
                //if(uv.y <= 0.0001f){ uv.y = 0.001f; }
                //if(uv.y >= 0.9999f){ uv.y = 0.999f; }
                data.uvs.emplace_back(uv.x, uv.y);
            }else{
                data.uvs.emplace_back(0.0f, 0.0f);
            }
            if (aimesh.mNormals) {
                auto& norm = aimesh.mNormals[j];
                data.normals.emplace_back(norm.x, norm.y, norm.z);
            }
            if (aimesh.mTangents) {
                auto& tang = aimesh.mTangents[j];
                //data.tangents.emplace_back(tang.x,tang.y,tang.z);
            }
            if (aimesh.mBitangents) {
                auto& binorm = aimesh.mBitangents[j];
                //data.binormals.emplace_back(binorm.x,binorm.y,binorm.z);
            }
        }
        #pragma endregion

        #pragma region indices
        data.indices.reserve(aimesh.mNumFaces * 3);
        for (uint j = 0; j < aimesh.mNumFaces; ++j) {
            const auto& face = aimesh.mFaces[j];

            const auto& index0 = face.mIndices[0];
            const auto& index1 = face.mIndices[1];
            const auto& index2 = face.mIndices[2];

            data.indices.emplace_back(index0);
            data.indices.emplace_back(index1);
            data.indices.emplace_back(index2);
        }
        #pragma endregion

        #pragma region Skeleton
        if (aimesh.mNumBones > 0) {
            part.mesh->m_Skeleton = new epriv::MeshSkeleton();
            auto& skeleton = *part.mesh->m_Skeleton;

            #pragma region IndividualBones
            //build bone information
            for (uint k = 0; k < aimesh.mNumBones; ++k) {
                auto& boneNode = *(_map.at(aimesh.mBones[k]->mName.data));
                auto& assimpBone = *aimesh.mBones[k];
                uint BoneIndex(0);
                if (!skeleton.m_BoneMapping.count(boneNode.Name)) {
                    BoneIndex = skeleton.m_NumBones;
                    ++skeleton.m_NumBones;
                    skeleton.m_BoneInfo.emplace_back();
                }else{
                    BoneIndex = skeleton.m_BoneMapping.at(boneNode.Name);
                }
                skeleton.m_BoneMapping.emplace(boneNode.Name, BoneIndex);
                skeleton.m_BoneInfo[BoneIndex].BoneOffset = Math::assimpToGLMMat4(assimpBone.mOffsetMatrix);
                for (uint j = 0; j < assimpBone.mNumWeights; ++j) {
                    uint VertexID = assimpBone.mWeights[j].mVertexId;
                    float Weight = assimpBone.mWeights[j].mWeight;
                    epriv::VertexBoneData d;
                    d.AddBoneData(BoneIndex, Weight);
                    data.m_Bones.emplace(VertexID, d);
                }
            }
            //build skeleton parent child relationship
            for (auto& node : _map) {
                const auto& assimpNode = root.FindNode(node.first.c_str());
                auto iter = assimpNode;
                while (iter != 0 && iter->mParent != 0) {
                    if (_map.count(iter->mParent->mName.data)) {
                        node.second->Parent = _map.at(iter->mParent->mName.data);
                        node.second->Parent->Children.push_back(node.second);
                        break; //might need to double check this
                    }
                    iter = iter->mParent;
                }
            }
            //and finalize the root node
            if (!skeleton.m_RootNode) {
                for (auto& node : _map) {
                    if (!node.second->Parent) {
                        skeleton.m_RootNode = node.second;
                        break;
                    }
                }
            }
            #pragma endregion

            #pragma region Animations
            if (scene.mAnimations && scene.mNumAnimations > 0) {
                for (uint k = 0; k < scene.mNumAnimations; ++k) {
                    const aiAnimation& anim = *scene.mAnimations[k];
                    string key(anim.mName.C_Str());
                    if (key == "") {
                        key = "Animation " + to_string(skeleton.m_AnimationData.size());
                    }
                    if (!skeleton.m_AnimationData.count(key)) {
                        skeleton.m_AnimationData.emplace(std::piecewise_construct,std::forward_as_tuple(key),std::forward_as_tuple(skeleton, anim));
                    }
                }
            }
            #pragma endregion

            //TODO: remove "fill" function from MeshSkeleton
            for (auto& _b : data.m_Bones) {
                const VertexBoneData& b = _b.second;
                skeleton.m_BoneIDs.push_back(glm::vec4(b.IDs[0], b.IDs[1], b.IDs[2], b.IDs[3]));
                skeleton.m_BoneWeights.push_back(glm::vec4(b.Weights[0], b.Weights[1], b.Weights[2], b.Weights[3]));
            }
        }
        #pragma endregion
        MeshLoader::CalculateTBNAssimp(data);

        part.name = aimesh.mName.C_Str();

        Mesh* mesh = new Mesh(data, part.name);
        part.mesh = mesh;
        //TODO: add more here
        
        _parts.push_back(part);
    }
    for (uint i = 0; i < node.mNumChildren; ++i) {
        MeshLoader::LoadProcessNode(_parts, scene, *node.mChildren[i], _map);
    }
}


void epriv::MeshLoader::FinalizeData(Mesh& mesh,epriv::MeshImportedData& data, float threshold) {
    mesh.m_threshold = threshold;

    if (data.uvs.size() == 0)         data.uvs.resize(data.points.size());
    if (data.normals.size() == 0)     data.normals.resize(data.points.size());
    if (data.binormals.size() == 0)   data.binormals.resize(data.points.size());
    if (data.tangents.size() == 0)    data.tangents.resize(data.points.size());
    if (!mesh.m_VertexData) {
        if (mesh.m_Skeleton) {
            mesh.m_VertexData = new VertexData(VertexDataFormat::VertexDataAnimated);
        }else{
            mesh.m_VertexData = new VertexData(VertexDataFormat::VertexDataBasic);
        }
    }
    auto& vertexData = *mesh.m_VertexData;
    vector<vector<GLuint>> normals;
    normals.resize(3);
    if (mesh.m_threshold == 0.0f) {
        normals[0].reserve(data.normals.size());
        normals[1].reserve(data.binormals.size());
        normals[2].reserve(data.tangents.size());
        for (size_t i = 0; i < data.normals.size(); ++i)
            normals[0].push_back(Math::pack3NormalsInto32Int(data.normals[i]));
        for (size_t i = 0; i < data.binormals.size(); ++i)
            normals[1].push_back(Math::pack3NormalsInto32Int(data.binormals[i]));
        for (size_t i = 0; i < data.tangents.size(); ++i)
            normals[2].push_back(Math::pack3NormalsInto32Int(data.tangents[i]));
        vertexData.setData(0, data.points);
        vertexData.setData(1, data.uvs);
        vertexData.setData(2, normals[0]);
        vertexData.setData(3, normals[1]);
        vertexData.setData(4, normals[2]);
        vertexData.setDataIndices(data.indices);
    }else{
        vector<ushort> _indices;
        vector<glm::vec3> temp_pos; temp_pos.reserve(data.points.size());
        vector<glm::vec2> temp_uvs; temp_uvs.reserve(data.uvs.size());
        vector<glm::vec3> temp_normals; temp_normals.reserve(data.normals.size());
        vector<glm::vec3> temp_binormals; temp_binormals.reserve(data.binormals.size());
        vector<glm::vec3> temp_tangents; temp_tangents.reserve(data.tangents.size());
        for (uint i = 0; i < data.points.size(); ++i) {
            ushort index;
            bool found = epriv::MeshLoader::GetSimilarVertexIndex(data.points[i], data.uvs[i], data.normals[i], temp_pos, temp_uvs, temp_normals, index, mesh.m_threshold);
            if (found) {
                _indices.emplace_back(index);
                //average out TBN. But it cancels out normal mapping on some flat surfaces
                //temp_binormals[index] += data.binormals[i];
                //temp_tangents[index] += data.tangents[i];
            }else{
                temp_pos.emplace_back(data.points[i]);
                temp_uvs.emplace_back(data.uvs[i]);
                temp_normals.emplace_back(data.normals[i]);
                temp_binormals.emplace_back(data.binormals[i]);
                temp_tangents.emplace_back(data.tangents[i]);
                _indices.emplace_back((ushort)temp_pos.size() - 1);
            }
        }
        normals[0].reserve(temp_normals.size());
        normals[1].reserve(temp_binormals.size());
        normals[2].reserve(temp_tangents.size());
        for (size_t i = 0; i < temp_normals.size(); ++i)
            normals[0].push_back(Math::pack3NormalsInto32Int(temp_normals[i]));
        for (size_t i = 0; i < temp_binormals.size(); ++i)
            normals[1].push_back(Math::pack3NormalsInto32Int(temp_binormals[i]));
        for (size_t i = 0; i < temp_tangents.size(); ++i)
            normals[2].push_back(Math::pack3NormalsInto32Int(temp_tangents[i]));
        vertexData.setData(0, temp_pos);
        vertexData.setData(1, temp_uvs);
        vertexData.setData(2, normals[0]);
        vertexData.setData(3, normals[1]);
        vertexData.setData(4, normals[2]);
        vertexData.setDataIndices(_indices);
    }
    if (mesh.m_Skeleton) {
        vector<vector<glm::vec4>> boneStuff;
        boneStuff.resize(2);
        auto& _skeleton = *mesh.m_Skeleton;
        boneStuff[0].reserve(_skeleton.m_BoneIDs.size());
        boneStuff[1].reserve(_skeleton.m_BoneIDs.size());
        for (uint i = 0; i < _skeleton.m_BoneIDs.size(); ++i) {
            boneStuff[0].push_back(_skeleton.m_BoneIDs[i]);
            boneStuff[1].push_back(_skeleton.m_BoneWeights[i]);
        }
        vertexData.setData(5, boneStuff[0]);
        vertexData.setData(6, boneStuff[1]);
    }
}



bool epriv::MeshLoader::IsNear(float& v1, float& v2, const float& threshold) {
    return std::abs(v1 - v2) < threshold;
}
bool epriv::MeshLoader::IsNear(glm::vec2& v1, glm::vec2& v2, const float& threshold) {
    return (std::abs(v1.x - v2.x) < threshold && std::abs(v1.y - v2.y) < threshold) ? true : false;
}
bool epriv::MeshLoader::IsNear(glm::vec3& v1, glm::vec3& v2, const float& threshold) {
    return (std::abs(v1.x - v2.x) < threshold && std::abs(v1.y - v2.y) < threshold && std::abs(v1.z - v2.z) < threshold) ? true : false;
}
bool epriv::MeshLoader::IsSpecialFloat(float& f) {
    if (boostm::isnan(f) || boostm::isinf(f)) return true;
    return false;
}
bool epriv::MeshLoader::IsSpecialFloat(glm::vec2& v) {
    if (boostm::isnan(v.x) || boostm::isnan(v.y)) return true;
    if (boostm::isinf(v.x) || boostm::isinf(v.y)) return true;
    return false;
}
bool epriv::MeshLoader::IsSpecialFloat(glm::vec3& v) {
    if (boostm::isnan(v.x) || boostm::isnan(v.y) || boostm::isnan(v.z)) return true;
    if (boostm::isinf(v.x) || boostm::isinf(v.y) || boostm::isinf(v.z)) return true;
    return false;
}
bool epriv::MeshLoader::GetSimilarVertexIndex(glm::vec3& in_pos, glm::vec2& in_uv, glm::vec3& in_norm, vector<glm::vec3>& pts, vector<glm::vec2>& uvs, vector<glm::vec3>& norms, unsigned short& result, const float& threshold) {
    for (uint t = 0; t < pts.size(); ++t) {
        if (IsNear(in_pos, pts[t], threshold) && IsNear(in_uv, uvs[t], threshold) && IsNear(in_norm, norms[t], threshold)) {
            result = t;
            return true;
        }
    }
    return false;
}
void epriv::MeshLoader::CalculateTBNAssimp(MeshImportedData& data) {
    if (data.normals.size() == 0) return;
    uint pointsSize(data.points.size());

    data.tangents.reserve(data.normals.size());
    data.binormals.reserve(data.normals.size());
    for (uint i = 0; i < pointsSize; i += 3) {
        uint p0(i + 0);
        uint p1(i + 1);
        uint p2(i + 2);

        glm::vec3 point1, point2, point3;
        glm::vec2 uv1, uv2, uv3;

        uint uvSize(data.uvs.size());

        if (pointsSize > p0) point1 = data.points[p0];
        else                 point1 = glm::vec3(0.0f);
        if (pointsSize > p1) point2 = data.points[p1];
        else                 point2 = glm::vec3(0.0f);
        if (pointsSize > p2) point3 = data.points[p2];
        else                 point3 = glm::vec3(0.0f);

        if (uvSize > p0)     uv1   = data.uvs[p0];
        else                 uv1   = glm::vec2(0.0f);
        if (uvSize > p1)     uv2   = data.uvs[p1];
        else                 uv2   = glm::vec2(0.0f);
        if (uvSize > p2)     uv3   = data.uvs[p2];
        else                 uv3   = glm::vec2(0.0f);

        glm::vec3 v(point2 - point1);
        glm::vec3 w(point3 - point1);

        // texture offset p1->p2 and p1->p3
        float sx(uv2.x - uv1.x);
        float sy(uv2.y - uv1.y);
        float tx(uv3.x - uv1.x);
        float ty(uv3.y - uv1.y);
        float dirCorrection = 1.0;

        if ((tx * sy - ty * sx) < 0.0f) dirCorrection = -1.0f; //this is important for normals and mirrored mesh geometry using identical uv's

        // when t1, t2, t3 in same position in UV space, just use default UV direction.
        //if ( 0 == sx && 0 == sy && 0 == tx && 0 == ty ) {
        if (sx * ty == sy * tx) {
            sx = 0.0; sy = 1.0;
            tx = 1.0; ty = 0.0;
        }
        // tangent points in the direction where to positive X axis of the texture coord's would point in model space
        // bitangent's points along the positive Y axis of the texture coord's, respectively
        glm::vec3 tangent, bitangent;
        tangent.x   = (w.x * sy - v.x * ty) * dirCorrection;
        tangent.y   = (w.y * sy - v.y * ty) * dirCorrection;
        tangent.z   = (w.z * sy - v.z * ty) * dirCorrection;
        bitangent.x = (w.x * sx - v.x * tx) * dirCorrection;
        bitangent.y = (w.y * sx - v.y * tx) * dirCorrection;
        bitangent.z = (w.z * sx - v.z * tx) * dirCorrection;

        // store for every vertex of that face
        for (uint b = 0; b < 3; ++b) {
            uint p;
            glm::vec3 normal;
            if (b == 0)      p = p0;
            else if (b == 1) p = p1;
            else             p = p2;

            if (data.normals.size() > p) normal = data.normals[p];
            else                         normal = glm::vec3(0.0f);

            // project tangent and bitangent into the plane formed by the vertex' normal
            glm::vec3 localTangent(tangent - normal * (tangent   * normal));
            glm::vec3 localBitangent(bitangent - normal * (bitangent * normal));
            localTangent   = glm::normalize(localTangent);
            localBitangent = glm::normalize(localBitangent);

            // reconstruct tangent/bitangent according to normal and bitangent/tangent when it's infinite or NaN.
            bool invalid_tangent   = IsSpecialFloat(localTangent);
            bool invalid_bitangent = IsSpecialFloat(localBitangent);
            if (invalid_tangent != invalid_bitangent) {
                if (invalid_tangent) localTangent   = glm::normalize(glm::cross(normal, localBitangent));
                else                 localBitangent = glm::normalize(glm::cross(localTangent, normal));
            }
            data.tangents.push_back(localTangent);
            data.binormals.push_back(localBitangent);
        }
    }
    for (uint i = 0; i < data.points.size(); ++i) {
        //hmm.. should b and t be swapped here?
        auto& n = data.normals[i];
        auto& b = data.tangents[i];
        auto& t = data.binormals[i];
        t = glm::normalize(t - n * glm::dot(n, t)); // Gram-Schmidt orthogonalize
    }
};
VertexData* epriv::MeshLoader::LoadFrom_OBJCC(string& filename) {
    boost::iostreams::mapped_file_source stream(filename.c_str());
    //TODO: try possible optimizations

    uint blockStart = 0;
    const uint8_t* _data = (uint8_t*)stream.data();

    uint32_t sizes[3];
    for (uint i = 0; i < 3; ++i) {
        sizes[i] = (uint32_t)_data[blockStart] << 24;
        sizes[i] |= (uint32_t)_data[blockStart + 1] << 16;
        sizes[i] |= (uint32_t)_data[blockStart + 2] << 8;
        sizes[i] |= (uint32_t)_data[blockStart + 3];
        blockStart += 4;
    }
    VertexData* returnData;
    if (sizes[2] == 1) {
        returnData = new VertexData(VertexDataFormat::VertexDataAnimated);
    }else{
        returnData = new VertexData(VertexDataFormat::VertexDataBasic);
    }
    auto& data = *returnData;
    data.indices.reserve(sizes[1]);
    vector<glm::vec3> temp_pos;     temp_pos.reserve(sizes[0]);
    vector<glm::vec2> temp_uvs;     temp_uvs.reserve(sizes[0]);
    vector<GLuint>    temp_norm;    temp_norm.reserve(sizes[0]);
    vector<GLuint>    temp_binorm;  temp_binorm.reserve(sizes[0]);
    vector<GLuint>    temp_tang;    temp_tang.reserve(sizes[0]);
    vector<glm::vec4> temp_bID;     temp_bID.reserve(sizes[0]);
    vector<glm::vec4> temp_bW;      temp_bW.reserve(sizes[0]);

    for (uint i = 0; i < sizes[0]; ++i) {
        //positions
        float    outPos[3];
        uint16_t inPos[3];
        for (uint j = 0; j < 3; ++j) {
            inPos[j]    = (uint32_t)_data[blockStart] << 8;
            inPos[j]   |= (uint32_t)_data[blockStart + 1];
            blockStart += 2;
        }
        Math::Float32From16(outPos, inPos, 3);
        temp_pos.emplace_back(outPos[0], outPos[1], outPos[2]);
        //uvs
        float    outUV[2];
        uint16_t inUV[2];
        for (uint j = 0; j < 2; ++j) {
            inUV[j]     = (uint32_t)_data[blockStart] << 8;
            inUV[j]    |= (uint32_t)_data[blockStart + 1];
            blockStart += 2;
        }
        Math::Float32From16(outUV, inUV, 2);
        temp_uvs.emplace_back(outUV[0], outUV[1]);
        //normals (remember they are GLuints right now)
        uint32_t inn[3];
        for (uint j = 0; j < 3; ++j) {
            inn[j]      = (uint32_t)_data[blockStart    ] << 24;
            inn[j]     |= (uint32_t)_data[blockStart + 1] << 16;
            inn[j]     |= (uint32_t)_data[blockStart + 2] << 8;
            inn[j]     |= (uint32_t)_data[blockStart + 3];
            blockStart += 4;
        }
        temp_norm.emplace_back(inn[0]);
        temp_binorm.emplace_back(inn[1]);
        temp_tang.emplace_back(inn[2]);
        if (sizes[2] == 1) { //skeleton is present
            //boneID's
            float    outBI[4];
            uint16_t inbI[4];
            for (uint k = 0; k < 4; ++k) {
                inbI[k]     = (uint32_t)_data[blockStart] << 8;
                inbI[k]    |= (uint32_t)_data[blockStart + 1];
                blockStart += 2;
            }
            Math::Float32From16(outBI, inbI, 4);
            temp_bID.emplace_back(outBI[0], outBI[1], outBI[2], outBI[3]);
            //boneWeight's
            float    outBW[4];
            uint16_t inBW[4];
            for (uint k = 0; k < 4; ++k) {
                inBW[k]     = (uint32_t)_data[blockStart] << 8;
                inBW[k]    |= (uint32_t)_data[blockStart + 1];
                blockStart += 2;
            }
            Math::Float32From16(outBW, inBW, 4);
            temp_bW.emplace_back(outBW[0], outBW[1], outBW[2], outBW[3]);
        }
    }
    //indices
    for (uint i = 0; i < sizes[1]; ++i) {
        uint16_t      inindices;
        inindices   = (uint32_t)_data[blockStart    ] << 8;
        inindices  |= (uint32_t)_data[blockStart + 1];
        blockStart += 2;
        data.indices.emplace_back((uint16_t)inindices);
    }
    data.setData(0, temp_pos);
    data.setData(1, temp_uvs);
    data.setData(2, temp_norm);
    data.setData(3, temp_binorm);
    data.setData(4, temp_tang);
    if (temp_bID.size() > 0) {
        data.setData(5, temp_bID);
        data.setData(6, temp_bW);
    }
    data.setDataIndices(data.indices);
    return returnData;
}

void epriv::MeshLoader::SaveTo_OBJCC(VertexData& data, string filename) {
    ofstream stream(filename, ios::binary);

    vector<vector<uint>> _indices;
    _indices.resize(3);

    //header - should only be 3 entries, one for m_Vertices , one for m_Indices, and one to tell if animation data is present or not
    uint32_t   sizes[3];
    sizes[0] = data.dataSizes[0];
    sizes[1] = data.indices.size();
    if (data.data.size() > 5) { //vertices contain animation data
        sizes[2] = 1;
    }else{
        sizes[2] = 0;
    }
    for (uint i = 0; i < 3; ++i) {
        writeUint32tBigEndian(sizes[i], stream);
    }
    const auto& positions   = data.getData<glm::vec3>(0);
    const auto& uvs         = data.getData<glm::vec2>(1);
    const auto& normals     = data.getData<GLuint>(2);
    const auto& binormals   = data.getData<GLuint>(3);
    const auto& tangents    = data.getData<GLuint>(4);
    vector<glm::vec4>         boneIDs;
    vector<glm::vec4>         boneWeights;

    if (sizes[2] == 1) { //animation data is present
        boneIDs     = data.getData<glm::vec4>(5);
        boneWeights = data.getData<glm::vec4>(6);
    }
    for (size_t j = 0; j < sizes[0]; ++j) {
        const auto& position   = positions[j];
        const auto& uv         = uvs[j];
        const auto& normal     = normals[j];
        const auto& binormal   = binormals[j];
        const auto& tangent    = tangents[j];
        glm::vec4* boneID      = nullptr;
        glm::vec4* boneWeight  = nullptr;

        //positions
        uint16_t outp[3];
        Math::Float16From32(&outp[0], position.x);
        Math::Float16From32(&outp[1], position.y);
        Math::Float16From32(&outp[2], position.z);
        for (uint i = 0; i < 3; ++i) {
            writeUint16tBigEndian(outp[i], stream);
        }
        //uvs
        uint16_t outu[2];
        Math::Float16From32(&outu[0], uv.x);
        Math::Float16From32(&outu[1], uv.y);
        for (uint i = 0; i < 2; ++i) {
            writeUint16tBigEndian(outu[i], stream);
        }
        //normals (remember they are GLuints right now)
        uint32_t outn[3];
        outn[0] = normal;
        outn[1] = binormal;
        outn[2] = tangent;
        for (uint i = 0; i < 3; ++i) {
            writeUint32tBigEndian(outn[i], stream);
        }
        if (sizes[2] == 1) { //animation data is present
            boneID = &(boneIDs[j]);
            boneWeight = &(boneWeights[j]);
            auto& bID = *boneID;
            auto& bW = *boneWeight;

            //boneID's
            uint16_t outbI[4];
            Math::Float16From32(&outbI[0], bID.x);
            Math::Float16From32(&outbI[1], bID.y);
            Math::Float16From32(&outbI[2], bID.z);
            Math::Float16From32(&outbI[3], bID.w);
            for (uint i = 0; i < 4; ++i) {
                writeUint16tBigEndian(outbI[i], stream);
            }
            //boneWeight's
            uint16_t outbW[4];
            Math::Float16From32(&outbW[0], bW.x);
            Math::Float16From32(&outbW[1], bW.y);
            Math::Float16From32(&outbW[2], bW.z);
            Math::Float16From32(&outbW[3], bW.w);
            for (uint i = 0; i < 4; ++i) {
                writeUint16tBigEndian(outbW[i], stream);
            }
        }
    }
    //indices
    for (uint i = 0; i < sizes[1]; ++i) {
        uint16_t _ind = data.indices[i];
        writeUint16tBigEndian(_ind, stream);
    }
    stream.close();
}
