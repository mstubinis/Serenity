#include "GameSkybox.h"
#include "Engine_Resources.h"
#include "Mesh.h"
#include "Camera.h"
#include "Texture.h"
#include "ShaderProgram.h"
#include "Engine_Math.h"
#include "Engine_Renderer.h"
#include "Engine_Window.h"

#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace Engine;

GameSkybox::GameSkybox(std::string name, unsigned int numFlares, Scene* scene):Skybox(name,scene){
    if(numFlares > 0){
        for(unsigned int i = 0; i < numFlares; i++){
            SkyboxSunFlare flare;

            float x = (((rand() % 100) - 50.0f) / 100.0f); x*=999999;
            float y = (((rand() % 100) - 50.0f) / 100.0f); y*=999999;
            float z = (((rand() % 100) - 50.0f) / 100.0f); z*=999999;

            float r = (rand() % 100) / 100.0f;
            float g = (rand() % 100) / 100.0f;
            float b = (rand() % 100) / 100.0f;

            float scl = ((rand() % 100)) / 300.0f; scl += 0.25;

            flare.position = glm::vec3(x,y,z);
            flare.scale = scl;
            flare.color = glm::vec3(r,g,b);
            m_SunFlares.push_back(flare);
        }
    }
}
GameSkybox::~GameSkybox(){
    m_SunFlares.clear();
}
void GameSkybox::update(){
    Skybox::update();
}
void GameSkybox::render(bool godsRays){
    Skybox::render(godsRays);
    if(m_SunFlares.size() > 0){
        glEnablei(GL_BLEND,0);
        glDisable(GL_DEPTH_TEST);
        glBlendFunci(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA,0);

        Texture* texture = Resources::getTexture("data/Textures/Skyboxes/StarFlare.png");
        GLuint shader = Resources::getShader("Deferred_HUD")->program();
        glUseProgram(shader);
        for(auto flare:m_SunFlares){
            glm::vec3 pos = Math::getScreenCoordinates(glm::vec3(Resources::getActiveCamera()->getPosition()) - flare.position,false);
            glm::vec4 col = glm::vec4(flare.color.x,flare.color.y,flare.color.z,1);
            glm::vec2 scl = glm::vec2(flare.scale,flare.scale);

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, texture->address());
            glUniform1i(glGetUniformLocation(shader,"DiffuseMap"),0);
            glUniform1i(glGetUniformLocation(shader,"DiffuseMapEnabled"),1);

            glUniform1i(glGetUniformLocation(shader,"Shadeless"),1);
            glUniform4f(glGetUniformLocation(shader, "Object_Color"),col.x,col.y,col.z,1);

            glm::mat4 model = glm::mat4(1);
            model = glm::translate(model, glm::vec3(pos.x,
                                                    Resources::getWindowSize().y-pos.y,
                                                    -0.5 - 1));
            model = glm::rotate(model,0.0f,glm::vec3(0,0,1));
            model = glm::scale(model, glm::vec3(texture->width(),texture->height(),1));
            model = glm::scale(model, glm::vec3(scl.x,scl.y,1));
            glm::mat4 world = Engine::Renderer::Detail::RenderManagement::m_2DProjectionMatrix * model; //we dont want the view matrix as we want to assume this "World" matrix originates from (0,0,0)

            glUniformMatrix4fv(glGetUniformLocation(shader, "MVP"), 1, GL_FALSE, glm::value_ptr(world));
            glUniformMatrix4fv(glGetUniformLocation(shader, "Model"), 1, GL_FALSE, glm::value_ptr(model));

            Resources::getMesh("Plane")->render();
        }
        glUseProgram(0);
        glEnable(GL_DEPTH_TEST);
        glDisable(GL_BLEND);
    }
}