#include "Skybox.h"
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

SkyboxEmpty::SkyboxEmpty(string name,Scene* scene){
    if(scene == nullptr) scene = Resources::getCurrentScene();
    if(scene->getSkybox() == nullptr)
        scene->setSkybox(this);
}
SkyboxEmpty::~SkyboxEmpty(){

}
        
Skybox::Skybox(string name,Scene* scene):SkyboxEmpty(name,scene){
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

        for(uint i = 0; i < 6; i++){
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
    glActiveTexture(GL_TEXTURE0);

    string front = name + "/Right.jpg";
    string back = name + "/Left.jpg";
    string left = name + "/Top.jpg";
    string right = name + "/Bottom.jpg";
    string top = name + "/Front.jpg";
    string bottom = name + "/Back.jpg";
    string names[6] = {front,back,left,right,top,bottom};

    m_Texture = new Texture(names,"Cubemap",GL_TEXTURE_CUBE_MAP);
}
Skybox::~Skybox(){
}
void Skybox::update(){
}
void Skybox::draw(){
    ShaderP* p = Resources::getShaderProgram("Deferred_Skybox"); p->bind();

    Camera* c = Resources::getActiveCamera();
    glm::mat4 view = glm::mat4(glm::mat3(c->getView()));
    Renderer::sendUniformMatrix4f("VP",c->getProjection() * view);

    Renderer::bindTexture("Texture",m_Texture,0);

    glBindBuffer( GL_ARRAY_BUFFER, m_Buffer);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glDrawArrays(GL_TRIANGLES, 0, Skybox::m_Vertices.size());
    glDisableVertexAttribArray(0);

    Renderer::unbindTextureCubemap(0);
    p->unbind();
}
