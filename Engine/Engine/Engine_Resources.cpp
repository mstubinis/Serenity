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
#include "Particles.h"

#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <SFML/Graphics.hpp>

using namespace Engine::Resources;

//if m_DeltaTime is not set to 1, initialization of objects will be incorrect
float Detail::ResourceManagement::m_DeltaTime = 1;
sf::Window* Detail::ResourceManagement::m_Window = NULL;
sf::Mouse* Detail::ResourceManagement::m_Mouse = NULL;
Scene* Detail::ResourceManagement::m_CurrentScene = NULL;
Camera* Detail::ResourceManagement::m_ActiveCamera = NULL;

std::unordered_map<std::string,Object*> _getObjectsDefaults(){ std::unordered_map<std::string,Object*> k; return k; }
std::unordered_map<std::string,Object*> Detail::ResourceManagement::m_Objects = _getObjectsDefaults();

std::unordered_map<std::string,Camera*> _getCamerasDefaults(){ std::unordered_map<std::string,Camera*> k; return k; }
std::unordered_map<std::string,Camera*> Detail::ResourceManagement::m_Cameras = _getCamerasDefaults();

std::unordered_map<std::string,Font*> _getFontsDefaults(){ std::unordered_map<std::string,Font*> k; return k; }
std::unordered_map<std::string,Font*> Detail::ResourceManagement::m_Fonts = _getFontsDefaults();

std::unordered_map<std::string,Texture*> _getTexturesDefaults(){ std::unordered_map<std::string,Texture*> k; return k; }
std::unordered_map<std::string,Texture*> Detail::ResourceManagement::m_Textures = _getTexturesDefaults();

std::unordered_map<std::string,Scene*> _getScenesDefaults(){ std::unordered_map<std::string,Scene*> k; return k; }
std::unordered_map<std::string,Scene*> Detail::ResourceManagement::m_Scenes = _getScenesDefaults();

std::unordered_map<std::string,Mesh*> _getMeshDefaults(){ std::unordered_map<std::string,Mesh*> k; return k; }
std::unordered_map<std::string,Mesh*> Detail::ResourceManagement::m_Meshes = _getMeshDefaults();

std::unordered_map<std::string,Material*> _getMaterialDefaults(){ std::unordered_map<std::string,Material*> k; return k; }
std::unordered_map<std::string,Material*> Detail::ResourceManagement::m_Materials = _getMaterialDefaults();

std::unordered_map<std::string,ParticleInfo*> _getParticleInfoDefaults(){ std::unordered_map<std::string,ParticleInfo*> k; return k; }
std::unordered_map<std::string,ParticleInfo*> Detail::ResourceManagement::m_ParticleInfos = _getParticleInfoDefaults();

std::unordered_map<std::string,ShaderP*> _getShaderDefaults(){ std::unordered_map<std::string,ShaderP*> k; return k; }
std::unordered_map<std::string,ShaderP*> Detail::ResourceManagement::m_Shaders = _getShaderDefaults();

void Engine::Resources::Detail::ResourceManagement::destruct(){
	for (auto it = m_Meshes.begin();it != m_Meshes.end(); ++it )              SAFE_DELETE(it->second); 
	for (auto it = m_Textures.begin();it != m_Textures.end(); ++it )          SAFE_DELETE(it->second); 
	for (auto it = m_Fonts.begin();it != m_Fonts.end(); ++it )                SAFE_DELETE(it->second);
	for (auto it = m_Materials.begin();it != m_Materials.end(); ++it )        SAFE_DELETE(it->second);
	for (auto it = m_ParticleInfos.begin();it != m_ParticleInfos.end(); ++it )SAFE_DELETE(it->second);
	for (auto it = m_Shaders.begin();it != m_Shaders.end(); ++it )            SAFE_DELETE(it->second);
	for (auto it = m_Objects.begin();it != m_Objects.end(); ++it )            SAFE_DELETE(it->second); 
	for (auto it = m_Scenes.begin();it != m_Scenes.end(); ++it )              SAFE_DELETE(it->second);
	SAFE_DELETE( Detail::ResourceManagement::m_Mouse);
	SAFE_DELETE( Detail::ResourceManagement::m_Window);
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

void Engine::Resources::addParticleInfo(std::string name, std::string material){
	if (Detail::ResourceManagement::m_ParticleInfos.size() > 0 && Detail::ResourceManagement::m_ParticleInfos.count(name))
		return;
	Detail::ResourceManagement::m_ParticleInfos[name] = new ParticleInfo(material);
}
void Engine::Resources::addParticleInfo(std::string name, Material* material){
	if (Detail::ResourceManagement::m_ParticleInfos.size() > 0 && Detail::ResourceManagement::m_ParticleInfos.count(name))
		return;
	Detail::ResourceManagement::m_ParticleInfos[name] = new ParticleInfo(material);
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
	addShader("Deferred_Edge","Shaders/deferred_lighting_vert.glsl","Shaders/deferred_edge_frag.glsl");
	addShader("Deferred_Bloom","Shaders/deferred_lighting_vert.glsl","Shaders/deferred_bloom_frag.glsl");
	addShader("Deferred_Final","Shaders/deferred_lighting_vert.glsl","Shaders/deferred_final_frag.glsl");
	addShader("Deferred_Skybox","Shaders/vert_skybox.glsl","Shaders/deferred_frag_skybox.glsl");
	addShader("Deferred_Skybox_HUD","Shaders/vert_skybox.glsl","Shaders/deferred_frag_HUD.glsl");
	addShader("Deferred_Light","Shaders/deferred_lighting_vert.glsl","Shaders/deferred_lighting_frag.glsl");

	Resources::Detail::ResourceManagement::m_Meshes["Plane"] = new Mesh(1.0f,1.0f);
}