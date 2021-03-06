#include <core/engine/utils/PrecompiledHeader.h>
#include <core/engine/mesh/Mesh.h>
#include <core/engine/mesh/MeshLoading.h>
#include <core/engine/mesh/MeshImportedData.h>
#include <core/engine/mesh/Skeleton.h>
#include <core/engine/mesh/MeshCollisionFactory.h>
#include <core/engine/events/Event.h>
#include <core/engine/system/Engine.h>
#include <core/Terrain.h>

#include <core/engine/physics/Collision.h>
#include <core/engine/math/Engine_Math.h>
#include <core/engine/math/MathCompression.h>
#include <core/engine/scene/Camera.h>

#include <boost/math/special_functions/fpclassify.hpp>

#include <BulletCollision/CollisionShapes/btEmptyShape.h>
#include <BulletCollision/CollisionShapes/btMultiSphereShape.h>
#include <BulletCollision/CollisionShapes/btUniformScalingShape.h>
#include <BulletCollision/CollisionShapes/btScaledBvhTriangleMeshShape.h>
#include <BulletCollision/Gimpact/btGImpactShape.h>
#include <BulletCollision/CollisionShapes/btHeightfieldTerrainShape.h>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

using namespace Engine;
using namespace Engine::priv;

MeshCPUData::MeshCPUData(const MeshCPUData& other) 
    : m_RadiusBox        { other.m_RadiusBox }
    , m_Skeleton         { std::exchange(other.m_Skeleton, nullptr) }
    , m_RootNode         { std::exchange(other.m_RootNode, nullptr) }
    , m_CollisionFactory { std::exchange(other.m_CollisionFactory, nullptr) }
    , m_VertexData       { std::exchange(other.m_VertexData, nullptr) }
    , m_File             { other.m_File }
    , m_Radius           { other.m_Radius }
    , m_Threshold        { other.m_Threshold }
{
    internal_transfer_cpu_datas();
    internal_calculate_radius(); //TODO: do we need this?
}
MeshCPUData& MeshCPUData::operator=(const MeshCPUData& other) {
    m_RadiusBox        = other.m_RadiusBox;
    m_Skeleton         = std::exchange(other.m_Skeleton, nullptr);
    m_RootNode         = std::exchange(other.m_RootNode, nullptr);
    m_CollisionFactory = std::exchange(other.m_CollisionFactory, nullptr);
    m_VertexData       = std::exchange(other.m_VertexData, nullptr);
    m_File             = other.m_File;
    m_Radius           = other.m_Radius;
    m_Threshold        = other.m_Threshold;
    internal_transfer_cpu_datas();
    internal_calculate_radius(); //TODO: do we need this?
    return *this;
}
MeshCPUData::MeshCPUData(MeshCPUData&& other) noexcept 
    : m_RadiusBox        { std::move(other.m_RadiusBox) }
    , m_Skeleton         { std::exchange(other.m_Skeleton, nullptr) }
    , m_RootNode         { std::exchange(other.m_RootNode, nullptr) }
    , m_CollisionFactory { std::exchange(other.m_CollisionFactory, nullptr) }
    , m_VertexData       { std::exchange(other.m_VertexData, nullptr) }
    , m_File             { std::move(other.m_File) }
    , m_Radius           { std::move(other.m_Radius) }
    , m_Threshold        { std::move(other.m_Threshold) }
{
    internal_transfer_cpu_datas();
    internal_calculate_radius(); //TODO: do we need this?
}
MeshCPUData& MeshCPUData::operator=(MeshCPUData&& other) noexcept {
    m_RadiusBox        = std::move(other.m_RadiusBox);
    m_Skeleton         = std::exchange(other.m_Skeleton, nullptr);
    m_RootNode         = std::exchange(other.m_RootNode, nullptr);
    m_CollisionFactory = std::exchange(other.m_CollisionFactory, nullptr);
    m_VertexData       = std::exchange(other.m_VertexData, nullptr);
    m_File             = std::move(other.m_File);
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
    if (m_Skeleton) {
        for (auto& [name, anim] : m_Skeleton->getAnimationData()) {
            anim.m_MeshCPUData = this;
        }
    }
}
void MeshCPUData::internal_calculate_radius() {
    if (!m_VertexData) {
        return;
    }
    m_RadiusBox = glm::vec3(0.0f);
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

void InternalMeshPublicInterface::LoadGPU(Mesh& mesh) {
    mesh.m_CPUData.m_VertexData->finalize(); //transfer vertex data to gpu
    mesh.Resource::load();
}
void InternalMeshPublicInterface::UnloadCPU(Mesh& mesh) {
    mesh.Resource::unload();
}
void InternalMeshPublicInterface::UnloadGPU(Mesh& mesh) {
    SAFE_DELETE(mesh.m_CPUData.m_VertexData);
}
void InternalMeshPublicInterface::InitBlankMesh(Mesh& mesh) {
    mesh.registerEvent(EventType::WindowFullscreenChanged);
    mesh.setCustomBindFunctor(DefaultMeshBindFunctor);
    mesh.setCustomUnbindFunctor(DefaultMeshUnbindFunctor);
}
bool InternalMeshPublicInterface::SupportsInstancing() {
    return (
        RenderModule::OPENGL_VERSION >= 31 ||
        OpenGLExtensions::supported(OpenGLExtensions::EXT_draw_instanced) || 
        OpenGLExtensions::supported(OpenGLExtensions::ARB_draw_instanced)
    );
}
btCollisionShape* InternalMeshPublicInterface::internal_build_collision(Handle meshHandle, ModelInstance* modelInstance, CollisionType collisionType, bool isCompoundChild) noexcept {
    Engine::priv::MeshCollisionFactory* factory = nullptr;
    if (!meshHandle) {
        factory = Engine::priv::Core::m_Engine->m_Misc.m_BuiltInMeshes.getCubeMesh().get<Mesh>()->m_CPUData.m_CollisionFactory;
    }else{
        factory = meshHandle.get<Mesh>()->m_CPUData.m_CollisionFactory;
    }
    if (factory) {
        switch (collisionType) {
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
            }
        }
    }
    return new btEmptyShape();
}
btCollisionShape* InternalMeshPublicInterface::BuildCollision(Handle meshHandle, CollisionType collisionType, bool isCompoundChild) {
    return internal_build_collision(meshHandle, nullptr, collisionType, isCompoundChild);
}
btCollisionShape* InternalMeshPublicInterface::BuildCollision(ModelInstance* modelInstance, CollisionType collisionType, bool isCompoundChild) {
    Handle meshHandle = (modelInstance && modelInstance->mesh()) ? modelInstance->mesh() : Handle{};
    return internal_build_collision(meshHandle, modelInstance, collisionType, isCompoundChild);
}
void InternalMeshPublicInterface::FinalizeVertexData(Handle meshHandle, MeshImportedData& data) {
    auto& cpuData = meshHandle.get<Mesh>()->m_CPUData;
    InternalMeshPublicInterface::FinalizeVertexData(cpuData, data);
}
void InternalMeshPublicInterface::FinalizeVertexData(MeshCPUData& cpuData, MeshImportedData& data) {
    data.uvs.resize(data.points.size());
    data.normals.resize(data.points.size());
    data.binormals.resize(data.points.size());
    data.tangents.resize(data.points.size());
    if (!cpuData.m_VertexData) {
        if (cpuData.m_Skeleton) {
            cpuData.m_VertexData = NEW VertexData(VertexDataFormat::VertexDataAnimated);
        }else{
            cpuData.m_VertexData = NEW VertexData(VertexDataFormat::VertexDataBasic);
        }
    }
    auto& vertexData = *cpuData.m_VertexData;
    vertexData.clearData();
    std::vector<std::vector<GLuint>> normals;
    normals.resize(3);
    if (cpuData.m_Threshold == 0.0f) {
        #pragma region No Threshold
        normals[0].reserve(data.normals.size());
        normals[1].reserve(data.binormals.size());
        normals[2].reserve(data.tangents.size());
        for (size_t i = 0; i < data.normals.size(); ++i) {
            normals[0].emplace_back(Engine::Compression::pack3NormalsInto32Int(data.normals[i]));
        }
        for (size_t i = 0; i < data.binormals.size(); ++i) {
            normals[1].emplace_back(Engine::Compression::pack3NormalsInto32Int(data.binormals[i]));
        }
        for (size_t i = 0; i < data.tangents.size(); ++i) {
            normals[2].emplace_back(Engine::Compression::pack3NormalsInto32Int(data.tangents[i]));
        }
        vertexData.setData(0, data.points.data(), data.points.size(), MeshModifyFlags::None);
        vertexData.setData(1, data.uvs.data(), data.uvs.size(), MeshModifyFlags::None);
        vertexData.setData(2, normals[0].data(), normals[0].size(), MeshModifyFlags::None);
        vertexData.setData(3, normals[1].data(), normals[1].size(), MeshModifyFlags::None);
        vertexData.setData(4, normals[2].data(), normals[2].size(), MeshModifyFlags::None);
        vertexData.setIndices(data.indices.data(), data.indices.size(), MeshModifyFlags::RecalculateTriangles);
        #pragma endregion
    }else{
        #pragma region Some Threshold
        std::vector<uint32_t>  indices;
        std::vector<glm::vec3> temp_pos;             temp_pos.reserve(data.points.size());
        std::vector<glm::vec2> temp_uvs;             temp_uvs.reserve(data.uvs.size());
        std::vector<glm::vec3> temp_normals;         temp_normals.reserve(data.normals.size());
        std::vector<glm::vec3> temp_binormals;       temp_binormals.reserve(data.binormals.size());
        std::vector<glm::vec3> temp_tangents;        temp_tangents.reserve(data.tangents.size());
        std::vector<glm::vec4> boneIDs;              boneIDs.reserve(data.m_Bones.size());
        std::vector<glm::vec4> boneWeights;          boneWeights.reserve(data.m_Bones.size());

        for (size_t i = 0; i < data.points.size(); ++i) {
            uint32_t index;
            bool found = priv::MeshLoader::GetSimilarVertexIndex(data.points[i], data.uvs[i], data.normals[i], temp_pos, temp_uvs, temp_normals, index, cpuData.m_Threshold);
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
                    boneIDs.emplace_back(
                        data.m_Bones[i].IDs[0], 
                        data.m_Bones[i].IDs[1], 
                        data.m_Bones[i].IDs[2], 
                        data.m_Bones[i].IDs[3]
                    );
                    boneWeights.emplace_back(
                        data.m_Bones[i].Weights[0], 
                        data.m_Bones[i].Weights[1], 
                        data.m_Bones[i].Weights[2], 
                        data.m_Bones[i].Weights[3]
                    );
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
void InternalMeshPublicInterface::CalculateRadius(Handle meshHandle) {
    Mesh& mesh       = *meshHandle.get<Mesh>();
    mesh.m_CPUData.internal_calculate_radius();
}







Mesh::Mesh() 
    : Resource{ ResourceType::Mesh }
{
    InternalMeshPublicInterface::InitBlankMesh(*this);
}

//TERRAIN MESH
void Mesh::internal_build_from_terrain(const Terrain& terrain) {
    MeshImportedData data;

    uint32_t count      = 0;
    float offsetSectorX = 0.0f;
    float offsetSectorY = 0.0f;

    auto hash_position  = [](glm::vec3& position, uint32_t decimal_places) {
        std::stringstream one, two, thr;
        one << std::fixed << std::setprecision(decimal_places) << position.x;
        two << std::fixed << std::setprecision(decimal_places) << position.y;
        thr << std::fixed << std::setprecision(decimal_places) << position.z;
        return one.str() + "_" + two.str() + "_" + thr.str();
    };

    std::unordered_map<std::string, VertexSmoothingGroup> m_VertexMap;
    auto& heightfields  = terrain.m_TerrainData.m_BtHeightfieldShapes;

    uint32_t width      = (uint32_t)heightfields[0][0]->getUserIndex();
    uint32_t length     = (uint32_t)heightfields[0][0]->getUserIndex2();
    const float fWidth  = (float)width;
    const float fLength = (float)length;

    float totalVertexSizeX = fWidth * heightfields.size();
    float totalVertexSizeY = fLength * heightfields[0].size();

    for (size_t sectorX = 0; sectorX < heightfields.size(); ++sectorX) {
        for (size_t sectorY = 0; sectorY < heightfields[sectorX].size(); ++sectorY) {
            auto& heightfield = *heightfields[sectorX][sectorY];
            offsetSectorX     = sectorX * fWidth;
            offsetSectorY     = sectorY * fLength;

            for (uint32_t i = 0; i < width; ++i) {
                for (uint32_t j = 0; j < length; ++j) {

                    uint32_t vertexAtX = i + (width * (uint32_t)sectorY);
                    uint32_t vertexAtY = j + (length * (uint32_t)sectorX);

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
                        data.points.emplace_back(verts[i].position);
                        data.uvs.emplace_back(verts[i].uv);
                        data.normals.emplace_back(verts[i].normal);
                    }
                    for (int i = 0; i < 4; ++i) {
                        smooths[i].normal = verts[i].normal;
                        smooths[i].index = (data.points.size() - 1) - static_cast<size_t>(3 - i);
                        m_VertexMap[hash_position(verts[i].position, 4)].data.emplace_back(std::move(smooths[i]));
                    }

                    if (valid[0] || valid[1] || valid[2] || valid[3]) {
                        data.indices.emplace_back(count + 0);
                        data.indices.emplace_back(count + 2);
                        data.indices.emplace_back(count + 1);

                        data.indices.emplace_back(count + 2);
                        data.indices.emplace_back(count + 3);
                        data.indices.emplace_back(count + 1);
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
            data.normals[v.index] = smoothed;
        }
    }
    MeshLoader::CalculateTBNAssimp(data);
    InternalMeshPublicInterface::FinalizeVertexData(terrain.m_MeshHandle, data);
    InternalMeshPublicInterface::CalculateRadius(terrain.m_MeshHandle);
    SAFE_DELETE(m_CPUData.m_CollisionFactory);
}
void Mesh::internal_recalc_indices_from_terrain(const Terrain& terrain) {
    MeshImportedData data;
    uint32_t count = 0;
    auto& heightfields = terrain.m_TerrainData.m_BtHeightfieldShapes;
    for (size_t sectorX = 0; sectorX < heightfields.size(); ++sectorX) {
        for (size_t sectorY = 0; sectorY < heightfields[sectorX].size(); ++sectorY) {
            auto& heightfield = *heightfields[sectorX][sectorY];
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
                        data.indices.emplace_back(count + 0);
                        data.indices.emplace_back(count + 2);
                        data.indices.emplace_back(count + 1);

                        data.indices.emplace_back(count + 2);
                        data.indices.emplace_back(count + 3);
                        data.indices.emplace_back(count + 1);
                    }
                    count += 4;
                }
            }
        }
    }
    m_CPUData.m_VertexData->bind();
    modifyIndices(data.indices.data(), data.indices.size());
    m_CPUData.m_VertexData->unbind();
}
Mesh::Mesh(const std::string& name, const Terrain& terrain, float threshold) 
    : Resource{ ResourceType::Mesh }
{
    m_CPUData.m_Threshold = threshold;
    InternalMeshPublicInterface::InitBlankMesh(*this);
    internal_build_from_terrain(terrain);
    load();
}
Mesh::Mesh(VertexData& data, const std::string& name, float threshold) 
    : Resource{ ResourceType::Mesh, name }
{
    m_CPUData.m_VertexData = &data;
    m_CPUData.m_Threshold  = threshold;
    InternalMeshPublicInterface::InitBlankMesh(*this);
}
Mesh::Mesh(const std::string& name, float width, float height, float threshold) 
    : Resource{ ResourceType::Mesh, name }
{
    m_CPUData.m_Threshold = threshold;
    InternalMeshPublicInterface::InitBlankMesh(*this);

    MeshImportedData data;

    std::vector<priv::Vertex> quad; quad.resize(4);

    quad[0].uv = glm::vec2(0.0f, height);
    quad[1].uv = glm::vec2(width, height);
    quad[2].uv = glm::vec2(width, 0.0f);
    quad[3].uv = glm::vec2(0.0f, 0.0f);

    quad[0].position = glm::vec3(-width / 2.0f, -height / 2.0f, 0.0f);
    quad[1].position = glm::vec3(width / 2.0f, -height / 2.0f, 0.0f);
    quad[2].position = glm::vec3(width / 2.0f, height / 2.0f, 0.0f);
    quad[3].position = glm::vec3(-width / 2.0f, height / 2.0f, 0.0f);

    for (uint32_t i = 0; i < 3; ++i) {   //triangle 1 (0, 1, 2)
        data.points.emplace_back(quad[i].position);
        data.uvs.emplace_back(quad[i].uv);
    }
    for (uint32_t i = 0; i < 3; ++i) {   //triangle 2 (2, 3, 0)
        data.points.emplace_back(quad[(i + 2) % 4].position);
        data.uvs.emplace_back(quad[(i + 2) % 4].uv);
    }
    m_CPUData.m_VertexData = NEW VertexData(VertexDataFormat::VertexDataNoLighting);
    MeshLoader::FinalizeData(this->m_CPUData, data, threshold);

    load();
}
Mesh::Mesh(const std::string& fileOrData, float threshold) 
    : Resource{ ResourceType::Mesh }
{
    m_CPUData.m_Threshold = threshold;
    InternalMeshPublicInterface::InitBlankMesh(*this);

    setName("Custom Mesh");
    uint8_t flags = MeshLoadingFlags::Points | MeshLoadingFlags::Faces | MeshLoadingFlags::UVs | MeshLoadingFlags::Normals | MeshLoadingFlags::TBN;

    MeshImportedData data;
    std::vector<std::vector<uint>> indices;
    indices.resize(3);
    std::istringstream stream;
    stream.str(fileOrData);

    //first read in all data
    for (std::string line; std::getline(stream, line, '\n');) {
        if (line[0] == 'o') {
        }else if (line[0] == 'v' && line[1] == ' ') {
            if (flags && MeshLoadingFlags::Points) {
                auto& p = data.file_points.emplace_back();
                auto res = std::sscanf(line.substr(2, line.size()).c_str(), "%f %f %f", &p.x, &p.y, &p.z);
            }
        }else if (line[0] == 'v' && line[1] == 't') {
            if (flags && MeshLoadingFlags::UVs) {
                auto& uv = data.file_uvs.emplace_back();
                auto res = std::sscanf(line.substr(2, line.size()).c_str(), "%f %f", &uv.x, &uv.y);
                uv.y = 1.0f - uv.y;
            }
        }else if (line[0] == 'v' && line[1] == 'n') {
            if (flags && MeshLoadingFlags::Normals) {
                auto& n = data.file_normals.emplace_back();
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
    , m_CustomBindFunctor   { std::move(other.m_CustomBindFunctor) }
    , m_CustomUnbindFunctor { std::move(other.m_CustomUnbindFunctor) }
    , m_CPUData             { std::move(other.m_CPUData) }
{}
Mesh& Mesh::operator=(Mesh&& other) noexcept {
    Resource::operator=(std::move(other));
    m_CustomBindFunctor   = std::move(other.m_CustomBindFunctor);
    m_CustomUnbindFunctor = std::move(other.m_CustomUnbindFunctor);
    m_CPUData             = std::move(other.m_CPUData);
    return *this;
}
Mesh::~Mesh() {
    unregisterEvent(EventType::WindowFullscreenChanged);
    unload();
}
std::unordered_map<std::string, AnimationData>& Mesh::animationData() {
    return m_CPUData.m_Skeleton->m_AnimationData;
}
void Mesh::load() {
    if(!isLoaded()){
        InternalMeshPublicInterface::LoadGPU(*this);
        //Resource::load();
    }
}
void Mesh::unload() {
    if(isLoaded()){
        InternalMeshPublicInterface::UnloadGPU(*this);
        InternalMeshPublicInterface::UnloadCPU(*this);
        //Resource::unload();
    }
}
void Mesh::onEvent(const Event& e) {
    if (e.type == EventType::WindowFullscreenChanged) {
        m_CPUData.m_VertexData->finalize();
    }
}
//TODO: optimize this a bit more (bubble sort?)
void Mesh::sortTriangles(const Camera& camera, ModelInstance& instance, const glm::mat4& bodyModelMatrix, SortingMode sortMode) {
    #ifndef _DEBUG
        auto& triangles      = m_CPUData.m_VertexData->m_Triangles;
        if (triangles.size() == 0) {
            return;
        }
        glm::vec3 camPos     = camera.getPosition();

        auto lambda_sorter   = [&camPos, sortMode, &instance, &bodyModelMatrix](priv::Triangle& lhs, priv::Triangle& rhs) {
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
            return false;
        };
        //std::execution::par_unseq seems to really help here for performance
        std::sort( std::execution::par_unseq, triangles.begin(), triangles.end(), lambda_sorter);

        std::vector<uint32_t> newIndices;
        newIndices.reserve(m_CPUData.m_VertexData->m_Indices.size());
        for (size_t i = 0; i < triangles.size(); ++i) {
            auto& triangle = triangles[i];
            newIndices.emplace_back(triangle.index1);
            newIndices.emplace_back(triangle.index2);
            newIndices.emplace_back(triangle.index3);
        }
        Mesh::modifyIndices(newIndices.data(), newIndices.size());
    #endif
}