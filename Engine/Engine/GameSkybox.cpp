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

GameSkybox::GameSkybox(std::string name, uint numFlares, Scene* scene):Skybox(name,scene){
    if(numFlares > 0){
        for(uint i = 0; i < numFlares; i++){
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
        GLuint shader = Resources::getShaderProgram("Deferred_HUD")->program();
        glUseProgram(shader);

        for(auto flare:m_SunFlares){
            glm::vec3 pos = Math::getScreenCoordinates(glm::vec3(Resources::getActiveCamera()->getPosition()) - flare.position,false);
            glm::vec4 col = glm::vec4(flare.color.x,flare.color.y,flare.color.z,1);
            glm::vec2 scl = glm::vec2(flare.scale,flare.scale);

			texture->render(glm::vec2(pos.x,pos.y),col,0,scl,0.5f);
        }
        glUseProgram(0);
        glEnable(GL_DEPTH_TEST);
        glDisable(GL_BLEND);
    }
}