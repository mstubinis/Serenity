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

ResourceManager::ResourceManager(){
	dt = 0;
}
void ResourceManager::INIT_Game_Resources(){
	Add_Shader_Program("Deferred","Shaders/vert.glsl","Shaders/deferred_frag.glsl");
	Add_Shader_Program("Deferred_HUD","Shaders/vert_HUD.glsl","Shaders/deferred_frag_HUD.glsl");
	Add_Shader_Program("Deferred_Blur_Horizontal","Shaders/deferred_blur_horizontal.glsl","Shaders/deferred_blur_frag.glsl");
	Add_Shader_Program("Deferred_Blur_Vertical","Shaders/deferred_blur_vertical.glsl","Shaders/deferred_blur_frag.glsl");
	Add_Shader_Program("Deferred_SSAO","Shaders/deferred_lighting_vert.glsl","Shaders/deferred_ssao_frag.glsl");
	Add_Shader_Program("Deferred_Final","Shaders/deferred_lighting_vert.glsl","Shaders/deferred_final_frag.glsl");
	Add_Shader_Program("Deferred_Skybox","Shaders/vert_skybox.glsl","Shaders/deferred_frag_skybox.glsl");

	Add_Shader_Program("AS_SkyFromSpace","Shaders/AS_skyFromSpace_vert.glsl","Shaders/AS_skyFromSpace_frag.glsl");
	Add_Shader_Program("AS_SkyFromAtmosphere","Shaders/AS_skyFromAtmosphere_vert.glsl","Shaders/AS_skyFromAtmosphere_frag.glsl");
	Add_Shader_Program("AS_GroundFromSpace","Shaders/AS_groundFromSpace_vert.glsl","Shaders/AS_groundFromSpace_frag.glsl");

	Add_Shader_Program("Deferred_Light","Shaders/deferred_lighting_vert.glsl","Shaders/deferred_lighting_frag.glsl");

	Add_Mesh("Skybox","Models/skybox.obj");
	Add_Mesh("DEBUGLight","Models/debugLight.obj");
	Add_Mesh("Planet","Models/planet.obj");
	Add_Mesh("Defiant","Models/defiant.obj");
	Add_Mesh("Starbase","Models/starbase.obj");

	Add_Material("Star","Textures/sun.png","","");
	Add_Material("Default","Textures/sun.png","","");
	Add_Material("Earth","Textures/earth.png","","");
	Add_Material("Defiant","Textures/defiant.png","Textures/defiantNormal.png","Textures/defiantGlow.png");

	m_Cameras["Debug"] = new Camera(45,Window->getSize().x/(float)Window->getSize().y,0.1f,100000.0f);
	m_Cameras["HUD"] = new Camera(0,(float)Window->getSize().x,0,(float)Window->getSize().y,0.005f,100000.0f);

	Set_Active_Camera("Debug");
}
ResourceManager::~ResourceManager(){
	for(auto mesh:m_Meshes){
		delete mesh.second;
	}
	for(auto mat:m_Materials){
		delete mat.second;
	}
	for(auto cam:m_Cameras){
		delete cam.second;
	}
	for(auto obj:Objects){
		delete obj;
	}
	for(auto light:Lights){
		delete light;
	}
	for(auto shaderP:m_ShaderPrograms){
		delete shaderP.second;
	}
}
void ResourceManager::Set_Active_Camera(Camera* camera){ m_Current_Camera = camera; }
void ResourceManager::Set_Active_Camera(std::string name){ m_Current_Camera = m_Cameras[name]; }
void ResourceManager::Load_Texture_Into_GLuint(GLuint& address, std::string filename){
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
void ResourceManager::Load_Cubemap_Texture_Into_GLuint(GLuint& address, std::string filenames[]){
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

void ResourceManager::Add_Mesh(std::string file){
	std::string name = file.substr(0, file.size()-4);
	if (m_Meshes.size() > 0 && m_Meshes.count(name))
		return;
	m_Meshes[name] = new Mesh(file);
}
void ResourceManager::Add_Mesh(std::string name, std::string file){
	if (m_Meshes.size() > 0 && m_Meshes.count(name))
		return;
	m_Meshes[name] = new Mesh(file);
}
void ResourceManager::Add_Material(std::string name, std::string diffuse, std::string normal, std::string glow){
	if (m_Materials.size() > 0 && m_Materials.count(name))
		return;
	m_Materials[name] = new Material(diffuse,normal,glow);
}
void ResourceManager::Add_Camera(std::string name, Camera* camera){
	if (m_Cameras.size() > 0 && m_Cameras.count(name))
		return;
	m_Cameras[name] = camera;
}
void ResourceManager::Add_Shader_Program(std::string name, std::string vs, std::string ps){
	if (m_ShaderPrograms.size() > 0 && m_ShaderPrograms.count(name))
		return;
	m_ShaderPrograms[name] = new ShaderP(vs,ps);
}
#pragma region Getters
Mesh* ResourceManager::Default_Mesh(){ return m_Meshes["Default"]; }
Mesh* ResourceManager::Get_Mesh(std::string name){ 
	if(name == "")
		return nullptr;
	return m_Meshes[name]; 
}
Material* ResourceManager::Default_Material(){ return m_Materials["Default"]; }
Material* ResourceManager::Get_Material(std::string name){ 
	if(name == "")
		return nullptr;
	return m_Materials[name]; 
}
Camera* ResourceManager::Default_Camera(){ return m_Cameras["Debug"]; }
Camera* ResourceManager::Get_Camera(std::string name){ return m_Cameras[name]; }
Camera* ResourceManager::Current_Camera(){ return m_Current_Camera; }
ShaderP* ResourceManager::Default_Shader_Program(){ return m_ShaderPrograms["Default"]; }
ShaderP* ResourceManager::Get_Shader_Program(std::string name){ return m_ShaderPrograms[name]; }
#pragma endregion