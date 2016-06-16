#include "Skybox.h"
#include "Engine_Resources.h"
#include "ShaderProgram.h"
#include "Camera.h"
#include "Mesh.h"
#include "Texture.h"
#include "Scene.h"

using namespace Engine;

GLuint Skybox::m_Buffer;
std::vector<glm::vec3> Skybox::m_Vertices;

SkyboxEmpty::SkyboxEmpty(std::string name,Scene* scene){
    m_Model = glm::mat4(1);
    if(scene == nullptr) scene = Resources::getCurrentScene();
    if(scene->getSkybox() == nullptr)
        scene->setSkybox(this);
}
SkyboxEmpty::~SkyboxEmpty(){

}
        

Skybox::Skybox(std::string name,Scene* scene):SkyboxEmpty(name,scene){
    if(Skybox::m_Vertices.size() == 0){
        std::vector<glm::vec3> temp;
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

        for(unsigned int i = 0; i < 6; i++){
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

    std::string front = name + "/Right.jpg";
    std::string back = name + "/Left.jpg";
    std::string left = name + "/Top.jpg";
    std::string right = name + "/Bottom.jpg";
    std::string top = name + "/Front.jpg";
    std::string bottom = name + "/Back.jpg";
    std::string names[6] = {front,back,left,right,top,bottom};

    m_Texture = new Texture(names,"Cubemap ",GL_TEXTURE_CUBE_MAP);

    m_Model = glm::mat4(1);
    m_Model = glm::translate(m_Model, glm::vec3(Resources::getActiveCamera()->getPosition()));
    m_Model = glm::scale(m_Model,glm::vec3(999999,999999,999999));
}
Skybox::~Skybox(){
}
void Skybox::update(){
    glm::vec3 p = glm::vec3(Resources::getActiveCamera()->getPosition());
    m_Model[3][0] = p.x;
    m_Model[3][1] = p.y;
    m_Model[3][2] = p.z;
}

void Skybox::render(bool godsRays){
    GLuint shader = Resources::getShader("Deferred_Skybox")->program();
    glUseProgram(shader);

    glUniformMatrix4fv(glGetUniformLocation(shader, "VP" ), 1, GL_FALSE, glm::value_ptr(Resources::getActiveCamera()->getViewProjection()));
    glUniformMatrix4fv(glGetUniformLocation(shader, "Model" ), 1, GL_FALSE, glm::value_ptr(m_Model));

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_Texture->address());
    glUniform1i(glGetUniformLocation(shader, "Texture"), 0);

	if(godsRays == true){
		glUniform1i(glGetUniformLocation(shader, "HasGodsRays"), 1);
	}
	else{
		glUniform1i(glGetUniformLocation(shader, "HasGodsRays"), 0);
	}

    glBindBuffer( GL_ARRAY_BUFFER, m_Buffer);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glDrawArrays(GL_TRIANGLES, 0, Skybox::m_Vertices.size());
    glDisableVertexAttribArray(0);
    glUseProgram(0);
}