#include <core/engine/mesh/MeshImpl.h>
#include <core/engine/mesh/VertexData.h>
#include <core/engine/mesh/MeshCollisionFactory.h>
#include <core/engine/mesh/MeshIncludes.h>
#include <core/engine/mesh/MeshImportedData.h>
#include <core/engine/mesh/MeshLoading.h>

#include <core/engine/Engine_Utils.h>
#include <core/engine/Engine_Math.h>

#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

#include <iostream>
#include <map>

using namespace Engine;
using namespace std;

epriv::MeshImpl::MeshImpl() {
    m_File = "";
    m_Skeleton = nullptr;
    m_VertexData = nullptr;
    m_CollisionFactory = nullptr;
    m_threshold = 0.0005f;
}
epriv::MeshImpl::~MeshImpl() {

}

void epriv::MeshImpl::unload_cpu() {
    SAFE_DELETE(m_Skeleton);
    SAFE_DELETE(m_CollisionFactory);
    cout << "(Mesh) ";
}
void epriv::MeshImpl::unload_gpu() {
    SAFE_DELETE(m_VertexData);
}

void epriv::MeshImpl::load_cpu() {
    /*
    if (m_File != "") {
        _loadFromFile(super, m_File, m_threshold);
    }
    _calculateMeshRadius(*m_VertexData);
    m_CollisionFactory = new epriv::MeshCollisionFactory(super, *m_VertexData);
    */
}
void epriv::MeshImpl::load_gpu() {
    m_VertexData->finalize(); //transfer vertex data to gpu
    cout << "(Mesh) ";
}

void epriv::MeshImpl::calculate_radius(VertexData& vertexData) {
    glm::vec3 max = glm::vec3(0.0f);
    const auto& data = vertexData.getData<glm::vec3>(0);
    for (auto& _vertex : data) {
        const float x = abs(_vertex.x);
        const float y = abs(_vertex.y);
        const float z = abs(_vertex.z);
        if (x > max.x)  max.x = x;
        if (y > max.y)  max.y = y;
        if (z > max.z)  max.z = z;
    }
    m_radiusBox = max;
    m_radius = Math::Max(m_radiusBox);
}


void epriv::MeshImpl::triangulate_component_indices(epriv::MeshImportedData& data, vector<uint>& point_indices, vector<uint>& uv_indices, vector<uint>& normal_indices, unsigned char _flags) {
    uint count = 0;
    epriv::Triangle triangle;
    for (uint i = 0; i < point_indices.size(); ++i) {
        glm::vec3 pos(0.0f);
        glm::vec2 uv(0.0f);
        glm::vec3 norm(1.0f);
        if (_flags && epriv::LOAD_POINTS && data.file_points.size() > 0) {
            pos = data.file_points[point_indices[i] - 1];
            data.points.push_back(pos);
        }
        if (_flags && epriv::LOAD_UVS && data.file_uvs.size() > 0) {
            uv = data.file_uvs[uv_indices[i] - 1];
            data.uvs.push_back(uv);
        }
        if (_flags && epriv::LOAD_NORMALS && data.file_normals.size() > 0) {
            norm = data.file_normals[normal_indices[i] - 1];
            data.normals.push_back(norm);
        }
        //data.indices.emplace_back((ushort)count);
        ++count;
        epriv::Vertex* _vertex = &triangle.v1;
        if (count == 2) {
            _vertex = &triangle.v2;
        }else if (count == 3) {
            _vertex = &triangle.v3;
        }
        _vertex->position = pos;
        _vertex->uv       = uv;
        _vertex->normal   = norm;
        if (count == 3) {
            data.file_triangles.push_back(triangle);
            count = 0;
        }
    }
}


void epriv::MeshImpl::finalize_vertex_data(epriv::MeshImportedData& data) {
    if (data.uvs.size() == 0)         data.uvs.resize(data.points.size());
    if (data.normals.size() == 0)     data.normals.resize(data.points.size());
    if (data.binormals.size() == 0)   data.binormals.resize(data.points.size());
    if (data.tangents.size() == 0)    data.tangents.resize(data.points.size());
    if (!m_VertexData) {
        if (m_Skeleton) {
            m_VertexData = new VertexData(VertexDataFormat::VertexDataAnimated);
        }else{
            m_VertexData = new VertexData(VertexDataFormat::VertexDataBasic);
        }
    }
    auto& vertexData = *m_VertexData;
    vector<vector<GLuint>> normals;
    normals.resize(3);
    if (m_threshold == 0.0f) {
        #pragma region No Threshold
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
        #pragma endregion
    }else{
        #pragma region Some Threshold
        vector<ushort> _indices;
        vector<glm::vec3> temp_pos; temp_pos.reserve(data.points.size());
        vector<glm::vec2> temp_uvs; temp_uvs.reserve(data.uvs.size());
        vector<glm::vec3> temp_normals; temp_normals.reserve(data.normals.size());
        vector<glm::vec3> temp_binormals; temp_binormals.reserve(data.binormals.size());
        vector<glm::vec3> temp_tangents; temp_tangents.reserve(data.tangents.size());
        for (uint i = 0; i < data.points.size(); ++i) {
            ushort index;
            bool found = epriv::MeshLoader::GetSimilarVertexIndex(data.points[i], data.uvs[i], data.normals[i], temp_pos, temp_uvs, temp_normals, index, m_threshold);
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
        #pragma endregion
    }
    if (m_Skeleton) {
        vector<vector<glm::vec4>> boneStuff;
        boneStuff.resize(2);
        auto& _skeleton = *m_Skeleton;
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