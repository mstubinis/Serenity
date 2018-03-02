#include "GameSkybox.h"
#include "Engine_Resources.h"
#include "Mesh.h"
#include "Camera.h"
#include "Texture.h"
#include "ShaderProgram.h"
#include "Engine_Math.h"
#include "Engine_Renderer.h"
#include "Engine_Window.h"
#include "Scene.h"

#include "ResourceManifest.h"
#include "Material.h"

#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <glm/vec3.hpp>

using namespace Engine;
using namespace std;

GameSkybox::GameSkybox(string name, uint numFlares, Scene* scene):Skybox(name,scene){
    if(numFlares > 0){
        for(uint i = 0; i < numFlares; ++i){
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
void GameSkybox::draw(){
    Skybox::draw();
    if(m_SunFlares.size() > 0){
		Material* sunFlareMat = Resources::getMaterial(ResourceManifest::SunFlareMaterial);
		Texture* texture = sunFlareMat->getComponent(MaterialComponentType::Diffuse)->texture();
        for(auto flare:m_SunFlares){
            glm::vec3 pos = Math::getScreenCoordinates(glm::vec3(Resources::getCurrentScene()->getActiveCamera()->getPosition()) - flare.position,false);
            glm::vec4 col = glm::vec4(flare.color.x,flare.color.y,flare.color.z,1);
            glm::vec2 scl = glm::vec2(flare.scale,flare.scale);
            texture->render(glm::vec2(pos.x,pos.y),col,0,scl,0.5f);
        }
    }
}