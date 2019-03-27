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

class Mesh::impl final{
    public:
        VertexData*                  m_VertexData;
        epriv::MeshCollisionFactory* m_CollisionFactory;
        epriv::MeshSkeleton*         m_Skeleton;
        string                       m_File;
        glm::vec3                    m_radiusBox;
        float                        m_radius;
        float                        m_threshold;
        vector<glm::mat4>            m_InstanceMatrices;
        uint                         m_InstanceCount;
        void _initGlobal(float threshold){
            m_InstanceCount    = 0;
            m_File             = "";
            m_Skeleton         = nullptr;
            m_VertexData       = nullptr;
            m_CollisionFactory = nullptr;
            m_threshold        = threshold;
        }
        void _init(Mesh& super,string& name,unordered_map<string,float>& grid,uint width,uint length,float threshold){//grid
            epriv::MeshImportedData d;
            _initGlobal(threshold);  
            for(uint i = 0; i < width-1; ++i){
                for(uint j = 0; j < length-1; ++j){
                    string key1(to_string(i) + "," + to_string(j));
                    string key2(to_string(i + 1) + "," + to_string(j));
                    string key3(to_string(i) + "," + to_string(j + 1));
                    string key4(to_string(i + 1) + "," + to_string(j + 1));

                    epriv::Vertex v1,v2,v3,v4;
                    v1.position = glm::vec3(i - width / 2.0f, grid[key1], j - length / 2.0f);
                    v2.position = glm::vec3((i + 1) - width / 2.0f, grid[key2], j - length / 2.0f);
                    v3.position = glm::vec3(i - width / 2.0f, grid[key3], (j + 1) - length / 2.0f);
                    v4.position = glm::vec3((i + 1) - width / 2.0f, grid[key4], (j + 1) - length / 2.0f);

                    glm::vec3 a(v4.position - v1.position);
                    glm::vec3 b(v2.position - v3.position);
                    glm::vec3 normal(glm::normalize(glm::cross(a,b)));

                    v1.normal = normal;
                    v2.normal = normal;
                    v3.normal = normal;
                    v4.normal = normal;

                    v1.uv = glm::vec2(float(i) / float(width), float(j) / float(length));
                    v2.uv = glm::vec2(float(i + 1) / float(width), float(j) / float(length));
                    v3.uv = glm::vec2(float(i) / float(width), float(j + 1) / float(length));
                    v4.uv = glm::vec2(float(i + 1) / float(width), float(j + 1) / float(length));

                    d.points.push_back(v3.position); d.uvs.push_back(v3.uv); d.normals.push_back(v3.normal);
                    d.points.push_back(v2.position); d.uvs.push_back(v2.uv); d.normals.push_back(v2.normal);
                    d.points.push_back(v1.position); d.uvs.push_back(v1.uv); d.normals.push_back(v1.normal);

                    d.points.push_back(v3.position); d.uvs.push_back(v3.uv); d.normals.push_back(v3.normal);
                    d.points.push_back(v4.position); d.uvs.push_back(v4.uv); d.normals.push_back(v4.normal);
                    d.points.push_back(v2.position); d.uvs.push_back(v2.uv); d.normals.push_back(v2.normal);

                    epriv::MeshLoader::CalculateTBNAssimp(d);
                }
            }
            m_VertexData = nullptr;
            _finalizeData(d, threshold);
            super.load();
        }     
        void _init(Mesh& super,string& name,float width, float height,float threshold){//plane
            epriv::MeshImportedData d;
            _initGlobal(threshold);
            d.points.emplace_back(-width / 2.0f, -height / 2.0f, 0);
            d.points.emplace_back(width / 2.0f, height / 2.0f, 0);
            d.points.emplace_back(-width / 2.0f, height / 2.0f, 0);

            d.points.emplace_back(width / 2.0f, -height / 2.0f, 0);
            d.points.emplace_back(width / 2.0f, height / 2.0f, 0);
            d.points.emplace_back(-width / 2.0f, -height / 2.0f, 0);

            float uv_topLeft_x = 0.0f;
            float uv_topLeft_y = 0.0f;

            float uv_bottomLeft_x = 0.0f;
            float uv_bottomLeft_y = 0.0f + float(height);

            float uv_bottomRight_x = 0.0f + float(width);
            float uv_bottomRight_y = 0.0f + float(height);

            float uv_topRight_x = 0.0f + float(width);
            float uv_topRight_y = 0.0f;

            d.uvs.emplace_back(uv_bottomLeft_x, uv_bottomLeft_y);
            d.uvs.emplace_back(uv_topRight_x, uv_topRight_y);
            d.uvs.emplace_back(uv_topLeft_x, uv_topLeft_y);

            d.uvs.emplace_back(uv_bottomRight_x, uv_bottomRight_y);
            d.uvs.emplace_back(uv_topRight_x, uv_topRight_y);
            d.uvs.emplace_back(uv_bottomLeft_x, uv_bottomLeft_y);

            m_VertexData = new VertexData(VertexDataFormat::VertexDataNoLighting);
            _finalizeData(d, threshold);
            super.load();
        }
        void _init(Mesh& super,string& fileOrData,bool notMemory,float threshold,bool loadNow){//from file / data
            _initGlobal(threshold);
            if(notMemory){
                m_File = fileOrData;
            }else{
                _loadFromOBJMemory(threshold, epriv::LOAD_FACES | epriv::LOAD_UVS | epriv::LOAD_NORMALS | epriv::LOAD_TBN, fileOrData);
            }
            if(loadNow)
                super.load();
        }
        void _finalizeData(epriv::MeshImportedData& data,float threshold){
            m_threshold = threshold;

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
        void _loadFromFile(Mesh& super, string& file, float threshold) {
            string extension = boost::filesystem::extension(file);
            epriv::MeshImportedData d;

            if (extension == ".objcc") {
                m_VertexData = epriv::MeshLoader::LoadFrom_OBJCC(file);
            }else{
                epriv::MeshLoader::LoadInternal(m_Skeleton, d, m_File);
                _finalizeData(d, threshold);
                //if (extension == ".obj")
                    //epriv::MeshLoader::SaveTo_OBJCC(*m_VertexData, "data/Models/ribbon.objcc");
            }
        }
        void _loadDataIntoTriangles(epriv::MeshImportedData& data,vector<uint>& _pi,vector<uint>& _ui,vector<uint>& _ni,unsigned char _flags){
            uint count = 0;
            epriv::Triangle triangle;
            for(uint i=0; i < _pi.size(); ++i ){
                glm::vec3 pos(glm::vec3(0.0f,0.0f,0.0f));
                glm::vec2 uv(glm::vec2(0.0f,0.0f));
                glm::vec3 norm(glm::vec3(1.0f,1.0f,1.0f));
                if(_flags && epriv::LOAD_POINTS && data.file_points.size() > 0){  
                    pos = data.file_points[_pi[i]-1];
                    data.points.push_back(pos);
                }
                if(_flags && epriv::LOAD_UVS && data.file_uvs.size() > 0){
                    uv  = data.file_uvs[_ui[i]-1];
                    data.uvs.push_back(uv);
                }
                if(_flags && epriv::LOAD_NORMALS && data.file_normals.size() > 0){ 
                    norm = data.file_normals[_ni[i]-1];
                    data.normals.push_back(norm);
                }
                //data.indices.emplace_back((ushort)count);
                ++count;
                if(count == 1){
                    triangle.v1.position = pos;
                    triangle.v1.uv = uv;
                    triangle.v1.normal = norm;
                }else if(count == 2){
                    triangle.v2.position = pos;
                    triangle.v2.uv = uv;
                    triangle.v2.normal = norm;
                }else if(count >= 3){
                    triangle.v3.position = pos;
                    triangle.v3.uv = uv;
                    triangle.v3.normal = norm;
                    data.file_triangles.push_back(triangle);
                    count = 0;
                }
            }
        }
        void _loadObjDataFromLine(string& l,epriv::MeshImportedData& _d,vector<uint>& _pi,vector<uint>& _ui,vector<uint>& _ni,const char _f){
            if(l[0] == 'o'){
            }else if(l[0] == 'v' && l[1] == ' '){ 
                if(_f && epriv::LOAD_POINTS){
                    glm::vec3 p;
                    sscanf(l.substr(2,l.size()).c_str(),"%f %f %f",&p.x,&p.y,&p.z);
                    _d.file_points.push_back(p);
                }
            }else if(l[0] == 'v' && l[1] == 't'){
                if(_f && epriv::LOAD_UVS){
                    glm::vec2 uv;
                    sscanf(l.substr(2,l.size()).c_str(),"%f %f",&uv.x,&uv.y);
                    uv.y = 1.0f - uv.y;
                    _d.file_uvs.push_back(uv);
                }
            }else if(l[0] == 'v' && l[1] == 'n'){
                if(_f && epriv::LOAD_NORMALS){
                    glm::vec3 n;
                    sscanf(l.substr(2,l.size()).c_str(),"%f %f %f",&n.x,&n.y,&n.z);
                    _d.file_normals.push_back(n);
                }
            }else if(l[0] == 'f' && l[1] == ' '){
                if(_f && epriv::LOAD_FACES){
                    glm::uvec3 f1,f2,f3,f4 = glm::uvec3(1);
                    int matches = sscanf(l.substr(2,l.size()).c_str(),"%d/%d/%d %d/%d/%d %d/%d/%d %d/%d/%d",&f1.x,&f1.y,&f1.z,&f2.x,&f2.y,&f2.z,&f3.x,&f3.y,&f3.z,&f4.x,&f4.y,&f4.z);
                    if(matches < 3){
                        matches = sscanf(l.substr(2,l.size()).c_str(),"%d %d %d %d",&f1.x,&f2.x,&f3.x,&f4.x);
                    }
                    f1 = glm::max(f1,glm::uvec3(1)); f2 = glm::max(f2,glm::uvec3(1)); f3 = glm::max(f3,glm::uvec3(1)); f4 = glm::max(f4,glm::uvec3(1));
                    if(matches == 3 || matches == 6 || matches == 9){ //triangle
                        _pi.push_back(f1.x); _pi.push_back(f2.x); _pi.push_back(f3.x);
                        _ui.push_back(f1.y); _ui.push_back(f2.y); _ui.push_back(f3.y);
                        _ni.push_back(f1.z); _ni.push_back(f2.z); _ni.push_back(f3.z);
                    }else if(matches == 4 || matches == 8 || matches == 12){//quad
                        _pi.push_back(f1.x); _pi.push_back(f2.x); _pi.push_back(f3.x);
                        _ui.push_back(f1.y); _ui.push_back(f2.y); _ui.push_back(f3.y);
                        _ni.push_back(f1.z); _ni.push_back(f2.z); _ni.push_back(f3.z);

                        _pi.push_back(f1.x); _pi.push_back(f3.x); _pi.push_back(f4.x);
                        _ui.push_back(f1.y); _ui.push_back(f3.y); _ui.push_back(f4.y);
                        _ni.push_back(f1.z); _ni.push_back(f3.z); _ni.push_back(f4.z);
                    }
                }
            }
        }
        void _loadFromOBJMemory(float threshold,unsigned char _flags,string input){
            epriv::MeshImportedData d;
            vector<vector<uint>> indices; indices.resize(3);
            istringstream stream; stream.str(input);

            //first read in all data
            for(string line; getline(stream, line, '\n');){
                _loadObjDataFromLine(line, d, indices[0], indices[1], indices[2], _flags);
            }
            if(_flags && epriv::LOAD_FACES){
                _loadDataIntoTriangles(d, indices[0], indices[1], indices[2], _flags);
            }
            if(_flags && epriv::LOAD_TBN && d.normals.size() > 0){
                epriv::MeshLoader::CalculateTBNAssimp(d);
            }
            _finalizeData(d,threshold);
        }
        void _calculateMeshRadius(VertexData& vertexData){
            glm::vec3 max = glm::vec3(0.0f);
            const auto& data = vertexData.getData<glm::vec3>(0);
            for(auto& _vertex : data){
                float x = abs(_vertex.x);
                float y = abs(_vertex.y);
                float z = abs(_vertex.z);
                if(x > max.x) 
                    max.x = x; 
                if(y > max.y) 
                    max.y = y; 
                if(z > max.z) 
                    max.z = z;
            }
            m_radiusBox = max;
            m_radius = Math::Max(m_radiusBox);
        }
        
        void _unload_CPU(){
            SAFE_DELETE(m_Skeleton);
            SAFE_DELETE(m_CollisionFactory);
            cout << "(Mesh) ";
        }
        void _unload_GPU(){
            SAFE_DELETE(m_VertexData);

        }
        void _load_CPU(Mesh& super){
            if(m_File != ""){
                _loadFromFile(super,m_File,m_threshold);
            }
            _calculateMeshRadius(*m_VertexData);
            m_CollisionFactory = new epriv::MeshCollisionFactory(super,*m_VertexData);
        }
        void _load_GPU(){
            m_VertexData->finalize();
            cout << "(Mesh) ";
        }
     
};

namespace Engine {
    namespace epriv {
        struct DefaultMeshBindFunctor final{void operator()(BindableResource* r) const {
            const auto& m = *((Mesh*)r)->m_i;
            m.m_VertexData->bind();
        }};
        struct DefaultMeshUnbindFunctor final {void operator()(BindableResource* r) const {
            const auto& m = *((Mesh*)r)->m_i;
            m.m_VertexData->unbind();
        }};
    };
};



void epriv::InternalMeshPublicInterface::LoadCPU( Mesh& _mesh){
    _mesh.m_i->_load_CPU(_mesh);
}
void epriv::InternalMeshPublicInterface::LoadGPU( Mesh& _mesh){
    _mesh.m_i->_load_GPU();
    _mesh.EngineResource::load();
}
void epriv::InternalMeshPublicInterface::UnloadCPU( Mesh& _mesh){
    _mesh.m_i->_unload_CPU();
    _mesh.EngineResource::unload();
}
void epriv::InternalMeshPublicInterface::UnloadGPU( Mesh& _mesh){
    _mesh.m_i->_unload_GPU();
}
/*
void epriv::InternalMeshPublicInterface::UpdateInstance( Mesh& _mesh,uint _id, glm::mat4 _modelMatrix){
    auto& i = *_mesh.m_i;
    glBindBuffer(GL_ARRAY_BUFFER, i.m_buffers[2]);
    glBufferSubData(GL_ARRAY_BUFFER, _id * sizeof(glm::mat4), sizeof(glm::mat4), &_modelMatrix);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}
void epriv::InternalMeshPublicInterface::UpdateInstances( Mesh& _mesh,vector<glm::mat4>& _modelMatrices){
    auto& i = *_mesh.m_i;
    i.m_InstanceCount = _modelMatrices.size();
    if(_modelMatrices.size() == 0) return;
    glBindBuffer(GL_ARRAY_BUFFER, i.m_buffers[2]);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(glm::mat4) * i.m_InstanceCount, &_modelMatrices[0]);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}
*/
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
        case CollisionType::Box: { return _mesh->m_i->m_CollisionFactory->buildBoxShape(); }
        case CollisionType::ConvexHull: { return _mesh->m_i->m_CollisionFactory->buildConvexHull(); }
        case CollisionType::Sphere: { return _mesh->m_i->m_CollisionFactory->buildSphereShape(); }
        case CollisionType::TriangleShapeStatic: { return _mesh->m_i->m_CollisionFactory->buildTriangleShape(); }
        case CollisionType::TriangleShape: { return _mesh->m_i->m_CollisionFactory->buildTriangleShapeGImpact(); }
        default: { return new btEmptyShape(); }
    }
    return new btEmptyShape();
}


Mesh::Mesh(string name,unordered_map<string,float>& grid,uint width,uint length,float threshold):BindableResource(name),m_i(new impl){
    m_i->_init(*this,name,grid,width,length,threshold);
    registerEvent(EventType::WindowFullscreenChanged);
    setCustomBindFunctor(epriv::DefaultMeshBindFunctor());
    setCustomUnbindFunctor(epriv::DefaultMeshUnbindFunctor());
}
Mesh::Mesh(string name,float width, float height,float threshold):BindableResource(name),m_i(new impl){
    m_i->_init(*this,name,width,height,threshold);
    registerEvent(EventType::WindowFullscreenChanged);
    setCustomBindFunctor(epriv::DefaultMeshBindFunctor());
    setCustomUnbindFunctor(epriv::DefaultMeshUnbindFunctor());
}
Mesh::Mesh(string fileOrData,bool notMemory,float threshold,bool loadNow):BindableResource(fileOrData),m_i(new impl){
    if (!notMemory) setName("CustomMesh");
    m_i->_init(*this,fileOrData,notMemory,threshold,loadNow);
    registerEvent(EventType::WindowFullscreenChanged);
    setCustomBindFunctor(epriv::DefaultMeshBindFunctor());
    setCustomUnbindFunctor(epriv::DefaultMeshUnbindFunctor());
}
Mesh::~Mesh(){
    unregisterEvent(EventType::WindowFullscreenChanged);
    unload();
}

unordered_map<string, epriv::AnimationData>& Mesh::animationData(){ return m_i->m_Skeleton->m_AnimationData; }

const VertexData& Mesh::getVertexStructure() const { return *m_i->m_VertexData; }

const glm::vec3& Mesh::getRadiusBox() const { return m_i->m_radiusBox; }
const float Mesh::getRadius() const { return m_i->m_radius; }
void Mesh::render(bool instancing, MeshDrawMode::Mode mode){
    auto& i = *m_i;
    const uint& indicesSize = i.m_VertexData->indices.size();
    if (indicesSize == 0) return;
    if (instancing && epriv::InternalMeshPublicInterface::SupportsInstancing()) {
        const uint& instancesCount = i.m_InstanceCount;
        if (instancesCount == 0) return;
        if (epriv::RenderManager::OPENGL_VERSION >= 31) {
            glDrawElementsInstanced(mode, indicesSize, GL_UNSIGNED_SHORT, 0, instancesCount);
        } else if (epriv::OpenGLExtensionEnum::supported(epriv::OpenGLExtensionEnum::EXT_draw_instanced)) {
            glDrawElementsInstancedEXT(mode, indicesSize, GL_UNSIGNED_SHORT, 0, instancesCount);
        } else if (epriv::OpenGLExtensionEnum::supported(epriv::OpenGLExtensionEnum::ARB_draw_instanced)) {
            glDrawElementsInstancedARB(mode, indicesSize, GL_UNSIGNED_SHORT, 0, instancesCount);
        }
    }else{
        glDrawElements(mode, indicesSize, GL_UNSIGNED_SHORT, 0);
    }
}
void Mesh::playAnimation(vector<glm::mat4>& transforms,const string& animationName,float time){
    auto& i = *m_i->m_Skeleton;
    if(transforms.size() == 0){
        transforms.resize(i.numBones(),glm::mat4(1.0f));
    }
    i.m_AnimationData.at(animationName).BoneTransform(animationName,time, transforms);
}
void Mesh::load(){
    if(!isLoaded()){
        auto& _this = *this;
        auto& i = *m_i;
        i._load_CPU(_this);
        i._load_GPU();
        EngineResource::load();
    }
}
void Mesh::unload(){
    if(isLoaded()){
        auto& i = *m_i;
        i._unload_GPU();
        i._unload_CPU();
        EngineResource::unload();
    }
}
void Mesh::onEvent(const Event& e) {
    if (e.type == EventType::WindowFullscreenChanged) {
        const auto& i = *m_i;
        i.m_VertexData->finalize();
    }
}
