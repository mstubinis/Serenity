#include "Engine_Resources.h"
#include "Light.h"
#include "Planet.h"
#include "ShaderProgram.h"
#include "Mesh.h"
#include "Material.h"
#include "Camera.h"
#include "Scene.h"
#include "Texture.h"

using namespace Engine;

Planet::Planet(std::string mat, PlanetType type, glm::vec3 pos,float scl, std::string name,float atmosphere,Scene* scene):Object("Planet",mat,pos,glm::vec3(scl,scl,scl),name,true,scene){
	m_AtmosphereHeight = atmosphere;
	m_Type = type;
}
Planet::~Planet(){
	for(auto ring:m_Rings)
		delete ring;
}
void Planet::update(float dt){
	Object::update(dt);
	for(auto ring:m_Rings)
		ring->update(dt);
}
void Planet::render(Mesh* mesh, Material* mat,bool debug){
	bool renderPlanet = true;
	if(mesh == nullptr || !Resources::getActiveCamera()->sphereIntersectTest(this))
		renderPlanet = false;
	Camera* activeCamera = Resources::getActiveCamera();
	if(activeCamera->getDistance(this) > 450 * getRadius())
		renderPlanet = false;

	if(renderPlanet){
		#pragma region Ground
			GLuint shader = Resources::getShader("AS_GroundFromSpace")->getShaderProgram();

			float innerRadius = m_Radius;
			float outerRadius = innerRadius + (innerRadius * m_AtmosphereHeight);

			glUseProgram(shader);
			glUniformMatrix4fv(glGetUniformLocation(shader, "VP" ), 1, GL_FALSE, glm::value_ptr(activeCamera->getViewProjection()));
			glUniformMatrix4fv(glGetUniformLocation(shader, "World" ), 1, GL_FALSE, glm::value_ptr(m_Model));

			glUniform1f(glGetUniformLocation(shader, "far"),activeCamera->getFar());
			glUniform1f(glGetUniformLocation(shader, "C"),1.0f);

			glUniform4f(glGetUniformLocation(shader, "Object_Color"),m_Color.x,m_Color.y,m_Color.z,m_Color.w);

			for(auto component:mat->getComponents())
				mat->bindTexture(component.first,shader);

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

			glm::vec3 camPos = activeCamera->getPosition() - getPosition();
			glUniform3f(glGetUniformLocation(shader,"v3CameraPos"), camPos.x,camPos.y,camPos.z);

			glm::vec4 ambient = Resources::getCurrentScene()->getAmbientLightColor();
			glUniform4f(glGetUniformLocation(shader,"gAmbientColor"),ambient.x,ambient.y,ambient.z,ambient.w);

			glm::vec3 lightDir = Resources::getCurrentScene()->getLights().begin()->second->getPosition() - getPosition();
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


			mesh->render();
			glUseProgram(0);
			#pragma endregion

		#pragma region Atmosphere

		bool renderAtmosphere = true;
		if(m_AtmosphereHeight <= 0.0f) renderAtmosphere = false;

		if(renderAtmosphere){
			if(camHeight > outerRadius){ 
				shader = Resources::getShader("AS_SkyFromSpace")->getShaderProgram(); 
			}
			else{ 
				shader = Resources::getShader("AS_SkyFromAtmosphere")->getShaderProgram(); 
			}
			glUseProgram(shader);

			glCullFace(GL_FRONT);

			glEnable(GL_BLEND);
   			glBlendFunc(GL_ONE, GL_ONE);
			//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			glm::mat4 obj = m_Model;
			obj = glm::scale(obj,glm::vec3(1 + m_AtmosphereHeight));

			glUniformMatrix4fv(glGetUniformLocation(shader, "VP" ), 1, GL_FALSE, glm::value_ptr(activeCamera->getViewProjection()));
			glUniformMatrix4fv(glGetUniformLocation(shader, "World" ), 1, GL_FALSE, glm::value_ptr(obj));
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

			mesh->render();
			glUseProgram(0);

			glCullFace(GL_BACK);
			glDisable(GL_BLEND);
		}

		#pragma endregion
	}

	for(auto ring:m_Rings)
		ring->render();

}
void Planet::render(bool debug){ render(m_Mesh,m_Material,debug); }
void Planet::addRing(Ring* ring){ m_Rings.push_back(ring); }
Star::Star(glm::vec3 starColor, glm::vec3 lightColor, glm::vec3 pos,float scl, std::string name,Scene* scene): Planet("Star",PLANET_TYPE_STAR,pos,scl,name,0,scene){
	m_Light = new SunLight(glm::vec3(0,0,0),name + " Light",LIGHT_TYPE_SUN,scene);
	m_Light->setColor(lightColor.x,lightColor.y,lightColor.z,1);
	setColor(starColor.x,starColor.y,starColor.z,1);
	m_Material->setShadeless(true);

	addChild(m_Light);
}
Star::~Star(){
	delete m_Light;
}
void Star::render(Mesh* mesh, Material* mat,bool debug){
	Object::render(mesh,mat,debug);
}
void Star::render(bool debug){ Star::render(m_Mesh,m_Material,debug); }


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

	unsigned int count = 0;
	for(auto ringInfo: rings){
		sf::Color paintCol = sf::Color(ringInfo.color.r,ringInfo.color.g,ringInfo.color.b,255);
		glm::vec4 pC = glm::vec4(paintCol.r/255.0f,paintCol.g/255.0f,paintCol.b/255.0f,paintCol.a/255.0f);
		float alphaChangeRange = ringInfo.size - ringInfo.alphaBreakpoint;
		unsigned int newI = 0;
		for(unsigned int i = 0; i < ringInfo.size; i++){
			if(i > ringInfo.alphaBreakpoint){
				float numerator = alphaChangeRange - newI;
				pC.a = static_cast<float>(numerator/(alphaChangeRange));
				paintCol.a = pC.a * 255;
				newI++;
			}
			else{
				pC.a = 1;
				paintCol.a = 255;
			}

			if(count > 0){
				sf::Color backgroundColor = ringImage.getPixel(ringInfo.position + i,0);
				glm::vec4 bC = glm::vec4(backgroundColor.r/255.0f,backgroundColor.g/255.0f,backgroundColor.b/255.0f,backgroundColor.a/255.0f);
				sf::Color finalColor = sf::Color(255,255,255,255);

				float fA = pC.a + bC.a * (1-pC.a);
				finalColor.a = fA * 255;

				finalColor.r = ((pC.r*pC.a + bC.r*bC.a * (1-pC.a)) / fA)* 255;
				finalColor.g = ((pC.g*pC.a + bC.g*bC.a * (1-pC.a)) / fA)* 255;
				finalColor.b = ((pC.b*pC.a + bC.b*bC.a * (1-pC.a)) / fA)* 255;

				if(ringInfo.color.r < 0 || ringInfo.color.g < 0 || ringInfo.color.b < 0){
					finalColor = sf::Color(backgroundColor.r,backgroundColor.g,backgroundColor.b,0);

					float numerator = ringInfo.size - i;
					pC.a = static_cast<float>(numerator/(ringInfo.size));
					finalColor.a = 255 - (pC.a *255);
				}

				finalColor.r += rand() % 6 - 3;
				finalColor.g += rand() % 6 - 3;
				finalColor.b += rand() % 6 - 3;

				for(unsigned int s = 0; s < ringImage.getSize().y; s++){
					ringImage.setPixel(ringInfo.position + i,s,finalColor);
				}
			}
			else{
				sf::Color finalColor = paintCol;
				finalColor.r += rand() % 6 - 3;
				finalColor.g += rand() % 6 - 3;
				finalColor.b += rand() % 6 - 3;

				for(unsigned int s = 0; s < ringImage.getSize().y; s++){
					ringImage.setPixel(ringInfo.position + i,s,paintCol);
				}
			}
		}
		newI = 0;
		for(unsigned int i = 0; i < ringInfo.size; i++){
			if(i > ringInfo.alphaBreakpoint){
				float numerator = alphaChangeRange - newI;
				pC.a = static_cast<float>(numerator/(alphaChangeRange));
				paintCol.a = pC.a * 255;
				newI++;
			}
			else{
				pC.a = 1;
				paintCol.a = 255;
			}

			if(count > 0){
				sf::Color backgroundColor = ringImage.getPixel(ringInfo.position - i,0);
				glm::vec4 bC = glm::vec4(backgroundColor.r/255.0f,backgroundColor.g/255.0f,backgroundColor.b/255.0f,backgroundColor.a/255.0f);
				sf::Color finalColor = sf::Color(255,255,255,255);

				float fA = pC.a + bC.a * (1-pC.a);
				finalColor.a = fA * 255;

				finalColor.r = ((pC.r*pC.a + bC.r*bC.a * (1-pC.a)) / fA)* 255;
				finalColor.g = ((pC.g*pC.a + bC.g*bC.a * (1-pC.a)) / fA)* 255;
				finalColor.b = ((pC.b*pC.a + bC.b*bC.a * (1-pC.a)) / fA)* 255;

				if(ringInfo.color.r < 0 || ringInfo.color.g < 0 || ringInfo.color.b < 0){
					finalColor = sf::Color(backgroundColor.r,backgroundColor.g,backgroundColor.b,0);

					float numerator = ringInfo.size - i;
					pC.a = static_cast<float>(numerator/(ringInfo.size));
					finalColor.a = 255 - (pC.a *255);
				}
				finalColor.r += rand() % 6 - 3;
				finalColor.g += rand() % 6 - 3;
				finalColor.b += rand() % 6 - 3;

				for(unsigned int s = 0; s < ringImage.getSize().y; s++){
					ringImage.setPixel(ringInfo.position - i,s,finalColor);
				}
			}
			else{
				sf::Color finalColor = paintCol;
				finalColor.r += rand() % 6 - 3;
				finalColor.g += rand() % 6 - 3;
				finalColor.b += rand() % 6 - 3;

				for(unsigned int s = 0; s < ringImage.getSize().y; s++){
					ringImage.setPixel(ringInfo.position - i,s,paintCol);
				}
			}
		}
		count++;
	}
	Texture* diffuse = new Texture(ringImage.getPixelsPtr(),ringImage.getSize().x,ringImage.getSize().y,GL_TEXTURE_2D,parent->getName() + "RingsDiffuse");
	Resources::addMaterial(parent->getName() + "Rings",diffuse,nullptr,nullptr);
	this->material = Resources::getMaterial(parent->getName() + "Rings");
	ringImage.saveToFile("test.png");
}
void Ring::update(float dt){
}
void Ring::render(){
	Camera* activeCamera = Resources::getActiveCamera();
	glm::mat4 model = m_Parent->getModel();
	model = glm::rotate(model,45.0f,glm::vec3(1,0,0));
	Mesh* mesh = Resources::getMesh("Ring");
	float radius = mesh->getRadius() * m_Parent->getScale().x;

    glDisable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//glBlendFunc(GL_ONE, GL_ONE);

	GLuint shader = Resources::getShader("Deferred")->getShaderProgram();

	glUseProgram(shader);

	glUniformMatrix4fv(glGetUniformLocation(shader, "VP" ), 1, GL_FALSE, glm::value_ptr(activeCamera->getViewProjection()));
	glUniform1f(glGetUniformLocation(shader, "far"),activeCamera->getFar());
	glUniform1f(glGetUniformLocation(shader, "C"),1.0f);
	glUniformMatrix4fv(glGetUniformLocation(shader, "World" ), 1, GL_FALSE, glm::value_ptr(model));
	glm::vec4 color = m_Parent->getColor();
	glUniform4f(glGetUniformLocation(shader, "Object_Color"),color.x,color.y,color.z,color.w);
	glUniform1i(glGetUniformLocation(shader, "Shadeless"),static_cast<int>(material->getShadeless()));

	glUniform1f(glGetUniformLocation(shader, "far"),activeCamera->getFar());
	glUniform1f(glGetUniformLocation(shader, "C"),1.0f);
	for(auto component:material->getComponents())
		material->bindTexture(component.first,shader);
	mesh->render();
	glUseProgram(0);
	glEnable(GL_CULL_FACE);
	glDisable(GL_BLEND);
}