
#include <serenity/resources/mesh/MeshLoading.h>
#include <serenity/resources/mesh/MeshImportedData.h>
#include <serenity/resources/mesh/MeshCollisionFactory.h>
#include <serenity/resources/mesh/VertexData.h>
#include <serenity/resources/mesh/Mesh.h>
#include <serenity/math/Engine_Math.h>

#include <serenity/system/Engine.h>
#include <serenity/resources/Engine_Resources.h>

#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

#include <boost/iostreams/device/mapped_file.hpp>
#include <fstream>
#include <stack>

void Engine::priv::MeshLoader::LoadPopulateGlobalNodes(const aiScene& scene, aiNode* ai_node, MeshRequest& request) {
    struct Parameters final {
        aiNode*  ai_node     = nullptr;
        uint16_t parentIndex = 0;
        Parameters(aiNode* ai_node_, uint16_t parentIndex_)
            : ai_node{ ai_node_ }
            , parentIndex{ parentIndex_ }
        {}
    };

    std::stack<Parameters> stk;
    uint16_t ParentIndex = 0;
    stk.emplace(ai_node, ParentIndex);
    //dfs
    while (!stk.empty()) {
        auto data    = stk.top();
        stk.pop();
        auto& ainode = *data.ai_node;
        auto& engineNode = request.m_NodeData.m_Nodes.emplace_back(ainode);
        request.m_NodeData.m_NodeHeirarchy.emplace_back(data.parentIndex);
        request.m_NodeStrVector.push_back(ainode.mName.C_Str());
        for (uint32_t i = 0; i < ainode.mNumMeshes; ++i) {
            const aiMesh& aimesh  = *scene.mMeshes[ainode.mMeshes[i]];
            MeshRequestPart& part = request.m_Parts.emplace_back();
            part.name             = request.m_FileOrData + " - " + std::string{ aimesh.mName.C_Str() };
            part.handle           = Engine::Resources::addResource<Mesh>();
            part.cpuData.m_File   = request.m_FileOrData;
        }
        ++ParentIndex;
        for (uint32_t i = 0; i < ainode.mNumChildren; ++i) {
            auto* ai_child = ainode.mChildren[i];
            stk.emplace(ai_child, ParentIndex);
        }
    }
    request.m_NodeData.m_NodeTransforms.resize(request.m_NodeData.m_Nodes.size(), glm::mat4{1.0f});
}
void Engine::priv::MeshLoader::LoadProcessNodeData(MeshRequest& request, const aiScene& aiscene, const aiNode& rootAINode) {
    auto& root         = *(aiscene.mRootNode);
    uint32_t partIndex = 0;

    std::stack<const aiNode*> stk; 
    stk.push(&rootAINode);
    //dfs
    while (!stk.empty()) {
        auto& AINode = *stk.top();
        stk.pop();
        for (auto i = 0U; i < AINode.mNumMeshes; ++i) {
            const aiMesh& aimesh = *aiscene.mMeshes[AINode.mMeshes[i]];
            auto& part           = request.m_Parts[partIndex];
            MeshImportedData data{ aimesh };
       
            if (aimesh.mNumBones > 0) {
                part.cpuData.m_Skeleton = NEW MeshSkeleton{ aimesh, aiscene, request, &part.handle.get<Mesh>()->m_CPUData.m_NodeData, data };
            }
            MeshLoader::CalculateTBNAssimp(data);
            MeshLoader::FinalizeData(part.cpuData, data, 0.0005f);
            ++partIndex;
        }
        for (uint32_t i = 0; i < AINode.mNumChildren; ++i) {
            stk.push(AINode.mChildren[i]);
        }
    }
}
void Engine::priv::MeshLoader::FinalizeData(MeshCPUData& cpuData, MeshImportedData& data, float threshold) {
    cpuData.m_Threshold = threshold;
    PublicMesh::FinalizeVertexData(cpuData, data);
    cpuData.internal_calculate_radius();
    cpuData.m_CollisionFactory = (NEW MeshCollisionFactory{ cpuData });
}

bool Engine::priv::MeshLoader::GetSimilarVertexIndex(glm::vec3& in_pos, glm::vec2& in_uv, glm::vec3& in_norm, std::vector<glm::vec3>& pts, std::vector<glm::vec2>& uvs, std::vector<glm::vec3>& norms, uint32_t& result, float thrshld) {
    for (uint32_t t = 0; t < pts.size(); ++t) {
        if (Math::IsNear(in_pos, pts[t], thrshld) &&  Math::IsNear(in_uv, uvs[t], thrshld) &&  Math::IsNear(in_norm, norms[t], thrshld)) {
            result = t;
            return true;
        }
    }
    return false;
}
void Engine::priv::MeshLoader::CalculateTBNAssimp(MeshImportedData& importedData) {
    if (importedData.m_Normals.size() == 0) {
        return;
    }
    const auto pointsSize = importedData.m_Points.size();
    importedData.m_Tangents.reserve(importedData.m_Normals.size());
    importedData.m_Binormals.reserve(importedData.m_Normals.size());
    for (size_t i = 0; i < pointsSize; i += 3) {
        const size_t p0{ i + 0 };
        const size_t p1{ i + 1 };
        const size_t p2{ i + 2 };
        const size_t uvSize{ importedData.m_UVs.size() };

        const glm::vec3 point1 = (pointsSize > p0) ? importedData.m_Points[p0] : glm::vec3(0.0f);
        const glm::vec3 point2 = (pointsSize > p1) ? importedData.m_Points[p1] : glm::vec3(0.0f);
        const glm::vec3 point3 = (pointsSize > p2) ? importedData.m_Points[p2] : glm::vec3(0.0f);

        const glm::vec2 uv1 = (uvSize > p0) ? importedData.m_UVs[p0] : glm::vec2(0.0f);
        const glm::vec2 uv2 = (uvSize > p1) ? importedData.m_UVs[p1] : glm::vec2(0.0f);
        const glm::vec2 uv3 = (uvSize > p2) ? importedData.m_UVs[p2] : glm::vec2(0.0f);

        const glm::vec3 v{ point2 - point1 };
        const glm::vec3 w{ point3 - point1 };

        // texture offset p1->p2 and p1->p3
        float sx{ uv2.x - uv1.x };
        float sy{ uv2.y - uv1.y };
        float tx{ uv3.x - uv1.x };
        float ty{ uv3.y - uv1.y };
        float dirCorrection{ 1.0 };

        if ((tx * sy - ty * sx) < 0.0f) {
            dirCorrection = -1.0f; //this is important for normals and mirrored mesh geometry using identical uv's
        }
        // when t1, t2, t3 in same position in UV space, just use default UV direction.
        //if ( 0 == sx && 0 == sy && 0 == tx && 0 == ty ) {
        if (sx * ty == sy * tx) {
            sx = 0.0; 
            sy = 1.0;
            tx = 1.0; 
            ty = 0.0;
        }
        // tangent points in the direction where to positive X axis of the texture coord's would point in model space
        // bitangent's points along the positive Y axis of the texture coord's, respectively
        glm::vec3 tangent{
            (w.x * sy - v.x * ty) * dirCorrection,
            (w.y * sy - v.y * ty) * dirCorrection,
            (w.z * sy - v.z * ty) * dirCorrection
        };
        glm::vec3 bitangent{
            (w.x * sx - v.x * tx) * dirCorrection,
            (w.y * sx - v.y * tx) * dirCorrection,
            (w.z * sx - v.z * tx) * dirCorrection
        };

        // store for every vertex of that face
        for (auto b = 0; b < 3; ++b) {
            size_t p;
            glm::vec3 normal;
            if (b == 0)      p = p0;
            else if (b == 1) p = p1;
            else             p = p2;

            if (importedData.m_Normals.size() > p)
                normal = importedData.m_Normals[p];
            else                         
                normal = glm::vec3{ 0.0f };

            // project tangent and bitangent into the plane formed by the vertex' normal
            glm::vec3 localTangent{ tangent - normal * (tangent * normal) };
            glm::vec3 localBitangent{ bitangent - normal * (bitangent * normal) };
            localTangent   = glm::normalize(localTangent);
            localBitangent = glm::normalize(localBitangent);

            // reconstruct tangent/bitangent according to normal and bitangent/tangent when it's infinite or NaN.
            const bool invalid_tangent   = Engine::Math::IsSpecialFloat(localTangent);
            const bool invalid_bitangent = Engine::Math::IsSpecialFloat(localBitangent);
            if (invalid_tangent != invalid_bitangent) {
                if (invalid_tangent) 
                    localTangent   = glm::normalize(glm::cross(normal, localBitangent));
                else                 
                    localBitangent = glm::normalize(glm::cross(localTangent, normal));
            }
            importedData.m_Tangents.emplace_back(localTangent);
            importedData.m_Binormals.emplace_back(localBitangent);
        }
    }
    for (size_t i = 0; i < importedData.m_Points.size(); ++i) {
        //hmm.. should b and t be swapped here?
        auto& n = importedData.m_Normals[i];
        auto& b = importedData.m_Tangents[i];
        auto& t = importedData.m_Binormals[i];
        t = glm::normalize(t - n * glm::dot(n, t)); // Gram-Schmidt orthogonalize
    }
};
VertexData* Engine::priv::MeshLoader::LoadFrom_OBJCC(const std::string& filename) {
    boost::iostreams::mapped_file_source stream(filename.c_str());
    //TODO: try possible optimizations

    uint32_t blockStart = 0;
    const uint8_t* streamDataBuffer = (uint8_t*)stream.data();

    uint32_t sizes[3];
    for (uint32_t i = 0; i < 3; ++i) {
        readBigEndian(sizes[i], streamDataBuffer, 4U, blockStart);
    }
    
    const uint32_t sizes_attr     = sizes[0];
    const uint32_t sizes_indices  = sizes[1];
    const uint32_t sizes_skeleton = sizes[2];
    VertexData* vertexData = nullptr;
    if (sizes_skeleton == 1) {
        vertexData = NEW VertexData(VertexDataFormat::VertexDataAnimated);
    }else{
        vertexData = NEW VertexData(VertexDataFormat::VertexDataBasic);
    }
    ASSERT(vertexData != nullptr, "Engine::priv::MeshLoader::LoadFrom_OBJCC() : vertexData was nullptr!");
    vertexData->m_Indices.reserve(sizes_indices);

    auto temp_pos    = Engine::create_and_reserve<std::vector<glm::vec3>>(sizes_attr);
    auto temp_uvs    = Engine::create_and_reserve<std::vector<glm::vec2>>(sizes_attr);
    auto temp_norm   = Engine::create_and_reserve<std::vector<GLuint>>(sizes_attr);
    auto temp_binorm = Engine::create_and_reserve<std::vector<GLuint>>(sizes_attr);
    auto temp_tang   = Engine::create_and_reserve<std::vector<GLuint>>(sizes_attr);
    auto temp_bID    = Engine::create_and_reserve<std::vector<glm::vec4>>(sizes_skeleton == 1 ? sizes_attr : 0);
    auto temp_bW     = Engine::create_and_reserve<std::vector<glm::vec4>>(sizes_skeleton == 1 ? sizes_attr : 0);

    for (auto i = 0U; i < sizes_attr; ++i) {
        //positions stored as half floats

        auto lambda = [&blockStart, &streamDataBuffer]<typename INTYPE, typename T>(std::vector<T>& outContainer, uint32_t blockSize, INTYPE* in_, uint32_t size) {
            std::vector<float> outPos(3, 0.0f);
            for (uint32_t j = 0; j < size; ++j) {
                readBigEndian(in_[j], streamDataBuffer, blockSize, blockStart);
            }
            Engine::Math::Float32From16(outPos.data(), in_, size);
            auto& outVar = outContainer.emplace_back();
            for (uint32_t j = 0; j < size; ++j) {
                outVar[j] = outPos[j];
            }
        };
        uint16_t inPos[3];
        lambda(temp_pos, 2U, inPos, 3); //positions

        uint16_t inUV[2];
        lambda(temp_uvs, 2U, inUV, 2); //uvs

        //normals stored as unsigned int's
        uint32_t inn[3];
        for (auto j = 0; j < 3; ++j) {
            readBigEndian(inn[j], streamDataBuffer, 4U, blockStart);
        }
        temp_norm.emplace_back(inn[0]);
        temp_binorm.emplace_back(inn[1]);
        temp_tang.emplace_back(inn[2]);

        if (sizes_skeleton == 1) { //skeleton is present
            uint16_t  inIDs[4];
            lambda(temp_bID, 2U, inIDs, 4); //bone ids

            uint16_t  inWs[4];
            lambda(temp_bW, 2U, inWs, 4); //bone weights
        }
    }
    //indices
    for (auto i = 0U; i < sizes_indices; ++i) {
        uint16_t      inindice;
        readBigEndian(inindice, streamDataBuffer, 2, blockStart);
        vertexData->m_Indices.emplace_back(inindice);
    }
    vertexData->setData(0, temp_pos.data(), temp_pos.size(), MeshModifyFlags::None);
    vertexData->setData(1, temp_uvs.data(), temp_uvs.size(), MeshModifyFlags::None);
    vertexData->setData(2, temp_norm.data(), temp_norm.size(), MeshModifyFlags::None);
    vertexData->setData(3, temp_binorm.data(), temp_binorm.size(), MeshModifyFlags::None);
    vertexData->setData(4, temp_tang.data(), temp_tang.size(), MeshModifyFlags::None);
    if (temp_bID.size() > 0) {
        vertexData->setData(5, temp_bID.data(), temp_bID.size(), MeshModifyFlags::None);
        vertexData->setData(6, temp_bW.data(), temp_bW.size(), MeshModifyFlags::None);
    }
    vertexData->setIndices(vertexData->m_Indices.data(), vertexData->m_Indices.size(), MeshModifyFlags::RecalculateTriangles);
    return vertexData;
}
void Engine::priv::MeshLoader::SaveTo_OBJCC(VertexData& vertexData, std::string filename) {
    std::ofstream stream(filename, std::ios::binary | std::ios::out);

    //header - should only be 3 entries, one for m_Vertices , one for m_Indices, and one to tell if animation data is present or not
    std::vector<uint32_t>   sizes(3);
    sizes[0] = (uint32_t)vertexData.m_Data[0].m_Size;
    sizes[1] = (uint32_t)vertexData.m_Indices.size();
    if (vertexData.m_Data.size() > 5) { //vertices contain animation data
        sizes[2] = 1;
    }else{
        sizes[2] = 0;
    }
    for (size_t i = 0; i < sizes.size(); ++i) {
        writeBigEndian(stream, sizes[i], 4);
    }
    auto positions   = vertexData.getPositions();
    auto uvs         = vertexData.getData<glm::vec2>(1);
    auto normals     = vertexData.getData<GLuint>(2);
    auto binormals   = vertexData.getData<GLuint>(3);
    auto tangents    = vertexData.getData<GLuint>(4);
    std::vector<glm::vec4>    boneIDs;
    std::vector<glm::vec4>    boneWeights;

    if (sizes[2] == 1) { //animation data is present
        boneIDs     = vertexData.getData<glm::vec4>(5);
        boneWeights = vertexData.getData<glm::vec4>(6);
    }
    for (unsigned int j = 0; j < sizes[0]; ++j) {
        const auto& position   = positions[j];
        const auto& uv         = uvs[j];
        const auto& normal     = normals[j];
        const auto& binormal   = binormals[j];
        const auto& tangent    = tangents[j];
        glm::vec4* boneID      = nullptr;
        glm::vec4* boneWeight  = nullptr;

        //positions
        std::vector<uint16_t> outp(3);
        for (size_t i = 0; i < outp.size(); ++i) {
            Engine::Math::Float16From32(&outp[i], position[(glm::vec3::length_type)i]);
            writeBigEndian(stream, outp[i], 2);
        }
        //uvs
        std::vector<uint16_t> outu(2);
        for (size_t i = 0; i < outu.size(); ++i) {
            Engine::Math::Float16From32(&outu[i], uv[(glm::vec2::length_type)i]);
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
            std::vector<uint16_t> outbI(4);
            for (size_t i = 0; i < outbI.size(); ++i) {
                Engine::Math::Float16From32(&outbI[i], bID[(glm::vec4::length_type)i]);
                writeBigEndian(stream, outbI[i], 2);
            }
            //boneWeight's
            std::vector<uint16_t> outbW(4);
            for (size_t i = 0; i < outbW.size(); ++i) {
                Engine::Math::Float16From32(&outbW[i], bW[(glm::vec4::length_type)i]);
                writeBigEndian(stream, outbW[i], 2);
            }
        }
    }
    //indices
    for (uint32_t i = 0; i < sizes[1]; ++i) {
        uint16_t indice = vertexData.m_Indices[i];
        writeBigEndian(stream, indice, 2);
    }
    stream.close();
}