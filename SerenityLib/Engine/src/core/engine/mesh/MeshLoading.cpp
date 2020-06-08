#include <core/engine/mesh/MeshLoading.h>
#include <core/engine/mesh/MeshImportedData.h>
#include <core/engine/mesh/MeshCollisionFactory.h>
#include <core/engine/mesh/VertexData.h>
#include <core/engine/mesh/Mesh.h>
#include <core/engine/math/Engine_Math.h>

#include <core/engine/system/Engine.h>
#include <core/engine/resources/Engine_Resources.h>

#include <boost/math/special_functions/fpclassify.hpp>

#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

#include <boost/filesystem.hpp>
#include <boost/iostreams/device/mapped_file.hpp>
#include <iostream>
#include <fstream>
#include <memory_resource>

using namespace Engine;
using namespace std;

void priv::MeshLoader::LoadPopulateGlobalNodes(const aiNode& node, BoneNodeMap& boneNodeMap) {
    if (!boneNodeMap.count(node.mName.data)) {
        boneNodeMap.emplace( std::piecewise_construct, std::forward_as_tuple(node.mName.data), std::forward_as_tuple() );
        auto& newNode      = boneNodeMap.at(node.mName.data);
        newNode.Name       = node.mName.data;
        newNode.Transform  = Math::assimpToGLMMat4(node.mTransformation);
    }
    for (unsigned int i = 0; i < node.mNumChildren; ++i) {
        MeshLoader::LoadPopulateGlobalNodes(*node.mChildren[i], boneNodeMap);
    }
}

void priv::MeshLoader::LoadProcessNodeNames(const string& file, vector<MeshRequestPart>& meshRequestParts, const aiScene& scene, const aiNode& node, BoneNodeMap& boneNodeMap) {
    //just find names and meshes
    auto& root = *(scene.mRootNode);
    for (unsigned int i = 0; i < node.mNumMeshes; ++i) {
        const aiMesh& aimesh = *scene.mMeshes[node.mMeshes[i]];

        MeshRequestPart part;
        part.mesh   = NEW Mesh();
        part.name   = file + " - " + string(aimesh.mName.C_Str());
        part.mesh->setName(part.name);
        part.handle = priv::Core::m_Engine->m_ResourceManager.m_Resources.add(part.mesh, ResourceType::Mesh);

        meshRequestParts.push_back(std::move(part));
    }
    for (unsigned int i = 0; i < node.mNumChildren; ++i) {
        MeshLoader::LoadProcessNodeNames(file, meshRequestParts, scene, *node.mChildren[i], boneNodeMap);
    }
}
void priv::MeshLoader::LoadProcessNodeData(vector<MeshRequestPart>& meshRequestParts, const aiScene& scene, const aiNode& node, BoneNodeMap& boneNodeMap, uint& count) {
    auto& root = *(scene.mRootNode);

    for (unsigned int i = 0; i < node.mNumMeshes; ++i) {
        const aiMesh& aimesh = *scene.mMeshes[node.mMeshes[i]];

        auto& part = meshRequestParts[count];
        MeshImportedData data;

        #pragma region vertices
        if (aimesh.mVertices) { data.points.reserve(aimesh.mNumVertices); }
        if (aimesh.mTextureCoords[0]) { data.uvs.reserve(aimesh.mNumVertices); }
        if (aimesh.mNormals) { data.normals.reserve(aimesh.mNumVertices); }
        if (aimesh.mBitangents) { data.binormals.reserve(aimesh.mNumVertices); }
        if (aimesh.mTangents) { data.tangents.reserve(aimesh.mNumVertices); }
        for (unsigned int j = 0; j < aimesh.mNumVertices; ++j) {
            //pos
            auto& pos = aimesh.mVertices[j];
            data.points.emplace_back(pos.x, pos.y, pos.z);
            //uv
            if (aimesh.mTextureCoords[0]) {
                auto& uv = aimesh.mTextureCoords[0][j];
                //this is to fix uv compression errors near the poles.
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
        for (unsigned int j = 0; j < aimesh.mNumFaces; ++j) {
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
            part.mesh->m_Skeleton = NEW MeshSkeleton();
            auto& skeleton = *part.mesh->m_Skeleton;

            #pragma region IndividualBones
            //build bone information
            for (unsigned int k = 0; k < aimesh.mNumBones; ++k) {
                auto& boneNode   = boneNodeMap.at(aimesh.mBones[k]->mName.data);
                auto& assimpBone = *aimesh.mBones[k];
                unsigned int BoneIndex(0);
                if (!skeleton.m_BoneMapping.count(boneNode.Name)) {
                    BoneIndex = skeleton.m_NumBones;
                    ++skeleton.m_NumBones;
                    skeleton.m_BoneInfo.emplace_back();
                }else{
                    BoneIndex = skeleton.m_BoneMapping.at(boneNode.Name);
                }
                skeleton.m_BoneMapping.emplace(boneNode.Name, BoneIndex);
                skeleton.m_BoneInfo[BoneIndex].BoneOffset = Math::assimpToGLMMat4(assimpBone.mOffsetMatrix);
                for (unsigned int j = 0; j < assimpBone.mNumWeights; ++j) {
                    unsigned int VertexID = assimpBone.mWeights[j].mVertexId;
                    float Weight          = assimpBone.mWeights[j].mWeight;
                    priv::VertexBoneData d;
                    d.AddBoneData(BoneIndex, Weight);
                    data.m_Bones.emplace(VertexID, std::move(d));
                }
            }
            //build skeleton parent child relationship
            for (auto& node : boneNodeMap) {
                const auto& assimpNode = root.FindNode(node.first.c_str());
                auto iter = assimpNode;
                while (iter != 0 && iter->mParent != 0) {
                    if (boneNodeMap.count(iter->mParent->mName.data)) {
                        node.second.Parent = &boneNodeMap.at(iter->mParent->mName.data);
                        node.second.Parent->Children.push_back(&node.second);
                        break; //might need to double check this
                    }
                    iter = iter->mParent;
                }
            }
            //and finalize the root node
            if (!skeleton.m_RootNode) {
                for (auto& node : boneNodeMap) {
                    if (!node.second.Parent) {
                        skeleton.m_RootNode = &node.second;
                        break;
                    }
                }
            }
            #pragma endregion

            #pragma region Animations
            if (scene.mAnimations && scene.mNumAnimations > 0) {
                for (unsigned int k = 0; k < scene.mNumAnimations; ++k) {
                    const aiAnimation& anim = *scene.mAnimations[k];
                    string key(anim.mName.C_Str());
                    if (key.empty()) {
                        key = "Animation " + to_string(skeleton.m_AnimationData.size());
                    }
                    if (!skeleton.m_AnimationData.count(key)) {
                        skeleton.m_AnimationData.emplace(std::piecewise_construct, std::forward_as_tuple(key), std::forward_as_tuple(skeleton, anim));
                    }
                }
            }
            #pragma endregion
            for (auto& _b : data.m_Bones) {
                const VertexBoneData& b = _b.second;
                skeleton.m_BoneIDs.push_back(glm::vec4(b.IDs[0], b.IDs[1], b.IDs[2], b.IDs[3]));
                skeleton.m_BoneWeights.push_back(glm::vec4(b.Weights[0], b.Weights[1], b.Weights[2], b.Weights[3]));
            }
        }
        #pragma endregion
        MeshLoader::CalculateTBNAssimp(data);
        MeshLoader::FinalizeData(*part.mesh, data, 0.0005f);
        ++count;
    }
    for (unsigned int i = 0; i < node.mNumChildren; ++i) {
        MeshLoader::LoadProcessNodeData(meshRequestParts, scene, *node.mChildren[i], boneNodeMap, count);
    }
}

void priv::MeshLoader::FinalizeData(Mesh& mesh, priv::MeshImportedData& data, float threshold) {
    mesh.m_Threshold = threshold;
    InternalMeshPublicInterface::FinalizeVertexData(mesh, data);
    InternalMeshPublicInterface::CalculateRadius(mesh);
    SAFE_DELETE(mesh.m_CollisionFactory);
    mesh.m_CollisionFactory = NEW MeshCollisionFactory(mesh);
}

bool priv::MeshLoader::IsNear(float v1, float v2, float threshold) {
    return (std::abs(v1 - v2) < threshold);
}
bool priv::MeshLoader::IsNear(glm::vec2& v1, glm::vec2& v2, float threshold) {
    return (std::abs(v1.x - v2.x) < threshold && std::abs(v1.y - v2.y) < threshold);
}
bool priv::MeshLoader::IsNear(glm::vec3& v1, glm::vec3& v2, float threshold) {
    return (std::abs(v1.x - v2.x) < threshold && std::abs(v1.y - v2.y) < threshold && std::abs(v1.z - v2.z) < threshold);
}
bool priv::MeshLoader::IsSpecialFloat(float f) {
    if (boost::math::isnan(f)) return true;
    if (boost::math::isinf(f)) return true;
    return false;
}
bool priv::MeshLoader::IsSpecialFloat(const glm::vec2& v) {
    if (boost::math::isnan(v.x) || boost::math::isnan(v.y)) return true;
    if (boost::math::isinf(v.x) || boost::math::isinf(v.y)) return true;
    return false;
}
bool priv::MeshLoader::IsSpecialFloat(const glm::vec3& v) {
    if (boost::math::isnan(v.x) || boost::math::isnan(v.y) || boost::math::isnan(v.z)) return true;
    if (boost::math::isinf(v.x) || boost::math::isinf(v.y) || boost::math::isinf(v.z)) return true;
    return false;
}
bool priv::MeshLoader::GetSimilarVertexIndex(glm::vec3& in_pos, glm::vec2& in_uv, glm::vec3& in_norm, vector<glm::vec3>& pts, vector<glm::vec2>& uvs, vector<glm::vec3>& norms, unsigned int& result, float threshold) {
    for (size_t t = 0; t < pts.size(); ++t) {
        if (IsNear(in_pos, pts[t], threshold) && IsNear(in_uv, uvs[t], threshold) && IsNear(in_norm, norms[t], threshold)) {
            result = static_cast<unsigned int>(t);
            return true;
        }
    }
    return false;
}
void priv::MeshLoader::CalculateTBNAssimp(MeshImportedData& importedData) {
    if (importedData.normals.size() == 0) {
        return;
    }
    const auto pointsSize(importedData.points.size());
    importedData.tangents.reserve(importedData.normals.size());
    importedData.binormals.reserve(importedData.normals.size());
    for (size_t i = 0; i < pointsSize; i += 3) {
        const size_t p0(i + 0);
        const size_t p1(i + 1);
        const size_t p2(i + 2);

        glm::vec3 point1, point2, point3;
        glm::vec2 uv1, uv2, uv3;

        const auto uvSize(importedData.uvs.size());

        point1 = (pointsSize > p0) ? importedData.points[p0] : glm::vec3(0.0f);
        point2 = (pointsSize > p1) ? importedData.points[p1] : glm::vec3(0.0f);
        point3 = (pointsSize > p2) ? importedData.points[p2] : glm::vec3(0.0f);

        uv1 = (uvSize > p0) ? importedData.uvs[p0] : glm::vec2(0.0f);
        uv2 = (uvSize > p1) ? importedData.uvs[p1] : glm::vec2(0.0f);
        uv3 = (uvSize > p2) ? importedData.uvs[p2] : glm::vec2(0.0f);

        glm::vec3 v(point2 - point1);
        glm::vec3 w(point3 - point1);

        // texture offset p1->p2 and p1->p3
        float sx(uv2.x - uv1.x);
        float sy(uv2.y - uv1.y);
        float tx(uv3.x - uv1.x);
        float ty(uv3.y - uv1.y);
        float dirCorrection = 1.0;

        if ((tx * sy - ty * sx) < 0.0f) 
            dirCorrection = -1.0f; //this is important for normals and mirrored mesh geometry using identical uv's

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
        for (size_t b = 0; b < 3; ++b) {
            size_t p;
            glm::vec3 normal;
            if (b == 0)      p = p0;
            else if (b == 1) p = p1;
            else             p = p2;

            if (importedData.normals.size() > p)
                normal = importedData.normals[p];
            else                         
                normal = glm::vec3(0.0f);

            // project tangent and bitangent into the plane formed by the vertex' normal
            glm::vec3 localTangent(tangent - normal * (tangent * normal));
            glm::vec3 localBitangent(bitangent - normal * (bitangent * normal));
            localTangent   = glm::normalize(localTangent);
            localBitangent = glm::normalize(localBitangent);

            // reconstruct tangent/bitangent according to normal and bitangent/tangent when it's infinite or NaN.
            const bool invalid_tangent   = IsSpecialFloat(localTangent);
            const bool invalid_bitangent = IsSpecialFloat(localBitangent);
            if (invalid_tangent != invalid_bitangent) {
                if (invalid_tangent) localTangent   = glm::normalize(glm::cross(normal, localBitangent));
                else                 localBitangent = glm::normalize(glm::cross(localTangent, normal));
            }
            importedData.tangents.push_back(localTangent);
            importedData.binormals.push_back(localBitangent);
        }
    }
    for (size_t i = 0; i < importedData.points.size(); ++i) {
        //hmm.. should b and t be swapped here?
        auto& n = importedData.normals[i];
        auto& b = importedData.tangents[i];
        auto& t = importedData.binormals[i];
        t = glm::normalize(t - n * glm::dot(n, t)); // Gram-Schmidt orthogonalize
    }
};
VertexData* priv::MeshLoader::LoadFrom_OBJCC(string& filename) {
    VertexData* vertexData = nullptr;
    boost::iostreams::mapped_file_source stream(filename.c_str());
    //TODO: try possible optimizations

    uint32_t blockStart = 0;
    const uint8_t* streamDataBuffer = (uint8_t*)stream.data();

    uint32_t sizes[3];
    for (auto i = 0; i < 3; ++i) {
        readBigEndian(sizes[i], streamDataBuffer, 4U, blockStart);
    }
    
    const auto& sizes_attr     = sizes[0];
    const auto& sizes_indices  = sizes[1];
    const auto& sizes_skeleton = sizes[2];
    if (sizes_skeleton == 1) {
        vertexData = NEW VertexData(VertexDataFormat::VertexDataAnimated);
    }else{
        vertexData = NEW VertexData(VertexDataFormat::VertexDataBasic);
    }
    vertexData->m_Indices.reserve(sizes_indices);

    vector<glm::vec3> temp_pos;
    vector<glm::vec2> temp_uvs;
    vector<GLuint>    temp_norm;
    vector<GLuint>    temp_binorm;
    vector<GLuint>    temp_tang;
    vector<glm::vec4> temp_bID;
    vector<glm::vec4> temp_bW;

    temp_pos.reserve(sizes_attr);
    temp_uvs.reserve(sizes_attr);
    temp_norm.reserve(sizes_attr);
    temp_binorm.reserve(sizes_attr);
    temp_tang.reserve(sizes_attr);
    if (sizes_skeleton == 1) { //skeleton is present
        temp_bID.reserve(sizes_attr);
        temp_bW.reserve(sizes_attr);
    }
    for (auto i = 0U; i < sizes_attr; ++i) {
        //positions stored as half floats
        float    outPos[3];
        uint16_t inPos[3];
        for (auto j = 0; j < 3; ++j) {
            readBigEndian(inPos[j], streamDataBuffer, 2U, blockStart);
        }
        Math::Float32From16(outPos, inPos, 3);
        temp_pos.emplace_back(outPos[0], outPos[1], outPos[2]);
        //uvs stored as half floats
        float    outUV[2];
        uint16_t inUV[2];
        for (auto j = 0; j < 2; ++j) {
            readBigEndian(inUV[j], streamDataBuffer, 2U, blockStart);
        }
        Math::Float32From16(outUV, inUV, 2);
        temp_uvs.emplace_back(outUV[0], outUV[1]);
        //normals stored as unsigned int's
        uint32_t inn[3];
        for (auto j = 0; j < 3; ++j) {
            readBigEndian(inn[j], streamDataBuffer, 4U, blockStart);
        }
        temp_norm.emplace_back(inn[0]);
        temp_binorm.emplace_back(inn[1]);
        temp_tang.emplace_back(inn[2]);
        if (sizes_skeleton == 1) { //skeleton is present
            //boneID's
            float    outBI[4];
            uint16_t inbI[4];
            for (auto k = 0; k < 4; ++k) {
                readBigEndian(inbI[k], streamDataBuffer, 2U, blockStart);
            }
            Math::Float32From16(outBI, inbI, 4);
            temp_bID.emplace_back(outBI[0], outBI[1], outBI[2], outBI[3]);
            //boneWeight's
            float    outBW[4];
            uint16_t inBW[4];
            for (auto k = 0; k < 4; ++k) {
                readBigEndian(inBW[k], streamDataBuffer, 2U, blockStart);
            }
            Math::Float32From16(outBW, inBW, 4);
            temp_bW.emplace_back(outBW[0], outBW[1], outBW[2], outBW[3]);
        }
    }
    //indices
    for (auto i = 0U; i < sizes_indices; ++i) {
        uint16_t      inindice;
        readBigEndian(inindice, streamDataBuffer, 2, blockStart);
        vertexData->m_Indices.push_back(inindice);
    }
    vertexData->setData(0, temp_pos.data(), temp_pos.size());
    vertexData->setData(1, temp_uvs.data(), temp_uvs.size());
    vertexData->setData(2, temp_norm.data(), temp_norm.size());
    vertexData->setData(3, temp_binorm.data(), temp_binorm.size());
    vertexData->setData(4, temp_tang.data(), temp_tang.size());
    if (temp_bID.size() > 0) {
        vertexData->setData(5, temp_bID.data(), temp_bID.size());
        vertexData->setData(6, temp_bW.data(), temp_bW.size());
    }
    vertexData->setIndices(vertexData->m_Indices.data(), vertexData->m_Indices.size(), false, false, true);
    return vertexData;
}
void priv::MeshLoader::SaveTo_OBJCC(VertexData& vertexData, string filename) {
    std::ofstream stream(filename, ios::binary | ios::out);

    //header - should only be 3 entries, one for m_Vertices , one for m_Indices, and one to tell if animation data is present or not
    vector<uint32_t>   sizes(3);
    sizes[0] = static_cast<uint32_t>(vertexData.m_DataSizes[0]);
    sizes[1] = static_cast<uint32_t>(vertexData.m_Indices.size());
    if (vertexData.m_Data.size() > 5) { //vertices contain animation data
        sizes[2] = 1;
    }else{
        sizes[2] = 0;
    }
    for (auto i = 0; i < sizes.size(); ++i) {
        writeBigEndian(stream, sizes[i], 4);
    }
    const auto& positions   = vertexData.getData<glm::vec3>(0);
    const auto& uvs         = vertexData.getData<glm::vec2>(1);
    const auto& normals     = vertexData.getData<GLuint>(2);
    const auto& binormals   = vertexData.getData<GLuint>(3);
    const auto& tangents    = vertexData.getData<GLuint>(4);
    vector<glm::vec4>         boneIDs;
    vector<glm::vec4>         boneWeights;

    if (sizes[2] == 1) { //animation data is present
        boneIDs     = vertexData.getData<glm::vec4>(5);
        boneWeights = vertexData.getData<glm::vec4>(6);
    }
    for (auto j = 0; j < sizes[0]; ++j) {
        const auto& position   = positions[j];
        const auto& uv         = uvs[j];
        const auto& normal     = normals[j];
        const auto& binormal   = binormals[j];
        const auto& tangent    = tangents[j];
        glm::vec4* boneID      = nullptr;
        glm::vec4* boneWeight  = nullptr;

        //positions
        vector<uint16_t> outp(3);
        for (auto i = 0; i < outp.size(); ++i) {
            Math::Float16From32(&outp[i], position[i]);
            writeBigEndian(stream, outp[i], 2);
        }
        //uvs
        vector<uint16_t> outu(2);
        for (auto i = 0; i < outu.size(); ++i) {
            Math::Float16From32(&outu[i], uv[i]);
            writeBigEndian(stream, outu[i], 2);
        }
        //normals (remember they are GLuints right now)
        uint32_t outn[3];
        outn[0] = normal;
        outn[1] = binormal;
        outn[2] = tangent;
        for (auto i = 0; i < 3; ++i) {
            writeBigEndian(stream, outn[i], 4);
        }
        if (sizes[2] == 1) { //animation data is present
            boneID          = &(boneIDs[j]);
            boneWeight      = &(boneWeights[j]);
            const auto& bID = *boneID;
            const auto& bW  = *boneWeight;

            //boneID's
            vector<uint16_t> outbI(4);
            for (auto i = 0; i < outbI.size(); ++i) {
                Math::Float16From32(&outbI[i], bID[i]);
                writeBigEndian(stream, outbI[i], 2);
            }
            //boneWeight's
            vector<uint16_t> outbW(4);
            for (auto i = 0; i < outbW.size(); ++i) {
                Math::Float16From32(&outbW[i], bW[i]);
                writeBigEndian(stream, outbW[i], 2);
            }
        }
    }
    //indices
    for (auto i = 0; i < sizes[1]; ++i) {
        uint16_t indice = vertexData.m_Indices[i];
        writeBigEndian(stream, indice, 2);
    }
    stream.close();
}