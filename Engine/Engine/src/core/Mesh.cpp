#include "core/Mesh.h"
#include "core/engine/mesh/VertexData.h"
#include "core/engine/Engine_Resources.h"
#include "core/engine/Engine_Math.h"
#include "core/MeshInstance.h"

#include <bullet/btBulletDynamicsCommon.h>
#include <bullet/BulletCollision/CollisionShapes/btHeightfieldTerrainShape.h>
#include <bullet/BulletCollision/CollisionShapes/btShapeHull.h>
#include <bullet/BulletCollision/Gimpact/btGImpactShape.h>

#include <glm/gtc/matrix_transform.hpp>

#include <boost/filesystem.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/math/special_functions/fpclassify.hpp>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

#include <boost/iostreams/device/mapped_file.hpp>
#include <iostream>
#include <fstream>

using namespace std;
using namespace Engine;
namespace boostm = boost::math;

Mesh* Mesh::FontPlane = nullptr;
Mesh* Mesh::Plane = nullptr;
Mesh* Mesh::Cube = nullptr;

void float32(float* __restrict out, const uint16_t in) {
    uint32_t t1,t2,t3;
    t1 = in & 0x7fff;                       // Non-sign bits
    t2 = in & 0x8000;                       // Sign bit
    t3 = in & 0x7c00;                       // Exponent
    t1 <<= 13;                              // Align mantissa on MSB
    t2 <<= 16;                              // Shift sign bit into position
    t1 += 0x38000000;                       // Adjust bias
    t1 = (t3 == 0 ? 0 : t1);                // Denormals-as-zero
    t1 |= t2;                               // Re-insert sign bit
    *((uint32_t*)out) = t1;
};
void float16(uint16_t* __restrict out, const float in) {
    uint32_t inu = *((uint32_t*)&in);
    uint32_t t1,t2,t3;
    t1 = inu & 0x7fffffff;                 // Non-sign bits
    t2 = inu & 0x80000000;                 // Sign bit
    t3 = inu & 0x7f800000;                 // Exponent
    t1 >>= 13;                             // Align mantissa on MSB
    t2 >>= 16;                             // Shift sign bit into position
    t1 -= 0x1c000;                         // Adjust bias
    t1 = (t3 < 0x38800000) ? 0 : t1;
    t1 = (t3 > 0x47000000) ? 0x7bff : t1;
    t1 = (t3 == 0 ? 0 : t1);               // Denormals-as-zero
    t1 |= t2;                              // Re-insert sign bit
    *((uint16_t*)out) = t1;
};

namespace Engine{
    namespace epriv{
        enum LoadWhat{
            LOAD_POINTS  = 1,
            LOAD_UVS     = 2,
            LOAD_NORMALS = 4,
            LOAD_FACES   = 8,
            LOAD_TBN     = 16,
                       //= 32,
                       //= 64,
                       //= 128,
                       //= 256,
                       //= 512,
                       //= 1024,
                       //= 2048,
                       //= 4096,
        };
        class MeshLoader final {
            friend class ::Mesh;
            public:
                static inline bool IsNear(float& v1, float& v2, float& threshold) { 
                    return std::abs(v1 - v2) < threshold;
                }
                static inline bool IsNear(glm::vec2& v1, glm::vec2& v2, float& threshold) {
                    return (std::abs(v1.x - v2.x) < threshold && std::abs(v1.y - v2.y) < threshold) ? true : false;
                }
                static inline bool IsNear(glm::vec3& v1, glm::vec3& v2, float& threshold) {
                    return (std::abs(v1.x - v2.x) < threshold && std::abs(v1.y - v2.y) < threshold && std::abs(v1.z - v2.z) < threshold) ? true : false;
                }
                static inline bool IsSpecialFloat(float& f) {
                    if (boostm::isnan(f)) return true;
                    if (boostm::isinf(f)) return true;
                    return false;
                }
                static inline bool IsSpecialFloat(glm::vec2& v) {
                    if (boostm::isnan(v.x) || boostm::isnan(v.y)) return true;
                    if (boostm::isinf(v.x) || boostm::isinf(v.y)) return true;
                    return false;
                }
                static inline bool IsSpecialFloat(glm::vec3& v) {
                    if (boostm::isnan(v.x) || boostm::isnan(v.y) || boostm::isnan(v.z)) return true;
                    if (boostm::isinf(v.x) || boostm::isinf(v.y) || boostm::isinf(v.z)) return true;
                    return false;
                }
                static inline bool GetSimilarVertexIndex(glm::vec3& in_pos, glm::vec2& in_uv, glm::vec3& in_norm, vector<glm::vec3>& pts,vector<glm::vec2>& uvs, vector<glm::vec3>& norms, ushort& result, float& threshold) {
                    for (uint t = 0; t < pts.size(); ++t){
                        if (IsNear(in_pos,pts[t],threshold) && IsNear(in_uv,uvs[t],threshold) && IsNear(in_norm,norms[t],threshold)){
                            result = t;
                            return true;
                        }
                    }
                    return false;
                }
                static void CalculateTBNAssimp(ImportedMeshData& data) {
                    if (data.normals.size() == 0) return;
                    uint dataSize(data.points.size());
                    for (uint i = 0; i < dataSize; i += 3) {
                        uint p0(i + 0); uint p1(i + 1); uint p2(i + 2);

                        glm::vec3 dataP0, dataP1, dataP2;
                        glm::vec2 uvP0, uvP1, uvP2;

                        uint uvSize(data.uvs.size());

                        if (dataSize > p0) dataP0 = data.points[p0];
                        else              dataP0 = glm::vec3(0.0f);
                        if (dataSize > p1) dataP1 = data.points[p1];
                        else              dataP1 = glm::vec3(0.0f);
                        if (dataSize > p2) dataP2 = data.points[p2];
                        else              dataP2 = glm::vec3(0.0f);

                        if (uvSize > p0)   uvP0 = data.uvs[p0];
                        else              uvP0 = glm::vec2(0.0f);
                        if (uvSize > p1)   uvP1 = data.uvs[p1];
                        else              uvP1 = glm::vec2(0.0f);
                        if (uvSize > p2)   uvP2 = data.uvs[p2];
                        else              uvP2 = glm::vec2(0.0f);

                        glm::vec3 v(dataP1 - dataP0);
                        glm::vec3 w(dataP2 - dataP0);

                        // texture offset p1->p2 and p1->p3
                        float sx(uvP1.x - uvP0.x);
                        float sy(uvP1.y - uvP0.y);
                        float tx(uvP2.x - uvP0.x);
                        float ty(uvP2.y - uvP0.y);
                        float dirCorrection = 1.0;

                        if ((tx * sy - ty * sx) < 0.0f) dirCorrection = -1.0f;//this is important for normals and mirrored mesh geometry using identical uv's


                        // when t1, t2, t3 in same position in UV space, just use default UV direction.
                        //if ( 0 == sx && 0 == sy && 0 == tx && 0 == ty ) {
                        if (sx * ty == sy * tx) {
                            sx = 0.0; sy = 1.0;
                            tx = 1.0; ty = 0.0;
                        }

                        // tangent points in the direction where to positive X axis of the texture coord's would point in model space
                        // bitangent's points along the positive Y axis of the texture coord's, respectively
                        glm::vec3 tangent;
                        glm::vec3 bitangent;
                        tangent.x = (w.x * sy - v.x * ty) * dirCorrection;
                        tangent.y = (w.y * sy - v.y * ty) * dirCorrection;
                        tangent.z = (w.z * sy - v.z * ty) * dirCorrection;
                        bitangent.x = (w.x * sx - v.x * tx) * dirCorrection;
                        bitangent.y = (w.y * sx - v.y * tx) * dirCorrection;
                        bitangent.z = (w.z * sx - v.z * tx) * dirCorrection;

                        // store for every vertex of that face
                        for (uint b = 0; b < 3; ++b) {
                            uint p;
                            glm::vec3 normal;
                            if (b == 0)      p = p0;
                            else if (b == 1) p = p1;
                            else          p = p2;

                            if (data.normals.size() > p) normal = data.normals[p];
                            else                        normal = glm::vec3(0.0f);

                            // project tangent and bitangent into the plane formed by the vertex' normal
                            glm::vec3 localTangent(tangent - normal * (tangent   * normal));
                            glm::vec3 localBitangent(bitangent - normal * (bitangent * normal));
                            localTangent = glm::normalize(localTangent);
                            localBitangent = glm::normalize(localBitangent);

                            // reconstruct tangent/bitangent according to normal and bitangent/tangent when it's infinite or NaN.
                            bool invalid_tangent = IsSpecialFloat(localTangent);
                            bool invalid_bitangent = IsSpecialFloat(localBitangent);
                            if (invalid_tangent != invalid_bitangent) {
                                if (invalid_tangent) localTangent = glm::normalize(glm::cross(normal, localBitangent));
                                else                 localBitangent = glm::normalize(glm::cross(localTangent, normal));
                            }
                            data.tangents.push_back(localTangent);
                            data.binormals.push_back(localBitangent);
                        }
                    }
                    for (uint i = 0; i < data.points.size(); ++i) {
                        //hmm.. should b and t be swapped here?
                        glm::vec3& n = data.normals[i];
                        glm::vec3& b = data.tangents[i];
                        glm::vec3& t = data.binormals[i];
                        t = glm::normalize(t - n * glm::dot(n, t)); // Gram-Schmidt orthogonalize
                    }
                }
        };
        struct BoneInfo final {
            glm::mat4 BoneOffset, FinalTransform;
            BoneInfo() {
                BoneOffset = glm::mat4(0.0f);
                FinalTransform = glm::mat4(1.0f);
            }
        };
        struct BoneNode final {
            string Name;
            BoneNode* Parent;
            vector<BoneNode*> Children;
            glm::mat4 Transform;
            BoneNode(){
                Name = "";
                Parent = nullptr;
                Transform = glm::mat4(1.0f);
            }
        };
        class MeshSkeleton final{
            friend class  Mesh;
            friend class  Engine::epriv::AnimationData;
            friend struct Engine::epriv::DefaultMeshBindFunctor;
            friend struct Engine::epriv::DefaultMeshUnbindFunctor;
            private:
                BoneNode*                             m_RootNode;
                uint                                  m_NumBones;
                vector<BoneInfo>                      m_BoneInfo;
                vector<glm::vec4>                     m_BoneIDs, m_BoneWeights;
                unordered_map<string, uint>           m_BoneMapping; // maps a bone name to its index
                unordered_map<string, AnimationData>  m_AnimationData;
                glm::mat4                             m_GlobalInverseTransform;
                void fill(const ImportedMeshData& data) {
                    for (auto& _b : data.m_Bones) {
                        const VertexBoneData& b = _b.second;
                        m_BoneIDs.push_back( glm::vec4(b.IDs[0], b.IDs[1], b.IDs[2], b.IDs[3]));
                        m_BoneWeights.push_back( glm::vec4(b.Weights[0], b.Weights[1], b.Weights[2], b.Weights[3]));
                    }
                }
                void populateCleanupMap(BoneNode* node, unordered_map<string, BoneNode*>& _map) {
                    if (!_map.count(node->Name)) _map.emplace(node->Name, node);
                    for (auto& child : node->Children) {
                        populateCleanupMap(child, _map);
                    }
                }
                void cleanup() {
                    unordered_map<string, BoneNode*> nodes;
                    populateCleanupMap(m_RootNode, nodes);
                    SAFE_DELETE_MAP(nodes);
                }
                void clear() {
                    m_NumBones = 0;
                    m_BoneMapping.clear();
                }
            public:
                MeshSkeleton() { m_RootNode = nullptr; clear(); }
                MeshSkeleton(const ImportedMeshData& data) { fill(data); }
                ~MeshSkeleton() { clear();cleanup(); }
                uint numBones() { return m_NumBones; }
        };
        class CollisionFactory final{
            friend class ::Mesh;
            private:
                void _initConvexData(VertexData& data) {
                    auto& positions = data.getData<glm::vec3>(0);
                    if (!m_ConvexHullData) {
                        m_ConvesHullShape = new btConvexHullShape();
                        for (auto& pos : positions) {
                            m_ConvesHullShape->addPoint(btVector3(pos.x, pos.y, pos.z));
                        }
                        m_ConvexHullData = new btShapeHull(m_ConvesHullShape);
                        m_ConvexHullData->buildHull(m_ConvesHullShape->getMargin());
                        SAFE_DELETE(m_ConvesHullShape);
                        const btVector3* ptsArray = m_ConvexHullData->getVertexPointer();
                        m_ConvesHullShape = new btConvexHullShape();
                        for (int i = 0; i < m_ConvexHullData->numVertices(); ++i) {
                            m_ConvesHullShape->addPoint(btVector3(ptsArray[i].x(), ptsArray[i].y(), ptsArray[i].z()));
                        }
                        m_ConvesHullShape->setMargin(0.001f);
                        m_ConvesHullShape->recalcLocalAabb();
                    }
                }
                void _initTriangleData(VertexData& data) {
                    if (!m_TriangleStaticData) {
                        auto& positions = data.getData<glm::vec3>(0);
                        vector<glm::vec3> triangles;
                        for (auto& indice : data.indices) {
                            triangles.push_back(positions[indice]);
                        }
                        m_TriangleStaticData = new btTriangleMesh();
                        uint count = 0;
                        vector<glm::vec3> tri;
                        for (auto& position : triangles) {
                            tri.push_back(position);
                            ++count;
                            if (count == 3) {
                                btVector3 v1 = Math::btVectorFromGLM(tri[0]);
                                btVector3 v2 = Math::btVectorFromGLM(tri[1]);
                                btVector3 v3 = Math::btVectorFromGLM(tri[2]);
                                m_TriangleStaticData->addTriangle(v1, v2, v3, true);
                                vector_clear(tri);
                                count = 0;
                            }
                        }
                        m_TriangleStaticShape = new btBvhTriangleMeshShape(m_TriangleStaticData, true);
                        m_TriangleStaticShape->setMargin(0.001f);
                        m_TriangleStaticShape->recalcLocalAabb();
                    }
                }
            public:
                Mesh& m_Mesh;
                btShapeHull* m_ConvexHullData;
                btConvexHullShape* m_ConvesHullShape;
                btTriangleMesh* m_TriangleStaticData;
                btBvhTriangleMeshShape* m_TriangleStaticShape;
                CollisionFactory(Mesh& _mesh, VertexData& data):m_Mesh(_mesh){
                    m_ConvexHullData = nullptr;
                    m_ConvesHullShape = nullptr;
                    m_TriangleStaticData = nullptr;
                    m_TriangleStaticShape = nullptr;
                    _initConvexData(data);
                    _initTriangleData(data);
                }
                ~CollisionFactory() {
                    SAFE_DELETE(m_ConvexHullData);
                    SAFE_DELETE(m_ConvesHullShape);
                    SAFE_DELETE(m_TriangleStaticData);
                    SAFE_DELETE(m_TriangleStaticShape);
                }
                btSphereShape* buildSphereShape() {
                    btSphereShape* sphere = new btSphereShape(m_Mesh.getRadius());
                    sphere->setMargin(0.001f);
                    return sphere;
                }
                btBoxShape* buildBoxShape() {
                    btBoxShape* box = new btBoxShape(Math::btVectorFromGLM(m_Mesh.getRadiusBox()));
                    box->setMargin(0.001f);
                    return box;
                }
                btUniformScalingShape* buildConvexHull() {
                    btUniformScalingShape* shape = new btUniformScalingShape(m_ConvesHullShape,1.0f);
                    return shape;
                }
                btScaledBvhTriangleMeshShape* buildTriangleShape() {
                    btScaledBvhTriangleMeshShape* shape = new btScaledBvhTriangleMeshShape(m_TriangleStaticShape,btVector3(1.0f,1.0f,1.0f));
                    return shape;
                }
                btGImpactMeshShape* buildTriangleShapeGImpact() {
                    btGImpactMeshShape* shape = new btGImpactMeshShape(m_TriangleStaticData);
                    shape->setMargin(0.001f);
                    shape->updateBound();
                    return shape;
                }
            };
    };
};



class Mesh::impl final{
    public:
        VertexData*           m_VertexData;
        Engine::epriv::CollisionFactory* m_CollisionFactory;

        epriv::MeshSkeleton* m_Skeleton;
        string m_File;

        glm::vec3 m_radiusBox;
        float m_radius;
        float m_threshold;
        vector<glm::mat4> m_InstanceMatrices;
        uint m_InstanceCount;

        void _initGlobal(float threshold){
            m_InstanceCount = 0;
            m_File = "";
            m_Skeleton = nullptr;
            m_VertexData = nullptr;
            m_threshold = threshold;
        }
        void _initGlobalTwo(Mesh& super,epriv::ImportedMeshData& data,float threshold, VertexData* vertData = nullptr){
            m_VertexData = vertData;
            _finalizeData(data,threshold);
            super.load();
        }
        void _init(Mesh& super,string& name,unordered_map<string,float>& grid,uint width,uint length,float threshold){//grid
            epriv::ImportedMeshData d;
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
            _initGlobalTwo(super,d,threshold);
        }     
        void _init(Mesh& super,string& name,float width, float height,float threshold){//plane
            epriv::ImportedMeshData d;
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

            d.normals.resize(6, glm::vec3(1));  d.binormals.resize(6, glm::vec3(1));  d.tangents.resize(6, glm::vec3(1));
            _initGlobalTwo(super, d, threshold, new VertexData(VertexDataFormat::VertexData2DNoLighting));
        }
        void _init(Mesh& super,string& fileOrData,bool notMemory,float threshold,bool loadNow){//from file / data
            _initGlobal(threshold);
            if(notMemory){
                m_File = fileOrData;
            }else{
                _loadFromOBJMemory(threshold, epriv::LOAD_FACES | epriv::LOAD_UVS | epriv::LOAD_NORMALS | epriv::LOAD_TBN,fileOrData);
            }
            if(loadNow)
                super.load();
        }
        void _loadInternal(Mesh& mesh,epriv::ImportedMeshData& data,string& file){
            Assimp::Importer importer;
            const aiScene* AssimpScene = importer.ReadFile(file,aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
            if(!AssimpScene || AssimpScene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !AssimpScene->mRootNode){
                return;
            }
            //animation stuff
            aiMatrix4x4 m = AssimpScene->mRootNode->mTransformation; // node->mTransformation?
            m.Inverse();
            unordered_map<string, epriv::BoneNode*> map;
            if(AssimpScene->mAnimations && AssimpScene->mNumAnimations > 0 && !m_Skeleton){
                m_Skeleton = new epriv::MeshSkeleton();
                m_Skeleton->m_GlobalInverseTransform = Math::assimpToGLMMat4(m);
            }
            _processNode(mesh,data, *AssimpScene,*AssimpScene->mRootNode, *AssimpScene->mRootNode, map);
            if(m_Skeleton){
                m_Skeleton->fill(data);
            }
        }
        void _processNode(const Mesh& mesh,epriv::ImportedMeshData& data, const aiScene& scene,const aiNode& node, const aiNode& root, unordered_map<string, epriv::BoneNode*>& _map){

            //yes this is needed
            if (m_Skeleton && &node == &root) {
                _populateGlobalNodes(root, _map);
            }

            for(uint i = 0; i < node.mNumMeshes; ++i){
                aiMesh& aimesh = *scene.mMeshes[node.mMeshes[i]];
                #pragma region vertices
                for (uint i = 0; i < aimesh.mNumVertices; ++i) {
                    //pos
                    data.points.emplace_back(aimesh.mVertices[i].x, aimesh.mVertices[i].y, aimesh.mVertices[i].z);
                    //uv
                    if (aimesh.mTextureCoords[0]) {
                        //this is to prevent uv compression from beign f-ed up at the poles.
                        //if(aimesh.mTextureCoords[0][i].y <= 0.0001f){ aimesh.mTextureCoords[0][i].y = 0.001f; }
                        //if(aimesh.mTextureCoords[0][i].y >= 0.9999f){ aimesh.mTextureCoords[0][i].y = 0.999f; }
                        data.uvs.emplace_back(aimesh.mTextureCoords[0][i].x, aimesh.mTextureCoords[0][i].y);
                    }else{
                        data.uvs.emplace_back(0.0f, 0.0f);
                    }
                    if (aimesh.mNormals) {
                        data.normals.emplace_back(aimesh.mNormals[i].x, aimesh.mNormals[i].y, aimesh.mNormals[i].z);
                    }
                    if (aimesh.mTangents) {
                        //data.tangents.emplace_back(aimesh.mTangents[i].x,aimesh.mTangents[i].y,aimesh.mTangents[i].z);
                    }
                    if (aimesh.mBitangents) {
                        //data.binormals.emplace_back(aimesh.mBitangents[i].x,aimesh.mBitangents[i].y,aimesh.mBitangents[i].z);
                    }
                }
                #pragma endregion
                // Process indices
                #pragma region indices
                for(uint i = 0; i < aimesh.mNumFaces; ++i){
                    aiFace& face = aimesh.mFaces[i];

                    uint& index0 = face.mIndices[0];
                    uint& index1 = face.mIndices[1];
                    uint& index2 = face.mIndices[2];

                    data.indices.push_back(index0);
                    data.indices.push_back(index1);
                    data.indices.push_back(index2);

                    epriv::Vertex v1,v2,v3;

                    v1.position = data.points[index0];
                    v2.position = data.points[index1];
                    v3.position = data.points[index2];
                    if(data.uvs.size() > 0){ 
                        v1.uv = data.uvs[index0];
                        v2.uv = data.uvs[index1];
                        v3.uv = data.uvs[index2];
                    }
                    if(data.normals.size() > 0){
                        v1.normal = data.normals[index0];
                        v2.normal = data.normals[index1];
                        v3.normal = data.normals[index2];
                    }
                    data.file_triangles.emplace_back(v1,v2,v3);

                }
                #pragma endregion
                //bones
                #pragma region Skeleton
                if(aimesh.mNumBones > 0){
                    auto& skeleton = *m_Skeleton;
                    #pragma region IndividualBones
                    //build bone information
                    for (uint k = 0; k < aimesh.mNumBones; ++k) {
                        auto* boneNode = _map.at(aimesh.mBones[k]->mName.data);
                        auto& assimpBone = *aimesh.mBones[k];
                        uint BoneIndex(0);
                        if(!skeleton.m_BoneMapping.count(boneNode->Name)) {
                            BoneIndex = skeleton.m_NumBones;
                            ++skeleton.m_NumBones; 
                            skeleton.m_BoneInfo.emplace_back();
                        }else{
                            BoneIndex = skeleton.m_BoneMapping.at(boneNode->Name);
                        }
                        skeleton.m_BoneMapping.emplace(boneNode->Name,BoneIndex);
                        skeleton.m_BoneInfo[BoneIndex].BoneOffset = Math::assimpToGLMMat4(assimpBone.mOffsetMatrix);
                        for (uint j = 0; j < assimpBone.mNumWeights; ++j) {
                            uint VertexID = assimpBone.mWeights[j].mVertexId;
                            float Weight = assimpBone.mWeights[j].mWeight;
                            epriv::VertexBoneData d;
                            d.AddBoneData(BoneIndex, Weight);
                            data.m_Bones.emplace(VertexID,d);
                        }
                    }	
                    //build skeleton parent child relationship
                    for (auto& node : _map) {
                        const auto& assimpNode = root.FindNode(node.first.c_str());
                        auto iter = assimpNode;
                        while (iter != 0 && iter->mParent != 0){
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
                    if(scene.mAnimations && scene.mNumAnimations > 0){
                        for(uint j = 0; j < scene.mNumAnimations; ++j){			 
                             const aiAnimation& anim = *scene.mAnimations[j];
                             string key(anim.mName.C_Str());
                             if(key == ""){
                                 key = "Animation " + to_string(skeleton.m_AnimationData.size());
                             }
                             if(!skeleton.m_AnimationData.count(key)){
                                 skeleton.m_AnimationData.emplace(std::piecewise_construct,std::forward_as_tuple(key),std::forward_as_tuple(mesh, anim));
                             }
                        }
                    }
                    #pragma endregion
                }
                #pragma endregion
                epriv::MeshLoader::CalculateTBNAssimp(data);
            }
            for(uint i = 0; i < node.mNumChildren; ++i){
                _processNode(mesh, data, scene, *node.mChildren[i],*scene.mRootNode, _map);
            }
        }
        void _populateGlobalNodes(const aiNode& node, unordered_map<string, epriv::BoneNode*>& _map) {
            if (!_map.count(node.mName.data)) {
                epriv::BoneNode* bone_node = new epriv::BoneNode();
                bone_node->Name = node.mName.data;
                bone_node->Transform = Math::assimpToGLMMat4(const_cast<aiMatrix4x4&>(node.mTransformation));
                _map.emplace(node.mName.data, bone_node);
            }
            for (uint y = 0; y < node.mNumChildren; ++y) {
                _populateGlobalNodes(*node.mChildren[y], _map);
            }
        }
        void _finalizeData(epriv::ImportedMeshData& data,float threshold){
            m_threshold = threshold;

            if(data.uvs.size() == 0)         data.uvs.resize(data.points.size());
            if(data.normals.size() == 0)     data.normals.resize(data.points.size());
            if(data.binormals.size() == 0)   data.binormals.resize(data.points.size());
            if(data.tangents.size() == 0)    data.tangents.resize(data.points.size());

            if (!m_VertexData) {
                if (m_Skeleton) {
                    m_VertexData = new VertexData(VertexDataFormat::VertexDataAnimated);
                }else{
                    m_VertexData = new VertexData(VertexDataFormat::VertexDataBasic);
                }
            }

            auto& vertexData = *m_VertexData;
            vector<vector<GLuint>> normals; normals.resize(3);
            if (m_threshold == 0.0f) { 
                normals[0].reserve(data.normals.size()); normals[1].reserve(data.binormals.size()); normals[2].reserve(data.tangents.size());
                for (size_t i = 0; i < data.normals.size(); ++i) normals[0].push_back(Math::pack3NormalsInto32Int(data.normals[i]));
                for (size_t i = 0; i < data.binormals.size(); ++i) normals[1].push_back(Math::pack3NormalsInto32Int(data.binormals[i]));
                for (size_t i = 0; i < data.tangents.size(); ++i) normals[2].push_back(Math::pack3NormalsInto32Int(data.tangents[i]));
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
                normals[0].reserve(temp_normals.size()); normals[1].reserve(temp_binormals.size()); normals[2].reserve(temp_tangents.size());
                for (size_t i = 0; i < temp_normals.size(); ++i) normals[0].push_back(Math::pack3NormalsInto32Int(temp_normals[i]));
                for (size_t i = 0; i < temp_binormals.size(); ++i) normals[1].push_back(Math::pack3NormalsInto32Int(temp_binormals[i]));
                for (size_t i = 0; i < temp_tangents.size(); ++i) normals[2].push_back(Math::pack3NormalsInto32Int(temp_tangents[i]));
                vertexData.setData(0, temp_pos);
                vertexData.setData(1, temp_uvs);
                vertexData.setData(2, normals[0]);
                vertexData.setData(3, normals[1]);
                vertexData.setData(4, normals[2]);
                vertexData.setDataIndices(_indices);
            }
            if (m_Skeleton) {
                vector<vector<glm::vec4>> boneStuff; boneStuff.resize(2);
                auto& _skeleton = *m_Skeleton;
                boneStuff[0].reserve(_skeleton.m_BoneIDs.size()); boneStuff[1].reserve(_skeleton.m_BoneIDs.size());   
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
            epriv::ImportedMeshData d;

            if (extension == ".objc") {
                _readFromObjCompressed(file, d);
                _finalizeData(d, threshold);
            }else if (extension == ".objcc") {
                _readFromObjCompressed1(file);
            }else{
                if (extension == ".obj")
                    _writeToObjCompressed1(); 
                _loadInternal(super, d, m_File);
                _finalizeData(d, threshold);
            }
        }
        void _loadDataIntoTriangles(epriv::ImportedMeshData& data,vector<uint>& _pi,vector<uint>& _ui,vector<uint>& _ni,unsigned char _flags){
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
        void _loadObjDataFromLine(string& l,epriv::ImportedMeshData& _d,vector<uint>& _pi,vector<uint>& _ui,vector<uint>& _ni,const char _f){
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
            epriv::ImportedMeshData d;
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
        void _calculateMeshRadius(Mesh& super){
            float maxX = 0; float maxY = 0; float maxZ = 0;
            auto& data = m_VertexData->getData<glm::vec3>(0);
            for(auto& _vertex : data){
                float x = abs(_vertex.x); float y = abs(_vertex.y); float z = abs(_vertex.z);
                if(x > maxX) maxX = x; if(y > maxY) maxY = y; if(z > maxZ) maxZ = z;
            }
            m_radiusBox = glm::vec3(maxX,maxY,maxZ);
            m_radius = Math::Max(m_radiusBox);
        }
        void _modifyPoints(vector<glm::vec3>& modifiedPts){
            m_VertexData->setData(0, modifiedPts, true, false);
        }
        void _modifyUVs(vector<glm::vec2>& modifiedUVs){
            m_VertexData->setData(1, modifiedUVs, true, false);
        }
        void _modifyPointsAndUVs(vector<glm::vec3>& modifiedPts,vector<glm::vec2>& modifiedUVs){
            m_VertexData->setData(0, modifiedPts, true, false);
            m_VertexData->setData(1, modifiedUVs, true, false);
        }
        void _modifyIndices(vector<ushort>& modifiedIndices) {
            m_VertexData->setDataIndices(modifiedIndices, true);
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
            _calculateMeshRadius(super);
            m_CollisionFactory = new Engine::epriv::CollisionFactory(super,*m_VertexData);
        }
        void _load_GPU(){
            m_VertexData->finalize();
            cout << "(Mesh) ";
        }
        //slow loading, but smaller file size than .objcc
        void _readFromObjCompressed(string& filename, epriv::ImportedMeshData& data) {
            boost::iostreams::mapped_file_source stream(filename.c_str());
            //TODO: try possible optimizations

            uint count1 = 0;
            uint count = 12;
            const uint8_t* _data = (uint8_t*)stream.data();

            uint32_t sizes[6];
            for (uint i = 0; i < 6; ++i) {
                const uint& _count = count1 * 4;
                sizes[i]  = (uint32_t)_data[_count    ] << 24;
                sizes[i] |= (uint32_t)_data[_count + 1] << 16;
                sizes[i] |= (uint32_t)_data[_count + 2] << 8;
                sizes[i] |= (uint32_t)_data[_count + 3];
                ++count1;
            }
            data.file_points.reserve(sizes[0]);
            data.file_uvs.reserve(sizes[1]);
            data.file_normals.reserve(sizes[2]);

            vector<vector<uint>> _indices; _indices.resize(3);
            _indices[0].reserve(sizes[3]);
            _indices[1].reserve(sizes[4]);
            _indices[2].reserve(sizes[5]);

            //positions
            for (uint i = 0; i < sizes[0]; ++i) {
                float out[3];
                uint16_t in[3];
                for (uint j = 0; j < 3; ++j) {
                    const uint& _count = count * 2;
                    in[j]  = (uint32_t)_data[_count    ] << 8;
                    in[j] |= (uint32_t)_data[_count + 1];
                    ++count;
                }
                float32(&out[0], in[0]);
                float32(&out[1], in[1]);
                float32(&out[2], in[2]);
                data.file_points.emplace_back(out[0], out[1], out[2]);
            }
            //uvs
            for (uint i = 0; i < sizes[1]; ++i) {
                float out[2];
                uint16_t in[2];
                for (uint j = 0; j < 2; ++j) {
                    const uint& _count = count * 2;
                    in[j]  = (uint32_t)_data[_count    ] << 8;
                    in[j] |= (uint32_t)_data[_count + 1];
                    ++count;
                }
                float32(&out[0], in[0]);
                float32(&out[1], in[1]);
                data.file_uvs.emplace_back(out[0], out[1]);
            }
            //normals
            for (uint i = 0; i < sizes[2]; ++i) {
                float out[3];
                uint16_t in[3];
                for (uint j = 0; j < 3; ++j) {
                    const uint& _count = count * 2;
                    in[j]  = (uint32_t)_data[_count    ] << 8;
                    in[j] |= (uint32_t)_data[_count + 1];
                    ++count;
                }
                float32(&out[0], in[0]);
                float32(&out[1], in[1]);
                float32(&out[2], in[2]);
                data.file_normals.emplace_back(out[0], out[1], out[2]);
            }
            //indices
            for (uint i = 0; i < _indices.size(); ++i) {
                for (uint j = 0; j < sizes[3+i]; ++j) {
                    uint16_t c;
                    const uint& _count = count * 2;
                    c  = (uint32_t)_data[_count     ] << 8;
                    c |= (uint32_t)_data[_count + 1];
                    _indices[i].emplace_back(c);
                    ++count;
                }
            }
            stream.close();
            _loadDataIntoTriangles(data, _indices[0], _indices[1], _indices[2], epriv::LOAD_FACES | epriv::LOAD_UVS | epriv::LOAD_NORMALS | epriv::LOAD_TBN);
            epriv::MeshLoader::CalculateTBNAssimp(data);
        }
        //slow loading, but smaller file size than .objcc
        void _writeToObjCompressed() {
            epriv::ImportedMeshData d;
            vector<vector<uint>> _indices; _indices.resize(3);
            ifstream input(m_File);

            //first read in all data
            for (string line; getline(input, line, '\n');) {
                _loadObjDataFromLine(line, d, _indices[0], _indices[1], _indices[2], epriv::LOAD_FACES | epriv::LOAD_UVS | epriv::LOAD_NORMALS | epriv::LOAD_TBN);
            }
            //header:
            string f = m_File;
            string ext = boost::filesystem::extension(m_File);
            f = f.substr(0, f.size() - ext.size()); f += ".objc";
            ofstream stream(f,ios::binary);

            //header
            uint32_t sizes[6];
            sizes[0] = d.file_points.size();
            sizes[1] = d.file_uvs.size();
            sizes[2] = d.file_normals.size();
            for (uint i = 0; i < _indices.size(); ++i) {
                sizes[3+i] = _indices[i].size();
            }
            for (uint i = 0; i < 6; ++i) {
                writeUint32tBigEndian(sizes[i], stream);
            }
            for (auto& pos : d.file_points) {
                uint16_t out[3];
                float16(&out[0], pos.x); float16(&out[1], pos.y); float16(&out[2], pos.z);
                for (uint i = 0; i < 3; ++i) {
                    writeUint16tBigEndian(out[i], stream);
                }
            }
            for (auto& uv : d.file_uvs) {
                uint16_t out[2];
                float16(&out[0], uv.x); float16(&out[1], uv.y);
                for (uint i = 0; i < 2; ++i) {
                    writeUint16tBigEndian(out[i], stream);
                }
            }
            for (auto& norm : d.file_normals) {
                uint16_t out[3];
                float16(&out[0], norm.x); float16(&out[1], norm.y); float16(&out[2], norm.z);
                for (uint i = 0; i < 3; ++i) {
                    writeUint16tBigEndian(out[i], stream);
                }
            }
            //indices
            for (uint i = 0; i < _indices.size(); ++i) {
                for (auto& ind : _indices[i]) {
                    uint16_t _ind = (uint16_t)ind;
                    writeUint16tBigEndian(_ind, stream);
                }
            }
            stream.close();
        }  
        //very fast loading, might be slightly larger than .objc
        void _readFromObjCompressed1(string& filename) {
            boost::iostreams::mapped_file_source stream(filename.c_str());
            //TODO: try possible optimizations

            uint blockStart = 0;
            const uint8_t* _data = (uint8_t*)stream.data();

            uint32_t sizes[3];
            for (uint i = 0; i < 3; ++i) {
                sizes[i]  = (uint32_t)_data[blockStart    ] << 24;
                sizes[i] |= (uint32_t)_data[blockStart + 1] << 16;
                sizes[i] |= (uint32_t)_data[blockStart + 2] << 8;
                sizes[i] |= (uint32_t)_data[blockStart + 3];
                blockStart += 4;
            }

            if (sizes[2] == 1) {
                m_VertexData = new VertexData(VertexDataFormat::VertexDataAnimated);
            }else{
                m_VertexData = new VertexData(VertexDataFormat::VertexDataBasic);
            }

            auto& data = *m_VertexData;
            data.indices.reserve(sizes[1]);
            vector<glm::vec3> temp_pos; temp_pos.reserve(sizes[0]);
            vector<glm::vec2> temp_uvs; temp_uvs.reserve(sizes[0]);
            vector<GLuint> temp_norm; temp_norm.reserve(sizes[0]);
            vector<GLuint> temp_binorm; temp_binorm.reserve(sizes[0]);
            vector<GLuint> temp_tang; temp_tang.reserve(sizes[0]);
            vector<glm::vec4> temp_bID; temp_bID.reserve(sizes[0]);
            vector<glm::vec4> temp_bW; temp_bW.reserve(sizes[0]);

            for (uint i = 0; i < sizes[0]; ++i) {
                //positions
                float outPos[3];
                uint16_t inPos[3];
                for (uint j = 0; j < 3; ++j) {
                    inPos[j] = (uint32_t)_data[blockStart] << 8;
                    inPos[j] |= (uint32_t)_data[blockStart + 1];
                    blockStart += 2;
                }
                float32(&outPos[0], inPos[0]); float32(&outPos[1], inPos[1]); float32(&outPos[2], inPos[2]);
                temp_pos.emplace_back(outPos[0], outPos[1], outPos[2]);
                //uvs
                float outUV[2];
                uint16_t inUV[2];
                for (uint j = 0; j < 2; ++j) {
                    inUV[j] = (uint32_t)_data[blockStart] << 8;
                    inUV[j] |= (uint32_t)_data[blockStart + 1];
                    blockStart += 2;
                }
                float32(&outUV[0], inUV[0]);
                float32(&outUV[1], inUV[1]);
                temp_uvs.emplace_back(outUV[0], outUV[1]);
                //normals (remember they are GLuints right now)
                uint32_t inn[3];
                for (uint i = 0; i < 3; ++i) {
                    inn[i] = (uint32_t)_data[blockStart] << 24;
                    inn[i] |= (uint32_t)_data[blockStart + 1] << 16;
                    inn[i] |= (uint32_t)_data[blockStart + 2] << 8;
                    inn[i] |= (uint32_t)_data[blockStart + 3];
                    blockStart += 4;
                }
                temp_norm.emplace_back(inn[0]);
                temp_binorm.emplace_back(inn[1]);
                temp_tang.emplace_back(inn[2]);
                if (sizes[2] == 1) { //skeleton is present
                    //boneID's
                    float outBI[4];
                    uint16_t inbI[4];
                    for (uint j = 0; j < 4; ++j) {
                        inbI[j] = (uint32_t)_data[blockStart] << 8;
                        inbI[j] |= (uint32_t)_data[blockStart + 1];
                        blockStart += 2;
                    }
                    float32(&outBI[0], inbI[0]);
                    float32(&outBI[1], inbI[1]);
                    float32(&outBI[2], inbI[2]);
                    float32(&outBI[3], inbI[3]);
                    temp_bID.emplace_back(outBI[0], outBI[1], outBI[2], outBI[3]);
                    //boneWeight's
                    float outBW[4];
                    uint16_t inBW[4];
                    for (uint j = 0; j < 4; ++j) {
                        inBW[j] = (uint32_t)_data[blockStart] << 8;
                        inBW[j] |= (uint32_t)_data[blockStart + 1];
                        blockStart += 2;
                    }
                    float32(&outBW[0], inBW[0]);
                    float32(&outBW[1], inBW[1]);
                    float32(&outBW[2], inBW[2]);
                    float32(&outBW[3], inBW[3]);
                    temp_bW.emplace_back(outBW[0], outBW[1], outBW[2], outBW[3]);
                }
            }
            //indices
            for (uint i = 0; i < sizes[1]; ++i) {
                uint16_t inindices;
                inindices  = (uint32_t)_data[blockStart    ] << 8;
                inindices |= (uint32_t)_data[blockStart + 1];
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
        }

        //very fast loading, might be slightly larger than .objc
        void _writeToObjCompressed1() {
            string f = m_File;
            string ext = boost::filesystem::extension(m_File);
            f = (f.substr(0, f.size() - ext.size())) + ".objcc";
            ofstream stream(f, ios::binary);

            epriv::ImportedMeshData d;
            auto& data = *m_VertexData;

            vector<vector<uint>> _indices;
            _indices.resize(3);

            ifstream input(m_File);

            //first read in all data
            for (string line; getline(input, line, '\n');) {
                _loadObjDataFromLine(line, d, _indices[0], _indices[1], _indices[2], epriv::LOAD_FACES | epriv::LOAD_UVS | epriv::LOAD_NORMALS | epriv::LOAD_TBN);
            }
            //then finalize
            _loadDataIntoTriangles(d, _indices[0], _indices[1], _indices[2], epriv::LOAD_FACES | epriv::LOAD_UVS | epriv::LOAD_NORMALS | epriv::LOAD_TBN);
            epriv::MeshLoader::CalculateTBNAssimp(d);
            _finalizeData(d, 0.0005f);

            //header - should only be 3 entries, one for m_Vertices , one for m_Indices, and one to tell if skeleton or not
            uint32_t sizes[3];
            sizes[0] = data.dataSizes[0];
            sizes[1] = data.indices.size();
            sizes[2] = (m_Skeleton) ? 1 : 0;

            for (uint i = 0; i < 3; ++i) {
                writeUint32tBigEndian(sizes[i], stream);
            }
            auto& positions = data.getData<glm::vec3>(0);
            auto& uvs = data.getData<glm::vec2>(0);
            auto& normals = data.getData<GLuint>(0);
            auto& binormals = data.getData<GLuint>(0);
            auto& tangents = data.getData<GLuint>(0);

            if (m_Skeleton) {
                auto& boneIDs = data.getData<glm::vec4>(0);
                auto& boneWeights = data.getData<glm::vec4>(0);
                for (size_t j = 0; j < sizes[0]; ++j) {
                    const auto& position = positions[j];
                    const auto& uv = uvs[j];
                    const auto& normal = normals[j];
                    const auto& binormal = binormals[j];
                    const auto& tangent = tangents[j];
                    const auto& boneID = boneIDs[j];
                    const auto& boneWeight = boneWeights[j];

                    //positions
                    uint16_t outp[3];
                    float16(&outp[0], position.x);  float16(&outp[1], position.y);  float16(&outp[2], position.z);
                    for (uint i = 0; i < 3; ++i) {
                        writeUint16tBigEndian(outp[i], stream);
                    }
                    //uvs
                    uint16_t outu[2];
                    float16(&outu[0], uv.x);  float16(&outu[1], uv.y);
                    for (uint i = 0; i < 2; ++i) {
                        writeUint16tBigEndian(outu[i], stream);
                    }
                    //normals (remember they are GLuints right now)
                    uint32_t outn[3];
                    outn[0] = normal;  outn[1] = binormal;  outn[2] = tangent;
                    for (uint i = 0; i < 3; ++i) {
                        writeUint32tBigEndian(outn[i], stream);
                    }
                    //boneID's
                    uint16_t outbI[4];
                    float16(&outbI[0], boneID.x);  float16(&outbI[1], boneID.y); float16(&outbI[2], boneID.z);  float16(&outbI[3], boneID.w);
                    for (uint i = 0; i < 4; ++i) {
                        writeUint16tBigEndian(outbI[i], stream);
                    }
                    //boneWeight's
                    uint16_t outbW[4];
                    float16(&outbW[0], boneWeight.x);  float16(&outbW[1], boneWeight.y); float16(&outbW[2], boneWeight.z);  float16(&outbW[3], boneWeight.w);
                    for (uint i = 0; i < 4; ++i) {
                        writeUint16tBigEndian(outbW[i], stream);
                    }
                }
            }else{
                for (size_t j = 0; j < sizes[0]; ++j) {
                    const auto& position = positions[j];
                    const auto& uv = uvs[j];
                    const auto& normal = normals[j];
                    const auto& binormal = binormals[j];
                    const auto& tangent = tangents[j];
                    //positions
                    uint16_t outp[3];
                    float16(&outp[0], position.x);  float16(&outp[1], position.y);  float16(&outp[2], position.z);
                    for (uint i = 0; i < 3; ++i) {
                        writeUint16tBigEndian(outp[i], stream);
                    }
                    //uvs
                    uint16_t outu[2];
                    float16(&outu[0], uv.x);  float16(&outu[1], uv.y);
                    for (uint i = 0; i < 2; ++i) {
                        writeUint16tBigEndian(outu[i], stream);
                    }
                    //normals (remember they are GLuints right now)
                    uint32_t outn[3];
                    outn[0] = normal;  outn[1] = binormal;  outn[2] = tangent;
                    for (uint i = 0; i < 3; ++i) {
                        writeUint32tBigEndian(outn[i], stream);
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
};

namespace Engine {
    namespace epriv {
        struct DefaultMeshBindFunctor final{void operator()(BindableResource* r) const {
            auto& m = *((Mesh*)r)->m_i;
            m.m_VertexData->bind();
        }};
        struct DefaultMeshUnbindFunctor final {void operator()(BindableResource* r) const {
            auto& m = *((Mesh*)r)->m_i;
            m.m_VertexData->unbind();
        }};
    };
};

epriv::AnimationData::AnimationData(const Mesh& mesh,const aiAnimation& assimpAnimation){
    m_Mesh = const_cast<Mesh*>(&mesh);
    m_TicksPerSecond = assimpAnimation.mTicksPerSecond;
    m_DurationInTicks = assimpAnimation.mDuration;
    for (uint o = 0; o < assimpAnimation.mNumChannels; ++o) {
        const aiNodeAnim& aiAnimNode = *assimpAnimation.mChannels[o];
        if (!m_KeyframeData.count(aiAnimNode.mNodeName.data)) {
            AnimationChannel animChannel;
            for (uint b = 0; b < aiAnimNode.mNumPositionKeys; ++b) {
                animChannel.PositionKeys.emplace_back(aiAnimNode.mPositionKeys[b].mTime, Math::assimpToGLMVec3(aiAnimNode.mPositionKeys[b].mValue));
            }
            for (uint b = 0; b < aiAnimNode.mNumRotationKeys; ++b) {
                animChannel.RotationKeys.emplace_back(aiAnimNode.mRotationKeys[b].mTime, aiAnimNode.mRotationKeys[b].mValue);
            }
            for (uint b = 0; b < aiAnimNode.mNumScalingKeys; ++b) {
                animChannel.ScalingKeys.emplace_back(aiAnimNode.mScalingKeys[b].mTime, Math::assimpToGLMVec3(aiAnimNode.mScalingKeys[b].mValue));
            }
            m_KeyframeData.emplace(aiAnimNode.mNodeName.data, std::move(animChannel));
        }
    }
}
epriv::AnimationData::~AnimationData() { 
    m_KeyframeData.clear();
}
void epriv::AnimationData::ReadNodeHeirarchy(const string& animationName, float time, const BoneNode* node, glm::mat4& ParentTransform, vector<glm::mat4>& Transforms) {
    string BoneName(node->Name);
    glm::mat4 NodeTransform(node->Transform);
    if (m_KeyframeData.count(BoneName)) {
        const auto keyframes(m_KeyframeData.at(BoneName));
        glm::vec3 s; CalcInterpolatedScaling(s, time, keyframes);
        aiQuaternion q; CalcInterpolatedRotation(q, time, keyframes);
        glm::mat4 rotation(Math::assimpToGLMMat3(q.GetMatrix()));
        glm::vec3 t; CalcInterpolatedPosition(t, time, keyframes);
        NodeTransform = glm::mat4(1.0f);
        NodeTransform = glm::translate(NodeTransform, t);
        NodeTransform *= rotation;
        NodeTransform = glm::scale(NodeTransform, s);
    }
    glm::mat4 Transform(ParentTransform * NodeTransform);
    auto& skeleton = *m_Mesh->m_i->m_Skeleton;
    if (skeleton.m_BoneMapping.count(BoneName)) {
        uint BoneIndex(skeleton.m_BoneMapping.at(BoneName));
        BoneInfo& boneInfo = skeleton.m_BoneInfo[BoneIndex];
        glm::mat4& Final = boneInfo.FinalTransform;
        Final = skeleton.m_GlobalInverseTransform * Transform * boneInfo.BoneOffset;
        //this line allows for animation combinations. only works when additional animations start off in their resting places...
        Final = Transforms[BoneIndex] * Final;
    }
    for (uint i = 0; i < node->Children.size(); ++i) {
        ReadNodeHeirarchy(animationName, time, node->Children[i], Transform, Transforms);
    }
}
void epriv::AnimationData::BoneTransform(const string& animationName, float TimeInSeconds, vector<glm::mat4>& Transforms) {
    float TicksPerSecond = float(m_TicksPerSecond != 0 ? m_TicksPerSecond : 25.0f);
    float TimeInTicks(TimeInSeconds * TicksPerSecond);
    float AnimationTime(float(fmod(TimeInTicks, m_DurationInTicks)));
    glm::mat4 ParentIdentity(1.0f);
    auto& skeleton = *m_Mesh->m_i->m_Skeleton;
    ReadNodeHeirarchy(animationName, AnimationTime, skeleton.m_RootNode, ParentIdentity, Transforms);
    for (uint i = 0; i < skeleton.m_NumBones; ++i) {
        Transforms[i] = skeleton.m_BoneInfo[i].FinalTransform;
    }
}
void epriv::AnimationData::CalcInterpolatedPosition(glm::vec3& Out, float AnimationTime, const AnimationChannel& node) {
    if (node.PositionKeys.size() == 1) {
        Out = node.PositionKeys[0].value; return;
    }
    uint PositionIndex(FindPosition(AnimationTime, node));
    uint NextIndex(PositionIndex + 1);
    float DeltaTime((float)(node.PositionKeys[NextIndex].time - node.PositionKeys[PositionIndex].time));
    float Factor((AnimationTime - (float)node.PositionKeys[PositionIndex].time) / DeltaTime);
    glm::vec3 Start(node.PositionKeys[PositionIndex].value);
    glm::vec3 End(node.PositionKeys[NextIndex].value);
    Out = Start + Factor * (End - Start);
}
void epriv::AnimationData::CalcInterpolatedRotation(aiQuaternion& Out, float AnimationTime, const AnimationChannel& node) {
    if (node.RotationKeys.size() == 1) {
        Out = node.RotationKeys[0].value; return;
    }
    uint RotationIndex(FindRotation(AnimationTime, node));
    uint NextIndex(RotationIndex + 1);
    float DeltaTime((float)(node.RotationKeys[NextIndex].time - node.RotationKeys[RotationIndex].time));
    float Factor((AnimationTime - (float)node.RotationKeys[RotationIndex].time) / DeltaTime);
    const aiQuaternion& StartRotationQ = node.RotationKeys[RotationIndex].value;
    const aiQuaternion& EndRotationQ = node.RotationKeys[NextIndex].value;
    aiQuaternion::Interpolate(Out, StartRotationQ, EndRotationQ, Factor);
    Out = Out.Normalize();
}
void epriv::AnimationData::CalcInterpolatedScaling(glm::vec3& Out, float AnimationTime, const AnimationChannel& node) {
    if (node.ScalingKeys.size() == 1) {
        Out = node.ScalingKeys[0].value; return;
    }
    uint ScalingIndex(FindScaling(AnimationTime, node));
    uint NextIndex(ScalingIndex + 1);
    float DeltaTime((float)(node.ScalingKeys[NextIndex].time - node.ScalingKeys[ScalingIndex].time));
    float Factor((AnimationTime - (float)node.ScalingKeys[ScalingIndex].time) / DeltaTime);
    glm::vec3 Start(node.ScalingKeys[ScalingIndex].value);
    glm::vec3 End(node.ScalingKeys[NextIndex].value);
    Out = Start + Factor * (End - Start);
}
uint epriv::AnimationData::FindPosition(float AnimationTime, const AnimationChannel& node) {
    for (uint i = 0; i < node.PositionKeys.size() - 1; ++i) { if (AnimationTime < (float)node.PositionKeys[i + 1].time) { return i; } }return 0;
}
uint epriv::AnimationData::FindRotation(float AnimationTime, const AnimationChannel& node) {
    for (uint i = 0; i < node.RotationKeys.size() - 1; ++i) { if (AnimationTime < (float)node.RotationKeys[i + 1].time) { return i; } }return 0;
}
uint epriv::AnimationData::FindScaling(float AnimationTime, const AnimationChannel& node) {
    for (uint i = 0; i < node.ScalingKeys.size() - 1; ++i) { if (AnimationTime < (float)node.ScalingKeys[i + 1].time) { return i; } }return 0;
}
float epriv::AnimationData::duration() { 
    float TicksPerSecond(float(m_TicksPerSecond != 0 ? m_TicksPerSecond : 25.0f)); return float(float(m_DurationInTicks) / TicksPerSecond);
}


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

const glm::vec3& Mesh::getRadiusBox() const { return m_i->m_radiusBox; }
const float Mesh::getRadius() const { return m_i->m_radius; }
void Mesh::render(bool instancing, MeshDrawMode::Mode mode){
    auto& i = *m_i;
    const uint& indicesSize = i.m_VertexData->indices.size();
    if(instancing && epriv::InternalMeshPublicInterface::SupportsInstancing()){
        const uint& instancesCount = i.m_InstanceCount;
        if(instancesCount == 0) return;
        if(epriv::RenderManager::OPENGL_VERSION >= 31){
               glDrawElementsInstanced(mode, indicesSize, GL_UNSIGNED_SHORT, 0, instancesCount);
        }else if(epriv::OpenGLExtensionEnum::supported(epriv::OpenGLExtensionEnum::EXT_draw_instanced)){
            glDrawElementsInstancedEXT(mode, indicesSize, GL_UNSIGNED_SHORT, 0, instancesCount);
        }else if(epriv::OpenGLExtensionEnum::supported(epriv::OpenGLExtensionEnum::ARB_draw_instanced)){
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
void Mesh::modifyPoints(vector<glm::vec3>& modifiedPts){ m_i->_modifyPoints(modifiedPts); }
void Mesh::modifyUVs(vector<glm::vec2>& modifiedUVs){ m_i->_modifyUVs(modifiedUVs); }
void Mesh::modifyPointsAndUVs(vector<glm::vec3>& modifiedPts, vector<glm::vec2>& modifiedUVs){ m_i->_modifyPointsAndUVs(modifiedPts,modifiedUVs); }
void Mesh::modifyIndices(vector<ushort>& modifiedIndices) { m_i->_modifyIndices(modifiedIndices); }
void Mesh::onEvent(const Event& e) {
    if (e.type == EventType::WindowFullscreenChanged) {
        auto& i = *m_i;
        i.m_VertexData->finalize();
    }
}