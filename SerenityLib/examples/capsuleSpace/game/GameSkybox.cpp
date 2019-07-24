#include "GameSkybox.h"
#include "ResourceManifest.h"

#include <core/engine/math/Engine_Math.h>
#include <core/engine/renderer/Engine_Renderer.h>
#include <core/engine/Engine_Window.h>
#include <core/engine/resources/Engine_Resources.h>
#include <core/engine/mesh/Mesh.h>
#include <core/engine/scene/Camera.h>
#include <core/engine/textures/Texture.h>
#include <core/ShaderProgram.h>
#include <core/Material.h>
#include <core/engine/scene/Scene.h>

#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <glm/vec3.hpp>

using namespace Engine;
using namespace std;

struct SkyboxSunFlare final{
    glm::vec3 position;
    glm::vec3 color;
    float scale;
    SkyboxSunFlare(){ position = glm::vec3(0); color = glm::vec3(0); scale = 0;}
    ~SkyboxSunFlare(){ position = glm::vec3(0); color = glm::vec3(0); scale = 0;}
};

class GameSkybox::impl final{
    public:
        vector<SkyboxSunFlare> sunFlares;

        void _init(const uint& _numFlares){
            if(_numFlares > 0){
                for(uint i = 0; i < _numFlares; ++i){
                    SkyboxSunFlare flare;

                    float x = (((rand() % 100) - 50.0f) / 100.0f); x*=999999.0f;
                    float y = (((rand() % 100) - 50.0f) / 100.0f); y*=999999.0f;
                    float z = (((rand() % 100) - 50.0f) / 100.0f); z*=999999.0f;

                    float r = (rand() % 100) / 100.0f;
                    float g = (rand() % 100) / 100.0f;
                    float b = (rand() % 100) / 100.0f;

                    float scl = ((((rand() % 100)) / 300.0f) + 0.15f) * 0.25f;

                    flare.position = glm::vec3(x,y,z);
                    flare.scale = scl;
                    flare.color = glm::vec3(r,g,b);
                    sunFlares.push_back(flare);
                }
            }
        }
        void _destruct(){
            vector_clear(sunFlares);
        }
        void _draw(){
            //find a better way of doing this
            /*
            if(sunFlares.size() > 0){
                Material* mat = Resources::getMaterial(ResourceManifest::StarFlareMaterial);
                Texture* texture = mat->getComponent(MaterialComponentType::Diffuse)->texture();
                for(auto& flare:sunFlares){
                    //glm::vec3 pos = Math::getScreenCoordinates(glm::vec3(Resources::getCurrentScene()->getActiveCamera()->getPosition()) - flare.position,false);
                    glm::vec3 pos = Math::getScreenCoordinates(flare.position,false);
                    glm::vec4 col = glm::vec4(flare.color.x,flare.color.y,flare.color.z,1);
                    glm::vec2 scl = glm::vec2(flare.scale,flare.scale);
                    texture->render(glm::vec2(pos.x,pos.y),col,0,scl,0.1f);
                }
            }
            */
        }
};
GameSkybox::GameSkybox(const string& name, const uint& numFlares):Skybox(name),m_i(new impl){
    m_i->_init(numFlares); 
}
GameSkybox::GameSkybox(const string* names, const uint& numFlares):Skybox(names),m_i(new impl){
    m_i->_init(numFlares); 
}
GameSkybox::~GameSkybox(){ 
    m_i->_destruct(); 
}
void GameSkybox::update(){
    Skybox::update();
}
void GameSkybox::draw(){
    Skybox::draw();
    m_i->_draw();
}