#include "Mesh.h"

#include <bullet/btBulletDynamicsCommon.h>

#include <boost/filesystem.hpp>
#include <boost/iostreams/device/mapped_file.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

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

	_init();
}
Mesh::Mesh(std::string filename){
	_loadFromFile(filename);
	_init();
}
void Mesh::_loadFromFile(std::string filename){
	m_Collision = nullptr;
	assert(filename.length() >= 4);
	std::string extention;
	for(unsigned int i = filename.length() - 4; i < filename.length(); i++)
		extention += tolower(filename.at(i));
	if(extention == ".obj")
		_loadFromOBJ(filename);
	if(extention == ".ply")
		_loadFromPLY(filename);
}
void Mesh::_loadFromPLY(std::string filename){
	std::string colFile = filename.substr(0,filename.size()-4);
	colFile += "Col.ply";
	if(boost::filesystem::exists(colFile)){
		m_Collision = Mesh::_loadColFromPLY(colFile);
	}
	else{
		m_Collision = Mesh::_loadColFromPLY(filename);
	}

	bool StartReadingGeometryData = false;
	boost::iostreams::stream<boost::iostreams::mapped_file_source> str(filename);
	std::vector<Vertex> vertices;
	for(std::string line; std::getline(str, line, '\n');){
		if(StartReadingGeometryData == true){
			int SlashCount = 0;
			int whitespaceCount = 0;
			int SpaceCount = 0;

			std::string x; std::string y; std::string z; std::string w;
			std::string u; std::string v;
			std::string n1; std::string n2;std::string n3;

			Vertex vert;
			#pragma region Vertex
			if(line[1] != ' '){
				for(unsigned int i = 0; i < line.length(); i++){
					if(line.at(i) == ' ')    SpaceCount++;
					if(SpaceCount == 0)      x += line.at(i);
					else if(SpaceCount == 1) y += line.at(i);
					else if(SpaceCount == 2) z += line.at(i);
					else if(SpaceCount == 3) n1 += line.at(i);
					else if(SpaceCount == 4) n2 += line.at(i);
					else if(SpaceCount == 5) n3 += line.at(i);
					else if(SpaceCount == 6) u += line.at(i);
					else if(SpaceCount == 7) v += line.at(i);
				}
				vert.position = glm::vec3(static_cast<float>(::atof(x.c_str())),static_cast<float>(::atof(y.c_str())),static_cast<float>(::atof(z.c_str())));
				vert.uv = glm::vec2(static_cast<float>(::atof(u.c_str())),1-static_cast<float>(::atof(v.c_str())));
				vert.normal = glm::vec3( static_cast<float>(::atof(n1.c_str())),static_cast<float>(::atof(n2.c_str())),static_cast<float>(::atof(n3.c_str())));
				vertices.push_back(vert);
			}
			#pragma endregion
			#pragma region Indices
			if((line[0] == '3' || line[0] == '4') && line[1] == ' '){
				for(unsigned int i = 0; i < line.length(); i++){
					if(line.at(i) == ' '){ 
						SpaceCount++;
						whitespaceCount++;
					}
					if(SpaceCount == 1) x += line.at(i);
					else if(SpaceCount == 2) y += line.at(i);
					else if(SpaceCount == 3) z += line.at(i);
					else if(SpaceCount == 4) w += line.at(i);
				}
				int index1 = static_cast<int>(::atof(x.c_str()));
				int index2 = static_cast<int>(::atof(y.c_str()));
				int index3 = static_cast<int>(::atof(z.c_str()));
				int index4 = static_cast<int>(::atof(w.c_str()));
				if(whitespaceCount == 4)
					_generateQuad(vertices[index1],vertices[index2],vertices[index3],vertices[index4]);
				else
					_generateTriangle(vertices[index1],vertices[index2],vertices[index3]);
			}
			#pragma endregion
		}
		if(line == "end_header")
			StartReadingGeometryData = true;
	}
}
btConvexHullShape* Mesh::_loadColFromPLY(std::string filename){
	bool StartReadingGeometryData = false;
	boost::iostreams::stream<boost::iostreams::mapped_file_source> str(filename);
	std::vector<Vertex> vertices;
	btConvexHullShape* collision = new btConvexHullShape();
	for(std::string line; std::getline(str, line, '\n');){
		if(StartReadingGeometryData == true){
			int SlashCount = 0;
			int whitespaceCount = 0;
			int SpaceCount = 0;

			std::string x; std::string y; std::string z;
			Vertex vert;
			#pragma region Vertex
			if(line[1] != ' '){
				for(unsigned int i = 0; i < line.length(); i++){
					if(line.at(i) == ' ')    SpaceCount++;
					if(SpaceCount == 0)      x += line.at(i);
					else if(SpaceCount == 1) y += line.at(i);
					else if(SpaceCount == 2) z += line.at(i);
				}
				vert.position = glm::vec3(static_cast<float>(::atof(x.c_str())),static_cast<float>(::atof(y.c_str())),static_cast<float>(::atof(z.c_str())));
				vertices.push_back(vert);

				btVector3 pos = btVector3(vert.position.x,vert.position.y,vert.position.z);
				collision->addPoint(pos);
			}
			#pragma endregion
		}
		if(line == "end_header")
			StartReadingGeometryData = true;
	}
	return collision;
}
void Mesh::_loadFromOBJ(std::string filename){
	std::string colFile = filename.substr(0,filename.size()-4);
	colFile += "Col.obj";
	if(boost::filesystem::exists(colFile)){
		m_Collision = Mesh::_loadColFromOBJ(colFile);
	}
	else{
		m_Collision = Mesh::_loadColFromOBJ(filename);
	}

	std::vector<glm::vec3> pointData;
	std::vector<glm::vec2> uvData;
	std::vector<glm::vec3> normalData;

	std::vector<std::vector<glm::vec3>> listOfVerts;

	boost::iostreams::stream<boost::iostreams::mapped_file_source> str(filename);

	int index = 1;
	for(std::string line; std::getline(str, line, '\n');){
		std::string x; std::string y; std::string z;
		unsigned int whilespaceCount = 0;
		unsigned int slashCount = 0;
		if(line[0] == 'v'){ 
			for(auto c:line){
				if(c == ' ')                      whilespaceCount++;
				else{
					if(whilespaceCount == 1)      x += c;
					else if(whilespaceCount == 2) y += c;
					else if(whilespaceCount == 3) z += c;
				}
			}
			if(line[1] == ' '){//vertex point
				float x1 = static_cast<float>(::atof(x.c_str()));
				float y1 = static_cast<float>(::atof(y.c_str()));
				float z1 = static_cast<float>(::atof(z.c_str()));
				pointData.push_back(glm::vec3(x1,y1,z1));
			}
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
	for(auto face:listOfVerts){
		Vertex v1,v2,v3,v4;

		v1.position = pointData.at(static_cast<unsigned int>(face.at(0).x-1));
		v2.position = pointData.at(static_cast<unsigned int>(face.at(1).x-1));
		v3.position = pointData.at(static_cast<unsigned int>(face.at(2).x-1));
		
		if(uvData.size() > 0){
			v1.uv = uvData.at(static_cast<unsigned int>(face.at(0).y-1));
			v2.uv = uvData.at(static_cast<unsigned int>(face.at(1).y-1));
			v3.uv = uvData.at(static_cast<unsigned int>(face.at(2).y-1));
		}
		if(normalData.size() > 0){
			v1.normal = normalData.at(static_cast<unsigned int>(face.at(0).z-1));
			v2.normal = normalData.at(static_cast<unsigned int>(face.at(1).z-1));
			v3.normal = normalData.at(static_cast<unsigned int>(face.at(2).z-1));
		}
		if(face.size() == 4){//quad
			v4.position = pointData.at(static_cast<unsigned int>(face.at(3).x-1));
			if(uvData.size() > 0)
				v4.uv = uvData.at(static_cast<unsigned int>(face.at(3).y-1));
			if(normalData.size() > 0)
				v4.normal = normalData.at(static_cast<unsigned int>(face.at(3).z-1));
			_generateQuad(v1,v2,v3,v4);
		}
		else{//triangle
			_generateTriangle(v1,v2,v3);
		}
	}
}
btConvexHullShape* Mesh::_loadColFromOBJ(std::string filename){
	btConvexHullShape* collision = new btConvexHullShape();
	boost::iostreams::stream<boost::iostreams::mapped_file_source> str(filename);

	for(std::string line; std::getline(str, line, '\n');){
		std::string x; std::string y; std::string z;
		unsigned int whilespaceCount = 0;
		if(line[0] == 'v' && line[1] == ' '){
			for(auto c:line){
				if(c == ' ')                      whilespaceCount++;
				else{
					if(whilespaceCount == 1)      x += c;
					else if(whilespaceCount == 2) y += c;
					else if(whilespaceCount == 3) z += c;
				}
			}
			float x1 = static_cast<float>(::atof(x.c_str()));
			float y1 = static_cast<float>(::atof(y.c_str()));
			float z1 = static_cast<float>(::atof(z.c_str()));

			btVector3 pos = btVector3(x1,y1,z1);
			collision->addPoint(pos);
		}
		else if(line[0] == 'v' && line[1] != ' ') //we are done loading in the points, break out of the method
			return collision;
	}
	return collision;
}
Mesh::~Mesh(){
	for(unsigned int i = 0; i < NUM_VERTEX_DATA; i++)
		glDeleteBuffers(1, &m_buffers[i]);
	delete m_Collision;
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
}
void Mesh::_generateQuad(Vertex& v1, Vertex& v2, Vertex& v3, Vertex& v4){
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
void Mesh::render(){
	//for each unique vertex data type (position, color, uv, normal, tangent)...
	for(unsigned int i = 0; i < NUM_VERTEX_DATA; i++){
		glBindBuffer( GL_ARRAY_BUFFER, m_buffers[i] );
		glEnableVertexAttribArray(i);
		glVertexAttribPointer(i, VERTEX_AMOUNTS[i], GL_FLOAT, GL_FALSE, 0, 0);
	}
	glDrawArrays(GL_TRIANGLES, 0, m_Points.size());
	for(unsigned int i = 0; i < NUM_VERTEX_DATA; i++)
		glDisableVertexAttribArray(i);
}
void Mesh::_calculateTangent(Vertex& v1, Vertex& v2, Vertex& v3){
	glm::vec3 tangent;
	float vector1[3], vector2[3];
	float tuVector[2], tvVector[2];
	float den;

	// Calculate the two vectors for this face.
	vector1[0] = v2.position.x - v1.position.x;
	vector1[1] = v2.position.y - v1.position.y;
	vector1[2] = v2.position.z - v1.position.z;

	vector2[0] = v3.position.x - v1.position.x;
	vector2[1] = v3.position.y - v1.position.y;
	vector2[2] = v3.position.z - v1.position.z;

	// Calculate the tu and tv texture space vectors.
	tuVector[0] = v2.uv.x - v1.uv.x;
	tvVector[0] = v2.uv.y - v1.uv.y;

	tuVector[1] = v3.uv.x - v1.uv.x;
	tvVector[1] = v3.uv.y - v1.uv.y;

	// Calculate the denominator of the tangent/binormal equation.
	den = 1.0f / (tuVector[0] * tvVector[1] - tuVector[1] * tvVector[0]);

	// Calculate the cross products and multiply by the coefficient to get the tangent and binormal.

	tangent.x = (tvVector[1] * vector1[0] - tvVector[0] * vector2[0]) * den;
	tangent.y = (tvVector[1] * vector1[1] - tvVector[0] * vector2[1]) * den;
	tangent.z = (tvVector[1] * vector1[2] - tvVector[0] * vector2[2]) * den;

	glm::vec3 t1 = glm::normalize(tangent - v1.normal * glm::dot(v1.normal, tangent));
	glm::vec3 t2 = glm::normalize(tangent - v2.normal * glm::dot(v2.normal, tangent));
	glm::vec3 t3 = glm::normalize(tangent - v3.normal * glm::dot(v3.normal, tangent));

	v1.tangent = t1;   v2.tangent = t2;   v3.tangent = t3;
}