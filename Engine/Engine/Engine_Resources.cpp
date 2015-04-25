#include "Engine_Resources.h"
#include "ShaderProgram.h"

#include "Object.h"
#include "Light.h"
#include "ObjectDynamic.h"
#include "Skybox.h"
#include "Mesh.h"
#include "Material.h"
#include "Camera.h"
#include "Scene.h"
#include "Texture.h"
#include "Font.h"

#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <SFML/Graphics.hpp>

using namespace Engine::Resources;

//if dt is not set to 1, initialization of objects will be incorrect
float Detail::ResourceManagement::m_DeltaTime = 1;
sf::Window* Detail::ResourceManagement::m_Window = nullptr;
sf::Mouse* Detail::ResourceManagement::m_Mouse = nullptr;
Scene* Detail::ResourceManagement::m_CurrentScene = nullptr;
Camera* Detail::ResourceManagement::m_ActiveCamera = nullptr;

std::unordered_map<std::string,Font*> _getFontsDefaults(){ std::unordered_map<std::string,Font*> k; return k; }
std::unordered_map<std::string,Font*> Detail::ResourceManagement::m_Fonts = _getFontsDefaults();

std::unordered_map<std::string,Texture*> _getTexturesDefaults(){ std::unordered_map<std::string,Texture*> k; return k; }
std::unordered_map<std::string,Texture*> Detail::ResourceManagement::m_Textures = _getTexturesDefaults();

std::unordered_map<std::string,Scene*> _getScenesDefaults(){ std::unordered_map<std::string,Scene*> k; return k; }
std::unordered_map<std::string,Scene*> Detail::ResourceManagement::m_Scenes = _getScenesDefaults();

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
	for(auto texture:Detail::ResourceManagement::m_Textures){
		delete texture.second;
	}
	for(auto font:Detail::ResourceManagement::m_Fonts){
		delete font.second;
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
	for(auto scene:Detail::ResourceManagement::m_Scenes){
		delete scene.second;
	}
	delete Detail::ResourceManagement::m_CurrentScene;
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
void Engine::Resources::addMaterial(std::string name, Texture* diffuse, Texture* normal, Texture* glow){
	if (Detail::ResourceManagement::m_Materials.size() > 0 && Detail::ResourceManagement::m_Materials.count(name))
		return;
	Detail::ResourceManagement::m_Materials[name] = new Material(diffuse,normal,glow);
}
void Engine::Resources::addShader(std::string name, std::string vertexShaderFile, std::string fragmentShaderFile){
	if (Detail::ResourceManagement::m_Shaders.size() > 0 && Detail::ResourceManagement::m_Shaders.count(name))
		return;
	Detail::ResourceManagement::m_Shaders[name] = new ShaderP(vertexShaderFile,fragmentShaderFile);
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

	Resources::Detail::ResourceManagement::m_Meshes["Plane"] = new Mesh(1.0f,1.0f);
}