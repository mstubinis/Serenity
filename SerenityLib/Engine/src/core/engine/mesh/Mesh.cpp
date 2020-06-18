#include <core/engine/mesh/Mesh.h>
#include <core/engine/mesh/MeshLoading.h>
#include <core/engine/mesh/MeshImportedData.h>
#include <core/engine/mesh/Skeleton.h>
#include <core/engine/mesh/MeshCollisionFactory.h>
#include <core/engine/events/Event.h>
#include <core/engine/system/Engine.h>
#include <core/Terrain.h>

#include <core/engine/resources/Engine_Resources.h>
#include <core/engine/renderer/Renderer.h>
#include <core/engine/physics/Engine_Physics.h>
#include <core/engine/physics/Collision.h>
#include <core/engine/math/Engine_Math.h>
#include <core/engine/scene/Camera.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/norm.hpp>

#include <boost/filesystem.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/math/special_functions/fpclassify.hpp>

#include <btBulletDynamicsCommon.h>
#include <BulletCollision/Gimpact/btGImpactShape.h>
#include <BulletCollision/CollisionShapes/btHeightfieldTerrainShape.h>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

#include <boost/iostreams/device/mapped_file.hpp>
#include <iostream>
#include <fstream>
#include <execution>
#include <iomanip>

using namespace std;
using namespace Engine;
using namespace Engine::priv;
namespace boostm = boost::math;

namespace Engine::priv {
    struct DefaultMeshBindFunctor final { void operator()(Mesh* mesh_ptr, const Engine::priv::Renderer* renderer) const {
        mesh_ptr->m_VertexData->bind();
    }};
    struct DefaultMeshUnbindFunctor final { void operator()(Mesh* mesh_ptr, const Engine::priv::Renderer* renderer) const {
        mesh_ptr->m_VertexData->unbind();
    }};
};

void InternalMeshPublicInterface::LoadGPU(Mesh& mesh){
    mesh.m_VertexData->finalize(); //transfer vertex data to gpu

    mesh.EngineResource::load();

    Event e(EventType::MeshLoaded);
    e.eventMeshLoaded = EventMeshLoaded(&mesh);
    Engine::priv::Core::m_Engine->m_EventModule.m_EventDispatcher.dispatchEvent(e);
}
void InternalMeshPublicInterface::UnloadCPU(Mesh& mesh){
    SAFE_DELETE(mesh.m_Skeleton);
    SAFE_DELETE(mesh.m_CollisionFactory);

    //cleanup the node tree
    if (mesh.m_RootNode) {
        std::vector<Engine::priv::MeshInfoNode*> cleanup_vector;
        std::queue<Engine::priv::MeshInfoNode*> q;
        q.push(mesh.m_RootNode);
        while (!q.empty()) {
            size_t size = q.size();
            while (size > 0) {
                auto* front = q.front();
                cleanup_vector.push_back(front);
                for (const auto& child : front->Children) {
                    q.push(child);
                }
                q.pop();
                --size;
            }
        }
        SAFE_DELETE_VECTOR(cleanup_vector);
    }

    mesh.EngineResource::unload();
}
void InternalMeshPublicInterface::UnloadGPU(Mesh& mesh){
    SAFE_DELETE(mesh.m_VertexData);
}
void InternalMeshPublicInterface::InitBlankMesh(Mesh& mesh) {
    mesh.registerEvent(EventType::WindowFullscreenChanged);
    mesh.setCustomBindFunctor(DefaultMeshBindFunctor());
    mesh.setCustomUnbindFunctor(DefaultMeshUnbindFunctor());
}
bool InternalMeshPublicInterface::SupportsInstancing(){
    return (
        Renderer::OPENGL_VERSION >= 31 || 
        OpenGLExtensions::supported(OpenGLExtensions::EXT_draw_instanced) || 
        OpenGLExtensions::supported(OpenGLExtensions::ARB_draw_instanced)
    );
}
btCollisionShape* InternalMeshPublicInterface::BuildCollision(Mesh* mesh, CollisionType::Type type, bool isCompoundChild) {
    Engine::priv::MeshCollisionFactory* factory = nullptr;
    if (!mesh) {
        factory = Engine::priv::Core::m_Engine->m_Misc.m_BuiltInMeshes.getCubeMesh().m_CollisionFactory;
    }else{
        factory = mesh->m_CollisionFactory;
    }
    if (factory) {
        switch (type) {
            case CollisionType::None: {
                return new btEmptyShape();
            }case CollisionType::Box: {
                return factory->buildBoxShape(nullptr, isCompoundChild);
            }case CollisionType::ConvexHull: {
                return factory->buildConvexHull(nullptr, isCompoundChild);
            }case CollisionType::Sphere: {
                return factory->buildSphereShape(nullptr, isCompoundChild);
            }case CollisionType::TriangleShapeStatic: {
                return factory->buildTriangleShape(nullptr, isCompoundChild);
            }case CollisionType::TriangleShape: {
                return factory->buildTriangleShapeGImpact(nullptr, isCompoundChild);
            }default: {
                return new btEmptyShape();
            }
        }
    }
    return new btEmptyShape();
}
btCollisionShape* InternalMeshPublicInterface::BuildCollision(ModelInstance* modelInstance, CollisionType::Type type, bool isCompoundChild) {
    Mesh* mesh = nullptr;
    if (modelInstance) {
        if (modelInstance->mesh()) {
            mesh = modelInstance->mesh();
        }
    }
    Engine::priv::MeshCollisionFactory* factory = nullptr;
    if (!mesh) {
        factory = Engine::priv::Core::m_Engine->m_Misc.m_BuiltInMeshes.getCubeMesh().m_CollisionFactory;
    }else{
        factory = mesh->m_CollisionFactory;
    }
    if (factory) {
        switch (type) {
            case CollisionType::None: {
                return new btEmptyShape();
            }case CollisionType::Box: {
                return factory->buildBoxShape(modelInstance, isCompoundChild);
            }case CollisionType::ConvexHull: {
                return factory->buildConvexHull(modelInstance, isCompoundChild);
            }case CollisionType::Sphere: {
                return factory->buildSphereShape(modelInstance, isCompoundChild);
            }case CollisionType::TriangleShapeStatic: {
                return factory->buildTriangleShape(modelInstance, isCompoundChild);
            }case CollisionType::TriangleShape: {
                return factory->buildTriangleShapeGImpact(modelInstance, isCompoundChild);
            }default: {
                return new btEmptyShape();
            }
        }
    }
    return new btEmptyShape();
}
void InternalMeshPublicInterface::FinalizeVertexData(Mesh& mesh, MeshImportedData& data) {
    if (data.uvs.size() == 0)         data.uvs.resize(data.points.size());
    if (data.normals.size() == 0)     data.normals.resize(data.points.size());
    if (data.binormals.size() == 0)   data.binormals.resize(data.points.size());
    if (data.tangents.size() == 0)    data.tangents.resize(data.points.size());
    if (!mesh.m_VertexData) {
        if (mesh.m_Skeleton) {
            mesh.m_VertexData = NEW VertexData(VertexDataFormat::VertexDataAnimated);
        }else{
            mesh.m_VertexData = NEW VertexData(VertexDataFormat::VertexDataBasic);
        }
    }
    auto& vertexData = *mesh.m_VertexData;
    vertexData.clearData();
    vector<vector<GLuint>> normals;
    normals.resize(3);
    if (mesh.m_Threshold == 0.0f) {
#pragma region No Threshold
        normals[0].reserve(data.normals.size());
        normals[1].reserve(data.binormals.size());
        normals[2].reserve(data.tangents.size());
        for (size_t i = 0; i < data.normals.size(); ++i) {
            normals[0].push_back(Math::pack3NormalsInto32Int(data.normals[i]));
        }
        for (size_t i = 0; i < data.binormals.size(); ++i) {
            normals[1].push_back(Math::pack3NormalsInto32Int(data.binormals[i]));
        }
        for (size_t i = 0; i < data.tangents.size(); ++i) {
            normals[2].push_back(Math::pack3NormalsInto32Int(data.tangents[i]));
        }
        vertexData.setData(0, data.points.data(), data.points.size());
        vertexData.setData(1, data.uvs.data(), data.uvs.size());
        vertexData.setData(2, normals[0].data(), normals[0].size());
        vertexData.setData(3, normals[1].data(), normals[1].size());
        vertexData.setData(4, normals[2].data(), normals[2].size());
        vertexData.setIndices(data.indices.data(), data.indices.size(), false, false, true);
#pragma endregion
    }else{
#pragma region Some Threshold
        vector<uint32_t>  indices;
        vector<glm::vec3> temp_pos;             temp_pos.reserve(data.points.size());
        vector<glm::vec2> temp_uvs;             temp_uvs.reserve(data.uvs.size());
        vector<glm::vec3> temp_normals;         temp_normals.reserve(data.normals.size());
        vector<glm::vec3> temp_binormals;       temp_binormals.reserve(data.binormals.size());
        vector<glm::vec3> temp_tangents;        temp_tangents.reserve(data.tangents.size());
        vector<glm::vec4> boneIDs;              boneIDs.reserve(data.m_Bones.size());
        vector<glm::vec4> boneWeights;          boneWeights.reserve(data.m_Bones.size());

        for (unsigned int i = 0; i < data.points.size(); ++i) {
            unsigned int index;
            bool found = priv::MeshLoader::GetSimilarVertexIndex(data.points[i], data.uvs[i], data.normals[i], temp_pos, temp_uvs, temp_normals, index, mesh.m_Threshold);
            if (found) {
                indices.emplace_back(index);
                //average out TBN. But it cancels out normal mapping on some flat surfaces
                //temp_binormals[index] += data.binormals[i];
                //temp_tangents[index] += data.tangents[i];
            }else{
                temp_pos.emplace_back(data.points[i]);
                temp_uvs.emplace_back(data.uvs[i]);
                temp_normals.emplace_back(data.normals[i]);
                temp_binormals.emplace_back(data.binormals[i]);
                temp_tangents.emplace_back(data.tangents[i]);

                if (data.m_Bones.size() > 0) {
                    boneIDs.emplace_back(data.m_Bones[i].IDs[0], data.m_Bones[i].IDs[1], data.m_Bones[i].IDs[2], data.m_Bones[i].IDs[3]);
                    boneWeights.emplace_back(data.m_Bones[i].Weights[0], data.m_Bones[i].Weights[1], data.m_Bones[i].Weights[2], data.m_Bones[i].Weights[3]);
                }
                indices.emplace_back(static_cast<uint32_t>(temp_pos.size() - 1));
            }
        }
        normals[0].reserve(temp_normals.size());
        normals[1].reserve(temp_binormals.size());
        normals[2].reserve(temp_tangents.size());
        for (size_t i = 0; i < temp_normals.size(); ++i) {
            normals[0].push_back(Math::pack3NormalsInto32Int(temp_normals[i]));
        }
        for (size_t i = 0; i < temp_binormals.size(); ++i) {
            normals[1].push_back(Math::pack3NormalsInto32Int(temp_binormals[i]));
        }
        for (size_t i = 0; i < temp_tangents.size(); ++i) {
            normals[2].push_back(Math::pack3NormalsInto32Int(temp_tangents[i]));
        }

        vertexData.setData(0, temp_pos.data(), temp_pos.size());
        vertexData.setData(1, temp_uvs.data(), temp_uvs.size());
        vertexData.setData(2, normals[0].data(), normals[0].size());
        vertexData.setData(3, normals[1].data(), normals[1].size());
        vertexData.setData(4, normals[2].data(), normals[2].size());
        if (boneIDs.size() > 0) {
            vertexData.setData(5, boneIDs.data(), boneIDs.size());
            vertexData.setData(6, boneWeights.data(), boneWeights.size());
        }
        vertexData.setIndices(indices.data(), indices.size(), false, false, true);
#pragma endregion
    }
}
void InternalMeshPublicInterface::TriangulateComponentIndices(Mesh& mesh, MeshImportedData& data, std::vector<std::vector<uint>>& indices, unsigned char flags) {
    for (size_t i = 0; i < indices[0].size(); ++i) {
        glm::vec3 pos(0.0f);
        glm::vec2 uv(0.0f);
        glm::vec3 norm(1.0f);
        if (flags && MeshLoadingFlags::Points && data.file_points.size() > 0) {
            pos = data.file_points[indices[0][i] - 1];
            data.points.push_back(pos);
        }
        if (flags && MeshLoadingFlags::UVs && data.file_uvs.size() > 0) {
            uv = data.file_uvs[indices[1][i] - 1];
            data.uvs.push_back(uv);
        }
        if (flags && MeshLoadingFlags::Normals && data.file_normals.size() > 0) {
            norm = data.file_normals[indices[2][i] - 1];
            data.normals.push_back(norm);
        }
    }
}
void InternalMeshPublicInterface::CalculateRadius(Mesh& mesh) {
    mesh.m_radiusBox = glm::vec3(0.0f);
    vector<glm::vec3> points = mesh.m_VertexData->getPositions();
    for (const auto& vertex : points) {
        const float x = abs(vertex.x);
        const float y = abs(vertex.y);
        const float z = abs(vertex.z);
        if (x > mesh.m_radiusBox.x)  mesh.m_radiusBox.x = x;
        if (y > mesh.m_radiusBox.y)  mesh.m_radiusBox.y = y;
        if (z > mesh.m_radiusBox.z)  mesh.m_radiusBox.z = z;
    }
    mesh.m_radius = Math::Max(mesh.m_radiusBox);
}

Mesh::Mesh() : EngineResource(ResourceType::Mesh) {
    InternalMeshPublicInterface::InitBlankMesh(*this);
}

//TERRAIN MESH
void Mesh::internal_build_from_terrain(const Terrain& terrain) {
    MeshImportedData data;

    unsigned int count = 0;
    float offsetSectorX = 0.0f;
    float offsetSectorY = 0.0f;

    auto hash_position = [](glm::vec3& position, unsigned int decimal_places) {
        stringstream one, two, thr;
        one << std::fixed << std::setprecision(decimal_places) << position.x;
        two << std::fixed << std::setprecision(decimal_places) << position.y;
        thr << std::fixed << std::setprecision(decimal_places) << position.z;
        return one.str() + "_" + two.str() + "_" + thr.str();
    };

    unordered_map<string, VertexSmoothingGroup> m_VertexMap;
    auto& heightfields = terrain.m_TerrainData.m_BtHeightfieldShapes;
    for (size_t sectorX = 0; sectorX < heightfields.size(); ++sectorX) {
        for (size_t sectorY = 0; sectorY < heightfields[sectorX].size(); ++sectorY) {
            auto& heightfield = *heightfields[sectorX][sectorY];
            unsigned int width = static_cast<unsigned int>(heightfield.getUserIndex());
            unsigned int length = static_cast<unsigned int>(heightfield.getUserIndex2());
            const float fWidth = static_cast<float>(width);
            const float fLength = static_cast<float>(length);
            offsetSectorX = sectorX * fWidth;
            offsetSectorY = sectorY * fLength;

            for (unsigned int i = 0; i < width; ++i) {
                float fI = static_cast<float>(i);
                for (unsigned int j = 0; j < length; ++j) {
                    float fJ = static_cast<float>(j);
                    btVector3 vert1, vert2, vert3, vert4;

                    bool valid[4];
                    valid[0] = heightfield.getAndValidateVertex(i,     j,     vert1, false);
                    valid[1] = heightfield.getAndValidateVertex(i + 1, j,     vert2, false);
                    valid[2] = heightfield.getAndValidateVertex(i,     j + 1, vert3, false);
                    valid[3] = heightfield.getAndValidateVertex(i + 1, j + 1, vert4, false);

                    priv::Vertex v1, v2, v3, v4;

                    v1.position = glm::vec3(offsetSectorY + vert1.x(), vert1.y(), offsetSectorX + vert1.z());
                    v2.position = glm::vec3(offsetSectorY + vert2.x(), vert2.y(), offsetSectorX + vert2.z());
                    v3.position = glm::vec3(offsetSectorY + vert3.x(), vert3.y(), offsetSectorX + vert3.z());
                    v4.position = glm::vec3(offsetSectorY + vert4.x(), vert4.y(), offsetSectorX + vert4.z());

                    glm::vec3 a = v4.position - v1.position;
                    glm::vec3 b = v2.position - v3.position;
                    glm::vec3 normal = glm::normalize(glm::cross(a, b));

                    v1.normal = normal;
                    v1.uv = glm::vec2(fI / fWidth, fJ / fLength);

                    data.points.push_back(v1.position);
                    data.uvs.push_back(v1.uv);
                    data.normals.push_back(v1.normal);
                        
                    VertexSmoothingData v1s;
                    v1s.normal = v1.normal;
                    v1s.index = data.points.size() - 1;
                    m_VertexMap[hash_position(v1.position, 4)].data.push_back(std::move(v1s));

                    v2.normal = normal;
                    v2.uv = glm::vec2((fI + 1.0f) / fWidth, fJ / fLength);

                    data.points.push_back(v2.position);
                    data.uvs.push_back(v2.uv);
                    data.normals.push_back(v2.normal);
                        
                    VertexSmoothingData v2s;
                    v2s.normal = v2.normal;
                    v2s.index = data.points.size() - 1;
                    m_VertexMap[hash_position(v2.position, 4)].data.push_back(std::move(v2s));

                    v3.normal = normal;
                    v3.uv = glm::vec2(fI / fWidth, (fJ + 1.0f) / fLength);

                    data.points.push_back(v3.position);
                    data.uvs.push_back(v3.uv);
                    data.normals.push_back(v3.normal);
                        
                    VertexSmoothingData v3s;
                    v3s.normal = v3.normal;
                    v3s.index = data.points.size() - 1;
                    m_VertexMap[hash_position(v3.position, 4)].data.push_back(std::move(v3s));

                    v4.normal = normal;
                    v4.uv = glm::vec2((fI + 1.0f) / fWidth, (fJ + 1.0f) / fLength);

                    data.points.push_back(v4.position);
                    data.uvs.push_back(v4.uv);
                    data.normals.push_back(v4.normal);

                    VertexSmoothingData v4s;
                    v4s.normal = v4.normal;
                    v4s.index = data.points.size() - 1;
                    m_VertexMap[hash_position(v4.position, 4)].data.push_back(std::move(v4s));

                    if (valid[0] || valid[1] || valid[2] || valid[3]) {
                        data.indices.push_back(count + 0);
                        data.indices.push_back(count + 2);
                        data.indices.push_back(count + 1);

                        data.indices.push_back(count + 2);
                        data.indices.push_back(count + 3);
                        data.indices.push_back(count + 1);
                    }
                    count += 4;
                }
            }
        }
    }
    //TODO: optimize if possible
    //now smooth the normals
    for (auto& itr : m_VertexMap) {
        auto& smoothed = itr.second.smoothedNormal;
        for (auto& vertex : itr.second.data) {
            smoothed += vertex.normal;
        }
        smoothed = glm::normalize(smoothed);
        for (auto& vertex : itr.second.data) {
            data.normals[vertex.index] = smoothed;
        }
    }
    MeshLoader::CalculateTBNAssimp(data);
    InternalMeshPublicInterface::FinalizeVertexData(*this, data);
    InternalMeshPublicInterface::CalculateRadius(*this);
    SAFE_DELETE(m_CollisionFactory);
}
void Mesh::internal_recalc_indices_from_terrain(const Terrain& terrain) {
    MeshImportedData data;
    unsigned int count = 0;
    auto& heightfields = terrain.m_TerrainData.m_BtHeightfieldShapes;
    for (size_t sectorX = 0; sectorX < heightfields.size(); ++sectorX) {
        for (size_t sectorY = 0; sectorY < heightfields[sectorX].size(); ++sectorY) {
            auto& heightfield = *heightfields[sectorX][sectorY];
            unsigned int width  = static_cast<unsigned int>(heightfield.getUserIndex());
            unsigned int length = static_cast<unsigned int>(heightfield.getUserIndex2());
            for (unsigned int i = 0; i < width; ++i) {
                for (unsigned int j = 0; j < length; ++j) {
                    btVector3 vert1, vert2, vert3, vert4;
                    bool valid[4];
                    valid[0] = heightfield.getAndValidateVertex(i, j,         vert1, false);
                    valid[1] = heightfield.getAndValidateVertex(i + 1, j,     vert2, false);
                    valid[2] = heightfield.getAndValidateVertex(i, j + 1,     vert3, false);
                    valid[3] = heightfield.getAndValidateVertex(i + 1, j + 1, vert4, false);
                    if (valid[0] || valid[1] || valid[2] || valid[3]) {
                        data.indices.push_back(count + 0);
                        data.indices.push_back(count + 2);
                        data.indices.push_back(count + 1);

                        data.indices.push_back(count + 2);
                        data.indices.push_back(count + 3);
                        data.indices.push_back(count + 1);
                    }
                    count += 4;
                }
            }
        }
    }
    m_VertexData->bind();
    modifyIndices(data.indices.data(), data.indices.size(), MeshModifyFlags::Default | MeshModifyFlags::UploadToGPU);
    m_VertexData->unbind();
}
Mesh::Mesh(const string& name, const Terrain& terrain, float threshold) : EngineResource(ResourceType::Mesh) {
    InternalMeshPublicInterface::InitBlankMesh(*this);
    m_Threshold = threshold;
    internal_build_from_terrain(terrain);
    load();
}
Mesh::Mesh(VertexData* data, const string& name, float threshold) : EngineResource(ResourceType::Mesh, name) {
    InternalMeshPublicInterface::InitBlankMesh(*this);
    m_VertexData = data;
    m_Threshold = threshold;
}
Mesh::Mesh(const string& name, float width, float height, float threshold) : EngineResource(ResourceType::Mesh, name){
    InternalMeshPublicInterface::InitBlankMesh(*this);
    m_Threshold = threshold;

    MeshImportedData data;

    vector<priv::Vertex> quad; quad.resize(4);

    quad[0].uv = glm::vec2(0.0f, height);
    quad[1].uv = glm::vec2(width, height);
    quad[2].uv = glm::vec2(width, 0.0f);
    quad[3].uv = glm::vec2(0.0f, 0.0f);

    quad[0].position = glm::vec3(-width / 2.0f, -height / 2.0f, 0.0f);
    quad[1].position = glm::vec3(width / 2.0f, -height / 2.0f, 0.0f);
    quad[2].position = glm::vec3(width / 2.0f, height / 2.0f, 0.0f);
    quad[3].position = glm::vec3(-width / 2.0f, height / 2.0f, 0.0f);

    for (unsigned int i = 0; i < 3; ++i) {   //triangle 1 (0, 1, 2)
        data.points.emplace_back(quad[i].position);
        data.uvs.emplace_back(quad[i].uv);
    }
    for (unsigned int i = 0; i < 3; ++i) {   //triangle 2 (2, 3, 0)
        data.points.emplace_back(quad[(i + 2) % 4].position);
        data.uvs.emplace_back(quad[(i + 2) % 4].uv);
    }
    m_VertexData = NEW VertexData(VertexDataFormat::VertexDataNoLighting);
    MeshLoader::FinalizeData(*this, data, threshold);

    load();
}
Mesh::Mesh(const string& fileOrData, float threshold) : EngineResource(ResourceType::Mesh) {
    InternalMeshPublicInterface::InitBlankMesh(*this);
    m_Threshold = threshold;

    setName("Custom Mesh");
    unsigned char flags = MeshLoadingFlags::Points | MeshLoadingFlags::Faces | MeshLoadingFlags::UVs | MeshLoadingFlags::Normals | MeshLoadingFlags::TBN;

    MeshImportedData data;
    vector<vector<uint>> indices;
    indices.resize(3);
    istringstream stream;
    stream.str(fileOrData);

    //first read in all data
    for (string line; getline(stream, line, '\n');) {
        if (line[0] == 'o') {
        }else if (line[0] == 'v' && line[1] == ' ') {
            if (flags && MeshLoadingFlags::Points) {
                glm::vec3 p;
                auto res = sscanf(line.substr(2, line.size()).c_str(), "%f %f %f", &p.x, &p.y, &p.z);
                data.file_points.push_back(p);
            }
        }else if (line[0] == 'v' && line[1] == 't') {
            if (flags && MeshLoadingFlags::UVs) {
                glm::vec2 uv;
                auto res = sscanf(line.substr(2, line.size()).c_str(), "%f %f", &uv.x, &uv.y);
                uv.y = 1.0f - uv.y;
                data.file_uvs.push_back(uv);
            }
        }else if (line[0] == 'v' && line[1] == 'n') {
            if (flags && MeshLoadingFlags::Normals) {
                glm::vec3 n;
                auto res = sscanf(line.substr(2, line.size()).c_str(), "%f %f %f", &n.x, &n.y, &n.z);
                data.file_normals.push_back(n);
            }
        }else if (line[0] == 'f' && line[1] == ' ') {
            if (flags && MeshLoadingFlags::Faces) {
                glm::uvec3 f1, f2, f3, f4 = glm::uvec3(1);
                int matches = sscanf(line.substr(2, line.size()).c_str(), "%d/%d/%d %d/%d/%d %d/%d/%d %d/%d/%d", &f1.x, &f1.y, &f1.z, &f2.x, &f2.y, &f2.z, &f3.x, &f3.y, &f3.z, &f4.x, &f4.y, &f4.z);
                if (matches < 3) {
                    matches = sscanf(line.substr(2, line.size()).c_str(), "%d %d %d %d", &f1.x, &f2.x, &f3.x, &f4.x);
                }
                f1 = glm::max(f1, glm::uvec3(1)); f2 = glm::max(f2, glm::uvec3(1)); f3 = glm::max(f3, glm::uvec3(1)); f4 = glm::max(f4, glm::uvec3(1));
                if (matches == 3 || matches == 6 || matches == 9) { //triangle
                    indices[0].push_back(f1.x); indices[0].push_back(f2.x); indices[0].push_back(f3.x);
                    indices[1].push_back(f1.y); indices[1].push_back(f2.y); indices[1].push_back(f3.y);
                    indices[2].push_back(f1.z); indices[2].push_back(f2.z); indices[2].push_back(f3.z);
                }else if (matches == 4 || matches == 8 || matches == 12) {//quad
                    indices[0].push_back(f1.x); indices[0].push_back(f2.x); indices[0].push_back(f3.x);
                    indices[1].push_back(f1.y); indices[1].push_back(f2.y); indices[1].push_back(f3.y);
                    indices[2].push_back(f1.z); indices[2].push_back(f2.z); indices[2].push_back(f3.z);

                    indices[0].push_back(f1.x); indices[0].push_back(f3.x); indices[0].push_back(f4.x);
                    indices[1].push_back(f1.y); indices[1].push_back(f3.y); indices[1].push_back(f4.y);
                    indices[2].push_back(f1.z); indices[2].push_back(f3.z); indices[2].push_back(f4.z);
                }
            }
        }
    }
    if (flags && MeshLoadingFlags::Faces) {
        InternalMeshPublicInterface::TriangulateComponentIndices(*this, data, indices, flags);
    }
    if (flags && MeshLoadingFlags::TBN) {
        MeshLoader::CalculateTBNAssimp(data);
    }
    MeshLoader::FinalizeData(*this, data, threshold);

    load();
}
Mesh::~Mesh(){
    unregisterEvent(EventType::WindowFullscreenChanged);
    unload();
}
bool Mesh::operator==(const bool rhs) const {
    if (rhs == true) {
        return (m_VertexData);
    }
    return !(m_VertexData);
}
Mesh::operator bool() const {
    return (m_VertexData);
}

unordered_map<string, AnimationData>& Mesh::animationData(){ 
    return m_Skeleton->m_AnimationData; 
}
const VertexData& Mesh::getVertexData() const { 
    return *m_VertexData; 
}
const glm::vec3& Mesh::getRadiusBox() const { 
    return m_radiusBox; 
}
float Mesh::getRadius() const { 
    return m_radius; 
}
void Mesh::load(){
    if(!isLoaded()){
        InternalMeshPublicInterface::LoadGPU(*this);
        EngineResource::load();
    }
}
void Mesh::unload(){
    if(isLoaded()){
        InternalMeshPublicInterface::UnloadGPU(*this);
        InternalMeshPublicInterface::UnloadCPU(*this);
        EngineResource::unload();
    }
}
void Mesh::onEvent(const Event& e) {
    if (e.type == EventType::WindowFullscreenChanged) {
        m_VertexData->finalize();
    }
}
//TODO: optimize this a bit more (bubble sort?)
void Mesh::sortTriangles(const Camera& camera, ModelInstance& instance, const glm::mat4& bodyModelMatrix, SortingMode::Mode sortMode) {
    #ifndef _DEBUG
        auto& triangles     = m_VertexData->m_Triangles;
        if (triangles.size() == 0) {
            return;
        }
        glm::vec3 camPos    = camera.getPosition();

        auto lambda_sorter = [&](priv::Triangle& lhs, priv::Triangle& rhs) {
            glm::mat4 model1 = instance.modelMatrix() * bodyModelMatrix;
            glm::mat4 model2 = model1;

            model1 = glm::translate(model1, lhs.midpoint);
            model2 = glm::translate(model2, rhs.midpoint);

            glm::vec3 model1Pos = Math::getMatrixPosition(model1);
            glm::vec3 model2Pos = Math::getMatrixPosition(model2);

            if (sortMode == SortingMode::FrontToBack)
                return glm::distance2(camPos, model1Pos) < glm::distance2(camPos, model2Pos);
            else if (sortMode == SortingMode::BackToFront)
                return glm::distance2(camPos, model1Pos) > glm::distance2(camPos, model2Pos);
            else
                return false;
            return false;
        };
        //std::execution::par_unseq seems to really help here for performance
        std::sort( std::execution::par_unseq, triangles.begin(), triangles.end(), lambda_sorter);

        vector<unsigned int> newIndices;
        newIndices.reserve(m_VertexData->m_Indices.size());
        for (size_t i = 0; i < triangles.size(); ++i) {
            auto& triangle = triangles[i];
            newIndices.push_back(triangle.index1);
            newIndices.push_back(triangle.index2);
            newIndices.push_back(triangle.index3);
        }
        Mesh::modifyIndices(newIndices.data(), newIndices.size());
    #endif
}