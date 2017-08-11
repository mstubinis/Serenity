#include "Engine_Resources.h"
#include "Engine_Renderer.h"
#include "Light.h"
#include "Planet.h"
#include "ShaderProgram.h"
#include "Mesh.h"
#include "Material.h"
#include "Camera.h"
#include "Scene.h"
#include "Texture.h"

#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace Engine;

struct AtmosphericScatteringMeshInstanceBindFunctor{void operator()(EngineResource* r) const {
    MeshInstance* i = static_cast<MeshInstance*>(r);

    boost::weak_ptr<Object> o = Resources::getObjectPtr(i->parent()->name());

    Planet* obj = static_cast<Planet*>(o.lock().get());
    Camera* c = Resources::getActiveCamera();

    float atmosphereHeight = obj->getAtmosphereHeight();
    if(atmosphereHeight <= 0){
        atmosphereHeight = 0.025f;
    }
    ShaderP* program = Engine::Renderer::Detail::RendererInfo::GeneralInfo::current_shader_program;
    
    Renderer::sendUniform1i("nSamples", 2);
    Renderer::sendUniform1f("fSamples", 2.0f);   
    
    glm::vec3& pos = obj->getPosition();
    glm::quat& orientation = obj->getOrientation();
    glm::vec3 camPos = c->getPosition() - pos;
    Renderer::sendUniform3f("v3CameraPos", camPos);
      
    glm::mat4 rot = glm::mat4(1.0f);
    rot *= glm::mat4_cast(orientation);
    Renderer::sendUniformMatrix4f("Rot",rot);
    
    glm::vec3 lightDir = Resources::getCurrentScene()->lights().begin()->second->getPosition() - pos;
    lightDir = glm::normalize(lightDir);
    Renderer::sendUniform3f("v3LightDir", lightDir);
    
    float Km = 0.0025f;
    float Kr = 0.0015f;
    float ESun = 20.0f;
    Renderer::sendUniform1f("fKrESun", Kr * ESun);
    Renderer::sendUniform1f("fKmESun", Km * ESun);
    Renderer::sendUniform1f("fKr4PI", Kr * 4 * 3.14159265358979323846f);
    Renderer::sendUniform1f("fKm4PI", Km * 4 * 3.14159265358979323846f);
    Renderer::sendUniform1i("hasAtmosphere",1);
    Renderer::sendUniform1i("HasAtmosphere",1);   
    float camHeight = glm::length(camPos);
    float camHeight2 = camHeight*camHeight;
    Renderer::sendUniform1f("fCameraHeight2", camHeight2);
    float fScaledepth = 0.25f;
    float innerRadius = obj->getDefaultRadius();
    Renderer::sendUniform1f("fExposure", 2.0f);
    Renderer::sendUniform1f("fInnerRadius", innerRadius);
    Renderer::sendUniform1f("fScaleDepth",fScaledepth);
    float outerRadius = innerRadius + (innerRadius * atmosphereHeight);
    float fScale = 1.0f / (outerRadius - innerRadius);
    Renderer::sendUniform1f("fOuterRadius", outerRadius);
    Renderer::sendUniform1f("fOuterRadius2", outerRadius*outerRadius);
    Renderer::sendUniform1f("fScale",fScale);
    Renderer::sendUniform1f("fScaleOverScaleDepth", fScale / fScaledepth);
    if(atmosphereHeight > 0){
        //GroundFromAtmosphere should be currently binded
        glm::mat4 mod = glm::mat4(1.0f);
        mod = glm::translate(mod,pos);
        mod *= glm::mat4_cast(orientation);
        mod = glm::scale(mod,obj->getScale());

        Renderer::sendUniformMatrix4f("Model",mod);

        glm::vec3 v3InvWaveLength = glm::vec3(1.0f/glm::pow(0.65f,4.0f),1.0f/glm::pow(0.57f,4.0f),1.0f/glm::pow(0.475f,4.0f));
        Renderer::sendUniform3f("v3InvWavelength", v3InvWaveLength);

        i->mesh()->render();

        program->unbind();

        if(camHeight > outerRadius){ 
            program = Resources::getShaderProgram("AS_SkyFromSpace"); 
            glBlendFunc(GL_ONE, GL_ONE);
        }
        else{
            program = Resources::getShaderProgram("AS_SkyFromAtmosphere");
            glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
        }
        program->bind();

        if(Engine::Renderer::Detail::RendererInfo::GodRaysInfo::godRays) Renderer::sendUniform1i("HasGodsRays",1);
        else                                                             Renderer::sendUniform1i("HasGodsRays",0);

        Renderer::Settings::cullFace(GL_FRONT);
        glEnable(GL_BLEND);
        mod = glm::mat4(1.0f);
        mod = glm::translate(mod,pos);
        mod = glm::scale(mod,obj->getScale());
        mod = glm::scale(mod,glm::vec3(1.0f + atmosphereHeight));

        Renderer::sendUniformMatrix4f("VP",c->getViewProjection());
        Renderer::sendUniformMatrix4f("Model",mod);
        Renderer::sendUniform1fSafe("fcoeff",2.0f / glm::log2(c->getFar() + 1.0f));

        Renderer::sendUniform1i("nSamples", 2);
        Renderer::sendUniform1f("fSamples", 2.0f);
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
        Renderer::sendUniform1f("fKr4PI", Kr * 4 * 3.14159265358979323846f);
        Renderer::sendUniform1f("fKm4PI", Km * 4 * 3.14159265358979323846f);

        Renderer::sendUniform1f("fScaleDepth",fScaledepth);
        Renderer::sendUniform1f("fScale",fScale);
        Renderer::sendUniform1f("fScaleOverScaleDepth", fScale / fScaledepth);

        // Gravity
        float g = -0.98f;
        Renderer::sendUniform1f("g",g);
        Renderer::sendUniform1f("g2", g*g);
        Renderer::sendUniform1f("fExposure",2.0f);
        i->mesh()->render();
        Renderer::Settings::cullFace(GL_BACK);
        glDisable(GL_BLEND);
    }
    else{
        //GroundFromAtmosphere should be currently binded
        glm::mat4 mod = glm::mat4(1.0f);
        mod = glm::translate(mod,pos);
        mod *= glm::mat4_cast(orientation);
        mod = glm::scale(mod,obj->getScale());

        Renderer::sendUniformMatrix4f("Model",mod);

        glm::vec3 v3InvWaveLength = glm::vec3(1.0f/glm::pow(0.29f,4.0f),1.0f/glm::pow(0.29f,4.0f),1.0f/glm::pow(0.29f,4.0f));
        Renderer::sendUniform3f("v3InvWavelength", v3InvWaveLength);

        i->mesh()->render();
        program->unbind();
    }
    /*
    shader = Resources::getShaderProgram("Deferred")->program();
    glUseProgram(shader);
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    for(auto ring:m_Rings)
        ring->draw(shader);

    glDisable(GL_BLEND);
    glUseProgram(0);
    */
}};

Planet::Planet(std::string mat, PlanetType type, glm::vec3 pos,float scl, std::string name,float atmosphere,Scene* scene):ObjectDisplay("Planet",mat,pos,glm::vec3(scl),name,scene){
    m_AtmosphereHeight = atmosphere;
    m_Type = type;
    m_OrbitInfo = nullptr;
    m_RotationInfo = nullptr;
    if(type != PLANET_TYPE_STAR){
        AtmosphericScatteringMeshInstanceBindFunctor f;
        m_DisplayItems.at(0)->setCustomBindFunctor(f);
    }
}
Planet::~Planet(){
    for(auto ring:m_Rings)    
        SAFE_DELETE(ring);
    SAFE_DELETE(m_OrbitInfo);
    SAFE_DELETE(m_RotationInfo);
}
void Planet::update(float dt){
    if(m_RotationInfo != nullptr){
        float speed = 360.0f * dt; //speed per second. now we need seconds per rotation cycle
        float secondsToRotate = m_RotationInfo->days * 86400.0f;
        float finalSpeed = 1.0f / (secondsToRotate * (speed));
        rotate(0,finalSpeed,0);
    }
    if(m_OrbitInfo != nullptr){
        //m_OrbitInfo->setOrbitalPosition(((1.0/(m_OrbitInfo->days*86400.0))*dt)*6.283188,this);
    }
    for(auto ring:m_Rings)  ring->update(dt);
    ObjectDisplay::update(dt);
}
void Planet::setOrbit(OrbitInfo* o){ 
    m_OrbitInfo = o; 
    update(0);
}
void Planet::setRotation(RotationInfo* r){ 
    m_RotationInfo = r;
	rotate(0,0,glm::radians(-m_RotationInfo->tilt),false);
}
void Planet::addRing(Ring* ring){ m_Rings.push_back(ring); }
Star::Star(glm::vec3 starColor, glm::vec3 lightColor, glm::vec3 pos,float scl, std::string name,Scene* scene): Planet("Star",PLANET_TYPE_STAR,pos,scl,name,0,scene){
    m_Light = new SunLight(glm::vec3(0),name + " Light",LightType::Sun,scene);
    m_Light->setColor(lightColor.x,lightColor.y,lightColor.z,1);
    setColor(starColor.x,starColor.y,starColor.z,1);
    setGodsRaysColor(starColor.x,starColor.y,starColor.z);
    addChild(m_Light);
}
Star::~Star(){
}
Ring::Ring(std::vector<RingInfo>& rings,Planet* parent){
    m_Parent = parent;
    _makeRingImage(rings,parent);
    m_Parent->addRing(this);
}
Ring::~Ring(){
}
void Ring::_makeRingImage(std::vector<RingInfo>& rings,Planet* parent){
    sf::Image ringImage;
    ringImage.create(1024,2,sf::Color::Black);
    ringImage.createMaskFromColor(sf::Color::Black,0);

    uint count = 0;
    for(auto ringInfo: rings){
        glm::vec4 pC = glm::vec4(ringInfo.color.r/255.0f,ringInfo.color.g/255.0f,ringInfo.color.b/255.0f,1.0f);
        uint alphaChangeRange = ringInfo.size - ringInfo.alphaBreakpoint;
        uint newI = 0;
        for(uint i = 0; i < ringInfo.size; i++){
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
        count++;
    }    
    Texture* diffuse = new Texture(ringImage,parent->name() + "RingsDiffuse",GL_TEXTURE_2D,false,GL_SRGB8_ALPHA8);
    Resources::addMaterial(parent->name() + "Rings",diffuse,nullptr,nullptr,nullptr,nullptr);
    this->material = Resources::getMaterial(parent->name() + "Rings");
}
void Ring::update(float dt){
}
void Ring::draw(GLuint shader){
    Camera* activeCamera = Resources::getActiveCamera();
    glm::mat4 model = m_Parent->getModel();
    Mesh* mesh = Resources::getMesh("Ring");
    float radius = mesh->getRadius() * m_Parent->getScale().x;
    
    glUniformMatrix4fv(glGetUniformLocation(shader, "VP"), 1, GL_FALSE, glm::value_ptr(activeCamera->getViewProjection()));
    glUniformMatrix4fv(glGetUniformLocation(shader, "Model"), 1, GL_FALSE, glm::value_ptr(model));
    glm::vec4 color = m_Parent->getColor();
    glUniform4f(glGetUniformLocation(shader, "Object_Color"),color.x,color.y,color.z,color.w);
    glUniform1i(glGetUniformLocation(shader, "Shadeless"),int(material->shadeless()));
    glUniform1f(glGetUniformLocation(shader, "matID"),float(float(material->id())/255.0f));

    glUniform1f(glGetUniformLocation(shader, "far"),activeCamera->getFar());
    glUniform1f(glGetUniformLocation(shader, "C"),1.0f);

    material->bind();
    mesh->render();
}

OrbitInfo::OrbitInfo(float _eccentricity, float _days, float _majorRadius,float _angle,std::string _parent){
    angle = _angle;
    eccentricity = _eccentricity;
    days = _days;
    majorRadius = _majorRadius;
    minorRadius = glm::sqrt(majorRadius*majorRadius*(1 - (eccentricity*eccentricity))); //b² = a²(1 - e²)
    parent = Engine::Resources::getObjectPtr(_parent);
}
glm::vec3 OrbitInfo::getOrbitalPosition(float angle,Object* thisPlanet){
    glm::vec3 offset = glm::vec3(0);
    glm::vec3 currentPos = thisPlanet->getPosition();
    if(exists(parent)){
        glm::vec3 parentPos = parent.lock().get()->getPosition();

        float newX = parentPos.x - glm::cos(angle)*majorRadius;
        float newZ = parentPos.z - glm::sin(angle)*minorRadius;

        offset = glm::vec3(newX - currentPos.x,0,newZ - currentPos.z);
    }
    return (currentPos + offset);
}
void OrbitInfo::setOrbitalPosition(float a,Object* planet){
    angle += a;
    glm::vec3 nextPos = getOrbitalPosition(angle,planet);
    planet->setPosition(nextPos);
}
