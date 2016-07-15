#include "Engine_MeshLoader.h"

#include <boost/filesystem.hpp>
#include <boost/iostreams/device/mapped_file.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

#include <map>

using namespace Engine::Resources;

void MeshLoader::loadObj(MeshData& ret,std::string filename,unsigned char _loadWhat){
    std::vector<glm::vec3> _p;
    std::vector<glm::vec2> _u;
    std::vector<glm::vec3> _n;
    std::vector<std::vector<glm::vec3>> _f;
    std::map<std::string,ObjectLoadingData> objects;
    std::string last = "";
    boost::iostreams::stream<boost::iostreams::mapped_file_source> str(filename);

    //first read in all vertex data
    for(std::string line; std::getline(str, line, '\n');){
        if(line[0] == 'o'){
            if(last != ""){
                ObjectLoadingData data;
                data.Faces = _f;
				data.Normals = _n;
				data.Points = _p;
				data.UVs = _u;
                objects[last] = data;
                _f.clear();
            }
            last = line;
        }	
		else if(line[0] == 'v' && line[1] == ' '){ 
			if(_loadWhat && LOAD_POINTS){
				glm::vec3 position;
				sscanf(line.substr(2,line.size()).c_str(),"%f %f %f",&position.x,&position.y,&position.z);
				_p.push_back(position);
			}
		}
		else if(line[0] == 'v' && line[1] == 't'){
			if(_loadWhat && LOAD_UVS){
				glm::vec2 uv;
				sscanf(line.substr(2,line.size()).c_str(),"%f %f",&uv.x,&uv.y);
				uv.y = 1.0f - uv.y;
				_u.push_back(uv);
			}
		}
		else if(line[0] == 'v' && line[1] == 'n'){
			if(_loadWhat && LOAD_NORMALS){
				glm::vec3 normal;
				sscanf(line.substr(2,line.size()).c_str(),"%f %f %f",&normal.x,&normal.y,&normal.z);
				_n.push_back(normal);
			}
		}
        //faces
        else if(line[0] == 'f' && line[1] == ' '){
			if(_loadWhat && LOAD_FACES){
				std::vector<glm::vec3> vertices;
				glm::uvec3 f1,f2,f3;
				std::string l = line.substr(2,line.size());
				sscanf(l.c_str(),"%d/%d/%d %d/%d/%d %d/%d/%d",&f1.x,&f1.y,&f1.z,&f2.x,&f2.y,&f2.z,&f3.x,&f3.y,&f3.z);
				if(f2.x == 0 && f2.y == 0 && f2.z == 0 && f3.x == 0 && f3.y == 0 && f3.z == 0){
					sscanf(l.c_str(),"%d %d %d",&f1.x,&f2.x,&f3.x);
					if(f1.x <= 0) f1.x = 1; if(f2.x <= 0) f2.x = 1; if(f3.x <= 0) f3.x = 1;
					f1.y = 1; f1.z = 1; f2.y = 1; f2.z = 1; f3.y = 1; f3.z = 1;
					vertices.push_back(glm::vec3(f1));
					vertices.push_back(glm::vec3(f2));
					vertices.push_back(glm::vec3(f3));
				}
				else{
					vertices.push_back(glm::vec3(f1));
					vertices.push_back(glm::vec3(f2));
					vertices.push_back(glm::vec3(f3));
				}
				_f.push_back(vertices);
			}
        }
    }
    ObjectLoadingData data;
    data.Faces = _f;
	data.Normals = _n;
	data.Points = _p;
	data.UVs = _u;
    objects[last] = data;

	if(_loadWhat && LOAD_POINTS)  ret.file_points = _p;
	if(_loadWhat && LOAD_UVS)     ret.file_uvs = _u;
	if(_loadWhat && LOAD_NORMALS) ret.file_normals = _n;
	if(_loadWhat && LOAD_FACES)   ret.file_faces = _f;

    for(auto o:objects){
        for(auto face:o.second.Faces){
            Vertex v1,v2,v3,v4;
            v1.position = o.second.Points.at(uint(face.at(0).x-1));
            v2.position = o.second.Points.at(uint(face.at(1).x-1));
            v3.position = o.second.Points.at(uint(face.at(2).x-1));
        
            if(o.second.UVs.size() > 0){
                v1.uv = o.second.UVs.at(uint(face.at(0).y-1));
                v2.uv = o.second.UVs.at(uint(face.at(1).y-1));
                v3.uv = o.second.UVs.at(uint(face.at(2).y-1));
            }
            if(o.second.Normals.size() > 0){
                v1.normal = o.second.Normals.at(uint(face.at(0).z-1));
                v2.normal = o.second.Normals.at(uint(face.at(1).z-1));
                v3.normal = o.second.Normals.at(uint(face.at(2).z-1));
            }
            if(face.size() == 4){//quad
                v4.position = o.second.Points.at(uint(face.at(3).x-1));
                if(o.second.UVs.size() > 0)
                    v4.uv = o.second.UVs.at(uint(face.at(3).y-1));
                if(o.second.Normals.size() > 0)
                    v4.normal = o.second.Normals.at(uint(face.at(3).z-1));
                _generateQuad(ret,v1,v2,v3,v4);
            }
            else{//triangle
                _generateTriangle(ret,v1,v2,v3);
            }
        }
    }
}
void MeshLoader::loadObjFromMemory(MeshData& ret,std::string data,unsigned char _loadWhat){
    std::vector<glm::vec3> _p;
    std::vector<glm::vec2> _u;
    std::vector<glm::vec3> _n;
    std::vector<std::vector<glm::vec3>> _f;
    std::map<std::string,ObjectLoadingData> objects;
    std::string last = "";

    std::istringstream input;  input.str(data);

    //first read in all vertex data
    for(std::string line; std::getline(input, line, '\n');){
        if(line[0] == 'o'){
            if(last != ""){
                ObjectLoadingData data;
                data.Faces = _f;
				data.Normals = _n;
				data.Points = _p;
				data.UVs = _u;
                objects[last] = data;
                _f.clear();
            }
            last = line;
        }
        else if(line[0] == 'v' && line[1] == ' '){ 
			if(_loadWhat && LOAD_POINTS){
				glm::vec3 position;
				sscanf(line.substr(2,line.size()).c_str(),"%f %f %f",&position.x,&position.y,&position.z);
				_p.push_back(position);
			}
        }
		else if(line[0] == 'v' && line[1] == 't'){
			if(_loadWhat && LOAD_UVS){
				glm::vec2 uv;
				sscanf(line.substr(2,line.size()).c_str(),"%f %f",&uv.x,&uv.y);
				uv.y = 1.0f - uv.y;
				_u.push_back(uv);
			}
		}
		else if(line[0] == 'v' && line[1] == 'n'){
			if(_loadWhat && LOAD_NORMALS){
				glm::vec3 normal;
				sscanf(line.substr(2,line.size()).c_str(),"%f %f %f",&normal.x,&normal.y,&normal.z);
				_n.push_back(normal);
			}
		}
        //faces
        else if(line[0] == 'f' && line[1] == ' '){
			if(_loadWhat && LOAD_FACES){
				std::vector<glm::vec3> vertices;
				glm::uvec3 f1,f2,f3;
				std::string l = line.substr(2,line.size());
				sscanf(l.c_str(),"%d/%d/%d %d/%d/%d %d/%d/%d",&f1.x,&f1.y,&f1.z,&f2.x,&f2.y,&f2.z,&f3.x,&f3.y,&f3.z);
				if(f2.x == 0 && f2.y == 0 && f2.z == 0 && f3.x == 0 && f3.y == 0 && f3.z == 0){
					sscanf(l.c_str(),"%d %d %d",&f1.x,&f2.x,&f3.x);
					if(f1.x <= 0) f1.x = 1; if(f2.x <= 0) f2.x = 1; if(f3.x <= 0) f3.x = 1;
					f1.y = 1; f1.z = 1; f2.y = 1; f2.z = 1; f3.y = 1; f3.z = 1;
					vertices.push_back(glm::vec3(f1));
					vertices.push_back(glm::vec3(f2));
					vertices.push_back(glm::vec3(f3));
				}
				else{
					vertices.push_back(glm::vec3(f1));
					vertices.push_back(glm::vec3(f2));
					vertices.push_back(glm::vec3(f3));
				}
				_f.push_back(vertices);
			}
        }
    }
    ObjectLoadingData da;
    da.Faces = _f;
	da.Normals = _n;
	da.Points = _p;
	da.UVs = _u;
    objects[last] = da;

	if(_loadWhat && LOAD_POINTS)  ret.file_points = _p;
	if(_loadWhat && LOAD_UVS)     ret.file_uvs = _u;
	if(_loadWhat && LOAD_NORMALS) ret.file_normals = _n;
	if(_loadWhat && LOAD_FACES)   ret.file_faces = _f;

    for(auto o:objects){
        for(auto face:o.second.Faces){
            Vertex v1,v2,v3,v4;
            v1.position = o.second.Points.at(uint(face.at(0).x-1));
            v2.position = o.second.Points.at(uint(face.at(1).x-1));
            v3.position = o.second.Points.at(uint(face.at(2).x-1));
        
            if(o.second.UVs.size() > 0){
                v1.uv = o.second.UVs.at(uint(face.at(0).y-1));
                v2.uv = o.second.UVs.at(uint(face.at(1).y-1));
                v3.uv = o.second.UVs.at(uint(face.at(2).y-1));
            }
            if(o.second.Normals.size() > 0){
                v1.normal = o.second.Normals.at(uint(face.at(0).z-1));
                v2.normal = o.second.Normals.at(uint(face.at(1).z-1));
                v3.normal = o.second.Normals.at(uint(face.at(2).z-1));
            }
            if(face.size() == 4){//quad
                v4.position = o.second.Points.at(uint(face.at(3).x-1));
                if(o.second.UVs.size() > 0)
                    v4.uv = o.second.UVs.at(uint(face.at(3).y-1));
                if(o.second.Normals.size() > 0)
                    v4.normal = o.second.Normals.at(uint(face.at(3).z-1));
                _generateQuad(ret,v1,v2,v3,v4);
            }
            else{//triangle
                _generateTriangle(ret,v1,v2,v3);
            }
        }
    }
}
void MeshLoader::_calculateTangent(Vertex& v1, Vertex& v2, Vertex& v3){
    glm::vec3 deltaPos1 = v2.position - v1.position;
    glm::vec3 deltaPos2 = v3.position - v1.position;
 
    glm::vec2 deltaUV1 = v2.uv-v1.uv;
    glm::vec2 deltaUV2 = v3.uv-v1.uv;
    float r = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);
    glm::vec3 tangent = (deltaPos1 * deltaUV2.y   - deltaPos2 * deltaUV1.y)*r;
    glm::vec3 bitangent = (deltaPos2 * deltaUV1.x   - deltaPos1 * deltaUV2.x)*r;

    glm::vec3 t1 = glm::normalize(tangent - v1.normal * glm::dot(v1.normal, tangent));
    glm::vec3 t2 = glm::normalize(tangent - v2.normal * glm::dot(v2.normal, tangent));
    glm::vec3 t3 = glm::normalize(tangent - v3.normal * glm::dot(v3.normal, tangent));

    glm::vec3 b1 = glm::normalize(bitangent - v1.normal * glm::dot(v1.normal, bitangent));
    glm::vec3 b2 = glm::normalize(bitangent - v2.normal * glm::dot(v2.normal, bitangent));
    glm::vec3 b3 = glm::normalize(bitangent - v3.normal * glm::dot(v3.normal, bitangent));

    v1.tangent = t1;   v2.tangent = t2;   v3.tangent = t3;
    v1.binormal = b1; v2.binormal = b2; v3.binormal = b3;
}
void MeshLoader::_generateTriangle(MeshData& data,Vertex& v1, Vertex& v2, Vertex& v3){
    data.points.push_back(v1.position);
    data.uvs.push_back(v1.uv);
	data.normals.push_back(v1.normal);

	data.points.push_back(v2.position);
	data.uvs.push_back(v2.uv);
	data.normals.push_back(v2.normal);

	data.points.push_back(v3.position);
	data.uvs.push_back(v3.uv);
	data.normals.push_back(v3.normal);

    _calculateTangent(v1,v2,v3);

    data.tangents.push_back(v1.tangent);
    data.tangents.push_back(v2.tangent);
    data.tangents.push_back(v3.tangent);

	data.binormals.push_back(v1.binormal);
	data.binormals.push_back(v2.binormal);
	data.binormals.push_back(v3.binormal);
}
void MeshLoader::_generateQuad(MeshData& data,Vertex& v1, Vertex& v2, Vertex& v3, Vertex& v4){
    _generateTriangle(data,v1,v2,v3);
    _generateTriangle(data,v2,v4,v3);
}