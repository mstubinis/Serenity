
#include <serenity/resources/mesh/Mesh.h>
#include <serenity/resources/mesh/MeshLoading.h>
#include <serenity/resources/mesh/MeshImportedData.h>
#include <serenity/resources/mesh/animation/Skeleton.h>
#include <serenity/resources/mesh/MeshCollisionFactory.h>
#include <serenity/events/Event.h>
#include <serenity/system/Engine.h>

#include <serenity/Terrain.h>

#include <serenity/math/Engine_Math.h>
#include <serenity/math/MathCompression.h>
#include <serenity/scene/Camera.h>

#include <boost/math/special_functions/fpclassify.hpp>

#include <BulletCollision/CollisionShapes/btEmptyShape.h>
#include <BulletCollision/CollisionShapes/btSphereShape.h>
#include <BulletCollision/CollisionShapes/btMultiSphereShape.h>
#include <BulletCollision/CollisionShapes/btUniformScalingShape.h>
#include <BulletCollision/CollisionShapes/btScaledBvhTriangleMeshShape.h>
#include <BulletCollision/Gimpact/btGImpactShape.h>
#include <BulletCollision/CollisionShapes/btHeightfieldTerrainShape.h>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <iomanip>
#include <execution>

using namespace Engine;
using namespace Engine::priv;

MeshCPUData::MeshCPUData(const MeshCPUData& other) 
    : m_NodeData         { other.m_NodeData }
    , m_File             { other.m_File }
    , m_RadiusBox        { other.m_RadiusBox }
    , m_Skeleton         { std::exchange(other.m_Skeleton, nullptr) }
    , m_CollisionFactory { std::exchange(other.m_CollisionFactory, nullptr) }
    , m_VertexData       { std::exchange(other.m_VertexData, nullptr) }
    , m_Radius           { other.m_Radius }
    , m_Threshold        { other.m_Threshold }
{
    internal_transfer_cpu_datas();
    internal_calculate_radius(); //TODO: do we need this?
}
MeshCPUData& MeshCPUData::operator=(const MeshCPUData& other) {
    m_NodeData          = other.m_NodeData;
    m_File              = other.m_File;
    m_RadiusBox         = other.m_RadiusBox;
    m_Skeleton          = std::exchange(other.m_Skeleton, nullptr);
    m_CollisionFactory  = std::exchange(other.m_CollisionFactory, nullptr);
    m_VertexData        = std::exchange(other.m_VertexData, nullptr);
    m_Radius            = other.m_Radius;
    m_Threshold         = other.m_Threshold;
    internal_transfer_cpu_datas();
    internal_calculate_radius(); //TODO: do we need this?
    return *this;
}
MeshCPUData::MeshCPUData(MeshCPUData&& other) noexcept 
    : m_NodeData         { std::move(other.m_NodeData) }
    , m_File             { std::move(other.m_File) }
    , m_RadiusBox        { std::move(other.m_RadiusBox) }
    , m_Skeleton         { std::exchange(other.m_Skeleton, nullptr) }
    , m_CollisionFactory { std::exchange(other.m_CollisionFactory, nullptr) }
    , m_VertexData       { std::exchange(other.m_VertexData, nullptr) }
    , m_Radius           { std::move(other.m_Radius) }
    , m_Threshold        { std::move(other.m_Threshold) }
{
    internal_transfer_cpu_datas();
    internal_calculate_radius(); //TODO: do we need this?
}
MeshCPUData& MeshCPUData::operator=(MeshCPUData&& other) noexcept {
    m_NodeData         = std::move(other.m_NodeData);
    m_File             = std::move(other.m_File);
    m_RadiusBox        = std::move(other.m_RadiusBox);
    m_Skeleton         = std::exchange(other.m_Skeleton, nullptr);
    m_CollisionFactory = std::exchange(other.m_CollisionFactory, nullptr);
    m_VertexData       = std::exchange(other.m_VertexData, nullptr);
    m_Radius           = std::move(other.m_Radius);
    m_Threshold        = std::move(other.m_Threshold);
    internal_transfer_cpu_datas();
    internal_calculate_radius(); //TODO: do we need this?
    return *this;
}
void MeshCPUData::internal_transfer_cpu_datas() {
    if (m_CollisionFactory) {
        m_CollisionFactory->m_CPUData = this;
    }
}
void MeshCPUData::internal_calculate_radius() {
    if (!m_VertexData) {
        return;
    }
    m_RadiusBox = glm::vec3{ 0.0f };
    auto points = m_VertexData->getPositions();
    for (const auto& vertex : points) {
        m_RadiusBox.x = std::max(m_RadiusBox.x, std::abs(vertex.x));
        m_RadiusBox.y = std::max(m_RadiusBox.y, std::abs(vertex.y));
        m_RadiusBox.z = std::max(m_RadiusBox.z, std::abs(vertex.z));
    }
    m_Radius = Math::Max(m_RadiusBox);
}

constexpr auto DefaultMeshBindFunctor = [](Mesh* mesh_ptr, const Engine::priv::RenderModule* renderer) {
    mesh_ptr->getVertexData().bind();
};
constexpr auto DefaultMeshUnbindFunctor = [](Mesh* mesh_ptr, const Engine::priv::RenderModule* renderer) {
    mesh_ptr->getVertexData().unbind();
};

void PublicMesh::LoadGPU(Mesh& mesh) {
    mesh.m_CPUData.m_VertexData->finalize(); //transfer vertex data to gpu
    mesh.Resource::load();
}
void PublicMesh::UnloadCPU(Mesh& mesh) {
    mesh.Resource::unload();
}
void PublicMesh::UnloadGPU(Mesh& mesh) {
    SAFE_DELETE(mesh.m_CPUData.m_VertexData);
}
void PublicMesh::InitBlankMesh(Mesh& mesh) {
    mesh.registerEvent(EventType::WindowFullscreenChanged);
    mesh.setCustomBindFunctor(DefaultMeshBindFunctor);
    mesh.setCustomUnbindFunctor(DefaultMeshUnbindFunctor);
}
bool PublicMesh::SupportsInstancing() {
    return (Engine::priv::OpenGLState::constants.supportsInstancing() || OpenGLExtensions::supported(OpenGLExtensions::EXT_draw_instanced) || OpenGLExtensions::supported(OpenGLExtensions::ARB_draw_instanced));
}
btCollisionShape* PublicMesh::internal_build_collision(Handle meshHandle, ModelInstance* modelInstance, CollisionType collisionType, bool isCompoundChild) noexcept {
    Engine::priv::MeshCollisionFactory* factory = nullptr;
    if (!meshHandle) {
        factory = Engine::priv::Core::m_Engine->m_Misc.m_BuiltInMeshes.getCubeMesh().get<Mesh>()->m_CPUData.m_CollisionFactory;
    }else{
        factory = meshHandle.get<Mesh>()->m_CPUData.m_CollisionFactory;
    }
    if (factory) {
        switch (collisionType) {
            case CollisionType::BOX_SHAPE_PROXYTYPE: {
                return factory->buildBoxShape(modelInstance, isCompoundChild);
            }case CollisionType::CONVEX_HULL_SHAPE_PROXYTYPE: {
                return factory->buildConvexHull(modelInstance, isCompoundChild);
            }case CollisionType::SPHERE_SHAPE_PROXYTYPE: {
                return factory->buildSphereShape(modelInstance, isCompoundChild);
            }case CollisionType::MULTI_SPHERE_SHAPE_PROXYTYPE: {
                return factory->buildMultiSphereShape(modelInstance, isCompoundChild);
            }case CollisionType::TRIANGLE_MESH_SHAPE_PROXYTYPE: {
                return factory->buildTriangleShape(modelInstance, isCompoundChild);
            }case CollisionType::SCALED_TRIANGLE_MESH_SHAPE_PROXYTYPE: {
                return factory->buildTriangleShape(modelInstance, isCompoundChild);
            }case CollisionType::GIMPACT_SHAPE_PROXYTYPE: {
                return factory->buildTriangleShapeGImpact(modelInstance, isCompoundChild);
            }
        }
    }
    return new btEmptyShape{};
}
btCollisionShape* PublicMesh::BuildCollision(Handle meshHandle, CollisionType collisionType, bool isCompoundChild) {
    return internal_build_collision(meshHandle, nullptr, collisionType, isCompoundChild);
}
btCollisionShape* PublicMesh::BuildCollision(ModelInstance* modelInstance, CollisionType collisionType, bool isCompoundChild) {
    Handle meshHandle = (modelInstance && modelInstance->getMesh()) ? modelInstance->getMesh() : Handle{};
    return internal_build_collision(meshHandle, modelInstance, collisionType, isCompoundChild);
}
void PublicMesh::FinalizeVertexData(Handle meshHandle, MeshImportedData& data) {
    auto& cpuData = meshHandle.get<Mesh>()->m_CPUData;
    PublicMesh::FinalizeVertexData(cpuData, data);
}
void PublicMesh::FinalizeVertexData(MeshCPUData& cpuData, MeshImportedData& data) {
    data.m_UVs.resize(data.m_Points.size());
    data.m_Normals.resize(data.m_Points.size());
    data.m_Binormals.resize(data.m_Points.size());
    data.m_Tangents.resize(data.m_Points.size());
    if (!cpuData.m_VertexData) {
        if (cpuData.m_Skeleton) {
            cpuData.m_VertexData = NEW VertexData{ VertexDataFormat::VertexDataAnimated };
        }else{
            cpuData.m_VertexData = NEW VertexData{ VertexDataFormat::VertexDataBasic };
        }
    }
    auto& vertexData = *cpuData.m_VertexData;
    vertexData.clearData();

    auto normals = Engine::create_and_resize<std::vector<std::vector<GLuint>>>(3, std::vector<GLuint>{});
    if (cpuData.m_Threshold == 0.0f) {
        #pragma region No Threshold
        normals[0].reserve(data.m_Normals.size());
        normals[1].reserve(data.m_Binormals.size());
        normals[2].reserve(data.m_Tangents.size());
        for (size_t i = 0; i < data.m_Normals.size(); ++i) {
            normals[0].emplace_back(Engine::Compression::pack3NormalsInto32Int(data.m_Normals[i]));
        }
        for (size_t i = 0; i < data.m_Binormals.size(); ++i) {
            normals[1].emplace_back(Engine::Compression::pack3NormalsInto32Int(data.m_Binormals[i]));
        }
        for (size_t i = 0; i < data.m_Tangents.size(); ++i) {
            normals[2].emplace_back(Engine::Compression::pack3NormalsInto32Int(data.m_Tangents[i]));
        }
        vertexData.setData(0, data.m_Points.data(), data.m_Points.size(), MeshModifyFlags::None);
        vertexData.setData(1, data.m_UVs.data(), data.m_UVs.size(), MeshModifyFlags::None);
        vertexData.setData(2, normals[0].data(), normals[0].size(), MeshModifyFlags::None);
        vertexData.setData(3, normals[1].data(), normals[1].size(), MeshModifyFlags::None);
        vertexData.setData(4, normals[2].data(), normals[2].size(), MeshModifyFlags::None);
        vertexData.setIndices(data.m_Indices.data(), data.m_Indices.size(), MeshModifyFlags::RecalculateTriangles);
        #pragma endregion
    }else{
        #pragma region Some Threshold
        auto indices        = Engine::create_and_reserve<std::vector<uint32_t>>((uint32_t)data.m_Points.size());
        auto temp_pos       = Engine::create_and_reserve<std::vector<glm::vec3>>((uint32_t)data.m_Points.size());
        auto temp_uvs       = Engine::create_and_reserve<std::vector<glm::vec2>>((uint32_t)data.m_UVs.size());
        auto temp_normals   = Engine::create_and_reserve<std::vector<glm::vec3>>((uint32_t)data.m_Normals.size());
        auto temp_binormals = Engine::create_and_reserve<std::vector<glm::vec3>>((uint32_t)data.m_Binormals.size());
        auto temp_tangents  = Engine::create_and_reserve<std::vector<glm::vec3>>((uint32_t)data.m_Tangents.size());
        auto boneIDs        = Engine::create_and_reserve<std::vector<glm::vec4>>((uint32_t)data.m_Bones.size());
        auto boneWeights    = Engine::create_and_reserve<std::vector<glm::vec4>>((uint32_t)data.m_Bones.size());
        for (size_t i = 0; i < data.m_Points.size(); ++i) {
            uint32_t index;
            bool found = priv::MeshLoader::GetSimilarVertexIndex(data.m_Points[i], data.m_UVs[i], data.m_Normals[i], temp_pos, temp_uvs, temp_normals, index, cpuData.m_Threshold);
            if (found) {
                indices.emplace_back(index);
                //average out TBN. But it cancels out normal mapping on some flat surfaces
                //temp_binormals[index] += data.m_Binormals[i];
                //temp_tangents[index] += data.m_Tangents[i];
            }else{
                temp_pos.emplace_back(data.m_Points[i]);
                temp_uvs.emplace_back(data.m_UVs[i]);
                temp_normals.emplace_back(data.m_Normals[i]);
                temp_binormals.emplace_back(data.m_Binormals[i]);
                temp_tangents.emplace_back(data.m_Tangents[i]);

                if (data.m_Bones.size() > 0) {
                    const auto& ids     = data.m_Bones[(uint32_t)i].IDs;
                    const auto& weights = data.m_Bones[(uint32_t)i].Weights;
                    boneIDs.emplace_back(ids[0], ids[1], ids[2], ids[3]);
                    boneWeights.emplace_back(weights[0], weights[1], weights[2], weights[3]);
                }
                indices.emplace_back((uint32_t)temp_pos.size() - 1);
            }
        }
        normals[0].reserve(temp_normals.size());
        normals[1].reserve(temp_binormals.size());
        normals[2].reserve(temp_tangents.size());
        for (size_t i = 0; i < temp_normals.size(); ++i) {
            normals[0].emplace_back(Engine::Compression::pack3NormalsInto32Int(temp_normals[i]));
        }
        for (size_t i = 0; i < temp_binormals.size(); ++i) {
            normals[1].emplace_back(Engine::Compression::pack3NormalsInto32Int(temp_binormals[i]));
        }
        for (size_t i = 0; i < temp_tangents.size(); ++i) {
            normals[2].emplace_back(Engine::Compression::pack3NormalsInto32Int(temp_tangents[i]));
        }

        vertexData.setData(0, temp_pos.data(), temp_pos.size(), MeshModifyFlags::None);
        vertexData.setData(1, temp_uvs.data(), temp_uvs.size(), MeshModifyFlags::None);
        vertexData.setData(2, normals[0].data(), normals[0].size(), MeshModifyFlags::None);
        vertexData.setData(3, normals[1].data(), normals[1].size(), MeshModifyFlags::None);
        vertexData.setData(4, normals[2].data(), normals[2].size(), MeshModifyFlags::None);
        if (boneIDs.size() > 0) {
            vertexData.setData(5, boneIDs.data(), boneIDs.size(), MeshModifyFlags::None);
            vertexData.setData(6, boneWeights.data(), boneWeights.size(), MeshModifyFlags::None);
        }
        vertexData.setIndices(indices.data(), indices.size(), MeshModifyFlags::RecalculateTriangles);
        #pragma endregion
    }
}
void PublicMesh::CalculateRadius(Handle meshHandle) {
    Mesh& mesh       = *meshHandle.get<Mesh>();
    mesh.m_CPUData.internal_calculate_radius();
}
void PublicMesh::CalculateRadius(MeshCPUData& cpuData) {
    cpuData.internal_calculate_radius();
}






Mesh::Mesh() 
    : Resource{ ResourceType::Mesh }
{
    PublicMesh::InitBlankMesh(*this);
}

//TERRAIN MESH
void Mesh::internal_build_from_terrain(const Terrain& terrain) {
    MeshImportedData data;

    uint32_t count      = 0;
    float offsetSectorX = 0.0f;
    float offsetSectorY = 0.0f;

    auto hash_position  = [](const glm::vec3& position) {
        int64_t hash = 0;
        int32_t x    = static_cast<int32_t>(position.x);
        int32_t z    = static_cast<int32_t>(position.z);
        hash         = (static_cast<int64_t>(x) << 32) | z;
        return hash;
    };


    std::unordered_map<int64_t, VertexSmoothingGroup> m_VertexMap;
    auto& heightfields     = terrain.m_TerrainData.m_BtHeightfieldShapes;

    uint32_t sideSize      = terrain.m_TerrainData.m_BtHeightfieldShapesSizeRows > 0 ? (uint32_t)heightfields[0]->getUserIndex2() : 0;
    const float fsideSize  = (float)sideSize;

    float totalVertexSizeX = fsideSize * terrain.m_TerrainData.m_BtHeightfieldShapesSizeRows;
    float totalVertexSizeY = terrain.m_TerrainData.m_BtHeightfieldShapesSizeRows > 0 ? (fsideSize * terrain.m_TerrainData.m_BtHeightfieldShapesSizeCols) : 0.0f;

    for (size_t sectorX = 0; sectorX < terrain.m_TerrainData.m_BtHeightfieldShapesSizeRows; ++sectorX) {
        for (size_t sectorY = 0; sectorY < terrain.m_TerrainData.m_BtHeightfieldShapesSizeCols; ++sectorY) {
            auto& heightfield = *heightfields[(terrain.m_TerrainData.m_BtHeightfieldShapesSizeRows * sectorX) + sectorY];
            offsetSectorX     = sectorX * fsideSize;
            offsetSectorY     = sectorY * fsideSize;

            for (uint32_t i = 0; i < sideSize; ++i) {
                for (uint32_t j = 0; j < sideSize; ++j) {

                    uint32_t vertexAtX = i + (sideSize * (uint32_t)sectorY);
                    uint32_t vertexAtY = j + (sideSize * (uint32_t)sectorX);

                    btVector3 btVerts[4];
                    priv::Vertex verts[4];
                    VertexSmoothingData smooths[4];
                    bool valid[4];

                    valid[0] = heightfield.getAndValidateVertex(i,     j,     btVerts[0], false);
                    valid[1] = heightfield.getAndValidateVertex(i + 1, j,     btVerts[1], false);
                    valid[2] = heightfield.getAndValidateVertex(i,     j + 1, btVerts[2], false);
                    valid[3] = heightfield.getAndValidateVertex(i + 1, j + 1, btVerts[3], false);

                    for (int i = 0; i < 4; ++i) {
                        verts[i].position = glm::vec3(offsetSectorY + btVerts[i].x(), btVerts[i].y(), offsetSectorX + btVerts[i].z());
                    }

                    glm::vec3 a = verts[3].position - verts[0].position;
                    glm::vec3 b = verts[1].position - verts[2].position;
                    glm::vec3 normal = glm::normalize(glm::cross(a, b));

                    for (int i = 0; i < 4; ++i) {
                        verts[i].normal = normal;
                    }
                    verts[0].uv.x = vertexAtX / totalVertexSizeX;
                    verts[0].uv.y = vertexAtY / totalVertexSizeY;

                    verts[1].uv.x = (vertexAtX + 1.0f) / totalVertexSizeX;
                    verts[1].uv.y = vertexAtY / totalVertexSizeY;

                    verts[2].uv.x = vertexAtX / totalVertexSizeX;
                    verts[2].uv.y = (vertexAtY + 1.0f) / totalVertexSizeY;

                    verts[3].uv.x = (vertexAtX + 1.0f) / totalVertexSizeX;
                    verts[3].uv.y = (vertexAtY + 1.0f) / totalVertexSizeY;

                    for (int i = 0; i < 4; ++i) {
                        data.m_Points.emplace_back(verts[i].position);
                        data.m_UVs.emplace_back(verts[i].uv);
                        data.m_Normals.emplace_back(verts[i].normal);
                    }
                    for (int i = 0; i < 4; ++i) {
                        smooths[i].normal = verts[i].normal + glm::vec3{ 0.0001f };
                        smooths[i].index = (data.m_Points.size() - 1) - static_cast<size_t>(3 - i);

                        m_VertexMap[hash_position(verts[i].position)].data.emplace_back(std::move(smooths[i]));
                    }

                    if (valid[0] || valid[1] || valid[2] || valid[3]) {
                        data.m_Indices.emplace_back(count + 0);
                        data.m_Indices.emplace_back(count + 2);
                        data.m_Indices.emplace_back(count + 1);

                        data.m_Indices.emplace_back(count + 2);
                        data.m_Indices.emplace_back(count + 3);
                        data.m_Indices.emplace_back(count + 1);
                    }
                    count += 4;
                }
            }
        }
    }

    //TODO: optimize if possible
    //now smooth the normals
    for (auto& [name, vertex] : m_VertexMap) {
        auto& smoothed = vertex.smoothedNormal;
        for (auto& v : vertex.data) {
            smoothed += v.normal;
        }
        smoothed = glm::normalize(smoothed);
        for (auto& v : vertex.data) {
            data.m_Normals[v.index] = smoothed;
        }
    }
    MeshLoader::CalculateTBNAssimp(data);
    if (terrain.m_MeshHandle.null()) {
        PublicMesh::FinalizeVertexData(m_CPUData, data);
        PublicMesh::CalculateRadius(m_CPUData);
    }else{
        PublicMesh::FinalizeVertexData(terrain.m_MeshHandle, data);
        PublicMesh::CalculateRadius(terrain.m_MeshHandle);
    }
    SAFE_DELETE(m_CPUData.m_CollisionFactory);
}
void Mesh::internal_recalc_indices_from_terrain(const Terrain& terrain) {
    MeshImportedData data;
    uint32_t count = 0;
    auto& heightfields = terrain.m_TerrainData.m_BtHeightfieldShapes;
    for (size_t sectorX = 0; sectorX < terrain.m_TerrainData.m_BtHeightfieldShapesSizeRows; ++sectorX) {
        for (size_t sectorY = 0; sectorY < terrain.m_TerrainData.m_BtHeightfieldShapesSizeCols; ++sectorY) {
            auto& heightfield = *heightfields[(terrain.m_TerrainData.m_BtHeightfieldShapesSizeRows * sectorX) + sectorY];
            uint32_t width  = (uint32_t)heightfield.getUserIndex();
            uint32_t length = (uint32_t)heightfield.getUserIndex2();
            for (uint32_t i = 0; i < width; ++i) {
                for (uint32_t j = 0; j < length; ++j) {
                    btVector3 vert1, vert2, vert3, vert4;
                    bool valid[4];
                    valid[0] = heightfield.getAndValidateVertex(i, j,         vert1, false);
                    valid[1] = heightfield.getAndValidateVertex(i + 1, j,     vert2, false);
                    valid[2] = heightfield.getAndValidateVertex(i, j + 1,     vert3, false);
                    valid[3] = heightfield.getAndValidateVertex(i + 1, j + 1, vert4, false);
                    if (valid[0] || valid[1] || valid[2] || valid[3]) {
                        data.m_Indices.emplace_back(count + 0);
                        data.m_Indices.emplace_back(count + 2);
                        data.m_Indices.emplace_back(count + 1);

                        data.m_Indices.emplace_back(count + 2);
                        data.m_Indices.emplace_back(count + 3);
                        data.m_Indices.emplace_back(count + 1);
                    }
                    count += 4;
                }
            }
        }
    }
    m_CPUData.m_VertexData->bind();
    modifyIndices(data.m_Indices.data(), data.m_Indices.size());
    m_CPUData.m_VertexData->unbind();
}
Mesh::Mesh(std::string_view name, const Terrain& terrain, float threshold)
    : Resource{ ResourceType::Mesh }
{
    m_CPUData.m_Threshold = threshold;
    PublicMesh::InitBlankMesh(*this);
    internal_build_from_terrain(terrain);
    load();
}
Mesh::Mesh(VertexData& data, std::string_view name, float threshold)
    : Resource{ ResourceType::Mesh, name }
{
    m_CPUData.m_VertexData = &data;
    m_CPUData.m_Threshold  = threshold;
    PublicMesh::InitBlankMesh(*this);
}
Mesh::Mesh(std::string_view name, float width, float height, float threshold)
    : Resource{ ResourceType::Mesh, name }
{
    m_CPUData.m_Threshold = threshold;
    PublicMesh::InitBlankMesh(*this);

    MeshImportedData data;

    auto quad  = Engine::create_and_resize<std::vector<priv::Vertex>>(4, priv::Vertex{});
    quad[0].uv = glm::vec2{ 0.0f, height };
    quad[1].uv = glm::vec2{ width, height };
    quad[2].uv = glm::vec2{ width, 0.0f };
    quad[3].uv = glm::vec2{ 0.0f, 0.0f };

    quad[0].position = glm::vec3{ -width / 2.0f, -height / 2.0f, 0.0f };
    quad[1].position = glm::vec3{ width / 2.0f, -height / 2.0f, 0.0f };
    quad[2].position = glm::vec3{ width / 2.0f, height / 2.0f, 0.0f };
    quad[3].position = glm::vec3{ -width / 2.0f, height / 2.0f, 0.0f };

    for (uint32_t i = 0; i < 3; ++i) {   //triangle 1 (0, 1, 2)
        data.m_Points.emplace_back(quad[i].position);
        data.m_UVs.emplace_back(quad[i].uv);
    }
    for (uint32_t i = 0; i < 3; ++i) {   //triangle 2 (2, 3, 0)
        data.m_Points.emplace_back(quad[(i + 2) % 4].position);
        data.m_UVs.emplace_back(quad[(i + 2) % 4].uv);
    }
    m_CPUData.m_VertexData = NEW VertexData{ VertexDataFormat::VertexDataNoLighting };
    MeshLoader::FinalizeData(this->m_CPUData, data, threshold);

    load();
}
Mesh::Mesh(std::string_view fileOrData, float threshold)
    : Resource{ ResourceType::Mesh }
{
    m_CPUData.m_Threshold = threshold;
    PublicMesh::InitBlankMesh(*this);

    setName("Custom Mesh");
    uint8_t flags = MeshLoadingFlags::Points | MeshLoadingFlags::Faces | MeshLoadingFlags::UVs | MeshLoadingFlags::Normals | MeshLoadingFlags::TBN;

    MeshImportedData data;
    std::vector<std::vector<uint>> indices;
    indices.resize(3);
    std::istringstream stream{ fileOrData.data() };

    //first read in all data
    for (std::string line; std::getline(stream, line, '\n');) {
        if (line[0] == 'o') {
        }else if (line[0] == 'v' && line[1] == ' ') {
            if (flags && MeshLoadingFlags::Points) {
                auto& p = data.m_FilePoints.emplace_back();
                auto res = std::sscanf(line.substr(2, line.size()).c_str(), "%f %f %f", &p.x, &p.y, &p.z);
            }
        }else if (line[0] == 'v' && line[1] == 't') {
            if (flags && MeshLoadingFlags::UVs) {
                auto& uv = data.m_FileUVs.emplace_back();
                auto res = std::sscanf(line.substr(2, line.size()).c_str(), "%f %f", &uv.x, &uv.y);
                uv.y = 1.0f - uv.y;
            }
        }else if (line[0] == 'v' && line[1] == 'n') {
            if (flags && MeshLoadingFlags::Normals) {
                auto& n = data.m_FileNormals.emplace_back();
                auto res = std::sscanf(line.substr(2, line.size()).c_str(), "%f %f %f", &n.x, &n.y, &n.z);
            }
        }else if (line[0] == 'f' && line[1] == ' ') {
            if (flags && MeshLoadingFlags::Faces) {
                glm::uvec3 f1, f2, f3, f4 = glm::uvec3(1);
                int matches = std::sscanf(line.substr(2, line.size()).c_str(), "%d/%d/%d %d/%d/%d %d/%d/%d %d/%d/%d", &f1.x, &f1.y, &f1.z, &f2.x, &f2.y, &f2.z, &f3.x, &f3.y, &f3.z, &f4.x, &f4.y, &f4.z);
                if (matches < 3) {
                    matches = std::sscanf(line.substr(2, line.size()).c_str(), "%d %d %d %d", &f1.x, &f2.x, &f3.x, &f4.x);
                }
                f1 = glm::max(f1, glm::uvec3(1)); 
                f2 = glm::max(f2, glm::uvec3(1)); 
                f3 = glm::max(f3, glm::uvec3(1)); 
                f4 = glm::max(f4, glm::uvec3(1));
                if (matches == 3 || matches == 6 || matches == 9) { //triangle
                    indices[0].emplace_back(f1.x); indices[0].emplace_back(f2.x); indices[0].emplace_back(f3.x);
                    indices[1].emplace_back(f1.y); indices[1].emplace_back(f2.y); indices[1].emplace_back(f3.y);
                    indices[2].emplace_back(f1.z); indices[2].emplace_back(f2.z); indices[2].emplace_back(f3.z);
                }else if (matches == 4 || matches == 8 || matches == 12) {//quad
                    indices[0].emplace_back(f1.x); indices[0].emplace_back(f2.x); indices[0].emplace_back(f3.x);
                    indices[1].emplace_back(f1.y); indices[1].emplace_back(f2.y); indices[1].emplace_back(f3.y);
                    indices[2].emplace_back(f1.z); indices[2].emplace_back(f2.z); indices[2].emplace_back(f3.z);

                    indices[0].emplace_back(f1.x); indices[0].emplace_back(f3.x); indices[0].emplace_back(f4.x);
                    indices[1].emplace_back(f1.y); indices[1].emplace_back(f3.y); indices[1].emplace_back(f4.y);
                    indices[2].emplace_back(f1.z); indices[2].emplace_back(f3.z); indices[2].emplace_back(f4.z);
                }
            }
        }
    }
    if (flags & MeshLoadingFlags::Faces) {
        data.triangulateIndices(indices, flags);
    }
    if (flags & MeshLoadingFlags::TBN) {
        MeshLoader::CalculateTBNAssimp(data);
    }
    MeshLoader::FinalizeData(this->m_CPUData, data, threshold);

    load();
}
Mesh::Mesh(Mesh&& other) noexcept 
    : Resource{ std::move(other) }
    , m_CPUData             { std::move(other.m_CPUData) }
    , m_CustomBindFunctor   { std::move(other.m_CustomBindFunctor) }
    , m_CustomUnbindFunctor { std::move(other.m_CustomUnbindFunctor) }
{}
Mesh& Mesh::operator=(Mesh&& other) noexcept {
    Resource::operator=(std::move(other));
    m_CPUData             = std::move(other.m_CPUData);
    m_CustomBindFunctor   = std::move(other.m_CustomBindFunctor);
    m_CustomUnbindFunctor = std::move(other.m_CustomUnbindFunctor);
    return *this;
}
Mesh::~Mesh() {
    unregisterEvent(EventType::WindowFullscreenChanged);
    unload();
}
Engine::priv::MeshSkeleton::AnimationDataMap& Mesh::getAnimationData() {
    return m_CPUData.m_Skeleton->m_AnimationMapping;
}
void Mesh::load() {
    if(!isLoaded()){
        PublicMesh::LoadGPU(*this);
        //Resource::load();
    }
}
void Mesh::unload() {
    if(isLoaded()){
        PublicMesh::UnloadGPU(*this);
        PublicMesh::UnloadCPU(*this);
        //Resource::unload();
    }
}
void Mesh::onEvent(const Event& e) {
    if (e.type == EventType::WindowFullscreenChanged) {
        m_CPUData.m_VertexData->finalize();
    }
}
//TODO: optimize this a bit more
void Mesh::sortTriangles(const Camera& camera, ModelInstance& instance, const glm::mat4& bodyModelMatrix, SortingMode sortMode) {
    #ifndef _DEBUG
        auto& triangles      = m_CPUData.m_VertexData->m_Triangles;
        if (triangles.size() == 0) {
            return;
        }
        glm::vec3 camPos     = camera.getPosition();
        glm::mat4 worldModelMatrix = instance.getModelMatrix() * bodyModelMatrix;
        auto lambda_sorter   = [&camPos, sortMode, &worldModelMatrix](priv::Triangle& lhs, priv::Triangle& rhs) {
            glm::mat4 model1 = worldModelMatrix;
            glm::mat4 model2 = worldModelMatrix;

            model1 = glm::translate(model1, lhs.midpoint);
            model2 = glm::translate(model2, rhs.midpoint);

            glm::vec3 model1Pos = Math::getMatrixPosition(model1);
            glm::vec3 model2Pos = Math::getMatrixPosition(model2);

            if (sortMode == SortingMode::FrontToBack)
                return glm::distance2(camPos, model1Pos) < glm::distance2(camPos, model2Pos);
            else if (sortMode == SortingMode::BackToFront)
                return glm::distance2(camPos, model1Pos) > glm::distance2(camPos, model2Pos);
            return false;
        };
        //std::execution::par_unseq seems to really help here for performance
        Engine::sort(std::execution::par_unseq, triangles, lambda_sorter);

        auto newIndices = Engine::create_and_reserve<std::vector<uint32_t>>( static_cast<uint32_t>(m_CPUData.m_VertexData->m_Indices.size()) );
        for (size_t i = 0; i < triangles.size(); ++i) {
            auto& triangle = triangles[i];
            newIndices.emplace_back(triangle.index1);
            newIndices.emplace_back(triangle.index2);
            newIndices.emplace_back(triangle.index3);
        }
        Mesh::modifyIndices(newIndices.data(), newIndices.size());
    #endif
}