#include "Engine_Resources.h"
#include "Engine_Renderer.h"
#include "Mesh.h"
#include "MeshInstance.h"
#include "Engine_ThreadManager.h"

#include <bullet/btBulletDynamicsCommon.h>
#include <bullet/BulletCollision/CollisionShapes/btHeightfieldTerrainShape.h>
#include <glm/gtc/matrix_transform.hpp>

#include <boost/filesystem.hpp>
#include <boost/tuple/tuple.hpp>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <boost/math/special_functions/fpclassify.hpp>

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
                       //= 32
        };
        struct MeshVertexData{
            glm::vec3 position;
            //float uv;
            glm::vec2 uv;
            GLuint normal;
            GLuint binormal;
            GLuint tangent;
            MeshVertexData(){ }
            MeshVertexData(const MeshVertexData& c){
                position=c.position; uv=c.uv; normal=c.normal; binormal=c.binormal; tangent=c.tangent;
            }
            ~MeshVertexData(){ }
        };
        struct MeshVertexDataAnimated final: public MeshVertexData{
            glm::vec4 boneIDs;
            glm::vec4 boneWeights;
            MeshVertexDataAnimated():MeshVertexData(){ }
            MeshVertexDataAnimated(const MeshVertexData& c){
                position=c.position; uv=c.uv; normal=c.normal; binormal=c.binormal; tangent=c.tangent;
            }
            MeshVertexDataAnimated(const MeshVertexDataAnimated& c){
                position=c.position; uv=c.uv; normal=c.normal; binormal=c.binormal; tangent=c.tangent; boneIDs=c.boneIDs; boneWeights=c.boneWeights;
            }
            ~MeshVertexDataAnimated(){ }
        };
        unordered_map<uint,boost::tuple<uint,GLuint,GLuint,GLuint>> VERTEX_ANIMATED_FORMAT_DATA = [](){
            unordered_map<uint,boost::tuple<uint,GLuint,GLuint,GLuint>> m;
            m[VertexFormatAnimated::Position]    = boost::make_tuple(3,        GL_FLOAT,              GL_FALSE,  0);
            //m[VertexFormatAnimated::UV]        = boost::make_tuple(1,        GL_FLOAT,              GL_FALSE,  offsetof(MeshVertexDataAnimated,uv));
            m[VertexFormatAnimated::UV]          = boost::make_tuple(2,        GL_FLOAT,              GL_FALSE,  offsetof(MeshVertexDataAnimated,uv));
            m[VertexFormatAnimated::Normal]      = boost::make_tuple(GL_BGRA,  GL_INT_2_10_10_10_REV, GL_TRUE,   offsetof(MeshVertexDataAnimated,normal));
            m[VertexFormatAnimated::Binormal]    = boost::make_tuple(GL_BGRA,  GL_INT_2_10_10_10_REV, GL_TRUE,   offsetof(MeshVertexDataAnimated,binormal));
            m[VertexFormatAnimated::Tangent]     = boost::make_tuple(GL_BGRA,  GL_INT_2_10_10_10_REV, GL_TRUE,   offsetof(MeshVertexDataAnimated,tangent));
            m[VertexFormatAnimated::BoneIDs]     = boost::make_tuple(4,        GL_FLOAT,              GL_FALSE,  offsetof(MeshVertexDataAnimated,boneIDs));
            m[VertexFormatAnimated::BoneWeights] = boost::make_tuple(4,        GL_FLOAT,              GL_FALSE,  offsetof(MeshVertexDataAnimated,boneWeights));
            return m;
        }();

        class MeshLoader final {
            friend class ::Mesh;
            public:
                static bool IsNear(float v1, float v2, float threshold) { return fabs(v1 - v2) < threshold; }
                static bool IsSpecialFloat(float f) {
                    if (boostm::isnan(f)) return true;
                    if (boostm::isinf(f)) return true;
                    return false;
                }
                static bool IsSpecialFloat(glm::vec2& v) {
                    if (boostm::isnan(v.x) || boostm::isnan(v.y)) return true;
                    if (boostm::isinf(v.x) || boostm::isinf(v.y)) return true;
                    return false;
                }
                static bool IsSpecialFloat(glm::vec3& v) {
                    if (boostm::isnan(v.x) || boostm::isnan(v.y) || boostm::isnan(v.z)) return true;
                    if (boostm::isinf(v.x) || boostm::isinf(v.y) || boostm::isinf(v.z)) return true;
                    return false;
                }
                static bool GetSimilarVertexIndex(glm::vec3& in_pos, glm::vec2& in_uv, glm::vec3& in_norm, vector<glm::vec3>& pts,vector<glm::vec2>& uvs, vector<glm::vec3>& norms, ushort& result, float threshold) {
                    for (uint t = 0; t < pts.size(); ++t){
                        if (IsNear(in_pos.x,pts.at(t).x,threshold) && IsNear(in_pos.y,pts.at(t).y,threshold) &&
                            IsNear(in_pos.z,pts.at(t).z,threshold) && IsNear(in_uv.x,uvs.at(t).x,threshold) &&
                            IsNear(in_uv.y,uvs.at(t).y,threshold) && IsNear(in_norm.x,norms.at(t).x,threshold) &&
                            IsNear(in_norm.y,norms.at(t).y,threshold) && IsNear(in_norm.z,norms.at(t).z,threshold)
                        ){
                            result = t;
                            return true;
                        }
                    }
                    return false;
                }
                static void CalculateTBNAssimp(epriv::ImportedMeshData& data) {
                    if (data.normals.size() == 0) return;
                    uint dataSize(data.points.size());
                    for (uint i = 0; i < dataSize; i += 3) {
                        uint p0(i + 0); uint p1(i + 1); uint p2(i + 2);

                        glm::vec3 dataP0, dataP1, dataP2;
                        glm::vec2 uvP0, uvP1, uvP2;

                        uint uvSize(data.uvs.size());

                        if (dataSize > p0) dataP0 = data.points.at(p0);
                        else              dataP0 = glm::vec3(0.0f);
                        if (dataSize > p1) dataP1 = data.points.at(p1);
                        else              dataP1 = glm::vec3(0.0f);
                        if (dataSize > p2) dataP2 = data.points.at(p2);
                        else              dataP2 = glm::vec3(0.0f);

                        if (uvSize > p0)   uvP0 = data.uvs.at(p0);
                        else              uvP0 = glm::vec2(0.0f);
                        if (uvSize > p1)   uvP1 = data.uvs.at(p1);
                        else              uvP1 = glm::vec2(0.0f);
                        if (uvSize > p2)   uvP2 = data.uvs.at(p2);
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

                            if (data.normals.size() > p) normal = data.normals.at(p);
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
						glm::vec3& n = data.normals.at(i);
						glm::vec3& t = data.binormals.at(i);
						glm::vec3& b = data.tangents.at(i);

						t = glm::normalize(t - n * glm::dot(n, t)); // Gram-Schmidt orthogonalize
					}
                }
        };
    };
};



class epriv::MeshSkeleton::impl final{
    public:
        unordered_map<string,uint> m_BoneMapping; // maps a bone name to its index
        uint m_NumBones;
        vector<epriv::BoneInfo> m_BoneInfo;
        glm::mat4 m_GlobalInverseTransform;
        unordered_map<string,epriv::AnimationData*> m_AnimationData;
        vector<glm::vec4> m_BoneIDs;
        vector<glm::vec4> m_BoneWeights;

        void _fill(epriv::ImportedMeshData& data){
            for(auto bone:data.m_Bones){
                epriv::VertexBoneData& b = bone.second;
                m_BoneIDs    .push_back(glm::vec4(b.IDs[0],    b.IDs[1],    b.IDs[2],    b.IDs[3]));
                m_BoneWeights.push_back(glm::vec4(b.Weights[0],b.Weights[1],b.Weights[2],b.Weights[3]));
            }
        }
        void _clear(){
            for(auto animationData:m_AnimationData){ delete animationData.second; }
            m_NumBones = 0;
            m_AnimationData.clear();
            m_BoneMapping.clear();
            vector_clear(m_BoneInfo);
            vector_clear(m_BoneIDs);
            vector_clear(m_BoneWeights);
        }
};
class Mesh::impl final{
    public:
        static DefaultMeshBindFunctor DEFAULT_BIND_FUNCTOR;
        static DefaultMeshUnbindFunctor DEFAULT_UNBIND_FUNCTOR;

        vector<GLuint> m_buffers;
        Collision* m_Collision;

        epriv::MeshSkeleton* m_Skeleton;

        const aiScene* m_aiScene;
        Assimp::Importer m_Importer;
        string m_File;

        glm::vec3 m_radiusBox;
        float m_radius;
        float m_threshold;
        CollisionType::Type m_Type;

        vector<epriv::MeshVertexData> m_Vertices;
        vector<ushort> m_Indices;
        vector<glm::mat4> m_InstanceMatrices;
        uint m_InstanceCount;
        GLuint m_VAO;

        void _initGlobal(float threshold){
            m_InstanceCount = 0;
            m_VAO = 0;
            m_File = "";
            m_Collision = nullptr;
            m_Skeleton = nullptr;
            m_threshold = threshold;
        }
        void _initGlobalTwo(Mesh* super,epriv::ImportedMeshData& data,float threshold){
			_finalizeData(data,threshold);
            super->setCustomBindFunctor(DEFAULT_BIND_FUNCTOR);
            super->setCustomUnbindFunctor(DEFAULT_UNBIND_FUNCTOR);
            super->load();
        }
        void _init(Mesh* super,string& name,unordered_map<string,float>& grid,uint width,uint length,float threshold){//grid
            epriv::ImportedMeshData d;
            _initGlobal(threshold);  
            for(uint i = 0; i < width-1; ++i){
                for(uint j = 0; j < length-1; ++j){
                    string key1(to_string(i) + "," + to_string(j));
                    string key2(to_string(i+1) + "," + to_string(j));
                    string key3(to_string(i) + "," + to_string(j+1));
                    string key4(to_string(i+1) + "," + to_string(j+1));

                    epriv::Vertex v1,v2,v3,v4;
                    v1.position = glm::vec3(i-width/2.0f,   grid.at(key1), j-length/2.0f);
                    v2.position = glm::vec3((i+1)-width/2.0f, grid.at(key2), j-length/2.0f);
                    v3.position = glm::vec3(i-width/2.0f,   grid.at(key3), (j+1)-length/2.0f);
                    v4.position = glm::vec3((i+1)-width/2.0f, grid.at(key4), (j+1)-length/2.0f);

                    glm::vec3 a(v4.position - v1.position);
                    glm::vec3 b(v2.position - v3.position);
                    glm::vec3 normal(glm::normalize(glm::cross(a,b)));

                    v1.normal = normal;
                    v2.normal = normal;
                    v3.normal = normal;
                    v4.normal = normal;

                    v1.uv = glm::vec2(float(i) / float(width),float(j) / float(length));
                    v2.uv = glm::vec2(float(i+1) / float(width),float(j) / float(length));
                    v3.uv = glm::vec2(float(i) / float(width),float(j+1) / float(length));
                    v4.uv = glm::vec2(float(i+1) / float(width),float(j+1) / float(length));

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
        void _init(Mesh* super,string& name,float x, float y,float width, float height,float threshold){//plane with offset uvs
            epriv::ImportedMeshData d;
            _initGlobal(threshold);

            d.points.emplace_back(glm::vec3(0));
            d.points.emplace_back(glm::vec3(width,height,0));
            d.points.emplace_back(glm::vec3(0,height,0));

            d.points.emplace_back(glm::vec3(width,0,0));
            d.points.emplace_back(glm::vec3(width,height,0));
            d.points.emplace_back(glm::vec3(0));

            float uv_topLeft_x = float(x / 256.0f);
            float uv_topLeft_y = float(y / 256.0f);

            float uv_bottomLeft_x = float(x / 256.0f);
            float uv_bottomLeft_y = float(y / 256.0f) + float(height / 256.0f);

            float uv_bottomRight_x = float(x / 256.0f) + float(width / 256.0f);
            float uv_bottomRight_y = float(y / 256.0f) + float(height / 256.0f);

            float uv_topRight_x = float(x / 256.0f) + float(width / 256.0f);
            float uv_topRight_y = float(y / 256.0f);

            d.uvs.emplace_back(glm::vec2(uv_bottomLeft_x,uv_bottomLeft_y));
            d.uvs.emplace_back(glm::vec2(uv_topRight_x,uv_topRight_y));
            d.uvs.emplace_back(glm::vec2(uv_topLeft_x,uv_topLeft_y));

            d.uvs.emplace_back(glm::vec2(uv_bottomRight_x,uv_bottomRight_y));
            d.uvs.emplace_back(glm::vec2(uv_topRight_x,uv_topRight_y));
            d.uvs.emplace_back(glm::vec2(uv_bottomLeft_x,uv_bottomLeft_y));

            d.normals.resize(6,glm::vec3(1));  d.binormals.resize(6,glm::vec3(1));  d.tangents.resize(6,glm::vec3(1));
            _initGlobalTwo(super,d,threshold);
        }
        void _init(Mesh* super,string& name,float width, float height,float threshold){//plane
            epriv::ImportedMeshData d;
            _initGlobal(threshold);
            d.points.emplace_back(glm::vec3(-width/2.0f,-height/2.0f,0));
            d.points.emplace_back(glm::vec3(width/2.0f,height/2.0f,0));
            d.points.emplace_back(glm::vec3(-width/2.0f,height/2.0f,0));

            d.points.emplace_back(glm::vec3(width/2.0f,-height/2.0f,0));
            d.points.emplace_back(glm::vec3(width/2.0f,height/2.0f,0));
            d.points.emplace_back(glm::vec3(-width/2.0f,-height/2.0f,0));

            float uv_topLeft_x = 0.0f;
            float uv_topLeft_y = 0.0f;

            float uv_bottomLeft_x = 0.0f;
            float uv_bottomLeft_y = 0.0f + float(height);

            float uv_bottomRight_x = 0.0f + float(width);
            float uv_bottomRight_y = 0.0f + float(height);

            float uv_topRight_x = 0.0f + float(width);
            float uv_topRight_y = 0.0f;

            d.uvs.emplace_back(glm::vec2(uv_bottomLeft_x,uv_bottomLeft_y));
            d.uvs.emplace_back(glm::vec2(uv_topRight_x,uv_topRight_y));
            d.uvs.emplace_back(glm::vec2(uv_topLeft_x,uv_topLeft_y));

            d.uvs.emplace_back(glm::vec2(uv_bottomRight_x,uv_bottomRight_y));
            d.uvs.emplace_back(glm::vec2(uv_topRight_x,uv_topRight_y));
            d.uvs.emplace_back(glm::vec2(uv_bottomLeft_x,uv_bottomLeft_y));

            d.normals.resize(6,glm::vec3(1));  d.binormals.resize(6,glm::vec3(1));  d.tangents.resize(6,glm::vec3(1));
            _initGlobalTwo(super,d,threshold);
        }
        void _init(Mesh* super,string& fileOrData,CollisionType::Type type,bool notMemory,float threshold,bool loadNow){//from file / data
            _initGlobal(threshold);
            m_Type = type;
            if(notMemory){
                m_File = fileOrData;
                if (m_File == "data/Models/defiant.obj")
                    _loadFromOBJMemory(super, type, threshold, epriv::LOAD_FACES | epriv::LOAD_UVS | epriv::LOAD_NORMALS | epriv::LOAD_TBN, fileOrData);
            }else{
                _loadFromOBJMemory(super,type,threshold, epriv::LOAD_FACES | epriv::LOAD_UVS | epriv::LOAD_NORMALS | epriv::LOAD_TBN,fileOrData);
            }
            super->setCustomBindFunctor(DEFAULT_BIND_FUNCTOR);
            super->setCustomUnbindFunctor(DEFAULT_UNBIND_FUNCTOR);
			if(loadNow)
                super->load();
        }
        void _clearData(){
            vector_clear(m_Vertices);
            if(m_Skeleton){
                SAFE_DELETE(m_Skeleton);
            }
        }
        void _loadInternal(Mesh* mesh,epriv::ImportedMeshData& data,string& file){
            m_aiScene = m_Importer.ReadFile(file,aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace); 
            if(!m_aiScene || m_aiScene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !m_aiScene->mRootNode){
                return;
            }
            //animation stuff
            aiMatrix4x4 m = m_aiScene->mRootNode->mTransformation; // node->mTransformation?
            m.Inverse();
            bool doOther = false;
            if(m_aiScene->mAnimations && m_aiScene->mNumAnimations > 0 && m_Skeleton == nullptr){
                m_Skeleton = new epriv::MeshSkeleton();
                m_Skeleton->m_i->m_GlobalInverseTransform = Math::assimpToGLMMat4(m);
                doOther = true;
            }
            _processNode(mesh,data,m_aiScene->mRootNode,m_aiScene);
            if(doOther){
                m_Skeleton->fill(data);
            }
        }
        void _processNode(Mesh* mesh,epriv::ImportedMeshData& data,aiNode* node, const aiScene* scene){
            for(uint i = 0; i < node->mNumMeshes; ++i){
                aiMesh& aimesh = *scene->mMeshes[node->mMeshes[i]];
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

                    v1.position = data.points.at(index0);
                    v2.position = data.points.at(index1);
                    v3.position = data.points.at(index2);
                    if(data.uvs.size() > 0){ 
                        v1.uv = data.uvs.at(index0);
                        v2.uv = data.uvs.at(index1);
                        v3.uv = data.uvs.at(index2);
                    }
                    if(data.normals.size() > 0){
                        v1.normal = data.normals.at(index0);
                        v2.normal = data.normals.at(index1);
                        v3.normal = data.normals.at(index2);
                    }
                    data.file_triangles.emplace_back(v1,v2,v3);

                }
                #pragma endregion
                //bones
                #pragma region Skeleton
                if(aimesh.mNumBones > 0){
                    auto& skeleton = *m_Skeleton->m_i;
                    for (uint i = 0; i < aimesh.mNumBones; ++i) { 
                        uint BoneIndex = 0; 
                        string BoneName(aimesh.mBones[i]->mName.data);
                        if(!skeleton.m_BoneMapping.count(BoneName)) {
                            BoneIndex = skeleton.m_NumBones;
                            skeleton.m_NumBones++; 
                            epriv::BoneInfo bi;
                            skeleton.m_BoneInfo.push_back(bi);
                        }
                        else{
                            BoneIndex = skeleton.m_BoneMapping.at(BoneName);
                        }
                        skeleton.m_BoneMapping.emplace(BoneName,BoneIndex);
                        aiMatrix4x4 n = aimesh.mBones[i]->mOffsetMatrix;
                        skeleton.m_BoneInfo.at(BoneIndex).BoneOffset = Math::assimpToGLMMat4(n);
                        for (uint j = 0; j < aimesh.mBones[i]->mNumWeights; ++j) {
                            uint VertexID = aimesh.mBones[i]->mWeights[j].mVertexId;
                            float Weight = aimesh.mBones[i]->mWeights[j].mWeight; 
                            epriv::VertexBoneData d;
                            d.AddBoneData(BoneIndex, Weight);
                            data.m_Bones.emplace(VertexID,d);
                        }
                    }
                    if(scene->mAnimations && scene->mNumAnimations > 0){
                        for(uint i = 0; i < scene->mNumAnimations; ++i){
                             aiAnimation* anim = scene->mAnimations[i];
                             string key(anim->mName.C_Str());
                             if(key == ""){
                                 key = "Animation " + to_string(skeleton.m_AnimationData.size());
                             }
                             if(!skeleton.m_AnimationData.count(key)){
                                auto* animData = new epriv::AnimationData(mesh,anim);
                                skeleton.m_AnimationData.emplace(key,animData);
                             }
                        }
                    }
                }
                #pragma endregion
                epriv::MeshLoader::CalculateTBNAssimp(data);
            }
            for(uint i = 0; i < node->mNumChildren; ++i){
                _processNode(mesh,data,node->mChildren[i], scene);
            }
        }

        void _finalizeData(epriv::ImportedMeshData& data,float threshold){
            m_threshold = threshold;

            if(data.uvs.size() == 0) data.uvs.resize(data.points.size());
            if(data.normals.size() == 0) data.normals.resize(data.points.size());
            if(data.binormals.size() == 0) data.binormals.resize(data.points.size());
            if(data.tangents.size() == 0) data.tangents.resize(data.points.size());

			if (threshold == 0.0f) {
				uint c = 0;
				for (auto pt : data.points) {
					if (m_Skeleton) {
						epriv::MeshVertexDataAnimated vert;
						vert.position = pt;
						//vert.uv = Math::pack2FloatsInto1Float(data.uvs.at(c));
						vert.uv = data.uvs.at(c);
						vert.normal = Math::pack3NormalsInto32Int(data.normals.at(c));
						if (c <= data.binormals.size() - 1)
							vert.binormal = Math::pack3NormalsInto32Int(data.binormals.at(c));
						if (c <= data.tangents.size() - 1)
							vert.tangent = Math::pack3NormalsInto32Int(data.tangents.at(c));
						m_Vertices.push_back(vert);
					}
					else {
						epriv::MeshVertexData vert;
						vert.position = pt;
						//vert.uv = Math::pack2FloatsInto1Float(data.uvs.at(c));
						vert.uv = data.uvs.at(c);
						vert.normal = Math::pack3NormalsInto32Int(data.normals.at(c));
						vert.binormal = Math::pack3NormalsInto32Int(data.binormals.at(c));
						vert.tangent = Math::pack3NormalsInto32Int(data.tangents.at(c));
						m_Vertices.push_back(vert);
					}
					++c;
				}
				m_Indices = data.indices;
				return;
			}
			vector<glm::vec3> temp_pos;
			vector<glm::vec2> temp_uvs;
			vector<glm::vec3> temp_normals;
			vector<glm::vec3> temp_binormals;
			vector<glm::vec3> temp_tangents;
			for (uint i = 0; i < data.points.size(); ++i) {
				ushort index;
				bool found = epriv::MeshLoader::GetSimilarVertexIndex(data.points.at(i), data.uvs.at(i), data.normals.at(i), temp_pos, temp_uvs, temp_normals, index, threshold);
				if (found) {
					m_Indices.push_back(index);

					//average out TBN. But it cancels out normal mapping on some flat surfaces
					//temp_binormals.at(index) += data.binormals.at(i);
					//temp_tangents.at(index) += data.tangents.at(i);
				}
				else {
					if (m_Skeleton) {
						epriv::MeshVertexDataAnimated vert;
						vert.position = data.points.at(i);
						m_Vertices.push_back(vert);
					}
					else {
						epriv::MeshVertexData vert;
						vert.position = data.points.at(i);
						m_Vertices.push_back(vert);
					}
					temp_pos.push_back(data.points.at(i));
					temp_uvs.push_back(data.uvs.at(i));
					temp_normals.push_back(data.normals.at(i));
					temp_binormals.push_back(data.binormals.at(i));
					temp_tangents.push_back(data.tangents.at(i));
					m_Indices.push_back((ushort)m_Vertices.size() - 1);
				}
			}
			for (uint i = 0; i < m_Vertices.size(); ++i) {
				/*
				if(m_Skeleton){
					auto& vert = (epriv::MeshVertexDataAnimated)m_Vertices.at(i);
					//vert.uv = Math::pack2FloatsInto1Float(temp_uvs.at(i));
					vert.uv = temp_uvs.at(i);
					vert.normal = Math::pack3NormalsInto32Int(temp_normals.at(i));
					vert.binormal = Math::pack3NormalsInto32Int(temp_binormals.at(i));
					vert.tangent = Math::pack3NormalsInto32Int(temp_tangents.at(i));
				}
				else{
					auto& vert = m_Vertices.at(i);
					//vert.uv = Math::pack2FloatsInto1Float(temp_uvs.at(i));
					vert.uv = temp_uvs.at(i);
					vert.normal = Math::pack3NormalsInto32Int(temp_normals.at(i));
					vert.binormal = Math::pack3NormalsInto32Int(temp_binormals.at(i));
					vert.tangent = Math::pack3NormalsInto32Int(temp_tangents.at(i));
				}
				*/
				auto& vert = m_Vertices.at(i);
				//vert.uv = Math::pack2FloatsInto1Float(temp_uvs.at(i));
				vert.uv = temp_uvs.at(i);
				vert.normal = Math::pack3NormalsInto32Int(temp_normals.at(i));
				vert.binormal = Math::pack3NormalsInto32Int(temp_binormals.at(i));
				vert.tangent = Math::pack3NormalsInto32Int(temp_tangents.at(i));
			}
        }
        void _loadFromFile(Mesh* super,string file,CollisionType::Type type,float threshold){
            string extention; for(uint i = m_File.length() - 4; i < m_File.length(); ++i)extention += tolower(m_File.at(i));
            epriv::ImportedMeshData d;
            _loadInternal(super,d,m_File);
            m_threshold = threshold; //this is needed
			_finalizeData(d,m_threshold);

            if(type == CollisionType::None){
                m_Collision = new Collision(new btEmptyShape());
            }
            else{
                string colFile = m_File.substr(0,m_File.size()-4);
                colFile += "Col.obj";
                if(boost::filesystem::exists(colFile)){
                    d.clear();
                    _loadInternal(super,d,colFile);
                }
                m_Collision = new Collision(d,type);
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
                    pos = data.file_points.at(_pi[i]-1);
                    data.points.push_back(pos);
                }
                if(_flags && epriv::LOAD_UVS && data.file_uvs.size() > 0){
                    uv  = data.file_uvs.at(_ui[i]-1);
                    data.uvs.push_back(uv);
                }
                if(_flags && epriv::LOAD_NORMALS && data.file_normals.size() > 0){ 
                    norm = data.file_normals.at(_ni[i]-1);
                    data.normals.push_back(norm);
                }
                ++count;
                if(count == 1){
                    triangle.v1.position = pos;
                    triangle.v1.uv = uv;
                    triangle.v1.normal = norm;
                }
                else if(count == 2){
                    triangle.v2.position = pos;
                    triangle.v2.uv = uv;
                    triangle.v2.normal = norm;
                }
                else if(count >= 3){
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
            }
            //vertex positions
            else if(l[0] == 'v' && l[1] == ' '){ 
                if(_f && epriv::LOAD_POINTS){
                    glm::vec3 p;
                    sscanf(l.substr(2,l.size()).c_str(),"%f %f %f",&p.x,&p.y,&p.z);
                    _d.file_points.push_back(p);
                }
            }
            //vertex uvs
            else if(l[0] == 'v' && l[1] == 't'){
                if(_f && epriv::LOAD_UVS){
                    glm::vec2 uv;
                    sscanf(l.substr(2,l.size()).c_str(),"%f %f",&uv.x,&uv.y);
                    uv.y = 1.0f - uv.y;
                    _d.file_uvs.push_back(uv);
                }
            }
            //vertex normals
            else if(l[0] == 'v' && l[1] == 'n'){
                if(_f && epriv::LOAD_NORMALS){
                    glm::vec3 n;
                    sscanf(l.substr(2,l.size()).c_str(),"%f %f %f",&n.x,&n.y,&n.z);
                    _d.file_normals.push_back(n);
                }
            }
            //faces
            else if(l[0] == 'f' && l[1] == ' '){
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
                    }
                    else if(matches == 4 || matches == 8 || matches == 12){//quad
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
        void _loadFromOBJMemory(Mesh* super,CollisionType::Type type,float threshold,unsigned char _flags,string input){
            epriv::ImportedMeshData d;

            vector<uint> positionIndices;
            vector<uint> uvIndices;
            vector<uint> normalIndices;

            istringstream stream;
            stream.str(input);

            //first read in all data
            for(string line; getline(stream, line, '\n');){
                _loadObjDataFromLine(line,d,positionIndices,uvIndices,normalIndices,_flags);
            }
            if(_flags && epriv::LOAD_FACES){
                _loadDataIntoTriangles(d,positionIndices,uvIndices,normalIndices,_flags);
            }
            if(_flags && epriv::LOAD_TBN && d.normals.size() > 0){
                epriv::MeshLoader::CalculateTBNAssimp(d);
            }
			_finalizeData(d,threshold);
            if(type == CollisionType::None){
                m_Collision = new Collision(new btEmptyShape);
            }
            else{
                m_Collision = new Collision(d,type);
            }
        }
        void _calculateMeshRadius(Mesh* super){
            float maxX = 0; float maxY = 0; float maxZ = 0;
            for(auto vertex:m_Vertices){
                float x = abs(vertex.position.x); float y = abs(vertex.position.y); float z = abs(vertex.position.z);
                if(x > maxX) maxX = x; if(y > maxY) maxY = y; if(z > maxZ) maxZ = z;
            }
            m_radiusBox = glm::vec3(maxX,maxY,maxZ);
            m_radius = Math::Max(m_radiusBox);
        }
        void _modifyPoints(vector<glm::vec3>& modifiedPts){
            glBindBuffer(GL_ARRAY_BUFFER, m_buffers.at(0));
            if(m_Skeleton){
                auto& skeleton = *m_Skeleton->m_i;
                vector<epriv::MeshVertexDataAnimated> temp; //this is needed to store the bone info into the buffer.
                for(uint i = 0; i < skeleton.m_BoneIDs.size(); ++i){
                    auto vert = (epriv::MeshVertexDataAnimated)m_Vertices.at(i);
                    vert.boneIDs = skeleton.m_BoneIDs.at(i);
                    vert.boneWeights = skeleton.m_BoneWeights.at(i);
                    temp.push_back(vert);
                }
                for(uint i = 0; i < modifiedPts.size(); ++i){ temp.at(i).position = modifiedPts.at(i); }
                glBufferSubData(GL_ARRAY_BUFFER,0,m_Vertices.size() * sizeof(epriv::MeshVertexDataAnimated),&temp[0]);
                vector_clear(temp);
            }
            else{
                for(uint i = 0; i < modifiedPts.size(); ++i){
                    m_Vertices.at(i).position = modifiedPts.at(i);
                }
                glBufferSubData(GL_ARRAY_BUFFER,0,m_Vertices.size() * sizeof(epriv::MeshVertexData),&m_Vertices[0]);
            }
        }
        void _modifyUVs(vector<glm::vec2>& modifiedUVs){
            glBindBuffer(GL_ARRAY_BUFFER, m_buffers.at(0));
            if(m_Skeleton){
                auto& skeleton = *m_Skeleton->m_i;
                vector<epriv::MeshVertexDataAnimated> temp; //this is needed to store the bone info into the buffer.
                for(uint i = 0; i < skeleton.m_BoneIDs.size(); ++i){
                    auto vert = (epriv::MeshVertexDataAnimated)m_Vertices.at(i);
                    vert.boneIDs = skeleton.m_BoneIDs.at(i);
                    vert.boneWeights = skeleton.m_BoneWeights.at(i);
                    temp.push_back(vert);
                }
                for(uint i = 0; i < modifiedUVs.size(); ++i){ temp.at(i).uv = modifiedUVs.at(i); }
                glBufferSubData(GL_ARRAY_BUFFER,0,m_Vertices.size() * sizeof(epriv::MeshVertexDataAnimated),&temp[0]);
                vector_clear(temp);
            }
            else{
                for(uint i = 0; i < modifiedUVs.size(); ++i){ m_Vertices.at(i).uv = modifiedUVs.at(i); }
                glBufferSubData(GL_ARRAY_BUFFER,0,m_Vertices.size() * sizeof(epriv::MeshVertexData),&m_Vertices[0]);
            }
        }
        void _modifyPointsAndUVs(vector<glm::vec3>& modifiedPts,vector<glm::vec2>& modifiedUVs){
            glBindBuffer(GL_ARRAY_BUFFER, m_buffers.at(0));
            if(m_Skeleton){
                auto& skeleton = *m_Skeleton->m_i;
                vector<epriv::MeshVertexDataAnimated> temp; //this is needed to store the bone info into the buffer.
                for(uint i = 0; i < skeleton.m_BoneIDs.size(); ++i){
                    auto vert = (epriv::MeshVertexDataAnimated)m_Vertices.at(i);
                    vert.boneIDs = skeleton.m_BoneIDs.at(i);
                    vert.boneWeights = skeleton.m_BoneWeights.at(i);
                    temp.push_back(vert);
                }
                for(uint i = 0; i < modifiedPts.size(); ++i){ temp.at(i).position = modifiedPts.at(i); }
                for(uint i = 0; i < modifiedUVs.size(); ++i){ temp.at(i).uv = modifiedUVs.at(i); }
                glBufferSubData(GL_ARRAY_BUFFER,0,m_Vertices.size() * sizeof(epriv::MeshVertexDataAnimated),&temp[0]);
                vector_clear(temp);
            }
            else{
                for(uint i = 0; i < modifiedPts.size(); ++i){ m_Vertices.at(i).position = modifiedPts.at(i); }
                for(uint i = 0; i < modifiedUVs.size(); ++i){ m_Vertices.at(i).uv = modifiedUVs.at(i); }
                glBufferSubData(GL_ARRAY_BUFFER,0,m_Vertices.size() * sizeof(epriv::MeshVertexData),&m_Vertices[0]);
            }
        }
		void _bindMeshDataToGPU(){
			glBindBuffer(GL_ARRAY_BUFFER, m_buffers.at(0));
			uint _structSize = m_Skeleton ? sizeof(epriv::MeshVertexDataAnimated) : sizeof(epriv::MeshVertexData);
			uint _enumTotal = m_Skeleton ? epriv::VertexFormatAnimated::_TOTAL : epriv::VertexFormat::_TOTAL;
			for (uint i = 0; i < _enumTotal; ++i) {
				auto& d = epriv::VERTEX_ANIMATED_FORMAT_DATA.at(i);
				glEnableVertexAttribArray(i);
				glVertexAttribPointer(i, d.get<0>(), d.get<1>(), d.get<2>(), _structSize, (void*)d.get<3>());
			}
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_buffers.at(1));
			//instances
			if (m_buffers.size() >= 3) {
				glBindBuffer(GL_ARRAY_BUFFER, m_buffers.at(2));
				uint attributeIndex = epriv::VertexFormatAnimated::_TOTAL;
				for (uint j = 0; j < 4; ++j) {
					glEnableVertexAttribArray(attributeIndex + j);
					glVertexAttribPointer(attributeIndex + j, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(j * sizeof(glm::vec4)));
					if (epriv::RenderManager::OPENGL_VERSION >= 33) { glVertexAttribDivisor(attributeIndex + j, 1); }
				}
			}
		}
		void _buildVAO(){
			Renderer::deleteVAO(m_VAO);
			if (epriv::RenderManager::OPENGL_VERSION >= 30) {
				Renderer::genAndBindVAO(m_VAO);
				_bindMeshDataToGPU();
				Renderer::bindVAO(0);
			}
		}
        void _unload_CPU(Mesh* super){
            if(m_File != ""){
                _clearData();
            }
            cout << "(Mesh) ";
        }
        void _unload_GPU(Mesh* super){
            for(uint i = 0; i < m_buffers.size(); ++i){
                glDeleteBuffers(1,&m_buffers.at(i));
            }
			Renderer::deleteVAO(m_VAO);
            vector_clear(m_buffers);
        }
        void _load_CPU(Mesh* super){
            if(m_File != ""){
                _loadFromFile(super,m_File,m_Type,m_threshold);
            }
            _calculateMeshRadius(super);
        }
        void _load_GPU(Mesh* super){
            if(m_buffers.size() > 0){
                _unload_GPU(super);
            }

            //vertex data
            m_buffers.push_back(0);
            glGenBuffers(1, &m_buffers.at(0));
            glBindBuffer(GL_ARRAY_BUFFER, m_buffers.at(0));
            if(m_Skeleton){
                auto& skeleton = *m_Skeleton->m_i;
                vector<epriv::MeshVertexDataAnimated> temp; //this is needed to store the bone info into the buffer.
                for(uint i = 0; i < skeleton.m_BoneIDs.size(); ++i){
                    auto vert = (epriv::MeshVertexDataAnimated)m_Vertices.at(i);
                    vert.boneIDs = skeleton.m_BoneIDs.at(i);
                    vert.boneWeights = skeleton.m_BoneWeights.at(i);
                    temp.push_back(vert);
                }
                glBufferData(GL_ARRAY_BUFFER, m_Vertices.size() * sizeof(epriv::MeshVertexDataAnimated),&temp[0], GL_DYNAMIC_DRAW );
                vector_clear(temp);
            }
            else{
                glBufferData(GL_ARRAY_BUFFER, m_Vertices.size() * sizeof(epriv::MeshVertexData),&m_Vertices[0], GL_DYNAMIC_DRAW );
            }
            //index data
            m_buffers.push_back(0);
            glGenBuffers(1, &m_buffers.at(1));
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_buffers.at(1));
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_Indices.size() * sizeof(ushort), &m_Indices[0] , GL_STATIC_DRAW);
            /*
            if(InternalMeshPublicInterface::SupportsInstancing()){
                //instancing data
                m_buffers.push_back(0);
                vector<glm::mat4> defaultMatrices;
                glm::mat4 _identity = glm::mat4(1.0f);
                defaultMatrices.resize(NUM_MAX_INSTANCES,_identity);
                glGenBuffers(1, &m_buffers.at(2));
                glBindBuffer(GL_ARRAY_BUFFER, m_buffers.at(2));
                glBufferData(GL_ARRAY_BUFFER, NUM_MAX_INSTANCES * sizeof(glm::mat4), &defaultMatrices[0], GL_DYNAMIC_DRAW);           
            }
            */
            //support vao's
			_buildVAO();
            cout << "(Mesh) ";
        }


        void _writeToObjCompressed(epriv::ImportedMeshData& data) {
            //header:
            //faces example f 372/688/534 435/860/656 368/686/532
            //number of chars used to represent a face, faces MUST be triangleized
            string f = m_File;
            string ext = boost::filesystem::extension(m_File);
            f = f.substr(0, f.size() - ext.size());
            f += ".objc";
            ofstream stream;
            stream.open(f, ios::out | ios::binary);


            //header
            uint32_t posSize = data.points.size() * 6;
            uint32_t uvSize = data.uvs.size() * 4;
            uint32_t normSize = data.normals.size() * 6;
            uint32_t indiceSize = data.indices.size() * 2;

            char* header = new char[16];
            header[0] = (posSize >> 24) & 0xFF;
            header[1] = (posSize >> 16) & 0xFF;
            header[2] = (posSize >> 8) & 0xFF;
            header[3] = posSize & 0xFF;

            header[4] = (uvSize >> 24) & 0xFF;
            header[5] = (uvSize >> 16) & 0xFF;
            header[6] = (uvSize >> 8) & 0xFF;
            header[7] = uvSize & 0xFF;

            header[8] = (normSize >> 24) & 0xFF;
            header[9] = (normSize >> 16) & 0xFF;
            header[10] = (normSize >> 8) & 0xFF;
            header[11] = normSize & 0xFF;

            header[12] = (indiceSize >> 24) & 0xFF;
            header[13] = (indiceSize >> 16) & 0xFF;
            header[14] = (indiceSize >> 8) & 0xFF;
            header[15] = indiceSize & 0xFF;

            stream.write(header, 16);
            delete[] header;

            char* positionBuffer = new char[posSize];
            uint count = 0;
            for (auto pos : data.points) {
                uint16_t outX, outY, outZ;
                float16(&outX, pos.x);
                float16(&outY, pos.y);
                float16(&outZ, pos.z);
                uchar x_part1, x_part2, y_part1, y_part2, z_part1, z_part2;
                x_part1 = outX & 0xF; // bits 0..3
                x_part2 = (outX >> 4) & 0xF; // bits 4..7
                y_part1 = outY & 0xF; // bits 0..3
                y_part2 = (outY >> 4) & 0xF; // bits 4..7
                z_part1 = outZ & 0xF; // bits 0..3
                z_part2 = (outZ >> 4) & 0xF; // bits 4..7    
                positionBuffer[(count * 6) + 0] = x_part1;
                positionBuffer[(count * 6) + 1] = x_part2;
                positionBuffer[(count * 6) + 2] = y_part1;
                positionBuffer[(count * 6) + 3] = y_part2;
                positionBuffer[(count * 6) + 4] = z_part1;
                positionBuffer[(count * 6) + 5] = z_part2;
                ++count;
            }

            stream.write(positionBuffer, posSize);
            delete[] positionBuffer;

            count = 0;
            char* uvBuffer = new char[uvSize];
            for (auto uv : data.uvs) {
                uint16_t outX, outY;
                float16(&outX, uv.x);
                float16(&outY, uv.y);
                uchar x_part1, x_part2, y_part1, y_part2;
                x_part1 = outX & 0xF; // bits 0..3
                x_part2 = (outX >> 4) & 0xF; // bits 4..7
                y_part1 = outY & 0xF; // bits 0..3
                y_part2 = (outY >> 4) & 0xF; // bits 4..7 
                uvBuffer[(count * 4) + 0] = x_part1;
                uvBuffer[(count * 4) + 1] = x_part2;
                uvBuffer[(count * 4) + 2] = y_part1;
                uvBuffer[(count * 4) + 3] = y_part2;
                ++count;
            }
            stream.write(uvBuffer, uvSize);
            delete[] uvBuffer;


            count = 0;
            char* normBuffer = new char[normSize];
            for (auto norm : data.normals) {
                uint16_t outX, outY,outZ;
                float16(&outX, norm.x);
                float16(&outY, norm.y);
                float16(&outZ, norm.z);
                uchar x_part1, x_part2, y_part1, y_part2, z_part1, z_part2;
                x_part1 = outX & 0xF; // bits 0..3
                x_part2 = (outX >> 4) & 0xF; // bits 4..7
                y_part1 = outY & 0xF; // bits 0..3
                y_part2 = (outY >> 4) & 0xF; // bits 4..7 
                z_part1 = outZ & 0xF; // bits 0..3
                z_part2 = (outZ >> 4) & 0xF; // bits 4..7 
                normBuffer[(count * 6) + 0] = x_part1;
                normBuffer[(count * 6) + 1] = x_part2;
                normBuffer[(count * 6) + 2] = y_part1;
                normBuffer[(count * 6) + 3] = y_part2;
                normBuffer[(count * 6) + 4] = z_part1;
                normBuffer[(count * 6) + 5] = z_part2;
                ++count;
            }
            stream.write(normBuffer, normSize);
            delete[] normBuffer;

            //pack indices normally, unpack normally too
            count = 0;
            char* indiceBuffer = new char[indiceSize];
            for (auto ind : data.indices) {
                uchar x_part1, x_part2;
                x_part1 = ind & 0xF; // bits 0..3
                x_part2 = (ind >> 4) & 0xF; // bits 4..7
                indiceBuffer[(count * 2) + 0] = x_part1;
                indiceBuffer[(count * 2) + 1] = x_part2;
                ++count;
            }
            stream.write(indiceBuffer, indiceSize);
            delete[] indiceBuffer;

            stream.close();
        }
        
};
struct DefaultMeshBindFunctor{void operator()(BindableResource* r) const {
    auto& m = *((Mesh*)r)->m_i;
    if(m.m_VAO){
        Renderer::bindVAO(m.m_VAO);
    }else{
		m._bindMeshDataToGPU();
    }
}};
struct DefaultMeshUnbindFunctor{void operator()(BindableResource* r) const {
    auto& m = *((Mesh*)r)->m_i;
    if(m.m_VAO){
        Renderer::bindVAO(0);
    }else{
        uint _enumTotal  = m.m_Skeleton ? epriv::VertexFormatAnimated::_TOTAL : epriv::VertexFormat::_TOTAL;
        for(uint i = 0; i < _enumTotal; ++i){ glDisableVertexAttribArray(i); }
        //instances
        if(m.m_buffers.size() >= 3){
            uint attributeIndex = epriv::VertexFormatAnimated::_TOTAL;
            for(uint j = 0; j < 4; ++j){ glDisableVertexAttribArray(attributeIndex + j); }
        }
    }
}};
DefaultMeshBindFunctor Mesh::impl::DEFAULT_BIND_FUNCTOR;
DefaultMeshUnbindFunctor Mesh::impl::DEFAULT_UNBIND_FUNCTOR;

class epriv::AnimationData::impl{
    public:
        Mesh* m_Mesh;
        double m_TicksPerSecond;
        double m_DurationInTicks;
        unordered_map<string,aiNodeAnim*> m_KeyframeData;

        void _Init(Mesh* mesh,aiAnimation* anim){
            m_Mesh = mesh;
            m_TicksPerSecond = anim->mTicksPerSecond;
            m_DurationInTicks = anim->mDuration;
            for(uint i = 0; i < anim->mNumChannels; ++i){
                string key(anim->mChannels[i]->mNodeName.data);
                m_KeyframeData.emplace(key,anim->mChannels[i]);
            }
        }
        void _ReadNodeHeirarchy(const string& animationName,float time, const aiNode* n,glm::mat4& ParentTransform,vector<glm::mat4>& Transforms){
            string BoneName(n->mName.data);
            glm::mat4 NodeTransform(Math::assimpToGLMMat4(const_cast<aiMatrix4x4&>(n->mTransformation)));
            if(m_KeyframeData.count(BoneName)){
                const aiNodeAnim* keyframes(m_KeyframeData.at(BoneName));
                if(keyframes){
                    glm::vec3 s; _CalcInterpolatedScaling(s, time, keyframes);
                    aiQuaternion q; _CalcInterpolatedRotation(q, time, keyframes);
                    glm::mat4 rotation(glm::mat4(Math::assimpToGLMMat3(q.GetMatrix())));
                    glm::vec3 t; _CalcInterpolatedPosition(t, time, keyframes);
                    NodeTransform = glm::mat4(1.0f);
                    NodeTransform = glm::translate(NodeTransform,t);
                    NodeTransform *= rotation;
                    NodeTransform = glm::scale(NodeTransform,s);
                }
            }
            glm::mat4 Transform(ParentTransform * NodeTransform);
            auto& skeleton = *m_Mesh->m_i->m_Skeleton;
            if(skeleton.m_i->m_BoneMapping.count(BoneName)){
                uint BoneIndex(skeleton.m_i->m_BoneMapping.at(BoneName));
                glm::mat4& Final(skeleton.m_i->m_BoneInfo.at(BoneIndex).FinalTransform);
                Final = skeleton.m_i->m_GlobalInverseTransform * Transform * skeleton.m_i->m_BoneInfo.at(BoneIndex).BoneOffset;
                //this line allows for animation combinations. only works when additional animations start off in their resting places...
                Final = Transforms.at(BoneIndex) * Final;
            }
            for(uint i = 0; i < n->mNumChildren; ++i){
                _ReadNodeHeirarchy(animationName,time,n->mChildren[i],Transform,Transforms);
            }
        }
        void _BoneTransform(const string& animationName,float TimeInSeconds, vector<glm::mat4>& Transforms){
            float TicksPerSecond = float(m_TicksPerSecond != 0 ? m_TicksPerSecond : 25.0f);
            float TimeInTicks(TimeInSeconds * TicksPerSecond);
            float AnimationTime(float(fmod(TimeInTicks, m_DurationInTicks)));
            glm::mat4 Identity(glm::mat4(1.0f));
            _ReadNodeHeirarchy(animationName,AnimationTime, m_Mesh->m_i->m_aiScene->mRootNode, Identity,Transforms);
            auto& skeleton = *m_Mesh->m_i->m_Skeleton;
            for(uint i = 0; i < skeleton.m_i->m_NumBones; ++i){
                Transforms.at(i) = skeleton.m_i->m_BoneInfo.at(i).FinalTransform;
            }
        }
        void _CalcInterpolatedPosition(glm::vec3& Out, float AnimationTime, const aiNodeAnim* node){
            if (node->mNumPositionKeys == 1) {
                Out = Math::assimpToGLMVec3(node->mPositionKeys[0].mValue); return;
            }
            uint PositionIndex(_FindPosition(AnimationTime,node));
            uint NextIndex(PositionIndex + 1);
            float DeltaTime((float)(node->mPositionKeys[NextIndex].mTime - node->mPositionKeys[PositionIndex].mTime));
            float Factor((AnimationTime - (float)node->mPositionKeys[PositionIndex].mTime) / DeltaTime);
            glm::vec3 Start(Math::assimpToGLMVec3(node->mPositionKeys[PositionIndex].mValue));
            glm::vec3 End(Math::assimpToGLMVec3(node->mPositionKeys[NextIndex].mValue));
            Out = Start + Factor * (End - Start);
        }
        void _CalcInterpolatedRotation(aiQuaternion& Out, float AnimationTime, const aiNodeAnim* node){
            if (node->mNumRotationKeys == 1) {
                Out = node->mRotationKeys[0].mValue; return;
            }
            uint RotationIndex(_FindRotation(AnimationTime, node));
            uint NextIndex(RotationIndex + 1);
            float DeltaTime((float)(node->mRotationKeys[NextIndex].mTime - node->mRotationKeys[RotationIndex].mTime));
            float Factor((AnimationTime - (float)node->mRotationKeys[RotationIndex].mTime) / DeltaTime);
            const aiQuaternion& StartRotationQ = node->mRotationKeys[RotationIndex].mValue;
            const aiQuaternion& EndRotationQ   = node->mRotationKeys[NextIndex].mValue;    
            aiQuaternion::Interpolate(Out, StartRotationQ, EndRotationQ, Factor);
            Out = Out.Normalize();
        }
        void _CalcInterpolatedScaling(glm::vec3& Out, float AnimationTime, const aiNodeAnim* node){
            if (node->mNumScalingKeys == 1) {
                Out = Math::assimpToGLMVec3(node->mScalingKeys[0].mValue); return;
            }
            uint ScalingIndex(_FindScaling(AnimationTime, node));
            uint NextIndex(ScalingIndex + 1);
            float DeltaTime((float)(node->mScalingKeys[NextIndex].mTime - node->mScalingKeys[ScalingIndex].mTime));
            float Factor((AnimationTime - (float)node->mScalingKeys[ScalingIndex].mTime) / DeltaTime);
            glm::vec3 Start(Math::assimpToGLMVec3(node->mScalingKeys[ScalingIndex].mValue));
            glm::vec3 End(Math::assimpToGLMVec3(node->mScalingKeys[NextIndex].mValue));
            Out = Start + Factor * (End - Start);
        }
        uint _FindPosition(float AnimationTime, const aiNodeAnim* node){
            for(uint i=0;i<node->mNumPositionKeys-1;++i){if(AnimationTime<(float)node->mPositionKeys[i+1].mTime){return i;}}return 0;
        }
        uint _FindRotation(float AnimationTime, const aiNodeAnim* node){
            for(uint i=0;i<node->mNumRotationKeys-1;++i){if(AnimationTime<(float)node->mRotationKeys[i+1].mTime){return i;}}return 0;
        }
        uint _FindScaling(float AnimationTime, const aiNodeAnim* node){
            for(uint i=0;i<node->mNumScalingKeys-1;++i){if(AnimationTime<(float)node->mScalingKeys[i+1].mTime){return i;}}return 0;
        }
        float _Duration(){
            float TicksPerSecond(float(m_TicksPerSecond != 0 ? m_TicksPerSecond : 25.0f));return float(float(m_DurationInTicks) / TicksPerSecond);
        }
};
epriv::AnimationData::AnimationData(Mesh* m, aiAnimation* a) :m_i(new impl) { m_i->_Init(m, a); }
epriv::AnimationData::~AnimationData() { }
float epriv::AnimationData::duration() { return m_i->_Duration(); }

epriv::MeshSkeleton::MeshSkeleton():m_i(new impl){ clear(); }
epriv::MeshSkeleton::MeshSkeleton(epriv::ImportedMeshData& data):m_i(new impl){ fill(data); }
void epriv::MeshSkeleton::fill(epriv::ImportedMeshData& data){ m_i->_fill(data); }
void epriv::MeshSkeleton::clear(){ m_i->_clear(); }
epriv::MeshSkeleton::~MeshSkeleton(){ clear(); }
uint epriv::MeshSkeleton::numBones(){ return m_i->m_NumBones; }

void InternalMeshPublicInterface::LoadCPU(Mesh* mesh){
    //if(!mesh->isLoaded()){
        mesh->m_i->_load_CPU(mesh);
    //}
}
void InternalMeshPublicInterface::LoadGPU(Mesh* mesh){
    //if(!mesh->isLoaded()){
        mesh->m_i->_load_GPU(mesh);
        mesh->EngineResource::load();
    //}
}
void InternalMeshPublicInterface::UnloadCPU(Mesh* mesh){
    //if(mesh->isLoaded()){
        mesh->m_i->_unload_CPU(mesh);
        mesh->EngineResource::unload();
    //}
}
void InternalMeshPublicInterface::UnloadGPU(Mesh* mesh){
    //if(mesh->isLoaded()){
        mesh->m_i->_unload_GPU(mesh);
    //}
}
void InternalMeshPublicInterface::UpdateInstance(Mesh* mesh,uint _id, glm::mat4 _modelMatrix){
    auto& i = *mesh->m_i;
    glBindBuffer(GL_ARRAY_BUFFER, i.m_buffers.at(2));
    glBufferSubData(GL_ARRAY_BUFFER, _id * sizeof(glm::mat4), sizeof(glm::mat4), &_modelMatrix);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}
void InternalMeshPublicInterface::UpdateInstances(Mesh* mesh,vector<glm::mat4>& _modelMatrices){
    auto& i = *mesh->m_i;
    i.m_InstanceCount = _modelMatrices.size();
    if(_modelMatrices.size() == 0) return;
    glBindBuffer(GL_ARRAY_BUFFER, i.m_buffers.at(2));
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(glm::mat4) * i.m_InstanceCount, &_modelMatrices[0]);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}
bool InternalMeshPublicInterface::SupportsInstancing(){
    if(epriv::RenderManager::OPENGL_VERSION >= 31 || 
    epriv::OpenGLExtensionEnum::supported(epriv::OpenGLExtensionEnum::EXT_draw_instanced) || 
    epriv::OpenGLExtensionEnum::supported(epriv::OpenGLExtensionEnum::ARB_draw_instanced)){
        return true;
    }
    return false;
}
Mesh::Mesh(string name,unordered_map<string,float>& grid,uint width,uint length,float threshold):BindableResource(name),m_i(new impl){
    m_i->_init(this,name,grid,width,length,threshold);
	registerEvent(EventType::WindowFullscreenChanged);
}
Mesh::Mesh(string name,float x, float y,float width, float height,float threshold):BindableResource(name),m_i(new impl){
    m_i->_init(this,name,x,y,width,height,threshold);
	registerEvent(EventType::WindowFullscreenChanged);
}
Mesh::Mesh(string name,float width, float height,float threshold):BindableResource(name),m_i(new impl){
    m_i->_init(this,name,width,height,threshold);
	registerEvent(EventType::WindowFullscreenChanged);
}
Mesh::Mesh(string fileOrData,CollisionType::Type type,bool notMemory,float threshold,bool loadNow):BindableResource(fileOrData),m_i(new impl){
    if(!notMemory) setName("CustomMesh");
    m_i->_init(this,fileOrData,type,notMemory,threshold,loadNow);
	registerEvent(EventType::WindowFullscreenChanged);
}
Mesh::~Mesh(){
	unregisterEvent(EventType::WindowFullscreenChanged);
    unload();
    m_i->_clearData();
}
Collision* Mesh::getCollision() const { return m_i->m_Collision; }

unordered_map<string, epriv::AnimationData*>& Mesh::animationData(){ return m_i->m_Skeleton->m_i->m_AnimationData; }

const glm::vec3& Mesh::getRadiusBox() const { return m_i->m_radiusBox; }
const float Mesh::getRadius() const { return m_i->m_radius; }
void Mesh::render(bool instancing,GLuint mode){
    auto& i = *m_i;
    const uint& indicesSize = i.m_Indices.size();
    if(instancing && InternalMeshPublicInterface::SupportsInstancing()){
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
    i.m_i->m_AnimationData.at(animationName)->m_i->_BoneTransform(animationName,time, transforms);
}
void Mesh::load(){
    if(!isLoaded()){
        auto& i = *m_i;
        i._load_CPU(this);
        i._load_GPU(this);
        EngineResource::load();
    }
}
void Mesh::unload(){
    if(isLoaded() && useCount() == 0){
        auto& i = *m_i;
        i._unload_GPU(this);
        i._unload_CPU(this);
        EngineResource::unload();
    }
}
void Mesh::modifyPoints(vector<glm::vec3>& modifiedPts){ m_i->_modifyPoints(modifiedPts); }
void Mesh::modifyUVs(vector<glm::vec2>& modifiedUVs){ m_i->_modifyUVs(modifiedUVs); }
void Mesh::modifyPointsAndUVs(vector<glm::vec3>& modifiedPts, vector<glm::vec2>& modifiedUVs){ m_i->_modifyPointsAndUVs(modifiedPts,modifiedUVs); }
void Mesh::onEvent(const Event& e) {
	if (e.type == EventType::WindowFullscreenChanged) {
		auto& i = *m_i;
		i._buildVAO();
	}
}