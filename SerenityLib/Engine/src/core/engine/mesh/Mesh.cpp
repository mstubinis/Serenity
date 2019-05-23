#include <core/engine/mesh/Mesh.h>
#include <core/engine/mesh/MeshLoading.h>
#include <core/engine/mesh/MeshImportedData.h>
#include <core/engine/mesh/Skeleton.h>
#include <core/engine/mesh/MeshCollisionFactory.h>

#include <core/engine/resources/Engine_Resources.h>
#include <core/engine/Engine_Math.h>
#include <core/MeshInstance.h>

#include <glm/gtc/matrix_transform.hpp>

#include <boost/filesystem.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/math/special_functions/fpclassify.hpp>

#include <bullet/btBulletDynamicsCommon.h>
#include <bullet/BulletCollision/Gimpact/btGImpactShape.h>
#include <bullet/BulletCollision/CollisionShapes/btHeightfieldTerrainShape.h>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

#include <boost/iostreams/device/mapped_file.hpp>
#include <iostream>
#include <fstream>

using namespace std;
using namespace Engine;
namespace boostm = boost::math;

Mesh* Mesh::FontPlane = nullptr;
Mesh* Mesh::Plane     = nullptr;
Mesh* Mesh::Cube      = nullptr;

namespace Engine {
    namespace epriv {
        struct DefaultMeshBindFunctor final{void operator()(BindableResource* r) const {
            const auto& m = *(Mesh*)r; m.m_VertexData->bind();
        }};
        struct DefaultMeshUnbindFunctor final {void operator()(BindableResource* r) const {
            const auto& m = *(Mesh*)r; m.m_VertexData->unbind();
        }};
    };
};

void epriv::InternalMeshPublicInterface::LoadCPU( Mesh& _mesh){
    _mesh.load_cpu();
}
void epriv::InternalMeshPublicInterface::LoadGPU( Mesh& _mesh){
    _mesh.load_gpu();
    _mesh.EngineResource::load();
}
void epriv::InternalMeshPublicInterface::UnloadCPU( Mesh& _mesh){
    _mesh.unload_cpu();
    _mesh.EngineResource::unload();
}
void epriv::InternalMeshPublicInterface::UnloadGPU( Mesh& _mesh){
    _mesh.unload_gpu();
}

bool epriv::InternalMeshPublicInterface::SupportsInstancing(){
    if(epriv::RenderManager::OPENGL_VERSION >= 31 || 
    epriv::OpenGLExtensionEnum::supported(epriv::OpenGLExtensionEnum::EXT_draw_instanced) || 
    epriv::OpenGLExtensionEnum::supported(epriv::OpenGLExtensionEnum::ARB_draw_instanced)){
        return true;
    }
    return false;
}

btCollisionShape* epriv::InternalMeshPublicInterface::BuildCollision(Mesh* _mesh, CollisionType::Type _type) {
    if(!_mesh) return new btEmptyShape();
    switch (_type) {
        case CollisionType::None: { return new btEmptyShape(); }
        case CollisionType::Box: { return _mesh->m_CollisionFactory->buildBoxShape(); }
        case CollisionType::ConvexHull: { return _mesh->m_CollisionFactory->buildConvexHull(); }
        case CollisionType::Sphere: { return _mesh->m_CollisionFactory->buildSphereShape(); }
        case CollisionType::TriangleShapeStatic: { return _mesh->m_CollisionFactory->buildTriangleShape(); }
        case CollisionType::TriangleShape: { return _mesh->m_CollisionFactory->buildTriangleShapeGImpact(); }
        default: { return new btEmptyShape(); }
    }
    return new btEmptyShape();
}


void Mesh::calculate_radius() {
    m_radiusBox = glm::vec3(0.0f);
    const auto& data = (*m_VertexData).getData<glm::vec3>(0);
    for (auto& _vertex : data) {
        const float x = abs(_vertex.x);
        const float y = abs(_vertex.y);
        const float z = abs(_vertex.z);
        if (x > m_radiusBox.x)  m_radiusBox.x = x;
        if (y > m_radiusBox.y)  m_radiusBox.y = y;
        if (z > m_radiusBox.z)  m_radiusBox.z = z;
    }
    m_radius = Math::Max(m_radiusBox);
}

void Mesh::unload_cpu() {
    SAFE_DELETE(m_Skeleton);
    SAFE_DELETE(m_CollisionFactory);
    cout << "(Mesh) ";
}
void Mesh::unload_gpu() {
    SAFE_DELETE(m_VertexData);
}

void Mesh::load_cpu() {
    /*
    if (m_File != "") {
        const string& extension = boost::filesystem::extension(m_File);
        epriv::MeshImportedData d;

        if (extension == ".objcc") {
            m_VertexData = epriv::MeshLoader::LoadFrom_OBJCC(m_File);
            calculate_radius();
        }else{
            //epriv::MeshLoader::LoadInternal(m_Skeleton, d, m_File);
            //epriv::MeshLoader::FinalizeData(*this, d, m_threshold);
            //if (extension == ".obj")
                //epriv::MeshLoader::SaveTo_OBJCC(*m_VertexData, "data/Models/ribbon.objcc");
        }
    }
    m_CollisionFactory = new epriv::MeshCollisionFactory(*this);
    */
}
void Mesh::load_gpu() {
    m_VertexData->finalize(); //transfer vertex data to gpu
    cout << "(Mesh) ";
}
void Mesh::triangulate_component_indices(epriv::MeshImportedData& data, vector<vector<uint>>& indices, unsigned char _flags) {
    for (uint i = 0; i < indices[0].size(); ++i) {
        glm::vec3 pos(0.0f);
        glm::vec2 uv(0.0f);
        glm::vec3 norm(1.0f);
        if (_flags && epriv::LOAD_POINTS && data.file_points.size() > 0) {
            pos = data.file_points[indices[0][i] - 1];
            data.points.push_back(pos);
        }
        if (_flags && epriv::LOAD_UVS && data.file_uvs.size() > 0) {
            uv = data.file_uvs[indices[1][i] - 1];
            data.uvs.push_back(uv);
        }
        if (_flags && epriv::LOAD_NORMALS && data.file_normals.size() > 0) {
            norm = data.file_normals[indices[2][i] - 1];
            data.normals.push_back(norm);
        }
    }
}


void Mesh::finalize_vertex_data(epriv::MeshImportedData& data) {
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
            }
            else {
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


void Mesh::init_blank() {
    m_File             = "";
    m_Skeleton         = nullptr;
    m_VertexData       = nullptr;
    m_CollisionFactory = nullptr;
    m_threshold        = 0.0005f;

    registerEvent(EventType::WindowFullscreenChanged);
    setCustomBindFunctor(epriv::DefaultMeshBindFunctor());
    setCustomUnbindFunctor(epriv::DefaultMeshUnbindFunctor());
}

Mesh::Mesh() {
    init_blank();
}

Mesh::Mesh(const string& name, const btHeightfieldTerrainShape& heightfield, float threshold) {
    init_blank();
    m_threshold = threshold;
    epriv::MeshImportedData data;
    const uint& width = heightfield.getHeightStickWidth();
    const uint& length = heightfield.getHeightStickLength();
    for (uint i = 0; i < width - 1; i++) {
        for (uint j = 0; j < length - 1; j++) {
            btVector3 vert1, vert2, vert3, vert4;
            heightfield.getVertex1(i, j, vert1);
            heightfield.getVertex1(i + 1, j, vert2);
            heightfield.getVertex1(i, j + 1, vert3);
            heightfield.getVertex1(i + 1, j + 1, vert4);

            epriv::Vertex v1, v2, v3, v4;
            v1.position = glm::vec3(vert1.x(), vert1.y(), vert1.z());
            v2.position = glm::vec3(vert2.x(), vert2.y(), vert2.z());
            v3.position = glm::vec3(vert3.x(), vert3.y(), vert3.z());
            v4.position = glm::vec3(vert4.x(), vert4.y(), vert4.z());

            glm::vec3 a = v4.position - v1.position;
            glm::vec3 b = v2.position - v3.position;
            glm::vec3 normal = glm::normalize(glm::cross(a, b));

            v1.normal = normal;
            v2.normal = normal;
            v3.normal = normal;
            v4.normal = normal;

            v1.uv = glm::vec2(float(i) / float(width), float(j) / float(length));
            v2.uv = glm::vec2(float(i + 1) / float(width), float(j) / float(length));
            v3.uv = glm::vec2(float(i) / float(width), float(j + 1) / float(length));
            v4.uv = glm::vec2(float(i + 1) / float(width), float(j + 1) / float(length));

            data.points.push_back(v3.position); data.uvs.push_back(v3.uv); data.normals.push_back(v3.normal);
            data.points.push_back(v2.position); data.uvs.push_back(v2.uv); data.normals.push_back(v2.normal);
            data.points.push_back(v1.position); data.uvs.push_back(v1.uv); data.normals.push_back(v1.normal);

            data.points.push_back(v3.position); data.uvs.push_back(v3.uv); data.normals.push_back(v3.normal);
            data.points.push_back(v4.position); data.uvs.push_back(v4.uv); data.normals.push_back(v4.normal);
            data.points.push_back(v2.position); data.uvs.push_back(v2.uv); data.normals.push_back(v2.normal);
        }
    }
    epriv::MeshLoader::CalculateTBNAssimp(data);
    epriv::MeshLoader::FinalizeData(*this, data, threshold);

    load();
}


Mesh::Mesh(VertexData* data, const string& name, float threshold):BindableResource(name) {
    init_blank();
    m_VertexData = data;
    m_threshold = threshold;
}
Mesh::Mesh(string name,float width, float height,float threshold):BindableResource(name){
    init_blank();
    m_threshold = threshold;

    epriv::MeshImportedData data;

    vector<epriv::Vertex> quad; quad.resize(4);
    quad[0].uv = glm::vec2(0.0f, 0.0f);
    quad[1].uv = glm::vec2(width, 0.0f);
    quad[2].uv = glm::vec2(width, height);
    quad[3].uv = glm::vec2(0.0f, height);

    quad[0].position = glm::vec3(-width / 2.0f, -height / 2.0f, 0.0f);
    quad[1].position = glm::vec3(width / 2.0f, -height / 2.0f, 0.0f);
    quad[2].position = glm::vec3(width / 2.0f, height / 2.0f, 0.0f);
    quad[3].position = glm::vec3(-width / 2.0f, height / 2.0f, 0.0f);

    for (uint i = 0; i < 3; ++i) {   //triangle 1 (0, 1, 2)
        data.points.emplace_back(quad[i].position);
        data.uvs.emplace_back(quad[i].uv);
    }
    for (uint i = 0; i < 3; ++i) {   //triangle 2 (2, 3, 0)
        data.points.emplace_back(quad[(i + 2) % 4].position);
        data.uvs.emplace_back(quad[(i + 2) % 4].uv);
    }
    m_VertexData = new VertexData(VertexDataFormat::VertexDataNoLighting);
    epriv::MeshLoader::FinalizeData(*this, data, threshold);

    load();
}
Mesh::Mesh(string fileOrData,float threshold):BindableResource(""){
    init_blank();
    m_threshold = threshold;

    setName("Custom Mesh");
    unsigned char _flags = epriv::LOAD_FACES | epriv::LOAD_UVS | epriv::LOAD_NORMALS | epriv::LOAD_TBN;

    epriv::MeshImportedData data;
    vector<vector<uint>> indices; indices.resize(3);
    istringstream stream; stream.str(fileOrData);

    //first read in all data
    for (string line; getline(stream, line, '\n');) {
        if (line[0] == 'o') {
        }
        else if (line[0] == 'v' && line[1] == ' ') {
            if (_flags && epriv::LOAD_POINTS) {
                glm::vec3 p;
                sscanf(line.substr(2, line.size()).c_str(), "%f %f %f", &p.x, &p.y, &p.z);
                data.file_points.push_back(p);
            }
        }else if (line[0] == 'v' && line[1] == 't') {
            if (_flags && epriv::LOAD_UVS) {
                glm::vec2 uv;
                sscanf(line.substr(2, line.size()).c_str(), "%f %f", &uv.x, &uv.y);
                uv.y = 1.0f - uv.y;
                data.file_uvs.push_back(uv);
            }
        }else if (line[0] == 'v' && line[1] == 'n') {
            if (_flags && epriv::LOAD_NORMALS) {
                glm::vec3 n;
                sscanf(line.substr(2, line.size()).c_str(), "%f %f %f", &n.x, &n.y, &n.z);
                data.file_normals.push_back(n);
            }
        }else if (line[0] == 'f' && line[1] == ' ') {
            if (_flags && epriv::LOAD_FACES) {
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
    if (_flags && epriv::LOAD_FACES) {
        triangulate_component_indices(data, indices, _flags);
    }
    if (_flags && epriv::LOAD_TBN) {
        epriv::MeshLoader::CalculateTBNAssimp(data);
    }
    epriv::MeshLoader::FinalizeData(*this, data, threshold);

    load();
}
Mesh::~Mesh(){
    unregisterEvent(EventType::WindowFullscreenChanged);
    unload();
}

unordered_map<string, epriv::AnimationData>& Mesh::animationData(){ return m_Skeleton->m_AnimationData; }

const VertexData& Mesh::getVertexStructure() const { return *m_VertexData; }

const glm::vec3& Mesh::getRadiusBox() const { return m_radiusBox; }
const float Mesh::getRadius() const { return m_radius; }
void Mesh::render(bool instancing, MeshDrawMode::Mode mode){
    const uint& indicesSize = m_VertexData->indices.size();
    if (indicesSize == 0) return;
    if (instancing && epriv::InternalMeshPublicInterface::SupportsInstancing()) {
        //const uint& instancesCount = m_InstanceCount;
        //if (instancesCount == 0) return;
        //if (epriv::RenderManager::OPENGL_VERSION >= 31) {
        //    glDrawElementsInstanced(mode, indicesSize, GL_UNSIGNED_SHORT, 0, instancesCount);
        //} else if (epriv::OpenGLExtensionEnum::supported(epriv::OpenGLExtensionEnum::EXT_draw_instanced)) {
        //    glDrawElementsInstancedEXT(mode, indicesSize, GL_UNSIGNED_SHORT, 0, instancesCount);
        //} else if (epriv::OpenGLExtensionEnum::supported(epriv::OpenGLExtensionEnum::ARB_draw_instanced)) {
        //    glDrawElementsInstancedARB(mode, indicesSize, GL_UNSIGNED_SHORT, 0, instancesCount);
        //}
    }else{
        glDrawElements(mode, indicesSize, GL_UNSIGNED_SHORT, 0);
    }
}
void Mesh::playAnimation(vector<glm::mat4>& transforms,const string& animationName,float time){
    auto& i = *m_Skeleton;
    if(transforms.size() == 0){
        transforms.resize(i.numBones(),glm::mat4(1.0f));
    }
    i.m_AnimationData.at(animationName).BoneTransform(animationName,time, transforms);
}
void Mesh::load(){
    if(!isLoaded()){
        load_cpu();
        load_gpu();
        EngineResource::load();
    }
}
void Mesh::unload(){
    if(isLoaded()){
        unload_gpu();
        unload_cpu();
        EngineResource::unload();
    }
}
void Mesh::onEvent(const Event& e) {
    if (e.type == EventType::WindowFullscreenChanged) {
        m_VertexData->finalize();
    }
}