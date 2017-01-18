#include "Engine_Resources.h"
#include "Engine_MeshLoader.h"
#include "Mesh.h"

#include <bullet/btBulletDynamicsCommon.h>
#include <bullet/BulletCollision/CollisionShapes/btHeightfieldTerrainShape.h>

#include <boost/filesystem.hpp>

Mesh::Mesh(std::string& name,btHeightfieldTerrainShape* heightfield){
    m_Collision = nullptr;
	ImportedMeshData d;
    uint width = heightfield->getHeightStickWidth();
    uint length = heightfield->getHeightStickLength();
    for(uint i = 0; i < width-1; i++){
        for(uint j = 0; j < length-1; j++){
            btVector3 vert1,vert2,vert3,vert4;
            heightfield->getVertex1(i,  j,  vert1);
            heightfield->getVertex1(i+1,j,  vert2);
            heightfield->getVertex1(i,  j+1,vert3);
            heightfield->getVertex1(i+1,j+1,vert4);

            Vertex v1,v2,v3,v4;
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

			Engine::Resources::MeshLoader::Detail::_calculateTBN(d);

        }
    }
	_loadData(d);
    _calculateMeshRadius();
	this->setName(name);
}
Mesh::Mesh(std::string& name,std::unordered_map<std::string,float>& grid,uint width,uint length){
    m_Collision = nullptr;
	ImportedMeshData d;
    for(uint i = 0; i < width-1; i++){
        for(uint j = 0; j < length-1; j++){
			std::string key1 = boost::lexical_cast<std::string>(i) + "," + boost::lexical_cast<std::string>(j);
			std::string key2 = boost::lexical_cast<std::string>(i+1) + "," + boost::lexical_cast<std::string>(j);
			std::string key3 = boost::lexical_cast<std::string>(i) + "," + boost::lexical_cast<std::string>(j+1);
			std::string key4 = boost::lexical_cast<std::string>(i+1) + "," + boost::lexical_cast<std::string>(j+1);

            Vertex v1,v2,v3,v4;
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

			Engine::Resources::MeshLoader::Detail::_calculateTBN(d);
        }
    }
	_loadData(d);
    _calculateMeshRadius();
	this->setName(name);
}
Mesh::Mesh(std::string& name,float x, float y,float width, float height){
    m_Collision = nullptr;
	ImportedMeshData d;
	d.points.push_back(glm::vec3(0,0,0));
    d.points.push_back(glm::vec3(width,height,0));
    d.points.push_back(glm::vec3(0,height,0));

    d.points.push_back(glm::vec3(width,0,0));
    d.points.push_back(glm::vec3(width,height,0));
    d.points.push_back(glm::vec3(0,0,0));

    float uv_topLeft_x = float(x/256.0f);
    float uv_topLeft_y = float(y/256.0f);
    
    float uv_bottomLeft_x = float(x/256.0f);
    float uv_bottomLeft_y = float(y/256.0f) + float(height/256.0f);

    float uv_bottomRight_x = float(x/256.0f) + float(width/256.0f);
    float uv_bottomRight_y = float(y/256.0f) + float(height/256.0f);

    float uv_topRight_x = float(x/256.0f) + float(width/256.0f);
    float uv_topRight_y = float(y/256.0f);

	d.uvs.push_back(glm::vec2(uv_bottomLeft_x,uv_bottomLeft_y));
    d.uvs.push_back(glm::vec2(uv_topRight_x,uv_topRight_y));
    d.uvs.push_back(glm::vec2(uv_topLeft_x,uv_topLeft_y));

    d.uvs.push_back(glm::vec2(uv_bottomRight_x,uv_bottomRight_y));
    d.uvs.push_back(glm::vec2(uv_topRight_x,uv_topRight_y));
    d.uvs.push_back(glm::vec2(uv_bottomLeft_x,uv_bottomLeft_y));

	d.normals.resize(6,glm::vec3(1,1,1));
	d.binormals.resize(6,glm::vec3(1,1,1));
    d.tangents.resize(6,glm::vec3(1,1,1));

	_loadData(d);
    _calculateMeshRadius();
	this->setName(name);
}
Mesh::Mesh(std::string& name,float width, float height){
    m_Collision = nullptr;
	ImportedMeshData d;
	d.points.push_back(glm::vec3(-width/2.0f,-height/2.0f,0));
    d.points.push_back(glm::vec3(width/2.0f,height/2.0f,0));
    d.points.push_back(glm::vec3(-width/2.0f,height/2.0f,0));

    d.points.push_back(glm::vec3(width/2.0f,-height/2.0f,0));
    d.points.push_back(glm::vec3(width/2.0f,height/2.0f,0));
    d.points.push_back(glm::vec3(-width/2.0f,-height/2.0f,0));

    float uv_topLeft_x = float(0);
    float uv_topLeft_y = float(0);
    
    float uv_bottomLeft_x = float(0);
    float uv_bottomLeft_y = float(0) + float(height);

    float uv_bottomRight_x = float(0) + float(width);
    float uv_bottomRight_y = float(0) + float(height);

    float uv_topRight_x = float(0) + float(width);
    float uv_topRight_y = float(0);

	d.uvs.push_back(glm::vec2(uv_bottomLeft_x,uv_bottomLeft_y));
    d.uvs.push_back(glm::vec2(uv_topRight_x,uv_topRight_y));
    d.uvs.push_back(glm::vec2(uv_topLeft_x,uv_topLeft_y));

    d.uvs.push_back(glm::vec2(uv_bottomRight_x,uv_bottomRight_y));
    d.uvs.push_back(glm::vec2(uv_topRight_x,uv_topRight_y));
    d.uvs.push_back(glm::vec2(uv_bottomLeft_x,uv_bottomLeft_y));

	d.normals.resize(6,glm::vec3(1,1,1));
	d.binormals.resize(6,glm::vec3(1,1,1));
    d.tangents.resize(6,glm::vec3(1,1,1));

	_loadData(d);
    _calculateMeshRadius();
	this->setName(name);
}
Mesh::Mesh(std::string& name,std::string filename,COLLISION_TYPE type,bool notMemory){
    m_Collision = nullptr;
    if(notMemory)
        _loadFromFile(filename, type);
    else
        _loadFromOBJMemory(filename,type);
    _calculateMeshRadius();
	this->setName(name);
}
Mesh::~Mesh(){
    cleanupRenderingContext();
}
void Mesh::_loadData(ImportedMeshData& data,float threshold){
	if(data.uvs.size() == 0) data.uvs.resize(data.points.size());
	if(data.normals.size() == 0) data.normals.resize(data.points.size());
	if(data.binormals.size() == 0) data.binormals.resize(data.points.size());
	if(data.tangents.size() == 0) data.tangents.resize(data.points.size());

	Engine::Resources::MeshLoader::Detail::_indexVBO(data,m_Indices,m_Points,m_UVs,m_Normals,m_Binormals,m_Tangents,threshold);
}
void Mesh::_loadFromFile(std::string file,COLLISION_TYPE type){
    std::string extention; for(uint i = file.length() - 4; i < file.length(); i++)extention += tolower(file.at(i));
    if(extention == ".obj")
        _loadFromOBJ(file,type);
}
void Mesh::_loadFromOBJ(std::string filename,COLLISION_TYPE type){
	ImportedMeshData d;
	Engine::Resources::MeshLoader::loadObj(d,filename);
	_loadData(d);

    if(type == COLLISION_TYPE_NONE){
        m_Collision = new Collision(new btEmptyShape());
    }
    else{
        std::string colFile = filename.substr(0,filename.size()-4);
        colFile += "Col.obj";
        if(boost::filesystem::exists(colFile)){
			d.clear();
			Engine::Resources::MeshLoader::loadObj(d,colFile);
        }
        m_Collision = new Collision(d,type);
    }
}
void Mesh::_loadFromOBJMemory(std::string data,COLLISION_TYPE type){
	ImportedMeshData d;
	Engine::Resources::MeshLoader::loadObjFromMemory(d,data);
	_loadData(d);

    if(type == COLLISION_TYPE_NONE){
        m_Collision = new Collision(new btEmptyShape());
    }
    else{
        m_Collision = new Collision(d,type);
    }
}
void Mesh::initRenderingContext(){
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

	glGenBuffers(1, &m_elementbuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_elementbuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_Indices.size() * sizeof(ushort), &m_Indices[0] , GL_STATIC_DRAW);
}
void Mesh::cleanupRenderingContext(){
    for(uint i = 0; i < NUM_VERTEX_DATA; i++){
        glDeleteBuffers(1, &m_buffers[i]);
    }
	glDeleteBuffers(1,&m_elementbuffer);
}
void Mesh::_calculateMeshRadius(){
    float maxX = 0; float maxY = 0; float maxZ = 0;
    for(auto point:m_Points){
        float x = abs(point.x); float y = abs(point.y); float z = abs(point.z);
        if(x > maxX) maxX = x; if(y > maxY) maxY = y; if(z > maxZ) maxZ = z;
    }
    m_radiusBox = glm::vec3(maxX,maxY,maxZ);
    m_radius = Engine::Math::Max(m_radiusBox);
}
void Mesh::render(GLuint mode){
    for(uint i = 0; i < NUM_VERTEX_DATA; i++){
        glBindBuffer( GL_ARRAY_BUFFER, m_buffers[i] );
        glEnableVertexAttribArray(i);
        glVertexAttribPointer(i, VERTEX_AMOUNTS[i], GL_FLOAT, GL_FALSE, 0,(void*)0);
    }
    //glDrawArrays(mode, 0, m_Points.size());
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_elementbuffer);
	glDrawElements(mode,m_Indices.size(),GL_UNSIGNED_SHORT,(void*)0);
    for(uint i = 0; i < NUM_VERTEX_DATA; i++)
        glDisableVertexAttribArray(i);
}