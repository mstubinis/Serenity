#include "Engine_Resources.h"
#include "ShaderProgram.h"
#include <SOIL\SOIL.h>

ResourceManager::ResourceManager(){
	dt = 0;
}
void ResourceManager::INIT_Game_Resources(){
	float window_x = static_cast<float>(Window->getSize().x);
	float window_y = static_cast<float>(Window->getSize().y);

	Add_Shader_Program("Default","Shaders\\vert.glsl","Shaders\\frag.glsl");

	Add_Shader_Program("Deferred","Shaders\\vert.glsl","Shaders\\deferred_frag.glsl");
	Add_Shader_Program("Deferred_Blur_Horizontal","Shaders\\deferred_blur_horizontal.glsl","Shaders\\deferred_blur_frag.glsl");
	Add_Shader_Program("Deferred_Blur_Vertical","Shaders\\deferred_blur_vertical.glsl","Shaders\\deferred_blur_frag.glsl");
	Add_Shader_Program("Deferred_SSAO","Shaders\\deferred_lighting_vert.glsl","Shaders\\deferred_ssao_frag.glsl");
	Add_Shader_Program("Deferred_Final","Shaders\\deferred_lighting_vert.glsl","Shaders\\deferred_final_frag.glsl");
	Add_Shader_Program("Deferred_Skybox","Shaders\\vert_skybox.glsl","Shaders\\deferred_frag_skybox.glsl");

	Add_Shader_Program("Deferred_ASSpace","Shaders\\deferred_a_scatter_space_vert.glsl","Shaders\\deferred_a_scatter_space_frag.glsl");

	Add_Shader_Program("Deferred_Light_Point","Shaders\\deferred_lighting_vert.glsl","Shaders\\deferred_lighting_point_frag.glsl");
	Add_Shader_Program("Deferred_Light_Dir","Shaders\\deferred_lighting_vert.glsl","Shaders\\deferred_lighting_dir_frag.glsl");

	Add_Mesh("Default","Models\\Scar.ply");
	Add_Mesh("Skybox","Models\\Skybox.ply");
	Add_Mesh("LightSphere","Models\\LightSphere.ply");
	Add_Mesh("DEBUGLight","Models\\DEBUGLight.ply");
	Add_Mesh("Ground","Models\\Ground.ply");
	Add_Mesh("Planet","Models\\planet.ply");

	Add_Material("Default","Textures\\Scar.png","Textures\\ScarNormal.png","Textures\\ScarGlow.png");
	Add_Material("Rock","Textures\\rock.png","Textures\\rockNormal.png","");
	Add_Material("Stone","Textures\\stone.png","Textures\\stoneNormal.png","");

	m_Cameras["Default"] = new Camera(45.0f,float(window_x/window_y),0.1f,1000.0f);
	m_Cameras["Debug"] = new Camera(45.0f,float(window_x/window_y),0.1f,1000.0f);

	Set_Active_Camera("Default");
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
	#pragma region Delete Lights
	for(auto light:Lights_Points){
		delete light;
	}
	for(auto light:Lights_Directional){
		delete light;
	}
	#pragma endregion
	for(auto shaderP:m_ShaderPrograms){
		delete shaderP.second;
	}
}
void ResourceManager::Update(float dt)
{
}
void ResourceManager::Set_Active_Camera(Camera* camera){ m_Current_Camera = camera; }
void ResourceManager::Set_Active_Camera(std::string name){ m_Current_Camera = m_Cameras[name]; }
void ResourceManager::Load_Texture_Into_GLuint(GLuint& address, std::string filename){
	glGenTextures(1, &address);
	unsigned char* image;
	int width, height;

	if(filename != ""){
		glBindTexture(GL_TEXTURE_2D, address);
		image = SOIL_load_image(filename.c_str(), &width, &height, 0, SOIL_LOAD_RGBA);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA,GL_UNSIGNED_BYTE, image);
		SOIL_free_image_data(image);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else if(filename == "")
		address = 0;

}
void ResourceManager::Load_Cubemap_Texture_Into_GLuint(GLuint& address, std::string filenames[]){
	glGenTextures(1, &address);
	for(unsigned int i = 0; i < 6; i++){
		unsigned char* image;
		int width, height;
		if(filenames[i] != ""){
			glBindTexture(GL_TEXTURE_CUBE_MAP, address);
			image = SOIL_load_image(filenames[i].c_str(), &width, &height, 0, SOIL_LOAD_RGB);
			GLenum skyboxSide;
			if(i==0)
				skyboxSide = GL_TEXTURE_CUBE_MAP_POSITIVE_Z;
			else if(i == 1)
				skyboxSide = GL_TEXTURE_CUBE_MAP_NEGATIVE_Z;
			else if(i == 2)
				skyboxSide = GL_TEXTURE_CUBE_MAP_NEGATIVE_X;
			else if(i == 3)
				skyboxSide = GL_TEXTURE_CUBE_MAP_POSITIVE_X;
			else if(i == 4)
				skyboxSide = GL_TEXTURE_CUBE_MAP_POSITIVE_Y;
			else
				skyboxSide = GL_TEXTURE_CUBE_MAP_NEGATIVE_Y;
			glTexImage2D(skyboxSide, 0, GL_RGB,width, height, 0, GL_RGB, GL_UNSIGNED_BYTE,image);
			SOIL_free_image_data(image);
		}
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

void ResourceManager::Add_Shader_Program(std::string name, std::string vs, std::string ps){
	if (m_ShaderPrograms.size() > 0 && m_ShaderPrograms.count(name))
		return;
	m_ShaderPrograms[name] = new ShaderP(vs,ps);
}
#pragma region Getters
Mesh* ResourceManager::Default_Mesh(){ return m_Meshes["Default"]; }
Mesh* ResourceManager::Get_Mesh(std::string name){ return m_Meshes[name]; }
Material* ResourceManager::Default_Material(){ return m_Materials["Default"]; }
Material* ResourceManager::Get_Material(std::string name){ return m_Materials[name]; }
Camera* ResourceManager::Default_Camera(){ return m_Cameras["Default"]; }
Camera* ResourceManager::Get_Camera(std::string name){ return m_Cameras[name]; }
Camera* ResourceManager::Current_Camera(){ return m_Current_Camera; }
ShaderP* ResourceManager::Default_Shader_Program(){ return m_ShaderPrograms["Default"]; }
ShaderP* ResourceManager::Get_Shader_Program(std::string name){ return m_ShaderPrograms[name]; }
#pragma endregion