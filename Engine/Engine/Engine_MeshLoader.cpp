#include "Engine_MeshLoader.h"
#include "Mesh.h"

#include <boost/filesystem.hpp>
#include <boost/iostreams/device/mapped_file.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <map>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

using namespace Engine::Resources;

bool is_near(float v1, float v2, float threshold){ return fabs( v1-v2 ) < threshold; }


void MeshLoader::load(Mesh* mesh,ImportedMeshData& data, std::string file){	
	MeshLoader::Detail::MeshLoadingManagement::_load(mesh,data,file);
}
void MeshLoader::Detail::MeshLoadingManagement::_load(Mesh* mesh,ImportedMeshData& data, std::string file){	
	mesh->m_aiScene = mesh->m_Importer.ReadFile(file,aiProcess_FlipUVs | aiProcess_CalcTangentSpace | 
		aiProcess_Triangulate); 
    if(!mesh->m_aiScene || mesh->m_aiScene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !mesh->m_aiScene->mRootNode){
        return;
    }
    //animation stuff
    aiMatrix4x4 m = mesh->m_aiScene->mRootNode->mTransformation; // node->mTransformation?
    m.Inverse();
	data.m_GlobalInverseTransform = glm::mat4(m.a1,m.a2,m.a3,m.a4,
                                              m.b1,m.b2,m.b3,m.b4,
                                              m.c1,m.c2,m.c3,m.c4,
                                              m.d1,m.d2,m.d3,m.d4);
    MeshLoader::Detail::MeshLoadingManagement::_processNode(mesh,data,mesh->m_aiScene->mRootNode, mesh->m_aiScene);
}
void MeshLoader::Detail::MeshLoadingManagement::_processNode(Mesh* mesh,ImportedMeshData& data,aiNode* node, const aiScene* scene){
    for(uint i = 0; i < node->mNumMeshes; i++){
        aiMesh* aimesh = scene->mMeshes[node->mMeshes[i]];
        #pragma region vertices
        for(uint i = 0; i < aimesh->mNumVertices; i++){
            //pos
            glm::vec3 pos;
            pos.x = aimesh->mVertices[i].x; pos.y = aimesh->mVertices[i].y; pos.z = aimesh->mVertices[i].z;
            data.points.push_back(pos);

            //uv
            glm::vec2 uv;
            if(aimesh->mTextureCoords[0]){ uv.x = aimesh->mTextureCoords[0][i].x; uv.y = aimesh->mTextureCoords[0][i].y; }
            else{ uv = glm::vec2(0.0f, 0.0f); }
            data.uvs.push_back(uv);

            //norm
            glm::vec3 norm;
            if(aimesh->mNormals){
                norm.x = aimesh->mNormals[i].x; norm.y = aimesh->mNormals[i].y; norm.z = aimesh->mNormals[i].z;
                data.normals.push_back(norm);

                //binorm
                glm::vec3 binorm;
                binorm.x = aimesh->mBitangents[i].x; binorm.y = aimesh->mBitangents[i].y; binorm.z = aimesh->mBitangents[i].z;
                data.binormals.push_back(binorm);

                //tangent
                glm::vec3 tangent;
                tangent.x = aimesh->mTangents[i].x; tangent.y = aimesh->mTangents[i].y; tangent.z = aimesh->mTangents[i].z;
                data.tangents.push_back(tangent);
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
				std::string BoneName(aimesh->mBones[i]->mName.data);
				if (data.m_BoneMapping.find(BoneName) == data.m_BoneMapping.end()) {
					BoneIndex = data.m_NumBones;
					data.m_NumBones++; 
					BoneInfo bi;
					data.m_BoneInfo.push_back(bi);
				}
				else{
					BoneIndex = data.m_BoneMapping[BoneName];
				}
				data.m_BoneMapping[BoneName] = BoneIndex;

				aiMatrix4x4 n = aimesh->mBones[i]->mOffsetMatrix;
				data.m_BoneInfo[BoneIndex].BoneOffset = glm::mat4(n.a1,n.a2,n.a3,n.a4,n.b1,n.b2,n.b3,n.b4,n.c1,n.c2,n.c3,n.c4,n.d1,n.d2,n.d3,n.d4);

				for (uint j = 0; j < aimesh->mBones[i]->mNumWeights; j++) {
					uint VertexID = aimesh->mBones[i]->mWeights[j].mVertexId;
					float Weight = aimesh->mBones[i]->mWeights[j].mWeight; 
					VertexBoneData d;
					d.AddBoneData(BoneIndex, Weight);
					data.m_Bones[VertexID] = d;
				}
			}
		}
		if(scene->mAnimations && scene->mNumAnimations > 0){
            for(uint i = 0; i < scene->mNumAnimations; i++){
                 aiAnimation* anim = scene->mAnimations[i];
                 AnimationData* animData = new AnimationData(mesh,anim);
                 std::string key(anim->mName.C_Str());
				 if(key == ""){
					 key = "Animation " + boost::lexical_cast<std::string>(mesh->animations().size());
				 }
                 mesh->animations().emplace(key,animData);
            }
        }
    }
    for(uint i = 0; i < node->mNumChildren; i++){
        MeshLoader::Detail::MeshLoadingManagement::_processNode(mesh,data,node->mChildren[i], scene);
    }
}

void MeshLoader::Detail::_OBJ::_loadObjDataFromLine(std::string& l,ImportedMeshData& data, std::vector<uint>& _pi, std::vector<uint>& _ui, std::vector<uint>& _ni, const char _f){
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

void MeshLoader::loadObjFromMemory(ImportedMeshData& data,std::string input,unsigned char _flags){
    std::vector<uint> positionIndices;
    std::vector<uint> uvIndices;
    std::vector<uint> normalIndices;

    std::istringstream stream;
    stream.str(input);

    //first read in all data
    for(std::string line; std::getline(stream, line, '\n');){
        MeshLoader::Detail::_OBJ::_loadObjDataFromLine(line,data,positionIndices,uvIndices,normalIndices,_flags);
    }
    if(_flags && LOAD_FACES){
        MeshLoader::Detail::MeshLoadingManagement::_loadDataIntoTriangles(data,positionIndices,uvIndices,normalIndices,_flags);
    }
    if(_flags && LOAD_TBN && data.normals.size() > 0){
        MeshLoader::Detail::MeshLoadingManagement::_calculateTBN(data);
    }
}
void MeshLoader::Detail::MeshLoadingManagement::_loadDataIntoTriangles(ImportedMeshData& data,std::vector<uint>& _pi, std::vector<uint>& _ui,std::vector<uint>& _ni,unsigned char _flags){
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
void MeshLoader::Detail::MeshLoadingManagement::_calculateTBN(ImportedMeshData& data){
    if(data.normals.size() == 0) return;
    for(uint i=0; i < data.points.size(); i+=3){
        glm::vec3 deltaPos1 = data.points[i + 1] - data.points[i + 0];
        glm::vec3 deltaPos2 = data.points[i + 2] - data.points[i + 0];

        glm::vec2 deltaUV1 = data.uvs[i + 1] - data.uvs[i + 0];
        glm::vec2 deltaUV2 = data.uvs[i + 2] - data.uvs[i + 0];

        float r = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);
        glm::vec3 tangent = (deltaPos1 * deltaUV2.y   - deltaPos2 * deltaUV1.y) * r;
        glm::vec3 bitangent = (deltaPos2 * deltaUV1.x   - deltaPos1 * deltaUV2.x) * r;

        glm::vec3 t1 = glm::normalize(tangent - data.normals[i + 0] * glm::dot(data.normals[i + 0], tangent));
        glm::vec3 t2 = glm::normalize(tangent - data.normals[i + 1] * glm::dot(data.normals[i + 1], tangent));
        glm::vec3 t3 = glm::normalize(tangent - data.normals[i + 2] * glm::dot(data.normals[i + 2], tangent));

        glm::vec3 b1 = glm::normalize(bitangent - data.normals[i + 0] * glm::dot(data.normals[i + 0], bitangent));
        glm::vec3 b2 = glm::normalize(bitangent - data.normals[i + 1] * glm::dot(data.normals[i + 1], bitangent));
        glm::vec3 b3 = glm::normalize(bitangent - data.normals[i + 2] * glm::dot(data.normals[i + 2], bitangent));

        data.tangents.push_back(t1); data.tangents.push_back(t2); data.tangents.push_back(t3);
        data.binormals.push_back(b1); data.binormals.push_back(b2); data.binormals.push_back(b3);
    }
    MeshLoader::Detail::MeshLoadingManagement::_calculateGramSchmidt(data.points,data.normals,data.binormals,data.tangents);
}

bool MeshLoader::Detail::MeshLoadingManagement::_getSimilarVertexIndex(glm::vec3& in_pos, glm::vec2& in_uv, glm::vec3& in_norm, std::vector<glm::vec3>& out_vertices,std::vector<glm::vec2>& out_uvs,std::vector<glm::vec3>& out_normals,ushort& result, float threshold){
    for (uint i=0; i < out_vertices.size(); i++ ){
        if (is_near( in_pos.x , out_vertices[i].x ,threshold) &&
            is_near( in_pos.y , out_vertices[i].y ,threshold) &&
            is_near( in_pos.z , out_vertices[i].z ,threshold) &&
            is_near( in_uv.x  , out_uvs[i].x      ,threshold) &&
            is_near( in_uv.y  , out_uvs[i].y      ,threshold) &&
            is_near( in_norm.x , out_normals[i].x ,threshold) &&
            is_near( in_norm.y , out_normals[i].y ,threshold) &&
            is_near( in_norm.z , out_normals[i].z ,threshold)
        ){
            result = i;
            return true;
        }
    }
    return false;
}
void MeshLoader::Detail::MeshLoadingManagement::_calculateGramSchmidt(std::vector<glm::vec3>& points,std::vector<glm::vec3>& normals,std::vector<glm::vec3>& binormals,std::vector<glm::vec3>& tangents){
    //this does something funky with mirrored uvs.
    for(uint i=0; i < points.size(); i++){
        glm::vec3& n = normals[i];
        glm::vec3& t = binormals[i];
        glm::vec3& b = tangents[i];
        // Gram-Schmidt orthogonalize
        t = glm::normalize(t - n * glm::dot(n, t));
    }
    /*
    //this is bad for mirrored uvs in most cases...
    for(uint i=0; i < points.size(); i++){
        // Calculate handedness
        glm::vec3& n = normals[i];
        glm::vec3& t = binormals[i];
        glm::vec3& b = tangents[i];
        if (glm::dot(glm::cross(n, t), b) < 0.0f){ 
            t *= -1.0f; 
        }
    }
    */
}
void MeshLoader::Detail::MeshLoadingManagement::_indexVBO(ImportedMeshData& data,std::vector<ushort> & out_indices,std::vector<glm::vec3>& out_pos, std::vector<glm::vec2>& out_uvs, std::vector<glm::vec3>& out_norm, std::vector<glm::vec3>& out_binorm,std::vector<glm::vec3>& out_tangents, float threshold){
    if(threshold == 0.0f){
        out_pos = data.points;
        out_norm = data.normals;
        out_uvs = data.uvs;
        out_binorm = data.binormals;
        out_tangents = data.tangents;
        out_indices = data.indices;
        return;
    }   
    for (uint i=0; i < data.points.size(); i++ ){
        ushort index;
        bool found = _getSimilarVertexIndex(data.points[i], data.uvs[i], data.normals[i],out_pos, out_uvs, out_norm, index,threshold);
        if ( found ){
            out_indices.push_back( index );

            //average out TBN. I think this does more harm than good though
            out_tangents[index] += data.tangents[i];
            out_binorm[index] += data.binormals[i];
        }
        else{
            out_pos.push_back( data.points[i]);
            out_uvs     .push_back(data.uvs[i]);
            out_norm .push_back(data.normals[i]);
            out_tangents .push_back(data.tangents[i]);
            out_binorm .push_back(data.binormals[i]);
            ushort newindex = (ushort)out_pos.size() - 1;
            out_indices .push_back(newindex);
        }
    }
}