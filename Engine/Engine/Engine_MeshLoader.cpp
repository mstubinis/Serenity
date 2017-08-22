#include "Engine_MeshLoader.h"
#include "Mesh.h"

#include <boost/filesystem.hpp>
#include <boost/iostreams/device/mapped_file.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/math/special_functions/fpclassify.hpp>
#include <map>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <glm/mat4x4.hpp>

using namespace Engine::Resources;
using namespace std;

bool is_near(float v1, float v2, float threshold){ return fabs( v1-v2 ) < threshold; }
bool is_special_float(float f){
    if(boost::math::isnan(f)) return true;
    if(boost::math::isinf(f)) return true;
    return false;
}

void MeshLoader::load(Mesh* mesh,ImportedMeshData& data,string file){	
    MeshLoader::Detail::MeshLoadingManagement::_load(mesh,data,file);
}
void MeshLoader::Detail::MeshLoadingManagement::_load(Mesh* mesh,ImportedMeshData& data,string file){
    mesh->m_aiScene = mesh->m_Importer.ReadFile(file,aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace); 
    if(!mesh->m_aiScene || mesh->m_aiScene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !mesh->m_aiScene->mRootNode){
        return;
    }
    //animation stuff
    aiMatrix4x4 m = mesh->m_aiScene->mRootNode->mTransformation; // node->mTransformation?
    m.Inverse();
    bool doOther = false;
    if(mesh->m_aiScene->mAnimations && mesh->m_aiScene->mNumAnimations > 0 && mesh->m_Skeleton == nullptr){
        mesh->m_Skeleton = new MeshSkeleton();
        mesh->m_Skeleton->m_GlobalInverseTransform = Engine::Math::assimpToGLMMat4(m);
        doOther = true;
    }
    MeshLoader::Detail::MeshLoadingManagement::_processNode(mesh,data,mesh->m_aiScene->mRootNode, mesh->m_aiScene);

    if(doOther == true){
        mesh->m_Skeleton->fill(data);
    }
}
void MeshLoader::Detail::MeshLoadingManagement::_processNode(Mesh* mesh,ImportedMeshData& data,aiNode* node, const aiScene* scene){
    for(uint i = 0; i < node->mNumMeshes; i++){
        aiMesh* aimesh = scene->mMeshes[node->mMeshes[i]];
        #pragma region vertices
        for(uint i = 0; i < aimesh->mNumVertices; i++){
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
                //norm = glm::normalize(norm);
                data.normals.push_back(norm);

                //tangent
                glm::vec3 tangent;
                tangent.x = aimesh->mTangents[i].x;
                tangent.y = aimesh->mTangents[i].y;
                tangent.z = aimesh->mTangents[i].z;
                //tangent = glm::normalize(tangent);
                //data.tangents.push_back(tangent);

                //binorm
                glm::vec3 binorm;
                binorm.x = aimesh->mBitangents[i].x;
                binorm.y = aimesh->mBitangents[i].y;
                binorm.z = aimesh->mBitangents[i].z;
                //binorm = glm::normalize(binorm);
                //data.binormals.push_back(binorm);
            }
        }
        #pragma endregion
        // Process indices
        #pragma region indices
        for(uint i = 0; i < aimesh->mNumFaces; i++){
            aiFace face = aimesh->mFaces[i];
            Triangle t;
            for(uint j = 0; j < face.mNumIndices; j++){
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
        if(aimesh->mNumBones > 0){
            for (uint i = 0; i < aimesh->mNumBones; i++) { 
                uint BoneIndex = 0; 
                string BoneName(aimesh->mBones[i]->mName.data);
                if(!mesh->m_Skeleton->m_BoneMapping.count(BoneName)) {
                    BoneIndex = mesh->m_Skeleton->m_NumBones;
                    mesh->m_Skeleton->m_NumBones++; 
                    BoneInfo bi;
                    mesh->m_Skeleton->m_BoneInfo.push_back(bi);
                }
                else{
                    BoneIndex = mesh->m_Skeleton->m_BoneMapping.at(BoneName);
                }
                mesh->m_Skeleton->m_BoneMapping.emplace(BoneName,BoneIndex);
                aiMatrix4x4 n = aimesh->mBones[i]->mOffsetMatrix;
                mesh->m_Skeleton->m_BoneInfo[BoneIndex].BoneOffset = Engine::Math::assimpToGLMMat4(n);
                for (uint j = 0; j < aimesh->mBones[i]->mNumWeights; j++) {
                    uint VertexID = aimesh->mBones[i]->mWeights[j].mVertexId;
                    float Weight = aimesh->mBones[i]->mWeights[j].mWeight; 
                    VertexBoneData d;
                    d.AddBoneData(BoneIndex, Weight);
                    data.m_Bones.emplace(VertexID,d);
                }
            }
        }
        if(scene->mAnimations && scene->mNumAnimations > 0){
            for(uint i = 0; i < scene->mNumAnimations; i++){
                 aiAnimation* anim = scene->mAnimations[i];
                 string key(anim->mName.C_Str());
                 if(key == ""){
                     key = "Animation " + to_string(mesh->m_Skeleton->m_AnimationData.size());
                 }
                 if(!mesh->m_Skeleton->m_AnimationData.count(key)){
                    AnimationData* animData = new AnimationData(mesh,anim);
                    mesh->m_Skeleton->m_AnimationData.emplace(key,animData);
                 }
            }
        }
		_calculateTBN(data);
        //_calculateGramSchmidt(data.points,data.normals,data.binormals,data.tangents);
    }
    for(uint i = 0; i < node->mNumChildren; i++){
        MeshLoader::Detail::MeshLoadingManagement::_processNode(mesh,data,node->mChildren[i], scene);
    }
}

void MeshLoader::Detail::_OBJ::_loadObjDataFromLine(string& l,ImportedMeshData& data,vector<uint>& _pi,vector<uint>& _ui,vector<uint>& _ni,const char _f){
    if(l[0] == 'o'){
    }
    //vertex positions
    else if(l[0] == 'v' && l[1] == ' '){ 
        if(_f && LOAD_POINTS){
            glm::vec3 p;
            sscanf(l.substr(2,l.size()).c_str(),"%f %f %f",&p.x,&p.y,&p.z);
            data.file_points.push_back(p);
        }
    }
    //vertex uvs
    else if(l[0] == 'v' && l[1] == 't'){
        if(_f && LOAD_UVS){
            glm::vec2 uv;
            sscanf(l.substr(2,l.size()).c_str(),"%f %f",&uv.x,&uv.y);
            uv.y = 1.0f - uv.y;
            data.file_uvs.push_back(uv);
        }
    }
    //vertex normals
    else if(l[0] == 'v' && l[1] == 'n'){
        if(_f && LOAD_NORMALS){
            glm::vec3 n;
            sscanf(l.substr(2,l.size()).c_str(),"%f %f %f",&n.x,&n.y,&n.z);
            data.file_normals.push_back(n);
        }
    }
    //faces
    else if(l[0] == 'f' && l[1] == ' '){
        if(_f && LOAD_FACES){
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

void MeshLoader::loadObjFromMemory(ImportedMeshData& data,string input,unsigned char _flags){
    vector<uint> positionIndices;
    vector<uint> uvIndices;
    vector<uint> normalIndices;

    istringstream stream;
    stream.str(input);

    //first read in all data
    for(string line; getline(stream, line, '\n');){
        MeshLoader::Detail::_OBJ::_loadObjDataFromLine(line,data,positionIndices,uvIndices,normalIndices,_flags);
    }
    if(_flags && LOAD_FACES){
        MeshLoader::Detail::MeshLoadingManagement::_loadDataIntoTriangles(data,positionIndices,uvIndices,normalIndices,_flags);
    }
    if(_flags && LOAD_TBN && data.normals.size() > 0){
        MeshLoader::Detail::MeshLoadingManagement::_calculateTBN(data);
    }
}
void MeshLoader::Detail::MeshLoadingManagement::_loadDataIntoTriangles(ImportedMeshData& data,vector<uint>& _pi,vector<uint>& _ui,vector<uint>& _ni,unsigned char _flags){
    uint count = 0;
    Triangle triangle;
    for(uint i=0; i < _pi.size(); i++ ){
        glm::vec3 pos  = glm::vec3(0,0,0);
        glm::vec2 uv   = glm::vec2(0,0);
        glm::vec3 norm = glm::vec3(1,1,1);
        if(_flags && LOAD_POINTS && data.file_points.size() > 0){  
            pos = data.file_points.at(_pi[i]-1);
            data.points.push_back(pos);
        }
        if(_flags && LOAD_UVS && data.file_uvs.size() > 0){
            uv  = data.file_uvs.at(_ui[i]-1);
            data.uvs.push_back(uv);
        }
        if(_flags && LOAD_NORMALS && data.file_normals.size() > 0){ 
            norm = data.file_normals.at(_ni[i]-1);
            data.normals.push_back(norm);
        }
        count++;
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
void MeshLoader::Detail::MeshLoadingManagement::_calculateTBNAssimp(ImportedMeshData& data){
    if(data.normals.size() == 0) return;
    for(uint i=0; i < data.points.size(); i+=3){
        uint p0 = i + 0; uint p1 = i + 1; uint p2 = i + 2;
        
        glm::vec3 v = data.points[p1] - data.points[p0];
        glm::vec3 w = data.points[p2] - data.points[p0];
        
        // texture offset p1->p2 and p1->p3
        float sx = data.uvs[p1].x - data.uvs[p0].x;
        float sy = data.uvs[p1].y - data.uvs[p0].y;
        float tx = data.uvs[p2].x - data.uvs[p0].x;
        float ty = data.uvs[p2].y - data.uvs[p0].y;
        
        float dirCorrection = (tx * sy - ty * sx) < 0.0f ? -1.0f : 1.0f;
        // when t1, t2, t3 in same position in UV space, just use default UV direction.
        //if ( 0 == sx && 0 ==sy && 0 == tx && 0 == ty ) {
        if ( sx * ty == sy * tx ){
            sx = 0.0; sy = 1.0;
            tx = 1.0; ty = 0.0;
        }
        // tangent points in the direction where to positive X axis of the texture coord's would point in model space
        // bitangent's points along the positive Y axis of the texture coord's, respectively
        glm::vec3 tangent; glm::vec3 bitangent;
        tangent.x = (w.x * sy - v.x * ty) * dirCorrection;
        tangent.y = (w.y * sy - v.y * ty) * dirCorrection;
        tangent.z = (w.z * sy - v.z * ty) * dirCorrection;
        bitangent.x = (w.x * sx - v.x * tx) * dirCorrection;
        bitangent.y = (w.y * sx - v.y * tx) * dirCorrection;
        bitangent.z = (w.z * sx - v.z * tx) * dirCorrection;
        
        // store for every vertex of that face
        for( uint b = 0; b < 3; ++b ) {
            uint p;
            if(b==0)      p = p0;
            else if(b==1) p = p1;
            else          p = p2;

            // project tangent and bitangent into the plane formed by the vertex' normal
            glm::vec3 localTangent = tangent - data.normals[p] * (tangent * data.normals[p]);
            glm::vec3 localBitangent = bitangent - data.normals[p] * (bitangent * data.normals[p]);
            localTangent = glm::normalize(localTangent);
            localBitangent = glm::normalize(localBitangent);

            // reconstruct tangent/bitangent according to normal and bitangent/tangent when it's infinite or NaN.
            bool invalid_tangent = is_special_float(localTangent.x) || is_special_float(localTangent.y) || is_special_float(localTangent.z);
            bool invalid_bitangent = is_special_float(localBitangent.x) || is_special_float(localBitangent.y) || is_special_float(localBitangent.z);
            if (invalid_tangent != invalid_bitangent) {
                if (invalid_tangent) {
                    localTangent = glm::normalize(glm::cross(data.normals[p],localBitangent));
                } else {
                    localBitangent = glm::normalize(glm::cross(localTangent,data.normals[p]));
                }
            }
            data.tangents.push_back(localTangent);
            data.binormals.push_back(localBitangent);
        }
    }
}
void MeshLoader::Detail::MeshLoadingManagement::_calculateTBN(ImportedMeshData& data){
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
        
        glm::vec3 t1 = tangent; glm::vec3 t2 = tangent; glm::vec3 t3 = tangent;
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

bool MeshLoader::Detail::MeshLoadingManagement::_getSimilarVertexIndex(glm::vec3& in_pos,glm::vec2& in_uv,glm::vec3& in_norm,vector<glm::vec3>& out_vertices,vector<glm::vec2>& out_uvs,vector<glm::vec3>& out_normals,ushort& result, float threshold){
    for (uint i=0; i < out_vertices.size(); i++ ){
        if (is_near( in_pos.x , out_vertices[i].x ,threshold) && is_near( in_pos.y , out_vertices[i].y ,threshold) &&
            is_near( in_pos.z , out_vertices[i].z ,threshold) && is_near( in_uv.x  , out_uvs[i].x      ,threshold) &&
            is_near( in_uv.y  , out_uvs[i].y      ,threshold) && is_near( in_norm.x , out_normals[i].x ,threshold) &&
            is_near( in_norm.y , out_normals[i].y ,threshold) && is_near( in_norm.z , out_normals[i].z ,threshold)
        ){
            result = i;
            return true;
        }
    }
    return false;
}
void MeshLoader::Detail::MeshLoadingManagement::_calculateGramSchmidt(vector<glm::vec3>& points,vector<glm::vec3>& normals,vector<glm::vec3>& binormals,vector<glm::vec3>& tangents){
    for(uint i=0; i < points.size(); i++){
        glm::vec3& n = normals[i];
        glm::vec3& t = binormals[i];
        glm::vec3& b = tangents[i];
        // Gram-Schmidt orthogonalize
        t = glm::normalize(t - glm::dot(t, n) * n);
        b = glm::cross(n, t);

        //handedness
        //if (glm::dot(glm::cross(n, t), b) < 0.0f){
        //     t = t * -1.0f;
        //}
    }
}
void MeshLoader::Detail::MeshLoadingManagement::_indexVBO(ImportedMeshData& data,vector<ushort> & out_indices,vector<glm::vec3>& out_pos,vector<float>& out_uvs,vector<GLuint>& out_norm,vector<GLuint>& out_binorm,vector<GLuint>& out_tangents, float threshold){
    if(threshold == 0.0f){
        out_pos = data.points;
        for(auto uvs:data.uvs){ out_uvs.push_back(Engine::Math::pack2FloatsInto1Float(uvs)); }
        
        for(auto normals:data.normals){ out_norm.push_back(Engine::Math::pack3NormalsInto32Int(normals)); }
        for(auto binormals:data.binormals){ out_binorm.push_back(Engine::Math::pack3NormalsInto32Int(binormals)); }
        for(auto tangents:data.tangents){ out_tangents.push_back(Engine::Math::pack3NormalsInto32Int(tangents)); }

        out_indices = data.indices;
        return;
    }
    vector<glm::vec2> temp_uvs;
    vector<glm::vec3> temp_normals;
    vector<glm::vec3> temp_binormals;
    vector<glm::vec3> temp_tangents;
    for (uint i=0; i < data.points.size(); i++){
        ushort index;
        bool found = _getSimilarVertexIndex(data.points.at(i), data.uvs.at(i), data.normals.at(i),out_pos, temp_uvs, temp_normals, index,threshold);
        if (found){
            out_indices.push_back(index);

            //we wont average TBN. this is because assimp smoothes the averages for us
            
            //also note: assimp source: if ( 0 == sx && 0 ==sy && 0 == tx && 0 == ty ). should be replaced with:
            // if ( sx * ty == sy * tx ). in their CalcTangentsProcess.cpp file
            
            //average out TBN. I think this does more harm than good though
            temp_binormals.at(index) += data.binormals.at(i);
            temp_tangents.at(index) += data.tangents.at(i);
        }
        else{
            out_pos.push_back( data.points.at(i));
            temp_uvs.push_back(data.uvs.at(i));

            temp_normals .push_back(data.normals.at(i));
            temp_binormals.push_back(data.binormals.at(i));
            temp_tangents.push_back(data.tangents.at(i));

            out_indices.push_back((ushort)out_pos.size() - 1);
        }
    }
    for(auto uvs:temp_uvs){ out_uvs.push_back(Engine::Math::pack2FloatsInto1Float(uvs)); }
    for(auto normals:temp_normals){ out_norm.push_back(Engine::Math::pack3NormalsInto32Int(normals)); }
    for(auto binormals:temp_binormals){ out_binorm.push_back(Engine::Math::pack3NormalsInto32Int(binormals)); }
    for(auto tangents:temp_tangents){ out_tangents.push_back(Engine::Math::pack3NormalsInto32Int(tangents)); }
}
