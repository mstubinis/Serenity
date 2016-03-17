#include "Engine_Resources.h"
#include "Engine_Sounds.h"
#include "ShaderProgram.h"

#include "Object.h"
#include "Light.h"
#include "ObjectDynamic.h"
#include "Skybox.h"
#include "Mesh.h"
#include "Material.h"
#include "Camera.h"
#include "Particles.h"
#include "Texture.h"
#include "Font.h"
#include "Scene.h"

#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <boost/make_shared.hpp>

#include <SFML/Graphics.hpp>

using namespace Engine::Resources;

float Detail::ResourceManagement::m_DeltaTime = 1;

sf::Window* Detail::ResourceManagement::m_Window;
std::string Detail::ResourceManagement::m_WindowName;
sf::Mouse* Detail::ResourceManagement::m_Mouse;
Scene* Detail::ResourceManagement::m_CurrentScene;
Camera* Detail::ResourceManagement::m_ActiveCamera;

std::unordered_map<std::string,boost::shared_ptr<Object>> Detail::ResourceManagement::m_Objects;
std::unordered_map<std::string,boost::weak_ptr<Camera>> Detail::ResourceManagement::m_Cameras;
std::unordered_map<std::string,boost::shared_ptr<Font>> Detail::ResourceManagement::m_Fonts;
std::unordered_map<std::string,boost::shared_ptr<Texture>> Detail::ResourceManagement::m_Textures;
std::unordered_map<std::string,boost::shared_ptr<Scene>> Detail::ResourceManagement::m_Scenes;
std::unordered_map<std::string,boost::shared_ptr<Mesh>> Detail::ResourceManagement::m_Meshes;
std::unordered_map<std::string,boost::shared_ptr<Material>> Detail::ResourceManagement::m_Materials;
std::unordered_map<std::string,boost::shared_ptr<ParticleInfo>> Detail::ResourceManagement::m_ParticleInfos;
std::unordered_map<std::string,boost::shared_ptr<ShaderP>> Detail::ResourceManagement::m_Shaders;
std::unordered_map<std::string,boost::shared_ptr<SoundEffect>> Detail::ResourceManagement::m_Sounds;

void Engine::Resources::Detail::ResourceManagement::destruct(){
	for (auto it = m_Meshes.begin();it != m_Meshes.end(); ++it )               
		it->second.reset();
	for (auto it = m_Textures.begin();it != m_Textures.end(); ++it )           
		it->second.reset();
	for (auto it = m_Fonts.begin();it != m_Fonts.end(); ++it )                 
		it->second.reset();
	for (auto it = m_Materials.begin();it != m_Materials.end(); ++it )         
		it->second.reset();
	for (auto it = m_ParticleInfos.begin();it != m_ParticleInfos.end(); ++it ) 
		it->second.reset();
	for (auto it = m_Shaders.begin();it != m_Shaders.end(); ++it )             
		it->second.reset();
	for (auto it = m_Objects.begin();it != m_Objects.end(); ++it )             
		it->second.reset();
	for (auto it = m_Sounds.begin();it != m_Sounds.end(); ++it )               
		it->second.reset();
	for (auto it = m_Scenes.begin();it != m_Scenes.end(); ++it )               
		it->second.reset();
	SAFE_DELETE( Detail::ResourceManagement::m_Mouse);
	SAFE_DELETE( Detail::ResourceManagement::m_Window);
}

void Engine::Resources::addMesh(std::string name,std::string file){
	if (Detail::ResourceManagement::m_Meshes.size() > 0 && Detail::ResourceManagement::m_Meshes.count(name))
		return;
	Detail::ResourceManagement::m_Meshes[name] = boost::make_shared<Mesh>(file);
}
void Engine::Resources::addMesh(std::string file){
	std::string name = file.substr(0, file.size()-4); Engine::Resources::addMesh(name,file);
}

void Engine::Resources::addMaterial(std::string name, std::string diffuse, std::string normal , std::string glow ){
	if (Detail::ResourceManagement::m_Materials.size() > 0 && Detail::ResourceManagement::m_Materials.count(name))
		return;
	Detail::ResourceManagement::m_Materials[name] = boost::make_shared<Material>(diffuse,normal,glow);
}
void Engine::Resources::addMaterial(std::string name, Texture* diffuse, Texture* normal, Texture* glow){
	if (Detail::ResourceManagement::m_Materials.size() > 0 && Detail::ResourceManagement::m_Materials.count(name))
		return;
	Detail::ResourceManagement::m_Materials[name] = boost::make_shared<Material>(diffuse,normal,glow);
}

void Engine::Resources::addParticleInfo(std::string name, std::string material){
	if (Detail::ResourceManagement::m_ParticleInfos.size() > 0 && Detail::ResourceManagement::m_ParticleInfos.count(name))
		return;
	Detail::ResourceManagement::m_ParticleInfos[name] = boost::make_shared<ParticleInfo>(material);
}
void Engine::Resources::addParticleInfo(std::string name, Material* material){
	if (Detail::ResourceManagement::m_ParticleInfos.size() > 0 && Detail::ResourceManagement::m_ParticleInfos.count(name))
		return;
	Detail::ResourceManagement::m_ParticleInfos[name] = boost::make_shared<ParticleInfo>(material);
}

void Engine::Resources::addShader(std::string name, std::string vertexShaderFile, std::string fragmentShaderFile){
	if (Detail::ResourceManagement::m_Shaders.size() > 0 && Detail::ResourceManagement::m_Shaders.count(name))
		return;
	Detail::ResourceManagement::m_Shaders[name] = boost::make_shared<ShaderP>(vertexShaderFile,fragmentShaderFile);
}

void Engine::Resources::addSound(std::string name, std::string file){
	if (Detail::ResourceManagement::m_Sounds.size() > 0 && Detail::ResourceManagement::m_Sounds.count(name))
		return;
	Detail::ResourceManagement::m_Sounds[name] = boost::make_shared<SoundEffect>(file);
}

void Engine::Resources::removeMesh(std::string name){
	if (Detail::ResourceManagement::m_Meshes.size() > 0 && Detail::ResourceManagement::m_Meshes.count(name)){
		Detail::ResourceManagement::m_Meshes[name].reset();
		Detail::ResourceManagement::m_Meshes.erase(name);
	}
}
void Engine::Resources::removeMaterial(std::string name){
	if (Detail::ResourceManagement::m_Materials.size() > 0 && Detail::ResourceManagement::m_Materials.count(name)){
		Detail::ResourceManagement::m_Materials[name].reset();
		Detail::ResourceManagement::m_Materials.erase(name);
	}
}
void Engine::Resources::removeParticleInfo(std::string name){
	if (Detail::ResourceManagement::m_ParticleInfos.size() > 0 && Detail::ResourceManagement::m_ParticleInfos.count(name)){
		Detail::ResourceManagement::m_ParticleInfos[name].reset();
		Detail::ResourceManagement::m_ParticleInfos.erase(name);
	}
}
void Engine::Resources::removeSound(std::string name){
	if (Detail::ResourceManagement::m_Sounds.size() > 0 && Detail::ResourceManagement::m_Sounds.count(name)){
		Detail::ResourceManagement::m_Sounds[name].reset();
		Detail::ResourceManagement::m_Sounds.erase(name);
	}
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

	Resources::Detail::ResourceManagement::m_Meshes["Plane"] = boost::make_shared<Mesh>(1.0f,1.0f);
}