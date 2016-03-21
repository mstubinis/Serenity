#include "Mesh.h"

#include <bullet/btBulletDynamicsCommon.h>

#include <boost/filesystem.hpp>
#include <boost/iostreams/device/mapped_file.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

#include "Engine_Physics.h"
#include "Engine_Resources.h"

Mesh::Mesh(int x, int y,int width, int height){
	m_Collision = nullptr;

	m_Points.push_back(glm::vec3(0,0,0));
	m_Points.push_back(glm::vec3(width,height,0));
	m_Points.push_back(glm::vec3(0,height,0));

	m_Points.push_back(glm::vec3(width,0,0));
	m_Points.push_back(glm::vec3(width,height,0));
	m_Points.push_back(glm::vec3(0,0,0));

	float uv_topLeft_x = float(x/256.0f);
	float uv_topLeft_y = float(y/256.0f);
	
	float uv_bottomLeft_x = float(x/256.0f);
	float uv_bottomLeft_y = float(y/256.0f) + float(height/256.0f);

	float uv_bottomRight_x = float(x/256.0f) + float(width/256.0f);
	float uv_bottomRight_y = float(y/256.0f) + float(height/256.0f);

	float uv_topRight_x = float(x/256.0f) + float(width/256.0f);
	float uv_topRight_y = float(y/256.0f);

	m_UVs.push_back(glm::vec2(uv_bottomLeft_x,uv_bottomLeft_y));
	m_UVs.push_back(glm::vec2(uv_topRight_x,uv_topRight_y));
	m_UVs.push_back(glm::vec2(uv_topLeft_x,uv_topLeft_y));

	m_UVs.push_back(glm::vec2(uv_bottomRight_x,uv_bottomRight_y));
	m_UVs.push_back(glm::vec2(uv_topRight_x,uv_topRight_y));
	m_UVs.push_back(glm::vec2(uv_bottomLeft_x,uv_bottomLeft_y));

	m_Normals.push_back(glm::vec3(1,1,1));
	m_Normals.push_back(glm::vec3(1,1,1));
	m_Normals.push_back(glm::vec3(1,1,1));
	m_Normals.push_back(glm::vec3(1,1,1));
	m_Normals.push_back(glm::vec3(1,1,1));
	m_Normals.push_back(glm::vec3(1,1,1));

	m_Tangents.push_back(glm::vec3(1,1,1));
	m_Tangents.push_back(glm::vec3(1,1,1));
	m_Tangents.push_back(glm::vec3(1,1,1));
	m_Tangents.push_back(glm::vec3(1,1,1));
	m_Tangents.push_back(glm::vec3(1,1,1));
	m_Tangents.push_back(glm::vec3(1,1,1));

	m_Binormals.push_back(glm::vec3(1,1,1));
	m_Binormals.push_back(glm::vec3(1,1,1));
	m_Binormals.push_back(glm::vec3(1,1,1));
	m_Binormals.push_back(glm::vec3(1,1,1));
	m_Binormals.push_back(glm::vec3(1,1,1));
	m_Binormals.push_back(glm::vec3(1,1,1));

	_init();
}
Mesh::Mesh(float width, float height){
	m_Collision = nullptr;

	m_Points.push_back(glm::vec3(-width/2.0f,-height/2.0f,0));
	m_Points.push_back(glm::vec3(width/2.0f,height/2.0f,0));
	m_Points.push_back(glm::vec3(-width/2.0f,height/2.0f,0));

	m_Points.push_back(glm::vec3(width/2.0f,-height/2.0f,0));
	m_Points.push_back(glm::vec3(width/2.0f,height/2.0f,0));
	m_Points.push_back(glm::vec3(-width/2.0f,-height/2.0f,0));

	float uv_topLeft_x = float(0);
	float uv_topLeft_y = float(0);
	
	float uv_bottomLeft_x = float(0);
	float uv_bottomLeft_y = float(0) + float(height);

	float uv_bottomRight_x = float(0) + float(width);
	float uv_bottomRight_y = float(0) + float(height);

	float uv_topRight_x = float(0) + float(width);
	float uv_topRight_y = float(0);

	m_UVs.push_back(glm::vec2(uv_bottomLeft_x,uv_bottomLeft_y));
	m_UVs.push_back(glm::vec2(uv_topRight_x,uv_topRight_y));
	m_UVs.push_back(glm::vec2(uv_topLeft_x,uv_topLeft_y));

	m_UVs.push_back(glm::vec2(uv_bottomRight_x,uv_bottomRight_y));
	m_UVs.push_back(glm::vec2(uv_topRight_x,uv_topRight_y));
	m_UVs.push_back(glm::vec2(uv_bottomLeft_x,uv_bottomLeft_y));


	m_Normals.push_back(glm::vec3(1,1,1));
	m_Normals.push_back(glm::vec3(1,1,1));
	m_Normals.push_back(glm::vec3(1,1,1));
	m_Normals.push_back(glm::vec3(1,1,1));
	m_Normals.push_back(glm::vec3(1,1,1));
	m_Normals.push_back(glm::vec3(1,1,1));

	m_Tangents.push_back(glm::vec3(1,1,1));
	m_Tangents.push_back(glm::vec3(1,1,1));
	m_Tangents.push_back(glm::vec3(1,1,1));
	m_Tangents.push_back(glm::vec3(1,1,1));
	m_Tangents.push_back(glm::vec3(1,1,1));
	m_Tangents.push_back(glm::vec3(1,1,1));

	m_Binormals.push_back(glm::vec3(1,1,1));
	m_Binormals.push_back(glm::vec3(1,1,1));
	m_Binormals.push_back(glm::vec3(1,1,1));
	m_Binormals.push_back(glm::vec3(1,1,1));
	m_Binormals.push_back(glm::vec3(1,1,1));
	m_Binormals.push_back(glm::vec3(1,1,1));

	_init();
}
Mesh::Mesh(std::string filename){
	m_Collision = nullptr;
	_loadFromFile(filename);
	_init();
}
Mesh::~Mesh(){
	for(unsigned int i = 0; i < NUM_VERTEX_DATA; i++) glDeleteBuffers(1, &m_buffers[i]);
}
void Mesh::_loadFromFile(std::string filename){
	std::string extention;
	for(unsigned int i = filename.length() - 4; i < filename.length(); i++)extention += tolower(filename.at(i));
	if(extention == ".obj")
		_loadFromOBJ(filename);
}
void Mesh::_loadFromOBJ(std::string filename){
	std::string colFile = filename.substr(0,filename.size()-4);
	colFile += "Col.obj";
	if(boost::filesystem::exists(colFile)){
		m_Collision = new MeshCollision(colFile,COLLISION_TYPE_CONVEXHULL);
	}
	else{
		m_Collision = new MeshCollision(filename,COLLISION_TYPE_CONVEXHULL);
	}

	std::vector<glm::vec3> pointData;
	std::vector<glm::vec2> uvData;
	std::vector<glm::vec3> normalData;
	std::vector<std::vector<glm::vec3>> listOfVerts;

	std::map<std::string,ObjectLoadingData> objects;

	std::string last = "";

	boost::iostreams::stream<boost::iostreams::mapped_file_source> str(filename);

	//first read in all vertex data
	int index = 1;
	for(std::string line; std::getline(str, line, '\n');){
		std::string x; std::string y; std::string z;
		unsigned int whilespaceCount = 0;
		unsigned int slashCount = 0;
		if(line[0] == 'o'){
			if(last != ""){
				ObjectLoadingData data;
				data.Faces = listOfVerts;
				data.Normals = normalData;
				data.Points = pointData;
				data.UVs = uvData;
				objects[last] = data;
				listOfVerts.clear();
			}
			last = line;
		}
		else if(line[0] == 'v'){ 
			for(auto c:line){
				if(c == ' ')                      whilespaceCount++;
				else{
					if(whilespaceCount == 1)      x += c;
					else if(whilespaceCount == 2) y += c;
					else if(whilespaceCount == 3) z += c;
				}
			}
			if(line[1] == ' ')//vertex point
				pointData.push_back(glm::vec3(static_cast<float>(::atof(x.c_str())),static_cast<float>(::atof(y.c_str())),static_cast<float>(::atof(z.c_str()))));
			else if(line[1] == 't')//vertex uv
				uvData.push_back(glm::vec2(static_cast<float>(::atof(x.c_str())),1-static_cast<float>(::atof(y.c_str()))));
			else if(line[1] == 'n')//vertex norm
				normalData.push_back(glm::vec3(static_cast<float>(::atof(x.c_str())),static_cast<float>(::atof(y.c_str())),static_cast<float>(::atof(z.c_str()))));
			index++;
		}
		//faces
		else if(line[0] == 'f' && line[1] == ' '){
			std::vector<glm::vec3> vertices;
			unsigned int count = 0;
			for(auto c:line){
				if(c == '/') {
					slashCount++;
				}
				else if(c == ' '){ 
					//global listOfVerts
					if(whilespaceCount != 0){
						glm::vec3 vertex = glm::vec3(static_cast<float>(::atof(x.c_str())),static_cast<float>(::atof(y.c_str())),static_cast<float>(::atof(z.c_str())));
						vertices.push_back(vertex);
						x = ""; y = ""; z = "";
						slashCount = 0;
					}
					whilespaceCount++;
				}
				else{
					if(whilespaceCount > 0){
						if(slashCount == 0)      x += c;
						else if(slashCount == 1) y += c;
						else if(slashCount == 2) z += c;
					}
				}
				count++;
			}
			glm::vec3 vertex = glm::vec3(static_cast<float>(::atof(x.c_str())),static_cast<float>(::atof(y.c_str())),static_cast<float>(::atof(z.c_str())));
			vertices.push_back(vertex);
			listOfVerts.push_back(vertices);
		}
	}
	ObjectLoadingData data;
	data.Faces = listOfVerts;
	data.Normals = normalData;
	data.Points = pointData;
	data.UVs = uvData;
	objects[last] = data;

	for(auto o:objects){
		for(auto face:o.second.Faces){
			Vertex v1,v2,v3,v4;
			v1.position = o.second.Points.at(static_cast<unsigned int>(face.at(0).x-1));
			v2.position = o.second.Points.at(static_cast<unsigned int>(face.at(1).x-1));
			v3.position = o.second.Points.at(static_cast<unsigned int>(face.at(2).x-1));
		
			if(o.second.UVs.size() > 0){
				v1.uv = o.second.UVs.at(static_cast<unsigned int>(face.at(0).y-1));
				v2.uv = o.second.UVs.at(static_cast<unsigned int>(face.at(1).y-1));
				v3.uv = o.second.UVs.at(static_cast<unsigned int>(face.at(2).y-1));
			}
			if(o.second.Normals.size() > 0){
				v1.normal = o.second.Normals.at(static_cast<unsigned int>(face.at(0).z-1));
				v2.normal = o.second.Normals.at(static_cast<unsigned int>(face.at(1).z-1));
				v3.normal = o.second.Normals.at(static_cast<unsigned int>(face.at(2).z-1));
			}
			if(face.size() == 4){//quad
				v4.position = o.second.Points.at(static_cast<unsigned int>(face.at(3).x-1));
				if(o.second.UVs.size() > 0)
					v4.uv = o.second.UVs.at(static_cast<unsigned int>(face.at(3).y-1));
				if(o.second.Normals.size() > 0)
					v4.normal = o.second.Normals.at(static_cast<unsigned int>(face.at(3).z-1));
				_generateQuad(v1,v2,v3,v4);
			}
			else{//triangle
				_generateTriangle(v1,v2,v3);
			}
		}
	}
}

void Mesh::_generateTriangle(Vertex& v1, Vertex& v2, Vertex& v3){
	m_Points.push_back(v1.position);
	m_UVs.push_back(v1.uv);
	m_Normals.push_back(v1.normal);

	m_Points.push_back(v2.position);
	m_UVs.push_back(v2.uv);
	m_Normals.push_back(v2.normal);

	m_Points.push_back(v3.position);
	m_UVs.push_back(v3.uv);
	m_Normals.push_back(v3.normal);

	_calculateTangent(v1,v2,v3);

	m_Tangents.push_back(v1.tangent);
	m_Tangents.push_back(v2.tangent);
	m_Tangents.push_back(v3.tangent);

	m_Binormals.push_back(v1.binormal);
	m_Binormals.push_back(v2.binormal);
	m_Binormals.push_back(v3.binormal);
}
void Mesh::_generateQuad(Vertex& v1, Vertex& v2, Vertex& v3, Vertex& v4){
	//well this was easy
	_generateTriangle(v1,v2,v3);
	_generateTriangle(v1,v3,v4);
}
void Mesh::_init(){
	//Bind the data to the buffers
	glGenBuffers((sizeof(m_buffers)/sizeof(m_buffers[0])), m_buffers);

	glBindBuffer(GL_ARRAY_BUFFER, m_buffers[0] );
	glBufferData(GL_ARRAY_BUFFER, m_Points.size() * sizeof(glm::vec3),&m_Points[0], GL_STATIC_DRAW );

	glBindBuffer(GL_ARRAY_BUFFER, m_buffers[1]);
	glBufferData(GL_ARRAY_BUFFER, m_UVs.size() * sizeof(glm::vec2), &m_UVs[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, m_buffers[2]);
	glBufferData(GL_ARRAY_BUFFER, m_Normals.size() * sizeof(glm::vec3), &m_Normals[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, m_buffers[3]);
	glBufferData(GL_ARRAY_BUFFER, m_Tangents.size() * sizeof(glm::vec3), &m_Tangents[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, m_buffers[4]);
	glBufferData(GL_ARRAY_BUFFER, m_Binormals.size() * sizeof(glm::vec3), &m_Binormals[0], GL_STATIC_DRAW);

	#pragma region Calculate Mesh Radius (x, y, and z)
	float maxX = 0;
	float maxY = 0;
	float maxZ = 0;
	for(auto point:m_Points){
		float x = abs(point.x);
		float y = abs(point.y);
		float z = abs(point.z);

		if(x > maxX)
			maxX = x;
		if(y > maxY)
			maxY = y;
		if(z > maxZ)
			maxZ = z;
	}
	m_radiusBox = glm::vec3(maxX,maxY,maxZ);
	m_radius = glm::max(maxX, glm::max(maxY,maxZ));
	#pragma endregion
}
void Mesh::render(GLuint mode){
	//for each unique vertex data type (position, color, uv, normal, tangent)...
	for(unsigned int i = 0; i < NUM_VERTEX_DATA; i++){
		glEnableVertexAttribArray(i);
		glBindBuffer( GL_ARRAY_BUFFER, m_buffers[i] );
		glVertexAttribPointer(i, VERTEX_AMOUNTS[i], GL_FLOAT, GL_FALSE, 0, 0);
	}
	glDrawArrays(mode, 0, m_Points.size());
	for(unsigned int i = 0; i < NUM_VERTEX_DATA; i++)
		glDisableVertexAttribArray(i);
}
void Mesh::_calculateTangent(Vertex& v1, Vertex& v2, Vertex& v3){
	glm::vec3 deltaPos1 = v2.position-v1.position;
	glm::vec3 deltaPos2 = v3.position-v1.position;
 
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


	//i dont even think this is needed.
	//this will need some fiddeling around with  ////////////////////
	//if (glm::dot(glm::cross(t1, b1), v1.normal) < 0.0f){t1 *= -1.0f;}
	//if (glm::dot(glm::cross(t2, b2), v2.normal) < 0.0f){t2 *= -1.0f;}
	//if (glm::dot(glm::cross(t3, b3), v3.normal) < 0.0f){t3 *= -1.0f;}
	/////////////////////////////////////////////////////////////////

	v1.tangent = t1;   v2.tangent = t2;   v3.tangent = t3;
	v1.binormal = -b1; v2.binormal = -b2; v3.binormal = -b3;
}