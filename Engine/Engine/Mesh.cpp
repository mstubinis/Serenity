#include "Engine_Resources.h"
#include "Engine_Renderer.h"
#include "Mesh.h"
#include "MeshInstance.h"

#include <bullet/btBulletDynamicsCommon.h>
#include <bullet/BulletCollision/CollisionShapes/btHeightfieldTerrainShape.h>
#include <glm/gtc/matrix_transform.hpp>

#include <boost/filesystem.hpp>
#include <boost/tuple/tuple.hpp>

#include <iostream>


std::unordered_map<uint,boost::tuple<uint,uint,uint,uint,uint,uint> _populateVertexFormatMap(){
    std::unordered_map<uint,boost::tuple<uint,uint,uint,uint,uint,uint> m;
                                                //#components  //componentFormat   //normalized?
    m[VertexFormat::Position]    = boost::tuple(3,             GL_FLOAT,           GL_FALSE,       0,0,0);
    m[VertexFormat::UV]          = boost::tuple(2,             GL_FLOAT,           GL_FALSE,       0,0,0);
    m[VertexFormat::Normal]      = boost::tuple(3,             GL_FLOAT,           GL_FALSE,       0,0,0);
    m[VertexFormat::Binormal]    = boost::tuple(3,             GL_FLOAT,           GL_FALSE,       0,0,0);
    m[VertexFormat::Tangent]     = boost::tuple(3,             GL_FLOAT,           GL_FALSE,       0,0,0);
    m[VertexFormat::BoneIDs]     = boost::tuple(4,             GL_FLOAT,           GL_FALSE,       0,0,0);
    m[VertexFormat::BoneWeights] = boost::tuple(4,             GL_FLOAT,           GL_FALSE,       0,0,0);
    
    return m;
}
std::unordered_map<uint,boost::tuple<uint,uint,uint,uint,uint,uint> VERTEX_FORMAT_DATA = _populateVertexFormatMap();

struct DefaultMeshBindFunctor{void operator()(BindableResource* r) const {
    Mesh* mesh = static_cast<Mesh*>(r);
    for(uint i = 0; i < VertexFormat::EnumTotal; i++){
        if(i <= 4 || (mesh->m_Skeleton != nullptr && (i >= 5 && mesh->m_Skeleton->m_BoneIDs.size() > 0))){
            boost::tuple<uint,uint,uint,uint,uint,uint>& format = VERTEX_FORMAT_DATA.at(i);
            glBindBuffer(GL_ARRAY_BUFFER, mesh->m_buffers[i]);
            glEnableVertexAttribArray(i);
            glVertexAttribPointer(i, format.get<0>(), format.get<1>(), format.get<2>(), 0,(void*)0);
        }
    }
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->m_elementbuffer);
}};
struct DefaultMeshUnbindFunctor{void operator()(BindableResource* r) const {
    //Mesh* mesh = static_cast<Mesh*>(r);
    for(uint i = 0; i < VertexFormat::EnumTotal; i++){
        glDisableVertexAttribArray(i);
    }
}};
DefaultMeshBindFunctor Mesh::DEFAULT_BIND_FUNCTOR;
DefaultMeshUnbindFunctor Mesh::DEFAULT_UNBIND_FUNCTOR;

Mesh::Mesh(std::string& name,btHeightfieldTerrainShape* heightfield,float threshold):BindableResource(name){
    m_File = "";
    m_threshold = threshold;
    m_Collision = nullptr;
    m_Skeleton = nullptr;
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
    this->setCustomBindFunctor(Mesh::DEFAULT_BIND_FUNCTOR);
    this->setCustomUnbindFunctor(Mesh::DEFAULT_UNBIND_FUNCTOR);
    this->load();
}
Mesh::Mesh(std::string& name,std::unordered_map<std::string,float>& grid,uint width,uint length,float threshold):BindableResource(name){
    m_File = "";
    m_threshold = threshold;
    m_Collision = nullptr;
    m_Skeleton = nullptr;
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
    this->setCustomBindFunctor(Mesh::DEFAULT_BIND_FUNCTOR);
    this->setCustomUnbindFunctor(Mesh::DEFAULT_UNBIND_FUNCTOR);
    this->load();
}
Mesh::Mesh(std::string& name,float x, float y,float width, float height,float threshold):BindableResource(name){
    m_File = "";
    m_threshold = threshold;
    m_Collision = nullptr;
    m_Skeleton = nullptr;
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
    this->setCustomBindFunctor(Mesh::DEFAULT_BIND_FUNCTOR);
    this->setCustomUnbindFunctor(Mesh::DEFAULT_UNBIND_FUNCTOR);
    this->load();
}
Mesh::Mesh(std::string& name,float width, float height,float threshold):BindableResource(name){
    m_File = "";
    m_threshold = threshold;
    m_Collision = nullptr;
    m_Skeleton = nullptr;
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
    this->setCustomBindFunctor(Mesh::DEFAULT_BIND_FUNCTOR);
    this->setCustomUnbindFunctor(Mesh::DEFAULT_UNBIND_FUNCTOR);
    this->load();
}
Mesh::Mesh(std::string& name,std::string filename,CollisionType type,bool notMemory,float threshold):BindableResource(name){
    m_File = "";
    m_Collision = nullptr;
    m_Skeleton = nullptr;
    if(notMemory){
        m_File = filename;
        m_Type = type;
        m_threshold = threshold;
    }
    else{
        _loadFromOBJMemory(filename,type,threshold);
    }
    this->setCustomBindFunctor(Mesh::DEFAULT_BIND_FUNCTOR);
    this->setCustomUnbindFunctor(Mesh::DEFAULT_UNBIND_FUNCTOR);
    this->load();
}
Mesh::~Mesh(){
    this->unload();
    _clearData(); //does this need to be called twice?
}
void Mesh::_loadData(ImportedMeshData& data,float threshold){
    m_threshold = threshold;

    if(data.uvs.size() == 0) data.uvs.resize(data.points.size());
    if(data.normals.size() == 0) data.normals.resize(data.points.size());
    if(data.binormals.size() == 0) data.binormals.resize(data.points.size());
    if(data.tangents.size() == 0) data.tangents.resize(data.points.size());

    Engine::Resources::MeshLoader::Detail::MeshLoadingManagement::_indexVBO(data,m_Indices,m_Points,m_UVs,m_Normals,m_Binormals,m_Tangents,m_threshold);
}
void Mesh::_clearData(){
    m_Indices.clear();
    m_Points.clear();
    m_UVs.clear();
    m_Normals.clear();
    m_Binormals.clear();
    m_Tangents.clear();
    if(m_Skeleton != nullptr){
        delete m_Skeleton;
        m_Skeleton = nullptr;
    }
}
void Mesh::_loadFromFile(std::string file,CollisionType type,float threshold){
    std::string extention; for(uint i = m_File.length() - 4; i < m_File.length(); i++)extention += tolower(m_File.at(i));
    ImportedMeshData d;
    Engine::Resources::MeshLoader::load(this,d,m_File);
    m_threshold = threshold; //this is needed
    _loadData(d,m_threshold);

    if(type == CollisionType::None){
        m_Collision = new Collision(new btEmptyShape());
    }
    else{
        std::string colFile = m_File.substr(0,m_File.size()-4);
        colFile += "Col.obj";
        if(boost::filesystem::exists(colFile)){
            d.clear();
            Engine::Resources::MeshLoader::load(this,d,colFile);
        }
        m_Collision = new Collision(d,type);
    }
}
void Mesh::_loadFromOBJMemory(std::string data,CollisionType type,float threshold){
    ImportedMeshData d;
    Engine::Resources::MeshLoader::loadObjFromMemory(d,data);
    _loadData(d,threshold);

    if(type == CollisionType::None){
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

    if(m_Skeleton != nullptr){
        glBindBuffer(GL_ARRAY_BUFFER, m_buffers[5]);
        glBufferData(GL_ARRAY_BUFFER, m_Skeleton->m_BoneIDs.size() * sizeof(glm::vec4), &m_Skeleton->m_BoneIDs[0], GL_STATIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, m_buffers[6]);
        glBufferData(GL_ARRAY_BUFFER, m_Skeleton->m_BoneWeights.size() * sizeof(glm::vec4), &m_Skeleton->m_BoneWeights[0], GL_STATIC_DRAW);
    }
    glGenBuffers(1, &m_elementbuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_elementbuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_Indices.size() * sizeof(ushort), &m_Indices[0] , GL_STATIC_DRAW);
}
void Mesh::cleanupRenderingContext(){
    for(uint i = 0; i < VertexFormat::EnumTotal; i++){
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
    glDrawElements(mode,m_Indices.size(),GL_UNSIGNED_SHORT,(void*)0);
}

void Mesh::playAnimation(std::vector<glm::mat4>& transforms,const std::string& animationName,float time){
    m_Skeleton->m_AnimationData[animationName]->_BoneTransform(animationName,time, transforms);
}
void Mesh::load(){
    if(!isLoaded()){
        if(m_File != ""){
            _loadFromFile(m_File,m_Type,m_threshold);
        }
        _calculateMeshRadius();
        initRenderingContext();
        std::cout << "(Mesh) ";
        EngineResource::load();
    }
}
void Mesh::unload(){
    if(isLoaded() && useCount() == 0){
        if(m_File != ""){
            _clearData();
        }
        cleanupRenderingContext();
        std::cout << "(Mesh) ";
        EngineResource::unload();
    }
}

AnimationData::AnimationData(Mesh* mesh,aiAnimation* anim){
    m_Mesh = mesh;
    m_TicksPerSecond = anim->mTicksPerSecond;
    m_DurationInTicks = anim->mDuration;
    for(uint i = 0; i < anim->mNumChannels; i++){
        std::string key = (anim->mChannels[i]->mNodeName.data);
        m_KeyframeData.emplace(key,anim->mChannels[i]);
    }
}
AnimationData::~AnimationData(){
    
}
uint AnimationData::_FindPosition(float AnimationTime, const aiNodeAnim* node){    
    for (uint i = 0 ; i < node->mNumPositionKeys - 1 ; i++) {
        if (AnimationTime < (float)node->mPositionKeys[i + 1].mTime) { return i; }
    }  
    return 0;
}
uint AnimationData::_FindRotation(float AnimationTime, const aiNodeAnim* node){
    for (uint i = 0 ; i < node->mNumRotationKeys - 1 ; i++) {
        if (AnimationTime < (float)node->mRotationKeys[i + 1].mTime) { return i; }
    }   
    return 0;
}
uint AnimationData::_FindScaling(float AnimationTime, const aiNodeAnim* node){  
    for (uint i = 0 ; i < node->mNumScalingKeys - 1 ; i++) {
        if (AnimationTime < (float)node->mScalingKeys[i + 1].mTime) { return i; }
    }
    return 0;
}
void AnimationData::_CalcInterpolatedPosition(glm::vec3& Out, float AnimationTime, const aiNodeAnim* node){
    if (node->mNumPositionKeys == 1) {
        Out = Engine::Math::assimpToGLMVec3(node->mPositionKeys[0].mValue);
        return;
    }           
    uint PositionIndex = _FindPosition(AnimationTime,node);
    uint NextPositionIndex = (PositionIndex + 1);
    //assert(NextPositionIndex < node->mNumPositionKeys);
    float DeltaTime = (float)(node->mPositionKeys[NextPositionIndex].mTime - node->mPositionKeys[PositionIndex].mTime);
    float Factor = (AnimationTime - (float)node->mPositionKeys[PositionIndex].mTime) / DeltaTime;
    //assert(Factor >= 0.0f && Factor <= 1.0f);
    glm::vec3 Start = Engine::Math::assimpToGLMVec3(node->mPositionKeys[PositionIndex].mValue);
    glm::vec3 End = Engine::Math::assimpToGLMVec3(node->mPositionKeys[NextPositionIndex].mValue);
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
    //assert(NextRotationIndex < node->mNumRotationKeys);
    float DeltaTime = (float)(node->mRotationKeys[NextRotationIndex].mTime - node->mRotationKeys[RotationIndex].mTime);
    float Factor = (AnimationTime - (float)node->mRotationKeys[RotationIndex].mTime) / DeltaTime;
    //assert(Factor >= 0.0f && Factor <= 1.0f);
    const aiQuaternion& StartRotationQ = node->mRotationKeys[RotationIndex].mValue;
    const aiQuaternion& EndRotationQ   = node->mRotationKeys[NextRotationIndex].mValue;    
    aiQuaternion::Interpolate(Out, StartRotationQ, EndRotationQ, Factor);
    Out = Out.Normalize();
}
void AnimationData::_CalcInterpolatedScaling(glm::vec3& Out, float AnimationTime, const aiNodeAnim* node){
    if (node->mNumScalingKeys == 1) {
        Out = Engine::Math::assimpToGLMVec3(node->mScalingKeys[0].mValue);
        return;
    }
    uint ScalingIndex = _FindScaling(AnimationTime, node);
    uint NextScalingIndex = (ScalingIndex + 1);
    //assert(NextScalingIndex < node->mNumScalingKeys);
    float DeltaTime = (float)(node->mScalingKeys[NextScalingIndex].mTime - node->mScalingKeys[ScalingIndex].mTime);
    float Factor = (AnimationTime - (float)node->mScalingKeys[ScalingIndex].mTime) / DeltaTime;
    //assert(Factor >= 0.0f && Factor <= 1.0f);
    glm::vec3 Start = Engine::Math::assimpToGLMVec3(node->mScalingKeys[ScalingIndex].mValue);
    glm::vec3 End   = Engine::Math::assimpToGLMVec3(node->mScalingKeys[NextScalingIndex].mValue);
    glm::vec3 Delta = End - Start;
    Out = Start + Factor * Delta;
}
void AnimationData::_ReadNodeHeirarchy(const std::string& animationName,float time, const aiNode* n, glm::mat4& ParentTransform,std::vector<glm::mat4>& Transforms){    
    std::string BoneName(n->mName.data);
    glm::mat4 NodeTransform = Engine::Math::assimpToGLMMat4(const_cast<aiMatrix4x4&>(n->mTransformation));
    if(m_KeyframeData.count(BoneName)){
        const aiNodeAnim* keyframes = m_KeyframeData.at(BoneName);
        if(keyframes){
            glm::vec3 s;    _CalcInterpolatedScaling(s, time, keyframes);
        
            aiQuaternion q; _CalcInterpolatedRotation(q, time, keyframes);
            glm::mat4 rotation = glm::mat4(Engine::Math::assimpToGLMMat3(q.GetMatrix()));

            glm::vec3 t;    _CalcInterpolatedPosition(t, time, keyframes);

            NodeTransform = glm::mat4(1.0f);
            NodeTransform = glm::translate(NodeTransform,t);
            NodeTransform *= rotation;
            NodeTransform = glm::scale(NodeTransform,s);
        }
    }
    glm::mat4 Transform = ParentTransform * NodeTransform;
    if(m_Mesh->m_Skeleton->m_BoneMapping.count(BoneName)){
        uint BoneIndex = m_Mesh->m_Skeleton->m_BoneMapping.at(BoneName);
        glm::mat4& Final = m_Mesh->m_Skeleton->m_BoneInfo.at(BoneIndex).FinalTransform;
        Final = m_Mesh->m_Skeleton->m_GlobalInverseTransform * Transform * m_Mesh->m_Skeleton->m_BoneInfo.at(BoneIndex).BoneOffset;

        //this line allows for animation combinations. only works when additional animations start off in their resting places...
        Final = Transforms.at(BoneIndex) * Final;
    }
    for(uint i = 0; i < n->mNumChildren; i++){
        _ReadNodeHeirarchy(animationName,time,n->mChildren[i],Transform,Transforms);
    }
}
void AnimationData::_BoneTransform(const std::string& animationName,float TimeInSeconds, std::vector<glm::mat4>& Transforms){   
    float TicksPerSecond = float(m_TicksPerSecond != 0 ? m_TicksPerSecond : 25.0f);
    float TimeInTicks = TimeInSeconds * TicksPerSecond;
    float AnimationTime = float(fmod(TimeInTicks, m_DurationInTicks));
    glm::mat4 Identity = glm::mat4(1.0f);
    _ReadNodeHeirarchy(animationName,AnimationTime, m_Mesh->m_aiScene->mRootNode, Identity,Transforms);

    for(uint i = 0; i < m_Mesh->m_Skeleton->m_NumBones; i++){
        Transforms.at(i) = m_Mesh->m_Skeleton->m_BoneInfo.at(i).FinalTransform;
    }
}
float AnimationData::duration(){
    float TicksPerSecond = float(m_TicksPerSecond != 0 ? m_TicksPerSecond : 25.0f);
    return float(float(m_DurationInTicks) / TicksPerSecond);
}
MeshSkeleton::MeshSkeleton(){
    clear();
}
MeshSkeleton::MeshSkeleton(ImportedMeshData& data){
    fill(data);
}
void MeshSkeleton::fill(ImportedMeshData& data){
    for(auto bone:data.m_Bones){
        VertexBoneData& b = bone.second;
        m_BoneIDs    .push_back(glm::vec4(b.IDs[0],    b.IDs[1],    b.IDs[2],    b.IDs[3]));
        m_BoneWeights.push_back(glm::vec4(b.Weights[0],b.Weights[1],b.Weights[2],b.Weights[3]));
    }
}
void MeshSkeleton::clear(){
    for(auto animationData : m_AnimationData){
        delete animationData.second;
    }
    m_AnimationData.clear();
    m_NumBones = 0;
    m_BoneMapping.clear();
    m_BoneInfo.clear();
    m_BoneIDs.clear();
    m_BoneWeights.clear();
}
MeshSkeleton::~MeshSkeleton(){
    clear();
}
