#include "Light.h"
#include "Engine_Renderer.h"
#include "Engine_Resources.h"
#include "ShaderProgram.h"
#include "Camera.h"
#include "Mesh.h"
#include "Scene.h"

#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/make_shared.hpp>

using namespace Engine;

SunLight::SunLight(glm::v3 pos,std::string name,unsigned int type,Scene* scene):ObjectDisplay("","",pos,glm::vec3(1),name,scene){
    m_Type = type;
	m_Active = true;
    m_AmbientIntensity = 0.05f;
    m_DiffuseIntensity = 1.0f;
    m_SpecularIntensity = 1.0f;

    if(scene == nullptr){
        scene = Resources::getCurrentScene();
    }
    scene->getLights()[m_Name] = this;
}
SunLight::~SunLight(){
}
void SunLight::setName(std::string name){
    if(name == m_Name) return;

    std::string oldName = m_Name; m_Name = name;

    Resources::Detail::ResourceManagement::m_CurrentScene->getLights()[name] = this;
    if(Resources::Detail::ResourceManagement::m_CurrentScene->getLights().count(oldName)){
        Resources::Detail::ResourceManagement::m_CurrentScene->getLights().erase(oldName);
    }
    Resources::Detail::ResourceManagement::m_Objects[name] = boost::shared_ptr<Object>(this);
    if(Resources::Detail::ResourceManagement::m_Objects.count(oldName)){
        Resources::Detail::ResourceManagement::m_Objects[oldName].reset();
        Resources::Detail::ResourceManagement::m_Objects.erase(oldName);
    }
}
void SunLight::update(float dt){
    ObjectBasic::update(dt);
}
void SunLight::render(GLuint shader,bool debug){ 
	ObjectDisplay::render(shader,debug); 
}
void SunLight::draw(GLuint shader, bool debug,bool godsRays){ 
	ObjectDisplay::draw(shader,debug,godsRays); 
}
void SunLight::sendGenericAttributesToShader(GLuint shader){
    glUniform1i(glGetUniformLocation(shader,"LightType"), int(m_Type));
    glUniform3f(glGetUniformLocation(shader,"LightColor"), m_Color.x, m_Color.y, m_Color.z);
    glUniform1f(glGetUniformLocation(shader,"LightAmbientIntensity"), m_AmbientIntensity);
    glUniform1f(glGetUniformLocation(shader,"LightDiffuseIntensity"), m_DiffuseIntensity);
    glUniform1f(glGetUniformLocation(shader,"LightSpecularIntensity"), m_SpecularIntensity);
}
void SunLight::lighten(GLuint shader){
	if(!m_Active) return;
    sendGenericAttributesToShader(shader);
    glm::vec3 pos = glm::vec3(getPosition());
    glUniform3f(glGetUniformLocation(shader,"LightPosition"), pos.x, pos.y, pos.z);

    glm::vec3 campos = glm::vec3(Resources::getActiveCamera()->getPosition());
    glUniform3f(glGetUniformLocation(shader,"gCameraPosition"), campos.x, campos.y, campos.z);

    Renderer::Detail::renderFullscreenQuad(shader,Resources::getWindowSize().x,Resources::getWindowSize().y);
}
DirectionalLight::DirectionalLight(std::string name, glm::vec3 dir,Scene* scene): SunLight(glm::v3(0),name,LIGHT_TYPE_DIRECTIONAL,scene){
    m_Direction = glm::normalize(dir);
}
DirectionalLight::~DirectionalLight(){
}
void DirectionalLight::lighten(GLuint shader){
	if(!m_Active) return;
    sendGenericAttributesToShader(shader);
    glUniform3f(glGetUniformLocation(shader,"LightDirection"), m_Direction.x, m_Direction.y,m_Direction.z);
    Renderer::Detail::renderFullscreenQuad(shader,Resources::getWindowSize().x,Resources::getWindowSize().y);
}

PointLight::PointLight(std::string name, glm::v3 pos,Scene* scene): SunLight(pos,name,LIGHT_TYPE_POINT,scene){

	if(!Resources::Detail::ResourceManagement::m_Meshes.count("PointLightBounds")){
		#pragma region MeshData
		std::string data =  "v 0.000000 -0.138668 0.000000\n"
							"v 0.028175 -0.134224 0.020470\n"
							"v -0.010761 -0.134224 0.033121\n"
							"v 0.100341 -0.062015 0.072901\n"
							"v 0.084525 -0.091177 0.061410\n"
							"v 0.112700 -0.069653 0.040940\n"
							"v -0.034826 -0.134224 0.000000\n"
							"v -0.010761 -0.134224 -0.033121\n"
							"v 0.028175 -0.134224 -0.020470\n"
							"v 0.119351 -0.034826 0.061410\n"
							"v -0.038326 -0.062015 0.117958\n"
							"v -0.004110 -0.069653 0.119835\n"
							"v -0.021523 -0.034827 0.132487\n"
							"v -0.124028 -0.062015 0.000000\n"
							"v -0.115240 -0.069653 0.033121\n"
							"v -0.132653 -0.034826 0.020470\n"
							"v -0.038326 -0.062015 -0.117958\n"
							"v -0.067111 -0.069653 -0.099365\n"
							"v -0.060460 -0.034827 -0.119835\n"
							"v 0.100341 -0.062015 -0.072901\n"
							"v 0.073763 -0.069653 -0.094532\n"
							"v 0.095287 -0.034827 -0.094532\n"
							"v 0.095287 -0.034827 0.094532\n"
							"v -0.060460 -0.034827 0.119836\n"
							"v -0.132653 -0.034826 -0.020470\n"
							"v -0.021523 -0.034827 -0.132487\n"
							"v 0.119351 -0.034826 -0.061410\n"
							"v 0.038326 0.062015 0.117958\n"
							"v 0.067111 0.069653 0.099365\n"
							"v 0.032285 0.091177 0.099365\n"
							"v -0.100341 0.062015 0.072901\n"
							"v -0.073763 0.069653 0.094532\n"
							"v -0.084525 0.091177 0.061410\n"
							"v -0.100341 0.062015 -0.072901\n"
							"v -0.112700 0.069653 -0.040940\n"
							"v -0.084525 0.091177 -0.061410\n"
							"v 0.038326 0.062015 -0.117958\n"
							"v 0.004110 0.069654 -0.119835\n"
							"v 0.032285 0.091177 -0.099364\n"
							"v 0.124029 0.062015 0.000000\n"
							"v 0.115240 0.069653 -0.033121\n"
							"v 0.104479 0.091177 0.000000\n"
							"v -0.032285 -0.091177 0.099365\n"
							"v -0.022527 -0.117958 0.069333\n"
							"v 0.007320 -0.100342 0.095430\n"
							"v 0.019164 -0.124029 0.058979\n"
							"v 0.036452 -0.072903 0.112184\n"
							"v 0.050171 -0.100342 0.081506\n"
							"v 0.073763 -0.069653 0.094532\n"
							"v 0.058979 -0.117958 0.042850\n"
							"v 0.112700 -0.069653 -0.040940\n"
							"v 0.084525 -0.091177 -0.061410\n"
							"v 0.117958 -0.072903 0.000000\n"
							"v 0.093021 -0.100342 -0.022528\n"
							"v 0.093021 -0.100342 0.022528\n"
							"v 0.058979 -0.117958 -0.042850\n"
							"v 0.062014 -0.124029 0.000000\n"
							"v -0.104478 -0.091176 0.000000\n"
							"v -0.072902 -0.117958 0.000000\n"
							"v -0.088497 -0.100341 0.036451\n"
							"v -0.050170 -0.124029 0.036451\n"
							"v -0.095430 -0.072903 0.069334\n"
							"v -0.062014 -0.100342 0.072902\n"
							"v -0.067111 -0.069653 0.099365\n"
							"v -0.032285 -0.091177 -0.099364\n"
							"v -0.022527 -0.117958 -0.069333\n"
							"v -0.062014 -0.100342 -0.072902\n"
							"v -0.050170 -0.124029 -0.036451\n"
							"v -0.095430 -0.072903 -0.069334\n"
							"v -0.088497 -0.100341 -0.036451\n"
							"v -0.115240 -0.069653 -0.033121\n"
							"v 0.050171 -0.100342 -0.081506\n"
							"v 0.019164 -0.124029 -0.058979\n"
							"v 0.036452 -0.072903 -0.112184\n"
							"v 0.007320 -0.100342 -0.095429\n"
							"v -0.004110 -0.069653 -0.119835\n"
							"v 0.132654 0.034826 0.020470\n"
							"v 0.132654 0.034826 -0.020470\n"
							"v 0.131882 0.000000 0.042850\n"
							"v 0.138668 0.000000 0.000000\n"
							"v 0.131348 -0.038327 0.022528\n"
							"v 0.131882 0.000000 -0.042850\n"
							"v 0.131348 -0.038327 -0.022528\n"
							"v 0.021523 0.034827 0.132487\n"
							"v 0.060461 0.034827 0.119836\n"
							"v 0.000000 0.000000 0.138668\n"
							"v 0.042851 0.000000 0.131881\n"
							"v 0.019164 -0.038328 0.131881\n"
							"v 0.081507 0.000000 0.112185\n"
							"v 0.062015 -0.038328 0.117958\n"
							"v -0.119351 0.034827 0.061410\n"
							"v -0.095287 0.034827 0.094532\n"
							"v -0.131881 0.000000 0.042850\n"
							"v -0.112185 0.000000 0.081507\n"
							"v -0.119505 -0.038327 0.058979\n"
							"v -0.081507 0.000000 0.112185\n"
							"v -0.093021 -0.038327 0.095430\n"
							"v -0.095287 0.034827 -0.094532\n"
							"v -0.119351 0.034827 -0.061410\n"
							"v -0.081507 0.000000 -0.112185\n"
							"v -0.112185 0.000000 -0.081507\n"
							"v -0.093021 -0.038327 -0.095430\n"
							"v -0.131881 0.000000 -0.042850\n"
							"v -0.119505 -0.038327 -0.058979\n"
							"v 0.060461 0.034827 -0.119835\n"
							"v 0.021523 0.034827 -0.132487\n"
							"v 0.081507 0.000000 -0.112185\n"
							"v 0.042851 0.000000 -0.131881\n"
							"v 0.062015 -0.038327 -0.117958\n"
							"v 0.000000 0.000000 -0.138668\n"
							"v 0.019164 -0.038327 -0.131881\n"
							"v 0.093021 0.038327 0.095430\n"
							"v 0.112185 -0.000000 0.081507\n"
							"v 0.095430 0.072903 0.069334\n"
							"v 0.119505 0.038327 0.058979\n"
							"v 0.115240 0.069653 0.033122\n"
							"v -0.062015 0.038327 0.117958\n"
							"v -0.042851 0.000000 0.131881\n"
							"v -0.036451 0.072903 0.112184\n"
							"v -0.019164 0.038327 0.131881\n"
							"v 0.004110 0.069654 0.119835\n"
							"v -0.131348 0.038328 -0.022528\n"
							"v -0.138668 0.000000 0.000000\n"
							"v -0.117958 0.072903 0.000000\n"
							"v -0.131348 0.038327 0.022528\n"
							"v -0.112700 0.069653 0.040940\n"
							"v -0.019164 0.038328 -0.131881\n"
							"v -0.042851 0.000000 -0.131881\n"
							"v -0.036451 0.072903 -0.112184\n"
							"v -0.062014 0.038328 -0.117958\n"
							"v -0.073763 0.069653 -0.094532\n"
							"v 0.119505 0.038328 -0.058979\n"
							"v 0.112185 0.000000 -0.081506\n"
							"v 0.095430 0.072903 -0.069334\n"
							"v 0.093021 0.038328 -0.095430\n"
							"v 0.067111 0.069653 -0.099365\n"
							"v 0.010762 0.134224 0.033122\n"
							"v 0.034826 0.134224 0.000000\n"
							"v 0.000000 0.138668 0.000000\n"
							"v 0.022527 0.117959 0.069333\n"
							"v 0.050170 0.124029 0.036451\n"
							"v 0.062014 0.100342 0.072902\n"
							"v 0.072902 0.117959 0.000000\n"
							"v 0.088497 0.100342 0.036451\n"
							"v -0.028175 0.134224 0.020470\n"
							"v -0.058979 0.117959 0.042850\n"
							"v -0.019164 0.124029 0.058978\n"
							"v -0.050171 0.100342 0.081506\n"
							"v -0.007320 0.100342 0.095429\n"
							"v -0.028175 0.134224 -0.020470\n"
							"v -0.058979 0.117959 -0.042850\n"
							"v -0.062014 0.124029 0.000000\n"
							"v -0.093021 0.100342 -0.022527\n"
							"v -0.093021 0.100342 0.022528\n"
							"v 0.010762 0.134224 -0.033121\n"
							"v 0.022527 0.117959 -0.069333\n"
							"v -0.019163 0.124029 -0.058978\n"
							"v -0.007320 0.100342 -0.095429\n"
							"v -0.050171 0.100342 -0.081506\n"
							"v 0.050170 0.124029 -0.036451\n"
							"v 0.088497 0.100342 -0.036451\n"
							"v 0.062014 0.100342 -0.072902\n"
							"f 1 2 3\n"
							"f 4 5 6\n"
							"f 1 3 7\n"
							"f 1 7 8\n"
							"f 1 8 9\n"
							"f 4 6 10\n"
							"f 11 12 13\n"
							"f 14 15 16\n"
							"f 17 18 19\n"
							"f 20 21 22\n"
							"f 4 10 23\n"
							"f 11 13 24\n"
							"f 14 16 25\n"
							"f 17 19 26\n"
							"f 20 22 27\n"
							"f 28 29 30\n"
							"f 31 32 33\n"
							"f 34 35 36\n"
							"f 37 38 39\n"
							"f 40 41 42\n"
							"f 43 12 11\n"
							"f 44 45 43\n"
							"f 3 46 44\n"
							"f 43 45 12\n"
							"f 45 47 12\n"
							"f 44 46 45\n"
							"f 46 48 45\n"
							"f 45 48 47\n"
							"f 48 49 47\n"
							"f 3 2 46\n"
							"f 2 50 46\n"
							"f 46 50 48\n"
							"f 50 5 48\n"
							"f 48 5 49\n"
							"f 5 4 49\n"
							"f 51 52 20\n"
							"f 53 54 51\n"
							"f 6 55 53\n"
							"f 51 54 52\n"
							"f 54 56 52\n"
							"f 53 55 54\n"
							"f 55 57 54\n"
							"f 54 57 56\n"
							"f 57 9 56\n"
							"f 6 5 55\n"
							"f 5 50 55\n"
							"f 55 50 57\n"
							"f 50 2 57\n"
							"f 57 2 9\n"
							"f 2 1 9\n"
							"f 58 15 14\n"
							"f 59 60 58\n"
							"f 7 61 59\n"
							"f 58 60 15\n"
							"f 60 62 15\n"
							"f 59 61 60\n"
							"f 61 63 60\n"
							"f 60 63 62\n"
							"f 63 64 62\n"
							"f 7 3 61\n"
							"f 3 44 61\n"
							"f 61 44 63\n"
							"f 44 43 63\n"
							"f 63 43 64\n"
							"f 43 11 64\n"
							"f 65 18 17\n"
							"f 66 67 65\n"
							"f 8 68 66\n"
							"f 65 67 18\n"
							"f 67 69 18\n"
							"f 66 68 67\n"
							"f 68 70 67\n"
							"f 67 70 69\n"
							"f 70 71 69\n"
							"f 8 7 68\n"
							"f 7 59 68\n"
							"f 68 59 70\n"
							"f 59 58 70\n"
							"f 70 58 71\n"
							"f 58 14 71\n"
							"f 52 21 20\n"
							"f 56 72 52\n"
							"f 9 73 56\n"
							"f 52 72 21\n"
							"f 72 74 21\n"
							"f 56 73 72\n"
							"f 73 75 72\n"
							"f 72 75 74\n"
							"f 75 76 74\n"
							"f 9 8 73\n"
							"f 8 66 73\n"
							"f 73 66 75\n"
							"f 66 65 75\n"
							"f 75 65 76\n"
							"f 65 17 76\n"
							"f 77 78 40\n"
							"f 79 80 77\n"
							"f 10 81 79\n"
							"f 77 80 78\n"
							"f 80 82 78\n"
							"f 79 81 80\n"
							"f 81 83 80\n"
							"f 80 83 82\n"
							"f 83 27 82\n"
							"f 10 6 81\n"
							"f 6 53 81\n"
							"f 81 53 83\n"
							"f 53 51 83\n"
							"f 83 51 27\n"
							"f 51 20 27\n"
							"f 84 85 28\n"
							"f 86 87 84\n"
							"f 13 88 86\n"
							"f 84 87 85\n"
							"f 87 89 85\n"
							"f 86 88 87\n"
							"f 88 90 87\n"
							"f 87 90 89\n"
							"f 90 23 89\n"
							"f 13 12 88\n"
							"f 12 47 88\n"
							"f 88 47 90\n"
							"f 47 49 90\n"
							"f 90 49 23\n"
							"f 49 4 23\n"
							"f 91 92 31\n"
							"f 93 94 91\n"
							"f 16 95 93\n"
							"f 91 94 92\n"
							"f 94 96 92\n"
							"f 93 95 94\n"
							"f 95 97 94\n"
							"f 94 97 96\n"
							"f 97 24 96\n"
							"f 16 15 95\n"
							"f 15 62 95\n"
							"f 95 62 97\n"
							"f 62 64 97\n"
							"f 97 64 24\n"
							"f 64 11 24\n"
							"f 98 99 34\n"
							"f 100 101 98\n"
							"f 19 102 100\n"
							"f 98 101 99\n"
							"f 101 103 99\n"
							"f 100 102 101\n"
							"f 102 104 101\n"
							"f 101 104 103\n"
							"f 104 25 103\n"
							"f 19 18 102\n"
							"f 18 69 102\n"
							"f 102 69 104\n"
							"f 69 71 104\n"
							"f 104 71 25\n"
							"f 71 14 25\n"
							"f 105 106 37\n"
							"f 107 108 105\n"
							"f 22 109 107\n"
							"f 105 108 106\n"
							"f 108 110 106\n"
							"f 107 109 108\n"
							"f 109 111 108\n"
							"f 108 111 110\n"
							"f 111 26 110\n"
							"f 22 21 109\n"
							"f 21 74 109\n"
							"f 109 74 111\n"
							"f 74 76 111\n"
							"f 111 76 26\n"
							"f 76 17 26\n"
							"f 85 29 28\n"
							"f 89 112 85\n"
							"f 23 113 89\n"
							"f 85 112 29\n"
							"f 112 114 29\n"
							"f 89 113 112\n"
							"f 113 115 112\n"
							"f 112 115 114\n"
							"f 115 116 114\n"
							"f 23 10 113\n"
							"f 10 79 113\n"
							"f 113 79 115\n"
							"f 79 77 115\n"
							"f 115 77 116\n"
							"f 77 40 116\n"
							"f 92 32 31\n"
							"f 96 117 92\n"
							"f 24 118 96\n"
							"f 92 117 32\n"
							"f 117 119 32\n"
							"f 96 118 117\n"
							"f 118 120 117\n"
							"f 117 120 119\n"
							"f 120 121 119\n"
							"f 24 13 118\n"
							"f 13 86 118\n"
							"f 118 86 120\n"
							"f 86 84 120\n"
							"f 120 84 121\n"
							"f 84 28 121\n"
							"f 99 35 34\n"
							"f 103 122 99\n"
							"f 25 123 103\n"
							"f 99 122 35\n"
							"f 122 124 35\n"
							"f 103 123 122\n"
							"f 123 125 122\n"
							"f 122 125 124\n"
							"f 125 126 124\n"
							"f 25 16 123\n"
							"f 16 93 123\n"
							"f 123 93 125\n"
							"f 93 91 125\n"
							"f 125 91 126\n"
							"f 91 31 126\n"
							"f 106 38 37\n"
							"f 110 127 106\n"
							"f 26 128 110\n"
							"f 106 127 38\n"
							"f 127 129 38\n"
							"f 110 128 127\n"
							"f 128 130 127\n"
							"f 127 130 129\n"
							"f 130 131 129\n"
							"f 26 19 128\n"
							"f 19 100 128\n"
							"f 128 100 130\n"
							"f 100 98 130\n"
							"f 130 98 131\n"
							"f 98 34 131\n"
							"f 78 41 40\n"
							"f 82 132 78\n"
							"f 27 133 82\n"
							"f 78 132 41\n"
							"f 132 134 41\n"
							"f 82 133 132\n"
							"f 133 135 132\n"
							"f 132 135 134\n"
							"f 135 136 134\n"
							"f 27 22 133\n"
							"f 22 107 133\n"
							"f 133 107 135\n"
							"f 107 105 135\n"
							"f 135 105 136\n"
							"f 105 37 136\n"
							"f 137 138 139\n"
							"f 140 141 137\n"
							"f 30 142 140\n"
							"f 137 141 138\n"
							"f 141 143 138\n"
							"f 140 142 141\n"
							"f 142 144 141\n"
							"f 141 144 143\n"
							"f 144 42 143\n"
							"f 30 29 142\n"
							"f 29 114 142\n"
							"f 142 114 144\n"
							"f 114 116 144\n"
							"f 144 116 42\n"
							"f 116 40 42\n"
							"f 145 137 139\n"
							"f 146 147 145\n"
							"f 33 148 146\n"
							"f 145 147 137\n"
							"f 147 140 137\n"
							"f 146 148 147\n"
							"f 148 149 147\n"
							"f 147 149 140\n"
							"f 149 30 140\n"
							"f 33 32 148\n"
							"f 32 119 148\n"
							"f 148 119 149\n"
							"f 119 121 149\n"
							"f 149 121 30\n"
							"f 121 28 30\n"
							"f 150 145 139\n"
							"f 151 152 150\n"
							"f 36 153 151\n"
							"f 150 152 145\n"
							"f 152 146 145\n"
							"f 151 153 152\n"
							"f 153 154 152\n"
							"f 152 154 146\n"
							"f 154 33 146\n"
							"f 36 35 153\n"
							"f 35 124 153\n"
							"f 153 124 154\n"
							"f 124 126 154\n"
							"f 154 126 33\n"
							"f 126 31 33\n"
							"f 155 150 139\n"
							"f 156 157 155\n"
							"f 39 158 156\n"
							"f 155 157 150\n"
							"f 157 151 150\n"
							"f 156 158 157\n"
							"f 158 159 157\n"
							"f 157 159 151\n"
							"f 159 36 151\n"
							"f 39 38 158\n"
							"f 38 129 158\n"
							"f 158 129 159\n"
							"f 129 131 159\n"
							"f 159 131 36\n"
							"f 131 34 36\n"
							"f 138 155 139\n"
							"f 143 160 138\n"
							"f 42 161 143\n"
							"f 138 160 155\n"
							"f 160 156 155\n"
							"f 143 161 160\n"
							"f 161 162 160\n"
							"f 160 162 156\n"
							"f 162 39 156\n"
							"f 42 41 161\n"
							"f 41 134 161\n"
							"f 161 134 162\n"
							"f 134 136 162\n"
							"f 162 136 39\n"
							"f 136 37 39\n";
		#pragma endregion
		Resources::Detail::ResourceManagement::m_Meshes["PointLightBounds"] = boost::make_shared<Mesh>(data,COLLISION_TYPE_NONE,false);
	}

    m_Constant = 0.3f;
    m_Linear = 0.2f;
    m_Exp = 0.3f;
    m_PointLightRadius = calculatePointLightRadius();
}
PointLight::~PointLight(){
}
float PointLight::calculatePointLightRadius(){
    float MaxChannel = glm::max(glm::max(m_Color.x, m_Color.y), m_Color.z);
    float ret = (-m_Linear + glm::sqrt(m_Linear * m_Linear -
        4 * m_Exp * (m_Exp - 256 * MaxChannel * m_DiffuseIntensity))) / (2 * m_Exp);
    return ret;
}
void PointLight::setConstant(float c){ 
    m_Constant = c;
    m_PointLightRadius = calculatePointLightRadius();
}
void PointLight::setLinear(float l){ 
    m_Linear = l;
    m_PointLightRadius = calculatePointLightRadius();
}
void PointLight::setExponent(float e){ 
    m_Exp = e;
    m_PointLightRadius = calculatePointLightRadius();
}
void PointLight::lighten(GLuint shader){
	if(!m_Active) return;

    Camera* camera = Resources::getActiveCamera();
    glm::v3 pos = getPosition();
    if((!camera->sphereIntersectTest(pos,m_PointLightRadius)) || (camera->getDistance(this) > 1100 * m_PointLightRadius))
        return;
    sendGenericAttributesToShader(shader);
    glUniform3f(glGetUniformLocation(shader,"LightPosition"),float(pos.x),float(pos.y),float(pos.z));

    glm::vec3 campos = glm::vec3(Resources::getActiveCamera()->getPosition());
    glUniform3f(glGetUniformLocation(shader,"gCameraPosition"), campos.x, campos.y, campos.z);

    glUniform1f(glGetUniformLocation(shader,"LightConstant"), m_Constant);
    glUniform1f(glGetUniformLocation(shader,"LightLinear"), m_Linear);
    glUniform1f(glGetUniformLocation(shader,"LightExp"), m_Exp);

    this->m_Orientation = Resources::getActiveCamera()->getOrientation();

    glm::mat4 m(1);
    m = glm::translate(m,glm::vec3(pos));
    m *= glm::mat4_cast(m_Orientation);
    m = glm::scale(m,glm::vec3(m_PointLightRadius));

    glUniformMatrix4fv(glGetUniformLocation(shader, "VP" ), 1, GL_FALSE, glm::value_ptr(Resources::getActiveCamera()->getViewProjection()));
    glUniformMatrix4fv(glGetUniformLocation(shader, "Model" ), 1, GL_FALSE, glm::value_ptr(m));

    if(glm::distance(campos,glm::vec3(pos)) > m_PointLightRadius){
        glCullFace(GL_BACK);
	}
    else{
        glCullFace(GL_FRONT);
    }
    Resources::getMesh("PointLightBounds")->render(); //this can bug out if we pass in custom uv's like in the renderQuad method

    glCullFace(GL_BACK);
}
SpotLight::SpotLight(std::string name, glm::v3 pos,Scene* scene): SunLight(pos,name,LIGHT_TYPE_SPOT){
    m_Direction = glm::vec3(0,0,-1);
    m_Cutoff = 0;
}
SpotLight::~SpotLight(){
}
void SpotLight::lighten(GLuint shader){
	if(!m_Active) return;
    sendGenericAttributesToShader(shader);
}