﻿#include "Engine_Resources.h"
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

OrbitInfo::OrbitInfo(float _eccentricity, float _days, float _majorRadius,glm::num _angle,std::string _parent){
    angle = _angle;
    eccentricity = _eccentricity;
    days = _days;
    majorRadius = _majorRadius;
    minorRadius = glm::sqrt(majorRadius*majorRadius*(1 - (eccentricity*eccentricity))); //b² = a²(1 - e²)
    parent = Engine::Resources::getObjectPtr(_parent);
}
glm::v3 OrbitInfo::getOrbitalPosition(glm::num angle,Object* thisPlanet){
    glm::v3 offset = glm::v3(0);
    glm::v3 currentPos = thisPlanet->getPosition();
    if(exists(parent)){
        glm::v3 parentPos = glm::v3(parent.lock().get()->getPosition());

        glm::num newX = parentPos.x - glm::cos(angle)*majorRadius;
        glm::num newZ = parentPos.z - glm::sin(angle)*minorRadius;

        offset = glm::vec3(newX - currentPos.x,0,newZ - currentPos.z);
    }
    return (currentPos + offset);
}
void OrbitInfo::setOrbitalPosition(glm::num a,Object* thisPlanet){
    angle += a;
    glm::v3 nextPos = getOrbitalPosition(angle,thisPlanet);
    thisPlanet->setPosition(nextPos);
}


Planet::Planet(std::string mat, PlanetType type, glm::v3 pos,glm::num scl, std::string name,float atmosphere,Scene* scene):ObjectDisplay("Planet",mat,pos,glm::vec3(scl),name,scene){
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
void Planet::render(GLuint shader,bool debug){
    shader = Resources::getShaderProgram("AS_GroundFromSpace")->program();
	ObjectDisplay::render(shader,debug);
}
void Planet::draw(GLuint shader,bool debug,bool godsRays){
    bool renderPlanet = true;
	Camera* activeCamera = Resources::getActiveCamera();
    if(m_DisplayItems.size() == 0 || !Resources::getActiveCamera()->sphereIntersectTest(this) || (activeCamera->getDistance(this) > 700 * getRadius()))
        renderPlanet = false;

    if(renderPlanet){
        #pragma region Ground
            float innerRadius = m_Radius;
            float outerRadius = innerRadius + (innerRadius * m_AtmosphereHeight);

            glm::mat4 mod = glm::mat4(1);
            mod = glm::translate(mod,glm::vec3(m_Position));
            mod *= glm::mat4_cast(m_Orientation);
            mod = glm::scale(mod,m_Scale);

            glm::mat4 rot = glm::mat4(1);
            rot *= glm::mat4_cast(m_Orientation);

            glUseProgram(shader);

			if(godsRays) glUniform1i(glGetUniformLocation(shader, "HasGodsRays"),1);
			else         glUniform1i(glGetUniformLocation(shader, "HasGodsRays"),0);


            glUniformMatrix4fv(glGetUniformLocation(shader, "VP" ), 1, GL_FALSE, glm::value_ptr(activeCamera->getViewProjection()));
            glUniformMatrix4fv(glGetUniformLocation(shader, "Model" ), 1, GL_FALSE, glm::value_ptr(mod));
            glUniformMatrix4fv(glGetUniformLocation(shader, "Rot" ), 1, GL_FALSE, glm::value_ptr(rot));

            glUniform1f(glGetUniformLocation(shader, "far"),activeCamera->getFar());
            glUniform1f(glGetUniformLocation(shader, "C"),1.0f);


            glUniform4f(glGetUniformLocation(shader, "Object_Color"),m_Color.x,m_Color.y,m_Color.z,m_Color.w);
			glUniform3f(glGetUniformLocation(shader, "Gods_Rays_Color"),m_GodsRaysColor.x,m_GodsRaysColor.y,m_GodsRaysColor.z);


            if(m_AtmosphereHeight > 0){
                glUniform1i(glGetUniformLocation(shader,"hasAtmosphere"),1);
                glUniform1i(glGetUniformLocation(shader,"HasAtmosphere"),1);
            }
            else{
                glUniform1i(glGetUniformLocation(shader,"hasAtmosphere"),0);
                glUniform1i(glGetUniformLocation(shader,"HasAtmosphere"),0);
            }

            float Km = 0.0025f;
            float Kr = 0.0015f;
            float ESun = 20.0f;

            glUniform1i(glGetUniformLocation(shader,"nSamples"), 2);
            glUniform1f(glGetUniformLocation(shader,"fSamples"), 2.0f);

            glm::vec3 camPos = glm::vec3(activeCamera->getPosition() - getPosition());
            glUniform3f(glGetUniformLocation(shader,"v3CameraPos"), camPos.x,camPos.y,camPos.z);

            glm::vec3 ambient = Resources::getCurrentScene()->getAmbientLightColor();
            glUniform3f(glGetUniformLocation(shader,"gAmbientColor"),ambient.x,ambient.y,ambient.z);

            glm::vec3 lightDir = glm::vec3(Resources::getCurrentScene()->lights().begin()->second->getPosition() - getPosition());
            lightDir = glm::normalize(lightDir);
            glUniform3f(glGetUniformLocation(shader,"v3LightDir"), lightDir.x,lightDir.y,lightDir.z);

            glm::vec3 v3InvWaveLength = glm::vec3(1.0f / glm::pow(0.65f, 4.0f),1.0f / glm::pow(0.57f, 4.0f),1.0f / glm::pow(0.475f, 4.0f));

            glUniform3f(glGetUniformLocation(shader,"v3InvWavelength"), v3InvWaveLength.x,v3InvWaveLength.y,v3InvWaveLength.z);

            float camHeight = glm::length(camPos);
            float camHeight2 = camHeight*camHeight;

            glUniform1f(glGetUniformLocation(shader,"fCameraHeight2"), camHeight2);


            glUniform1f(glGetUniformLocation(shader,"fOuterRadius"), outerRadius);
            glUniform1f(glGetUniformLocation(shader,"fOuterRadius2"), outerRadius*outerRadius);
            glUniform1f(glGetUniformLocation(shader,"fInnerRadius"), innerRadius);
            glUniform1f(glGetUniformLocation(shader,"fInnerRadius2"), innerRadius*innerRadius);

            glUniform1f(glGetUniformLocation(shader,"fKrESun"), Kr * ESun);
            glUniform1f(glGetUniformLocation(shader,"fKmESun"), Km * ESun);

            glUniform1f(glGetUniformLocation(shader,"fKr4PI"), Kr * 4 * 3.14159f);
            glUniform1f(glGetUniformLocation(shader,"fKm4PI"), Km * 4 * 3.14159f);

            float fScaledepth = 0.25f;
            float fScale = 1.0f / (outerRadius - innerRadius);

            glUniform1f(glGetUniformLocation(shader,"fScaleDepth"),fScaledepth);
            glUniform1f(glGetUniformLocation(shader,"fScale"),fScale);
            glUniform1f(glGetUniformLocation(shader,"fScaleOverScaleDepth"), fScale / fScaledepth);

            glUniform1f(glGetUniformLocation(shader,"fExposure"), 2.0f);
            for(auto item:m_DisplayItems){
				Material* mat = item->material();
                glUniform1f(glGetUniformLocation(shader, "BaseGlow"),mat->glow());
				glUniform1f(glGetUniformLocation(shader, "matID"),float(float(mat->id())/255.0f));
				mat->bind();
				item->mesh()->render();
            }
            glUseProgram(0);
            #pragma endregion

        #pragma region Atmosphere

        bool renderAtmosphere = true;
        if(m_AtmosphereHeight <= 0.0f) renderAtmosphere = false;

        if(renderAtmosphere){
            if(camHeight > outerRadius){ 
                shader = Resources::getShaderProgram("AS_SkyFromSpace")->program(); 
                glBlendFunc(GL_ONE, GL_ONE);
                //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            }
            else{ 
                shader = Resources::getShaderProgram("AS_SkyFromAtmosphere")->program();
				
                //glBlendFunc(GL_ONE, GL_ONE);
                glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
            }
            glUseProgram(shader);

			if(godsRays) glUniform1i(glGetUniformLocation(shader, "HasGodsRays"),1);
			else         glUniform1i(glGetUniformLocation(shader, "HasGodsRays"),0);

            glCullFace(GL_FRONT);

            glEnable(GL_BLEND);

            mod = glm::mat4(1);
            mod = glm::translate(mod,glm::vec3(m_Position));
            mod = glm::scale(mod,m_Scale);
            mod = glm::scale(mod,glm::vec3(1 + m_AtmosphereHeight));

            glUniformMatrix4fv(glGetUniformLocation(shader, "VP" ), 1, GL_FALSE, glm::value_ptr(activeCamera->getViewProjection()));
            glUniformMatrix4fv(glGetUniformLocation(shader, "Model" ), 1, GL_FALSE, glm::value_ptr(mod));
            glUniform1f(glGetUniformLocation(shader, "far"),activeCamera->getFar());
            glUniform1f(glGetUniformLocation(shader, "C"),1.0f);

            glUniform1i(glGetUniformLocation(shader,"nSamples"), 2);
            glUniform1f(glGetUniformLocation(shader,"fSamples"), 2.0f);

            glUniform3f(glGetUniformLocation(shader,"v3CameraPos"), camPos.x,camPos.y,camPos.z);

            glUniform3f(glGetUniformLocation(shader,"v3LightDir"), lightDir.x,lightDir.y,lightDir.z);

            glUniform3f(glGetUniformLocation(shader,"v3InvWavelength"), v3InvWaveLength.x,v3InvWaveLength.y,v3InvWaveLength.z);

            glUniform1f(glGetUniformLocation(shader,"fCameraHeight"),camHeight);
            glUniform1f(glGetUniformLocation(shader,"fCameraHeight2"), camHeight2);

            glUniform1f(glGetUniformLocation(shader,"fOuterRadius"), outerRadius);
            glUniform1f(glGetUniformLocation(shader,"fOuterRadius2"), outerRadius*outerRadius);
            glUniform1f(glGetUniformLocation(shader,"fInnerRadius"), innerRadius);
            glUniform1f(glGetUniformLocation(shader,"fInnerRadius2"), innerRadius*innerRadius);

            glUniform1f(glGetUniformLocation(shader,"fKrESun"), Kr * ESun);
            glUniform1f(glGetUniformLocation(shader,"fKmESun"), Km * ESun);

            glUniform1f(glGetUniformLocation(shader,"fKr4PI"), Kr * 4 * 3.14159f);
            glUniform1f(glGetUniformLocation(shader,"fKm4PI"), Km * 4 * 3.14159f);

            glUniform1f(glGetUniformLocation(shader,"fScaleDepth"),fScaledepth);
            glUniform1f(glGetUniformLocation(shader,"fScale"),fScale);
            glUniform1f(glGetUniformLocation(shader,"fScaleOverScaleDepth"), fScale / fScaledepth);

            // Gravity
            float g = -0.98f;
            glUniform1f(glGetUniformLocation(shader,"g"),g);
            glUniform1f(glGetUniformLocation(shader,"g2"), g*g);
            glUniform1f(glGetUniformLocation(shader,"fExposure"),2.0f);

            for(auto item:m_DisplayItems){
                item->mesh()->render();
            }
            glUseProgram(0);

            glCullFace(GL_BACK);
            glDisable(GL_BLEND);
        }

        #pragma endregion
    }
    shader = Resources::getShaderProgram("Deferred")->program();
    glUseProgram(shader);
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    for(auto ring:m_Rings)
        ring->draw(shader);

    glDisable(GL_BLEND);
    glUseProgram(0);

}
void Planet::addRing(Ring* ring){ m_Rings.push_back(ring); }
Star::Star(glm::vec3 starColor, glm::vec3 lightColor, glm::v3 pos,glm::num scl, std::string name,Scene* scene): Planet("Star",PLANET_TYPE_STAR,pos,scl,name,0,scene){
    m_Light = new SunLight(glm::v3(0),name + " Light",LIGHT_TYPE_SUN,scene);
    m_Light->setColor(lightColor.x,lightColor.y,lightColor.z,1);
    setColor(starColor.x,starColor.y,starColor.z,1);
	setGodsRaysColor(starColor.x,starColor.y,starColor.z);
    addChild(m_Light);
}
Star::~Star(){
}
void Star::render(GLuint shader,bool debug){
    ObjectDisplay::render(shader,debug);
}
void Star::draw(GLuint shader,bool debug,bool godsRays){
    ObjectDisplay::draw(shader,debug,godsRays);
}
Ring::Ring(std::vector<RingInfo> rings,Planet* parent){
    m_Parent = parent;
    _makeRingImage(rings,parent);
    m_Parent->addRing(this);
}
Ring::~Ring(){
}
void Ring::_makeRingImage(std::vector<RingInfo> rings,Planet* parent){
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

                for(unsigned int s = 0; s < ringImage.getSize().y; s++){
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

                for(unsigned int s = 0; s < ringImage.getSize().y; s++){
                    ringImage.setPixel(ringInfo.position + i,s,_pc);
                    ringImage.setPixel(ringInfo.position - i,s,_pc);
                }
            }
        }
        count++;
    }
    Texture* diffuse = new Texture(ringImage,parent->name() + "RingsDiffuse");
    Resources::addMaterial(parent->name() + "Rings",diffuse,nullptr,nullptr,nullptr,nullptr);
    this->material = Resources::getMaterial(parent->name() + "Rings");
}
void Ring::update(float dt){
}
void Ring::draw(GLuint shader){
    Camera* activeCamera = Resources::getActiveCamera();
    glm::m4 model = m_Parent->getModel();
    Mesh* mesh = Resources::getMesh("Ring");
    float radius = mesh->getRadius() * m_Parent->getScale().x;

    glUniformMatrix4fv(glGetUniformLocation(shader, "VP" ), 1, GL_FALSE, glm::value_ptr(activeCamera->getViewProjection()));
    glUniform1f(glGetUniformLocation(shader, "far"),activeCamera->getFar());
    glUniform1f(glGetUniformLocation(shader, "C"),1.0f);
    glUniformMatrix4fv(glGetUniformLocation(shader, "Model" ), 1, GL_FALSE, glm::value_ptr(glm::mat4(model)));
    glm::vec4 color = m_Parent->getColor();
    glUniform4f(glGetUniformLocation(shader, "Object_Color"),color.x,color.y,color.z,color.w);
    glUniform1i(glGetUniformLocation(shader, "Shadeless"),int(material->shadeless()));
	glUniform1f(glGetUniformLocation(shader, "matID"),float(float(material->id())/255.0f));

    glUniform1f(glGetUniformLocation(shader, "far"),activeCamera->getFar());
    glUniform1f(glGetUniformLocation(shader, "C"),1.0f);

	material->bind();
    mesh->render();
}