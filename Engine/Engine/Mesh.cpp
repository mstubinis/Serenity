#include "Mesh.h"
#include "Engine_Resources.h"

#include <iostream>
#include <fstream>

Mesh::Mesh(int x, int y,int width, int height){
	m_Points.push_back(glm::vec3(0,0,0));
	m_Points.push_back(glm::vec3(width,height,0));
	m_Points.push_back(glm::vec3(0,height,0));

	m_Points.push_back(glm::vec3(width,0,0));
	m_Points.push_back(glm::vec3(width,height,0));
	m_Points.push_back(glm::vec3(0,0,0));

	m_Colors.push_back(glm::vec4(1,1,1,1));
	m_Colors.push_back(glm::vec4(1,1,1,1));
	m_Colors.push_back(glm::vec4(1,1,1,1));
	m_Colors.push_back(glm::vec4(1,1,1,1));
	m_Colors.push_back(glm::vec4(1,1,1,1));
	m_Colors.push_back(glm::vec4(1,1,1,1));

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

	m_Binormals.push_back(glm::vec3(1,1,1));
	m_Binormals.push_back(glm::vec3(1,1,1));
	m_Binormals.push_back(glm::vec3(1,1,1));
	m_Binormals.push_back(glm::vec3(1,1,1));
	m_Binormals.push_back(glm::vec3(1,1,1));
	m_Binormals.push_back(glm::vec3(1,1,1));

	m_Tangents.push_back(glm::vec3(1,1,1));
	m_Tangents.push_back(glm::vec3(1,1,1));
	m_Tangents.push_back(glm::vec3(1,1,1));
	m_Tangents.push_back(glm::vec3(1,1,1));
	m_Tangents.push_back(glm::vec3(1,1,1));
	m_Tangents.push_back(glm::vec3(1,1,1));

	Init();
}
Mesh::Mesh(std::string filename){
	LoadFromFile(filename);
	Init();
}
void Mesh::LoadFromFile(std::string filename){
	assert(filename.length() >= 4);
	std::string extention;
	for(unsigned int i = filename.length() - 4; i < filename.length(); i++)
		extention += tolower(filename.at(i));
	//if(extention == ".obj")
		//LoadFromOBJ(filename);
	if(extention == ".ply")
		LoadFromPLY(filename);
}
void Mesh::LoadFromPLY(std::string filename){
	std::ifstream reader(filename);
	std::string line;

	std::vector<glm::vec3> pointData;
	std::vector<glm::vec4> colorData;
	std::vector<glm::vec2> uvData;
	std::vector<glm::vec3> normalData;

	bool StartReadingGeometryData = false;
	#pragma region FileReading
	if (reader.is_open()){
		while (std::getline(reader, line)){
			if(StartReadingGeometryData == true){
				int SlashCount = 0;
				int whitespaceCount = 0;
				int SpaceCount = 0;

				std::string x; std::string y; std::string z; std::string w;
				std::string uv1; std::string uv2;std::string uv3; std::string uv4;
				std::string n1; std::string n2;std::string n3; std::string n4;

				#pragma region Vertex
				if(line[1] != ' '){
					for(unsigned int i = 0; i < line.length(); i++){
						if(line.at(i) == ' ')
							SpaceCount++;
						if(SpaceCount == 0)
							x += line.at(i);
						else if(SpaceCount == 1)
							y += line.at(i);
						else if(SpaceCount == 2)
							z += line.at(i);
						else if(SpaceCount == 3)
							n1 += line.at(i);
						else if(SpaceCount == 4)
							n2 += line.at(i);
						else if(SpaceCount == 5)
							n3 += line.at(i);
						else if(SpaceCount == 6)
							uv1 += line.at(i);
						else if(SpaceCount == 7)
							uv2 += line.at(i);
					}
					float xPos = static_cast<float>(::atof(x.c_str()));
					float yPos = static_cast<float>(::atof(y.c_str()));
					float zPos = static_cast<float>(::atof(z.c_str()));

					float normX = static_cast<float>(::atof(n1.c_str()));
					float normY = static_cast<float>(::atof(n2.c_str()));
					float normZ = static_cast<float>(::atof(n3.c_str()));

					float u = static_cast<float>(::atof(uv1.c_str()));
					float v = static_cast<float>(::atof(uv2.c_str()));

					pointData.push_back(glm::vec3(xPos,yPos,zPos));
					colorData.push_back(glm::vec4(rand() % 100 * 0.01f,rand() % 100 * 0.01f,rand() % 100 * 0.01f,1));
					uvData.push_back(glm::vec2(u,1-v));
					normalData.push_back(glm::vec3(normX,normY,normZ));
				}
				#pragma endregion
				#pragma region Indices
				if((line[0] == '3' || line[0] == '4') && line[1] == ' '){
					for(unsigned int i = 0; i < line.length(); i++)
						if(line.at(i) == ' ')
							whitespaceCount++;
					for(unsigned int i = 0; i < line.length(); i++){
						if(line.at(i) == ' ')
							SpaceCount++;
						if(SpaceCount == 1)
							x += line.at(i);
						else if(SpaceCount == 2)
							y += line.at(i);
						else if(SpaceCount == 3)
							z += line.at(i);
						else if(SpaceCount == 4)
							w += line.at(i);
					}
					int index1 = static_cast<int>(::atof(x.c_str()));
					int index2 = static_cast<int>(::atof(y.c_str()));
					int index3 = static_cast<int>(::atof(z.c_str()));
					int index4 = static_cast<int>(::atof(w.c_str()));
					if(whitespaceCount == 4)
						GenerateQuad(pointData,colorData,uvData,normalData,index1,index2,index3,index4);
					else
						GenerateTriangle(pointData,colorData,uvData,normalData,index1,index2,index3);
				}
				#pragma endregion
			}
			if(line == "end_header")
				StartReadingGeometryData = true;
		}
	}
	reader.close();
}
Mesh::~Mesh(){
	for(unsigned int i = 0; i < NUM_VERTEX_DATA; i++)
		glDeleteBuffers(1, &m_buffers[i]);
}
void Mesh::GenerateTriangle(const std::vector<glm::vec3>& pointData, const std::vector<glm::vec4>& colorData,const std::vector<glm::vec2>& uvData,const std::vector<glm::vec3>& normalData,const int index1,const int index2,const int index3){
	m_Colors.push_back(glm::vec4(colorData[index1].x,colorData[index1].y,colorData[index1].z,colorData[index1].a));
	m_Points.push_back(glm::vec3(pointData[index1].x,pointData[index1].y,pointData[index1].z));
	m_UVs.push_back(glm::vec2(uvData[index1].x,uvData[index1].y));
	m_Normals.push_back(glm::vec3(normalData[index1].x,normalData[index1].y,normalData[index1].z));

	m_Colors.push_back(glm::vec4(colorData[index2].x,colorData[index2].y,colorData[index2].z,colorData[index2].a));
	m_Points.push_back(glm::vec3(pointData[index2].x,pointData[index2].y,pointData[index2].z));
	m_UVs.push_back(glm::vec2(uvData[index2].x,uvData[index2].y));
	m_Normals.push_back(glm::vec3(normalData[index2].x,normalData[index2].y,normalData[index2].z));

	m_Colors.push_back(glm::vec4(colorData[index3].x,colorData[index3].y,colorData[index3].z,colorData[index3].a));
	m_Points.push_back(glm::vec3(pointData[index3].x,pointData[index3].y,pointData[index3].z));
	m_UVs.push_back(glm::vec2(uvData[index3].x,uvData[index3].y));
	m_Normals.push_back(glm::vec3(normalData[index3].x,normalData[index3].y,normalData[index3].z));

	Vertex v1, v2, v3;

	v1.position = pointData[index1];
	v2.position = pointData[index2];
	v3.position = pointData[index3];

	v1.color = colorData[index1];
	v2.color = colorData[index2];
	v3.color = colorData[index3];

	v1.uv = uvData[index1];
	v2.uv = uvData[index2];
	v3.uv = uvData[index3];

	v1.normal = normalData[index1];
	v2.normal = normalData[index2];
	v3.normal = normalData[index3];

	CalculateTangentBinormal(v1,v2,v3);

	m_Binormals.push_back(v1.binormal);
	m_Binormals.push_back(v2.binormal);
	m_Binormals.push_back(v3.binormal);

	m_Tangents.push_back(v1.tangent);
	m_Tangents.push_back(v2.tangent);
	m_Tangents.push_back(v3.tangent);
}
void Mesh::GenerateQuad(const std::vector<glm::vec3>& pointData, const std::vector<glm::vec4>& colorData,const std::vector<glm::vec2>& uvData,const std::vector<glm::vec3>& normalData,const int index1,const int index2,const int index3,const int index4){
	GenerateTriangle(pointData,colorData,uvData,normalData,index1,index2,index3);
	GenerateTriangle(pointData,colorData,uvData,normalData,index1,index3,index4);
}
void Mesh::Init(){

	//Bind the data to the buffers
	glGenBuffers((sizeof(m_buffers)/sizeof(m_buffers[0])), m_buffers);

	glBindBuffer(GL_ARRAY_BUFFER, m_buffers[0] );
	glBufferData(GL_ARRAY_BUFFER, m_Points.size() * sizeof(glm::vec3),&m_Points[0], GL_STATIC_DRAW );

	glBindBuffer(GL_ARRAY_BUFFER, m_buffers[1]);
	glBufferData(GL_ARRAY_BUFFER, m_UVs.size() * sizeof(glm::vec2), &m_UVs[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, m_buffers[2]);
	glBufferData(GL_ARRAY_BUFFER, m_Normals.size() * sizeof(glm::vec3), &m_Normals[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, m_buffers[3]);
	glBufferData(GL_ARRAY_BUFFER, m_Binormals.size() * sizeof(glm::vec3), &m_Binormals[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, m_buffers[4]);
	glBufferData(GL_ARRAY_BUFFER, m_Tangents.size() * sizeof(glm::vec3), &m_Tangents[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, m_buffers[5]);
	glBufferData(GL_ARRAY_BUFFER, m_Colors.size() * sizeof(glm::vec4), &m_Colors[0], GL_STATIC_DRAW);

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
	m_radius = glm::vec3(maxX,maxY,maxZ);
	#pragma endregion
}
void Mesh::Render(){
	//for each unique vertex data type (position, color, uv, normal, binormal, tangent)...
	for(unsigned int i = 0; i < NUM_VERTEX_DATA; i++){
		glBindBuffer( GL_ARRAY_BUFFER, m_buffers[i] );
		glEnableVertexAttribArray( i );
		glVertexAttribPointer( i, VERTEX_AMOUNTS[i], GL_FLOAT, GL_FALSE, 0, 0 );
	}
	glDrawArrays(GL_TRIANGLES, 0, m_Points.size());
	for(unsigned int i = 0; i < NUM_VERTEX_DATA; i++)
		glDisableVertexAttribArray(i);
}
void Mesh::CalculateTangentBinormal(Vertex& v1, Vertex& v2, Vertex& v3){
	glm::vec3 tangent, binormal;
	float vector1[3], vector2[3];
	float tuVector[2], tvVector[2];
	float den;
	float length;

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

	binormal.x = (tuVector[0] * vector2[0] - tuVector[1] * vector1[0]) * den;
	binormal.y = (tuVector[0] * vector2[1] - tuVector[1] * vector1[1]) * den;
	binormal.z = (tuVector[0] * vector2[2] - tuVector[1] * vector1[2]) * den;

	// Calculate the length of this normal.
	length = sqrt((tangent.x * tangent.x) + (tangent.y * tangent.y) + (tangent.z * tangent.z));
			
	// Normalize the normal and then store it
	tangent.x = tangent.x / length;
	tangent.y = tangent.y / length;
	tangent.z = tangent.z / length;

	// Calculate the length of this normal.
	length = sqrt((binormal.x * binormal.x) + (binormal.y * binormal.y) + (binormal.z * binormal.z));
			
	// Normalize the normal and then store it
	binormal.x = binormal.x / length;
	binormal.y = binormal.y / length;
	binormal.z = binormal.z / length;

	v1.binormal = binormal; v2.binormal = binormal; v3.binormal = binormal;
	v1.tangent = tangent;   v2.tangent = tangent;   v3.tangent = tangent;
}
std::vector<glm::vec3>& Mesh::Points(){ return m_Points; }
std::vector<glm::vec4>& Mesh::Colors(){ return m_Colors; }
std::vector<glm::vec2>& Mesh::UVS(){ return m_UVs; }
std::vector<glm::vec3>& Mesh::Normals(){ return m_Normals; }
std::vector<glm::vec3>& Mesh::Binormals(){ return m_Binormals; }
std::vector<glm::vec3>& Mesh::Tangents(){ return m_Tangents; }
glm::vec3 Mesh::Radius(){ return m_radius; }
GLuint* Mesh::Buffers(){ return m_buffers; }