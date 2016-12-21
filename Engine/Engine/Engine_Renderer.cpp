#include "Engine_Renderer.h"
#include "Engine_Resources.h"
#include "Engine_Physics.h"
#include "ShaderProgram.h"
#include "GBuffer.h"
#include "Camera.h"
#include "Light.h"
#include "Font.h"
#include "Scene.h"
#include "Texture.h"
#include "Mesh.h"
#include "Skybox.h"
#include "Material.h"
#include "Object.h"
#include "ObjectDisplay.h"
#include "ObjectDynamic.h"

#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <boost/lexical_cast.hpp>
#include <random>
#include <math.h>

using namespace Engine;

#ifdef _WIN32
IDXGISwapChain* Renderer::Detail::RenderManagement::m_DirectXSwapChain;
ID3D11Device* Renderer::Detail::RenderManagement::m_DirectXDevice;
ID3D11DeviceContext* Renderer::Detail::RenderManagement::m_DirectXDeviceContext;
ID3D11RenderTargetView* Renderer::Detail::RenderManagement::m_DirectXBackBuffer;
#endif

bool Renderer::Detail::RendererInfo::GeneralInfo::alpha_test = false;
bool Renderer::Detail::RendererInfo::GeneralInfo::depth_test = true;
bool Renderer::Detail::RendererInfo::GeneralInfo::depth_mask = true;
GLuint Renderer::Detail::RendererInfo::GeneralInfo::current_shader_program = 0;

bool Renderer::Detail::RendererInfo::BloomInfo::bloom = true;
float Renderer::Detail::RendererInfo::BloomInfo::bloom_radius = 0.62f;
float Renderer::Detail::RendererInfo::BloomInfo::bloom_strength = 2.25f;

bool Renderer::Detail::RendererInfo::LightingInfo::lighting = true;

bool Renderer::Detail::RendererInfo::DebugDrawingInfo::debug = false;

bool Renderer::Detail::RendererInfo::GodRaysInfo::godRays = true;
float Renderer::Detail::RendererInfo::GodRaysInfo::godRays_exposure = 0.0034f;
float Renderer::Detail::RendererInfo::GodRaysInfo::godRays_decay = 1.0f;
float Renderer::Detail::RendererInfo::GodRaysInfo::godRays_density = 0.84f;
float Renderer::Detail::RendererInfo::GodRaysInfo::godRays_weight = 5.65f;
uint Renderer::Detail::RendererInfo::GodRaysInfo::godRays_samples = 45;
float Renderer::Detail::RendererInfo::GodRaysInfo::godRays_fovDegrees = 55.0f;
float Renderer::Detail::RendererInfo::GodRaysInfo::godRays_alphaFalloff = 2.0f;

bool Renderer::Detail::RendererInfo::SSAOInfo::ssao = true;
bool Renderer::Detail::RendererInfo::SSAOInfo::ssao_do_blur = true;
uint Renderer::Detail::RendererInfo::SSAOInfo::ssao_samples = 7;
float Renderer::Detail::RendererInfo::SSAOInfo::ssao_blur_strength = 0.5f;
float Renderer::Detail::RendererInfo::SSAOInfo::ssao_scale = 0.1f;
float Renderer::Detail::RendererInfo::SSAOInfo::ssao_intensity = 5.0f;
float Renderer::Detail::RendererInfo::SSAOInfo::ssao_bias = 0.24f;
float Renderer::Detail::RendererInfo::SSAOInfo::ssao_radius = 0.215f;
glm::vec2 Renderer::Detail::RendererInfo::SSAOInfo::ssao_Kernels[Renderer::Detail::RendererInfo::SSAOInfo::SSAO_KERNEL_COUNT];
GLuint Renderer::Detail::RendererInfo::SSAOInfo::ssao_noise_texture;

bool Renderer::Detail::RendererInfo::HDRInfo::hdr = true;
float Renderer::Detail::RendererInfo::HDRInfo::hdr_exposure = 1.2f;
float Renderer::Detail::RendererInfo::HDRInfo::hdr_gamma = 0.8f;

GBuffer* Renderer::Detail::RenderManagement::m_gBuffer = nullptr;
glm::mat4 Renderer::Detail::RenderManagement::m_2DProjectionMatrix;

std::vector<GeometryRenderInfo> Renderer::Detail::RenderManagement::m_ObjectsToBeRendered;
std::vector<GeometryRenderInfo> Renderer::Detail::RenderManagement::m_ForegroundObjectsToBeRendered;
std::vector<FontRenderInfo> Renderer::Detail::RenderManagement::m_FontsToBeRendered;
std::vector<TextureRenderInfo> Renderer::Detail::RenderManagement::m_TexturesToBeRendered;
std::vector<GeometryRenderInfo> Renderer::Detail::RenderManagement::m_ObjectsToBeForwardRendered;

std::vector<ShaderP*> Renderer::Detail::RenderManagement::m_GeometryPassShaderPrograms;

void Renderer::Settings::clear(bool color, bool depth, bool stencil){
	if(depth){ 
		Settings::enableDepthMask();
		if(color && stencil){ glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT); }
		else if(color && !stencil){ glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); }
		else{ glClear(GL_DEPTH_BUFFER_BIT); }
	}else{
		if(color && stencil){ glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT); }
		else if(color && !stencil){ glClear(GL_COLOR_BUFFER_BIT); }
		else{ glClear(GL_STENCIL_BUFFER_BIT); }
	}
}
void Renderer::Settings::enableAlphaTest(bool b){
	if(b && !Renderer::Detail::RendererInfo::GeneralInfo::alpha_test){
		glEnable(GL_ALPHA_TEST);
		Renderer::Detail::RendererInfo::GeneralInfo::alpha_test = true;
	}
	else if(!b && Renderer::Detail::RendererInfo::GeneralInfo::alpha_test){
		glDisable(GL_ALPHA_TEST);
		Renderer::Detail::RendererInfo::GeneralInfo::alpha_test = false;
	}
}
void Renderer::Settings::disableAlphaTest(){
	if(Renderer::Detail::RendererInfo::GeneralInfo::alpha_test){
		glDisable(GL_ALPHA_TEST);
		Renderer::Detail::RendererInfo::GeneralInfo::alpha_test = false;
	}
}
void Renderer::Settings::enableDepthTest(bool b){
	if(b && !Renderer::Detail::RendererInfo::GeneralInfo::depth_test){
		glEnable(GL_DEPTH_TEST);
		Renderer::Detail::RendererInfo::GeneralInfo::depth_test = true;
	}
	else if(!b && Renderer::Detail::RendererInfo::GeneralInfo::depth_test){
		glDisable(GL_DEPTH_TEST);
		Renderer::Detail::RendererInfo::GeneralInfo::depth_test = false;
	}
}
void Renderer::Settings::disableDepthTest(){
	if(Renderer::Detail::RendererInfo::GeneralInfo::depth_test){
		glDisable(GL_DEPTH_TEST);
		Renderer::Detail::RendererInfo::GeneralInfo::depth_test = false;
	}
}
void Renderer::Settings::enableDepthMask(bool b){
	if(b && !Renderer::Detail::RendererInfo::GeneralInfo::depth_mask){
		glDepthMask(GL_TRUE);
		Renderer::Detail::RendererInfo::GeneralInfo::depth_mask = true;
	}
	else if(!b && Renderer::Detail::RendererInfo::GeneralInfo::depth_mask){
		glDepthMask(GL_FALSE);
		Renderer::Detail::RendererInfo::GeneralInfo::depth_mask = false;
	}
}
void Renderer::Settings::disableDepthMask(){
	if(Renderer::Detail::RendererInfo::GeneralInfo::depth_mask){
		glDepthMask(GL_FALSE);
		Renderer::Detail::RendererInfo::GeneralInfo::depth_mask = false;
	}
}
void Renderer::useShader(ShaderP* program){
	if(program != 0){
		GLuint p = program->program();
		if(Renderer::Detail::RendererInfo::GeneralInfo::current_shader_program != p){
			glUseProgram(p);
			Renderer::Detail::RendererInfo::GeneralInfo::current_shader_program = p;
		}
	}
	else{
		if(Renderer::Detail::RendererInfo::GeneralInfo::current_shader_program != 0){
			glUseProgram(0);
			Renderer::Detail::RendererInfo::GeneralInfo::current_shader_program = 0;
		}
	}
}
void Renderer::useShader(std::string programName){
	Renderer::useShader(Resources::getShaderProgram(programName));
}
void Renderer::bindTexture(const char* location,Texture* texture,uint slot){
	Renderer::bindTexture(location,texture->address(),slot,texture->type());
}
void Renderer::bindTexture(const char* location,GLuint textureAddress,uint slot,GLuint textureType){
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(textureType, textureAddress);
	sendUniform1i(location,slot);
}
void Renderer::unbindTexture2D(uint slot){
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, 0);
}
void Renderer::unbindTextureCubemap(uint slot){
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}
void Renderer::sendUniform1d(const char* location,double x){
	glUniform1d(glGetUniformLocation(Detail::RendererInfo::GeneralInfo::current_shader_program,location), x );
}
void Renderer::sendUniform1i(const char* location,int x){
	glUniform1i(glGetUniformLocation(Detail::RendererInfo::GeneralInfo::current_shader_program,location), x );
}
void Renderer::sendUniform1f(const char* location,float x){
	glUniform1f(glGetUniformLocation(Detail::RendererInfo::GeneralInfo::current_shader_program,location), x );
}
void Renderer::sendUniform2d(const char* location,double x,double y){
	glUniform2d(glGetUniformLocation(Detail::RendererInfo::GeneralInfo::current_shader_program,location), x,y );
}
void Renderer::sendUniform2i(const char* location,int x,int y){
	glUniform2i(glGetUniformLocation(Detail::RendererInfo::GeneralInfo::current_shader_program,location), x,y );
}
void Renderer::sendUniform2f(const char* location,float x,float y){
	glUniform2f(glGetUniformLocation(Detail::RendererInfo::GeneralInfo::current_shader_program,location), x,y );
}
void Renderer::sendUniform3d(const char* location,double x,double y,double z){
	glUniform3d(glGetUniformLocation(Detail::RendererInfo::GeneralInfo::current_shader_program,location), x,y,z );
}
void Renderer::sendUniform3i(const char* location,int x,int y,int z){
	glUniform3i(glGetUniformLocation(Detail::RendererInfo::GeneralInfo::current_shader_program,location), x,y,z );
}
void Renderer::sendUniform3f(const char* location,float x,float y,float z){
	glUniform3f(glGetUniformLocation(Detail::RendererInfo::GeneralInfo::current_shader_program,location), x,y,z );
}
void Renderer::sendUniform4d(const char* location,double x,double y,double z,double w){
	glUniform4d(glGetUniformLocation(Detail::RendererInfo::GeneralInfo::current_shader_program,location), x,y,z,w );
}
void Renderer::sendUniform4i(const char* location,int x,int y,int z,int w){
	glUniform4i(glGetUniformLocation(Detail::RendererInfo::GeneralInfo::current_shader_program,location), x,y,z,w );
}
void Renderer::sendUniform4f(const char* location,float x,float y,float z,float w){
	glUniform4f(glGetUniformLocation(Detail::RendererInfo::GeneralInfo::current_shader_program,location), x,y,z,w );
}
void Renderer::sendUniformMatrix4f(const char* location,glm::mat4& m){
	glUniformMatrix4fv(glGetUniformLocation(Detail::RendererInfo::GeneralInfo::current_shader_program,location), 1, GL_FALSE, glm::value_ptr(m));
}
void Renderer::sendUniformMatrix4d(const char* location,glm::dmat4& m){
	glUniformMatrix4dv(glGetUniformLocation(Detail::RendererInfo::GeneralInfo::current_shader_program,location), 1, GL_FALSE, glm::value_ptr(m));
}
void Renderer::sendUniform2fv(const char* location,std::vector<glm::vec2>& data,uint limit){
	glUniform2fv(glGetUniformLocation(Detail::RendererInfo::GeneralInfo::current_shader_program,location),limit, glm::value_ptr(data[0]));
}
void Renderer::sendUniform3fv(const char* location,std::vector<glm::vec3>& data,uint limit){
	glUniform3fv(glGetUniformLocation(Detail::RendererInfo::GeneralInfo::current_shader_program,location),limit, glm::value_ptr(data[0]));
}
void Renderer::sendUniform4fv(const char* location,std::vector<glm::vec4>& data,uint limit){
	glUniform4fv(glGetUniformLocation(Detail::RendererInfo::GeneralInfo::current_shader_program,location),limit, glm::value_ptr(data[0]));
}
void Renderer::sendUniform2fv(const char* location,glm::vec2* data,uint limit){
	glUniform2fv(glGetUniformLocation(Detail::RendererInfo::GeneralInfo::current_shader_program,location),limit, glm::value_ptr(data[0]));
}
void Renderer::sendUniform3fv(const char* location,glm::vec3* data,uint limit){
	glUniform3fv(glGetUniformLocation(Detail::RendererInfo::GeneralInfo::current_shader_program,location),limit, glm::value_ptr(data[0]));
}
void Renderer::sendUniform4fv(const char* location,glm::vec4* data,uint limit){
	glUniform4fv(glGetUniformLocation(Detail::RendererInfo::GeneralInfo::current_shader_program,location),limit, glm::value_ptr(data[0]));
}

void Renderer::Detail::RenderManagement::init(){
    #ifdef _DEBUG
    Detail::RendererInfo::DebugDrawingInfo::debug = true;
    #else
    Detail::RendererInfo::DebugDrawingInfo::debug = false;
    #endif

    std::uniform_real_distribution<float> randFloats(0.0f,1.0f);//random floats between 0.0-1.0
	std::uniform_real_distribution<float> randFloats1(0.0f,1.0f);
    std::default_random_engine gen;
    std::vector<glm::vec2> kernels;
    for(uint i = 0; i < Renderer::Detail::RendererInfo::SSAOInfo::SSAO_KERNEL_COUNT; ++i){
        glm::vec2 sample(randFloats(gen)*2.0f-1.0f,randFloats(gen)*2.0f-1.0f);
        sample = glm::normalize(sample);
        sample *= randFloats(gen);
        float scale = float(i) / float(Renderer::Detail::RendererInfo::SSAOInfo::SSAO_KERNEL_COUNT);
        float a = 0.1f; float b = 1.0f; float f = scale * scale;
        scale = a + f * (b - a); //basic lerp
        sample *= scale;
        kernels.push_back(sample);
    }
    std::copy(kernels.begin(),kernels.end(),RendererInfo::SSAOInfo::ssao_Kernels);
    std::vector<glm::vec3> ssaoNoise;
    for(uint i = 0; i < RendererInfo::SSAOInfo::SSAO_NORMALMAP_SIZE*RendererInfo::SSAOInfo::SSAO_NORMALMAP_SIZE; i++){
        glm::vec3 noise(randFloats1(gen)*2.0-1.0,randFloats1(gen)*2.0-1.0,0.0f); 
        ssaoNoise.push_back(noise);
    } 
    glGenTextures(1, &RendererInfo::SSAOInfo::ssao_noise_texture);
    glBindTexture(GL_TEXTURE_2D, RendererInfo::SSAOInfo::ssao_noise_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, RendererInfo::SSAOInfo::SSAO_NORMALMAP_SIZE, 
                                              RendererInfo::SSAOInfo::SSAO_NORMALMAP_SIZE, 0, GL_RGB, GL_FLOAT, &ssaoNoise[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);  

    RenderManagement::m_2DProjectionMatrix = glm::ortho(0.0f,float(Resources::getWindowSize().x),0.0f,float(Resources::getWindowSize().y),0.005f,1000.0f);
}
void Renderer::Detail::RenderManagement::destruct(){
    SAFE_DELETE(RenderManagement::m_gBuffer);
    #ifdef _WIN32
    SAFE_DELETE_COM(RenderManagement::m_DirectXSwapChain);
    SAFE_DELETE_COM(RenderManagement::m_DirectXBackBuffer);
    SAFE_DELETE_COM(RenderManagement::m_DirectXDevice);
    SAFE_DELETE_COM(RenderManagement::m_DirectXDeviceContext);
    #endif
}
void Renderer::renderRectangle(glm::vec2 pos, glm::vec4 col, float w, float h, float angle, float depth){
    Detail::RenderManagement::getTextureRenderQueue().push_back(TextureRenderInfo("",pos,col,glm::vec2(w,h),angle,depth));
}
void Renderer::renderTexture(Texture* texture,glm::vec2 pos, glm::vec4 col,float angle, glm::vec2 scl, float depth){
	texture->render(pos,col,angle,scl,depth);
}
void Renderer::renderText(std::string text,Font* font, glm::vec2 pos,glm::vec4 color, float angle, glm::vec2 scl, float depth){
	font->renderText(text,pos,color,angle,scl,depth);
}

void Renderer::Detail::RenderManagement::_renderObjects(){
    for(auto item:m_ObjectsToBeRendered){
		item.object->draw(item.shader,RendererInfo::DebugDrawingInfo::debug,RendererInfo::GodRaysInfo::godRays);
    }
}
void Renderer::Detail::RenderManagement::_renderForegroundObjects(){
    for(auto item:m_ForegroundObjectsToBeRendered){
        item.object->draw(item.shader,RendererInfo::DebugDrawingInfo::debug);
    }
}
void Renderer::Detail::RenderManagement::_renderForwardRenderedObjects(){
    for(auto item:m_ObjectsToBeForwardRendered){
        item.object->draw(item.shader,RendererInfo::DebugDrawingInfo::debug);
    }
}
void Renderer::Detail::RenderManagement::_renderTextures(){
	useShader("Deferred_HUD");

	sendUniform1f("far",Resources::getActiveCamera()->getFar());
	sendUniform1f("C",1.0f);

    for(auto item:m_TexturesToBeRendered){
        Texture* texture = nullptr;
        if(item.texture != ""){
            texture = Resources::Detail::ResourceManagement::m_Textures[item.texture].get();
			bindTexture("DiffuseTexture",texture,0);
			sendUniform1i("DiffuseTextureEnabled",1);
        }
        else{
			bindTexture("DiffuseTexture",(GLuint)0,0);
			sendUniform1i("DiffuseTextureEnabled",0);
        }
		sendUniform1i("Shadeless",1);
		sendUniform4f("Object_Color",item.col.r,item.col.g,item.col.b,item.col.a);

        glm::mat4 model = glm::mat4(1);
        model = glm::translate(model, glm::vec3(item.pos.x,
                                                item.pos.y,
                                                -0.001f - item.depth));
        model = glm::rotate(model, item.rot,glm::vec3(0,0,1));
        if(item.texture != "")
            model = glm::scale(model, glm::vec3(texture->width(),texture->height(),1));
        model = glm::scale(model, glm::vec3(item.scl.x,item.scl.y,1));

		sendUniformMatrix4f("VP",m_2DProjectionMatrix);
		sendUniformMatrix4f("Model",model);

        Resources::getMesh("Plane")->render();
    }
    useShader(0);
}
void Renderer::Detail::RenderManagement::_renderText(){
	useShader("Deferred_HUD");
    for(auto item:m_FontsToBeRendered){
        Font* font = Resources::Detail::ResourceManagement::m_Fonts[item.texture].get();

		bindTexture("DiffuseTexture",font->getFontData()->getGlyphTexture(),0);
		sendUniform1i("DiffuseTextureEnabled",1);
        sendUniform1i("Shadeless",1);

		sendUniform4f("Object_Color",item.col.x,item.col.y,item.col.z,item.col.w);

        float y_offset = 0;
        float x = item.pos.x;
        for(auto c:item.text){
            if(c == '\n'){
                y_offset += (font->getFontData()->getGlyphData('X')->height+6) * item.scl.y;
                x = item.pos.x;
            }
            else{
                FontGlyph* glyph = font->getFontData()->getGlyphData(c);

                glyph->m_Model = glm::mat4(1);
                glyph->m_Model = glm::translate(glyph->m_Model, glm::vec3(x + glyph->xoffset ,item.pos.y - (glyph->height + glyph->yoffset) - y_offset,-0.001f - item.depth));
                glyph->m_Model = glm::rotate(glyph->m_Model, item.rot,glm::vec3(0,0,1));
                glyph->m_Model = glm::scale(glyph->m_Model, glm::vec3(item.scl.x,item.scl.y,1));

				sendUniformMatrix4f("VP",m_2DProjectionMatrix);
				sendUniformMatrix4f("Model",glyph->m_Model);

                glyph->char_mesh->render();
                x += (glyph->xadvance) * item.scl.x;
            }
        }
    }
    useShader(0);
}
void Renderer::Detail::RenderManagement::_passGeometry(){
	Settings::clear();
    Scene* s = Resources::getCurrentScene();
    glm::vec3 clear = s->getBackgroundColor();
    const float colors[4] = { clear.r, clear.g, clear.b, 1.0f };
    glClearBufferfv(GL_COLOR,BUFFER_TYPE_DIFFUSE,colors);
    glDisable(GL_BLEND); //disable blending on all mrts
    s->renderSkybox(RendererInfo::GodRaysInfo::godRays);

	//RENDER BACKGROUND OBJECTS THAT ARE IN FRONT OF SKYBOX HERE

	Settings::enableDepthTest();
	Settings::enableDepthMask();

    glEnablei(GL_BLEND,0); //enable blending on diffuse mrt only
    glBlendEquationi(GL_FUNC_ADD,0);
    glBlendFunci(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA,0);

	//RENDER NORMAL OBJECTS HERE
	for(auto shaderProgram:m_GeometryPassShaderPrograms){
		Renderer::useShader(shaderProgram);
		for(auto material:shaderProgram->getMaterials()){
			std::string matName = *(material.w.lock().get());
			Material* m = Resources::getMaterial(matName);
			m->bind(shaderProgram->program(),Resources::getAPI());
			for(auto object:m->getObjects()){
				std::string objName = *(object.w.lock().get());
				if(s->objects().count(objName)){
					Resources::getObject(objName)->draw(shaderProgram->program(),Detail::RendererInfo::DebugDrawingInfo::debug,Detail::RendererInfo::GodRaysInfo::godRays);
				}
			}
		}
		Renderer::useShader(0);
	}
	Settings::disableDepthTest();
	Settings::disableDepthMask();

	//RENDER FOREGROUND OBJECTS HERE
}
void Renderer::Detail::RenderManagement::_passLighting(){
    glm::vec3 camPos = glm::vec3(Resources::getActiveCamera()->getPosition());
	useShader("Deferred_Light");

	sendUniformMatrix4f("VP",Resources::getActiveCamera()->getViewProjection());
	sendUniform4fv("materials",Material::m_MaterialProperities,MATERIAL_COUNT_LIMIT);

	sendUniform2f("gScreenSize",(float)Resources::getWindowSize().x,(float)Resources::getWindowSize().y);

	bindTexture("gNormalMap",m_gBuffer->getTexture(BUFFER_TYPE_NORMAL),0);
	bindTexture("gPositionMap",m_gBuffer->getTexture(BUFFER_TYPE_POSITION),1);
	bindTexture("gMiscMap",m_gBuffer->getTexture(BUFFER_TYPE_MISC),2);
	bindTexture("gDiffuseMap",m_gBuffer->getTexture(BUFFER_TYPE_DIFFUSE),3);

    for (auto light:Resources::getCurrentScene()->lights()){
        light.second->lighten();
    }

    for(uint i = 0; i < 4; i++){ unbindTexture2D(i); }
	useShader(0);
}
void Renderer::Detail::RenderManagement::render(){
	if(!RendererInfo::GodRaysInfo::godRays)
		m_gBuffer->start(BUFFER_TYPE_DIFFUSE,BUFFER_TYPE_NORMAL,BUFFER_TYPE_MISC,BUFFER_TYPE_POSITION);
	else
		m_gBuffer->start(BUFFER_TYPE_DIFFUSE,BUFFER_TYPE_NORMAL,BUFFER_TYPE_MISC,BUFFER_TYPE_POSITION,BUFFER_TYPE_FREE1);
	RenderManagement::_passGeometry();
	m_gBuffer->stop();

	if(RendererInfo::GodRaysInfo::godRays){
		/*
		m_gBuffer->start(BUFFER_TYPE_GODSRAYS,"RGBA",false);
		Object* o = Resources::getObject("Sun");
		glm::vec3 sp = Math::getScreenCoordinates(glm::vec3(o->getPosition()),false);

		bool behind = Math::isPointWithinCone(Resources::getActiveCamera()->getPosition(),glm::v3(-Resources::getActiveCamera()->getViewVector()),o->getPosition(),Math::toRadians(RendererInfo::GodRaysInfo::godRays_fovDegrees));
		float alpha = Math::getAngleBetweenTwoVectors(glm::vec3(Resources::getActiveCamera()->getViewVector()),
			glm::vec3(Resources::getActiveCamera()->getPosition() - o->getPosition()),true) / RendererInfo::GodRaysInfo::godRays_fovDegrees;
		
		alpha = glm::pow(alpha,RendererInfo::GodRaysInfo::godRays_alphaFalloff);
		alpha = glm::clamp(alpha,0.001f,0.999f);

		_passGodsRays(glm::vec2(sp.x,sp.y),!behind,1.0f-alpha);
		m_gBuffer->stop();
		*/
	}
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_ONE, GL_ONE);
    if(RendererInfo::LightingInfo::lighting){
        m_gBuffer->start(BUFFER_TYPE_LIGHTING,"RGB");
        RenderManagement::_passLighting();
        m_gBuffer->stop();
    }
	glDisable(GL_BLEND);

    m_gBuffer->start(BUFFER_TYPE_BLOOM,"RGBA",false);
    RenderManagement::_passSSAO();
    m_gBuffer->stop();

	if(RendererInfo::SSAOInfo::ssao_do_blur || RendererInfo::BloomInfo::bloom){
		m_gBuffer->start(BUFFER_TYPE_FREE2,"RGBA",false);
		RenderManagement::_passBlur("Horizontal",BUFFER_TYPE_BLOOM,"RGBA");
		m_gBuffer->stop();
		m_gBuffer->start(BUFFER_TYPE_BLOOM,"RGBA",false);
		RenderManagement::_passBlur("Vertical",BUFFER_TYPE_FREE2,"RGBA");
		m_gBuffer->stop();
	}

    if(RendererInfo::HDRInfo::hdr){
        m_gBuffer->start(BUFFER_TYPE_MISC);
        RenderManagement::_passHDR();
        m_gBuffer->stop();
    }
    RenderManagement::_passFinal();

    //copy depth over
    glColorMask(0,0,0,0);
	useShader("Copy_Depth");

	bindTexture("gDepthMap",m_gBuffer->getTexture(BUFFER_TYPE_DEPTH),0);

    renderFullscreenQuad(Resources::getWindowSize().x,Resources::getWindowSize().y);

	unbindTexture2D(0);
	useShader(0);
    glColorMask(1,1,1,1);
    /////////////

    glEnable(GL_BLEND);
    if(RendererInfo::DebugDrawingInfo::debug)
        Physics::Detail::PhysicsManagement::render();
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    _renderForwardRenderedObjects();
    _renderForegroundObjects();

	Settings::enableAlphaTest();
    glAlphaFunc(GL_GREATER, 0.1f);


	Settings::enableDepthTest();
	Settings::enableDepthMask();

	Settings::clear(false,true,false); //clear depth only

    _renderTextures();
    _renderText();

	Settings::disableAlphaTest();

	m_ObjectsToBeRendered.clear();
	m_ForegroundObjectsToBeRendered.clear();
	m_ObjectsToBeForwardRendered.clear();
	m_FontsToBeRendered.clear();
	m_TexturesToBeRendered.clear();
}
void Renderer::Detail::RenderManagement::_passSSAO(){
	useShader("Deferred_SSAO");

	sendUniform1i("doSSAO",int(RendererInfo::SSAOInfo::ssao));
	sendUniform1i("doBloom",int(RendererInfo::BloomInfo::bloom));

	Camera* c = Resources::getActiveCamera();
	glm::vec3 camPos = glm::vec3(c->getPosition());

	sendUniform3f("gCameraPosition",camPos.x,camPos.y,camPos.z);
	sendUniform1f("gIntensity",RendererInfo::SSAOInfo::ssao_intensity);
	sendUniform1f("gBias",RendererInfo::SSAOInfo::ssao_bias);
	sendUniform1f("gRadius",RendererInfo::SSAOInfo::ssao_radius);
	sendUniform1f("gScale",RendererInfo::SSAOInfo::ssao_scale);
	sendUniform1i("gSampleCount",RendererInfo::SSAOInfo::ssao_samples);
	sendUniform1i("gNoiseTextureSize",RendererInfo::SSAOInfo::SSAO_NORMALMAP_SIZE);
	sendUniform2fv("poisson",RendererInfo::SSAOInfo::ssao_Kernels,Renderer::Detail::RendererInfo::SSAOInfo::SSAO_KERNEL_COUNT);
	sendUniform1i("far",int(c->getFar()));

	bindTexture("gNormalMap",m_gBuffer->getTexture(BUFFER_TYPE_NORMAL),0);
	bindTexture("gPositionMap",m_gBuffer->getTexture(BUFFER_TYPE_POSITION),1);
	bindTexture("gRandomMap",RendererInfo::SSAOInfo::ssao_noise_texture,2,GL_TEXTURE_2D);
	bindTexture("gMiscMap",m_gBuffer->getTexture(BUFFER_TYPE_MISC),3);
	bindTexture("gLightMap",m_gBuffer->getTexture(BUFFER_TYPE_LIGHTING),4);

    renderFullscreenQuad(Resources::getWindowSize().x,Resources::getWindowSize().y);

    for(uint i = 0; i < 5; i++){ unbindTexture2D(i); }
	useShader(0);
}
void Renderer::Detail::RenderManagement::_passEdge(GLuint texture, float radius){
	Settings::clear(true,false,false);

	useShader("Deferred_Edge");
	sendUniform2f("gScreenSize",float(Resources::getWindowSize().x),float(Resources::getWindowSize().y));
	sendUniform1f("radius", radius);

	bindTexture("texture",m_gBuffer->getTexture(texture),0);

    renderFullscreenQuad(Resources::getWindowSize().x,Resources::getWindowSize().y);

	unbindTexture2D(0);
    useShader(0);
}
void Renderer::Detail::RenderManagement::_passGodsRays(glm::vec2 lightPositionOnScreen,bool behind,float alpha){
    Settings::clear(true,false,false);

	useShader("Deferred_GodsRays");
	sendUniform1f("decay",RendererInfo::GodRaysInfo::godRays_decay);
	sendUniform1f("density",RendererInfo::GodRaysInfo::godRays_density);
	sendUniform1f("exposure",RendererInfo::GodRaysInfo::godRays_exposure);
	sendUniform1i("samples",RendererInfo::GodRaysInfo::godRays_samples);
	sendUniform1f("weight",RendererInfo::GodRaysInfo::godRays_weight);
	sendUniform2f("lightPositionOnScreen",
		float(lightPositionOnScreen.x)/float(Resources::getWindowSize().x),
		float(lightPositionOnScreen.y/float(Resources::getWindowSize().y))
	);

	sendUniform1i("behind",int(behind));
	sendUniform1f("alpha",alpha);

	bindTexture("firstPass",m_gBuffer->getTexture(BUFFER_TYPE_FREE1),0);

    renderFullscreenQuad(Resources::getWindowSize().x,Resources::getWindowSize().y);

	unbindTexture2D(0);
    useShader(0);
}
void Renderer::Detail::RenderManagement::_passHDR(){
    Settings::clear(true,false,false);

	useShader("Deferred_HDR");
	sendUniform1f("gamma",RendererInfo::HDRInfo::hdr_gamma);
	sendUniform1f("exposure",RendererInfo::HDRInfo::hdr_exposure);

	bindTexture("lightingBuffer",m_gBuffer->getTexture(BUFFER_TYPE_LIGHTING),0);
    renderFullscreenQuad(Resources::getWindowSize().x,Resources::getWindowSize().y);

	unbindTexture2D(0);
	useShader(0);
}

void Renderer::Detail::RenderManagement::_passBlur(std::string type, GLuint texture,std::string channels){
	useShader("Deferred_Blur");

	sendUniform1f("radius",RendererInfo::BloomInfo::bloom_radius);
	sendUniform4f("strengthModifier",RendererInfo::BloomInfo::bloom_strength,
		RendererInfo::BloomInfo::bloom_strength,
		RendererInfo::BloomInfo::bloom_strength,
		RendererInfo::SSAOInfo::ssao_blur_strength
	);

    if(channels.find("R") != std::string::npos) sendUniform1i("R",1);
    else                                        sendUniform1i("R",0);
    if(channels.find("G") != std::string::npos) sendUniform1i("G",1);
    else                                        sendUniform1i("G",0);
    if(channels.find("B") != std::string::npos) sendUniform1i("B",1);
    else                                        sendUniform1i("B",0);
    if(channels.find("A") != std::string::npos) sendUniform1i("A",1);
    else                                        sendUniform1i("A",0);

    if(type == "Horizontal"){ sendUniform2f("HV",1.0f,0.0f); }
    else{                     sendUniform2f("HV",0.0f,1.0f); }

	bindTexture("texture",m_gBuffer->getTexture(texture),0);

    renderFullscreenQuad(Resources::getWindowSize().x,Resources::getWindowSize().y);

	unbindTexture2D(0);
    useShader(0);
}
void Renderer::Detail::RenderManagement::_passFinal(){
    Settings::clear(true,false,false);

	useShader("Deferred_Final");

	sendUniform1f("gamma",RendererInfo::HDRInfo::hdr_gamma);

    glm::vec3 ambient = Resources::getCurrentScene()->getAmbientLightColor();
    sendUniform3f("gAmbientColor",ambient.x,ambient.y,ambient.z);

	sendUniform1i("HasSSAO",int(RendererInfo::SSAOInfo::ssao));
    sendUniform1i("HasLighting",int(RendererInfo::LightingInfo::lighting));
    sendUniform1i("HasBloom",int(RendererInfo::BloomInfo::bloom));
	sendUniform1i("HasHDR",int(RendererInfo::HDRInfo::hdr));

	bindTexture("gDiffuseMap",m_gBuffer->getTexture(BUFFER_TYPE_DIFFUSE),0);
	bindTexture("gLightMap",m_gBuffer->getTexture(BUFFER_TYPE_LIGHTING),1);
	bindTexture("gBloomMap",m_gBuffer->getTexture(BUFFER_TYPE_BLOOM),2);
	bindTexture("gNormalMap",m_gBuffer->getTexture(BUFFER_TYPE_NORMAL),3);
	bindTexture("gMiscMap",m_gBuffer->getTexture(BUFFER_TYPE_MISC),4);
	bindTexture("gGodsRaysMap",m_gBuffer->getTexture(BUFFER_TYPE_GODSRAYS),5);

    renderFullscreenQuad(Resources::getWindowSize().x,Resources::getWindowSize().y);

    for(uint i = 0; i < 6; i++){ unbindTexture2D(i); }
	useShader(0);
}
void Renderer::Detail::renderFullscreenQuad(uint width,uint height){
    glm::mat4 m(1);
	glm::mat4 p = glm::ortho(-float(width)/2,float(width)/2,-float(height)/2,float(height)/2);

	sendUniformMatrix4f("Model",m);
	sendUniformMatrix4f("VP",p);

    glViewport(0,0,width,height);

	glBegin(GL_QUADS);
		glTexCoord2f(0,0); glVertex2f(-float(width)/2,-float(height)/2);
		glTexCoord2f(1,0); glVertex2f(float(width)/2,-float(height)/2);
		glTexCoord2f(1,1); glVertex2f(float(width)/2,float(height)/2);
		glTexCoord2f(0,1); glVertex2f(-float(width)/2,float(height)/2);
	glEnd();
}

void Renderer::Detail::drawObject(ObjectDisplay* o, bool debug,bool godsRays){
	Camera* camera = Resources::getActiveCamera();
	Renderer::Detail::RenderManagement::_drawObjectInternal(camera,camera->getDistance(o),camera->sphereIntersectTest(o),o->getColor(),o->getGodsRaysColor(),o->getRadius(),o->getDisplayItems(),o->getModel(),o->visible(),debug,godsRays);
}
void Renderer::Detail::drawObject(ObjectDynamic* o, bool debug,bool godsRays){
	Camera* camera = Resources::getActiveCamera();
	Renderer::Detail::RenderManagement::_drawObjectInternal(camera,camera->getDistance(o),camera->sphereIntersectTest(o),o->getColor(),o->getGodsRaysColor(),o->getRadius(),o->getDisplayItems(),o->getModel(),o->visible(),debug,godsRays);
}
void Renderer::Detail::RenderManagement::_drawObjectInternal(Camera* camera,glm::num dist,bool intTest,glm::vec4& color, glm::vec3& raysColor,float radius,std::vector<DisplayItem*>& items,glm::m4 model,bool visible, bool debug,bool godsRays){
	if((items.size() == 0 || visible == false) || (!intTest) || (dist > 1100 * radius))
        return;

	sendUniformMatrix4f("VP",camera->getViewProjection());
	sendUniform1f("far",camera->getFar());
	sendUniform1f("C",1.0f);
	sendUniform4f("Object_Color",color.x,color.y,color.z,color.w);
	sendUniform3f("Gods_Rays_Color",raysColor.x,raysColor.y,raysColor.z);

	glm::vec3 camPos = glm::vec3(camera->getPosition());
	sendUniform3f("CameraPosition",camPos.x,camPos.y,camPos.z);

	if(godsRays) sendUniform1i("HasGodsRays",1);
	else         sendUniform1i("HasGodsRays",0);

    for(auto item:items){
		glm::mat4 m = glm::mat4(model);
        m = glm::translate(m,item->position);
        m *= glm::mat4_cast(item->orientation);
        m = glm::scale(m,item->scale);

        sendUniform1i("Shadeless",int(item->material->shadeless()));
        sendUniform1f("BaseGlow",item->material->glow());
		sendUniform1f("matID",float(float(item->material->id())/255.0f));
        sendUniformMatrix4f("Model",m);

        item->mesh->render();
    }
}

#ifdef _WIN32
void Renderer::Detail::RenderManagement::renderDirectX(){
    // clear the back buffer to a deep blue
    Scene* s = Resources::getCurrentScene();
    glm::vec3 clear = s->getBackgroundColor();
    const float colors[4] = { clear.r, clear.g, clear.b, 1.0f };
    m_DirectXDeviceContext->ClearRenderTargetView(m_DirectXBackBuffer,colors);
    // do 3D rendering on the back buffer here

    // switch the back buffer and the front buffer
    m_DirectXSwapChain->Present(0, 0);
}
#endif