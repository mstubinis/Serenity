#include "Engine_Resources.h"
#include "Engine_Renderer.h"
#include "Mesh.h"

#include <bullet/btBulletDynamicsCommon.h>
#include <bullet/BulletCollision/CollisionShapes/btHeightfieldTerrainShape.h>
#include <glm/gtc/matrix_transform.hpp>

#include <boost/filesystem.hpp>

Mesh::Mesh(std::string& name,btHeightfieldTerrainShape* heightfield,float threshold){
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

            Engine::Resources::MeshLoader::Detail::MeshLoadingManagement::_calculateTBN(d);

        }
    }
    _loadData(d,threshold);
    _calculateMeshRadius();
    this->setName(name);
}
Mesh::Mesh(std::string& name,std::unordered_map<std::string,float>& grid,uint width,uint length,float threshold){
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

            Engine::Resources::MeshLoader::Detail::MeshLoadingManagement::_calculateTBN(d);
        }
    }
	_loadData(d,threshold);
    _calculateMeshRadius();
    this->setName(name);
}
Mesh::Mesh(std::string& name,float x, float y,float width, float height,float threshold){
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

    _loadData(d,threshold);
    _calculateMeshRadius();
    this->setName(name);
}
Mesh::Mesh(std::string& name,float width, float height,float threshold){
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

    _loadData(d,threshold);
    _calculateMeshRadius();
    this->setName(name);
}
Mesh::Mesh(std::string& name,std::string filename,COLLISION_TYPE type,bool notMemory,float threshold){
    m_Collision = nullptr;
    if(notMemory)
        _loadFromFile(filename, type,threshold);
    else
        _loadFromOBJMemory(filename,type,threshold);
    _calculateMeshRadius();
    this->setName(name);
}
Mesh::~Mesh(){
    cleanupRenderingContext();
    for(auto anim:m_Animations)
        delete anim.second;
}
void Mesh::_loadData(ImportedMeshData& data,float threshold){
    if(data.uvs.size() == 0) data.uvs.resize(data.points.size());
    if(data.normals.size() == 0) data.normals.resize(data.points.size());
    if(data.binormals.size() == 0) data.binormals.resize(data.points.size());
    if(data.tangents.size() == 0) data.tangents.resize(data.points.size());

    Engine::Resources::MeshLoader::Detail::MeshLoadingManagement::_indexVBO(data,m_Indices,m_Points,m_UVs,m_Normals,m_Binormals,m_Tangents,threshold);
}
void Mesh::_loadFromFile(std::string file,COLLISION_TYPE type,float threshold){
    std::string extention; for(uint i = file.length() - 4; i < file.length(); i++)extention += tolower(file.at(i));
    //if(extention == ".obj")
        _loadFromOBJ(file,type,threshold);
}
void Mesh::_loadFromOBJ(std::string filename,COLLISION_TYPE type,float threshold){
    ImportedMeshData d;
    Engine::Resources::MeshLoader::load(this,d,filename);
    _loadData(d,threshold);

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
    m_BoneMapping = d.m_BoneMapping;
    m_NumBones = d.m_NumBones;
    m_BoneInfo = d.m_BoneInfo;
    m_GlobalInverseTransform = d.m_GlobalInverseTransform;

    for(auto bone:d.m_Bones){
		VertexBoneData& b = bone.second;
		m_BoneIDs.push_back(glm::vec4(b.IDs[0],b.IDs[1],b.IDs[2],b.IDs[3]));
		m_BoneWeights.push_back(glm::vec4(b.Weights[0],b.Weights[1],b.Weights[2],b.Weights[3]));
	}
}
void Mesh::_loadFromOBJMemory(std::string data,COLLISION_TYPE type,float threshold){
    ImportedMeshData d;
    Engine::Resources::MeshLoader::loadObjFromMemory(d,data);
    _loadData(d,threshold);

    if(type == COLLISION_TYPE_NONE){
        m_Collision = new Collision(new btEmptyShape());
    }
    else{
        m_Collision = new Collision(d,type);
    }
}
void Mesh::initRenderingContext(){
    glGenBuffers((sizeof(m_buffers)/sizeof(m_buffers[0])), m_buffers);

    glBindBuffer(GL_ARRAY_BUFFER, m_buffers[0]);
    glBufferData(GL_ARRAY_BUFFER, m_Points.size() * sizeof(glm::vec3),&m_Points[0], GL_STATIC_DRAW );

    glBindBuffer(GL_ARRAY_BUFFER, m_buffers[1]);
    glBufferData(GL_ARRAY_BUFFER, m_UVs.size() * sizeof(glm::vec2), &m_UVs[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, m_buffers[2]);
    glBufferData(GL_ARRAY_BUFFER, m_Normals.size() * sizeof(glm::vec3), &m_Normals[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, m_buffers[3]);
    glBufferData(GL_ARRAY_BUFFER, m_Binormals.size() * sizeof(glm::vec3), &m_Binormals[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, m_buffers[4]);
    glBufferData(GL_ARRAY_BUFFER, m_Tangents.size() * sizeof(glm::vec3), &m_Tangents[0], GL_STATIC_DRAW);

    if(m_BoneIDs.size() > 0){
		glBindBuffer(GL_ARRAY_BUFFER, m_buffers[5]);
		glBufferData(GL_ARRAY_BUFFER, m_BoneIDs.size() * sizeof(glm::vec4), &m_BoneIDs[0], GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, m_buffers[6]);
		glBufferData(GL_ARRAY_BUFFER, m_BoneWeights.size() * sizeof(glm::vec4), &m_BoneWeights[0], GL_STATIC_DRAW);
    }
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
	Engine::Renderer::sendUniform1iSafe("AnimationPlaying",0); //move this over to a per object basis
    for(uint i = 0; i < NUM_VERTEX_DATA; i++){
		if(i <= 4 || (i >= 5 && m_BoneIDs.size() > 0)){
			glBindBuffer(GL_ARRAY_BUFFER, m_buffers[i]);
			glEnableVertexAttribArray(i);
			glVertexAttribPointer(i, VERTEX_AMOUNTS[i], GL_FLOAT, GL_FALSE, 0,(void*)0);
		}
    }

    //glDrawArrays(mode, 0, m_Points.size());
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_elementbuffer);
    glDrawElements(mode,m_Indices.size(),GL_UNSIGNED_SHORT,(void*)0);
    for(uint i = 0; i < NUM_VERTEX_DATA; i++){
        glDisableVertexAttribArray(i);
	}
}
void Mesh::playAnimation(std::string animationName,float time){
    m_Animations[animationName]->play(time);
}

AnimationData::AnimationData(Mesh* mesh,aiAnimation* anim){
    m_Mesh = mesh;
    m_Animation = anim;
    for(uint i = 0; i < m_Animation->mNumChannels; i++){
        std::string key = (m_Animation->mChannels[i]->mNodeName.data);
        m_NodeAnimMap.emplace(key,m_Animation->mChannels[i]);
    }
}
AnimationData::~AnimationData(){
    for(auto node:m_NodeAnimMap){
    }
}

void AnimationData::play(float time){
    std::vector<glm::mat4> Transforms;      
    _BoneTransform(time, Transforms);
	Engine::Renderer::sendUniform1iSafe("AnimationPlaying",1); //move this over to a per object basis
	Engine::Renderer::sendUniformMatrix4fvSafe("gBones[0]",Transforms,Transforms.size());

    //for (uint i = 0; i < Transforms.size(); i++){
    //    _SetBoneTransform(i, Transforms.at(i));
    //}
}
uint AnimationData::_FindPosition(float AnimationTime, const aiNodeAnim* node){    
    for (uint i = 0 ; i < node->mNumPositionKeys - 1 ; i++) {
        if (AnimationTime < (float)node->mPositionKeys[i + 1].mTime) { return i; }
    }  
    assert(0);
    return 0;
}
uint AnimationData::_FindRotation(float AnimationTime, const aiNodeAnim* node){
    assert(node->mNumRotationKeys > 0);
    for (uint i = 0 ; i < node->mNumRotationKeys - 1 ; i++) {
        if (AnimationTime < (float)node->mRotationKeys[i + 1].mTime) { return i; }
    }   
    assert(0);
    return 0;
}
uint AnimationData::_FindScaling(float AnimationTime, const aiNodeAnim* node){
    assert(node->mNumScalingKeys > 0);  
    for (uint i = 0 ; i < node->mNumScalingKeys - 1 ; i++) {
        if (AnimationTime < (float)node->mScalingKeys[i + 1].mTime) { return i; }
    }
    assert(0);
    return 0;
}
void AnimationData::_CalcInterpolatedPosition(glm::vec3& Out, float AnimationTime, const aiNodeAnim* node){
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
void AnimationData::_CalcInterpolatedRotation(aiQuaternion& Out, float AnimationTime, const aiNodeAnim* node){
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
void AnimationData::_CalcInterpolatedScaling(glm::vec3& Out, float AnimationTime, const aiNodeAnim* node){
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
void AnimationData::_ReadNodeHeirarchy(float AnimationTime, const aiNode* node, const glm::mat4& ParentTransform){    
    std::string NodeName(node->mName.data);    
    aiMatrix4x4 m = node->mTransformation;
	glm::mat4 NodeTransformation = Engine::Math::assimpToGLMMat4(m);
    const aiNodeAnim* nodeAnim = m_NodeAnimMap[NodeName]; 
    if (nodeAnim){
        // Interpolate scaling and generate scaling transformation matrix
        glm::vec3 scl;
        _CalcInterpolatedScaling(scl, AnimationTime, nodeAnim);
        glm::mat4 scaleMatrix = glm::mat4(1.0f);
        scaleMatrix = glm::scale(scaleMatrix,scl);
        
        // Interpolate rotation and generate rotation transformation matrix
        aiQuaternion quat;
        _CalcInterpolatedRotation(quat, AnimationTime, nodeAnim);
        aiMatrix3x3 m = quat.GetMatrix();
		glm::mat3 matr = Engine::Math::assimpToGLMMat3(m);
        glm::mat4 rotationMatrix = glm::mat4(matr);

        // Interpolate translation and generate translation transformation matrix
        glm::vec3 translation;
        _CalcInterpolatedPosition(translation, AnimationTime, nodeAnim);
        glm::mat4 translationMatrix = glm::mat4(1.0f);
        translationMatrix = glm::translate(translationMatrix,translation);
        
        // Combine the above transformations
        NodeTransformation = translationMatrix * rotationMatrix * scaleMatrix;
    }    
    glm::mat4 GlobalTransformation = ParentTransform * NodeTransformation;  
    if (m_Mesh->m_BoneMapping.find(NodeName) != m_Mesh->m_BoneMapping.end()){
        uint BoneIndex = m_Mesh->m_BoneMapping[NodeName];
        m_Mesh->m_BoneInfo[BoneIndex].FinalTransformation = m_Mesh->m_GlobalInverseTransform * GlobalTransformation * m_Mesh->m_BoneInfo[BoneIndex].BoneOffset;
    } 
    for (uint i = 0; i < node->mNumChildren; i++){
        _ReadNodeHeirarchy(AnimationTime, node->mChildren[i], GlobalTransformation);
    }
}
void AnimationData::_BoneTransform(float TimeInSeconds, std::vector<glm::mat4>& Transforms){
    glm::mat4 Identity = glm::mat4(1);
    float TicksPerSecond = float(m_Animation->mTicksPerSecond != 0 ? m_Animation->mTicksPerSecond : 25.0f);
    float TimeInTicks = TimeInSeconds * TicksPerSecond;
    float AnimationTime = float(fmod(TimeInTicks, m_Animation->mDuration));
    _ReadNodeHeirarchy(AnimationTime, m_Mesh->m_aiScene->mRootNode, Identity);
    Transforms.resize(m_Mesh->m_NumBones,glm::mat4(1));
    for (uint i = 0; i < m_Mesh->m_NumBones; i++) {
        Transforms[i] = m_Mesh->m_BoneInfo[i].FinalTransformation;
    }
}
void AnimationData::_SetBoneTransform(uint Index, glm::mat4& Transform){
	std::string location = ("gBones[" + boost::lexical_cast<std::string>(Index) + "]");
	Engine::Renderer::sendUniformMatrix4f(location.c_str(),Transform);
}
