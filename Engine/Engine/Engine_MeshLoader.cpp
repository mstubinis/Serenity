#include "Engine_MeshLoader.h"

#include <boost/filesystem.hpp>
#include <boost/iostreams/device/mapped_file.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <map>

using namespace Engine::Resources;

bool is_near(float v1, float v2, float threshold){ return fabs( v1-v2 ) < threshold; }

void MeshLoader::Detail::_OBJ::_loadObjDataFromLine(std::string& l, std::vector<glm::vec3>& _p, std::vector<glm::vec2>& _u, std::vector<glm::vec3>& _n, std::vector<uint>& _pi, std::vector<uint>& _ui, std::vector<uint>& _ni, const char _f){
	if(l[0] == 'o'){
    }
	//vertex positions
    else if(l[0] == 'v' && l[1] == ' '){ 
        if(_f && LOAD_POINTS){
            glm::vec3 p;
			sscanf(l.substr(2,l.size()).c_str(),"%f %f %f",&p.x,&p.y,&p.z);
            _p.push_back(p);
        }
    }
	//vertex uvs
    else if(l[0] == 'v' && l[1] == 't'){
        if(_f && LOAD_UVS){
            glm::vec2 uv;
            sscanf(l.substr(2,l.size()).c_str(),"%f %f",&uv.x,&uv.y);
            uv.y = 1.0f - uv.y;
            _u.push_back(uv);
        }
    }
	//vertex normals
    else if(l[0] == 'v' && l[1] == 'n'){
        if(_f && LOAD_NORMALS){
            glm::vec3 n;
            sscanf(l.substr(2,l.size()).c_str(),"%f %f %f",&n.x,&n.y,&n.z);
            _n.push_back(n);
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

void MeshLoader::loadObj(ImportedMeshData& data,std::string filename,unsigned char _flags){
    std::vector<glm::vec3> _p;
	std::vector<glm::vec2> _u;
	std::vector<glm::vec3> _n;

    boost::iostreams::stream<boost::iostreams::mapped_file_source> str(filename);

    std::vector<uint> positionIndices;
    std::vector<uint> uvIndices;
    std::vector<uint> normalIndices;

    //first read in all data
    for(std::string line; std::getline(str, line, '\n');){
        MeshLoader::Detail::_OBJ::_loadObjDataFromLine(line,_p,_u,_n,positionIndices,uvIndices,normalIndices,_flags);
    }
    uint max = _p.size(); if(_n.size() > max) max = _n.size(); if(_u.size() > max) max = _u.size();
    if(_u.size() == 0) _u.resize(max); if(_n.size() == 0) _n.resize(max);

    if(_flags && LOAD_POINTS)  data.file_points = _p;
    if(_flags && LOAD_UVS)     data.file_uvs = _u;
    if(_flags && LOAD_NORMALS) data.file_normals = _n;
    if(_flags && LOAD_FACES){
        MeshLoader::Detail::_loadDataIntoTriangles(data,_p,_u,_n,positionIndices,uvIndices,normalIndices,_flags);
    }
    if(_flags && LOAD_TBN && data.normals.size() > 0){
        MeshLoader::Detail::_calculateTBN(data);
    }
}
void MeshLoader::loadObjFromMemory(ImportedMeshData& data,std::string input,unsigned char _flags){
    std::vector<glm::vec3> _p;
	std::vector<glm::vec2> _u;
	std::vector<glm::vec3> _n;

    std::vector<uint> positionIndices;
	std::vector<uint> uvIndices;
	std::vector<uint> normalIndices;

    std::istringstream stream;
	stream.str(input);

    //first read in all data
    for(std::string line; std::getline(stream, line, '\n');){
        MeshLoader::Detail::_OBJ::_loadObjDataFromLine(line,_p,_u,_n,positionIndices,uvIndices,normalIndices,_flags);
    }
    uint max = _p.size(); if(_n.size() > max) max = _n.size(); if(_u.size() > max) max = _u.size();
    if(_u.size() == 0) _u.resize(max); if(_n.size() == 0) _n.resize(max);

    if(_flags && LOAD_POINTS)  data.file_points = _p;
    if(_flags && LOAD_UVS)     data.file_uvs = _u;
    if(_flags && LOAD_NORMALS) data.file_normals = _n;
    if(_flags && LOAD_FACES){
        MeshLoader::Detail::_loadDataIntoTriangles(data,_p,_u,_n,positionIndices,uvIndices,normalIndices,_flags);
    }
    if(_flags && LOAD_TBN && data.normals.size() > 0){
        MeshLoader::Detail::_calculateTBN(data);
    }
}
void MeshLoader::Detail::_loadDataIntoTriangles(ImportedMeshData& data,std::vector<glm::vec3>& _p, std::vector<glm::vec2>& _u,std::vector<glm::vec3>& _n,std::vector<uint>& _pi, std::vector<uint>& _ui,std::vector<uint>& _ni,unsigned char _flags){
    uint count = 0;
	Triangle triangle;
    for(uint i=0; i < _pi.size(); i++ ){
        glm::vec3 pos  = _p[_pi[i]-1];
        glm::vec2 uv   = _u[_ui[i]-1];
        glm::vec3 norm = _n[_ni[i]-1];
        if(_flags && LOAD_POINTS)  data.points .push_back(pos);
        if(_flags && LOAD_UVS)     data.uvs    .push_back(uv);
        if(_flags && LOAD_NORMALS) data.normals.push_back(norm);
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
void MeshLoader::Detail::_calculateTBN(ImportedMeshData& data){
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
    MeshLoader::Detail::_calculateGramSchmidt(data.points,data.normals,data.binormals,data.tangents);
}

bool MeshLoader::Detail::_getSimilarVertexIndex(glm::vec3& in_pos, glm::vec2& in_uv, glm::vec3& in_norm, std::vector<glm::vec3>& out_vertices,std::vector<glm::vec2>& out_uvs,std::vector<glm::vec3>& out_normals,ushort& result, float threshold){
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
void MeshLoader::Detail::_calculateGramSchmidt(std::vector<glm::vec3>& points,std::vector<glm::vec3>& normals,std::vector<glm::vec3>& binormals,std::vector<glm::vec3>& tangents){
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
void MeshLoader::Detail::_indexVBO(ImportedMeshData& data,std::vector<ushort> & out_indices,std::vector<glm::vec3>& out_pos, std::vector<glm::vec2>& out_uvs, std::vector<glm::vec3>& out_norm, std::vector<glm::vec3>& out_binorm,std::vector<glm::vec3>& out_tangents, float threshold){
    if(threshold == 0.0f){
        out_pos = data.points;
        out_norm = data.normals;
        out_uvs = data.uvs;
        out_binorm = data.binormals;
        out_tangents = data.tangents;
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