#include "Skybox.h"
#include "Engine.h"
#include "Engine_Resources.h"
#include "Engine_Renderer.h"
#include "ShaderProgram.h"
#include "Camera.h"
#include "Mesh.h"
#include "Texture.h"
#include "Scene.h"

#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace Engine;
using namespace std;

GLuint Skybox::m_Buffer;
vector<glm::vec3> Skybox::m_Vertices;

SkyboxEmpty::SkyboxEmpty(Scene* scene){
    if(scene == nullptr){
        scene = Resources::getCurrentScene();
    }
    if(scene->skybox() == nullptr){
        scene->setSkybox(this);
    }
}
SkyboxEmpty::~SkyboxEmpty(){

}   
Skybox::Skybox(string* files,Scene* scene):SkyboxEmpty(scene){
    Skybox::initMesh();
    glActiveTexture(GL_TEXTURE0);
	
    string names[6] = {files[0],files[1],files[2],files[3],files[4],files[5]};

	//instead of using files[0] generate a proper name using the directory?
    m_Texture = new Texture(names,files[0] + "Cubemap",false,ImageInternalFormat::SRGB8_ALPHA8);
	
    m_Texture->genPBREnvMapData(32,m_Texture->width() / 4);
    epriv::Core::m_Engine->m_ResourceManager->_addTexture(m_Texture);
}
Skybox::Skybox(string filename,Scene* scene):SkyboxEmpty(scene){
    Skybox::initMesh();
    glActiveTexture(GL_TEXTURE0);
	
	m_Texture = new Texture(filename,false,ImageInternalFormat::SRGB8_ALPHA8);

    m_Texture->genPBREnvMapData(32,m_Texture->width() / 4);
    epriv::Core::m_Engine->m_ResourceManager->_addTexture(m_Texture);
}
Skybox::~Skybox(){
}
void Skybox::initMesh(){
    if(Skybox::m_Vertices.size() == 0){
        vector<glm::vec3> temp;
        temp.push_back(glm::vec3(-1,1,1));//1
        temp.push_back(glm::vec3(1,1,1));//2
        temp.push_back(glm::vec3(1,-1,1));//3
        temp.push_back(glm::vec3(-1,-1,1));//4
        temp.push_back(glm::vec3(-1,1,-1));//5
        temp.push_back(glm::vec3(-1,-1,-1));//6
        temp.push_back(glm::vec3(1,-1,-1));//7
        temp.push_back(glm::vec3(1,1,-1));//8
        temp.push_back(glm::vec3(-1,1,1));//9
        temp.push_back(glm::vec3(-1,-1,1));//10
        temp.push_back(glm::vec3(-1,-1,-1));//11
        temp.push_back(glm::vec3(-1,1,-1));//12
        temp.push_back(glm::vec3(-1,-1,1));//13
        temp.push_back(glm::vec3(1,-1,1));//14
        temp.push_back(glm::vec3(1,-1 ,-1));//15
        temp.push_back(glm::vec3(-1,-1,-1));//16
        temp.push_back(glm::vec3(1,-1,1));//17
        temp.push_back(glm::vec3(1,1,1));//18
        temp.push_back(glm::vec3(1,1,-1));//19
        temp.push_back(glm::vec3(1,-1,-1));//20
        temp.push_back(glm::vec3(1,1,1));//21
        temp.push_back(glm::vec3(-1,1,1));//22
        temp.push_back(glm::vec3(-1,1,-1));//23
        temp.push_back(glm::vec3(1,1,-1));//24

        for(uint i = 0; i < 6; ++i){
            glm::vec3 v1,v2,v3,v4;
            v1 = temp[ 0 + (i*4) ];
            v2 = temp[ 1 + (i*4) ];
            v3 = temp[ 2 + (i*4) ];
            v4 = temp[ 3 + (i*4) ];

            Skybox::m_Vertices.push_back(v1);
            Skybox::m_Vertices.push_back(v2);
            Skybox::m_Vertices.push_back(v3);

            Skybox::m_Vertices.push_back(v1);
            Skybox::m_Vertices.push_back(v3);
            Skybox::m_Vertices.push_back(v4);
        }
        glGenBuffers(1, &Skybox::m_Buffer);
        glBindBuffer(GL_ARRAY_BUFFER, Skybox::m_Buffer );
        glBufferData(GL_ARRAY_BUFFER, Skybox::m_Vertices.size() * sizeof(glm::vec3),&Skybox::m_Vertices[0], GL_STATIC_DRAW );
    }
}
void Skybox::bindMesh(){
    glBindBuffer( GL_ARRAY_BUFFER, m_Buffer);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glDrawArrays(GL_TRIANGLES, 0, Skybox::m_Vertices.size());
    glDisableVertexAttribArray(0);
}
