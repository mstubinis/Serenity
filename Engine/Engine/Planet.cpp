#include "Engine.h"
#include "Engine_Resources.h"
#include "Engine_Renderer.h"
#include "Light.h"
#include "Planet.h"
#include "ShaderProgram.h"
#include "Mesh.h"
#include "MeshInstance.h"
#include "Material.h"
#include "Camera.h"
#include "Scene.h"
#include "Texture.h"

#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "ResourceManifest.h"

using namespace Engine;
using namespace std;

struct AtmosphericScatteringMeshInstanceBindFunctor{void operator()(EngineResource* r) const {
    MeshInstance& i = *((MeshInstance*)r);
	

    Planet* obj = (Planet*)(i.entity());
    Camera* c = Resources::getCurrentScene()->getActiveCamera();
    
    float atmosphereHeight = obj->getAtmosphereHeight();

	glm::vec3& pos = obj->m_Body->position();
	glm::quat& orientation = obj->m_Body->rotation();
    glm::vec3 camPos = c->getPosition() - pos;
    float camHeight = glm::length(camPos);
    float camHeight2 = camHeight*camHeight;

    uint numberSamples = 1;
    //uint numberSamples = 8;
    
    glm::mat4 rot = glm::mat4(1.0f);
    rot *= glm::mat4_cast(orientation);
    
	glm::vec3 lightPos = Resources::getCurrentScene()->lights().at(0)->position();
    glm::vec3 lightDir = lightPos - pos;
    lightDir = glm::normalize(lightDir);
    float Km = 0.0025f;
    float Kr = 0.0015f;
    float ESun = 20.0f;
	glm::vec3 scl = obj->m_Body->getScale();
    
    float fScaledepth = 0.25f;
    float innerRadius = obj->getDefaultRadius();
    float outerRadius = innerRadius + (innerRadius * atmosphereHeight);
    float fScale = 1.0f / (outerRadius - innerRadius);
    if(camHeight <= outerRadius){
        Renderer::sendUniform1i("fromAtmosphere", 1);
    }
    else{
        Renderer::sendUniform1i("fromAtmosphere", 0);
    }
    Renderer::sendUniform4fSafe("Object_Color",i.color());
    Renderer::sendUniform3fSafe("Gods_Rays_Color",i.godRaysColor());

    Renderer::sendUniform1i("nSamples", numberSamples);
    Renderer::sendUniform1f("fSamples", float(numberSamples));   
    Renderer::sendUniform3f("v3CameraPos", camPos);
    Renderer::sendUniformMatrix4f("Rot",rot);
    Renderer::sendUniform3f("v3LightDir", lightDir);
    Renderer::sendUniform1f("fKrESun", Kr * ESun);
    Renderer::sendUniform1f("fKmESun", Km * ESun);
    Renderer::sendUniform1f("fKr4PI", Kr * 4 * 3.14159265358979f);
    Renderer::sendUniform1f("fKm4PI", Km * 4 * 3.14159265358979f);
    Renderer::sendUniform1i("HasAtmosphere",1);   
    Renderer::sendUniform1fSafe("fCameraHeight2", camHeight2);
    Renderer::sendUniform1fSafe("fExposure", 2.0f);
    Renderer::sendUniform1fSafe("fInnerRadius", innerRadius);
    Renderer::sendUniform1f("fScaleDepth",fScaledepth);
    Renderer::sendUniform1fSafe("fOuterRadius", outerRadius);
    Renderer::sendUniform1fSafe("fOuterRadius2", outerRadius*outerRadius);
    Renderer::sendUniform1f("fScale",fScale);
    Renderer::sendUniform1f("fScaleOverScaleDepth", fScale / fScaledepth);

	ShaderP* program;
    if(atmosphereHeight > 0){
        //Ground should be currently binded
        glm::mat4 mod = glm::mat4(1.0f);
        mod = glm::translate(mod,pos);
        mod *= glm::mat4_cast(orientation);
		mod = glm::scale(mod,scl);

        Renderer::sendUniformMatrix4f("Model",mod);

        glm::vec3 v3InvWaveLength = glm::vec3(1.0f/glm::pow(0.65f,4.0f),1.0f/glm::pow(0.57f,4.0f),1.0f/glm::pow(0.475f,4.0f));
        Renderer::sendUniform3f("v3InvWavelength", v3InvWaveLength);

        i.mesh()->render();

        if(camHeight > outerRadius){ 
			program = Resources::getShaderProgram(ResourceManifest::skyFromSpace); 
            glBlendFunc(GL_ONE, GL_ONE);
        }
        else{
			program = Resources::getShaderProgram(ResourceManifest::skyFromAtmosphere);
            glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
        }
        program->bind();

		if(Engine::Renderer::Settings::GodRays::enabled()) Renderer::sendUniform1i("HasGodsRays",1);
        else                                               Renderer::sendUniform1i("HasGodsRays",0);

        Renderer::Settings::cullFace(GL_FRONT);
		Renderer::GLEnable(GLState::BLEND);
        mod = glm::mat4(1.0f);
        mod = glm::translate(mod,pos);
		mod = glm::scale(mod,scl);
        mod = glm::scale(mod,glm::vec3(1.0f + atmosphereHeight));

        Renderer::sendUniformMatrix4f("VP",c->getViewProjection());
        Renderer::sendUniformMatrix4f("Model",mod);
        Renderer::sendUniform1fSafe("fcoeff",2.0f / glm::log2(c->getFar() + 1.0f));

        Renderer::sendUniform1i("nSamples", numberSamples);
        Renderer::sendUniform1f("fSamples", float(numberSamples));   
        Renderer::sendUniform3f("v3CameraPos", camPos);
        Renderer::sendUniform3f("v3LightDir", lightDir);
        Renderer::sendUniform3f("v3InvWavelength", v3InvWaveLength);

        Renderer::sendUniform1fSafe("fCameraHeight", camHeight);
        Renderer::sendUniform1fSafe("fCameraHeight2", camHeight2);

        Renderer::sendUniform1f("fOuterRadius", outerRadius);
        Renderer::sendUniform1fSafe("fOuterRadius2", outerRadius*outerRadius);
        Renderer::sendUniform1f("fInnerRadius", innerRadius);
        Renderer::sendUniform1fSafe("fInnerRadius2", innerRadius*innerRadius);

        Renderer::sendUniform1f("fKrESun", Kr * ESun);
        Renderer::sendUniform1f("fKmESun", Km * ESun);
        Renderer::sendUniform1f("fKr4PI", Kr * 4 * 3.14159265358979f);
        Renderer::sendUniform1f("fKm4PI", Km * 4 * 3.14159265358979f);

        Renderer::sendUniform1f("fScaleDepth",fScaledepth);
        Renderer::sendUniform1f("fScale",fScale);
        Renderer::sendUniform1f("fScaleOverScaleDepth", fScale / fScaledepth);

        // Gravity
        float g = -0.98f;
        Renderer::sendUniform1f("g",g);
        Renderer::sendUniform1f("g2", g*g);
        Renderer::sendUniform1f("fExposure",2.0f);
        i.mesh()->render();
        Renderer::Settings::cullFace(GL_BACK);
        Renderer::GLDisable(GLState::BLEND);
    }
    else{
        //Ground should be currently binded
		Renderer::sendUniform1i("HasAtmosphere",0);   

        glm::mat4 mod = glm::mat4(1.0f);
        mod = glm::translate(mod,pos);
        mod *= glm::mat4_cast(orientation);
		mod = glm::scale(mod,scl);

        Renderer::sendUniformMatrix4f("Model",mod);

        glm::vec3 v3InvWaveLength = glm::vec3(1.0f/glm::pow(0.29f,4.0f),1.0f/glm::pow(0.29f,4.0f),1.0f/glm::pow(0.29f,4.0f));
        Renderer::sendUniform3f("v3InvWavelength", v3InvWaveLength);
		Renderer::sendUniform3f("v3LightPos", lightPos);
        i.mesh()->render();
    }
    /*
    shader = Resources::getShaderProgram("Deferred")->program();
    glUseProgram(shader);
    GLEnable(GLState::BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    for(auto ring:m_Rings)
        ring->draw(shader);

    GLDisable(GLState::BLEND);
    glUseProgram(0);
    */
}};

Planet::Planet(Handle& mat,PlanetType type,glm::vec3 pos,float scl,string name,float atmosphere,Scene* scene):Entity(){
	scene->addEntity(this);
	m_Model = new ComponentModel(ResourceManifest::PlanetMesh,mat,this);
	addComponent(m_Model);
    if(type != PLANET_TYPE_STAR){
        AtmosphericScatteringMeshInstanceBindFunctor f;
		m_Model->setCustomBindFunctor(f,0);
    }
	
	m_Body = new ComponentBasicBody();
	addComponent(m_Body);
	m_Body->setScale(scl,scl,scl);
	m_Body->setPosition(pos);

    m_AtmosphereHeight = atmosphere;
    m_Type = type;
    m_OrbitInfo = nullptr;
    m_RotationInfo = nullptr;
}
Planet::~Planet(){
    for(auto ring:m_Rings)    
        SAFE_DELETE(ring);
    SAFE_DELETE(m_OrbitInfo);
    SAFE_DELETE(m_RotationInfo);
}
glm::vec3 Planet::getPosition(){ return m_Body->position(); }
void Planet::setPosition(float x,float y,float z){ m_Body->setPosition(x,y,z); }
void Planet::setPosition(glm::vec3& pos){ m_Body->setPosition(pos); }
void Planet::update(const float& dt){
    if(m_RotationInfo != nullptr){
        float speed = 360.0f * dt; //speed per second. now we need seconds per rotation cycle
        float secondsToRotate = m_RotationInfo->days * 86400.0f;
        float finalSpeed = 1.0f / (secondsToRotate * (speed));
        m_Body->rotate(0.0f,finalSpeed,0.0f);
    }
    if(m_OrbitInfo != nullptr){
        //m_OrbitInfo->setOrbitalPosition(((1.0/(m_OrbitInfo->days*86400.0))*dt)*6.283188,this);
    }
}
void Planet::setOrbit(OrbitInfo* o){ 
    m_OrbitInfo = o; 
    update(0);
}
void Planet::setRotation(RotationInfo* r){ 
    m_RotationInfo = r;
    //m_Body->rotate(0.0f,0.0f,glm::radians(-m_RotationInfo->tilt),false);
	m_Body->rotate(0.0f,0.0f,glm::radians(-m_RotationInfo->tilt));
}
void Planet::addRing(Ring* ring){ m_Rings.push_back(ring); }
glm::vec2 Planet::getGravityInfo(){ return glm::vec2(getRadius()*5,getRadius()*7); }
OrbitInfo* Planet::getOrbitInfo() const { return m_OrbitInfo; }
float Planet::getDefaultRadius(){ return m_Model->radius(); }
float Planet::getRadius() { return m_Model->radius() + (m_Model->radius() * m_AtmosphereHeight); }
float Planet::getAtmosphereHeight(){ return m_AtmosphereHeight; }



Star::Star(glm::vec3 starColor,glm::vec3 lightColor,glm::vec3 pos,float scl,string name,Scene* scene):Planet(ResourceManifest::StarMaterial,PLANET_TYPE_STAR,pos,scl,name,0.0f,scene){
    m_Light = new SunLight(glm::vec3(0.0f),LightType::Sun,scene);
    m_Light->setColor(lightColor.x,lightColor.y,lightColor.z,1);

	m_Model->getModel()->setColor(starColor.x,starColor.y,starColor.z,1.0f);
	m_Model->getModel()->setGodRaysColor(starColor.x,starColor.y,starColor.z);

    //addChild(m_Light);
	m_Light->setPosition(pos);
}
Star::~Star(){
}
Ring::Ring(vector<RingInfo>& rings,Planet* parent){
    m_Parent = parent;
    _makeRingImage(rings,parent);
    m_Parent->addRing(this);
}
Ring::~Ring(){
}
void Ring::_makeRingImage(vector<RingInfo>& rings,Planet* parent){
    sf::Image ringImage;
    ringImage.create(1024,2,sf::Color::Black);
    ringImage.createMaskFromColor(sf::Color::Black,0);

    uint count = 0;
    for(auto ringInfo: rings){
        glm::vec4 pC = glm::vec4(ringInfo.color.r/255.0f,ringInfo.color.g/255.0f,ringInfo.color.b/255.0f,1.0f);
        uint alphaChangeRange = ringInfo.size - ringInfo.alphaBreakpoint;
        uint newI = 0;
        for(uint i = 0; i < ringInfo.size; ++i){
            if(i > ringInfo.alphaBreakpoint){
                uint numerator = alphaChangeRange - newI;
                pC.a = float(numerator/(alphaChangeRange));
                newI++;
            }
            else{
                pC.a = 1;
            }

            if(count > 0){
                sf::Color bgFrontPixel = ringImage.getPixel(ringInfo.position + i,0);
                sf::Color bgBackPixel = ringImage.getPixel(ringInfo.position - i,0);

                glm::vec4 bgColorFront = glm::vec4(bgFrontPixel.r/255.0f,bgFrontPixel.g/255.0f,bgFrontPixel.b/255.0f,bgFrontPixel.a/255.0f);
                glm::vec4 bgColorBack = glm::vec4(bgBackPixel.r/255.0f,bgBackPixel.g/255.0f,bgBackPixel.b/255.0f,bgBackPixel.a/255.0f);

                glm::vec4 _fcf = Engine::Math::PaintersAlgorithm(bgColorFront,pC);
                glm::vec4 _fcb = Engine::Math::PaintersAlgorithm(bgColorBack,pC);

                if(ringInfo.color.r < 0 || ringInfo.color.g < 0 || ringInfo.color.b < 0){
                    _fcf = glm::vec4(bgColorFront.r,bgColorFront.g,bgColorFront.b,0);
                    _fcb = glm::vec4(bgColorBack.r,bgColorBack.g,bgColorBack.b,0);

                    uint numerator = ringInfo.size - i;
                    pC.a = float(numerator/(ringInfo.size));
                    _fcf.a = 1.0f - pC.a;
                    _fcb.a = 1.0f - pC.a;
                }

                float ra = float(int(rand() % 10 - 5))/255.0f;
                float ra1 = float(int(rand() % 10 - 5))/255.0f;

                _fcf.r += ra;
                _fcf.g += ra;
                _fcf.b += ra;

                _fcb.r += ra1;
                _fcb.g += ra1;
                _fcb.b += ra1;

                sf::Color fFront = sf::Color(sf::Uint8(_fcf.r) * 255, sf::Uint8(_fcf.g) * 255, sf::Uint8(_fcf.b) * 255, sf::Uint8(_fcf.a) * 255);
                sf::Color fBack = sf::Color(sf::Uint8(_fcb.r) * 255, sf::Uint8(_fcb.g) * 255, sf::Uint8(_fcb.b) * 255, sf::Uint8(_fcb.a) * 255);

                for(uint s = 0; s < ringImage.getSize().y; s++){
                    ringImage.setPixel(ringInfo.position + i,s,fFront);
                    ringImage.setPixel(ringInfo.position - i,s,fBack);
                }
            }
            else{
                int ra = rand() % 10 - 5;

                pC.r += ra;
                pC.g += ra;
                pC.b += ra;
                sf::Color _pc = sf::Color(sf::Uint8(pC.r) * 255, sf::Uint8(pC.g) * 255, sf::Uint8(pC.b) * 255, sf::Uint8(pC.a) * 255);

                for(uint s = 0; s < ringImage.getSize().y; s++){
                    ringImage.setPixel(ringInfo.position + i,s,_pc);
                    ringImage.setPixel(ringInfo.position - i,s,_pc);
                }
            }
        }
        ++count;
    }
    Texture* diffuse = new Texture(ringImage,"RingDiffuse",GL_TEXTURE_2D,false,ImageInternalFormat::SRGB8_ALPHA8);
	epriv::Core::m_Engine->m_ResourceManager->_addTexture(diffuse);
    Handle h = Resources::addMaterial("RingMaterial",diffuse,nullptr,nullptr,nullptr,nullptr);
    this->material = Resources::getMaterial(h);
}

OrbitInfo::OrbitInfo(float _eccentricity, float _days, float _majorRadius,float _angle,uint _parent){
    angle = _angle;
    eccentricity = _eccentricity;
    days = _days;
    majorRadius = _majorRadius;
    minorRadius = glm::sqrt(majorRadius * majorRadius * (1 - (eccentricity * eccentricity))); //b² = a²(1 - e²)
    parent = _parent;
}
glm::vec3 OrbitInfo::getOrbitalPosition(float angle,Planet* thisPlanet){
    glm::vec3 offset = glm::vec3(0.0f);
    glm::vec3 currentPos = thisPlanet->getPosition();
	Planet* parentPlanet = (Planet*)Resources::getCurrentScene()->getEntity(parent);
    if(parentPlanet){
		glm::vec3 parentPos = parentPlanet->getPosition();

        float newX = parentPos.x - glm::cos(angle) * majorRadius;
        float newZ = parentPos.z - glm::sin(angle) * minorRadius;

        offset = glm::vec3(newX - currentPos.x,0.0f,newZ - currentPos.z);
    }
    return (currentPos + offset);
}
void OrbitInfo::setOrbitalPosition(float a,Planet* planet){
    angle += a;
    glm::vec3 nextPos = getOrbitalPosition(angle,planet);
    planet->setPosition(nextPos);
}
