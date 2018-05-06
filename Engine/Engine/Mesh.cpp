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

using namespace std;
using namespace Engine;

Mesh* Mesh::Plane = nullptr;
Mesh* Mesh::Cube = nullptr;

namespace Engine{
    namespace epriv{
        enum LoadWhat{
            LOAD_POINTS  = 0x01,
            LOAD_UVS     = 0x02,
            LOAD_NORMALS = 0x04,
            LOAD_FACES   = 0x08,
            LOAD_TBN     = 0x16
                       //= 0x32
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
            m[VertexFormatAnimated::Position]    = boost::make_tuple(3,  GL_FLOAT,         GL_FALSE,  0);
            //m[VertexFormatAnimated::UV]        = boost::make_tuple(1,  GL_FLOAT,         GL_FALSE,  offsetof(MeshVertexDataAnimated,uv));
            m[VertexFormatAnimated::UV]          = boost::make_tuple(2,  GL_FLOAT,         GL_FALSE,  offsetof(MeshVertexDataAnimated,uv));
            m[VertexFormatAnimated::Normal]      = boost::make_tuple(GL_BGRA,  GL_INT_2_10_10_10_REV, GL_TRUE,    offsetof(MeshVertexDataAnimated,normal));
            m[VertexFormatAnimated::Binormal]    = boost::make_tuple(GL_BGRA,  GL_INT_2_10_10_10_REV, GL_TRUE,    offsetof(MeshVertexDataAnimated,binormal));
            m[VertexFormatAnimated::Tangent]     = boost::make_tuple(GL_BGRA,  GL_INT_2_10_10_10_REV, GL_TRUE,    offsetof(MeshVertexDataAnimated,tangent));
            m[VertexFormatAnimated::BoneIDs]     = boost::make_tuple(4,  GL_FLOAT,         GL_FALSE,  offsetof(MeshVertexDataAnimated,boneIDs));
            m[VertexFormatAnimated::BoneWeights] = boost::make_tuple(4,  GL_FLOAT,         GL_FALSE,  offsetof(MeshVertexDataAnimated,boneWeights));
            return m;
        }();
    };
};



class MeshSkeleton::impl final{
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
                Engine::epriv::VertexBoneData& b = bone.second;
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

        MeshSkeleton* m_Skeleton;

        const aiScene* m_aiScene;
        Assimp::Importer m_Importer;
        string m_File;

        glm::vec3 m_radiusBox;
        float m_radius;
        float m_threshold;
        bool m_SaveMeshData;
        CollisionType::Type m_Type;

        vector<epriv::MeshVertexData> m_Vertices;
        vector<ushort> m_Indices;

        void _initGlobal(float threshold){
            m_File = "";
            m_SaveMeshData = true;
            m_Collision = nullptr;
            m_Skeleton = nullptr;
            m_threshold = threshold;
        }
        void _initGlobalTwo(Mesh* super,epriv::ImportedMeshData& data,float threshold){
            _loadData(super,data,threshold);
            super->setCustomBindFunctor(DEFAULT_BIND_FUNCTOR);
            super->setCustomUnbindFunctor(DEFAULT_UNBIND_FUNCTOR);
            super->load();
        }
        
        void _init(Mesh* super,string& name,btHeightfieldTerrainShape* heightfield,float threshold){//heightmap
            epriv::ImportedMeshData d;
            _initGlobal(threshold);
            uint width = heightfield->getHeightStickWidth();
            uint length = heightfield->getHeightStickLength();
            for(uint i = 0; i < width-1; ++i){
                for(uint j = 0; j < length-1; ++j){
                    btVector3 vert1,vert2,vert3,vert4;
                    heightfield->getVertex1(i,  j,  vert1);
                    heightfield->getVertex1(i+1,j,  vert2);
                    heightfield->getVertex1(i,  j+1,vert3);
                    heightfield->getVertex1(i+1,j+1,vert4);

                    epriv::Vertex v1,v2,v3,v4;
                    v1.position = glm::vec3(vert1.x(),vert1.y(),vert1.z());
                    v2.position = glm::vec3(vert2.x(),vert2.y(),vert2.z());
                    v3.position = glm::vec3(vert3.x(),vert3.y(),vert3.z());
                    v4.position = glm::vec3(vert4.x(),vert4.y(),vert4.z());

                    glm::vec3 a = v4.position - v1.position;
                    glm::vec3 b = v2.position - v3.position;
                    glm::vec3 normal = glm::normalize(glm::cross(a,b));

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

                    _calculateTBN(d);
                }
            }
            _initGlobalTwo(super,d,threshold);
        }
        void _init(Mesh* super,string& name,unordered_map<string,float>& grid,uint width,uint length,float threshold){//grid
            epriv::ImportedMeshData d;
            _initGlobal(threshold);  
            for(uint i = 0; i < width-1; ++i){
                for(uint j = 0; j < length-1; ++j){
                    string key1 = to_string(i) + "," + to_string(j);
                    string key2 = to_string(i+1) + "," + to_string(j);
                    string key3 = to_string(i) + "," + to_string(j+1);
                    string key4 = to_string(i+1) + "," + to_string(j+1);

                    epriv::Vertex v1,v2,v3,v4;
                    v1.position = glm::vec3(i-width/2.0f,   grid[key1], j-length/2.0f);
                    v2.position = glm::vec3((i+1)-width/2.0f, grid[key2], j-length/2.0f);
                    v3.position = glm::vec3(i-width/2.0f,   grid[key3], (j+1)-length/2.0f);
                    v4.position = glm::vec3((i+1)-width/2.0f, grid[key4], (j+1)-length/2.0f);

                    glm::vec3 a = v4.position - v1.position;
                    glm::vec3 b = v2.position - v3.position;
                    glm::vec3 normal = glm::normalize(glm::cross(a,b));

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

                    _calculateTBN(d);
                }
            }
            _initGlobalTwo(super,d,threshold);
        }
        void _init(Mesh* super,string& name,float x, float y,float width, float height,float threshold){//plane with offset uvs
            epriv::ImportedMeshData d;
            _initGlobal(threshold);

            d.points.push_back(glm::vec3(0));
            d.points.push_back(glm::vec3(width,height,0));
            d.points.push_back(glm::vec3(0,height,0));

            d.points.push_back(glm::vec3(width,0,0));
            d.points.push_back(glm::vec3(width,height,0));
            d.points.push_back(glm::vec3(0));

            float uv_topLeft_x = float(x / 256.0f);
            float uv_topLeft_y = float(y / 256.0f);

            float uv_bottomLeft_x = float(x / 256.0f);
            float uv_bottomLeft_y = float(y / 256.0f) + float(height / 256.0f);

            float uv_bottomRight_x = float(x / 256.0f) + float(width / 256.0f);
            float uv_bottomRight_y = float(y / 256.0f) + float(height / 256.0f);

            float uv_topRight_x = float(x / 256.0f) + float(width / 256.0f);
            float uv_topRight_y = float(y / 256.0f);

            d.uvs.push_back(glm::vec2(uv_bottomLeft_x,uv_bottomLeft_y));
            d.uvs.push_back(glm::vec2(uv_topRight_x,uv_topRight_y));
            d.uvs.push_back(glm::vec2(uv_topLeft_x,uv_topLeft_y));

            d.uvs.push_back(glm::vec2(uv_bottomRight_x,uv_bottomRight_y));
            d.uvs.push_back(glm::vec2(uv_topRight_x,uv_topRight_y));
            d.uvs.push_back(glm::vec2(uv_bottomLeft_x,uv_bottomLeft_y));

            d.normals.resize(6,glm::vec3(1));  d.binormals.resize(6,glm::vec3(1));  d.tangents.resize(6,glm::vec3(1));
            _initGlobalTwo(super,d,threshold);
        }
        void _init(Mesh* super,string& name,float width, float height,float threshold){//plane
            epriv::ImportedMeshData d;
            _initGlobal(threshold);
            d.points.push_back(glm::vec3(-width/2.0f,-height/2.0f,0));
            d.points.push_back(glm::vec3(width/2.0f,height/2.0f,0));
            d.points.push_back(glm::vec3(-width/2.0f,height/2.0f,0));

            d.points.push_back(glm::vec3(width/2.0f,-height/2.0f,0));
            d.points.push_back(glm::vec3(width/2.0f,height/2.0f,0));
            d.points.push_back(glm::vec3(-width/2.0f,-height/2.0f,0));

            float uv_topLeft_x = 0.0f;
            float uv_topLeft_y = 0.0f;

            float uv_bottomLeft_x = 0.0f;
            float uv_bottomLeft_y = 0.0f + float(height);

            float uv_bottomRight_x = 0.0f + float(width);
            float uv_bottomRight_y = 0.0f + float(height);

            float uv_topRight_x = 0.0f + float(width);
            float uv_topRight_y = 0.0f;

            d.uvs.push_back(glm::vec2(uv_bottomLeft_x,uv_bottomLeft_y));
            d.uvs.push_back(glm::vec2(uv_topRight_x,uv_topRight_y));
            d.uvs.push_back(glm::vec2(uv_topLeft_x,uv_topLeft_y));

            d.uvs.push_back(glm::vec2(uv_bottomRight_x,uv_bottomRight_y));
            d.uvs.push_back(glm::vec2(uv_topRight_x,uv_topRight_y));
            d.uvs.push_back(glm::vec2(uv_bottomLeft_x,uv_bottomLeft_y));

            d.normals.resize(6,glm::vec3(1));  d.binormals.resize(6,glm::vec3(1));  d.tangents.resize(6,glm::vec3(1));
            _initGlobalTwo(super,d,threshold);
        }
        void _init(Mesh* super,string& fileOrData,CollisionType::Type type,bool notMemory,float threshold,bool loadImmediately){//from file / data
            _initGlobal(threshold);
            m_Type = type;
            if(notMemory){
                m_File = fileOrData;
            }
            else{
                _loadFromOBJMemory(super,type,threshold, epriv::LOAD_FACES | epriv::LOAD_UVS | epriv::LOAD_NORMALS | epriv::LOAD_TBN,fileOrData);
            }
            super->setCustomBindFunctor(DEFAULT_BIND_FUNCTOR);
            super->setCustomUnbindFunctor(DEFAULT_UNBIND_FUNCTOR);
			if(loadImmediately){
                super->load();
			}
        }
        void _clearData(Mesh* super){
            vector_clear(m_Vertices);
            if(m_Skeleton != nullptr){
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
            if(m_aiScene->mAnimations && m_aiScene->mNumAnimations > 0 && mesh->skeleton() == nullptr){
                m_Skeleton = new MeshSkeleton();
                m_Skeleton->m_i->m_GlobalInverseTransform = Engine::Math::assimpToGLMMat4(m);
                doOther = true;
            }
            _processNode(mesh,data,m_aiScene->mRootNode,m_aiScene);
            if(doOther == true){
                m_Skeleton->fill(data);
            }
        }
        void _processNode(Mesh* mesh,epriv::ImportedMeshData& data,aiNode* node, const aiScene* scene){
            for(uint i = 0; i < node->mNumMeshes; ++i){
                aiMesh* aimesh = scene->mMeshes[node->mMeshes[i]];
                #pragma region vertices
                for(uint i = 0; i < aimesh->mNumVertices; ++i){
                    //pos
                    glm::vec3 pos;
                    pos.x = aimesh->mVertices[i].x;
                    pos.y = aimesh->mVertices[i].y;
                    pos.z = aimesh->mVertices[i].z;
                    data.points.push_back(pos);

                    //uv
                    glm::vec2 uv;
                    if(aimesh->mTextureCoords[0]){ uv.x = aimesh->mTextureCoords[0][i].x; uv.y = aimesh->mTextureCoords[0][i].y; }
                    else{ uv = glm::vec2(0.0f, 0.0f); }

                    //this is to prevent uv compression from beign f-ed up at the poles.
                    if(uv.y <= 0.0001f){ uv.y = 0.001f; }
                    if(uv.y >= 0.9999f){ uv.y = 0.999f; }
                    data.uvs.push_back(uv);

                    //norm
                    glm::vec3 norm;
                    if(aimesh->mNormals){
                        norm.x = aimesh->mNormals[i].x;
                        norm.y = aimesh->mNormals[i].y;
                        norm.z = aimesh->mNormals[i].z;
                        data.normals.push_back(norm);

                        //tangent
                        glm::vec3 tangent;
                        tangent.x = aimesh->mTangents[i].x;
                        tangent.y = aimesh->mTangents[i].y;
                        tangent.z = aimesh->mTangents[i].z;
                        //data.tangents.push_back(tangent);

                        //binorm
                        glm::vec3 binorm;
                        binorm.x = aimesh->mBitangents[i].x;
                        binorm.y = aimesh->mBitangents[i].y;
                        binorm.z = aimesh->mBitangents[i].z;
                        //data.binormals.push_back(binorm);
                    }
                }
                #pragma endregion
                // Process indices
                #pragma region indices
                for(uint i = 0; i < aimesh->mNumFaces; ++i){
                    aiFace face = aimesh->mFaces[i];
                    epriv::Triangle t;
                    for(uint j = 0; j < face.mNumIndices; ++j){
                        ushort index = (ushort)face.mIndices[j];
                        data.indices.push_back(index);
                        if(j == 0){
                            t.v1.position = data.points.at(index);
                            if(data.uvs.size() > 0) t.v1.uv = data.uvs.at(index);
                            if(data.normals.size() > 0) t.v1.normal = data.normals.at(index);
                        }
                        else if(j == 1){
                            t.v2.position = data.points.at(index);
                            if(data.uvs.size() > 0) t.v2.uv = data.uvs.at(index);
                            if(data.normals.size() > 0) t.v2.normal = data.normals.at(index);
                        }
                        else if(j == 2){
                            t.v3.position = data.points.at(index);
                            if(data.uvs.size() > 0) t.v3.uv = data.uvs.at(index);
                            if(data.normals.size() > 0) t.v3.normal = data.normals.at(index);
                            data.file_triangles.push_back(t);
                        }
                    }
                }
                #pragma endregion
                //bones
                #pragma region Skeleton
                if(aimesh->mNumBones > 0){
                    for (uint i = 0; i < aimesh->mNumBones; ++i) { 
                        uint BoneIndex = 0; 
                        string BoneName(aimesh->mBones[i]->mName.data);
                        if(!mesh->skeleton()->m_i->m_BoneMapping.count(BoneName)) {
                            BoneIndex = mesh->skeleton()->m_i->m_NumBones;
                            mesh->skeleton()->m_i->m_NumBones++; 
                            epriv::BoneInfo bi;
                            mesh->skeleton()->m_i->m_BoneInfo.push_back(bi);
                        }
                        else{
                            BoneIndex = mesh->skeleton()->m_i->m_BoneMapping.at(BoneName);
                        }
                        mesh->skeleton()->m_i->m_BoneMapping.emplace(BoneName,BoneIndex);
                        aiMatrix4x4 n = aimesh->mBones[i]->mOffsetMatrix;
                        mesh->skeleton()->m_i->m_BoneInfo.at(BoneIndex).BoneOffset = Engine::Math::assimpToGLMMat4(n);
                        for (uint j = 0; j < aimesh->mBones[i]->mNumWeights; ++j) {
                            uint VertexID = aimesh->mBones[i]->mWeights[j].mVertexId;
                            float Weight = aimesh->mBones[i]->mWeights[j].mWeight; 
                            epriv::VertexBoneData d;
                            d.AddBoneData(BoneIndex, Weight);
                            data.m_Bones.emplace(VertexID,d);
                        }
                    }
                }
                if(scene->mAnimations && scene->mNumAnimations > 0){
                    for(uint i = 0; i < scene->mNumAnimations; ++i){
                         aiAnimation* anim = scene->mAnimations[i];
                         string key(anim->mName.C_Str());
                         if(key == ""){
                             key = "Animation " + to_string(mesh->skeleton()->m_i->m_AnimationData.size());
                         }
                         if(!mesh->skeleton()->m_i->m_AnimationData.count(key)){
                            epriv::AnimationData* animData = new epriv::AnimationData(mesh,anim);
                            mesh->skeleton()->m_i->m_AnimationData.emplace(key,animData);
                         }
                    }
                }
                #pragma endregion
                //_calculateTBN(data);
                _calculateTBNAssimp(data);
                //_calculateGramSchmidt(data.points,data.normals,data.binormals,data.tangents);
            }
            for(uint i = 0; i < node->mNumChildren; ++i){
                _processNode(mesh,data,node->mChildren[i], scene);
            }
        }
        bool _is_near(float v1, float v2, float threshold){ return fabs( v1-v2 ) < threshold; }
        bool _is_special_float(float f){
            if(boost::math::isnan(f)) return true; if(boost::math::isinf(f)) return true;
            return false;
        }
        bool _is_special_float(glm::vec2 v){
            if(boost::math::isnan(v.x) || boost::math::isnan(v.y) ) return true; if(boost::math::isinf(v.x) || boost::math::isinf(v.y) ) return true;
            return false;
        }
        bool _is_special_float(glm::vec3 v){
            if(boost::math::isnan(v.x) || boost::math::isnan(v.y) || boost::math::isnan(v.z) ) return true;
            if(boost::math::isinf(v.x) || boost::math::isinf(v.y) || boost::math::isinf(v.z) ) return true;
            return false;
        }
        bool _getSimilarVertexIndex(glm::vec3& in_pos,glm::vec2& in_uv,glm::vec3& in_norm,vector<epriv::MeshVertexData>& out_vertices,vector<glm::vec2>& uvs,vector<glm::vec3>& norms,ushort& result, float threshold){
            for (uint i=0; i < out_vertices.size(); ++i ){
                if (_is_near( in_pos.x , out_vertices.at(i).position.x ,threshold) && _is_near( in_pos.y , out_vertices.at(i).position.y ,threshold) &&
                    _is_near( in_pos.z , out_vertices.at(i).position.z ,threshold) && _is_near( in_uv.x  , uvs.at(i).x      ,threshold) &&
                    _is_near( in_uv.y  , uvs.at(i).y      ,threshold) && _is_near( in_norm.x , norms.at(i).x ,threshold) &&
                    _is_near( in_norm.y , norms.at(i).y ,threshold) && _is_near( in_norm.z , norms.at(i).z ,threshold)
                ){
                    result = i;
                    return true;
                }
            }
            return false;
        }
        void _calculateGramSchmidt(vector<glm::vec3>& points,vector<glm::vec3>& normals,vector<glm::vec3>& binormals,vector<glm::vec3>& tangents){
            for(uint i=0; i < points.size(); ++i){
                glm::vec3& n = normals.at(i);
                glm::vec3& t = binormals.at(i);
                glm::vec3& b = tangents.at(i);
                // Gram-Schmidt orthogonalize
                t = glm::normalize(t - n * glm::dot(n, t));

                //handedness
                if (glm::dot(glm::cross(n, t), b) < 0.0f){
                     t = t * -1.0f;
                }
            }
        }
        void _calculateTBN(epriv::ImportedMeshData& data){
            if(data.normals.size() == 0) return;
            for(uint i=0; i < data.points.size(); i+=3){
                uint p0 = i + 0; uint p1 = i + 1; uint p2 = i + 2;

                if(p2 > data.points.size() || p1 > data.points.size()) break;

                glm::vec3 deltaPos1 = data.points.at(p1) - data.points.at(p0);
                glm::vec3 deltaPos2 = data.points.at(p2) - data.points.at(p0);

                glm::vec2 deltaUV1 = data.uvs.at(p1) - data.uvs.at(p0);
                glm::vec2 deltaUV2 = data.uvs.at(p2) - data.uvs.at(p0);

                float r = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);
                glm::vec3 tangent = r * (deltaUV2.y * deltaPos1 - deltaUV1.y * deltaPos2);
                glm::vec3 bitangent = r * (deltaUV1.x * deltaPos2 - deltaUV2.x * deltaPos1);
                tangent = glm::normalize(tangent); bitangent = glm::normalize(bitangent);

                glm::vec3 t1 = tangent;   glm::vec3 t2 = tangent;   glm::vec3 t3 = tangent;
                glm::vec3 b1 = bitangent; glm::vec3 b2 = bitangent; glm::vec3 b3 = bitangent;
                //do we even need these next 6 lines?
                t1 = glm::normalize(tangent - data.normals.at(p0) * glm::dot(data.normals.at(p0), tangent));
                t2 = glm::normalize(tangent - data.normals.at(p1) * glm::dot(data.normals.at(p1), tangent));
                t3 = glm::normalize(tangent - data.normals.at(p2) * glm::dot(data.normals.at(p2), tangent));

                b1 = glm::normalize(bitangent - data.normals.at(p0) * glm::dot(data.normals.at(p0), bitangent));
                b2 = glm::normalize(bitangent - data.normals.at(p1) * glm::dot(data.normals.at(p1), bitangent));
                b3 = glm::normalize(bitangent - data.normals.at(p2) * glm::dot(data.normals.at(p2), bitangent));
                //////////////////////////////////////
                data.tangents.push_back(t1); data.tangents.push_back(t2); data.tangents.push_back(t3);
                data.binormals.push_back(b1); data.binormals.push_back(b2); data.binormals.push_back(b3);
            }
        }
        void _calculateTBNAssimp(epriv::ImportedMeshData& data){
            if(data.normals.size() == 0) return;
            uint dataSize = data.points.size();
            for(uint i=0; i < dataSize; i+=3){
                uint p0 = i+0; uint p1 = i+1; uint p2 = i+2;

                glm::vec3 dataP0,dataP1,dataP2;
                glm::vec2 uvP0,uvP1,uvP2;

                uint uvSize = data.uvs.size();

                if(dataSize > p0) dataP0 = data.points.at(p0);
                else              dataP0 = glm::vec3(0.0f);
                if(dataSize > p1) dataP1 = data.points.at(p1);
                else              dataP1 = glm::vec3(0.0f);
                if(dataSize > p2) dataP2 = data.points.at(p2);
                else              dataP2 = glm::vec3(0.0f);

                if(uvSize > p0)   uvP0 = data.uvs.at(p0);
                else              uvP0 = glm::vec2(0.0f);
                if(uvSize > p1)   uvP1 = data.uvs.at(p1);
                else              uvP1 = glm::vec2(0.0f);
                if(uvSize > p2)   uvP2 = data.uvs.at(p2);
                else              uvP2 = glm::vec2(0.0f);

                glm::vec3 v = dataP1 - dataP0;
                glm::vec3 w = dataP2 - dataP0;

                // texture offset p1->p2 and p1->p3
                float sx = uvP1.x - uvP0.x;
                float sy = uvP1.y - uvP0.y;
                float tx = uvP2.x - uvP0.x;
                float ty = uvP2.y - uvP0.y;

                float dirCorrection = (tx * sy - ty * sx) < 0.0f ? -1.0f : 1.0f;
                // when t1, t2, t3 in same position in UV space, just use default UV direction.
                //if ( 0 == sx && 0 == sy && 0 == tx && 0 == ty ) {
                if ( sx * ty == sy * tx ){
                    sx = 0.0; sy = 1.0;
                    tx = 1.0; ty = 0.0;
                }

                // tangent points in the direction where to positive X axis of the texture coord's would point in model space
                // bitangent's points along the positive Y axis of the texture coord's, respectively
                glm::vec3 tangent;
                glm::vec3 bitangent;
                tangent.x   = (w.x * sy - v.x * ty) * dirCorrection;
                tangent.y   = (w.y * sy - v.y * ty) * dirCorrection;
                tangent.z   = (w.z * sy - v.z * ty) * dirCorrection;
                bitangent.x = (w.x * sx - v.x * tx) * dirCorrection;
                bitangent.y = (w.y * sx - v.y * tx) * dirCorrection;
                bitangent.z = (w.z * sx - v.z * tx) * dirCorrection;

                // store for every vertex of that face
                for( uint b = 0; b < 3; ++b ) {
                    uint p;
                    glm::vec3 normal;
                    if(b==0)      p = p0;
                    else if(b==1) p = p1;
                    else          p = p2;

                    if(data.normals.size() > p) normal = data.normals.at(p);
                    else                        normal = glm::vec3(0.0f);

                    // project tangent and bitangent into the plane formed by the vertex' normal
                    glm::vec3 localTangent   = tangent   - normal * (tangent   * normal);
                    glm::vec3 localBitangent = bitangent - normal * (bitangent * normal);
                    localTangent   = glm::normalize(localTangent);
                    localBitangent = glm::normalize(localBitangent);

                    // reconstruct tangent/bitangent according to normal and bitangent/tangent when it's infinite or NaN.
                    bool invalid_tangent   = _is_special_float(localTangent);
                    bool invalid_bitangent = _is_special_float(localBitangent);
                    if (invalid_tangent != invalid_bitangent){
                        if (invalid_tangent) localTangent   = glm::normalize(glm::cross(normal,localBitangent));
                        else                 localBitangent = glm::normalize(glm::cross(localTangent,normal));
                    }
                    data.tangents.push_back(localTangent);
                    data.binormals.push_back(localBitangent);
                }
            }
        }
        void _indexVBO(Mesh* mesh,epriv::ImportedMeshData& data,vector<ushort>& out_indices,vector<epriv::MeshVertexData>& out_vertices, float threshold){
            if(threshold == 0.0f){
                uint c = 0;
                for(auto pt:data.points){
                    if(mesh->skeleton() != nullptr){
                        epriv::MeshVertexDataAnimated vert;
                        vert.position = pt;
                        //vert.uv = Engine::Math::pack2FloatsInto1Float(data.uvs.at(c));
                        vert.uv = data.uvs.at(c);
                        vert.normal = Engine::Math::pack3NormalsInto32Int(data.normals.at(c));
                        if(c <= data.binormals.size()-1)
                            vert.binormal = Engine::Math::pack3NormalsInto32Int(data.binormals.at(c));
                        if(c <= data.tangents.size()-1)
                            vert.tangent = Engine::Math::pack3NormalsInto32Int(data.tangents.at(c));
                        out_vertices.push_back(vert);
                    }
                    else{
                        epriv::MeshVertexData vert;
                        vert.position = pt;
                        //vert.uv = Engine::Math::pack2FloatsInto1Float(data.uvs.at(c));
                        vert.uv = data.uvs.at(c);
                        vert.normal = Engine::Math::pack3NormalsInto32Int(data.normals.at(c));
                        vert.binormal = Engine::Math::pack3NormalsInto32Int(data.binormals.at(c));
                        vert.tangent = Engine::Math::pack3NormalsInto32Int(data.tangents.at(c));
                        out_vertices.push_back(vert);
                    }
                    c++;
                }
                out_indices = data.indices;
                return;
            }
            vector<glm::vec2> temp_uvs;
            vector<glm::vec3> temp_normals;
            vector<glm::vec3> temp_binormals;
            vector<glm::vec3> temp_tangents;
            for (uint i=0; i < data.points.size(); ++i){
                ushort index;
                bool found = _getSimilarVertexIndex(data.points.at(i), data.uvs.at(i), data.normals.at(i),out_vertices,temp_uvs,temp_normals, index,threshold);
                if (found){
                    out_indices.push_back(index);

                    //average out TBN. I think this does more harm than good though
                    //temp_binormals.at(index) += data.binormals.at(i);
                    //temp_tangents.at(index) += data.tangents.at(i);
                }
                else{
                    if(m_Skeleton != nullptr){
                        epriv::MeshVertexDataAnimated vert;
                        vert.position = data.points.at(i);
                        out_vertices.push_back(vert);
                    }
                    else{
                        epriv::MeshVertexData vert;
                        vert.position = data.points.at(i);
                        out_vertices.push_back(vert);
                    }
                    temp_uvs.push_back(data.uvs.at(i));

                    temp_normals .push_back(data.normals.at(i));
                    temp_binormals.push_back(data.binormals.at(i));
                    temp_tangents.push_back(data.tangents.at(i));

                    out_indices.push_back((ushort)out_vertices.size() - 1);
                }
            }
            for(uint i = 0; i < out_vertices.size(); ++i){
                if(m_Skeleton != nullptr){
                    epriv::MeshVertexDataAnimated& vert = static_cast<epriv::MeshVertexDataAnimated>(out_vertices.at(i));
                    //vert.uv = Engine::Math::pack2FloatsInto1Float(temp_uvs.at(i));
                    vert.uv = temp_uvs.at(i);
                    vert.normal = Engine::Math::pack3NormalsInto32Int(temp_normals.at(i));
                    vert.binormal = Engine::Math::pack3NormalsInto32Int(temp_binormals.at(i));
                    vert.tangent = Engine::Math::pack3NormalsInto32Int(temp_tangents.at(i));
                }
                else{
                    epriv::MeshVertexData& vert = out_vertices.at(i);
                    //vert.uv = Engine::Math::pack2FloatsInto1Float(temp_uvs.at(i));
                    vert.uv = temp_uvs.at(i);
                    vert.normal = Engine::Math::pack3NormalsInto32Int(temp_normals.at(i));
                    vert.binormal = Engine::Math::pack3NormalsInto32Int(temp_binormals.at(i));
                    vert.tangent = Engine::Math::pack3NormalsInto32Int(temp_tangents.at(i));
                }
            }
        }
        void _loadData(Mesh* super,epriv::ImportedMeshData& data,float threshold){
            m_threshold = threshold;

            if(data.uvs.size() == 0) data.uvs.resize(data.points.size());
            if(data.normals.size() == 0) data.normals.resize(data.points.size());
            if(data.binormals.size() == 0) data.binormals.resize(data.points.size());
            if(data.tangents.size() == 0) data.tangents.resize(data.points.size());

            _indexVBO(super,data,m_Indices,m_Vertices,m_threshold);
        }
        void _loadFromFile(Mesh* super,string file,CollisionType::Type type,float threshold){
            string extention; for(uint i = m_File.length() - 4; i < m_File.length(); ++i)extention += tolower(m_File.at(i));
            epriv::ImportedMeshData d;
            _loadInternal(super,d,m_File);
            m_threshold = threshold; //this is needed
            _loadData(super,d,m_threshold);

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
                glm::vec3 pos  = glm::vec3(0,0,0);
                glm::vec2 uv   = glm::vec2(0,0);
                glm::vec3 norm = glm::vec3(1,1,1);
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
        void _loadObjDataFromLine(string& l,epriv::ImportedMeshData& data,vector<uint>& _pi,vector<uint>& _ui,vector<uint>& _ni,const char _f){
            if(l[0] == 'o'){
            }
            //vertex positions
            else if(l[0] == 'v' && l[1] == ' '){ 
                if(_f && epriv::LOAD_POINTS){
                    glm::vec3 p;
                    sscanf(l.substr(2,l.size()).c_str(),"%f %f %f",&p.x,&p.y,&p.z);
                    data.file_points.push_back(p);
                }
            }
            //vertex uvs
            else if(l[0] == 'v' && l[1] == 't'){
                if(_f && epriv::LOAD_UVS){
                    glm::vec2 uv;
                    sscanf(l.substr(2,l.size()).c_str(),"%f %f",&uv.x,&uv.y);
                    uv.y = 1.0f - uv.y;
                    data.file_uvs.push_back(uv);
                }
            }
            //vertex normals
            else if(l[0] == 'v' && l[1] == 'n'){
                if(_f && epriv::LOAD_NORMALS){
                    glm::vec3 n;
                    sscanf(l.substr(2,l.size()).c_str(),"%f %f %f",&n.x,&n.y,&n.z);
                    data.file_normals.push_back(n);
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
                _calculateTBN(d);
            }
            _loadData(super,d,threshold);
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
            m_radius = Engine::Math::Max(m_radiusBox);
        }
        void _initRenderingContext(){
            m_buffers.push_back(0);
            glGenBuffers(1, &m_buffers.at(0));
            glBindBuffer(GL_ARRAY_BUFFER, m_buffers.at(0));
            if(m_Skeleton != nullptr){
                vector<epriv::MeshVertexDataAnimated> temp; //this is needed to store the bone info into the buffer.
                for(uint i = 0; i < m_Skeleton->m_i->m_BoneIDs.size(); ++i){
                    epriv::MeshVertexDataAnimated& vert = (epriv::MeshVertexDataAnimated)m_Vertices.at(i);
                    vert.boneIDs = m_Skeleton->m_i->m_BoneIDs.at(i);
                    vert.boneWeights = m_Skeleton->m_i->m_BoneWeights.at(i);
                    temp.push_back(vert);
                }
                glBufferData(GL_ARRAY_BUFFER, m_Vertices.size() * sizeof(epriv::MeshVertexDataAnimated),&temp[0], GL_DYNAMIC_DRAW );
                vector_clear(temp);
            }
            else{
                glBufferData(GL_ARRAY_BUFFER, m_Vertices.size() * sizeof(epriv::MeshVertexData),&m_Vertices[0], GL_DYNAMIC_DRAW );
            }
            m_buffers.push_back(0);
            glGenBuffers(1, &m_buffers.at(1));
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_buffers.at(1));
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_Indices.size() * sizeof(ushort), &m_Indices[0] , GL_STATIC_DRAW);
            //cannot clear indices buffer. just dont do it. ;)
            if(m_SaveMeshData == false){
                vector_clear(m_Vertices);
            }
        }
		void _modifyPoints(vector<glm::vec3>& modifiedPts){
			glBindBuffer(GL_ARRAY_BUFFER, m_buffers.at(0));
            if(m_Skeleton != nullptr){
                vector<epriv::MeshVertexDataAnimated> temp; //this is needed to store the bone info into the buffer.
                for(uint i = 0; i < m_Skeleton->m_i->m_BoneIDs.size(); ++i){
                    epriv::MeshVertexDataAnimated& vert = (epriv::MeshVertexDataAnimated)m_Vertices.at(i);
                    vert.boneIDs = m_Skeleton->m_i->m_BoneIDs.at(i);
                    vert.boneWeights = m_Skeleton->m_i->m_BoneWeights.at(i);
                    temp.push_back(vert);
                }
				for(uint i = 0; i < modifiedPts.size(); ++i){ temp.at(i).position = modifiedPts.at(i); }
                glBufferData(GL_ARRAY_BUFFER, m_Vertices.size() * sizeof(epriv::MeshVertexDataAnimated),&temp[0], GL_DYNAMIC_DRAW );
                vector_clear(temp);
            }
            else{
				for(uint i = 0; i < modifiedPts.size(); ++i){
					m_Vertices.at(i).position = modifiedPts.at(i);
				}
                glBufferData(GL_ARRAY_BUFFER, m_Vertices.size() * sizeof(epriv::MeshVertexData),&m_Vertices[0], GL_DYNAMIC_DRAW );
            }
		}
		void _modifyUVs(vector<glm::vec2>& modifiedUVs){
			glBindBuffer(GL_ARRAY_BUFFER, m_buffers.at(0));
            if(m_Skeleton != nullptr){
                vector<epriv::MeshVertexDataAnimated> temp; //this is needed to store the bone info into the buffer.
                for(uint i = 0; i < m_Skeleton->m_i->m_BoneIDs.size(); ++i){
                    epriv::MeshVertexDataAnimated& vert = (epriv::MeshVertexDataAnimated)m_Vertices.at(i);
                    vert.boneIDs = m_Skeleton->m_i->m_BoneIDs.at(i);
                    vert.boneWeights = m_Skeleton->m_i->m_BoneWeights.at(i);
                    temp.push_back(vert);
                }
				for(uint i = 0; i < modifiedUVs.size(); ++i){ temp.at(i).uv = modifiedUVs.at(i); }
                glBufferData(GL_ARRAY_BUFFER, m_Vertices.size() * sizeof(epriv::MeshVertexDataAnimated),&temp[0], GL_DYNAMIC_DRAW );
                vector_clear(temp);
            }
            else{
				for(uint i = 0; i < modifiedUVs.size(); ++i){ m_Vertices.at(i).uv = modifiedUVs.at(i); }
                glBufferData(GL_ARRAY_BUFFER, m_Vertices.size() * sizeof(epriv::MeshVertexData),&m_Vertices[0], GL_DYNAMIC_DRAW );
            }
		}
		void _modifyPointsAndUVs(vector<glm::vec3>& modifiedPts,vector<glm::vec2>& modifiedUVs){
			glBindBuffer(GL_ARRAY_BUFFER, m_buffers.at(0));
            if(m_Skeleton != nullptr){
                vector<epriv::MeshVertexDataAnimated> temp; //this is needed to store the bone info into the buffer.
                for(uint i = 0; i < m_Skeleton->m_i->m_BoneIDs.size(); ++i){
                    epriv::MeshVertexDataAnimated& vert = (epriv::MeshVertexDataAnimated)m_Vertices.at(i);
                    vert.boneIDs = m_Skeleton->m_i->m_BoneIDs.at(i);
                    vert.boneWeights = m_Skeleton->m_i->m_BoneWeights.at(i);
                    temp.push_back(vert);
                }
				for(uint i = 0; i < modifiedPts.size(); ++i){ temp.at(i).position = modifiedPts.at(i); }
				for(uint i = 0; i < modifiedUVs.size(); ++i){ temp.at(i).uv = modifiedUVs.at(i); }
                glBufferData(GL_ARRAY_BUFFER, m_Vertices.size() * sizeof(epriv::MeshVertexDataAnimated),&temp[0], GL_DYNAMIC_DRAW );
                vector_clear(temp);
            }
            else{
				for(uint i = 0; i < modifiedPts.size(); ++i){ m_Vertices.at(i).position = modifiedPts.at(i); }
				for(uint i = 0; i < modifiedUVs.size(); ++i){ m_Vertices.at(i).uv = modifiedUVs.at(i); }
                glBufferData(GL_ARRAY_BUFFER, m_Vertices.size() * sizeof(epriv::MeshVertexData),&m_Vertices[0], GL_DYNAMIC_DRAW );
            }
		}
        void _cleanupRenderingContext(Mesh* super){
            for(uint i = 0; i < m_buffers.size(); ++i){
                glDeleteBuffers(1,&m_buffers.at(i));
            }
        }
        void _loadIntoCPU(Mesh* super){
            if(m_File != ""){
                _loadFromFile(super,m_File,m_Type,m_threshold);
            }
            _calculateMeshRadius(super);
        }
        void _loadIntoGPU(){
            _initRenderingContext();
            cout << "(Mesh) ";
        }
        void _unload(Mesh* super){
            if(m_File != ""){
                _clearData(super);
            }
            _cleanupRenderingContext(super);
            cout << "(Mesh) ";
        }
};
struct DefaultMeshBindFunctor{void operator()(BindableResource* r) const {
    Mesh* mesh = (Mesh*)r;
    glBindBuffer(GL_ARRAY_BUFFER, mesh->m_i->m_buffers.at(0));
    if(mesh->m_i->m_Skeleton != nullptr){
        for(uint i = 0; i < epriv::VertexFormatAnimated::EnumTotal; ++i){
            boost::tuple<uint,GLuint,GLuint,GLuint>& d = epriv::VERTEX_ANIMATED_FORMAT_DATA.at(i);
            glEnableVertexAttribArray(i);
            glVertexAttribPointer(i,d.get<0>(),d.get<1>(),d.get<2>(),sizeof(epriv::MeshVertexDataAnimated),(void*)d.get<3>());
        }
    }else{     
        for(uint i = 0; i < epriv::VertexFormat::EnumTotal; ++i){
            boost::tuple<uint,GLuint,GLuint,GLuint>& d = epriv::VERTEX_ANIMATED_FORMAT_DATA.at(i);
            glEnableVertexAttribArray(i);
            glVertexAttribPointer(i,d.get<0>(),d.get<1>(),d.get<2>(),sizeof(epriv::MeshVertexData),(void*)d.get<3>());
        }
    }
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->m_i->m_buffers.at(1));
}};
struct DefaultMeshUnbindFunctor{void operator()(BindableResource* r) const {
    Mesh* mesh = (Mesh*)r;
    if(mesh->m_i->m_Skeleton != nullptr){
        for(uint i = 0; i < epriv::VertexFormatAnimated::EnumTotal; ++i){ glDisableVertexAttribArray(i); }
    }else{
        for(uint i = 0; i < epriv::VertexFormat::EnumTotal; ++i){ glDisableVertexAttribArray(i); }
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

        void _init(Mesh* mesh,aiAnimation* anim){
            m_Mesh = mesh;
            m_TicksPerSecond = anim->mTicksPerSecond;
            m_DurationInTicks = anim->mDuration;
            for(uint i = 0; i < anim->mNumChannels; ++i){
                string key = (anim->mChannels[i]->mNodeName.data);
                m_KeyframeData.emplace(key,anim->mChannels[i]);
            }
        }

        void _ReadNodeHeirarchy(const string& animationName,float time, const aiNode* n,glm::mat4& ParentTransform,vector<glm::mat4>& Transforms){
            string BoneName(n->mName.data);
            glm::mat4 NodeTransform = Engine::Math::assimpToGLMMat4(const_cast<aiMatrix4x4&>(n->mTransformation));
            if(m_KeyframeData.count(BoneName)){
                const aiNodeAnim* keyframes = m_KeyframeData.at(BoneName);
                if(keyframes){
                    glm::vec3 s; _CalcInterpolatedScaling(s, time, keyframes);
                    aiQuaternion q; _CalcInterpolatedRotation(q, time, keyframes);
                    glm::mat4 rotation = glm::mat4(Engine::Math::assimpToGLMMat3(q.GetMatrix()));
                    glm::vec3 t; _CalcInterpolatedPosition(t, time, keyframes);
                    NodeTransform = glm::mat4(1.0f);
                    NodeTransform = glm::translate(NodeTransform,t);
                    NodeTransform *= rotation;
                    NodeTransform = glm::scale(NodeTransform,s);
                }
            }
            glm::mat4 Transform = ParentTransform * NodeTransform;
            if(m_Mesh->m_i->m_Skeleton->m_i->m_BoneMapping.count(BoneName)){
                uint BoneIndex = m_Mesh->m_i->m_Skeleton->m_i->m_BoneMapping.at(BoneName);
                glm::mat4& Final = m_Mesh->m_i->m_Skeleton->m_i->m_BoneInfo.at(BoneIndex).FinalTransform;
                Final = m_Mesh->m_i->m_Skeleton->m_i->m_GlobalInverseTransform * Transform * m_Mesh->m_i->m_Skeleton->m_i->m_BoneInfo.at(BoneIndex).BoneOffset;
                //this line allows for animation combinations. only works when additional animations start off in their resting places...
                Final = Transforms.at(BoneIndex) * Final;
            }
            for(uint i = 0; i < n->mNumChildren; ++i){
                _ReadNodeHeirarchy(animationName,time,n->mChildren[i],Transform,Transforms);
            }
        }
        void _BoneTransform(const string& animationName,float TimeInSeconds, vector<glm::mat4>& Transforms){
            float TicksPerSecond = float(m_TicksPerSecond != 0 ? m_TicksPerSecond : 25.0f);
            float TimeInTicks = TimeInSeconds * TicksPerSecond;
            float AnimationTime = float(fmod(TimeInTicks, m_DurationInTicks));
            glm::mat4 Identity = glm::mat4(1.0f);
            _ReadNodeHeirarchy(animationName,AnimationTime, m_Mesh->m_i->m_aiScene->mRootNode, Identity,Transforms);
            for(uint i = 0; i < m_Mesh->m_i->m_Skeleton->m_i->m_NumBones; ++i){
                Transforms.at(i) = m_Mesh->m_i->m_Skeleton->m_i->m_BoneInfo.at(i).FinalTransform;
            }
        }
        void _CalcInterpolatedPosition(glm::vec3& Out, float AnimationTime, const aiNodeAnim* node){
            if (node->mNumPositionKeys == 1) {
                Out = Math::assimpToGLMVec3(node->mPositionKeys[0].mValue); return;
            }
            uint PositionIndex = _FindPosition(AnimationTime,node);
            uint NextIndex = PositionIndex + 1;
            float DeltaTime = (float)(node->mPositionKeys[NextIndex].mTime - node->mPositionKeys[PositionIndex].mTime);
            float Factor = (AnimationTime - (float)node->mPositionKeys[PositionIndex].mTime) / DeltaTime;
            glm::vec3 Start = Math::assimpToGLMVec3(node->mPositionKeys[PositionIndex].mValue);
            glm::vec3 End = Math::assimpToGLMVec3(node->mPositionKeys[NextIndex].mValue);
            glm::vec3 Delta = End - Start;
            Out = Start + Factor * Delta;
        }
        void _CalcInterpolatedRotation(aiQuaternion& Out, float AnimationTime, const aiNodeAnim* node){
            if (node->mNumRotationKeys == 1) {
                Out = node->mRotationKeys[0].mValue; return;
            }
            uint RotationIndex = _FindRotation(AnimationTime, node);
            uint NextIndex = RotationIndex + 1;
            float DeltaTime = (float)(node->mRotationKeys[NextIndex].mTime - node->mRotationKeys[RotationIndex].mTime);
            float Factor = (AnimationTime - (float)node->mRotationKeys[RotationIndex].mTime) / DeltaTime;
            const aiQuaternion& StartRotationQ = node->mRotationKeys[RotationIndex].mValue;
            const aiQuaternion& EndRotationQ   = node->mRotationKeys[NextIndex].mValue;    
            aiQuaternion::Interpolate(Out, StartRotationQ, EndRotationQ, Factor);
            Out = Out.Normalize();
        }
        void _CalcInterpolatedScaling(glm::vec3& Out, float AnimationTime, const aiNodeAnim* node){
            if (node->mNumScalingKeys == 1) {
                Out = Math::assimpToGLMVec3(node->mScalingKeys[0].mValue); return;
            }
            uint ScalingIndex = _FindScaling(AnimationTime, node);
            uint NextIndex = ScalingIndex + 1;
            float DeltaTime = (float)(node->mScalingKeys[NextIndex].mTime - node->mScalingKeys[ScalingIndex].mTime);
            float Factor = (AnimationTime - (float)node->mScalingKeys[ScalingIndex].mTime) / DeltaTime;
            glm::vec3 Start = Math::assimpToGLMVec3(node->mScalingKeys[ScalingIndex].mValue);
            glm::vec3 End   = Math::assimpToGLMVec3(node->mScalingKeys[NextIndex].mValue);
            glm::vec3 Delta = End - Start;
            Out = Start + Factor * Delta;
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
            float TicksPerSecond = float(m_TicksPerSecond != 0 ? m_TicksPerSecond : 25.0f);
            return float(float(m_DurationInTicks) / TicksPerSecond);
        }
};


MeshSkeleton::MeshSkeleton():m_i(new impl){
    clear();
}
MeshSkeleton::MeshSkeleton(epriv::ImportedMeshData& data):m_i(new impl){
    fill(data);
}
void MeshSkeleton::fill(epriv::ImportedMeshData& data){ m_i->_fill(data); }
void MeshSkeleton::clear(){ m_i->_clear(); }
MeshSkeleton::~MeshSkeleton(){
    clear();
}
uint MeshSkeleton::numBones(){ return m_i->m_NumBones; }



void InternalMeshPublicInterface::LoadCPU(Mesh* mesh){
    if(!mesh->isLoaded()){
        mesh->m_i->_loadIntoCPU(mesh);
    }
}
void InternalMeshPublicInterface::LoadGPU(Mesh* mesh){
    if(!mesh->isLoaded()){
        mesh->m_i->_loadIntoGPU();
		mesh->EngineResource::load();
    }
}



Mesh::Mesh(string name,btHeightfieldTerrainShape* heightfield,float threshold):BindableResource(name),m_i(new impl){
    m_i->_init(this,name,heightfield,threshold);
}
Mesh::Mesh(string name,unordered_map<string,float>& grid,uint width,uint length,float threshold):BindableResource(name),m_i(new impl){
    m_i->_init(this,name,grid,width,length,threshold);
}
Mesh::Mesh(string name,float x, float y,float width, float height,float threshold):BindableResource(name),m_i(new impl){
    m_i->_init(this,name,x,y,width,height,threshold);
}
Mesh::Mesh(string name,float width, float height,float threshold):BindableResource(name),m_i(new impl){
    m_i->_init(this,name,width,height,threshold);
}
Mesh::Mesh(string fileOrData,CollisionType::Type type,bool notMemory,float threshold,bool loadImmediately):BindableResource(fileOrData),m_i(new impl){
    if(!notMemory) setName("CustomMesh");
    m_i->_init(this,fileOrData,type,notMemory,threshold,loadImmediately);
}
Mesh::~Mesh(){
    this->unload();
    m_i->_clearData(this);
}
Collision* Mesh::getCollision() const { return m_i->m_Collision; }

std::unordered_map<std::string, Engine::epriv::AnimationData*>& Mesh::animationData(){ return m_i->m_Skeleton->m_i->m_AnimationData; }

const glm::vec3& Mesh::getRadiusBox() const { return m_i->m_radiusBox; }
const float Mesh::getRadius() const { return m_i->m_radius; }
MeshSkeleton* Mesh::skeleton(){ return m_i->m_Skeleton; }
void Mesh::render(GLuint mode){
    glDrawElements(mode,m_i->m_Indices.size(),GL_UNSIGNED_SHORT,0);
}
void Mesh::playAnimation(vector<glm::mat4>& transforms,const string& animationName,float time){
    m_i->m_Skeleton->m_i->m_AnimationData[animationName]->m_i->_BoneTransform(animationName,time, transforms);
}
void Mesh::load(){
    if(!isLoaded()){
        m_i->_loadIntoCPU(this);
        m_i->_loadIntoGPU();
        EngineResource::load();
    }
}
void Mesh::unload(){
    if(isLoaded() && useCount() == 0){
        m_i->_unload(this);
        EngineResource::unload();
    }
}
void Mesh::modifyPoints(vector<glm::vec3>& modifiedPts){ m_i->_modifyPoints(modifiedPts); }
void Mesh::modifyUVs(vector<glm::vec2>& modifiedUVs){ m_i->_modifyUVs(modifiedUVs); }
void Mesh::modifyPointsAndUVs(vector<glm::vec3>& modifiedPts, vector<glm::vec2>& modifiedUVs){ m_i->_modifyPointsAndUVs(modifiedPts,modifiedUVs); }
void Mesh::saveMeshData(bool save){ m_i->m_SaveMeshData = save; }


epriv::AnimationData::AnimationData(Mesh* m,aiAnimation* a):m_i(new impl){ m_i->_init(m,a); }
epriv::AnimationData::~AnimationData(){ }
float epriv::AnimationData::duration(){ return m_i->_Duration(); }

