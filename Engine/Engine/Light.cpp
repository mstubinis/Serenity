#include "Light.h"
#include "Engine_Renderer.h"
#include "Engine_Resources.h"
#include "Engine_BuiltInResources.h"
#include "Engine_Math.h"
#include "Engine_GLStateMachine.h"
#include "Camera.h"
#include "Mesh.h"
#include "Scene.h"
#include "Texture.h"
#include "FramebufferObject.h"
#include "Engine_Window.h"
#include "Scene.h"

#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <boost/tuple/tuple.hpp>

using namespace Engine;
using namespace Engine::Renderer;
using namespace std;

vector<boost::tuple<float,float,float>> LIGHT_RANGES = [](){
    vector<boost::tuple<float,float,float>> m;
    m.resize(LightRange::_TOTAL,boost::make_tuple(0.0f,0.0f,0.0f));

    m[LightRange::_7]    = boost::make_tuple(1.0f, 0.7f, 1.8f);
    m[LightRange::_13]   = boost::make_tuple(1.0f, 0.35f, 0.44f);
    m[LightRange::_20]   = boost::make_tuple(1.0f, 0.22f, 0.20f);
    m[LightRange::_32]   = boost::make_tuple(1.0f, 0.14f, 0.07f);
    m[LightRange::_50]   = boost::make_tuple(1.0f, 0.09f, 0.032f);
    m[LightRange::_65]   = boost::make_tuple(1.0f, 0.07f, 0.017f);
    m[LightRange::_100]  = boost::make_tuple(1.0f, 0.045f, 0.0075f);
    m[LightRange::_160]  = boost::make_tuple(1.0f, 0.027f, 0.0028f);
    m[LightRange::_200]  = boost::make_tuple(1.0f, 0.022f, 0.0019f);
    m[LightRange::_325]  = boost::make_tuple(1.0f, 0.014f, 0.0007f);
    m[LightRange::_600]  = boost::make_tuple(1.0f, 0.007f, 0.0002f);
    m[LightRange::_3250] = boost::make_tuple(1.0f, 0.0014f, 0.000007f);

    return m;
}();

class SunLight::impl final{
    public:
        ComponentBody* m_Body;
        bool m_Active;
        glm::vec4 m_Color;
        LightType::Type m_Type;
        float m_AmbientIntensity, m_DiffuseIntensity, m_SpecularIntensity;
        void _init(SunLight& super,LightType::Type& type){
            m_Active = true;
            m_Color = glm::vec4(1.0f);
            m_Type = type;
            m_AmbientIntensity = 0.005f;
            m_DiffuseIntensity = 2.0f;
            m_SpecularIntensity = 1.0f;

            m_Body = new ComponentBody();
            super.addComponent(m_Body);
        }
};

SunLight::SunLight(glm::vec3 pos,LightType::Type type,Scene* scene):Entity(),m_i(new impl){
    if(!scene){
        scene = Resources::getCurrentScene();
    }
    scene->addEntity(*this); //keep lights out of the global per scene entity pool?
    epriv::InternalScenePublicInterface::GetLights(*scene).push_back(this);

    m_i->_init(*this,type);
    m_i->m_Body->setPosition(pos);
}
SunLight::~SunLight(){
}
void SunLight::lighten(){
    if(!isActive()) return;
    glm::vec3 pos = m_i->m_Body->position();
    sendUniform4("LightDataA", m_i->m_AmbientIntensity,m_i->m_DiffuseIntensity,m_i->m_SpecularIntensity,0.0f);
    sendUniform4("LightDataC",0.0f,pos.x,pos.y,pos.z);
    sendUniform4("LightDataD",m_i->m_Color.x, m_i->m_Color.y, m_i->m_Color.z,float(m_i->m_Type));
    sendUniform1Safe("Type",0.0f);

    renderFullscreenTriangle(Resources::getWindowSize().x,Resources::getWindowSize().y);
}
glm::vec3 SunLight::position(){ return m_i->m_Body->position(); }
void SunLight::setColor(float r,float g,float b,float a){ Engine::Math::setColor(m_i->m_Color,r,g,b,a); }
void SunLight::setColor(glm::vec4 col){ Engine::Math::setColor(m_i->m_Color,col.r,col.g,col.b,col.a); }
void SunLight::setPosition(float x,float y,float z){ m_i->m_Body->setPosition(x,y,z); }
void SunLight::setPosition(glm::vec3 pos){ m_i->m_Body->setPosition(pos); }
float SunLight::getAmbientIntensity(){ return m_i->m_AmbientIntensity; }
void SunLight::setAmbientIntensity(float a){ m_i->m_AmbientIntensity = a; }
float SunLight::getDiffuseIntensity(){ return m_i->m_DiffuseIntensity; }
void SunLight::setDiffuseIntensity(float d){ m_i->m_DiffuseIntensity = d; }
float SunLight::getSpecularIntensity(){ return m_i->m_SpecularIntensity; }
void SunLight::setSpecularIntensity(float s){ m_i->m_SpecularIntensity = s; }
void SunLight::activate(bool b){ m_i->m_Active = b; }
void SunLight::deactivate(){ m_i->m_Active = false; }
bool SunLight::isActive(){ return m_i->m_Active; }
uint SunLight::type(){ return m_i->m_Type; }
DirectionalLight::DirectionalLight(glm::vec3 dir,Scene* scene):SunLight(glm::vec3(0),LightType::Directional,scene){
    m_i->m_Body->alignTo(dir,0);
}
DirectionalLight::~DirectionalLight(){
}
void DirectionalLight::lighten(){
    if(!isActive()) return;
    glm::vec3 _forward = m_i->m_Body->forward();
    sendUniform4("LightDataA", m_i->m_AmbientIntensity,m_i->m_DiffuseIntensity,m_i->m_SpecularIntensity,_forward.x);
    sendUniform4("LightDataB", _forward.y,_forward.z,0.0f, 0.0f);
    sendUniform4("LightDataD",m_i->m_Color.x, m_i->m_Color.y, m_i->m_Color.z,float(m_i->m_Type));
    sendUniform1Safe("Type",0.0f);
    renderFullscreenTriangle(Resources::getWindowSize().x,Resources::getWindowSize().y);
}
PointLight::PointLight(glm::vec3 pos,Scene* scene): SunLight(pos,LightType::Point,scene){
    m_C = m_L = m_E = 0.1f;
    m_CullingRadius = calculateCullingRadius();
    m_AttenuationModel = LightAttenuation::Constant_Linear_Exponent;
}
PointLight::~PointLight(){
}
float PointLight::calculateCullingRadius(){
    float lightMax = Math::Max(m_i->m_Color.x,m_i->m_Color.y,m_i->m_Color.z);
    float radius = 0;
    //if(m_AttenuationModel == LightAttenuation::Constant_Linear_Exponent){
        radius = (-m_L +  glm::sqrt(m_L * m_L - 4.0f * m_E * (m_C - (256.0f / 5.0f) * lightMax))) / (2.0f * m_E);
    //}
    //else if(m_AttenuationModel == LightAttenuation::Distance_Squared){
    //    radius = glm::sqrt(lightMax * (256.0f / 5.0f)); // 51.2f   is   256.0f / 5.0f
    //}
    //else if(m_AttenuationModel == LightAttenuation::Distance){
    //    radius = (lightMax * (256.0f / 5.0f));
    //}
    m_i->m_Body->setScale(radius,radius,radius);
    return radius;
}
float PointLight::getCullingRadius(){ return m_CullingRadius; }
float PointLight::getConstant(){ return m_C; }
float PointLight::getLinear(){ return m_L; }
float PointLight::getExponent(){ return m_E; }
void PointLight::setConstant(float c){ m_C = c; m_CullingRadius = calculateCullingRadius(); }
void PointLight::setLinear(float l){ m_L = l; m_CullingRadius = calculateCullingRadius(); }
void PointLight::setExponent(float e){ m_E = e; m_CullingRadius = calculateCullingRadius(); }
void PointLight::setAttenuation(float c,float l, float e){ m_C = c; m_L = l; m_E = e; m_CullingRadius = calculateCullingRadius(); }
void PointLight::setAttenuation(LightRange::Range r){ auto& d=LIGHT_RANGES[uint(r)]; PointLight::setAttenuation(d.get<0>(),d.get<1>(),d.get<2>()); }
void PointLight::setAttenuationModel(LightAttenuation::Model model){
    m_AttenuationModel = model; m_CullingRadius = calculateCullingRadius();
}
void PointLight::lighten(){
    if(!isActive()) return;
    Camera* c = Resources::getCurrentScene()->getActiveCamera();
    glm::vec3 pos = m_i->m_Body->position();
    if((!c->sphereIntersectTest(pos,m_CullingRadius)) || (c->getDistance(pos) > 1100.0f * m_CullingRadius)) //1100.0f is the visibility threshold
        return;
    sendUniform4("LightDataA", m_i->m_AmbientIntensity,m_i->m_DiffuseIntensity,m_i->m_SpecularIntensity,0.0f);
    sendUniform4("LightDataB", 0.0f,0.0f,m_C,m_L);
    sendUniform4("LightDataC", m_E,pos.x,pos.y,pos.z);
    sendUniform4("LightDataD",m_i->m_Color.x, m_i->m_Color.y, m_i->m_Color.z,float(m_i->m_Type));
    sendUniform4Safe("LightDataE", 0.0f, 0.0f, float(m_AttenuationModel),0.0f);
    sendUniform1Safe("Type",1.0f);

    glm::vec3 camPos = c->getPosition();
    glm::mat4 model = m_i->m_Body->modelMatrix();
    glm::mat4 vp = c->getViewProjection();

    sendUniformMatrix4("Model",model);
    sendUniformMatrix4("VP", vp);

    GLEnable(GLState::DEPTH_TEST);
    if(glm::distance(c->getPosition(),pos) <= m_CullingRadius){ //inside the light volume
        Settings::cullFace(GL_FRONT);
        Renderer::setDepthFunc(DepthFunc::GEqual);
    }
    epriv::InternalMeshes::PointLightBounds->bind();
    epriv::InternalMeshes::PointLightBounds->render(false); //this can bug out if we pass in custom uv's like in the renderQuad method
    epriv::InternalMeshes::PointLightBounds->unbind();
    Settings::cullFace(GL_BACK);
    Renderer::setDepthFunc(DepthFunc::LEqual);
    GLDisable(GLState::DEPTH_TEST);
}
SpotLight::SpotLight(glm::vec3 pos,glm::vec3 direction,float cutoff, float outerCutoff,Scene* scene): PointLight(pos,scene){
    m_i->m_Body->alignTo(direction,0);
    setCutoff(cutoff);
    setCutoffOuter(outerCutoff);
    m_i->m_Type = LightType::Spot;
}
SpotLight::~SpotLight(){
}
void SpotLight::setCutoff(float cutoff){
    m_Cutoff = glm::cos(glm::radians(cutoff));
}
void SpotLight::setCutoffOuter(float outerCutoff){
    m_OuterCutoff = glm::cos(glm::radians(outerCutoff));
}

void SpotLight::lighten(){
    if(!isActive()) return;
    Camera* c = Resources::getCurrentScene()->getActiveCamera();
    glm::vec3 pos = m_i->m_Body->position();
    glm::vec3 _forward = m_i->m_Body->forward();
    if(!c->sphereIntersectTest(pos,m_CullingRadius) || (c->getDistance(pos) > 1100.0f * m_CullingRadius))
        return;
    sendUniform4("LightDataA", m_i->m_AmbientIntensity,m_i->m_DiffuseIntensity,m_i->m_SpecularIntensity,_forward.x);
    sendUniform4("LightDataB", _forward.y,_forward.z,m_C,m_L);
    sendUniform4("LightDataC", m_E,pos.x,pos.y,pos.z);
    sendUniform4("LightDataD",m_i->m_Color.x, m_i->m_Color.y, m_i->m_Color.z,float(m_i->m_Type));
    sendUniform4Safe("LightDataE", m_Cutoff, m_OuterCutoff, float(m_AttenuationModel),0.0f);
    sendUniform2Safe("VertexShaderData",m_OuterCutoff,m_CullingRadius);
    sendUniform1Safe("Type",2.0f);

    glm::vec3 camPos = c->getPosition();
    glm::mat4 model = m_i->m_Body->modelMatrix();
    glm::mat4 vp = c->getViewProjection();

    sendUniformMatrix4("Model", model);
    sendUniformMatrix4("VP", vp);

    GLEnable(GLState::DEPTH_TEST);
    if(glm::distance(c->getPosition(),pos) <= m_CullingRadius){ //inside the light volume                                                 
        Settings::cullFace(GL_FRONT);
        Renderer::setDepthFunc(DepthFunc::GEqual);
    }
    epriv::InternalMeshes::SpotLightBounds->bind();
    epriv::InternalMeshes::SpotLightBounds->render(false); //this can bug out if we pass in custom uv's like in the renderQuad method
    epriv::InternalMeshes::SpotLightBounds->unbind();
    Settings::cullFace(GL_BACK);
    Renderer::setDepthFunc(DepthFunc::LEqual);

    sendUniform1Safe("Type",0.0f); //is this really needed?
    GLDisable(GLState::DEPTH_TEST);
}
RodLight::RodLight(glm::vec3 pos,float rodLength,Scene* scene): PointLight(pos,scene){
    setRodLength(rodLength);
    m_i->m_Type = LightType::Rod;
    m_i->m_Body->setScale(m_CullingRadius,m_CullingRadius, (m_RodLength / 2.0f) + m_CullingRadius );
}
RodLight::~RodLight(){
}
float RodLight::calculateCullingRadius(){
    float res = PointLight::calculateCullingRadius();
    m_i->m_Body->setScale(m_CullingRadius,m_CullingRadius, (m_RodLength / 2.0f) + m_CullingRadius );
    return res;
}
void RodLight::setRodLength(float length){ 
    m_RodLength = length;
    m_i->m_Body->setScale(m_CullingRadius,m_CullingRadius, (m_RodLength / 2.0f) + m_CullingRadius );
}
void RodLight::lighten(){
    if(!isActive()) return;
    Camera* c = Resources::getCurrentScene()->getActiveCamera();
    glm::vec3 pos = m_i->m_Body->position();
    float cullingDistance = m_RodLength + (m_CullingRadius * 2.0f);
    if(!c->sphereIntersectTest(pos,cullingDistance) || (c->getDistance(pos) > 1100.0f * cullingDistance))
        return;
    float half = m_RodLength / 2.0f;
    glm::vec3 firstEndPt = pos + (m_i->m_Body->forward() * half);
    glm::vec3 secndEndPt = pos - (m_i->m_Body->forward() * half);
    sendUniform4("LightDataA", m_i->m_AmbientIntensity,m_i->m_DiffuseIntensity,m_i->m_SpecularIntensity,firstEndPt.x);
    sendUniform4("LightDataB", firstEndPt.y,firstEndPt.z,m_C,m_L);
    sendUniform4("LightDataC", m_E,secndEndPt.x,secndEndPt.y,secndEndPt.z);
    sendUniform4("LightDataD",m_i->m_Color.x, m_i->m_Color.y, m_i->m_Color.z,float(m_i->m_Type));
    sendUniform4Safe("LightDataE", m_RodLength, 0.0f, float(m_AttenuationModel),0.0f);
    sendUniform1Safe("Type",1.0f);

    glm::vec3 camPos = c->getPosition();
    glm::mat4 model = m_i->m_Body->modelMatrix();
    glm::mat4 vp = c->getViewProjection();

    sendUniformMatrix4("Model", model);
    sendUniformMatrix4("VP", vp);

    GLEnable(GLState::DEPTH_TEST);
    if(glm::distance(c->getPosition(),pos) <= cullingDistance){                                                  
        Settings::cullFace(GL_FRONT);
        Renderer::setDepthFunc(DepthFunc::GEqual);
    }
    epriv::InternalMeshes::RodLightBounds->bind();
    epriv::InternalMeshes::RodLightBounds->render(false); //this can bug out if we pass in custom uv's like in the renderQuad method
    epriv::InternalMeshes::RodLightBounds->unbind();
    Settings::cullFace(GL_BACK);
    Renderer::setDepthFunc(DepthFunc::LEqual);
    GLDisable(GLState::DEPTH_TEST);

    sendUniform1Safe("Type",0.0f); //is this really needed?
}
float RodLight::rodLength(){ return m_RodLength; }

/*
class LightProbe::impl{
    public:
        uint m_EnvMapSize;
        epriv::FramebufferObject* m_FBO;

        GLuint m_TextureEnvMap;
        GLuint m_TextureConvolutionMap;
        GLuint m_TexturePrefilterMap;
        uint m_TexturesMade;
        uint m_SidesPerFrame;
        uint m_CurrentSide;

        glm::mat4 m_Views[6];
        bool m_SecondFrame;
        bool m_OnlyOnce;
        bool m_DidFirst;
        void _init(uint envMapSize,LightProbe* super,bool onlyOnce,Scene* scene,uint sidesPerFrame){
            m_SecondFrame = false;
            m_TexturesMade = 0;
            if(sidesPerFrame >= 7) sidesPerFrame = 6;
            else if(sidesPerFrame <= 0) sidesPerFrame = 1;
            m_SidesPerFrame = sidesPerFrame;
            m_CurrentSide = 0;
            m_EnvMapSize = envMapSize;
            m_OnlyOnce = onlyOnce;
            glm::vec3 pos = super->getPosition();
            Camera* c = Resources::getCurrentScene()->getActiveCamera();
            if(c != nullptr) super->m_Projection = glm::perspective(glm::radians(90.0f),1.0f, c->getNear(), c->getFar());
            else             super->m_Projection = glm::perspective(glm::radians(90.0f),1.0f, 0.001f, 9999999999.0f);
            m_Views[0] = glm::lookAt(pos, pos + glm::vec3( 1, 0, 0), glm::vec3(0,-1, 0));
            m_Views[1] = glm::lookAt(pos, pos + glm::vec3(-1, 0, 0), glm::vec3(0,-1, 0));
            m_Views[2] = glm::lookAt(pos, pos + glm::vec3( 0, 1, 0), glm::vec3(0, 0, 1));
            m_Views[3] = glm::lookAt(pos, pos + glm::vec3( 0,-1, 0), glm::vec3(0, 0,-1));
            m_Views[4] = glm::lookAt(pos, pos + glm::vec3( 0, 0, 1), glm::vec3(0,-1, 0));
            m_Views[5] = glm::lookAt(pos, pos + glm::vec3( 0, 0,-1), glm::vec3(0,-1, 0));

            m_FBO = new epriv::FramebufferObject(super->name() + " _ProbeFBO",m_EnvMapSize,m_EnvMapSize,ImageInternalFormat::Depth16);
            m_FBO->check();
        }
        void _destruct(){
            glDeleteTextures(1,&m_TextureEnvMap);
            glDeleteTextures(1,&m_TextureConvolutionMap);
            glDeleteTextures(1,&m_TexturePrefilterMap);

            bindTexture(GL_TEXTURE_CUBE_MAP,0);
            SAFE_DELETE(m_FBO);
        }
        void _update(float dt,LightProbe& super,glm::mat4& viewMatrix){
            super.m_View = viewMatrix;
            super.m_Orientation = glm::conjugate(glm::quat_cast(super.m_View));
            super.m_Forward = Engine::Math::getForward(super.m_Orientation);
            super.m_Up = Engine::Math::getUp(super.m_Orientation);
            super.m_Right = glm::normalize(glm::cross(super.m_Forward,super.m_Up));

            if(super.m_Parent){
                super.m_Model = super.m_Parent->getModel(); 
            }else{
                super.m_Model = glm::mat4(1.0f);
            }
            glm::mat4 translationMatrix = glm::translate(super.getPosition());
            glm::mat4 rotationMatrix = glm::mat4_cast(super.m_Orientation);

            super.m_Model = translationMatrix * rotationMatrix * super.m_Model;
            super._constructFrustrum();
        }
        void _renderScene(LightProbe& super,glm::mat4& viewMatrix,uint& i){
            m_FBO->bind();
            _update(0,super,viewMatrix);
            glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,m_TextureEnvMap,0);
            Renderer::Settings::clear(true,true,true);
            epriv::Core::m_Engine->m_RenderManager->_render(super,m_EnvMapSize,m_EnvMapSize,false,false,false,false,super->m_Parent,false,m_FBO->address(),0);
            m_FBO->unbind();
        }
        void _renderConvolution(LightProbe& super,glm::mat4& viewMatrix,uint& i,uint& size){
            Engine::Math::removeMatrixPosition(viewMatrix);
            glm::mat4 vp = super.m_Projection * viewMatrix;
            Renderer::sendUniformMatrix4("VP", vp);
            glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,m_TextureConvolutionMap,0);
            Renderer::Settings::clear(true,true,false);
            Skybox::bindMesh();
        }
        void _renderPrefilter(LightProbe& super,glm::mat4& viewMatrix,uint& i,uint& m,uint& mipSize){
            Engine::Math::removeMatrixPosition(viewMatrix);
            glm::mat4 vp = super.m_Projection * viewMatrix;
            Renderer::sendUniformMatrix4("VP", vp);
            glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,m_TexturePrefilterMap,m);
            Renderer::Settings::clear(true,true,false);
            Skybox::bindMesh();
        }
        void _render(LightProbe& super,ShaderP& convolude,ShaderP& prefilter){
            #pragma region FindOutWhatToRender
            if(m_SecondFrame == false){ m_SecondFrame = true; return; }
            if(m_DidFirst == true && m_OnlyOnce == true) return;
            //determine which cube faces to render this frame
            vector<uint> m_Sides;
            if(m_SidesPerFrame == 6){ for(uint i = 0; i < 6; ++i){ m_Sides.push_back(i); } }
            else{ uint _count = 0;
                for(uint i = 0; i < 6; ++i){
                    if(_count >= m_SidesPerFrame) break;
                    if(i == m_CurrentSide){ m_Sides.push_back(m_CurrentSide); m_CurrentSide++; ++_count; }
                    if(m_CurrentSide >= 6) m_CurrentSide = 0;
                }
            }
            //////////////////////////////////////////////
            #pragma endregion

            //render the scene into a cubemap. this will be VERY expensive... (and now it works!)
            #pragma region RenderScene
            epriv::Core::m_Engine->m_RenderManager->_resizeGbuffer(m_EnvMapSize,m_EnvMapSize);
            
            if(m_TexturesMade == 0){
                genAndBindTexture(GL_TEXTURE_CUBE_MAP,m_TextureEnvMap);
                vector<GLubyte> testData(m_EnvMapSize * m_EnvMapSize * 256, 128);
                for (uint i = 0; i < 6; ++i){
                    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X+i,0,GL_RGBA8,m_EnvMapSize,m_EnvMapSize,0,GL_RGBA,GL_UNSIGNED_BYTE,&testData[0]);
                }
                glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

                glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

                glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_BASE_LEVEL, 0); 
                glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_LEVEL, 0); 
                m_TexturesMade++;
                bindTexture(GL_TEXTURE_CUBE_MAP,0);
            }
            else{
                bindTexture(GL_TEXTURE_CUBE_MAP,m_TextureEnvMap);
            }
            Camera* old = Resources::getCurrentScene()->getActiveCamera();
            Resources::getCurrentScene()->setActiveCamera(super);

            if(super->m_Parent != nullptr){
                super->m_Model = super->m_Parent->getModel(); 
            }else{
                super->m_Model = glm::mat4(1.0f);
            }
            glm::mat4 translationMatrix = glm::translate(super->getPosition());
            glm::mat4 rotationMatrix = glm::mat4_cast(super->m_Orientation);
            super->m_Model = translationMatrix * rotationMatrix * super->m_Model;
            super->_constructFrustrum();

            glm::vec3 pos = super->getPosition();
            m_Views[0] = glm::lookAt(pos, pos + glm::vec3( 1, 0, 0), glm::vec3(0,-1, 0));
            m_Views[1] = glm::lookAt(pos, pos + glm::vec3(-1, 0, 0), glm::vec3(0,-1, 0));
            m_Views[2] = glm::lookAt(pos, pos + glm::vec3( 0, 1, 0), glm::vec3(0, 0, 1));
            m_Views[3] = glm::lookAt(pos, pos + glm::vec3( 0,-1, 0), glm::vec3(0, 0,-1));
            m_Views[4] = glm::lookAt(pos, pos + glm::vec3( 0, 0, 1), glm::vec3(0,-1, 0));
            m_Views[5] = glm::lookAt(pos, pos + glm::vec3( 0, 0,-1), glm::vec3(0,-1, 0));
            
            for(auto side:m_Sides){
                _renderScene(super,m_Views[side],side);
            }
            Resources::getCurrentScene()->setActiveCamera(old);
            /////////////////////////////////////////////////////////////////
            #pragma endregion

            #pragma region RenderConvolution
            uint size = 32;
            if(m_TexturesMade == 1){
                Renderer::genAndBindTexture(GL_TEXTURE_CUBE_MAP,m_TextureConvolutionMap);
                vector<GLubyte> testData(size * size * 256, 155);
                for (uint i = 0; i < 6; ++i){
                    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, size, size, 0, GL_RGB, GL_FLOAT, &testData[0]);
                }
                glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

                glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

                glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_BASE_LEVEL, 0); 
                glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_LEVEL, 0); 
                m_TexturesMade++;
                bindTexture(GL_TEXTURE_CUBE_MAP,0);
            }else{
                bindTexture(GL_TEXTURE_CUBE_MAP,m_TextureConvolutionMap);
            }
            m_FBO->bind();
            
            convolude->bind();
            Renderer::sendTexture("cubemap",m_TextureEnvMap,0,GL_TEXTURE_CUBE_MAP);

            m_FBO->resize(size,size);

            for(auto side:m_Sides){
                _renderConvolution(super,m_Views[side],side,size);
            }
            #pragma endregion

            #pragma region RenderPrefilter
            //now gen EnvPrefilterMap for specular IBL.
            size = m_EnvMapSize/8;
            if(m_TexturesMade == 2){
                Renderer::genAndBindTexture(GL_TEXTURE_CUBE_MAP,m_TexturePrefilterMap);
                vector<GLubyte> testData(size * size * 256, 255);
                for (uint i = 0; i < 6; ++i){
                    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X+i,0,GL_RGB16F,size,size,0,GL_RGB,GL_FLOAT,&testData[0]);
                }
                glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

                glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

                glGenerateMipmap(GL_TEXTURE_CUBE_MAP); 
                m_TexturesMade++;
            }else{
                bindTexture(GL_TEXTURE_CUBE_MAP,m_TexturePrefilterMap);
            }

            prefilter->bind();
            Renderer::sendTexture("cubemap",m_TextureEnvMap,0,GL_TEXTURE_CUBE_MAP);
            Renderer::sendUniform1f("PiFourDividedByResSquaredTimesSix",12.56637f / float((m_EnvMapSize * m_EnvMapSize)*6));
            Renderer::sendUniform1i("NUM_SAMPLES",32);
            uint maxMipLevels = 5;
            for (uint m = 0; m < maxMipLevels; ++m){
                uint mipSize  = uint(size * glm::pow(0.5,m));
                m_FBO->resize(mipSize,mipSize);
                float roughness = (float)m/(float)(maxMipLevels-1);
                Renderer::sendUniform1f("roughness",roughness);
                float a = roughness * roughness;
                Renderer::sendUniform1f("a2",a*a);
                for(auto side:m_Sides){
                    _renderPrefilter(super,m_Views[side],side,m,mipSize);
                }
            }
            m_FBO->unbind();
            #pragma endregion
            m_DidFirst = true;
        }
};

LightProbe::LightProbe(string n, uint envMapSize,glm::vec3 pos,bool onlyOnce,Scene* scene,uint sidesPerFrame):Camera(glm::radians(90.0f),1.0f,0.01f,9999999.0f,scene),m_i(new impl){
    this->setPosition(pos);
    m_i->_init(envMapSize,this,onlyOnce,scene,sidesPerFrame);    
    if(scene == nullptr){
        scene = Resources::getCurrentScene();
    }
    scene->m_LightProbes.emplace(name(),this);
}
LightProbe::~LightProbe(){
    m_i->_destruct();
}
void LightProbe::renderCubemap(ShaderP* convolude,ShaderP* prefilter){ m_i->_render(this,convolude,prefilter); }
const uint LightProbe::getEnvMapSize() const{ return m_i->m_EnvMapSize; }
GLuint LightProbe::getEnvMap(){ return m_i->m_TextureEnvMap; }
GLuint LightProbe::getIrriadianceMap(){ return m_i->m_TextureConvolutionMap; }
GLuint LightProbe::getPrefilterMap(){ return m_i->m_TexturePrefilterMap; }
*/