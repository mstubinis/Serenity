#include "Engine_Resources.h"
#include "ShaderProgram.h"

#include "Object.h"
#include "Light.h"
#include "ObjectDynamic.h"
#include "Skybox.h"
#include "Mesh.h"
#include "Material.h"
#include "Camera.h"

#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <SFML/Graphics.hpp>

using namespace Engine::Resources;


float Detail::ResourceManagement::m_DeltaTime = 0;
Camera* Detail::ResourceManagement::m_ActiveCamera = nullptr;
sf::Window* Detail::ResourceManagement::m_Window = nullptr;
sf::Mouse* Detail::ResourceManagement::m_Mouse = nullptr;

std::vector<Object*> _getObjectsDefaults(){ std::vector<Object*> k; return k; }
std::vector<Object*> Detail::ResourceManagement::m_Objects = _getObjectsDefaults();

std::vector<SunLight*> _getLightsDefaults(){ std::vector<SunLight*> k; return k; }
std::vector<SunLight*> Detail::ResourceManagement::m_Lights = _getLightsDefaults();

std::unordered_map<std::string,Camera*> _getCameraDefaults(){ std::unordered_map<std::string,Camera*> k; return k; }
std::unordered_map<std::string,Camera*> Detail::ResourceManagement::m_Cameras = _getCameraDefaults();

std::unordered_map<std::string,Mesh*> _getMeshDefaults(){ std::unordered_map<std::string,Mesh*> k; return k; }
std::unordered_map<std::string,Mesh*> Detail::ResourceManagement::m_Meshes = _getMeshDefaults();

std::unordered_map<std::string,Material*> _getMaterialDefaults(){ std::unordered_map<std::string,Material*> k; return k; }
std::unordered_map<std::string,Material*> Detail::ResourceManagement::m_Materials = _getMaterialDefaults();

std::unordered_map<std::string,ShaderP*> _getShaderDefaults(){ std::unordered_map<std::string,ShaderP*> k; return k; }
std::unordered_map<std::string,ShaderP*> Detail::ResourceManagement::m_Shaders = _getShaderDefaults();

void Engine::Resources::Detail::ResourceManagement::destruct(){
	for(auto mesh:Detail::ResourceManagement::m_Meshes){
		delete mesh.second;
	}
	for(auto mat:Detail::ResourceManagement::m_Materials){
		delete mat.second;
	}
	for(auto cam:Detail::ResourceManagement::m_Cameras){
		delete cam.second;
	}
	for(auto shaderP:Detail::ResourceManagement::m_Shaders){
		delete shaderP.second;
	}
	for(auto light:Detail::ResourceManagement::m_Lights){
		delete light;
	}
	for(auto obj:Detail::ResourceManagement::m_Objects){
		delete obj;
	}
	delete Detail::ResourceManagement::m_Mouse;
	delete Detail::ResourceManagement::m_Window;
}

void Engine::Resources::addMesh(std::string name,std::string file){
	if (Detail::ResourceManagement::m_Meshes.size() > 0 && Detail::ResourceManagement::m_Meshes.count(name))
		return;
	Detail::ResourceManagement::m_Meshes[name] = new Mesh(file);
}
void Engine::Resources::addMesh(std::string file){
	std::string name = file.substr(0, file.size()-4);
	Engine::Resources::addMesh(name,file);
}

void Engine::Resources::addMaterial(std::string name, std::string diffuse, std::string normal , std::string glow ){
	if (Detail::ResourceManagement::m_Materials.size() > 0 && Detail::ResourceManagement::m_Materials.count(name))
		return;
	Detail::ResourceManagement::m_Materials[name] = new Material(diffuse,normal,glow);
}

void Engine::Resources::addShader(std::string name, std::string vertexShaderFile, std::string fragmentShaderFile){
	if (Detail::ResourceManagement::m_Shaders.size() > 0 && Detail::ResourceManagement::m_Shaders.count(name))
		return;
	Detail::ResourceManagement::m_Shaders[name] = new ShaderP(vertexShaderFile,fragmentShaderFile);
}

void Engine::Resources::loadTextureIntoGLuint(GLuint& address, std::string filename){
	glGenTextures(1, &address);
	sf::Image image;

	glBindTexture(GL_TEXTURE_2D, address);
	image.loadFromFile(filename.c_str());
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.getSize().x, image.getSize().y, 0, GL_RGBA,GL_UNSIGNED_BYTE, image.getPixelsPtr());

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glGenerateMipmap(GL_TEXTURE_2D);
}
void Engine::Resources::loadCubemapTextureIntoGLuint(GLuint& address, std::string filenames[]){
	glGenTextures(1, &address);
	for(unsigned int i = 0; i < 6; i++){
		sf::Image image;

		glBindTexture(GL_TEXTURE_CUBE_MAP, address);
		image.loadFromFile(filenames[i].c_str());
		GLenum skyboxSide;
		if(i==0)           skyboxSide = GL_TEXTURE_CUBE_MAP_POSITIVE_Z;
		else if(i == 1)    skyboxSide = GL_TEXTURE_CUBE_MAP_NEGATIVE_Z;
		else if(i == 2)    skyboxSide = GL_TEXTURE_CUBE_MAP_NEGATIVE_X;
		else if(i == 3)    skyboxSide = GL_TEXTURE_CUBE_MAP_POSITIVE_X;
		else if(i == 4)    skyboxSide = GL_TEXTURE_CUBE_MAP_POSITIVE_Y;
		else               skyboxSide = GL_TEXTURE_CUBE_MAP_NEGATIVE_Y;
		glTexImage2D(skyboxSide, 0, GL_RGBA,image.getSize().x, image.getSize().y, 0, GL_RGBA, GL_UNSIGNED_BYTE,image.getPixelsPtr());
	}
	glTexParameteri (GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri (GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri (GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri (GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri (GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
}

void Engine::Resources::initResources(){
	addShader("Deferred","Shaders/vert.glsl","Shaders/deferred_frag.glsl");
	addShader("Deferred_HUD","Shaders/vert_HUD.glsl","Shaders/deferred_frag_HUD.glsl");
	addShader("Deferred_Blur_Horizontal","Shaders/deferred_blur_horizontal.glsl","Shaders/deferred_blur_frag.glsl");
	addShader("Deferred_Blur_Vertical","Shaders/deferred_blur_vertical.glsl","Shaders/deferred_blur_frag.glsl");
	addShader("Deferred_SSAO","Shaders/deferred_lighting_vert.glsl","Shaders/deferred_ssao_frag.glsl");
	addShader("Deferred_Final","Shaders/deferred_lighting_vert.glsl","Shaders/deferred_final_frag.glsl");
	addShader("Deferred_Skybox","Shaders/vert_skybox.glsl","Shaders/deferred_frag_skybox.glsl");

	addShader("AS_SkyFromSpace","Shaders/AS_skyFromSpace_vert.glsl","Shaders/AS_skyFromSpace_frag.glsl");
	addShader("AS_SkyFromAtmosphere","Shaders/AS_skyFromAtmosphere_vert.glsl","Shaders/AS_skyFromAtmosphere_frag.glsl");
	addShader("AS_GroundFromSpace","Shaders/AS_groundFromSpace_vert.glsl","Shaders/AS_groundFromSpace_frag.glsl");

	addShader("Deferred_Light","Shaders/deferred_lighting_vert.glsl","Shaders/deferred_lighting_frag.glsl");

	addMesh("Skybox","Models/skybox.obj");
	addMesh("DEBUGLight","Models/debugLight.obj");
	addMesh("Planet","Models/planet.obj");
	addMesh("Defiant","Models/defiant.obj");
	addMesh("Starbase","Models/starbase.obj");

	addMaterial("Star","Textures/sun.png","","");
	addMaterial("Default","Textures/sun.png","","");
	addMaterial("Earth","Textures/earth.png","","");
	addMaterial("Defiant","Textures/defiant.png","Textures/defiantNormal.png","Textures/defiantGlow.png");

	Engine::Resources::Detail::ResourceManagement::m_Cameras["Debug"] = new Camera(45,getWindow()->getSize().x/(float)getWindow()->getSize().y,0.1f,100000.0f);
	Engine::Resources::Detail::ResourceManagement::m_Cameras["HUD"] = new Camera(0,(float)getWindow()->getSize().x,0,(float)getWindow()->getSize().y,0.005f,100000.0f);

	setActiveCamera("Debug");
}