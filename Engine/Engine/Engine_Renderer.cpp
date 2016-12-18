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

void Engine::Renderer::Detail::RenderManagement::init(){
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
void Engine::Renderer::Detail::RenderManagement::destruct(){
    SAFE_DELETE(RenderManagement::m_gBuffer);
    #ifdef _WIN32
    SAFE_DELETE_COM(RenderManagement::m_DirectXSwapChain);
    SAFE_DELETE_COM(RenderManagement::m_DirectXBackBuffer);
    SAFE_DELETE_COM(RenderManagement::m_DirectXDevice);
    SAFE_DELETE_COM(RenderManagement::m_DirectXDeviceContext);
    #endif
}
void Engine::Renderer::renderRectangle(glm::vec2 pos, glm::vec4 col, float w, float h, float angle, float depth){
    Detail::RenderManagement::getTextureRenderQueue().push_back(TextureRenderInfo("",pos,col,glm::vec2(w,h),angle,depth));
}
void Engine::Renderer::renderTexture(Texture* texture,glm::vec2 pos, glm::vec4 col,float angle, glm::vec2 scl, float depth){
	texture->render(pos,col,angle,scl,depth);
}
void Engine::Renderer::renderText(std::string text,Font* font, glm::vec2 pos,glm::vec4 color, float angle, glm::vec2 scl, float depth){
	font->renderText(text,pos,color,angle,scl,depth);
}

void Engine::Renderer::Detail::RenderManagement::_renderObjects(){
    for(auto item:m_ObjectsToBeRendered){
		item.object->draw(item.shader,RendererInfo::DebugDrawingInfo::debug,RendererInfo::GodRaysInfo::godRays);
    }
}
void Engine::Renderer::Detail::RenderManagement::_renderForegroundObjects(){
    for(auto item:m_ForegroundObjectsToBeRendered){
        item.object->draw(item.shader,RendererInfo::DebugDrawingInfo::debug);
    }
}
void Engine::Renderer::Detail::RenderManagement::_renderForwardRenderedObjects(){
    for(auto item:m_ObjectsToBeForwardRendered){
        item.object->draw(item.shader,RendererInfo::DebugDrawingInfo::debug);
    }
}
void Engine::Renderer::Detail::RenderManagement::_renderTextures(){
    GLuint shader = Resources::getShader("Deferred_HUD")->program();
    glUseProgram(shader);
    glUniform1f(glGetUniformLocation(shader, "far"),Resources::getActiveCamera()->getFar());
    glUniform1f(glGetUniformLocation(shader, "C"),1.0f);
    for(auto item:m_TexturesToBeRendered){
        Texture* texture = nullptr;
        if(item.texture != ""){
            texture = Resources::Detail::ResourceManagement::m_Textures[item.texture].get();
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, texture->address());
            glUniform1i(glGetUniformLocation(shader,"DiffuseTexture"),0);
            glUniform1i(glGetUniformLocation(shader,"DiffuseTextureEnabled"),1);
        }
        else{
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, 0);
            glUniform1i(glGetUniformLocation(shader,"DiffuseTexture"),0);
            glUniform1i(glGetUniformLocation(shader,"DiffuseTextureEnabled"),0);
        }
        glUniform1i(glGetUniformLocation(shader,"Shadeless"),1);
        glUniform4f(glGetUniformLocation(shader,"Object_Color"),item.col.r,item.col.g,item.col.b,item.col.a);

        glm::mat4 model = glm::mat4(1);
        model = glm::translate(model, glm::vec3(item.pos.x,
                                                item.pos.y,
                                                -0.001f - item.depth));
        model = glm::rotate(model, item.rot,glm::vec3(0,0,1));
        if(item.texture != "")
            model = glm::scale(model, glm::vec3(texture->width(),texture->height(),1));
        model = glm::scale(model, glm::vec3(item.scl.x,item.scl.y,1));

        glUniformMatrix4fv(glGetUniformLocation(shader, "VP"), 1, GL_FALSE, glm::value_ptr(m_2DProjectionMatrix));
        glUniformMatrix4fv(glGetUniformLocation(shader, "Model"), 1, GL_FALSE, glm::value_ptr(model));

        Resources::getMesh("Plane")->render();
    }
    glUseProgram(0);
}
void Engine::Renderer::Detail::RenderManagement::_renderText(){
    GLuint shader = Resources::getShader("Deferred_HUD")->program();
    glUseProgram(shader);
    for(auto item:m_FontsToBeRendered){
        Font* font = Resources::Detail::ResourceManagement::m_Fonts[item.texture].get();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, font->getFontData()->getGlyphTexture()->address());
        glUniform1i(glGetUniformLocation(shader,"DiffuseTexture"),0);
        glUniform1i(glGetUniformLocation(shader,"DiffuseTextureEnabled"),1);
        glUniform1i(glGetUniformLocation(shader,"Shadeless"),1);

        glUniform4f(glGetUniformLocation(shader, "Object_Color"),item.col.x,item.col.y,item.col.z,item.col.w);

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

                glUniformMatrix4fv(glGetUniformLocation(shader, "VP"), 1, GL_FALSE, glm::value_ptr(m_2DProjectionMatrix));
                glUniformMatrix4fv(glGetUniformLocation(shader, "Model"), 1, GL_FALSE, glm::value_ptr(glyph->m_Model));

                glyph->char_mesh->render();
                x += (glyph->xadvance) * item.scl.x;
            }
        }
    }
    glUseProgram(0);
}
void Engine::Renderer::Detail::RenderManagement::_passGeometry(){
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    Scene* s = Resources::getCurrentScene();
    glm::vec3 clear = s->getBackgroundColor();
    const float colors[4] = { clear.r, clear.g, clear.b, 1.0f };
    glClearBufferfv(GL_COLOR,BUFFER_TYPE_DIFFUSE,colors);
    glDisable(GL_BLEND); //disable blending on all mrts
    s->renderSkybox(RendererInfo::GodRaysInfo::godRays);

    glDepthMask(GL_TRUE);
    glEnable(GL_DEPTH_TEST);
    glEnablei(GL_BLEND,0); //enable blending on diffuse mrt only
    glBlendEquationi(GL_FUNC_ADD,0);
    glBlendFunci(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA,0);

    _renderObjects();

    glDepthMask(GL_FALSE);
    glDisable(GL_DEPTH_TEST);
}
void Engine::Renderer::Detail::RenderManagement::_passLighting(){
    GLuint shader = Resources::getShader("Deferred_Light")->program();
    glm::vec3 camPos = glm::vec3(Resources::getActiveCamera()->getPosition());
    glUseProgram(shader);

    glUniformMatrix4fv(glGetUniformLocation(shader, "VP" ), 1, GL_FALSE, glm::value_ptr(Resources::getActiveCamera()->getViewProjection()));
	glUniform4fv(glGetUniformLocation(shader,"materials"),MATERIAL_COUNT_LIMIT, glm::value_ptr(Material::m_MaterialProperities[0]));
	glUniform2f( glGetUniformLocation(shader,"gScreenSize"), (float)Resources::getWindowSize().x,(float)Resources::getWindowSize().y);

	glEnable(GL_TEXTURE_2D);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_gBuffer->getTexture(BUFFER_TYPE_NORMAL));
    glUniform1i( glGetUniformLocation(shader,"gNormalMap"), 0 );

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_gBuffer->getTexture(BUFFER_TYPE_POSITION));
    glUniform1i( glGetUniformLocation(shader,"gPositionMap"), 1 );

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, m_gBuffer->getTexture(BUFFER_TYPE_MISC));
    glUniform1i( glGetUniformLocation(shader,"gMiscMap"), 2 );

    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, m_gBuffer->getTexture(BUFFER_TYPE_DIFFUSE));
    glUniform1i( glGetUniformLocation(shader,"gDiffuseMap"), 3 );

    for (auto light:Resources::getCurrentScene()->getLights()){
        light.second->lighten(shader);
    }
    // Reset OpenGL state
    for(uint i = 0; i < 4; i++){
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
	glDisable(GL_TEXTURE_2D);
    glUseProgram(0);
}
void Engine::Renderer::Detail::RenderManagement::render(){
    for(auto object:Resources::getCurrentScene()->getObjects()){ object.second->render(0,RendererInfo::DebugDrawingInfo::debug); }

	if(!RendererInfo::GodRaysInfo::godRays)
		m_gBuffer->start(BUFFER_TYPE_DIFFUSE,BUFFER_TYPE_NORMAL,BUFFER_TYPE_MISC,BUFFER_TYPE_POSITION);
	else
		m_gBuffer->start(BUFFER_TYPE_DIFFUSE,BUFFER_TYPE_NORMAL,BUFFER_TYPE_MISC,BUFFER_TYPE_POSITION,BUFFER_TYPE_FREE1);
	RenderManagement::_passGeometry();
	m_gBuffer->stop();

	if(RendererInfo::GodRaysInfo::godRays){
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
    RenderManagement::_passSSAO(RendererInfo::SSAOInfo::ssao,RendererInfo::BloomInfo::bloom);
    m_gBuffer->stop();


	if(RendererInfo::SSAOInfo::ssao_do_blur || RendererInfo::BloomInfo::bloom){
		float& bloom_str = RendererInfo::BloomInfo::bloom_strength;
		float& ssao_str = RendererInfo::SSAOInfo::ssao_blur_strength;
		glm::vec4 str(bloom_str,bloom_str,bloom_str,ssao_str);

		m_gBuffer->start(BUFFER_TYPE_FREE2,"RGBA",false);
		RenderManagement::_passBlur("Horizontal",BUFFER_TYPE_BLOOM,RendererInfo::BloomInfo::bloom_radius,str,"RGBA");
		m_gBuffer->stop();
		m_gBuffer->start(BUFFER_TYPE_BLOOM,"RGBA",false);
		RenderManagement::_passBlur("Vertical",BUFFER_TYPE_FREE2,RendererInfo::BloomInfo::bloom_radius,str,"RGBA");
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
    GLuint shader = Resources::getShader("Copy_Depth")->program();
    glUseProgram(shader);
    glActiveTexture(GL_TEXTURE0);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, m_gBuffer->getTexture(BUFFER_TYPE_DEPTH));
    glUniform1i(glGetUniformLocation(shader,"gDepthMap"), 0 );

    renderFullscreenQuad(shader,Resources::getWindowSize().x,Resources::getWindowSize().y);

    glActiveTexture(GL_TEXTURE0);
    glDisable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);

    glUseProgram(0);
    glColorMask(1,1,1,1);
    /////////////

    glEnable(GL_BLEND);
    if(RendererInfo::DebugDrawingInfo::debug)
        Physics::Detail::PhysicsManagement::render();
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    _renderForwardRenderedObjects();
    _renderForegroundObjects();

    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GREATER, 0.1f);

    glDepthMask(GL_TRUE);
    glEnable(GL_DEPTH_TEST);

    glClear(GL_DEPTH_BUFFER_BIT);

    _renderTextures();
    _renderText();

    glDisable(GL_ALPHA_TEST);

	m_ObjectsToBeRendered.clear();
	m_ForegroundObjectsToBeRendered.clear();
	m_ObjectsToBeForwardRendered.clear();
	m_FontsToBeRendered.clear();
	m_TexturesToBeRendered.clear();
}
void Engine::Renderer::Detail::RenderManagement::_passSSAO(bool ssao, bool bloom){
    GLuint shader = Resources::getShader("Deferred_SSAO")->program();
    glUseProgram(shader);

    glUniform1i(glGetUniformLocation(shader,"doSSAO"),int(ssao));
    glUniform1i(glGetUniformLocation(shader,"doBloom"),int(bloom));

	glm::vec3 camPos = glm::vec3(Resources::getActiveCamera()->getPosition());
	glUniform3f(glGetUniformLocation(shader,"gCameraPosition"),camPos.x,camPos.y,camPos.z);
    glUniform1f(glGetUniformLocation(shader,"gIntensity"),RendererInfo::SSAOInfo::ssao_intensity);
    glUniform1f(glGetUniformLocation(shader,"gBias"),RendererInfo::SSAOInfo::ssao_bias);
    glUniform1f(glGetUniformLocation(shader,"gRadius"),RendererInfo::SSAOInfo::ssao_radius);
    glUniform1f(glGetUniformLocation(shader,"gScale"),RendererInfo::SSAOInfo::ssao_scale);
    glUniform1i(glGetUniformLocation(shader,"gSampleCount"),RendererInfo::SSAOInfo::ssao_samples);
    glUniform1i(glGetUniformLocation(shader,"gNoiseTextureSize"),RendererInfo::SSAOInfo::SSAO_NORMALMAP_SIZE);
    glUniform2fv(glGetUniformLocation(shader,"poisson"),Renderer::Detail::RendererInfo::SSAOInfo::SSAO_KERNEL_COUNT, glm::value_ptr(RendererInfo::SSAOInfo::ssao_Kernels[0]));
    glUniform1i(glGetUniformLocation(shader,"far"),int(Resources::getActiveCamera()->getFar()));

	glEnable(GL_TEXTURE_2D);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_gBuffer->getTexture(BUFFER_TYPE_NORMAL));
    glUniform1i(glGetUniformLocation(shader,"gNormalMap"), 0 );

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_gBuffer->getTexture(BUFFER_TYPE_POSITION));
    glUniform1i(glGetUniformLocation(shader,"gPositionMap"), 1 );

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D,RendererInfo::SSAOInfo::ssao_noise_texture);
    glUniform1i(glGetUniformLocation(shader,"gRandomMap"), 2 );

    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D,m_gBuffer->getTexture(BUFFER_TYPE_MISC));
    glUniform1i(glGetUniformLocation(shader,"gMiscMap"), 3 );

    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D,m_gBuffer->getTexture(BUFFER_TYPE_LIGHTING));
    glUniform1i(glGetUniformLocation(shader,"gLightMap"), 4 );


    renderFullscreenQuad(shader,Resources::getWindowSize().x,Resources::getWindowSize().y);

    for(uint i = 0; i < 5; i++){
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
	glDisable(GL_TEXTURE_2D);
    glUseProgram(0);
}
void Engine::Renderer::Detail::RenderManagement::_passEdge(GLuint texture, float radius){
    glClear(GL_COLOR_BUFFER_BIT);

    GLuint shader = Resources::getShader("Deferred_Edge")->program();
    glUseProgram(shader);

    glUniform2f(glGetUniformLocation(shader,"gScreenSize"),float(Resources::getWindowSize().x),float(Resources::getWindowSize().y));
    glUniform1f(glGetUniformLocation(shader,"radius"), radius);

    glActiveTexture(GL_TEXTURE0);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, m_gBuffer->getTexture(texture));
    glUniform1i(glGetUniformLocation(shader,"texture"), 0 );

    renderFullscreenQuad(shader,Resources::getWindowSize().x,Resources::getWindowSize().y);

    glActiveTexture(GL_TEXTURE0);
    glDisable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);

    glUseProgram(0);
}
void Engine::Renderer::Detail::RenderManagement::_passGodsRays(glm::vec2 lightPositionOnScreen,bool behind,float alpha){
    glClear(GL_COLOR_BUFFER_BIT);

    GLuint shader = Resources::getShader("Deferred_GodsRays")->program();
    glUseProgram(shader);

    glUniform1f(glGetUniformLocation(shader,"decay"), RendererInfo::GodRaysInfo::godRays_decay);
    glUniform1f(glGetUniformLocation(shader,"density"), RendererInfo::GodRaysInfo::godRays_density);
    glUniform1f(glGetUniformLocation(shader,"exposure"), RendererInfo::GodRaysInfo::godRays_exposure);
    glUniform1i(glGetUniformLocation(shader,"samples"), RendererInfo::GodRaysInfo::godRays_samples);
    glUniform1f(glGetUniformLocation(shader,"weight"), RendererInfo::GodRaysInfo::godRays_weight);
    glUniform2f(glGetUniformLocation(shader,"lightPositionOnScreen"),
		float(lightPositionOnScreen.x)/float(Resources::getWindowSize().x),
		float(lightPositionOnScreen.y/float(Resources::getWindowSize().y)
	));

	glUniform1i(glGetUniformLocation(shader,"behind"), int(behind));
	glUniform1f(glGetUniformLocation(shader,"alpha"), alpha);

    glActiveTexture(GL_TEXTURE0);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, m_gBuffer->getTexture(BUFFER_TYPE_FREE1));
    glUniform1i(glGetUniformLocation(shader,"firstPass"), 0 );

    renderFullscreenQuad(shader,Resources::getWindowSize().x,Resources::getWindowSize().y);

    glActiveTexture(GL_TEXTURE0);
    glDisable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D,0);

    glUseProgram(0);
}
void Engine::Renderer::Detail::RenderManagement::_passHDR(){
    glClear(GL_COLOR_BUFFER_BIT);

    GLuint shader = Resources::getShader("Deferred_HDR")->program();
    glUseProgram(shader);

    glUniform1f(glGetUniformLocation(shader,"gamma"),RendererInfo::HDRInfo::hdr_gamma);
    glUniform1f(glGetUniformLocation(shader,"exposure"),RendererInfo::HDRInfo::hdr_exposure);
    
    glActiveTexture(GL_TEXTURE0);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, m_gBuffer->getTexture(BUFFER_TYPE_LIGHTING));
    glUniform1i(glGetUniformLocation(shader,"lightingBuffer"), 0 );

    renderFullscreenQuad(shader,Resources::getWindowSize().x,Resources::getWindowSize().y);

    for(uint i = 0; i < 1; i++){
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, i);
    }
	glDisable(GL_TEXTURE_2D);
    glUseProgram(0);
}
void Engine::Renderer::Detail::RenderManagement::_passBlur(std::string type, GLuint texture, float radius,float str,std::string channels){
	_passBlur(type,texture,radius,glm::vec4(str),channels);
}
void Engine::Renderer::Detail::RenderManagement::_passBlur(std::string type, GLuint texture, float radius,glm::vec4 str,std::string channels){
    GLuint shader = Resources::getShader("Deferred_Blur")->program();
    glUseProgram(shader);

    glUniform1f(glGetUniformLocation(shader,"radius"), radius);
    glUniform4f(glGetUniformLocation(shader,"strengthModifier"), str.x,str.y,str.z,str.w);

    if(channels.find("R") != std::string::npos) glUniform1i(glGetUniformLocation(shader,"R"), 1);
    else                                        glUniform1i(glGetUniformLocation(shader,"R"), 0);
    if(channels.find("G") != std::string::npos) glUniform1i(glGetUniformLocation(shader,"G"), 1);
    else                                        glUniform1i(glGetUniformLocation(shader,"G"), 0);
    if(channels.find("B") != std::string::npos) glUniform1i(glGetUniformLocation(shader,"B"), 1);
    else                                        glUniform1i(glGetUniformLocation(shader,"B"), 0);
    if(channels.find("A") != std::string::npos) glUniform1i(glGetUniformLocation(shader,"A"), 1);
    else                                        glUniform1i(glGetUniformLocation(shader,"A"), 0);

    if(type == "Horizontal"){ 
        glUniform2f(glGetUniformLocation(shader,"HV"), 1.0f,0.0f);
    }
    else{
        glUniform2f(glGetUniformLocation(shader,"HV"), 0.0f,1.0f);
    }
    glActiveTexture(GL_TEXTURE0);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, m_gBuffer->getTexture(texture));
    glUniform1i(glGetUniformLocation(shader,"texture"), 0 );

    renderFullscreenQuad(shader,Resources::getWindowSize().x,Resources::getWindowSize().y);

    glActiveTexture(GL_TEXTURE0);
    glDisable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);

    glUseProgram(0);
}
void Engine::Renderer::Detail::RenderManagement::_passFinal(){
    glClear(GL_COLOR_BUFFER_BIT);

    GLuint shader = Resources::getShader("Deferred_Final")->program();
    glUseProgram(shader);

	glUniform1f(glGetUniformLocation(shader,"gamma"),RendererInfo::HDRInfo::hdr_gamma);
    glm::vec3 ambient = Resources::getCurrentScene()->getAmbientLightColor();
    glUniform3f(glGetUniformLocation(shader,"gAmbientColor"),ambient.x,ambient.y,ambient.z);

	glUniform1i( glGetUniformLocation(shader,"HasSSAO"),int(RendererInfo::SSAOInfo::ssao));
    glUniform1i( glGetUniformLocation(shader,"HasLighting"),int(RendererInfo::LightingInfo::lighting));
    glUniform1i( glGetUniformLocation(shader,"HasBloom"),int(RendererInfo::BloomInfo::bloom));
	glUniform1i( glGetUniformLocation(shader,"HasHDR"),int(RendererInfo::HDRInfo::hdr));

	glEnable(GL_TEXTURE_2D);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_gBuffer->getTexture(BUFFER_TYPE_DIFFUSE));
    glUniform1i( glGetUniformLocation(shader,"gDiffuseMap"), 0 );

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_gBuffer->getTexture(BUFFER_TYPE_LIGHTING));
    glUniform1i( glGetUniformLocation(shader,"gLightMap"), 1 );

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, m_gBuffer->getTexture(BUFFER_TYPE_BLOOM));
    glUniform1i( glGetUniformLocation(shader,"gBloomMap"), 2 );

    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, m_gBuffer->getTexture(BUFFER_TYPE_NORMAL));
    glUniform1i( glGetUniformLocation(shader,"gNormalMap"), 3 );

    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, m_gBuffer->getTexture(BUFFER_TYPE_MISC));
    glUniform1i( glGetUniformLocation(shader,"gMiscMap"), 4 );

    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, m_gBuffer->getTexture(BUFFER_TYPE_GODSRAYS));
    glUniform1i( glGetUniformLocation(shader,"gGodsRaysMap"), 5 );

    renderFullscreenQuad(shader,Resources::getWindowSize().x,Resources::getWindowSize().y);

    for(uint i = 0; i < 6; i++){
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
	glDisable(GL_TEXTURE_2D);
    glUseProgram(0);
}
void Engine::Renderer::Detail::renderFullscreenQuad(GLuint shader,uint width,uint height){
    glm::mat4 m(1);
	glm::mat4 p = glm::ortho(-float(width)/2,float(width)/2,-float(height)/2,float(height)/2);
    glUniformMatrix4fv(glGetUniformLocation(shader, "Model"), 1, GL_FALSE, glm::value_ptr(m));
	glUniformMatrix4fv(glGetUniformLocation(shader, "VP"), 1, GL_FALSE, glm::value_ptr(p));
    glViewport(0,0,width,height);

	glBegin(GL_QUADS);
		glTexCoord2f(0,0); glVertex2f(-float(width)/2,-float(height)/2);
		glTexCoord2f(1,0); glVertex2f(float(width)/2,-float(height)/2);
		glTexCoord2f(1,1); glVertex2f(float(width)/2,float(height)/2);
		glTexCoord2f(0,1); glVertex2f(-float(width)/2,float(height)/2);
	glEnd();
}

void Engine::Renderer::Detail::drawObject(ObjectDisplay* o, GLuint shader, bool debug,bool godsRays){
	Camera* camera = Resources::getActiveCamera();
	Engine::Renderer::Detail::RenderManagement::_drawObjectInternal(camera,camera->getDistance(o),camera->sphereIntersectTest(o),o->getColor(),o->getGodsRaysColor(),o->getRadius(),o->getDisplayItems(),o->getModel(),o->visible(),shader,debug,godsRays);
}
void Engine::Renderer::Detail::drawObject(ObjectDynamic* o, GLuint shader, bool debug,bool godsRays){
	Camera* camera = Resources::getActiveCamera();
	Engine::Renderer::Detail::RenderManagement::_drawObjectInternal(camera,camera->getDistance(o),camera->sphereIntersectTest(o),o->getColor(),o->getGodsRaysColor(),o->getRadius(),o->getDisplayItems(),o->getModel(),o->visible(),shader,debug,godsRays);
}
void Engine::Renderer::Detail::RenderManagement::_drawObjectInternal(Camera* camera,glm::num dist,bool intTest,glm::vec4& color, glm::vec3& raysColor,float radius,std::vector<DisplayItem*>& items,glm::m4 model,bool visible,GLuint shader, bool debug,bool godsRays){
	if((items.size() == 0 || visible == false) || (!intTest) || (dist > 1100 * radius))
        return;
    glUseProgram(shader);

    glUniformMatrix4fv(glGetUniformLocation(shader, "VP" ), 1, GL_FALSE, glm::value_ptr(camera->getViewProjection()));
    glUniform1f(glGetUniformLocation(shader, "far"),camera->getFar());
    glUniform1f(glGetUniformLocation(shader, "C"),1.0f);
	glUniform4f(glGetUniformLocation(shader, "Object_Color"),color.x,color.y,color.z,color.w);
	glUniform3f(glGetUniformLocation(shader, "Gods_Rays_Color"),raysColor.x,raysColor.y,raysColor.z);

	glm::vec3 camPos = glm::vec3(camera->getPosition());
	glUniform3f(glGetUniformLocation(shader,"CameraPosition"),camPos.x,camPos.y,camPos.z);

	if(godsRays) glUniform1i(glGetUniformLocation(shader, "HasGodsRays"),1);
	else         glUniform1i(glGetUniformLocation(shader, "HasGodsRays"),0);

    for(auto item:items){
		glm::mat4 m = glm::mat4(model);
        m = glm::translate(m,item->position);
        m *= glm::mat4_cast(item->orientation);
        m = glm::scale(m,item->scale);

        glUniform1i(glGetUniformLocation(shader, "Shadeless"),int(item->material->shadeless()));
        glUniform1f(glGetUniformLocation(shader, "BaseGlow"),item->material->glow());
		glUniform1f(glGetUniformLocation(shader, "matID"),float(float(item->material->id())/255.0f));

        glUniformMatrix4fv(glGetUniformLocation(shader, "Model" ), 1, GL_FALSE, glm::value_ptr(m));

		item->material->bind(shader,Resources::getAPI());
        item->mesh->render();
    }
    glUseProgram(0);
}

#ifdef _WIN32
void Engine::Renderer::Detail::RenderManagement::renderDirectX(){
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