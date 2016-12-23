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
#include <boost/lexical_cast.hpp>
#include <random>
#include <math.h>

using namespace Engine;
using namespace Engine::Renderer;
using namespace std;

#ifdef _WIN32
IDXGISwapChain* Detail::RenderManagement::m_DirectXSwapChain;
ID3D11Device* Detail::RenderManagement::m_DirectXDevice;
ID3D11DeviceContext* Detail::RenderManagement::m_DirectXDeviceContext;
ID3D11RenderTargetView* Detail::RenderManagement::m_DirectXBackBuffer;
#endif

bool Detail::RendererInfo::GeneralInfo::alpha_test = false;
bool Detail::RendererInfo::GeneralInfo::depth_test = true;
bool Detail::RendererInfo::GeneralInfo::depth_mask = true;
ShaderP* Detail::RendererInfo::GeneralInfo::current_shader_program = nullptr;
string Detail::RendererInfo::GeneralInfo::current_bound_material = "";

bool Detail::RendererInfo::BloomInfo::bloom = true;
float Detail::RendererInfo::BloomInfo::bloom_radius = 0.62f;
float Detail::RendererInfo::BloomInfo::bloom_strength = 2.25f;

bool Detail::RendererInfo::LightingInfo::lighting = true;

bool Detail::RendererInfo::DebugDrawingInfo::debug = false;

bool Detail::RendererInfo::GodRaysInfo::godRays = true;
float Detail::RendererInfo::GodRaysInfo::godRays_exposure = 0.0034f;
float Detail::RendererInfo::GodRaysInfo::godRays_decay = 1.0f;
float Detail::RendererInfo::GodRaysInfo::godRays_density = 0.84f;
float Detail::RendererInfo::GodRaysInfo::godRays_weight = 5.65f;
uint Detail::RendererInfo::GodRaysInfo::godRays_samples = 45;
float Detail::RendererInfo::GodRaysInfo::godRays_fovDegrees = 55.0f;
float Detail::RendererInfo::GodRaysInfo::godRays_alphaFalloff = 2.0f;

bool Detail::RendererInfo::SSAOInfo::ssao = true;
bool Detail::RendererInfo::SSAOInfo::ssao_do_blur = true;
uint Detail::RendererInfo::SSAOInfo::ssao_samples = 7;
float Detail::RendererInfo::SSAOInfo::ssao_blur_strength = 0.5f;
float Detail::RendererInfo::SSAOInfo::ssao_scale = 0.1f;
float Detail::RendererInfo::SSAOInfo::ssao_intensity = 5.0f;
float Detail::RendererInfo::SSAOInfo::ssao_bias = 0.24f;
float Detail::RendererInfo::SSAOInfo::ssao_radius = 0.215f;
glm::vec2 Detail::RendererInfo::SSAOInfo::ssao_Kernels[Renderer::Detail::RendererInfo::SSAOInfo::SSAO_KERNEL_COUNT];
GLuint Detail::RendererInfo::SSAOInfo::ssao_noise_texture;

bool Detail::RendererInfo::HDRInfo::hdr = true;
float Detail::RendererInfo::HDRInfo::hdr_exposure = 1.2f;
float Detail::RendererInfo::HDRInfo::hdr_gamma = 0.8f;

GBuffer* Detail::RenderManagement::m_gBuffer = nullptr;
glm::mat4 Detail::RenderManagement::m_2DProjectionMatrix;

vector<GeometryRenderInfo> Detail::RenderManagement::m_ObjectsToBeRendered;
vector<GeometryRenderInfo> Detail::RenderManagement::m_ForegroundObjectsToBeRendered;
vector<FontRenderInfo> Detail::RenderManagement::m_FontsToBeRendered;
vector<TextureRenderInfo> Detail::RenderManagement::m_TexturesToBeRendered;
vector<GeometryRenderInfo> Detail::RenderManagement::m_ObjectsToBeForwardRendered;

vector<ShaderP*> Detail::RenderManagement::m_GeometryPassShaderPrograms;

void Settings::clear(bool color, bool depth, bool stencil){
	if(depth){ 
		enableDepthMask();
		if(color && stencil){ glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT); }
		else if(color && !stencil){ glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); }
		else{ glClear(GL_DEPTH_BUFFER_BIT); }
	}else{
		if(color && stencil){ glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT); }
		else if(color && !stencil){ glClear(GL_COLOR_BUFFER_BIT); }
		else{ glClear(GL_STENCIL_BUFFER_BIT); }
	}
}
void Settings::enableAlphaTest(bool b){
	if(b && !Renderer::Detail::RendererInfo::GeneralInfo::alpha_test){
		glEnable(GL_ALPHA_TEST);
		Detail::RendererInfo::GeneralInfo::alpha_test = true;
	}
	else if(!b && Detail::RendererInfo::GeneralInfo::alpha_test){
		glDisable(GL_ALPHA_TEST);
		Detail::RendererInfo::GeneralInfo::alpha_test = false;
	}
}
void Settings::disableAlphaTest(){
	if(Detail::RendererInfo::GeneralInfo::alpha_test){
		glDisable(GL_ALPHA_TEST);
		Detail::RendererInfo::GeneralInfo::alpha_test = false;
	}
}
void Settings::enableDepthTest(bool b){
	if(b && !Detail::RendererInfo::GeneralInfo::depth_test){
		glEnable(GL_DEPTH_TEST);
		Detail::RendererInfo::GeneralInfo::depth_test = true;
	}
	else if(!b && Detail::RendererInfo::GeneralInfo::depth_test){
		glDisable(GL_DEPTH_TEST);
		Detail::RendererInfo::GeneralInfo::depth_test = false;
	}
}
void Settings::disableDepthTest(){
	if(Detail::RendererInfo::GeneralInfo::depth_test){
		glDisable(GL_DEPTH_TEST);
		Detail::RendererInfo::GeneralInfo::depth_test = false;
	}
}
void Settings::enableDepthMask(bool b){
	if(b && !Detail::RendererInfo::GeneralInfo::depth_mask){
		glDepthMask(GL_TRUE);
		Detail::RendererInfo::GeneralInfo::depth_mask = true;
	}
	else if(!b && Detail::RendererInfo::GeneralInfo::depth_mask){
		glDepthMask(GL_FALSE);
		Detail::RendererInfo::GeneralInfo::depth_mask = false;
	}
}
void Settings::disableDepthMask(){
	if(Detail::RendererInfo::GeneralInfo::depth_mask){
		glDepthMask(GL_FALSE);
		Detail::RendererInfo::GeneralInfo::depth_mask = false;
	}
}
void Renderer::bindShaderProgram(ShaderP* program){
	if(program != 0){
		if(Detail::RendererInfo::GeneralInfo::current_shader_program != program){
			GLuint p = program->program();
			glUseProgram(p);
			Detail::RendererInfo::GeneralInfo::current_shader_program = program;
		}
	}
	else{
		if(Detail::RendererInfo::GeneralInfo::current_shader_program != nullptr){
			glUseProgram(0);
			Detail::RendererInfo::GeneralInfo::current_shader_program = nullptr;
		}
	}
}
void Renderer::bindShaderProgram(string programName){
	Renderer::bindShaderProgram(Resources::getShaderProgram(programName));
}
void Renderer::bindTexture(const char* l,Texture* t,uint slot){
	Renderer::bindTexture(l,t->address(),slot,t->type());
}
void Renderer::bindTexture(const char* l,GLuint address,uint slot,GLuint type){
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(type, address);
	sendUniform1i(l,slot);
}
void Renderer::unbindTexture2D(uint slot){
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, 0);
}
void Renderer::unbindTextureCubemap(uint slot){
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

void Detail::RenderManagement::init(){
    #ifdef _DEBUG
    RendererInfo::DebugDrawingInfo::debug = true;
    #else
    RendererInfo::DebugDrawingInfo::debug = false;
    #endif

    uniform_real_distribution<float> randFloats(0.0f,1.0f);//random floats between 0.0-1.0
	uniform_real_distribution<float> randFloats1(0.0f,1.0f);
    default_random_engine gen;
    vector<glm::vec2> kernels;
    for(uint i = 0; i < RendererInfo::SSAOInfo::SSAO_KERNEL_COUNT; ++i){
        glm::vec2 sample(randFloats(gen)*2.0f-1.0f,randFloats(gen)*2.0f-1.0f);
        sample = glm::normalize(sample);
        sample *= randFloats(gen);
        float scale = float(i) / float(RendererInfo::SSAOInfo::SSAO_KERNEL_COUNT);
        float a = 0.1f; float b = 1.0f; float f = scale * scale;
        scale = a + f * (b - a); //basic lerp
        sample *= scale;
        kernels.push_back(sample);
    }
    copy(kernels.begin(),kernels.end(),RendererInfo::SSAOInfo::ssao_Kernels);
    vector<glm::vec3> ssaoNoise;
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
void Detail::RenderManagement::destruct(){
    SAFE_DELETE(RenderManagement::m_gBuffer);
    #ifdef _WIN32
    SAFE_DELETE_COM(RenderManagement::m_DirectXSwapChain);
    SAFE_DELETE_COM(RenderManagement::m_DirectXBackBuffer);
    SAFE_DELETE_COM(RenderManagement::m_DirectXDevice);
    SAFE_DELETE_COM(RenderManagement::m_DirectXDeviceContext);
    #endif
}
void Renderer::renderRectangle(glm::vec2& pos, glm::vec4& col, float w, float h, float angle, float depth){
    Detail::RenderManagement::getTextureRenderQueue().push_back(TextureRenderInfo("",pos,col,glm::vec2(w,h),angle,depth));
}
void Renderer::renderTexture(Texture* texture,glm::vec2& pos, glm::vec4& col,float angle, glm::vec2& scl, float depth){
	texture->render(pos,col,angle,scl,depth);
}
void Renderer::renderText(string& text,Font* font, glm::vec2& pos,glm::vec4& color, float angle, glm::vec2& scl, float depth){
	font->renderText(text,pos,color,angle,scl,depth);
}

void Detail::RenderManagement::_renderObjects(){
    for(auto item:m_ObjectsToBeRendered){
		item.object->draw(item.shader,RendererInfo::DebugDrawingInfo::debug,RendererInfo::GodRaysInfo::godRays);
    }
}
void Detail::RenderManagement::_renderForegroundObjects(){
    for(auto item:m_ForegroundObjectsToBeRendered){
        item.object->draw(item.shader,RendererInfo::DebugDrawingInfo::debug);
    }
}
void Detail::RenderManagement::_renderForwardRenderedObjects(){
    for(auto item:m_ObjectsToBeForwardRendered){
        item.object->draw(item.shader,RendererInfo::DebugDrawingInfo::debug);
    }
}
void Detail::RenderManagement::_renderTextures(){
	bindShaderProgram("Deferred_HUD");
    for(auto item:m_TexturesToBeRendered){
        Texture* texture = nullptr;
        if(item.texture != ""){
            texture = Resources::Detail::ResourceManagement::m_Textures[item.texture].get();
			bindTexture("DiffuseTexture",texture,0);
			sendUniform1iUnsafe("DiffuseTextureEnabled",1);
        }
        else{
			bindTexture("DiffuseTexture",(GLuint)0,0);
			sendUniform1iUnsafe("DiffuseTextureEnabled",0);
        }
		sendUniform4fUnsafe("Object_Color",item.col.r,item.col.g,item.col.b,item.col.a);

        glm::mat4 model = glm::mat4(1);
        model = glm::translate(model, glm::vec3(item.pos.x,
                                                item.pos.y,
                                                -0.001f - item.depth));
        model = glm::rotate(model, item.rot,glm::vec3(0,0,1));
        if(item.texture != "")
            model = glm::scale(model, glm::vec3(texture->width(),texture->height(),1));
        model = glm::scale(model, glm::vec3(item.scl.x,item.scl.y,1));

		sendUniformMatrix4fUnsafe("VP",m_2DProjectionMatrix);
		sendUniformMatrix4fUnsafe("Model",model);

        Resources::getMesh("Plane")->render();
    }
    bindShaderProgram(0);
}
void Detail::RenderManagement::_renderText(){
	bindShaderProgram("Deferred_HUD");
    for(auto item:m_FontsToBeRendered){
        Font* font = Resources::Detail::ResourceManagement::m_Fonts[item.texture].get();

		bindTexture("DiffuseTexture",font->getFontData()->getGlyphTexture(),0);
		sendUniform1iUnsafe("DiffuseTextureEnabled",1);
		sendUniform4fUnsafe("Object_Color",item.col.x,item.col.y,item.col.z,item.col.w);

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

				sendUniformMatrix4fUnsafe("VP",m_2DProjectionMatrix);
				sendUniformMatrix4fUnsafe("Model",glyph->m_Model);

                glyph->char_mesh->render();
                x += (glyph->xadvance) * item.scl.x;
            }
        }
    }
    bindShaderProgram(0);
}
void Detail::RenderManagement::_passGeometry(){
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
		shaderProgram->_bind();
		for(auto material:shaderProgram->getMaterials()){
			string matName = *(material.w.lock().get());
			Material* m = Resources::getMaterial(matName);
			m->_bind(shaderProgram->program(),Resources::getAPI());
			for(auto key:m->getObjects()){
				string renderedItemName = *(key.w.lock().get());
				RenderedItem* item = Resources::getRenderedItem(renderedItemName);
				string parentObjectName = item->parent();
				Object* o = Resources::getObject(parentObjectName);

				if(s->objects().count(parentObjectName)){
					o->bind();
					item->draw(RendererInfo::DebugDrawingInfo::debug,RendererInfo::GodRaysInfo::godRays);
				}
			}
		}
		bindShaderProgram(0);
	}
	Settings::disableDepthTest();
	Settings::disableDepthMask();

	//RENDER FOREGROUND OBJECTS HERE
}
void Detail::RenderManagement::_passLighting(){
    glm::vec3 camPos = glm::vec3(Resources::getActiveCamera()->getPosition());
	bindShaderProgram("Deferred_Light");

	sendUniformMatrix4fUnsafe("VP",Resources::getActiveCamera()->getViewProjection());
	sendUniform4fvUnsafe("materials[0]",Material::m_MaterialProperities,MATERIAL_COUNT_LIMIT);

	sendUniform2fUnsafe("gScreenSize",(float)Resources::getWindowSize().x,(float)Resources::getWindowSize().y);

	bindTexture("gNormalMap",m_gBuffer->getTexture(BUFFER_TYPE_NORMAL),0);
	bindTexture("gPositionMap",m_gBuffer->getTexture(BUFFER_TYPE_POSITION),1);
	bindTexture("gMiscMap",m_gBuffer->getTexture(BUFFER_TYPE_MISC),2);
	bindTexture("gDiffuseMap",m_gBuffer->getTexture(BUFFER_TYPE_DIFFUSE),3);

    for (auto light:Resources::getCurrentScene()->lights()){
        light.second->lighten();
    }

    for(uint i = 0; i < 4; i++){ unbindTexture2D(i); }
	bindShaderProgram(0);
}
void Detail::RenderManagement::render(){
	if(!RendererInfo::GodRaysInfo::godRays)
		m_gBuffer->start(BUFFER_TYPE_DIFFUSE,BUFFER_TYPE_NORMAL,BUFFER_TYPE_MISC,BUFFER_TYPE_POSITION);
	else
		m_gBuffer->start(BUFFER_TYPE_DIFFUSE,BUFFER_TYPE_NORMAL,BUFFER_TYPE_MISC,BUFFER_TYPE_POSITION,BUFFER_TYPE_FREE1);
	_passGeometry();
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
        _passLighting();
        m_gBuffer->stop();
    }
	glDisable(GL_BLEND);

    m_gBuffer->start(BUFFER_TYPE_BLOOM,"RGBA",false);
    _passSSAO();
    m_gBuffer->stop();

	if(RendererInfo::SSAOInfo::ssao_do_blur || RendererInfo::BloomInfo::bloom){
		m_gBuffer->start(BUFFER_TYPE_FREE2,"RGBA",false);
		_passBlur("Horizontal",BUFFER_TYPE_BLOOM,"RGBA");
		m_gBuffer->stop();
		m_gBuffer->start(BUFFER_TYPE_BLOOM,"RGBA",false);
		_passBlur("Vertical",BUFFER_TYPE_FREE2,"RGBA");
		m_gBuffer->stop();
	}

    if(RendererInfo::HDRInfo::hdr){
        m_gBuffer->start(BUFFER_TYPE_MISC);
        _passHDR();
        m_gBuffer->stop();
    }
    _passFinal();

    //copy depth over
    glColorMask(0,0,0,0);
	bindShaderProgram("Copy_Depth");

	bindTexture("gDepthMap",m_gBuffer->getTexture(BUFFER_TYPE_DEPTH),0);

    renderFullscreenQuad(Resources::getWindowSize().x,Resources::getWindowSize().y);

	unbindTexture2D(0);
	bindShaderProgram(0);
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
void Detail::RenderManagement::_passSSAO(){
	bindShaderProgram("Deferred_SSAO");

	sendUniform1iUnsafe("doSSAO",int(RendererInfo::SSAOInfo::ssao));
	sendUniform1iUnsafe("doBloom",int(RendererInfo::BloomInfo::bloom));

	Camera* c = Resources::getActiveCamera();
	glm::vec3 camPos = glm::vec3(c->getPosition());

	sendUniform3fUnsafe("gCameraPosition",camPos.x,camPos.y,camPos.z);
	sendUniform1fUnsafe("gIntensity",RendererInfo::SSAOInfo::ssao_intensity);
	sendUniform1fUnsafe("gBias",RendererInfo::SSAOInfo::ssao_bias);
	sendUniform1fUnsafe("gRadius",RendererInfo::SSAOInfo::ssao_radius);
	sendUniform1fUnsafe("gScale",RendererInfo::SSAOInfo::ssao_scale);
	sendUniform1iUnsafe("gSampleCount",RendererInfo::SSAOInfo::ssao_samples);
	sendUniform1iUnsafe("gNoiseTextureSize",RendererInfo::SSAOInfo::SSAO_NORMALMAP_SIZE);
	sendUniform2fvUnsafe("poisson[0]",RendererInfo::SSAOInfo::ssao_Kernels,RendererInfo::SSAOInfo::SSAO_KERNEL_COUNT);

	bindTexture("gNormalMap",m_gBuffer->getTexture(BUFFER_TYPE_NORMAL),0);
	bindTexture("gPositionMap",m_gBuffer->getTexture(BUFFER_TYPE_POSITION),1);
	bindTexture("gRandomMap",RendererInfo::SSAOInfo::ssao_noise_texture,2,GL_TEXTURE_2D);
	bindTexture("gMiscMap",m_gBuffer->getTexture(BUFFER_TYPE_MISC),3);
	bindTexture("gLightMap",m_gBuffer->getTexture(BUFFER_TYPE_LIGHTING),4);

    renderFullscreenQuad(Resources::getWindowSize().x,Resources::getWindowSize().y);

    for(uint i = 0; i < 5; i++){ unbindTexture2D(i); }
	bindShaderProgram(0);
}
void Detail::RenderManagement::_passEdge(GLuint texture, float radius){
	Settings::clear(true,false,false);

	bindShaderProgram("Deferred_Edge");
	sendUniform2fUnsafe("gScreenSize",float(Resources::getWindowSize().x),float(Resources::getWindowSize().y));
	sendUniform1fUnsafe("radius", radius);

	bindTexture("texture",m_gBuffer->getTexture(texture),0);

    renderFullscreenQuad(Resources::getWindowSize().x,Resources::getWindowSize().y);

	unbindTexture2D(0);
    bindShaderProgram(0);
}
void Detail::RenderManagement::_passGodsRays(glm::vec2 lightPositionOnScreen,bool behind,float alpha){
    Settings::clear(true,false,false);

	bindShaderProgram("Deferred_GodsRays");
	sendUniform1fUnsafe("decay",RendererInfo::GodRaysInfo::godRays_decay);
	sendUniform1fUnsafe("density",RendererInfo::GodRaysInfo::godRays_density);
	sendUniform1fUnsafe("exposure",RendererInfo::GodRaysInfo::godRays_exposure);
	sendUniform1iUnsafe("samples",RendererInfo::GodRaysInfo::godRays_samples);
	sendUniform1fUnsafe("weight",RendererInfo::GodRaysInfo::godRays_weight);
	sendUniform2fUnsafe("lightPositionOnScreen",
		float(lightPositionOnScreen.x)/float(Resources::getWindowSize().x),
		float(lightPositionOnScreen.y/float(Resources::getWindowSize().y))
	);

	sendUniform1iUnsafe("behind",int(behind));
	sendUniform1fUnsafe("alpha",alpha);

	bindTexture("firstPass",m_gBuffer->getTexture(BUFFER_TYPE_FREE1),0);

    renderFullscreenQuad(Resources::getWindowSize().x,Resources::getWindowSize().y);

	unbindTexture2D(0);
    bindShaderProgram(0);
}
void Detail::RenderManagement::_passHDR(){
    Settings::clear(true,false,false);

	bindShaderProgram("Deferred_HDR");
	sendUniform1fUnsafe("gamma",RendererInfo::HDRInfo::hdr_gamma);
	sendUniform1fUnsafe("exposure",RendererInfo::HDRInfo::hdr_exposure);

	bindTexture("lightingBuffer",m_gBuffer->getTexture(BUFFER_TYPE_LIGHTING),0);
    renderFullscreenQuad(Resources::getWindowSize().x,Resources::getWindowSize().y);

	unbindTexture2D(0);
	bindShaderProgram(0);
}
void Detail::RenderManagement::_passBlur(string type, GLuint texture,string channels){
	bindShaderProgram("Deferred_Blur");

	sendUniform1fUnsafe("radius",RendererInfo::BloomInfo::bloom_radius);
	sendUniform4fUnsafe("strengthModifier",RendererInfo::BloomInfo::bloom_strength,RendererInfo::BloomInfo::bloom_strength,RendererInfo::BloomInfo::bloom_strength,RendererInfo::SSAOInfo::ssao_blur_strength);

    if(channels.find("R") != string::npos) sendUniform1iUnsafe("R",1);
    else                                   sendUniform1iUnsafe("R",0);
    if(channels.find("G") != string::npos) sendUniform1iUnsafe("G",1);
    else                                   sendUniform1iUnsafe("G",0);
    if(channels.find("B") != string::npos) sendUniform1iUnsafe("B",1);
    else                                   sendUniform1iUnsafe("B",0);
    if(channels.find("A") != string::npos) sendUniform1iUnsafe("A",1);
    else                                   sendUniform1iUnsafe("A",0);

    if(type == "Horizontal"){ sendUniform2fUnsafe("HV",1.0f,0.0f); }
    else{                     sendUniform2fUnsafe("HV",0.0f,1.0f); }

	bindTexture("texture",m_gBuffer->getTexture(texture),0);

    renderFullscreenQuad(Resources::getWindowSize().x,Resources::getWindowSize().y);

	unbindTexture2D(0);
    bindShaderProgram(0);
}
void Detail::RenderManagement::_passFinal(){
    Settings::clear(true,false,false);

	bindShaderProgram("Deferred_Final");

	sendUniform1fUnsafe("gamma",RendererInfo::HDRInfo::hdr_gamma);

    glm::vec3 ambient = Resources::getCurrentScene()->getAmbientLightColor();
    sendUniform3fUnsafe("gAmbientColor",ambient.x,ambient.y,ambient.z);

	sendUniform1iUnsafe("HasSSAO",int(RendererInfo::SSAOInfo::ssao));
    sendUniform1iUnsafe("HasLighting",int(RendererInfo::LightingInfo::lighting));
    sendUniform1iUnsafe("HasBloom",int(RendererInfo::BloomInfo::bloom));
	sendUniform1iUnsafe("HasHDR",int(RendererInfo::HDRInfo::hdr));

	bindTexture("gDiffuseMap",m_gBuffer->getTexture(BUFFER_TYPE_DIFFUSE),0);
	bindTexture("gLightMap",m_gBuffer->getTexture(BUFFER_TYPE_LIGHTING),1);
	bindTexture("gBloomMap",m_gBuffer->getTexture(BUFFER_TYPE_BLOOM),2);
	bindTexture("gNormalMap",m_gBuffer->getTexture(BUFFER_TYPE_NORMAL),3);
	bindTexture("gMiscMap",m_gBuffer->getTexture(BUFFER_TYPE_MISC),4);
	bindTexture("gGodsRaysMap",m_gBuffer->getTexture(BUFFER_TYPE_GODSRAYS),5);

    renderFullscreenQuad(Resources::getWindowSize().x,Resources::getWindowSize().y);

    for(uint i = 0; i < 6; i++){ unbindTexture2D(i); }
	bindShaderProgram(0);
}
void Detail::renderFullscreenQuad(uint width,uint height){
    glm::mat4 m(1);
	glm::mat4 p = glm::ortho(-float(width)/2,float(width)/2,-float(height)/2,float(height)/2);

	sendUniformMatrix4fUnsafe("Model",m);
	sendUniformMatrix4fUnsafe("VP",p);

    glViewport(0,0,width,height);

	glBegin(GL_QUADS);
		glTexCoord2f(0,0); glVertex2f(-float(width)/2,-float(height)/2);
		glTexCoord2f(1,0); glVertex2f(float(width)/2,-float(height)/2);
		glTexCoord2f(1,1); glVertex2f(float(width)/2,float(height)/2);
		glTexCoord2f(0,1); glVertex2f(-float(width)/2,float(height)/2);
	glEnd();
}

#ifdef _WIN32
void Detail::RenderManagement::renderDirectX(){
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