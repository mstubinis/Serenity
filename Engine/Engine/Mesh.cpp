#include "Engine_Resources.h"
#include "Engine_MeshLoader.h"
#include "Mesh.h"

#include <bullet/btBulletDynamicsCommon.h>
#include <bullet/BulletCollision/CollisionShapes/btHeightfieldTerrainShape.h>
#include <glm/gtc/matrix_transform.hpp>

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
	Engine::Resources::MeshLoader::load(this,d,filename);
	_loadData(d);

    if(type == COLLISION_TYPE_NONE){
        m_Collision = new Collision(new btEmptyShape());
    }
    else{
        std::string colFile = filename.substr(0,filename.size()-4);
        colFile += "Col.obj";
        if(boost::filesystem::exists(colFile)){
			d.clear();
			Engine::Resources::MeshLoader::load(this,d,colFile);
        }
        m_Collision = new Collision(d,type);
    }
	m_aiScene = d.m_aiScene;
	m_NodeAnimMap = d.m_NodeAnimMap;

	m_BoneMapping = d.m_BoneMapping;
    m_NumBones = d.m_NumBones;
	m_BoneInfo = d.m_BoneInfo;
    m_GlobalInverseTransform = d.m_GlobalInverseTransform;
	m_Bones = d.m_Bones;
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
void Mesh::_boneTransform(float TimeInSeconds, std::vector<glm::mat4>& Transforms){
    glm::mat4 Identity = glm::mat4(1);

	float TicksPerSecond = m_aiScene->mAnimations[0]->mTicksPerSecond != 0 ? 
                           m_aiScene->mAnimations[0]->mTicksPerSecond : 25.0f;
    float TimeInTicks = TimeInSeconds * TicksPerSecond;
    float AnimationTime = fmod(TimeInTicks, m_aiScene->mAnimations[0]->mDuration);
    _ReadNodeHeirarchy(AnimationTime, m_aiScene->mRootNode, Identity);
    Transforms.resize(m_NumBones);
    for (uint i = 0 ; i < m_NumBones ; i++) {
        Transforms[i] = m_BoneInfo[i].FinalTransformation;
    }
}
void Mesh::_CalcInterpolatedPosition(glm::vec3& Out, float AnimationTime, const aiNodeAnim* node){
    if (node->mNumPositionKeys == 1) {
        Out.x = node->mPositionKeys[0].mValue.x;
		Out.y = node->mPositionKeys[0].mValue.y;
		Out.z = node->mPositionKeys[0].mValue.z;
        return;
    }           
    uint PositionIndex = _FindPosition(AnimationTime,node);
    uint NextPositionIndex = (PositionIndex + 1);
    assert(NextPositionIndex < node->mNumPositionKeys);
    float DeltaTime = (float)(node->mPositionKeys[NextPositionIndex].mTime - node->mPositionKeys[PositionIndex].mTime);
    float Factor = (AnimationTime - (float)node->mPositionKeys[PositionIndex].mTime) / DeltaTime;
    assert(Factor >= 0.0f && Factor <= 1.0f);
    glm::vec3 Start = glm::vec3(node->mPositionKeys[PositionIndex].mValue.x,node->mPositionKeys[PositionIndex].mValue.y,node->mPositionKeys[PositionIndex].mValue.z);
    glm::vec3 End = glm::vec3(node->mPositionKeys[NextPositionIndex].mValue.x,node->mPositionKeys[NextPositionIndex].mValue.y,node->mPositionKeys[NextPositionIndex].mValue.z);
    glm::vec3 Delta = End - Start;
    Out = Start + Factor * Delta;
}

void Mesh::_CalcInterpolatedRotation(aiQuaternion& Out, float AnimationTime, const aiNodeAnim* node){
	// we need at least two values to interpolate...
    if (node->mNumRotationKeys == 1) {
        Out = node->mRotationKeys[0].mValue;
        return;
    }
    uint RotationIndex = _FindRotation(AnimationTime, node);
    uint NextRotationIndex = (RotationIndex + 1);
    assert(NextRotationIndex < node->mNumRotationKeys);
    float DeltaTime = (float)(node->mRotationKeys[NextRotationIndex].mTime - node->mRotationKeys[RotationIndex].mTime);
    float Factor = (AnimationTime - (float)node->mRotationKeys[RotationIndex].mTime) / DeltaTime;
    assert(Factor >= 0.0f && Factor <= 1.0f);
    const aiQuaternion& StartRotationQ = node->mRotationKeys[RotationIndex].mValue;
    const aiQuaternion& EndRotationQ   = node->mRotationKeys[NextRotationIndex].mValue;    
    aiQuaternion::Interpolate(Out, StartRotationQ, EndRotationQ, Factor);
    Out = Out.Normalize();
}
uint Mesh::_FindPosition(float AnimationTime, const aiNodeAnim* pNodeAnim){    
    for (uint i = 0 ; i < pNodeAnim->mNumPositionKeys - 1 ; i++) {
        if (AnimationTime < (float)pNodeAnim->mPositionKeys[i + 1].mTime) {
            return i;
        }
    }  
    assert(0);
    return 0;
}
uint Mesh::_FindRotation(float AnimationTime, const aiNodeAnim* pNodeAnim){
    assert(pNodeAnim->mNumRotationKeys > 0);
    for (uint i = 0 ; i < pNodeAnim->mNumRotationKeys - 1 ; i++) {
        if (AnimationTime < (float)pNodeAnim->mRotationKeys[i + 1].mTime) {
            return i;
        }
    }   
    assert(0);
    return 0;
}
uint Mesh::_FindScaling(float AnimationTime, const aiNodeAnim* pNodeAnim){
    assert(pNodeAnim->mNumScalingKeys > 0);  
    for (uint i = 0 ; i < pNodeAnim->mNumScalingKeys - 1 ; i++) {
        if (AnimationTime < (float)pNodeAnim->mScalingKeys[i + 1].mTime) {
            return i;
        }
    }
    assert(0);
    return 0;
}
void Mesh::_CalcInterpolatedScaling(glm::vec3& Out, float AnimationTime, const aiNodeAnim* node){
    if (node->mNumScalingKeys == 1) {
        Out.x = node->mScalingKeys[0].mValue.x;
		Out.y = node->mScalingKeys[0].mValue.y;
		Out.z = node->mScalingKeys[0].mValue.z;
        return;
    }
    uint ScalingIndex = _FindScaling(AnimationTime, node);
    uint NextScalingIndex = (ScalingIndex + 1);
    assert(NextScalingIndex < node->mNumScalingKeys);
    float DeltaTime = (float)(node->mScalingKeys[NextScalingIndex].mTime - node->mScalingKeys[ScalingIndex].mTime);
    float Factor = (AnimationTime - (float)node->mScalingKeys[ScalingIndex].mTime) / DeltaTime;
    assert(Factor >= 0.0f && Factor <= 1.0f);
    glm::vec3 Start = glm::vec3(node->mScalingKeys[ScalingIndex].mValue.x,node->mScalingKeys[ScalingIndex].mValue.y,node->mScalingKeys[ScalingIndex].mValue.z);
    glm::vec3 End   = glm::vec3(node->mScalingKeys[NextScalingIndex].mValue.x,node->mScalingKeys[NextScalingIndex].mValue.y,node->mScalingKeys[NextScalingIndex].mValue.z);
    glm::vec3 Delta = End - Start;
    Out = Start + Factor * Delta;
}
const aiNodeAnim* Mesh::_FindNodeAnim(const aiAnimation* pAnimation, const std::string NodeName){
	return m_NodeAnimMap[NodeName];
}
void Mesh::_ReadNodeHeirarchy(float AnimationTime, const aiNode* pNode, const glm::mat4& ParentTransform){    
    std::string NodeName(pNode->mName.data);
	const aiAnimation* pAnimation = m_aiScene->mAnimations[0];     
	aiMatrix4x4 m = pNode->mTransformation;
    glm::mat4 NodeTransformation = glm::mat4(m.a1,m.a2,m.a3,m.a4,
				                             m.b1,m.b2,m.b3,m.b4,
			                                 m.c1,m.c2,m.c3,m.c4,
											 m.d1,m.d2,m.d3,m.d4);

    const aiNodeAnim* pNodeAnim = _FindNodeAnim(pAnimation, NodeName); 
    if (pNodeAnim) {
        // Interpolate scaling and generate scaling transformation matrix
        glm::vec3 Scaling;
        _CalcInterpolatedScaling(Scaling, AnimationTime, pNodeAnim);
        glm::mat4 ScalingM = glm::mat4(1);
		ScalingM = glm::scale(ScalingM,Scaling);
        
        // Interpolate rotation and generate rotation transformation matrix
        aiQuaternion RotationQ;
        _CalcInterpolatedRotation(RotationQ, AnimationTime, pNodeAnim);
		aiMatrix3x3 m = RotationQ.GetMatrix();
		glm::mat3 threeByThreeMatrix = glm::mat3(m.a1,m.a2,m.a3,
			                                     m.b1,m.b2,m.b3,
												 m.c1,m.c2,m.c3);
        glm::mat4 RotationM = glm::mat4(threeByThreeMatrix);

        // Interpolate translation and generate translation transformation matrix
        glm::vec3 Translation;
        _CalcInterpolatedPosition(Translation, AnimationTime, pNodeAnim);
        glm::mat4 TranslationM = glm::mat4(1);
		TranslationM = glm::translate(TranslationM,Translation);
        
        // Combine the above transformations
        NodeTransformation = TranslationM * RotationM * ScalingM;
    }    
    glm::mat4 GlobalTransformation = ParentTransform * NodeTransformation;  
    if (m_BoneMapping.find(NodeName) != m_BoneMapping.end()) {
        uint BoneIndex = m_BoneMapping[NodeName];
        m_BoneInfo[BoneIndex].FinalTransformation = m_GlobalInverseTransform * GlobalTransformation * m_BoneInfo[BoneIndex].BoneOffset;
    } 
    for (uint i = 0 ; i < pNode->mNumChildren ; i++) {
        _ReadNodeHeirarchy(AnimationTime, pNode->mChildren[i], GlobalTransformation);
    }
}