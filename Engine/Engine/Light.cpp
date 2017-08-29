#include "Light.h"
#include "Engine_Renderer.h"
#include "Engine_Resources.h"
#include "ShaderProgram.h"
#include "Camera.h"
#include "Mesh.h"
#include "Scene.h"
#include "Skybox.h"
#include "Texture.h"
#include "GBuffer.h"
#include "FramebufferObject.h"

#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/make_shared.hpp>
#include <boost/tuple/tuple.hpp>

using namespace Engine;
using namespace std;

unordered_map<uint,boost::tuple<float,float,float>> _populateLightRanges(){
    unordered_map<uint,boost::tuple<float,float,float>> m;

    m[LightRange::_7] = boost::make_tuple(1.0f, 0.7f, 1.8f);
    m[LightRange::_13] = boost::make_tuple(1.0f, 0.35f, 0.44f);
    m[LightRange::_20] = boost::make_tuple(1.0f, 0.22f, 0.20f);
    m[LightRange::_32] = boost::make_tuple(1.0f, 0.14f, 0.07f);
    m[LightRange::_50] = boost::make_tuple(1.0f, 0.09f, 0.032f);
    m[LightRange::_65] = boost::make_tuple(1.0f, 0.07f, 0.017f);
    m[LightRange::_100] = boost::make_tuple(1.0f, 0.045f, 0.0075f);
    m[LightRange::_160] = boost::make_tuple(1.0f, 0.027f, 0.0028f);
    m[LightRange::_200] = boost::make_tuple(1.0f, 0.022f, 0.0019f);
    m[LightRange::_325] = boost::make_tuple(1.0f, 0.014f, 0.0007f);
    m[LightRange::_600] = boost::make_tuple(1.0f, 0.007f, 0.0002f);
    m[LightRange::_3250] = boost::make_tuple(1.0f, 0.0014f, 0.000007f);

    return m;
}
unordered_map<uint,boost::tuple<float,float,float>> LIGHT_RANGES = _populateLightRanges();

SunLight::SunLight(glm::vec3 pos,string n,uint type,Scene* scene):ObjectDisplay("","",pos,glm::vec3(1.0f),n,scene){
    m_Type = type;
    m_Active = true;
    m_AmbientIntensity = 0.005f;
    m_DiffuseIntensity = 2.0f;
    m_SpecularIntensity = 1.0f;
    m_Color = glm::vec4(1.0,1.0f,1.0f,1.0f);
    if(scene == nullptr){
        scene = Resources::getCurrentScene();
    }
    scene->lights().emplace(name(),this);
}
SunLight::~SunLight(){
}
void SunLight::sendGenericAttributesToShader(){
    Renderer::sendUniform4f("LightDataD",m_Color.x, m_Color.y, m_Color.z,float(m_Type));
}
void SunLight::lighten(){
    if(!m_Active) return;
    glm::vec3 pos = getPosition();
    sendGenericAttributesToShader();
    Renderer::sendUniform4f("LightDataA", m_AmbientIntensity,m_DiffuseIntensity,m_SpecularIntensity,0.0f);
    Renderer::sendUniform4f("LightDataC",0.0f,pos.x,pos.y,pos.z);
    Renderer::sendUniform1fSafe("SpotLight",0.0f);
    Renderer::Detail::renderFullscreenQuad(Resources::getWindowSize().x,Resources::getWindowSize().y);
}
float SunLight::getAmbientIntensity(){ return m_AmbientIntensity; }
void SunLight::setAmbientIntensity(float a){ m_AmbientIntensity = a; }
float SunLight::getDiffuseIntensity(){ return m_DiffuseIntensity; }
void SunLight::setDiffuseIntensity(float d){ m_DiffuseIntensity = d; }
float SunLight::getSpecularIntensity(){ return m_SpecularIntensity; }
void SunLight::setSpecularIntensity(float s){ m_SpecularIntensity = s; }
void SunLight::activate(bool b){ m_Active = b; }
void SunLight::deactivate(){ m_Active = false; }
bool SunLight::isActive(){ return m_Active; }
uint SunLight::type(){ return m_Type; }
DirectionalLight::DirectionalLight(string name, glm::vec3 dir,Scene* scene): SunLight(glm::vec3(0),name,LightType::Directional,scene){
    alignTo(dir,0);
    ObjectBasic::update(0);
    m_Color = glm::vec4(1.0,1.0f,1.0f,1.0f);
}
DirectionalLight::~DirectionalLight(){
}
void DirectionalLight::lighten(){
    if(!m_Active) return;
    sendGenericAttributesToShader();
    Renderer::sendUniform4f("LightDataA", m_AmbientIntensity,m_DiffuseIntensity,m_SpecularIntensity,m_Forward.x);
    Renderer::sendUniform4f("LightDataB", m_Forward.y,m_Forward.z,0.0f, 0.0f);
    Renderer::sendUniform1fSafe("SpotLight",0.0f);
    Renderer::Detail::renderFullscreenQuad(Resources::getWindowSize().x,Resources::getWindowSize().y);
}

PointLight::PointLight(string name, glm::vec3 pos,Scene* scene): SunLight(pos,name,LightType::Point,scene){
    if(Resources::getMesh("PointLightBounds") == nullptr){
        #pragma region MeshData
        string data = 
            "v 0.000000 -1.000000 0.000000\n"
            "v 0.723607 -0.447220 0.525725\n"
            "v -0.276388 -0.447220 0.850649\n"
            "v -0.894426 -0.447216 0.000000\n"
            "v -0.276388 -0.447220 -0.850649\n"
            "v 0.723607 -0.447220 -0.525725\n"
            "v 0.276388 0.447220 0.850649\n"
            "v -0.723607 0.447220 0.525725\n"
            "v -0.723607 0.447220 -0.525725\n"
            "v 0.276388 0.447220 -0.850649\n"
            "v 0.894426 0.447216 0.000000\n"
            "v 0.000000 1.000000 0.000000\n"
            "v -0.232822 -0.657519 0.716563\n"
            "v -0.162456 -0.850654 0.499995\n"
            "v -0.077607 -0.967950 0.238853\n"
            "v 0.203181 -0.967950 0.147618\n"
            "v 0.425323 -0.850654 0.309011\n"
            "v 0.609547 -0.657519 0.442856\n"
            "v 0.531941 -0.502302 0.681712\n"
            "v 0.262869 -0.525738 0.809012\n"
            "v -0.029639 -0.502302 0.864184\n"
            "v 0.812729 -0.502301 -0.295238\n"
            "v 0.850648 -0.525736 0.000000\n"
            "v 0.812729 -0.502301 0.295238\n"
            "v 0.203181 -0.967950 -0.147618\n"
            "v 0.425323 -0.850654 -0.309011\n"
            "v 0.609547 -0.657519 -0.442856\n"
            "v -0.753442 -0.657515 0.000000\n"
            "v -0.525730 -0.850652 0.000000\n"
            "v -0.251147 -0.967949 0.000000\n"
            "v -0.483971 -0.502302 0.716565\n"
            "v -0.688189 -0.525736 0.499997\n"
            "v -0.831051 -0.502299 0.238853\n"
            "v -0.232822 -0.657519 -0.716563\n"
            "v -0.162456 -0.850654 -0.499995\n"
            "v -0.077607 -0.967950 -0.238853\n"
            "v -0.831051 -0.502299 -0.238853\n"
            "v -0.688189 -0.525736 -0.499997\n"
            "v -0.483971 -0.502302 -0.716565\n"
            "v -0.029639 -0.502302 -0.864184\n"
            "v 0.262869 -0.525738 -0.809012\n"
            "v 0.531941 -0.502302 -0.681712\n"
            "v 0.956626 0.251149 0.147618\n"
            "v 0.951058 -0.000000 0.309013\n"
            "v 0.860698 -0.251151 0.442858\n"
            "v 0.860698 -0.251151 -0.442858\n"
            "v 0.951058 0.000000 -0.309013\n"
            "v 0.956626 0.251149 -0.147618\n"
            "v 0.155215 0.251152 0.955422\n"
            "v 0.000000 -0.000000 1.000000\n"
            "v -0.155215 -0.251152 0.955422\n"
            "v 0.687159 -0.251152 0.681715\n"
            "v 0.587786 0.000000 0.809017\n"
            "v 0.436007 0.251152 0.864188\n"
            "v -0.860698 0.251151 0.442858\n"
            "v -0.951058 -0.000000 0.309013\n"
            "v -0.956626 -0.251149 0.147618\n"
            "v -0.436007 -0.251152 0.864188\n"
            "v -0.587786 0.000000 0.809017\n"
            "v -0.687159 0.251152 0.681715\n"
            "v -0.687159 0.251152 -0.681715\n"
            "v -0.587786 -0.000000 -0.809017\n"
            "v -0.436007 -0.251152 -0.864188\n"
            "v -0.956626 -0.251149 -0.147618\n"
            "v -0.951058 0.000000 -0.309013\n"
            "v -0.860698 0.251151 -0.442858\n"
            "v 0.436007 0.251152 -0.864188\n"
            "v 0.587786 -0.000000 -0.809017\n"
            "v 0.687159 -0.251152 -0.681715\n"
            "v -0.155215 -0.251152 -0.955422\n"
            "v 0.000000 0.000000 -1.000000\n"
            "v 0.155215 0.251152 -0.955422\n"
            "v 0.831051 0.502299 0.238853\n"
            "v 0.688189 0.525736 0.499997\n"
            "v 0.483971 0.502302 0.716565\n"
            "v 0.029639 0.502302 0.864184\n"
            "v -0.262869 0.525738 0.809012\n"
            "v -0.531941 0.502302 0.681712\n"
            "v -0.812729 0.502301 0.295238\n"
            "v -0.850648 0.525736 0.000000\n"
            "v -0.812729 0.502301 -0.295238\n"
            "v -0.531941 0.502302 -0.681712\n"
            "v -0.262869 0.525738 -0.809012\n"
            "v 0.029639 0.502302 -0.864184\n"
            "v 0.483971 0.502302 -0.716565\n"
            "v 0.688189 0.525736 -0.499997\n"
            "v 0.831051 0.502299 -0.238853\n"
            "v 0.077607 0.967950 0.238853\n"
            "v 0.162456 0.850654 0.499995\n"
            "v 0.232822 0.657519 0.716563\n"
            "v 0.753442 0.657515 0.000000\n"
            "v 0.525730 0.850652 0.000000\n"
            "v 0.251147 0.967949 0.000000\n"
            "v -0.203181 0.967950 0.147618\n"
            "v -0.425323 0.850654 0.309011\n"
            "v -0.609547 0.657519 0.442856\n"
            "v -0.203181 0.967950 -0.147618\n"
            "v -0.425323 0.850654 -0.309011\n"
            "v -0.609547 0.657519 -0.442856\n"
            "v 0.077607 0.967950 -0.238853\n"
            "v 0.162456 0.850654 -0.499995\n"
            "v 0.232822 0.657519 -0.716563\n"
            "v 0.361800 0.894429 -0.262863\n"
            "v 0.638194 0.723610 -0.262864\n"
            "v 0.447209 0.723612 -0.525728\n"
            "v -0.138197 0.894430 -0.425319\n"
            "v -0.052790 0.723612 -0.688185\n"
            "v -0.361804 0.723612 -0.587778\n"
            "v -0.447210 0.894429 0.000000\n"
            "v -0.670817 0.723611 -0.162457\n"
            "v -0.670817 0.723611 0.162457\n"
            "v -0.138197 0.894430 0.425319\n"
            "v -0.361804 0.723612 0.587778\n"
            "v -0.052790 0.723612 0.688185\n"
            "v 0.361800 0.894429 0.262863\n"
            "v 0.447209 0.723612 0.525728\n"
            "v 0.638194 0.723610 0.262864\n"
            "v 0.861804 0.276396 -0.425322\n"
            "v 0.809019 0.000000 -0.587782\n"
            "v 0.670821 0.276397 -0.688189\n"
            "v -0.138199 0.276397 -0.951055\n"
            "v -0.309016 -0.000000 -0.951057\n"
            "v -0.447215 0.276397 -0.850649\n"
            "v -0.947213 0.276396 -0.162458\n"
            "v -1.000000 0.000001 0.000000\n"
            "v -0.947213 0.276397 0.162458\n"
            "v -0.447216 0.276397 0.850648\n"
            "v -0.309017 -0.000001 0.951056\n"
            "v -0.138199 0.276397 0.951055\n"
            "v 0.670820 0.276396 0.688190\n"
            "v 0.809019 -0.000002 0.587783\n"
            "v 0.861804 0.276394 0.425323\n"
            "v 0.309017 -0.000000 -0.951056\n"
            "v 0.447216 -0.276398 -0.850648\n"
            "v 0.138199 -0.276398 -0.951055\n"
            "v -0.809018 -0.000000 -0.587783\n"
            "v -0.670819 -0.276397 -0.688191\n"
            "v -0.861803 -0.276396 -0.425324\n"
            "v -0.809018 0.000000 0.587783\n"
            "v -0.861803 -0.276396 0.425324\n"
            "v -0.670819 -0.276397 0.688191\n"
            "v 0.309017 0.000000 0.951056\n"
            "v 0.138199 -0.276398 0.951055\n"
            "v 0.447216 -0.276398 0.850648\n"
            "v 1.000000 0.000000 0.000000\n"
            "v 0.947213 -0.276396 0.162458\n"
            "v 0.947213 -0.276396 -0.162458\n"
            "v 0.361803 -0.723612 -0.587779\n"
            "v 0.138197 -0.894429 -0.425321\n"
            "v 0.052789 -0.723611 -0.688186\n"
            "v -0.447211 -0.723612 -0.525727\n"
            "v -0.361801 -0.894429 -0.262863\n"
            "v -0.638195 -0.723609 -0.262863\n"
            "v -0.638195 -0.723609 0.262864\n"
            "v -0.361801 -0.894428 0.262864\n"
            "v -0.447211 -0.723610 0.525729\n"
            "v 0.670817 -0.723611 -0.162457\n"
            "v 0.670818 -0.723610 0.162458\n"
            "v 0.447211 -0.894428 0.000001\n"
            "v 0.052790 -0.723612 0.688185\n"
            "v 0.138199 -0.894429 0.425321\n"
            "v 0.361805 -0.723611 0.587779\n"
            "f 1 16 15\n"
            "f 2 18 24\n"
            "f 1 15 30\n"
            "f 1 30 36\n"
            "f 1 36 25\n"
            "f 2 24 45\n"
            "f 3 21 51\n"
            "f 4 33 57\n"
            "f 5 39 63\n"
            "f 6 42 69\n"
            "f 2 45 52\n"
            "f 3 51 58\n"
            "f 4 57 64\n"
            "f 5 63 70\n"
            "f 6 69 46\n"
            "f 7 75 90\n"
            "f 8 78 96\n"
            "f 9 81 99\n"
            "f 10 84 102\n"
            "f 11 87 91\n"
            "f 93 100 12\n"
            "f 92 103 93\n"
            "f 91 104 92\n"
            "f 93 103 100\n"
            "f 103 101 100\n"
            "f 92 104 103\n"
            "f 104 105 103\n"
            "f 103 105 101\n"
            "f 105 102 101\n"
            "f 91 87 104\n"
            "f 87 86 104\n"
            "f 104 86 105\n"
            "f 86 85 105\n"
            "f 105 85 102\n"
            "f 85 10 102\n"
            "f 100 97 12\n"
            "f 101 106 100\n"
            "f 102 107 101\n"
            "f 100 106 97\n"
            "f 106 98 97\n"
            "f 101 107 106\n"
            "f 107 108 106\n"
            "f 106 108 98\n"
            "f 108 99 98\n"
            "f 102 84 107\n"
            "f 84 83 107\n"
            "f 107 83 108\n"
            "f 83 82 108\n"
            "f 108 82 99\n"
            "f 82 9 99\n"
            "f 97 94 12\n"
            "f 98 109 97\n"
            "f 99 110 98\n"
            "f 97 109 94\n"
            "f 109 95 94\n"
            "f 98 110 109\n"
            "f 110 111 109\n"
            "f 109 111 95\n"
            "f 111 96 95\n"
            "f 99 81 110\n"
            "f 81 80 110\n"
            "f 110 80 111\n"
            "f 80 79 111\n"
            "f 111 79 96\n"
            "f 79 8 96\n"
            "f 94 88 12\n"
            "f 95 112 94\n"
            "f 96 113 95\n"
            "f 94 112 88\n"
            "f 112 89 88\n"
            "f 95 113 112\n"
            "f 113 114 112\n"
            "f 112 114 89\n"
            "f 114 90 89\n"
            "f 96 78 113\n"
            "f 78 77 113\n"
            "f 113 77 114\n"
            "f 77 76 114\n"
            "f 114 76 90\n"
            "f 76 7 90\n"
            "f 88 93 12\n"
            "f 89 115 88\n"
            "f 90 116 89\n"
            "f 88 115 93\n"
            "f 115 92 93\n"
            "f 89 116 115\n"
            "f 116 117 115\n"
            "f 115 117 92\n"
            "f 117 91 92\n"
            "f 90 75 116\n"
            "f 75 74 116\n"
            "f 116 74 117\n"
            "f 74 73 117\n"
            "f 117 73 91\n"
            "f 73 11 91\n"
            "f 48 87 11\n"
            "f 47 118 48\n"
            "f 46 119 47\n"
            "f 48 118 87\n"
            "f 118 86 87\n"
            "f 47 119 118\n"
            "f 119 120 118\n"
            "f 118 120 86\n"
            "f 120 85 86\n"
            "f 46 69 119\n"
            "f 69 68 119\n"
            "f 119 68 120\n"
            "f 68 67 120\n"
            "f 120 67 85\n"
            "f 67 10 85\n"
            "f 72 84 10\n"
            "f 71 121 72\n"
            "f 70 122 71\n"
            "f 72 121 84\n"
            "f 121 83 84\n"
            "f 71 122 121\n"
            "f 122 123 121\n"
            "f 121 123 83\n"
            "f 123 82 83\n"
            "f 70 63 122\n"
            "f 63 62 122\n"
            "f 122 62 123\n"
            "f 62 61 123\n"
            "f 123 61 82\n"
            "f 61 9 82\n"
            "f 66 81 9\n"
            "f 65 124 66\n"
            "f 64 125 65\n"
            "f 66 124 81\n"
            "f 124 80 81\n"
            "f 65 125 124\n"
            "f 125 126 124\n"
            "f 124 126 80\n"
            "f 126 79 80\n"
            "f 64 57 125\n"
            "f 57 56 125\n"
            "f 125 56 126\n"
            "f 56 55 126\n"
            "f 126 55 79\n"
            "f 55 8 79\n"
            "f 60 78 8\n"
            "f 59 127 60\n"
            "f 58 128 59\n"
            "f 60 127 78\n"
            "f 127 77 78\n"
            "f 59 128 127\n"
            "f 128 129 127\n"
            "f 127 129 77\n"
            "f 129 76 77\n"
            "f 58 51 128\n"
            "f 51 50 128\n"
            "f 128 50 129\n"
            "f 50 49 129\n"
            "f 129 49 76\n"
            "f 49 7 76\n"
            "f 54 75 7\n"
            "f 53 130 54\n"
            "f 52 131 53\n"
            "f 54 130 75\n"
            "f 130 74 75\n"
            "f 53 131 130\n"
            "f 131 132 130\n"
            "f 130 132 74\n"
            "f 132 73 74\n"
            "f 52 45 131\n"
            "f 45 44 131\n"
            "f 131 44 132\n"
            "f 44 43 132\n"
            "f 132 43 73\n"
            "f 43 11 73\n"
            "f 67 72 10\n"
            "f 68 133 67\n"
            "f 69 134 68\n"
            "f 67 133 72\n"
            "f 133 71 72\n"
            "f 68 134 133\n"
            "f 134 135 133\n"
            "f 133 135 71\n"
            "f 135 70 71\n"
            "f 69 42 134\n"
            "f 42 41 134\n"
            "f 134 41 135\n"
            "f 41 40 135\n"
            "f 135 40 70\n"
            "f 40 5 70\n"
            "f 61 66 9\n"
            "f 62 136 61\n"
            "f 63 137 62\n"
            "f 61 136 66\n"
            "f 136 65 66\n"
            "f 62 137 136\n"
            "f 137 138 136\n"
            "f 136 138 65\n"
            "f 138 64 65\n"
            "f 63 39 137\n"
            "f 39 38 137\n"
            "f 137 38 138\n"
            "f 38 37 138\n"
            "f 138 37 64\n"
            "f 37 4 64\n"
            "f 55 60 8\n"
            "f 56 139 55\n"
            "f 57 140 56\n"
            "f 55 139 60\n"
            "f 139 59 60\n"
            "f 56 140 139\n"
            "f 140 141 139\n"
            "f 139 141 59\n"
            "f 141 58 59\n"
            "f 57 33 140\n"
            "f 33 32 140\n"
            "f 140 32 141\n"
            "f 32 31 141\n"
            "f 141 31 58\n"
            "f 31 3 58\n"
            "f 49 54 7\n"
            "f 50 142 49\n"
            "f 51 143 50\n"
            "f 49 142 54\n"
            "f 142 53 54\n"
            "f 50 143 142\n"
            "f 143 144 142\n"
            "f 142 144 53\n"
            "f 144 52 53\n"
            "f 51 21 143\n"
            "f 21 20 143\n"
            "f 143 20 144\n"
            "f 20 19 144\n"
            "f 144 19 52\n"
            "f 19 2 52\n"
            "f 43 48 11\n"
            "f 44 145 43\n"
            "f 45 146 44\n"
            "f 43 145 48\n"
            "f 145 47 48\n"
            "f 44 146 145\n"
            "f 146 147 145\n"
            "f 145 147 47\n"
            "f 147 46 47\n"
            "f 45 24 146\n"
            "f 24 23 146\n"
            "f 146 23 147\n"
            "f 23 22 147\n"
            "f 147 22 46\n"
            "f 22 6 46\n"
            "f 27 42 6\n"
            "f 26 148 27\n"
            "f 25 149 26\n"
            "f 27 148 42\n"
            "f 148 41 42\n"
            "f 26 149 148\n"
            "f 149 150 148\n"
            "f 148 150 41\n"
            "f 150 40 41\n"
            "f 25 36 149\n"
            "f 36 35 149\n"
            "f 149 35 150\n"
            "f 35 34 150\n"
            "f 150 34 40\n"
            "f 34 5 40\n"
            "f 34 39 5\n"
            "f 35 151 34\n"
            "f 36 152 35\n"
            "f 34 151 39\n"
            "f 151 38 39\n"
            "f 35 152 151\n"
            "f 152 153 151\n"
            "f 151 153 38\n"
            "f 153 37 38\n"
            "f 36 30 152\n"
            "f 30 29 152\n"
            "f 152 29 153\n"
            "f 29 28 153\n"
            "f 153 28 37\n"
            "f 28 4 37\n"
            "f 28 33 4\n"
            "f 29 154 28\n"
            "f 30 155 29\n"
            "f 28 154 33\n"
            "f 154 32 33\n"
            "f 29 155 154\n"
            "f 155 156 154\n"
            "f 154 156 32\n"
            "f 156 31 32\n"
            "f 30 15 155\n"
            "f 15 14 155\n"
            "f 155 14 156\n"
            "f 14 13 156\n"
            "f 156 13 31\n"
            "f 13 3 31\n"
            "f 22 27 6\n"
            "f 23 157 22\n"
            "f 24 158 23\n"
            "f 22 157 27\n"
            "f 157 26 27\n"
            "f 23 158 157\n"
            "f 158 159 157\n"
            "f 157 159 26\n"
            "f 159 25 26\n"
            "f 24 18 158\n"
            "f 18 17 158\n"
            "f 158 17 159\n"
            "f 17 16 159\n"
            "f 159 16 25\n"
            "f 16 1 25\n"
            "f 13 21 3\n"
            "f 14 160 13\n"
            "f 15 161 14\n"
            "f 13 160 21\n"
            "f 160 20 21\n"
            "f 14 161 160\n"
            "f 161 162 160\n"
            "f 160 162 20\n"
            "f 162 19 20\n"
            "f 15 16 161\n"
            "f 16 17 161\n"
            "f 161 17 162\n"
            "f 17 18 162\n"
            "f 162 18 19\n"
            "f 18 2 19";
        #pragma endregion
        Resources::addMesh("PointLightBounds",data,CollisionType::None,false);
    }
    m_Constant = 0.1f;
    m_Linear = 0.1f;
    m_Exp = 0.1f;
    m_CullingRadius = calculateCullingRadius();
    m_Color = glm::vec4(1.0,1.0f,1.0f,1.0f);
    m_AttenuationModel = LightAttenuation::Distance_Squared;
}
PointLight::~PointLight(){
}
float PointLight::calculateCullingRadius(){
    float lightMax = Engine::Math::Max(m_Color.x,m_Color.y,m_Color.z);
    float radius = 0;
    if(m_AttenuationModel == LightAttenuation::Constant_Linear_Exponent){
        radius = (-m_Linear +  glm::sqrt(m_Linear * m_Linear - 4.0f * m_Exp * (m_Constant - (256.0f / 5.0f) * lightMax))) / (2.0f * m_Exp);
    }
    else if(m_AttenuationModel == LightAttenuation::Distance_Squared){
        radius = glm::sqrt(lightMax * (256.0f / 5.0f)); // 51.2f   is   256.0f / 5.0f
    }
    else if(m_AttenuationModel == LightAttenuation::Distance){
        radius = (lightMax * (256.0f / 5.0f));
    }
    else if(m_AttenuationModel == LightAttenuation::Spherical_Quadratic){
        radius = (((lightMax * (256.0f / 5.0f) - 1.0f) * (m_Radius*m_Radius)) / m_Radius) * 0.5f;
    }
    return radius;
}
float PointLight::getCullingRadius(){ return m_CullingRadius; }
float PointLight::getConstant(){ return m_Constant; }
float PointLight::getLinear(){ return m_Linear; }
float PointLight::getExponent(){ return m_Exp; }
void PointLight::setConstant(float c){ m_Constant = c; m_CullingRadius = calculateCullingRadius(); }
void PointLight::setLinear(float l){ m_Linear = l; m_CullingRadius = calculateCullingRadius(); }
void PointLight::setExponent(float e){ m_Exp = e; m_CullingRadius = calculateCullingRadius(); }
void PointLight::setAttenuation(float c,float l, float e){ m_Constant = c; m_Linear = l; m_Exp = e; m_CullingRadius = calculateCullingRadius(); }
void PointLight::setAttenuation(LightRange::Range range){
    boost::tuple<float,float,float>& data = LIGHT_RANGES[uint(range)];
    PointLight::setAttenuation(data.get<0>(),data.get<1>(),data.get<2>());
}
void PointLight::setAttenuationModel(LightAttenuation::Model model){
    m_AttenuationModel = model;
}
void PointLight::update(float dt){
    if(m_Parent != nullptr){
        m_Model = m_Parent->getModel();
    }
    else{
        m_Model = glm::mat4(1.0f);
    }
    glm::mat4 translationMatrix = glm::translate(m_Position);
    glm::mat4 scaleMatrix = glm::scale(glm::vec3(m_CullingRadius));
    m_Model = translationMatrix * scaleMatrix * m_Model;
}
void PointLight::lighten(){
    if(!m_Active) return;
    Camera* c = Resources::getActiveCamera();
    glm::vec3 pos = getPosition();
    if((!c->sphereIntersectTest(pos,m_CullingRadius)) || (c->getDistance(this) > Object::m_VisibilityThreshold * m_CullingRadius))
        return;
    sendGenericAttributesToShader();

    Renderer::sendUniform4f("LightDataA", m_AmbientIntensity,m_DiffuseIntensity,m_SpecularIntensity,0.0f);
    Renderer::sendUniform4f("LightDataB", 0.0f,0.0f,m_Constant,m_Linear);
    Renderer::sendUniform4f("LightDataC", m_Exp,pos.x,pos.y,pos.z);
    Renderer::sendUniform1fSafe("SpotLight",0.0f);

    Renderer::sendUniformMatrix4f("Model",m_Model);
    Renderer::sendUniformMatrix4f("VP",c->getViewProjection());

    if(glm::distance(c->getPosition(),pos) <= m_CullingRadius){                                                  
        Renderer::Settings::cullFace(GL_FRONT);
    }
    Resources::getMesh("PointLightBounds")->bind();
    Resources::getMesh("PointLightBounds")->render(); //this can bug out if we pass in custom uv's like in the renderQuad method
    Resources::getMesh("PointLightBounds")->unbind();
    Renderer::Settings::cullFace(GL_BACK);
}
SpotLight::SpotLight(string name, glm::vec3 pos,glm::vec3 direction,float cutoff, float outerCutoff,Scene* scene): PointLight(name,pos,scene){
    if(Resources::getMesh("SpotLightBounds") == nullptr){
        #pragma region MeshData
        string data = 
            "v 0.000000 1.000000 -1.000000\n"
            "v 0.222521 0.974928 -1.000000\n"
            "v 0.433884 0.900969 -1.000000\n"
            "v 0.623490 0.781831 -1.000000\n"
            "v 0.781831 0.623490 -1.000000\n"
            "v 0.900969 0.433884 -1.000000\n"
            "v 0.000000 -0.000000 0.000000\n"
            "v 0.974928 0.222521 -1.000000\n"
            "v 1.000000 -0.000000 -1.000000\n"
            "v 0.974928 -0.222521 -1.000000\n"
            "v 0.900969 -0.433884 -1.000000\n"
            "v 0.781831 -0.623490 -1.000000\n"
            "v 0.623490 -0.781832 -1.000000\n"
            "v 0.433884 -0.900969 -1.000000\n"
            "v 0.222521 -0.974928 -1.000000\n"
            "v -0.000000 -1.000000 -1.000000\n"
            "v -0.222521 -0.974928 -1.000000\n"
            "v -0.433884 -0.900969 -1.000000\n"
            "v -0.623490 -0.781831 -1.000000\n"
            "v -0.781832 -0.623489 -1.000000\n"
            "v -0.900969 -0.433883 -1.000000\n"
            "v -0.974928 -0.222520 -1.000000\n"
            "v -1.000000 0.000001 -1.000000\n"
            "v -0.974928 0.222522 -1.000000\n"
            "v -0.900968 0.433885 -1.000000\n"
            "v -0.781831 0.623491 -1.000000\n"
            "v -0.623489 0.781832 -1.000000\n"
            "v -0.433882 0.900969 -1.000000\n"
            "v -0.222519 0.974928 -1.000000\n"
            "f 1 7 2\n"
            "f 2 7 3\n"
            "f 3 7 4\n"
            "f 4 7 5\n"
            "f 5 7 6\n"
            "f 6 7 8\n"
            "f 8 7 9\n"
            "f 9 7 10\n"
            "f 10 7 11\n"
            "f 11 7 12\n"
            "f 12 7 13\n"
            "f 13 7 14\n"
            "f 14 7 15\n"
            "f 15 7 16\n"
            "f 16 7 17\n"
            "f 17 7 18\n"
            "f 18 7 19\n"
            "f 19 7 20\n"
            "f 20 7 21\n"
            "f 21 7 22\n"
            "f 22 7 23\n"
            "f 23 7 24\n"
            "f 24 7 25\n"
            "f 25 7 26\n"
            "f 26 7 27\n"
            "f 27 7 28\n"
            "f 28 7 29\n"
            "f 29 7 1\n"
            "f 17 1 16\n"
            "f 1 15 16\n"
            "f 3 15 2\n"
            "f 4 14 3\n"
            "f 18 29 17\n"
            "f 19 28 18\n"
            "f 19 26 27\n"
            "f 20 25 26\n"
            "f 22 25 21\n"
            "f 23 24 22\n"
            "f 5 13 4\n"
            "f 6 12 5\n"
            "f 8 11 6\n"
            "f 9 10 8\n"
            "f 17 29 1\n"
            "f 1 2 15\n"
            "f 3 14 15\n"
            "f 4 13 14\n"
            "f 18 28 29\n"
            "f 19 27 28\n"
            "f 19 20 26\n"
            "f 20 21 25\n"
            "f 22 24 25\n"
            "f 5 12 13\n"
            "f 6 11 12\n"
            "f 8 10 11";
        #pragma endregion
        Resources::addMesh("SpotLightBounds",data,CollisionType::None,false);    
    }
    alignTo(direction,0);
    ObjectBasic::update(0);
    setCutoff(cutoff);
    setCutoffOuter(outerCutoff);
    m_Type = LightType::Spot;
}
SpotLight::~SpotLight(){
}
void SpotLight::setCutoff(float cutoff){
    m_Cutoff = glm::cos(glm::radians(cutoff));
}
void SpotLight::setCutoffOuter(float outerCutoff){
    m_OuterCutoff = glm::cos(glm::radians(outerCutoff));
}
void SpotLight::update(float dt){
    if(m_Parent != nullptr){
        m_Model = m_Parent->getModel();
    }
    else{
        m_Model = glm::mat4(1.0f);
    }
    glm::mat4 translationMatrix = glm::translate(m_Position);
    glm::mat4 rotationMatrix = glm::mat4_cast(m_Orientation);
    glm::mat4 scaleMatrix = glm::scale(glm::vec3(m_CullingRadius));
    m_Model = translationMatrix * rotationMatrix * scaleMatrix * m_Model;
}
void SpotLight::lighten(){
    if(!m_Active) return;
    Camera* c = Resources::getActiveCamera();
    glm::vec3 pos = getPosition();
    if(!c->sphereIntersectTest(pos,m_CullingRadius) || (c->getDistance(this) > Object::m_VisibilityThreshold * m_CullingRadius))
        return;
    sendGenericAttributesToShader();

    Renderer::sendUniform4f("LightDataA", m_AmbientIntensity,m_DiffuseIntensity,m_SpecularIntensity,m_Forward.x);
    Renderer::sendUniform4f("LightDataB", m_Forward.y,m_Forward.z,m_Constant,m_Linear);
    Renderer::sendUniform4f("LightDataC", m_Exp,pos.x,pos.y,pos.z);
    Renderer::sendUniform4fSafe("LightDataE", m_Cutoff, m_OuterCutoff, float(m_AttenuationModel),0.0f);
    Renderer::sendUniform2fSafe("VertexShaderData",m_OuterCutoff,m_CullingRadius);
    Renderer::sendUniform1fSafe("SpotLight",1.0f);

    Renderer::sendUniformMatrix4f("Model",m_Model);
    Renderer::sendUniformMatrix4f("VP",c->getViewProjection());

    if(glm::distance(c->getPosition(),pos) <= m_CullingRadius){                                                  
        Renderer::Settings::cullFace(GL_FRONT);
    }
    Resources::getMesh("SpotLightBounds")->bind();
    Resources::getMesh("SpotLightBounds")->render(); //this can bug out if we pass in custom uv's like in the renderQuad method
    Resources::getMesh("SpotLightBounds")->unbind();
    Renderer::Settings::cullFace(GL_BACK);

    Renderer::sendUniform1fSafe("SpotLight",0.0f);
}
RodLight::RodLight(string name, glm::vec3 pos,float rodLength,Scene* scene): PointLight(name,pos,scene){
    if(Resources::getMesh("RodLightBounds") == nullptr){
        #pragma region Data
        string data = 
            "v -0.000000 1.000000 -1.000000\n"
            "v -0.000000 1.000000 1.000000\n"
            "v 0.284630 0.959493 -1.000000\n"
            "v 0.284630 0.959493 1.000000\n"
            "v 0.546200 0.841254 -1.000000\n"
            "v 0.546200 0.841254 1.000000\n"
            "v 0.763521 0.654861 -1.000000\n"
            "v 0.763521 0.654861 1.000000\n"
            "v 0.918986 0.415415 -1.000000\n"
            "v 0.918986 0.415415 1.000000\n"
            "v 1.000000 0.142315 -1.000000\n"
            "v 1.000000 0.142315 1.000000\n"
            "v 1.000000 -0.142315 -1.000000\n"
            "v 1.000000 -0.142315 1.000000\n"
            "v 0.918986 -0.415415 -1.000000\n"
            "v 0.918986 -0.415415 1.000000\n"
            "v 0.763521 -0.654860 -1.000000\n"
            "v 0.763521 -0.654861 1.000000\n"
            "v 0.546200 -0.841253 -1.000000\n"
            "v 0.546200 -0.841253 1.000000\n"
            "v 0.284630 -0.959493 -1.000000\n"
            "v 0.284630 -0.959493 1.000000\n"
            "v 0.000000 -1.000000 -1.000000\n"
            "v 0.000000 -1.000000 1.000000\n"
            "v -0.284629 -0.959493 -1.000000\n"
            "v -0.284629 -0.959493 1.000000\n"
            "v -0.546200 -0.841253 -1.000000\n"
            "v -0.546200 -0.841254 1.000000\n"
            "v -0.763521 -0.654861 -1.000000\n"
            "v -0.763521 -0.654861 1.000000\n"
            "v -0.918986 -0.415415 -1.000000\n"
            "v -0.918986 -0.415415 1.000000\n"
            "v -1.000000 -0.142315 -1.000000\n"
            "v -1.000000 -0.142315 1.000000\n"
            "v -1.000000 0.142314 -1.000000\n"
            "v -1.000000 0.142314 1.000000\n"
            "v -0.918986 0.415415 -1.000000\n"
            "v -0.918986 0.415414 1.000000\n"
            "v -0.763522 0.654860 -1.000000\n"
            "v -0.763522 0.654860 1.000000\n"
            "v -0.546201 0.841253 -1.000000\n"
            "v -0.546201 0.841253 1.000000\n"
            "v -0.284631 0.959493 -1.000000\n"
            "v -0.284631 0.959493 1.000000\n"
            "f 2 3 1\n"
            "f 4 5 3\n"
            "f 6 7 5\n"
            "f 8 9 7\n"
            "f 10 11 9\n"
            "f 12 13 11\n"
            "f 14 15 13\n"
            "f 16 17 15\n"
            "f 18 19 17\n"
            "f 20 21 19\n"
            "f 21 24 23\n"
            "f 24 25 23\n"
            "f 26 27 25\n"
            "f 28 29 27\n"
            "f 29 32 31\n"
            "f 32 33 31\n"
            "f 34 35 33\n"
            "f 35 38 37\n"
            "f 38 39 37\n"
            "f 39 42 41\n"
            "f 25 1 23\n"
            "f 42 43 41\n"
            "f 44 1 43\n"
            "f 3 23 1\n"
            "f 27 43 25\n"
            "f 29 41 27\n"
            "f 31 39 29\n"
            "f 33 37 31\n"
            "f 5 21 3\n"
            "f 7 19 5\n"
            "f 9 17 7\n"
            "f 11 15 9\n"
            "f 2 22 4\n"
            "f 24 44 26\n"
            "f 26 42 28\n"
            "f 28 40 30\n"
            "f 30 38 32\n"
            "f 32 36 34\n"
            "f 6 22 20\n"
            "f 8 20 18\n"
            "f 8 16 10\n"
            "f 12 16 14\n"
            "f 2 4 3\n"
            "f 4 6 5\n"
            "f 6 8 7\n"
            "f 8 10 9\n"
            "f 10 12 11\n"
            "f 12 14 13\n"
            "f 14 16 15\n"
            "f 16 18 17\n"
            "f 18 20 19\n"
            "f 20 22 21\n"
            "f 21 22 24\n"
            "f 24 26 25\n"
            "f 26 28 27\n"
            "f 28 30 29\n"
            "f 29 30 32\n"
            "f 32 34 33\n"
            "f 34 36 35\n"
            "f 35 36 38\n"
            "f 38 40 39\n"
            "f 39 40 42\n"
            "f 25 43 1\n"
            "f 42 44 43\n"
            "f 44 2 1\n"
            "f 3 21 23\n"
            "f 27 41 43\n"
            "f 29 39 41\n"
            "f 31 37 39\n"
            "f 33 35 37\n"
            "f 5 19 21\n"
            "f 7 17 19\n"
            "f 9 15 17\n"
            "f 11 13 15\n"
            "f 2 24 22\n"
            "f 24 2 44\n"
            "f 26 44 42\n"
            "f 28 42 40\n"
            "f 30 40 38\n"
            "f 32 38 36\n"
            "f 6 4 22\n"
            "f 8 6 20\n"
            "f 8 18 16\n"
            "f 12 10 16";
        #pragma endregion
        Resources::addMesh("RodLightBounds",data,CollisionType::None,false);    
    }
    ObjectBasic::update(0);
    setRodLength(rodLength);
    m_Type = LightType::Rod;
}
RodLight::~RodLight(){
}
void RodLight::setRodLength(float length){ m_RodLength = length; }
void RodLight::update(float dt){
    if(m_Parent != nullptr){
        m_Model = m_Parent->getModel();
    }
    else{
        m_Model = glm::mat4(1.0f);
    }
    glm::mat4 translationMatrix = glm::translate(m_Position);
    glm::mat4 rotationMatrix = glm::mat4_cast(m_Orientation);
    glm::mat4 scaleMatrix = glm::scale(glm::vec3(m_CullingRadius,m_CullingRadius,(m_RodLength / 2.0f)+m_CullingRadius));
    m_Model = translationMatrix * rotationMatrix * scaleMatrix * m_Model;
}
void RodLight::lighten(){
    if(!m_Active) return;
    Camera* c = Resources::getActiveCamera();
    glm::vec3 pos = getPosition();
    float cullingDistance = m_RodLength+(m_CullingRadius*2.0f);
    if(!c->sphereIntersectTest(pos,cullingDistance) || (c->getDistance(this) > Object::m_VisibilityThreshold * cullingDistance))
        return;	
    sendGenericAttributesToShader();

    float half = m_RodLength / 2.0f;
    glm::vec3 firstEndPt = pos + (m_Forward * half);
    glm::vec3 secndEndPt = pos - (m_Forward * half);

    Renderer::sendUniform4f("LightDataA", m_AmbientIntensity,m_DiffuseIntensity,m_SpecularIntensity,firstEndPt.x);
    Renderer::sendUniform4f("LightDataB", firstEndPt.y,firstEndPt.z,m_Constant,m_Linear);
    Renderer::sendUniform4f("LightDataC", m_Exp,secndEndPt.x,secndEndPt.y,secndEndPt.z);
    Renderer::sendUniform4fSafe("LightDataE", m_RodLength, 0.0f, float(m_AttenuationModel),0.0f);
    Renderer::sendUniform1fSafe("SpotLight",0.0f);

    Renderer::sendUniformMatrix4f("Model",m_Model);
    Renderer::sendUniformMatrix4f("VP",c->getViewProjection());

    if(glm::distance(c->getPosition(),pos) <= cullingDistance){                                                  
        Renderer::Settings::cullFace(GL_FRONT);
    }
    Resources::getMesh("RodLightBounds")->bind();
    Resources::getMesh("RodLightBounds")->render(); //this can bug out if we pass in custom uv's like in the renderQuad method
    Resources::getMesh("RodLightBounds")->unbind();
    Renderer::Settings::cullFace(GL_BACK);

    Renderer::sendUniform1fSafe("SpotLight",0.0f);
}
float RodLight::rodLength(){ return m_RodLength; }

class LightProbe::impl{
    public:
        uint m_EnvMapSize;
        FramebufferObject* m_FBO;
        vector<GLuint> m_TextureAddresses;  
        glm::mat4 m_Views[6];
        bool m_OnlyOnce;
        bool m_DidFirst;
        void _init(uint envMapSize,LightProbe* super,bool onlyOnce,Scene* scene){
            m_EnvMapSize = envMapSize;
            m_OnlyOnce = onlyOnce;
            glm::vec3 pos = super->getPosition();
            Camera* c = Resources::getActiveCamera();
            if(c != nullptr) super->m_Projection = glm::perspective(glm::radians(90.0f), 1.0f, c->getNear(), c->getFar());
            else             super->m_Projection = glm::perspective(glm::radians(90.0f), 1.0f, 0.01f, 99999999.0f);
            m_Views[0] = glm::lookAt(pos, pos + glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f));
            m_Views[1] = glm::lookAt(pos, pos + glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f));
            m_Views[2] = glm::lookAt(pos, pos + glm::vec3( 0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f));
            m_Views[3] = glm::lookAt(pos, pos + glm::vec3( 0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f));
            m_Views[4] = glm::lookAt(pos, pos + glm::vec3( 0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f));
            m_Views[5] = glm::lookAt(pos, pos + glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f));

            m_FBO = new FramebufferObject(super->name() + " _ProbeFBO",m_EnvMapSize,m_EnvMapSize,ImageInternalFormat::Depth16);
        }
        void _destruct(){
            for(auto addr:m_TextureAddresses){
                glDeleteTextures(1,&addr);
            }
            glBindTexture(GL_TEXTURE_CUBE_MAP,0);
            SAFE_DELETE(m_FBO);
        }
        void _update(float dt,LightProbe* super){
            if(super->m_Parent != nullptr){ super->m_Model = super->m_Parent->getModel(); }
            else{ super->m_Model = glm::mat4(1.0f); }
            glm::mat4 translationMatrix = glm::translate(super->m_Position);
            glm::mat4 rotationMatrix = glm::mat4_cast(super->m_Orientation);

            super->m_Model = translationMatrix * rotationMatrix * super->m_Model;

            glm::vec3 pos = super->getPosition();
            m_Views[0] = glm::lookAt(pos, pos + glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f));
            m_Views[1] = glm::lookAt(pos, pos + glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f));
            m_Views[2] = glm::lookAt(pos, pos + glm::vec3( 0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f));
            m_Views[3] = glm::lookAt(pos, pos + glm::vec3( 0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f));
            m_Views[4] = glm::lookAt(pos, pos + glm::vec3( 0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f));
            m_Views[5] = glm::lookAt(pos, pos + glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f));
        }
        void _render(LightProbe* super){
            if(m_DidFirst == true && m_OnlyOnce == true) return;

            //Yes, i know, this is dangerous. Very dangerous
            Renderer::Detail::RenderManagement::m_gBuffer->resize(m_EnvMapSize,m_EnvMapSize);

            uint& prevReadBuffer = Renderer::Detail::RendererInfo::GeneralInfo::current_bound_read_fbo;
            uint& prevDrawBuffer = Renderer::Detail::RendererInfo::GeneralInfo::current_bound_draw_fbo;

            //render the scene into a cubemap. this will be VERY expensive...
            if(m_TextureAddresses.size() == 0){
                m_TextureAddresses.push_back(GLuint(0));
                glGenTextures(1,&m_TextureAddresses.at(0));
                glBindTexture(GL_TEXTURE_CUBE_MAP,m_TextureAddresses.at(0));
                for (uint i = 0; i < 6; ++i){
                    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X+i,0,GL_RGB16F,m_EnvMapSize,m_EnvMapSize,0,GL_RGB,GL_FLOAT,NULL);
                }
                Texture::setWrapping(GL_TEXTURE_CUBE_MAP,TextureWrap::ClampToEdge);
                Texture::setFilter(GL_TEXTURE_CUBE_MAP,TextureFilter::Linear);
            }
            else{
                glBindTexture(GL_TEXTURE_CUBE_MAP,m_TextureAddresses.at(0));
            }
            //_update(0,super); //this might be needed
            m_FBO->bind();
            for (uint i = 0; i < 6; ++i){
				Renderer::Settings::clear();
                super->m_View = m_Views[i];

                super->m_Orientation = glm::conjugate(glm::quat_cast(m_Views[i]));
                super->_constructFrustrum();
                
                glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_CUBE_MAP_POSITIVE_X+i,m_TextureAddresses.at(0),0);

                //replace this gbuffer eventually with a gbuffer for the light probe specifically... or recycle the default gbuffer (but that will be a serious
                // fps loss?)
                Renderer::Detail::RenderManagement::render(Renderer::Detail::RenderManagement::m_gBuffer,
                    super,m_EnvMapSize,m_EnvMapSize,false,false,false,false,super->m_Parent,false,m_FBO->address(),
                    m_FBO->attatchments().at(FramebufferAttatchment::at(FramebufferAttatchment::Depth))->address());
            }
            /////////////////////////////////////////////////////////////////
			m_FBO->bind();
            uint size = 32;
            if(m_TextureAddresses.size() == 1){
                m_TextureAddresses.push_back(GLuint(0));
                glGenTextures(1,&m_TextureAddresses.at(1));
                glBindTexture(GL_TEXTURE_CUBE_MAP,m_TextureAddresses.at(1));
                for (uint i = 0; i < 6; ++i){
                    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, size, size, 0, GL_RGB, GL_FLOAT, NULL);
                }
                Texture::setWrapping(GL_TEXTURE_CUBE_MAP,TextureWrap::ClampToEdge);
                Texture::setFilter(GL_TEXTURE_CUBE_MAP,TextureFilter::Linear);
            }
            else{
                glBindTexture(GL_TEXTURE_CUBE_MAP,m_TextureAddresses.at(1));
            }
            glRenderbufferStorage(GL_RENDERBUFFER,GL_DEPTH_COMPONENT16,size,size);
            ShaderP* p = Resources::getShaderProgram("Cubemap_Convolude"); p->bind();
            Renderer::bindTexture("cubemap",m_TextureAddresses.at(0),0,GL_TEXTURE_CUBE_MAP);

            Renderer::setViewport(0,0,size,size);
            for (uint i = 0; i < 6; ++i){
                glm::mat4 vp = super->m_Projection * m_Views[i];
                Renderer::sendUniformMatrix4f("VP", vp);
                glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_CUBE_MAP_POSITIVE_X+i,m_TextureAddresses.at(1),0);
                Renderer::Settings::clear(true,true,false);
                Skybox::bindMesh();
            }
            p->unbind();

            //now gen EnvPrefilterMap for specular IBL.
            size = m_EnvMapSize/4;
            if(m_TextureAddresses.size() == 2){
                m_TextureAddresses.push_back(GLuint(0));
                glGenTextures(1, &m_TextureAddresses.at(2));
                glBindTexture(GL_TEXTURE_CUBE_MAP,m_TextureAddresses.at(2));
                for (uint i = 0; i < 6; ++i){
                    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X+i,0,GL_RGB16F,size,size,0,GL_RGB,GL_FLOAT,NULL);
                }
                Texture::setWrapping(GL_TEXTURE_CUBE_MAP,TextureWrap::ClampToEdge);
                Texture::setMinFilter(GL_TEXTURE_CUBE_MAP,TextureFilter::Linear_Mipmap_Linear);
                Texture::setMaxFilter(GL_TEXTURE_CUBE_MAP,TextureFilter::Linear);
                glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
            }
            else{
                glBindTexture(GL_TEXTURE_CUBE_MAP,m_TextureAddresses.at(2));
            }

            p = Resources::getShaderProgram("Cubemap_Prefilter_Env"); p->bind();
            Renderer::bindTexture("cubemap",m_TextureAddresses.at(0),0,GL_TEXTURE_CUBE_MAP);
            Renderer::sendUniform1f("PiFourDividedByResSquaredTimesSix",12.56637f / float((m_EnvMapSize * m_EnvMapSize)*6));
            Renderer::sendUniform1i("NUM_SAMPLES",32);
            uint maxMipLevels = 5;
            for (uint m = 0; m < maxMipLevels; ++m){
                uint mipSize  = uint(size * glm::pow(0.5,m)); // reisze framebuffer according to mip-level size.
                glRenderbufferStorage(GL_RENDERBUFFER,GL_DEPTH_COMPONENT24,mipSize,mipSize);
                Renderer::setViewport(0,0,mipSize,mipSize);
                float roughness = (float)m/(float)(maxMipLevels-1);
                Renderer::sendUniform1f("roughness",roughness);
                float a = roughness * roughness;
                Renderer::sendUniform1f("a2",a*a);
                for (uint i = 0; i < 6; ++i){
                    glm::mat4 vp = super->m_Projection * m_Views[i];
                    Renderer::sendUniformMatrix4f("VP", vp);
                    glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_CUBE_MAP_POSITIVE_X+i,m_TextureAddresses.at(2),m);
                    Renderer::Settings::clear(true,true,false);
                    Skybox::bindMesh();
                }
            }
            m_FBO->unbind();
            Renderer::bindReadFBO(prevReadBuffer);
            Renderer::bindDrawFBO(prevDrawBuffer);
            m_DidFirst = true;
        }
};

LightProbe::LightProbe(string n, uint envMapSize,glm::vec3 pos,bool onlyOnce,Scene* scene):Camera(n,glm::radians(90.0f),1.0f,0.01f,9999999.0f,scene),m_i(new impl){
    m_i->_init(envMapSize,this,onlyOnce,scene);
    this->setPosition(pos);
    if(scene == nullptr){
        scene = Resources::getCurrentScene();
    }
    scene->m_LightProbes.emplace(name(),this);
}
LightProbe::~LightProbe(){
    m_i->_destruct();
}
void LightProbe::update(float dt){ m_i->_update(dt,this); }
void LightProbe::renderCubemap(){ m_i->_render(this); }
const uint LightProbe::getEnvMapSize() const{ return m_i->m_EnvMapSize; }
GLuint LightProbe::getEnvMap(){ 
	if(m_i->m_TextureAddresses.size() <= 0) return 0;
	return m_i->m_TextureAddresses.at(0); 
}
GLuint LightProbe::getIrriadianceMap(){
	if(m_i->m_TextureAddresses.size() <= 1) return 0;
	return m_i->m_TextureAddresses.at(1); 
}
GLuint LightProbe::getPrefilterMap(){
	if(m_i->m_TextureAddresses.size() <= 2) return 0;
	return m_i->m_TextureAddresses.at(2); 
}
