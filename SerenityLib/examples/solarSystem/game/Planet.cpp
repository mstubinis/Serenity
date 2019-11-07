#include "Planet.h"
#include "map/Map.h"
#include "ResourceManifest.h"

#include <core/engine/Engine.h>
#include <core/engine/math/Engine_Math.h>
#include <core/engine/lights/Lights.h>
#include <core/engine/shaders/ShaderProgram.h>
#include <core/engine/mesh/Mesh.h>
#include <core/ModelInstance.h>
#include <core/engine/materials/Material.h>
#include <core/engine/scene/Camera.h>
#include <core/engine/scene/Scene.h>
#include <core/engine/textures/Texture.h>

#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <SFML/Graphics.hpp>

#include <iostream>

using namespace Engine;
using namespace std;

unordered_map<PlanetType::Type, Handle> Planets::IconDatabase;

void Planets::init() {
    
    Planets::IconDatabase[PlanetType::Rocky] = Resources::loadTextureAsync("../data/Textures/HUD/RockyIcon.dds");
    Planets::IconDatabase[PlanetType::GasGiant] = Resources::loadTextureAsync("../data/Textures/HUD/GasGiantIcon.dds");
    Planets::IconDatabase[PlanetType::GasGiantRinged] = Resources::loadTextureAsync("../data/Textures/HUD/GasGiantRingedIcon.dds");
    Planets::IconDatabase[PlanetType::Moon] = Resources::loadTextureAsync("../data/Textures/HUD/MoonIcon.dds");
    Planets::IconDatabase[PlanetType::Star] = Resources::loadTextureAsync("../data/Textures/HUD/StarIcon.dds");
    //Planets::IconDatabase[PlanetType::Asteroid] = Resources::loadTextureAsync("../data/Textures/HUD/AsteroidIcon.dds");
    
}

float PlanetaryRenderSpace(float& outerRadius,float& _distanceReal) {
    //2.718281828459045235360287471352 = euler's number
    float _factor = 1.0f - glm::smoothstep(0.0f, glm::pow(outerRadius, 0.67f) * 215.0f, _distanceReal);
    _factor = glm::clamp(_factor, 0.01f, 0.99f);
    _factor *= glm::log2(_factor + 1.0f);
    _factor *= glm::exp(_factor) * _factor;
    _factor = glm::clamp(_factor, 0.0005f + (outerRadius * 0.0000000016f), 1.0f);
    return _factor;
}

struct PlanetLogicFunctor final {void operator()(ComponentLogic& _component, const double& dt) const {
    Planet& planet = *(Planet*)_component.getUserPointer();
    if (planet.m_RotationInfo) {
        planet.m_Entity.getComponent<ComponentBody>()->rotate(0.0f, glm::radians(planet.m_RotationInfo->speed * static_cast<float>(dt)), 0.0f);
    }
    if (planet.m_OrbitInfo) {
        //planet.m_OrbitInfo->setOrbitalPosition(((1.0f/(planet.m_OrbitInfo->info.y*86400.0f))*dt)*6.283188f, planet);
    }
}};

struct PlanetaryRingModelInstanceBindFunctor{void operator()(EngineResource* r) const {
    ModelInstance& i = *(ModelInstance*)r;
    Planet& obj = *(Planet*)i.getUserPointer(); 
    Camera* c = Resources::getCurrentScene()->getActiveCamera();
    float atmosphereHeight = obj.getAtmosphereHeight();
    auto& m_Body = *obj.getComponent<ComponentBody>();

    glm::vec3 pos = m_Body.position();
    glm::quat orientation = m_Body.rotation();
    glm::vec3 camPosR = c->getPosition();
    glm::vec3 camPos = camPosR - pos;
    float camHeight = glm::length(camPos);
    float camHeight2 = camHeight*camHeight;

    int numberSamples = 1;
    
    glm::mat4 rot = glm::mat4(1.0f);
    rot *= glm::mat4_cast(orientation);
    
    glm::vec3 lightPos = epriv::InternalScenePublicInterface::GetSunLights(*Resources::getCurrentScene())[0]->position();
    glm::vec3 lightDir = glm::normalize(lightPos - pos);
    float Km = 0.0025f;
    float Kr = 0.0015f;
    float ESun = 20.0f;
    glm::vec3 scl = m_Body.getScale();
    
    float fScaledepth = 0.25f;
    float innerRadius = obj.getGroundRadius() * 0.5f; //includes rings too
    float outerRadius = obj.getRadius();

    glm::mat4 model = m_Body.modelMatrixRendering();

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
    Renderer::sendUniform1Safe("HasAtmosphere",0);   

    float fScale = 1.0f / (outerRadius - innerRadius);
    glm::vec3 v3InvWaveLength = glm::vec3(1.0f/glm::pow(0.65f,4.0f),1.0f/glm::pow(0.57f,4.0f),1.0f/glm::pow(0.475f,4.0f));


    Renderer::sendUniform1Safe("fromAtmosphere", 0); 
    Renderer::sendUniform4Safe("Object_Color",i.color());
    Renderer::sendUniform3Safe("Gods_Rays_Color",i.godRaysColor());
    Renderer::sendUniform1Safe("nSamples", numberSamples); 
    float exposure = 2.0f;
    Renderer::sendUniformMatrix4Safe("RotMatrix",rot); 

    Renderer::sendUniform4Safe("VertDataMisc1",camPos.x,camPos.y,camPos.z,lightDir.x);
    Renderer::sendUniform4Safe("VertDataMisc2",camPosR.x,camPosR.y,camPosR.z,lightDir.y);
    Renderer::sendUniform4Safe("VertDataMisc3",v3InvWaveLength.x,v3InvWaveLength.y,v3InvWaveLength.z,lightDir.z);
    Renderer::sendUniform4Safe("VertDataScale",fScale,fScaledepth,fScale / fScaledepth,float(numberSamples));
    Renderer::sendUniform4Safe("VertDataRadius",camHeight2,outerRadius,outerRadius*outerRadius,innerRadius);
    Renderer::sendUniform4Safe("VertDatafK",Kr * ESun,Km * ESun,Kr * 12.56637061435916f,Km * 12.56637061435916f); //12.56637061435916 = 4 * pi

    Renderer::sendUniform4Safe("FragDataMisc1",lightPos.x,lightPos.y,lightPos.z,exposure);
    Renderer::sendUniformMatrix4Safe("Model",model);
}};

struct StarModelInstanceBindFunctor{void operator()(EngineResource* r) const {
    ModelInstance& i = *(ModelInstance*)r;
    Planet& obj = *(Planet*)i.getUserPointer();
    auto& m_Body = *obj.getComponent<ComponentBody>();
    Camera& c = *Resources::getCurrentScene()->getActiveCamera();
    glm::vec3 pos = m_Body.position();
    glm::vec3 camPosR = c.getPosition();
    glm::quat orientation = m_Body.rotation();

    Renderer::sendUniform4Safe("Object_Color",i.color());
    Renderer::sendUniform3Safe("Gods_Rays_Color", i.godRaysColor());
    Renderer::sendUniform1Safe("AnimationPlaying",0);
    glm::mat4 modelMatrix = i.modelMatrix();
    float outerRadius = obj.getRadius();

    modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::translate(modelMatrix, pos);
    modelMatrix *= glm::mat4_cast(orientation);
    modelMatrix = glm::scale(modelMatrix, glm::vec3(outerRadius));

    //TODO: experimental, simulation space to render space to help with depth buffer (a non-log depth buffer)
    /*
    float _distanceReal = glm::abs(glm::distance(camPosR, pos));
    float _factor = PlanetaryRenderSpace(outerRadius, _distanceReal);
    float _distance = _factor * _distanceReal;
    glm::vec3 _newPosition = glm::normalize(camPosR - pos) * _distance;
    float _newScale = obj.getRadius() * _factor;
    modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::translate(modelMatrix,camPosR - _newPosition);
    modelMatrix *= glm::mat4_cast(orientation);
    modelMatrix = glm::scale(modelMatrix,glm::vec3(_newScale));
    */

    glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(modelMatrix)));
    
    Renderer::sendUniformMatrix4Safe("Model", modelMatrix);
    Renderer::sendUniformMatrix3Safe("NormalMatrix",normalMatrix);
}};

struct StarModelInstanceUnbindFunctor {void operator()(EngineResource* r) const {
}};

struct AtmosphericScatteringGroundModelInstanceBindFunctor{void operator()(EngineResource* r) const {
    ModelInstance& i = *(ModelInstance*)r;
    Planet& obj = *(Planet*)i.getUserPointer();
    Camera& c = *Resources::getCurrentScene()->getActiveCamera();
    auto& m_Body = *obj.getComponent<ComponentBody>();
    float atmosphereHeight = obj.getAtmosphereHeight();

    glm::vec3 pos = m_Body.position();
    glm::quat orientation = m_Body.rotation();
    glm::vec3 camPosR = c.getPosition();
    glm::vec3 camPos = camPosR - pos;
    float camHeight = glm::length(camPos);
    float camHeight2 = camHeight*camHeight;

    int numberSamples = 1;
    
    glm::mat4 rot = glm::mat4(1.0f);
    rot *= glm::mat4_cast(orientation);
    
    glm::vec3 lightPos = epriv::InternalScenePublicInterface::GetSunLights(*Resources::getCurrentScene())[0]->position();
    glm::vec3 lightDir = glm::normalize(lightPos - pos);
    float Km = 0.0025f;
    float Kr = 0.0015f;
    float ESun = 20.0f;
    glm::vec3 scl = m_Body.getScale();
    
    float fScaledepth = 0.25f;
    float innerRadius = obj.getGroundRadius(); //includes rings too
    float outerRadius = obj.getRadius();

    glm::mat4 model = m_Body.modelMatrixRendering();

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
        Renderer::sendUniform1Safe("HasAtmosphere",0);   
    }else{
        Renderer::sendUniform1Safe("HasAtmosphere",1);   
    }
    float fScale = 1.0f / (outerRadius - innerRadius);
    glm::vec3 v3InvWaveLength = glm::vec3(1.0f/glm::pow(0.65f,4.0f),1.0f/glm::pow(0.57f,4.0f),1.0f/glm::pow(0.475f,4.0f));

    //pass ground based parameters to the gpu
    if(camHeight <= outerRadius){
        Renderer::sendUniform1Safe("fromAtmosphere", 1);
    }else{
        Renderer::sendUniform1Safe("fromAtmosphere", 0);
    }
    Renderer::sendUniform4Safe("Object_Color",i.color());
    Renderer::sendUniform3Safe("Gods_Rays_Color",i.godRaysColor());

    Renderer::sendUniform1Safe("nSamples", numberSamples); 
    float exposure = 2.0f;
    Renderer::sendUniformMatrix4Safe("RotMatrix",rot);

    Renderer::sendUniform4Safe("VertDataMisc1",camPos.x,camPos.y,camPos.z,lightDir.x);
    Renderer::sendUniform4Safe("VertDataMisc2",camPosR.x,camPosR.y,camPosR.z,lightDir.y);
    Renderer::sendUniform4Safe("VertDataMisc3",v3InvWaveLength.x,v3InvWaveLength.y,v3InvWaveLength.z,lightDir.z);
    Renderer::sendUniform4Safe("VertDataScale",fScale,fScaledepth,fScale / fScaledepth,float(numberSamples));
    Renderer::sendUniform4Safe("VertDataRadius",camHeight2,outerRadius,outerRadius*outerRadius,innerRadius);
    Renderer::sendUniform4Safe("VertDatafK",Kr * ESun,Km * ESun,Kr * 12.56637061435916f,Km * 12.56637061435916f); //12.56637061435916 = 4 * pi

    Renderer::sendUniform4Safe("FragDataMisc1",lightPos.x,lightPos.y,lightPos.z,exposure);
    Renderer::sendUniformMatrix4Safe("Model",model);
}};

struct AtmosphericScatteringGroundModelInstanceUnbindFunctor{void operator()(EngineResource* r) const {
}};

struct AtmosphericScatteringSkyModelInstanceBindFunctor{void operator()(EngineResource* r) const {
    ModelInstance& i = *(ModelInstance*)r;
    Planet& obj = *(Planet*)i.getUserPointer();
    Camera& c = *Resources::getCurrentScene()->getActiveCamera();
    auto& m_Body = *obj.getComponent<ComponentBody>();
    float atmosphereHeight = obj.getAtmosphereHeight();

    glm::vec3 thisPos = m_Body.position();
    glm::quat orientation = m_Body.rotation();
    glm::vec3 camPosR = c.getPosition();
    glm::vec3 camPos = camPosR - thisPos;
    float camHeight = glm::length(camPos);
    float camHeight2 = camHeight * camHeight;

    int numberSamples = 1;
    
    glm::vec3 lightPos = epriv::InternalScenePublicInterface::GetSunLights(*Resources::getCurrentScene())[0]->position();
    glm::vec3 lightDir = glm::normalize(lightPos - thisPos);
    float Km = 0.0025f;
    float Kr = 0.0015f;
    float ESun = 20.0f;
    
    float fScaledepth = 0.25f;
    float innerRadius = obj.getGroundRadius();  
    float outerRadius = obj.getRadius();
    glm::vec3 scl = m_Body.getScale();
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
    
    ShaderProgram* program;
    //and now render the atmosphere
    if(camHeight > outerRadius){
        program = Resources::getShaderProgram(ResourceManifest::skyFromSpace); 
    }else{
        program = Resources::getShaderProgram(ResourceManifest::skyFromAtmosphere);
    }
    program->bind();

    Renderer::cullFace(GL_FRONT);

    Renderer::sendUniformMatrix4Safe("Model", model);
    Renderer::sendUniform1Safe("nSamples", numberSamples);
    Renderer::sendUniform4Safe("VertDataMisc1", camPos.x, camPos.y, camPos.z, lightDir.x);
    Renderer::sendUniform4Safe("VertDataMisc2", camHeight, camHeight2, fDepth, lightDir.y);
    Renderer::sendUniform4Safe("VertDataMisc3", v3InvWaveLength.x, v3InvWaveLength.y, v3InvWaveLength.z, lightDir.z);
    Renderer::sendUniform4Safe("VertDataScale", fScale, fScaledepth, fScaleOverDepth, float(numberSamples));
    Renderer::sendUniform4Safe("VertDataRadius", outerRadius, outerRadius * outerRadius, innerRadius, innerRadius * innerRadius);
    Renderer::sendUniform4Safe("VertDatafK", Kr * ESun, Km * ESun, Kr * 12.56637061435916f, Km * 12.56637061435916f); //12.56637061435916 = 4 * pi
    Renderer::sendUniform4Safe("FragDataGravity", g, g * g, exposure, 0.0f);
}};

struct AtmosphericScatteringSkyModelInstanceUnbindFunctor{void operator()(EngineResource* r) const {
    Renderer::cullFace(GL_BACK);
}};

Planet::Planet(Handle& mat, const PlanetType::Type type, const glm_vec3& pos, const decimal scl, const string& name, const float atmosphere, Map* scene, const string& planet_type_name) : EntityWrapper(*scene) {
    auto& componentName = *m_Entity.addComponent<ComponentName>(name);

    auto& body = *addComponent<ComponentBody>();
    body.setPosition(pos);
    auto& model = *addComponent<ComponentModel>(ResourceManifest::PlanetMesh, mat, ResourceManifest::groundFromSpace);
    body.setScale(scl, scl, scl);
    auto& instance = model.getModel();
    instance.setUserPointer(this);

    m_AtmosphereHeight = atmosphere;
    if(type != PlanetType::Star){
        instance.setCustomBindFunctor(AtmosphericScatteringGroundModelInstanceBindFunctor());
        instance.setCustomUnbindFunctor(AtmosphericScatteringGroundModelInstanceUnbindFunctor());
    }
    if(m_AtmosphereHeight > 0){
        auto& skyInstance = model.addModel(ResourceManifest::PlanetMesh, ResourceManifest::EarthSkyMaterial, (ShaderProgram*)ResourceManifest::skyFromSpace.get(), RenderStage::GeometryTransparent);
        float aScale = instance.getScale().x;
        aScale = aScale + (aScale * m_AtmosphereHeight);
        skyInstance.setCustomBindFunctor(AtmosphericScatteringSkyModelInstanceBindFunctor());
        skyInstance.setCustomUnbindFunctor(AtmosphericScatteringSkyModelInstanceUnbindFunctor());
        skyInstance.setScale(aScale,aScale,aScale);
        skyInstance.setUserPointer(this);
    }
    auto& logic = *addComponent<ComponentLogic>(PlanetLogicFunctor(), this);

    m_Type = type;
    if (planet_type_name.empty()) {
        m_TypeName = getPlanetTypeNameAsString();
    }else{
        m_TypeName = planet_type_name;
    }

    m_OrbitInfo = nullptr;
    m_RotationInfo = nullptr;

    scene->m_Objects.push_back(this);
}
Planet::~Planet(){  
    SAFE_DELETE_VECTOR(m_Rings);
    SAFE_DELETE(m_OrbitInfo);
    SAFE_DELETE(m_RotationInfo);
}
const string Planet::getPlanetTypeNameAsString() const {
    switch (m_Type){
        case PlanetType::Asteroid: {
            return "Asteroid";
        }case PlanetType::GasGiant: {
            return "Gas Giant";
        }case PlanetType::GasGiantRinged: {
            return "Ringed Gas Giant";
        }case PlanetType::Moon: {
            return "Moon";
        }case PlanetType::Rocky: {
            return "Rocky Planet";
        }case PlanetType::Star: {
            return "Star";
        }default: {
            return "Planet";
        }
    }
    return "Planet";
}
const string& Planet::getTypeName() const {
    return m_TypeName;
}
const PlanetType::Type& Planet::getType() const {
    return m_Type;
}
const string Planet::getName() {
    return getComponent<ComponentName>()->name();
}
const glm_vec3 Planet::getPosition(){ 
    return getComponent<ComponentBody>()->position(); 
}
void Planet::setPosition(const decimal& x, const decimal& y, const decimal& z){
    getComponent<ComponentBody>()->setPosition(x,y,z); 
}
void Planet::setPosition(const glm_vec3& pos){ 
    getComponent<ComponentBody>()->setPosition(pos); 
}
void Planet::setOrbit(OrbitInfo* o){ 
    m_OrbitInfo = o; 
    getComponent<ComponentLogic>()->call(0);
}
void Planet::setRotation(RotationInfo* r){ 
    m_RotationInfo = r;
    getComponent<ComponentBody>()->rotate(glm::radians(-r->tilt),0.0f,0.0f);
}
void Planet::addRing(Ring* ring){ 
    m_Rings.push_back(ring); 
}
const glm::vec2 Planet::getGravityInfo() {
    return glm::vec2(getRadius() * 5, getRadius() * 7); 
}
OrbitInfo* Planet::getOrbitInfo() const { 
    return m_OrbitInfo; 
}
const float Planet::getGroundRadius() {
    auto& model = *getComponent<ComponentModel>();
    const auto& rad = model.radius();
    return rad - (rad * m_AtmosphereHeight);
}
const float Planet::getRadius() {
    auto& model = *getComponent<ComponentModel>();
    return model.radius(); 
}
const float& Planet::getAtmosphereHeight() const {
    return m_AtmosphereHeight; 
}

Star::Star(const glm::vec3& starColor, const glm::vec3& lightColor, const glm::vec3& godRaysColor, const glm_vec3& pos, const decimal scl, const string& name, Map* scene, const string& type_name) : Planet(ResourceManifest::StarMaterial, PlanetType::Star, pos, scl, name, 0.0f, scene, type_name) {
    m_Light = new SunLight(glm::vec3(0.0f),LightType::Sun,scene);
    m_Light->setColor(lightColor);

    auto* starModel = getComponent<ComponentModel>();
    if (starModel) {
        auto& instance = starModel->getModel();
        instance.setColor(starColor);
        instance.setGodRaysColor(godRaysColor);
        instance.setCustomBindFunctor(StarModelInstanceBindFunctor());
        instance.setCustomUnbindFunctor(StarModelInstanceUnbindFunctor());
        instance.setShaderProgram(nullptr,*starModel);
    }
    //addChild(m_Light);
    m_Light->setPosition(pos);
    scene->m_Objects.push_back(m_Light);
    if(!scene->getGodRaysSun()){
        scene->setGodRaysSun(&m_Entity);
    }
}
Star::~Star(){
}
Ring::Ring(vector<RingInfo>& rings,Planet* parent){
    m_Parent = parent;
    
    _makeRingImage(rings);
    m_Parent->addRing(this);

    auto& model = *m_Parent->getComponent<ComponentModel>();

    ModelInstance& ringInstance = model.addModel(
        ResourceManifest::RingMesh,
        m_MaterialHandle,
        (ShaderProgram*)ResourceManifest::groundFromSpace.get(), 
        RenderStage::GeometryTransparent
    );
    ringInstance.setCustomBindFunctor(PlanetaryRingModelInstanceBindFunctor());
    const float aScale = 1.0f;
    ringInstance.setScale(aScale,aScale,aScale);
    ringInstance.setUserPointer(parent);
    
}
Ring::~Ring(){
}
void Ring::_makeRingImage(const vector<RingInfo>& rings){
    
    sf::Image ringImage;
    ringImage.create(1024, 2, sf::Color(0,0,0,0));
    const auto& ringImageX = ringImage.getSize().x;
    const auto& ringImageY = ringImage.getSize().y;
    for(auto& ringInfo: rings){
        sf::Color paint_color(
			static_cast<sf::Uint8>(ringInfo.color.r),
			static_cast<sf::Uint8>(ringInfo.color.g),
			static_cast<sf::Uint8>(ringInfo.color.b),
            static_cast<sf::Uint8>(255)
		);

        uint alphaChange = ringInfo.size - ringInfo.alphaBreakpoint;
        uint alpha_i = 0;
        for(uint i = 0; i < ringInfo.size; ++i){
            if (i > ringInfo.alphaBreakpoint) {
                paint_color.a = static_cast<sf::Uint8>(((static_cast<float>(alphaChange - alpha_i) / static_cast<float>(alphaChange))) * 255.0f);
                ++alpha_i;
			}else{
				paint_color.a = static_cast<sf::Uint8>(255);
			}
            int xBack  =  ringInfo.position - i;
            int xFront =  ringInfo.position + i;
            if (xBack > 0 && xFront < static_cast<int>(ringImageX)) {
                const sf::Color& canvas_color_front = ringImage.getPixel(xFront, 0);
                const sf::Color& canvas_color_back = ringImage.getPixel(xBack,  0);
                sf::Color finalColorFront = Math::PaintersAlgorithm(paint_color, canvas_color_front);
                sf::Color finalColorBack  = Math::PaintersAlgorithm(paint_color, canvas_color_back);
                if (ringInfo.color.r < 0 && ringInfo.color.g < 0 && ringInfo.color.b < 0) {
                    //transparent color, removing the canvas color 
                    finalColorFront = sf::Color(canvas_color_front.r, canvas_color_front.g, canvas_color_front.b, static_cast<sf::Uint8>(255) - paint_color.a);
                    finalColorBack  = sf::Color(canvas_color_back.r,  canvas_color_back.g,  canvas_color_back.b, static_cast<sf::Uint8>(255) - paint_color.a);
                }
                for (uint s = 0; s < ringImageY; ++s) {
                    ringImage.setPixel(xFront, s, finalColorFront);
                    ringImage.setPixel(xBack,  s, finalColorBack);
                }
            }
        }
    }
    auto* texture = Resources::getTexture("RingDiffuse");
    if (!texture) {
        texture = new Texture(ringImage, "RingDiffuse", false, ImageInternalFormat::SRGB8_ALPHA8);
        texture->setAnisotropicFiltering(2.0f);
        epriv::Core::m_Engine->m_ResourceManager._addTexture(texture);
    }
    auto handle = Resources::loadMaterial("RingMaterial", texture);
    m_MaterialHandle = handle;
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
glm::vec3 OrbitInfo::getOrbitalPosition(float angle,Planet& thisPlanet){
    glm::vec3 offset = glm::vec3(0.0f);
    const glm::vec3& currentPos = thisPlanet.getPosition();
    if(parent){
        const glm::vec3 parentPos = parent->getPosition();
        const float newX = parentPos.x - glm::cos(angle) * info.w;
        const float newZ = parentPos.z - glm::sin(angle) * info.z;
        offset = glm::vec3(newX - currentPos.x, 0.0f, newZ - currentPos.z);
    }
    return (currentPos + offset);
}
void OrbitInfo::setOrbitalPosition(float a,Planet& planet){
    angle += a;
    const glm::vec3& nextPos = getOrbitalPosition(angle,planet);
    glm::mat4 modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::rotate(modelMatrix,inclination,glm::vec3(0,1,0));
    modelMatrix = glm::translate(modelMatrix,nextPos);
    planet.setPosition(modelMatrix[3][0],modelMatrix[3][1],modelMatrix[3][2]);
}
