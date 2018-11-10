#include "Planet.h"
#include "SolarSystem.h"
#include "core/engine/Engine.h"
#include "core/engine/Engine_GLStateMachine.h"
#include "core/engine/Engine_Math.h"
#include "core/Light.h"
#include "core/ShaderProgram.h"
#include "core/Mesh.h"
#include "core/MeshInstance.h"
#include "core/Material.h"
#include "core/Camera.h"
#include "core/Scene.h"
#include "core/Texture.h"

#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <SFML/Graphics.hpp>

#include "ResourceManifest.h"

#include <iostream>

using namespace Engine;
using namespace std;

float PlanetaryRenderSpace(float& outerRadius,float& _distanceReal) {
    //2.718281828459045235360287471352 = euler's number
    float _factor = 1.0f - glm::smoothstep(0.0f, glm::pow(outerRadius, 0.67f) * 215.0f, _distanceReal);
    _factor = glm::clamp(_factor, 0.01f, 0.99f);
    _factor *= glm::log2(_factor + 1.0f);
    _factor *= glm::exp(_factor) * _factor;
    _factor = glm::clamp(_factor, 0.0005f + (outerRadius * 0.0000000016f), 1.0f);
    return _factor;
}


struct PlanetLogicFunctor final {void operator()(ComponentLogic& _component, const float& dt) const {
    Planet& planet = *(Planet*)_component.getUserPointer();
    if (planet.m_RotationInfo) {
        planet.m_Entity.getComponent<ComponentBody>()->rotate(0.0f, glm::radians(planet.m_RotationInfo->speed * dt), 0.0f);
    }
    if (planet.m_OrbitInfo) {
        //planet.m_OrbitInfo->setOrbitalPosition(((1.0f/(planet.m_OrbitInfo->info.y*86400.0f))*dt)*6.283188f,this);
    }
}};

struct PlanetaryRingMeshInstanceBindFunctor{void operator()(EngineResource* r) const {
    MeshInstance& i = *(MeshInstance*)r;
    Planet& obj = *(Planet*)i.getUserPointer();
    
    Camera* c = Resources::getCurrentScene()->getActiveCamera();
    
    float atmosphereHeight = obj.getAtmosphereHeight();


    auto* m_Body = obj.m_Entity.getComponent<ComponentBody>();

    glm::vec3 pos = m_Body->position();
    glm::quat orientation = m_Body->rotation();
    glm::vec3 camPosR = c->getPosition();
    glm::vec3 camPos = camPosR - pos;
    float camHeight = glm::length(camPos);
    float camHeight2 = camHeight*camHeight;

    int numberSamples = 1;
    
    glm::mat4 rot = glm::mat4(1.0f);
    rot *= glm::mat4_cast(orientation);
    
    glm::vec3 lightPos = epriv::InternalScenePublicInterface::GetLights(*Resources::getCurrentScene())[0]->position();
    glm::vec3 lightDir = glm::normalize(lightPos - pos);
    float Km = 0.0025f;
    float Kr = 0.0015f;
    float ESun = 20.0f;
    glm::vec3 scl = m_Body->getScale();
    
    float fScaledepth = 0.25f;
    float innerRadius = obj.getGroundRadius() * 0.5f; //includes rings too
    float outerRadius = obj.getRadius();

    glm::mat4 model = m_Body->modelMatrix();

    //TODO: experimental, simulation space to render space to help with depth buffer (a non-log depth buffer)
    /*
    float _distanceReal = glm::abs(glm::distance(camPosR, pos));
    float _factor = PlanetaryRenderSpace(outerRadius, _distanceReal);
    float _distance = _factor * _distanceReal;
    glm::vec3 _newPosition = glm::normalize(camPosR - pos) * _distance;
    float _newScale = innerRadius * _factor;
    model = glm::mat4(1.0f);
    model = glm::translate(model,camPosR - _newPosition);
    model *= glm::mat4_cast(orientation);
    model = glm::scale(model,glm::vec3(_newScale));
    */

    outerRadius += (outerRadius *  0.025f);
    Renderer::sendUniform1("HasAtmosphere",0);   

    float fScale = 1.0f / (outerRadius - innerRadius);
    glm::vec3 v3InvWaveLength = glm::vec3(1.0f/glm::pow(0.65f,4.0f),1.0f/glm::pow(0.57f,4.0f),1.0f/glm::pow(0.475f,4.0f));


    Renderer::sendUniform1("fromAtmosphere", 0); 
    Renderer::sendUniform4Safe("Object_Color",i.color());
    Renderer::sendUniform3Safe("Gods_Rays_Color",i.godRaysColor());
    Renderer::sendUniform1("nSamples", numberSamples); 
    float exposure = 2.0f;
    Renderer::sendUniformMatrix4("Rot",rot); 

    Renderer::sendUniform4("VertDataMisc1",camPos.x,camPos.y,camPos.z,lightDir.x);
    Renderer::sendUniform4("VertDataMisc2",camPosR.x,camPosR.y,camPosR.z,lightDir.y);
    Renderer::sendUniform4("VertDataMisc3",v3InvWaveLength.x,v3InvWaveLength.y,v3InvWaveLength.z,lightDir.z);
    Renderer::sendUniform4("VertDataScale",fScale,fScaledepth,fScale / fScaledepth,float(numberSamples));
    Renderer::sendUniform4("VertDataRadius",camHeight2,outerRadius,outerRadius*outerRadius,innerRadius);
    Renderer::sendUniform4("VertDatafK",Kr * ESun,Km * ESun,Kr * 12.56637061435916f,Km * 12.56637061435916f); //12.56637061435916 = 4 * pi

    Renderer::sendUniform4("FragDataMisc1",lightPos.x,lightPos.y,lightPos.z,exposure);
    Renderer::sendUniformMatrix4Safe("Model",model);
}};

struct StarMeshInstanceBindFunctor{void operator()(EngineResource* r) const {
    MeshInstance& i = *(MeshInstance*)r;
    Planet& obj = *(Planet*)i.getUserPointer();
    auto* m_Body = obj.m_Entity.getComponent<ComponentBody>();
    Camera* c = Resources::getCurrentScene()->getActiveCamera();
    glm::vec3 pos = m_Body->position();
    glm::vec3 camPosR = c->getPosition();
    glm::quat orientation = m_Body->rotation();

    Renderer::sendUniform4Safe("Object_Color",i.color());
    Renderer::sendUniform3Safe("Gods_Rays_Color",i.godRaysColor());
    Renderer::sendUniform1Safe("AnimationPlaying",0);
    glm::mat4 model = i.model();
    float outerRadius = obj.getRadius();

    model = glm::mat4(1.0f);
    model = glm::translate(model, pos);
    model *= glm::mat4_cast(orientation);
    model = glm::scale(model, glm::vec3(outerRadius));

    //TODO: experimental, simulation space to render space to help with depth buffer (a non-log depth buffer)
    /*
    float _distanceReal = glm::abs(glm::distance(camPosR, pos));
    float _factor = PlanetaryRenderSpace(outerRadius, _distanceReal);
    float _distance = _factor * _distanceReal;
    glm::vec3 _newPosition = glm::normalize(camPosR - pos) * _distance;
    float _newScale = obj.getRadius() * _factor;
    model = glm::mat4(1.0f);
    model = glm::translate(model,camPosR - _newPosition);
    model *= glm::mat4_cast(orientation);
    model = glm::scale(model,glm::vec3(_newScale));
    */

    glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(model)));
    
    Renderer::sendUniformMatrix4Safe("Model",model);
    Renderer::sendUniformMatrix3Safe("NormalMatrix",normalMatrix);
}};

struct AtmosphericScatteringGroundMeshInstanceBindFunctor{void operator()(EngineResource* r) const {
    MeshInstance& i = *(MeshInstance*)r;
    Planet& obj = *(Planet*)i.getUserPointer();
    Camera* c = Resources::getCurrentScene()->getActiveCamera();
    auto* m_Body = obj.m_Entity.getComponent<ComponentBody>();
    float atmosphereHeight = obj.getAtmosphereHeight();

    glm::vec3 pos = m_Body->position();
    glm::quat orientation = m_Body->rotation();
    glm::vec3 camPosR = c->getPosition();
    glm::vec3 camPos = camPosR - pos;
    float camHeight = glm::length(camPos);
    float camHeight2 = camHeight*camHeight;

    int numberSamples = 1;
    
    glm::mat4 rot = glm::mat4(1.0f);
    rot *= glm::mat4_cast(orientation);
    
    glm::vec3 lightPos = epriv::InternalScenePublicInterface::GetLights(*Resources::getCurrentScene())[0]->position();
    glm::vec3 lightDir = glm::normalize(lightPos - pos);
    float Km = 0.0025f;
    float Kr = 0.0015f;
    float ESun = 20.0f;
    glm::vec3 scl = m_Body->getScale();
    
    float fScaledepth = 0.25f;
    float innerRadius = obj.getGroundRadius(); //includes rings too
    float outerRadius = obj.getRadius();

    glm::mat4 model = m_Body->modelMatrix();

    //TODO: experimental, simulation space to render space to help with depth buffer (a non-log depth buffer)
    /*
    float _distanceReal = glm::abs(glm::distance(camPosR, pos));
    float _factor = PlanetaryRenderSpace(innerRadius, _distanceReal);
    float _distance = _factor * _distanceReal;
    glm::vec3 _newPosition = glm::normalize(camPosR - pos) * _distance;
    float _newScale = scl.x * _factor;
    model = glm::mat4(1.0f);
    model = glm::translate(model,camPosR - _newPosition);
    model *= glm::mat4_cast(orientation);
    model = glm::scale(model,glm::vec3(_newScale));
    */

    if(atmosphereHeight <= 0){
        outerRadius += (outerRadius *  0.025f);
        Renderer::sendUniform1("HasAtmosphere",0);   
    }else{
        Renderer::sendUniform1("HasAtmosphere",1);   
    }
    float fScale = 1.0f / (outerRadius - innerRadius);
    glm::vec3 v3InvWaveLength = glm::vec3(1.0f/glm::pow(0.65f,4.0f),1.0f/glm::pow(0.57f,4.0f),1.0f/glm::pow(0.475f,4.0f));

    //pass ground based parameters to the gpu
    if(camHeight <= outerRadius){
        Renderer::sendUniform1("fromAtmosphere", 1);
    }else{
        Renderer::sendUniform1("fromAtmosphere", 0);
    }
    Renderer::sendUniform4Safe("Object_Color",i.color());
    Renderer::sendUniform3Safe("Gods_Rays_Color",i.godRaysColor());

    Renderer::sendUniform1("nSamples", numberSamples); 
    float exposure = 2.0f;
    Renderer::sendUniformMatrix4("Rot",rot);

    Renderer::sendUniform4("VertDataMisc1",camPos.x,camPos.y,camPos.z,lightDir.x);
    Renderer::sendUniform4("VertDataMisc2",camPosR.x,camPosR.y,camPosR.z,lightDir.y);
    Renderer::sendUniform4("VertDataMisc3",v3InvWaveLength.x,v3InvWaveLength.y,v3InvWaveLength.z,lightDir.z);
    Renderer::sendUniform4("VertDataScale",fScale,fScaledepth,fScale / fScaledepth,float(numberSamples));
    Renderer::sendUniform4("VertDataRadius",camHeight2,outerRadius,outerRadius*outerRadius,innerRadius);
    Renderer::sendUniform4("VertDatafK",Kr * ESun,Km * ESun,Kr * 12.56637061435916f,Km * 12.56637061435916f); //12.56637061435916 = 4 * pi

    Renderer::sendUniform4("FragDataMisc1",lightPos.x,lightPos.y,lightPos.z,exposure);
    Renderer::sendUniformMatrix4Safe("Model",model);
}};

struct AtmosphericScatteringGroundMeshInstanceUnbindFunctor{void operator()(EngineResource* r) const {
}};

struct AtmosphericScatteringSkyMeshInstanceBindFunctor{void operator()(EngineResource* r) const {
    MeshInstance& i = *(MeshInstance*)r;
    Planet& obj = *(Planet*)i.getUserPointer();
    Camera* c = Resources::getCurrentScene()->getActiveCamera();
    auto& m_Body = *obj.m_Entity.getComponent<ComponentBody>();
    float atmosphereHeight = obj.getAtmosphereHeight();

    glm::vec3 thisPos = m_Body.position();
    glm::quat orientation = m_Body.rotation();
    glm::vec3 camPosR = c->getPosition();
    glm::vec3 camPos = camPosR - thisPos;
    float camHeight = glm::length(camPos);
    float camHeight2 = camHeight * camHeight;

    int numberSamples = 1;
    
    glm::vec3 lightPos = epriv::InternalScenePublicInterface::GetLights(*Resources::getCurrentScene())[0]->position();
    glm::vec3 lightDir = glm::normalize(lightPos - thisPos);
    float Km = 0.0025f;
    float Kr = 0.0015f;
    float ESun = 20.0f;
    
    
    float fScaledepth = 0.25f;
    float innerRadius = obj.getGroundRadius();  
    float outerRadius = obj.getRadius();
    glm::vec3 scl = m_Body.getScale();
    //glm::vec3 scl = glm::vec3(outerRadius);
    float fScale = 1.0f / (outerRadius - innerRadius);
    float fScaleOverDepth = fScale / fScaledepth;
    float fDepth = glm::exp(fScaleOverDepth * (innerRadius - camHeight));
    float exposure = 2.0f;
    float g = -0.98f;
    glm::vec3 v3InvWaveLength = glm::vec3(1.0f/glm::pow(0.65f,4.0f),1.0f/glm::pow(0.57f,4.0f),1.0f/glm::pow(0.475f,4.0f));

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, thisPos);
    model = glm::scale(model, scl);
    model = glm::scale(model, glm::vec3(1.0f + atmosphereHeight));

    
    //experimental, simulation space to render space to help with depth buffer (a non-log depth buffer)
    /*
    float _distanceReal = glm::abs(glm::distance(camPosR, thisPos));
    float _factor = PlanetaryRenderSpace(outerRadius, _distanceReal);
    float _distance = _factor * _distanceReal;
    glm::vec3 _newPosition = glm::normalize(camPosR - thisPos) * _distance;
    float _newScale = scl.x * _factor;
    model = glm::mat4(1.0f);
    model = glm::translate(model,camPosR - _newPosition);
    model = glm::scale(model,glm::vec3(_newScale));
    model = glm::scale(model,glm::vec3(1.0f + atmosphereHeight));
    */
    
    ShaderP* program;
    //and now render the atmosphere
    if(camHeight > outerRadius){
        program = Resources::getShaderProgram(ResourceManifest::skyFromSpace); 
    }else{
        program = Resources::getShaderProgram(ResourceManifest::skyFromAtmosphere);
    }
    program->bind();

    Renderer::Settings::cullFace(GL_FRONT);
    Renderer::GLEnable(GLState::BLEND);


    Renderer::sendUniformMatrix4Safe("Model", model);
    Renderer::sendUniform1("nSamples", numberSamples);
    Renderer::sendUniform4("VertDataMisc1", camPos.x, camPos.y, camPos.z, lightDir.x);
    Renderer::sendUniform4("VertDataMisc2", camHeight, camHeight2, fDepth, lightDir.y);
    Renderer::sendUniform4("VertDataMisc3", v3InvWaveLength.x, v3InvWaveLength.y, v3InvWaveLength.z, lightDir.z);
    Renderer::sendUniform4("VertDataScale", fScale, fScaledepth, fScaleOverDepth, float(numberSamples));
    Renderer::sendUniform4("VertDataRadius", outerRadius, outerRadius * outerRadius, innerRadius, innerRadius * innerRadius);
    Renderer::sendUniform4("VertDatafK", Kr * ESun, Km * ESun, Kr * 12.56637061435916f, Km * 12.56637061435916f); //12.56637061435916 = 4 * pi
    Renderer::sendUniform4("FragDataGravity", g, g * g, exposure, 0.0f);
}};

struct AtmosphericScatteringSkyMeshInstanceUnbindFunctor{void operator()(EngineResource* r) const {
    Renderer::Settings::cullFace(GL_BACK);
    Renderer::GLDisable(GLState::BLEND);
}};

Planet::Planet(Handle& mat,PlanetType::Type type,glm::vec3 pos,float scl,string name,float atmosphere, SolarSystem* scene):EntityWrapper(*scene){
    auto* m_Model = m_Entity.addComponent<ComponentModel>(ResourceManifest::PlanetMesh,mat,ResourceManifest::groundFromSpace);
    auto& model = m_Model->getModel();
    model.setUserPointer(this);

    auto* m_Body = m_Entity.addComponent<ComponentBody>();
    m_Body->setScale(scl, scl, scl);
    m_Body->setPosition(pos);

    m_Entity.addComponent<ComponentName>(name);

    m_AtmosphereHeight = atmosphere;
    if(type != PlanetType::Star){
        model.setCustomBindFunctor(AtmosphericScatteringGroundMeshInstanceBindFunctor());
        model.setCustomUnbindFunctor(AtmosphericScatteringGroundMeshInstanceUnbindFunctor());
    }
    if(m_AtmosphereHeight > 0){
        uint index = m_Model->addModel(ResourceManifest::PlanetMesh,ResourceManifest::EarthSkyMaterial,(ShaderP*)ResourceManifest::skyFromSpace.get(),RenderStage::GeometryTransparent);
        MeshInstance& skyMesh = m_Model->getModel(index);
        float aScale = model.getScale().x;
        aScale = aScale + (aScale * m_AtmosphereHeight);
        skyMesh.setCustomBindFunctor(AtmosphericScatteringSkyMeshInstanceBindFunctor());
        skyMesh.setCustomUnbindFunctor(AtmosphericScatteringSkyMeshInstanceUnbindFunctor());
        skyMesh.setScale(aScale,aScale,aScale);
        skyMesh.setUserPointer(this);
    }

    m_Type = type;
    m_OrbitInfo = nullptr;
    m_RotationInfo = nullptr;

    m_Entity.addComponent<ComponentLogic>(PlanetLogicFunctor(), this);

    scene->m_Objects.push_back(this);
}
Planet::~Planet(){  
    SAFE_DELETE_VECTOR(m_Rings);
    SAFE_DELETE(m_OrbitInfo);
    SAFE_DELETE(m_RotationInfo);
}
glm::vec3 Planet::getPosition(){ return m_Entity.getComponent<ComponentBody>()->position(); }
void Planet::setPosition(float x,float y,float z){ m_Entity.getComponent<ComponentBody>()->setPosition(x,y,z); }
void Planet::setPosition(glm::vec3 pos){ m_Entity.getComponent<ComponentBody>()->setPosition(pos); }
void Planet::setOrbit(OrbitInfo* o){ 
    m_OrbitInfo = o; 
    m_Entity.getComponent<ComponentLogic>()->call(0);
}
void Planet::setRotation(RotationInfo* r){ 
    m_RotationInfo = r;
    m_Entity.getComponent<ComponentBody>()->rotate(glm::radians(-r->tilt),0.0f,0.0f);
}
void Planet::addRing(Ring* ring){ m_Rings.push_back(ring); }
glm::vec2 Planet::getGravityInfo(){ return glm::vec2(getRadius() * 5,getRadius() * 7); }
OrbitInfo* Planet::getOrbitInfo() const { return m_OrbitInfo; }
float Planet::getGroundRadius(){ auto& model = *m_Entity.getComponent<ComponentModel>(); return model.radius(); }
float Planet::getRadius() { auto& model = *m_Entity.getComponent<ComponentModel>(); return model.radius() + (model.radius() * m_AtmosphereHeight); }
float Planet::getAtmosphereHeight(){ return m_AtmosphereHeight; }

Star::Star(glm::vec3 starColor,glm::vec3 lightColor,glm::vec3 pos,float scl,string name, SolarSystem* scene):Planet(ResourceManifest::StarMaterial,PlanetType::Star,pos,scl,name,0.0f,scene){
    m_Light = new SunLight(glm::vec3(0.0f),LightType::Sun,scene);
    m_Light->setColor(lightColor);

    auto& model = (*m_Entity.getComponent<ComponentModel>()).getModel();

    model.setColor(starColor);
    model.setGodRaysColor(lightColor);
    model.setShaderProgram(nullptr);
    model.setCustomBindFunctor(StarMeshInstanceBindFunctor());

    //addChild(m_Light);
    m_Light->setPosition(pos);
    scene->m_Objects.push_back(m_Light);
    if(!Renderer::Settings::GodRays::getObject()){
        Renderer::Settings::GodRays::setObject(&m_Entity);
    }
}
Star::~Star(){
}
Ring::Ring(vector<RingInfo>& rings,Planet* parent){
    m_Parent = parent;
    _makeRingImage(rings);
    m_Parent->addRing(this);

    auto& model = *m_Parent->m_Entity.getComponent<ComponentModel>();

    uint index = model.addModel(ResourceManifest::RingMesh,m_MaterialHandle,(ShaderP*)ResourceManifest::groundFromSpace.get(), RenderStage::GeometryTransparent);
    MeshInstance& ringMesh = model.getModel(index);
    ringMesh.setCustomBindFunctor(PlanetaryRingMeshInstanceBindFunctor());
    float aScale = 1.0f;
    ringMesh.setScale(aScale,aScale,aScale);
    ringMesh.setUserPointer(parent);
}
Ring::~Ring(){
}
void Ring::_makeRingImage(vector<RingInfo>& rings){
    sf::Image ringImage;
    ringImage.create(1024, 2, sf::Color(0,0,0,0));
    uint count = 0;
    for(auto& ringInfo: rings){
        glm::vec4 pC = glm::vec4((float)ringInfo.color.r, (float)ringInfo.color.g, (float)ringInfo.color.b, 255.0f) / 255.0f;
        uint alphaChangeRange = ringInfo.size - ringInfo.alphaBreakpoint;
        uint newI = 0;
        for(uint i = 0; i < ringInfo.size; ++i){
            if (i > ringInfo.alphaBreakpoint) {
                pC.a = float(alphaChangeRange - newI) / (float)alphaChangeRange;
                ++newI;
            }else{
                pC.a = 1;
            } 

            int xBack = ringInfo.position - i;
            int xFront = ringInfo.position + i;
            if (xBack > 0 && xFront < (int)ringImage.getSize().x) {
                sf::Color fFront, fBack;
                sf::Color bgFrontPixel = ringImage.getPixel(xFront, 0);
                sf::Color bgBackPixel = ringImage.getPixel(xBack, 0);
                glm::vec4 bgColorFront = glm::vec4(bgFrontPixel.r, bgFrontPixel.g, bgFrontPixel.b, bgFrontPixel.a) / 255.0f;
                glm::vec4 bgColorBack = glm::vec4(bgBackPixel.r, bgBackPixel.g, bgBackPixel.b, bgBackPixel.a) / 255.0f;
                glm::vec4 _fcf = Engine::Math::PaintersAlgorithm(pC, bgColorFront);
                glm::vec4 _fcb = Engine::Math::PaintersAlgorithm(pC, bgColorBack); 
                if (ringInfo.color.r < 0 || ringInfo.color.g < 0 || ringInfo.color.b < 0) {
                    //transparent color, removing the BG color 
                    _fcf = glm::vec4(bgColorFront.r, bgColorFront.g, bgColorFront.b, 1.0f - pC.a);
                    _fcb = glm::vec4(bgColorBack.r, bgColorBack.g, bgColorBack.b, 1.0f - pC.a);
                }
                fFront = sf::Color(sf::Uint8(_fcf.r * 255.0f), sf::Uint8(_fcf.g * 255.0f), sf::Uint8(_fcf.b * 255.0f), sf::Uint8(_fcf.a * 255.0f));
                fBack = sf::Color(sf::Uint8(_fcb.r * 255.0f), sf::Uint8(_fcb.g * 255.0f), sf::Uint8(_fcb.b * 255.0f), sf::Uint8(_fcb.a * 255.0f));

                for (uint s = 0; s < ringImage.getSize().y; ++s) { 
                    ringImage.setPixel(xFront, s, fFront);
                    ringImage.setPixel(xBack, s, fBack);
                }
            }
        }
        ++count;
    }
    Texture* diffuse = new Texture(ringImage,"RingDiffuse",false,ImageInternalFormat::SRGB8_ALPHA8);
    diffuse->setAnisotropicFiltering(2.0f);
    epriv::Core::m_Engine->m_ResourceManager._addTexture(diffuse);
    m_MaterialHandle = Resources::addMaterial("RingMaterial", diffuse, nullptr, nullptr, nullptr);
	((Material*)m_MaterialHandle.get())->setSpecularModel(SpecularModel::None);
}
OrbitInfo::OrbitInfo(float _eccentricity, float _days, float _majorRadius,float _angle,Planet& _parent,float _inclination){
    //x = eccentricity, y = days, z = minorRadius, w = majorRadius
    angle = _angle;
    inclination = glm::radians(_inclination);
    info.x = _eccentricity;
    info.y = _days;
    info.w = _majorRadius;
    info.z = glm::sqrt(_majorRadius * _majorRadius * (1.0f - (_eccentricity * _eccentricity)));
    parent = &_parent;
}
glm::vec3 OrbitInfo::getOrbitalPosition(float angle,Planet* thisPlanet){
    glm::vec3 offset = glm::vec3(0.0f);
    glm::vec3 currentPos = thisPlanet->getPosition();
    if(parent){
        glm::vec3 parentPos = parent->getPosition();
        float newX = parentPos.x - glm::cos(angle) * info.w;
        float newZ = parentPos.z - glm::sin(angle) * info.z;
        offset = glm::vec3(newX - currentPos.x,0.0f,newZ - currentPos.z);
    }
    return (currentPos + offset);
}
void OrbitInfo::setOrbitalPosition(float a,Planet* planet){
    angle += a;
    glm::vec3 nextPos = getOrbitalPosition(angle,planet);
    glm::mat4 modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::rotate(modelMatrix,inclination,glm::vec3(0,1,0));
    modelMatrix = glm::translate(modelMatrix,nextPos);
    planet->setPosition(modelMatrix[3][0],modelMatrix[3][1],modelMatrix[3][2]);
}
